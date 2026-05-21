#ifndef BeamBackgroundFileParserV0_h
#define BeamBackgroundFileParserV0_h

#include "IBeamBackgroundFileParser.h"

#include <fstream>

class BeamBackgroundFileParserV0: public IBeamBackgroundFileParser {
public:
    BeamBackgroundFileParserV0(const std::string& filename, int pdgid, double beam_energy);

    bool load(IBeamBackgroundFileParser::BeamBackgroundData& _data);
    bool load(IBeamBackgroundFileParser::BeamBackgroundData& _data, int iEntry) { return load(_data); }
    bool SampleParticleNum(int&, int&) { return true; }
    //int totalEnteries() { return 0; }

private:
    std::ifstream m_input;

    int m_pdgid;
    double m_beam_energy;
};

#endif
