#ifndef TofRecAlg_h
#define TofRecAlg_h 1

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include <random>
#include "GaudiKernel/NTuple.h"
#include "edm4cepc/RecTofCollection.h"

class TofRecAlg : public Algorithm {
 public:
  // Constructor of this form must be provided
  TofRecAlg( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:
  DataHandle<edm4hep::MCParticleCollection> _inMCColHdl{"MCParticle", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::TrackCollection> _inTrackColHdl{"CompleteTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::RecTofCollection> m_rectofCol{"RecTofCollection", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<double> m_field{this, "Field", 3.0};

  NTuple::Tuple*       m_tuple;
  NTuple::Item<long>   m_nTracks;
  NTuple::Array<float> m_x;
  NTuple::Array<float> m_y;
  NTuple::Array<float> m_z;
  NTuple::Array<float> m_px;
  NTuple::Array<float> m_py;
  NTuple::Array<float> m_pz;
  NTuple::Array<float> m_p;
  NTuple::Array<float> m_d0;
  NTuple::Array<float> m_phi0;
  NTuple::Array<float> m_omega;
  NTuple::Array<float> m_z0;
  NTuple::Array<float> m_tanLambda;
  NTuple::Array<float> m_sigma_d0;
  NTuple::Array<float> m_sigma_phi0;
  NTuple::Array<float> m_sigma_omega;
  NTuple::Array<float> m_sigma_z0;
  NTuple::Array<float> m_sigma_tanLambda;
  
  NTuple::Array<float> m_ToFt;
  NTuple::Array<float> m_ToFterr;
  NTuple::Array<float> m_ToFx;
  NTuple::Array<float> m_ToFy;
  NTuple::Array<float> m_ToFz;
  
  NTuple::Matrix<float> trackState0;
  NTuple::Matrix<float> trackState1;
  NTuple::Matrix<float> trackState2;
  NTuple::Matrix<float> trackState3;
  NTuple::Matrix<float> trackState4;
  NTuple::Matrix<float> trackState5;

  std::map<int, NTuple::Matrix<float>*> trackStateMap = {
    {0, &trackState0},
    {1, &trackState1},
    {2, &trackState2},
    {3, &trackState3},
    {4, &trackState4},
    {5, &trackState5}
  };

  NTuple::Array<float> m_length;
  NTuple::Array<float> m_lengtherr;

  NTuple::Array<float> m_p_atIP;
  NTuple::Array<float> m_perr_atIP;
  NTuple::Array<float> m_p_atLast;
  NTuple::Array<float> m_perr_atLast;

  NTuple::Matrix<double> m_exp_time;
  NTuple::Matrix<double> m_exp_timeerr;

  int _nEvt;

  const std::map<int, double> masses = {//masses in GeV for e mu pi K p
    {0, 0.000511},
    {1, 0.105658},
    {2, 0.139570},
    {3, 0.493677},
    {4, 0.938272},
  };

  float GeV2kgms = 5.344286e-19;//momenta in GeV to kg m/s
  float GeV2kg = 1.78266192e-27;//mass in GeV to kg
  float c = 2.99792458e8;//spead of light in m/s
  float q = 1.60217662e-19;//electron charge in C

  float bunchcrossing = 0.02;//ns
  float tof_resolution = 0.05;//ns
  std::default_random_engine generator_tof;
  std::normal_distribution<float> normal_distribution_tof{0, tof_resolution};
  std::default_random_engine generator_bunch;
  std::normal_distribution<float> normal_distribution_bunch{0, bunchcrossing};


};

#endif
