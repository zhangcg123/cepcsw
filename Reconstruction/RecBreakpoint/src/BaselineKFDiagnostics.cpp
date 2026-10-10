#include "BaselineKFDiagnostics.h"
#include "BreakpointTrackSystem.h"
#include "TrackSystemSvc/IMarlinTrack.h"
#include "TrackSystemSvc/MarlinTrkUtils.h"
#include "kaldet/ILDVMeasLayer.h"
#include "kaldet/ILDVTrackHit.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

namespace breakpoint {
namespace {
using Matrix = MarlinTrk::MeasurementUpdate::Matrix;
std::vector<double> flatten(const TMatrixD &m) {
  std::vector<double> v;
  for (int i = 0; i < m.GetNrows(); ++i)
    for (int j = 0; j < m.GetNcols(); ++j)
      v.push_back(m(i, j));
  return v;
}
TMatrixD matrix(const Matrix &m) {
  TMatrixD r(m.rows, m.cols);
  for (int i = 0; i < m.rows; ++i)
    for (int j = 0; j < m.cols; ++j)
      r(i, j) = m.values.at(i * m.cols + j);
  return r;
}
TMatrixD leading(const Matrix &m, int rows, int cols) {
  if (m.rows < rows || m.cols < cols)
    throw std::runtime_error("Native matrix too small");
  return matrix(m).GetSub(0, rows - 1, 0, cols - 1);
}
// Passive prediction measurement, using the baseline layer implementation.
// The public diagnostic residual is retained separately, since its
// reconstruction from the post-update residual need not equal m-h(x_pred) for a
// nonlinear hit.
void predictionMeasurement(BreakpointTrackSystem &system,
                           edm4hep::TrackerHit hit,
                           MarlinTrk::MeasurementUpdate &u) {
  const auto *layer = system.layer(hit);
  if (!layer)
    throw std::runtime_error("Missing measurement layer");
  std::unique_ptr<ILDVTrackHit> native(layer->ConvertLCIOTrkHit(hit));
  if (!native)
    throw std::runtime_error("Cannot convert measurement");
  TKalTrackSite site(*native, u.predictedState.rows);
  TKalTrackState state(TKalMatrix(matrix(u.predictedState)), site,
                       TVKalSite::kPredicted, u.predictedState.rows);
  TKalMatrix expected(site.GetDimension(), 1),
      h(site.GetDimension(), u.predictedState.rows);
  if (!site.CalcExpectedMeasVec(state, expected) ||
      !site.CalcMeasVecDerivative(state, h))
    throw std::runtime_error("Cannot evaluate baseline predicted measurement");
  auto set = [](Matrix &to, const TMatrixD &from) {
    to.rows = from.GetNrows();
    to.cols = from.GetNcols();
    to.values = flatten(from);
  };
  set(u.predictedMeasurement, expected);
  set(u.residual, site.GetMeasVec() - expected);
  set(u.projector, h);
  set(u.measurementCovariance, site.GetMeasNoiseMat());
  set(u.innovationCovariance,
      site.GetMeasNoiseMat() +
          h * matrix(u.predictedCovariance) * transpose(h));
}
void require(int code, const char *operation) {
  if (code != MarlinTrk::IMarlinTrack::success)
    throw std::runtime_error(std::string(operation) + ": " +
                             std::to_string(code));
}
double quadratic(const TMatrixD &r, const TMatrixD &v) {
  const TMatrixD q = transpose(r) * inverseCovariance(v) * r;
  return q(0, 0);
}
} // namespace

void BaselineDirection::resize(std::size_t n) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  updateStatus.assign(n, -999);
  diagnosticValid.assign(n, 0);
  dimension.assign(n, 0);
  nativeDimension.assign(n, 0);
  previousHit.assign(n, -1);
  smoothStatus.assign(n, 0);
  localChi2.assign(n, nan);
  residualChi2.assign(n, nan);
  smoothedResidualChi2.assign(n, nan);
  for (auto *v : {&predicted,
                  &predictedCovariance,
                  &updated,
                  &updatedCovariance,
                  &pivot,
                  &innovation,
                  &innovationCovariance,
                  &projector,
                  &measurement,
                  &measurementCovariance,
                  &transport,
                  &processNoise,
                  &smoothed,
                  &smoothedCovariance,
                  &smoothedResidual,
                  &smoothedResidualCovariance,
                  &nativePredicted,
                  &nativePredictedCovariance,
                  &nativeProjector,
                  &nativeTransport,
                  &nativeNoise,
                  &backendResidual})
    v->resize(n);
}

BaselineDirection
baselineDiagnostics(BreakpointTrackSystem &system,
                    const std::vector<edm4hep::TrackerHit> &hits, double bz,
                    const std::vector<double> &variances, double maxChi2,
                    bool backward, bool smooth) {
  BaselineDirection out;
  out.resize(hits.size());
  try {
    if (hits.size() < 3 || variances.size() != 5)
      throw std::runtime_error("Insufficient hits or invalid seed");
    // Exact standard first/middle/last 2D-hit prefit; both directions receive
    // their own loose covariance, never the other direction's posterior.
    auto copy = hits;
    edm4hep::TrackState seed{};
    const bool direction = backward ? MarlinTrk::IMarlinTrack::backward
                                    : MarlinTrk::IMarlinTrack::forward;
    require(MarlinTrk::createPrefit(copy, &seed, bz, direction),
            "createPrefit");
    seed.covMatrix.fill(0);
    const int diagonal[] = {0, 2, 5, 9, 14};
    for (int i = 0; i < 5; ++i)
      seed.covMatrix[diagonal[i]] = variances[i];
    const auto initial = fromEDM(seed, bz);
    out.seedParameters = flatten(initial.mean);
    out.seedCovariance = flatten(initial.covariance);
    out.seedPivot = {initial.pivot.x, initial.pivot.y, initial.pivot.z};
    std::unique_ptr<MarlinTrk::IMarlinTrack> track(system.createTrack());
    if (!track)
      throw std::runtime_error("Cannot create baseline KF");
    auto first = backward ? hits.back() : hits.front();
    require(track->addHit(first), "register seed surface");
    require(track->initialise(seed, bz, direction), "initialize");
    std::vector<MarlinTrk::MeasurementUpdate> updates(hits.size());
    std::vector<int> accepted;
    int previous = -1;
    for (std::size_t k = 0; k < hits.size(); ++k) {
      const int i = backward ? hits.size() - 1 - k : k;
      auto hit = hits[i];
      double chi2 = std::numeric_limits<double>::quiet_NaN();
      auto &u = updates[i];
      out.previousHit[i] = previous;
      out.updateStatus[i] = track->addAndFit(hit, chi2, u, maxChi2);
      out.localChi2[i] = chi2;
      if (out.updateStatus[i] != MarlinTrk::IMarlinTrack::success)
        continue;
      accepted.push_back(i);
      previous = i;
      out.chi2 += chi2;
      edm4hep::TrackState state{};
      double cumulative;
      int ndf;
      require(track->getTrackState(hit, state, cumulative, ndf),
              "updated state");
      const auto x = fromEDM(state, bz);
      out.updated[i] = flatten(x.mean);
      out.updatedCovariance[i] = flatten(x.covariance);
      out.pivot[i] = {x.pivot.x, x.pivot.y, x.pivot.z};
      // Retain an explicit diagnostic failure code for backend incompatibility.
      out.diagnosticValid[i] = -100;
      out.nativeDimension[i] = u.predictedState.rows;
      if (!u.valid || u.predictedState.rows < 5)
        continue;
      out.backendResidual[i] = u.residual.values;
      predictionMeasurement(system, hit, u);
      out.nativePredicted[i] = u.predictedState.values;
      out.nativePredictedCovariance[i] = u.predictedCovariance.values;
      out.nativeProjector[i] = u.projector.values;
      out.nativeTransport[i] = u.transportJacobian.values;
      out.nativeNoise[i] = u.processNoiseCovariance.values;
      out.diagnosticValid[i] = 1;
      out.dimension[i] = u.residual.rows;
      out.predicted[i] = flatten(leading(u.predictedState, 5, 1));
      out.predictedCovariance[i] =
          flatten(leading(u.predictedCovariance, 5, 5));
      out.innovation[i] = u.residual.values;
      out.innovationCovariance[i] = u.innovationCovariance.values;
      out.projector[i] = flatten(leading(u.projector, u.projector.rows, 5));
      out.measurementCovariance[i] = u.measurementCovariance.values;
      if (u.transportJacobian.rows >= 5)
        out.transport[i] = flatten(leading(u.transportJacobian, 5, 5));
      if (u.processNoiseCovariance.rows >= 5)
        out.processNoise[i] = flatten(leading(u.processNoiseCovariance, 5, 5));
      out.measurement[i] =
          flatten(matrix(u.predictedMeasurement) + matrix(u.residual));
      try {
        out.residualChi2[i] =
            quadratic(matrix(u.residual), matrix(u.innovationCovariance));
      } catch (const std::exception &) {
        out.diagnosticValid[i] = -1;
      }
    }
    if (accepted.empty())
      throw std::runtime_error("No accepted hits");
    out.status = 1;
    if (!backward) {
      // Separate provenance check: standard KalTestTool finalization uses a
      // copied outer state followed by inward updates WITHOUT the chi2 cut.
      // This is NOT the independently seeded backward feature pass.
      out.productionContinuationStatus = -1;
      auto last = hits[accepted.back()];
      edm4hep::TrackState outer{};
      double total;
      int ndf;
      if (track->getTrackState(last, outer, total, ndf) == 0) {
        std::unique_ptr<MarlinTrk::IMarlinTrack> continuation(
            system.createTrack());
        bool ok = continuation && continuation->addHit(last) == 0 &&
                  continuation->initialise(
                      outer, bz, MarlinTrk::IMarlinTrack::forward) == 0;
        for (int j = static_cast<int>(accepted.size()) - 2; ok && j >= 0; --j) {
          auto h = hits[accepted[j]];
          double q;
          ok = continuation->addAndFit(h, q,
                                       std::numeric_limits<double>::max()) == 0;
        }
        edm4hep::TrackState ip{};
        auto inner = hits[accepted.front()];
        if (ok && continuation->propagate(edm4hep::Vector3d{0, 0, 0}, inner, ip,
                                          total, ndf) == 0) {
          const auto x = fromEDM(ip, bz);
          out.productionIPParameters = flatten(x.mean);
          out.productionIPCovariance = flatten(x.covariance);
          out.productionContinuationStatus = 1;
        }
      }
    }
    if (smooth) {
      out.smoothingStatus = track->smooth() == 0 ? 1 : -1;
      if (out.smoothingStatus == 1)
        for (int i : accepted) {
          auto hit = hits[i];
          edm4hep::TrackState state{};
          double chi2;
          int ndf;
          if (track->getTrackState(hit, state, chi2, ndf) != 0) {
            out.smoothStatus[i] = -1;
            continue;
          }
          const auto x = fromEDM(state, bz);
          out.smoothed[i] = flatten(x.mean);
          out.smoothedCovariance[i] = flatten(x.covariance);
          out.smoothStatus[i] = 1;
          const auto &u = updates[i];
          if (!u.valid || u.predictedState.rows < 5)
            continue;
          // Public TrackState exposes only the five helix parameters. Never
          // invent the smoothed nuisance coordinate if a measurement uses it.
          bool usesNuisance = false;
          for (int a = 0; a < u.projector.rows; ++a)
            for (int b = 5; b < u.projector.cols; ++b)
              usesNuisance |= u.projector.values[a * u.projector.cols + b] != 0;
          if (usesNuisance) {
            out.smoothStatus[i] = 3;
            continue;
          }
          // Evaluate the SAME affine measurement model as the forward update.
          // Included-hit residual covariance is V-H P_s H^T, NOT V+H P_s H^T.
          const auto h = leading(u.projector, u.projector.rows, 5);
          const TMatrixD r =
              matrix(u.residual) -
              h * stateDifference(x.mean, leading(u.predictedState, 5, 1));
          const TMatrixD v =
              matrix(u.measurementCovariance) - h * x.covariance * transpose(h);
          out.smoothedResidual[i] = flatten(r);
          out.smoothedResidualCovariance[i] = flatten(v);
          try {
            out.smoothedResidualChi2[i] = quadratic(r, v);
          } catch (const std::exception &) {
            out.smoothStatus[i] = 2;
          } // state valid; residual score unavailable
        }
    }
    auto endpoint = hits[backward ? accepted.back() : accepted.front()];
    edm4hep::TrackState ip{};
    double chi2;
    int ndf;
    if (track->propagate(edm4hep::Vector3d{0, 0, 0}, endpoint, ip, chi2, ndf) ==
        0) {
      const auto x = fromEDM(ip, bz);
      out.ipParameters = flatten(x.mean);
      out.ipCovariance = flatten(x.covariance);
    }
  } catch (const std::exception &e) {
    out.status = -1;
    out.error = e.what();
  }
  return out;
}
} // namespace breakpoint
