#ifndef BeamBackgroundFileParserV2_h
#define BeamBackgroundFileParserV2_h

#include "IBeamBackgroundFileParser.h"
#include "TFile.h"
#include "TTree.h"
#include <fstream>
#include <filesystem>
#include <random>

class BeamBackgroundFileParserV2: public IBeamBackgroundFileParser {
public:
    BeamBackgroundFileParserV2(const std::string& dirname, const std::string& treename, double beam_energy, double beam_time, int Nmcp);

    bool load(IBeamBackgroundFileParser::BeamBackgroundData&) { return 0; }
    bool load(IBeamBackgroundFileParser::BeamBackgroundData&, int iEntry);
    bool SampleParticleNum(int&, int&);
    //int totalEnteries() { return m_readTree->GetEntries(); }

private:
    std::unique_ptr<TFile> m_inputFile;
    TTree* m_readTree;
    double m_beam_energy, m_beam_time;;
    int m_Nmcp;

    double x, y, z, cosx, cosy, dz, dp, cosz;
    int pid, charge;

};

#endif
