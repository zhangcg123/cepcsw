
#include "BeamBackgroundFileParserV2.h"
#include "CLHEP/Random/RandFlat.h"
#include <sstream>
#include <cmath>

BeamBackgroundFileParserV2::BeamBackgroundFileParserV2(const std::string& dirpath,
                                                       const std::string& treename,
                                                       double beam_energy) {

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
    std::cout << "BeamBackgroundFileParserV1: readin file name "<<m_inputFile->GetName()<<", status " << m_inputFile->IsOpen() << std::endl;
    m_readTree = (TTree*)m_inputFile->Get(treename.c_str());
    std::cout << "  Get Tree: entries " << m_readTree->GetEntries() << std::endl;
    m_beam_energy = beam_energy;

    m_readTree->SetBranchAddress("X", &x);
    m_readTree->SetBranchAddress("Y", &y);
    m_readTree->SetBranchAddress("Z", &z);
    m_readTree->SetBranchAddress("PX", &px);
    m_readTree->SetBranchAddress("PY", &py);
    m_readTree->SetBranchAddress("DP", &dp);
    m_readTree->SetBranchAddress("PID", &pid);

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
        result.pdgid = (int)pid;
        result.x     = x * m2mm;
        result.y     = y * m2mm;
        result.z     = z * m2mm;

        result.px    = px;
        result.py    = py;
        result.pz    = sqrt(p*p - px*px - py*py);

        if(pid==22) result.mass  = 0.;
        else result.mass  = 0.000511; // assume e-/e+, mass is 0.511 MeV

        return true;

    }

    return false;
}

bool BeamBackgroundFileParserV2::SampleParticleNum(int& npart, int& start ){
  npart = m_readTree->GetEntries();
  start = 0;

  return true;
}
