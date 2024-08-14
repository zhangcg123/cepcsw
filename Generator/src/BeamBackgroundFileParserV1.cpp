
#include "BeamBackgroundFileParserV1.h"
#include "CLHEP/Random/RandPoisson.h"
#include "CLHEP/Random/RandFlat.h"
#include <iostream>
#include <sstream>
#include <cmath>

BeamBackgroundFileParserV1::BeamBackgroundFileParserV1(const std::string& filename,
                                                       const std::string& treename,
                                                       double beam_energy, 
                                                       double rate,
                                                       double timewindow,
                                                       int Nmcp) {


    m_inputFile.reset(TFile::Open(filename.c_str(), "read"));
    std::cout << "BeamBackgroundFileParserV1: readin file name "<<filename<<" / "<<m_inputFile->GetName()<<", status " << m_inputFile->IsOpen() << std::endl;

    m_readTree = (TTree*)m_inputFile->Get(treename.c_str());
    std::cout << "  Get Tree: entries " << m_readTree->GetEntries() << std::endl;
    m_beam_energy = beam_energy;

    m_readTree->SetBranchAddress("x", &x);
    m_readTree->SetBranchAddress("y", &y);
    m_readTree->SetBranchAddress("z", &z);
    m_readTree->SetBranchAddress("cosx", &cosx);
    m_readTree->SetBranchAddress("cosy", &cosy);
    m_readTree->SetBranchAddress("dp", &dp);
    m_readTree->SetBranchAddress("dz", &dz);
    m_readTree->SetBranchAddress("pid", &pid);
    m_readTree->SetBranchAddress("charge", &charge);
    m_readTree->SetBranchAddress("cosz", &cosz);

    m_rate = rate;
    m_timewindow = timewindow;
    m_Nmcp = Nmcp;
}

bool BeamBackgroundFileParserV1::load(IBeamBackgroundFileParser::BeamBackgroundData& result, int iEntry) {

    if (not m_inputFile->IsOpen()) {
        return false;
    }

    if(m_readTree && m_readTree->GetEntries()>=0){
        if(iEntry>=m_readTree->GetEntries()) iEntry = m_readTree->GetEntries()-1;
        m_readTree->GetEntry(iEntry);

        double p = m_beam_energy*(1+dp);
        const double m2mm = 1e3; // convert from m to mm
        result.x     = x * m2mm;
        result.y     = y * m2mm;
        result.z     = (z+dz) * m2mm;
        result.px    = p * cosx;
        result.py    = p * cosy;
        result.pz    = p * cosz;
        result.mass  = 0.000511; // assume e-/e+, mass is 0.511 MeV
        result.t = CLHEP::RandFlat::shoot(0., m_timewindow); // uniformly distributed within a time window, and ns
        result.pdgid = pid;
        result.charge = charge;
        return true;

    }

    return false;
}

bool BeamBackgroundFileParserV1::SampleParticleNum(int& npart, int& start ){

  if(m_Nmcp==-1){
    npart = int(m_rate * m_timewindow);
    npart = CLHEP::RandPoisson::shoot(npart);
  }
  else npart = m_Nmcp;

  int nTotPartInFile = m_readTree->GetEntries();
  if(npart>nTotPartInFile){
    start = 0;
    npart = nTotPartInFile;
  }
  else start = CLHEP::RandFlat::shoot(0., (double)nTotPartInFile-npart);

  return true;
}
