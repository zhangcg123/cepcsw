
#include "BeamBackgroundFileParserV2.h"
#include "CLHEP/Random/RandFlat.h"
#include <sstream>
#include <cmath>

BeamBackgroundFileParserV2::BeamBackgroundFileParserV2(const std::string& dirpath,
                                                       const std::string& treename,
                                                       double beam_energy,
                                                       double beam_time,
                                                       int Nmcp) {

    //Choose one root file from the dir
    std::vector<std::string> rootFiles;
    for (const auto& entry : std::filesystem::directory_iterator(dirpath)) {
        if (entry.path().extension() == ".root") {
            rootFiles.push_back(entry.path().string());
        }
    }
    std::cout << "BeamBackgroundFileParserV2: In Dir " << dirpath << ": file size "<<rootFiles.size()<<std::endl;

    int randomIndex = CLHEP::RandFlat::shoot(0., rootFiles.size()-1.);
    std::cout << "  Chosen file index "<<randomIndex<<", file name "<<rootFiles[randomIndex]<<std::endl;
    m_inputFile.reset(TFile::Open(rootFiles[randomIndex].c_str(), "read"));
    std::cout << "BeamBackgroundFileParserV2: readin file name "<<m_inputFile->GetName()<<", status " << m_inputFile->IsOpen() << std::endl;
    m_readTree = (TTree*)m_inputFile->Get(treename.c_str());
    std::cout << "  Get Tree: entries " << m_readTree->GetEntries() << std::endl;
    m_beam_energy = beam_energy;
    m_beam_time = beam_time;
    m_Nmcp = Nmcp;

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

}

bool BeamBackgroundFileParserV2::load(IBeamBackgroundFileParser::BeamBackgroundData& result, int iEntry) {

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
        result.t = m_beam_time; // bunch time
        result.pdgid = pid;
        result.charge = charge;

        return true;

    }

    return false;
}

bool BeamBackgroundFileParserV2::SampleParticleNum(int& npart, int& start ){

  if(m_Nmcp==-1){
    npart = m_readTree->GetEntries();
  }
  else npart = m_Nmcp;

  start = 0;
  return true;
}
