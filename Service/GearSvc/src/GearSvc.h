#ifndef GEAR_SVC_H
#define GEAR_SVC_H

#include "GearSvc/IGearSvc.h"
#include <GaudiKernel/Service.h>
#include "DD4hep/Detector.h"
#include "DDRec/Vector3D.h"
#include "gearimpl/SimpleMaterialImpl.h"
class TGeoNode;

class GearSvc : public extends<Service, IGearSvc>
{
    public:
        GearSvc(const std::string& name, ISvcLocator* svc);
        virtual ~GearSvc();

        gear::GearMgr* getGearMgr() override;

        StatusCode initialize() override;
        StatusCode finalize() override;

    private:
	StatusCode convertBeamPipe(dd4hep::DetElement& pipe);
	StatusCode convertVXD(dd4hep::DetElement& vxd);
	StatusCode convertStitching(dd4hep::DetElement& vtx);
	StatusCode convertComposite(dd4hep::DetElement& vtx);
	StatusCode convertSIT(dd4hep::DetElement& sit);
	StatusCode convertTPC(dd4hep::DetElement& tpc);
	StatusCode convertDC (dd4hep::DetElement& dc);
	StatusCode convertSET(dd4hep::DetElement& set);
	StatusCode convertFTD(dd4hep::DetElement& ftd);
	StatusCode convertETD(dd4hep::DetElement& etd);
	StatusCode convertCal(dd4hep::DetElement& cal);
	TGeoNode* FindNode(TGeoNode* mother, char* name);
	gear::SimpleMaterialImpl* CreateGearMaterial(const dd4hep::rec::Vector3D& a, const dd4hep::rec::Vector3D& b, const std::string name);

        Gaudi::Property<std::string> m_gearFile{this, "GearXMLFile", ""};
	Gaudi::Property<std::string> m_outputFile{this, "GearOutput", ""};
        Gaudi::Property<float>       m_field{this, "MagneticField", 0};

        gear::GearMgr* m_gearMgr;
};

#endif
