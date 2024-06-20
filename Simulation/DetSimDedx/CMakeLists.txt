
gaudi_add_module(DetSimDedx
                 SOURCES src/DummyDedxSimTool.cpp
                         src/BetheBlochEquationDedxSimTool.cpp
                         src/GFDndxSimTool.cpp
                         src/TrackHeedSimTool.cpp

                 LINK DetSimInterface
                      DetInterface
                      DetSegmentation
                      ${DD4hep_COMPONENT_LIBRARIES}
                      Gaudi::GaudiKernel
                      EDM4HEP::edm4hep EDM4HEP::edm4hepDict
                      k4FWCore::k4FWCore
                      Garfield::Garfield
                      OnnxRuntime
                      ${CLHEP_LIBRARIES}
 
)

