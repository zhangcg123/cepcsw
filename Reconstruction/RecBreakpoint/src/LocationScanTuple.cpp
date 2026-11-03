#include "HitLocationScan.h"
#include "edm4hep/TrackState.h"
#include "TTree.h"

namespace breakpoint {
void LocationFitScan::book(TTree& tree) {
  auto branch=[&](const char* suffix, auto* value) {
    tree.Branch((std::string("location_scan_")+suffix).c_str(),value);
  };
  branch("status",&status); branch("point_count",&pointCount);
  branch("truth_status",&truthStatus); branch("truth_hit",&truthHit);
  branch("truth_point",&truthPoint); branch("truth_error",&truthError);
  branch("point",&point); branch("kind",&kind); branch("hit",&hit);
  branch("system",&system); branch("layer",&layer); branch("setup",&setup);
  branch("actual_constraint_mask",&actualMask); branch("fit_status",&fitStatus);
  branch("resolved_ecal_sum_applied",&resolvedEcalSum);
  branch("attempt_status",&attemptStatus); branch("minuit_status",&minuitStatus);
  branch("attempt_minuit_status",&attemptMinuitStatus); branch("source_setup",&sourceSetup);
  branch("measurement_dimensions",&measurementDimensions);
  branch("nll2",&nll2); branch("chi2",&chi2); branch("log_determinant",&logDeterminant);
  branch("pt",&pt); branch("pt_error",&ptError); branch("prior_center",&priorCenter);
  branch("ip_parameters",&ipParameters); branch("ip_covariance",&ipCovariance);
  branch("ip_reference_point",&ipReferencePoint); branch("ip_time",&ipTime);
  branch("ip_location",&ipLocation);
  branch("fitted_log_loss",&fittedLoss); branch("fitted_variance",&fittedVariance);
  branch("error",&error);
}

void LocationFitScan::appendIP(const edm4hep::TrackState* state) {
  if (state) {
    ipParameters.insert(ipParameters.end(), {state->D0,state->phi,state->omega,state->Z0,state->tanLambda});
    ipCovariance.insert(ipCovariance.end(),state->covMatrix.begin(),state->covMatrix.end());
    ipReferencePoint.insert(ipReferencePoint.end(),
        {state->referencePoint.x,state->referencePoint.y,state->referencePoint.z});
    ipTime.push_back(state->time); ipLocation.push_back(state->location);
  } else {
    const double nan=std::numeric_limits<double>::quiet_NaN();
    ipParameters.insert(ipParameters.end(),5,nan);
    ipCovariance.insert(ipCovariance.end(),21,nan);
    ipReferencePoint.insert(ipReferencePoint.end(),3,nan);
    ipTime.push_back(nan); ipLocation.push_back(-1);
  }
}
} // namespace breakpoint
