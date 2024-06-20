
gaudi_add_module(DetSimFastModel
                 SOURCES src/DummyFastSimG4Tool.cpp
                         src/DummyFastSimG4Model.cpp
                         src/EcalFastSimG4Model.cpp
                         src/EcalFastSimG4Tool.cpp
                 LINK DetSimInterface
                      ${DD4hep_COMPONENT_LIBRARIES}
                      Gaudi::GaudiKernel
)

