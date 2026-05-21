#ifndef TEST_PLCIO_WRITE_ALG_H
#define TEST_PLCIO_WRITE_ALG_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"

namespace plcio {
    class EventHeaderCollection;
    class MCParticleCollection;
}

class PlcioReadAlg : public Algorithm
{

    public :

        PlcioReadAlg(const std::string& name, ISvcLocator* svcLoc);

        virtual StatusCode initialize();
        virtual StatusCode execute();
        virtual StatusCode finalize();

    private :

        DataHandle<plcio::EventHeaderCollection> m_headerCol{"EventHeader", Gaudi::DataHandle::Reader, this};
        DataHandle<plcio::MCParticleCollection> m_mcParCol{"MCParticle", Gaudi::DataHandle::Reader, this};

};

#endif  // TEST_PLCIO_WRITE_ALG_H
