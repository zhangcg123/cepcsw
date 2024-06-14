/*
 * Description:
 *   This algorithm is used to test the EDM4hep extension for cepc.
 *
 *   In the extension, we keep the same namespace as edm4hep. 
 *   The header file path is different.
 *
 * Author: 
 *   Tao Lin <lintao AT ihep.ac.cn>
 */

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"

#include "edm4cepc/RecTofCollection.h"

class Edm4cepcWriteAlg: public GaudiAlgorithm {
public:

    Edm4cepcWriteAlg(const std::string& name, ISvcLocator* svcLoc)
        : GaudiAlgorithm(name, svcLoc) {

    }

    StatusCode initialized() {

        return GaudiAlgorithm::initialize();
    }

    StatusCode execute() {

        auto rectofCol = m_rectofCol.createAndPut();

        for (size_t i = 0; i < 3; ++i) {
            auto rectof = rectofCol->create();

            rectof.setTime(100.);
            rectof.setTimeExp({99.,99.5,100,100.5,101});
            rectof.setSigma(1.);
            rectof.setPathLength({50,50.5,51,51.5,52});
            rectof.setPosition({100,100,10});
        }

        return StatusCode::SUCCESS;
    }

    StatusCode finalize() {

        return GaudiAlgorithm::finalize();
    }

private:
    DataHandle<edm4hep::RecTofCollection> m_rectofCol{"RecTofCollection", Gaudi::DataHandle::Writer, this};

};

DECLARE_COMPONENT(Edm4cepcWriteAlg)
