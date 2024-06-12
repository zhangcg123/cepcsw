#ifndef BeamBackgroundFileParserV1_h
#define BeamBackgroundFileParserV1_h

#include "IBeamBackgroundFileParser.h"
#include "TFile.h"
#include "TTree.h"
#include <fstream>

class BeamBackgroundFileParserV1: public IBeamBackgroundFileParser {
public:
    BeamBackgroundFileParserV1(const std::string& filename, const std::string& treename, double beam_energy, double rate, double timewindow);

    bool load(IBeamBackgroundFileParser::BeamBackgroundData&) { return 0; }
    bool load(IBeamBackgroundFileParser::BeamBackgroundData&, int iEntry);
    bool SampleParticleNum(int&, int&);
    //int totalEnteries() { return m_readTree->GetEntries(); }

private:
    std::unique_ptr<TFile> m_inputFile;
    TTree* m_readTree;
    double m_beam_energy;
    double m_rate;
    double m_timewindow; 

    double x, y, z, cosx, cosy, dz, dp, cosz;
    int pid;

};

#endif
