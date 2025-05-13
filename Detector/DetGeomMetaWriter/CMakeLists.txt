gaudi_add_module(DetGeomMetaWriter
                 SOURCES src/DetGeomMetaWriter.h
                         src/DetGeomMetaWriter.cpp
                 LINK DetInterface
                      ${DD4hep_COMPONENT_LIBRARIES} 
                      Gaudi::GaudiKernel
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      k4FWCore::k4FWCore
)