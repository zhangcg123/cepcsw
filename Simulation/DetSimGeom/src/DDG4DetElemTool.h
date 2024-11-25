#ifndef DDG4DetElemTool_h
#define DDG4DetElemTool_h

#include "GaudiKernel/AlgTool.h"
#include <Gaudi/Property.h>
#include <GaudiKernel/ToolHandle.h>

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "DetInterface/IGeomSvc.h"
#include "DetSimInterface/IDetElemTool.h"
#include "DetSimInterface/ISensDetTool.h"


class DDG4DetElemTool: public extends<AlgTool, IDetElemTool> {

public:
    using extends::extends;

    G4LogicalVolume* getLV() override;
    void ConstructSDandField() override;

    StatusCode initialize() override;
    StatusCode finalize() override;

private:
    Gaudi::Property<double> m_x{this, "X", 30.*m};
    Gaudi::Property<double> m_y{this, "Y", 30.*m};
    Gaudi::Property<double> m_z{this, "Z", 30.*m};
    Gaudi::Property<double> m_DeltaIntersection{this, "DeltaIntersection", 1.0e-8*mm};
    Gaudi::Property<double> m_DeltaOneStep{this, "DeltaOneStep",1.0e-7 * mm};
    Gaudi::Property<double> m_MinimumEpsilonStep{this, "MinimumEpsilonStep",1.0e-10 * mm};
    Gaudi::Property<double> m_MaximumEpsilonStep{this, "MaximumEpsilonStep",1.0e-8 * mm};

    // DD4hep XML compact file path
    Gaudi::Property<std::string> m_dd4hep_xmls{this, "detxml"};

    Gaudi::Property<bool> m_SD_enabled{this, "SDenabled", true};

    SmartIF<IGeomSvc> m_geosvc;
    ToolHandle<ISensDetTool> m_calo_sdtool;
    ToolHandle<ISensDetTool> m_driftchamber_sdtool;
    ToolHandle<ISensDetTool> m_tpc_sdtool;
    ToolHandle<ISensDetTool> m_tracker_sdtool;
    ToolHandle<ISensDetTool> m_muonbarrel_sdtool;
    ToolHandle<ISensDetTool> m_muonendcap_sdtool;
};

#endif
