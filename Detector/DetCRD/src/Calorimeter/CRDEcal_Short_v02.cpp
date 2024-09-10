//================================================================================
// Description — 32-Sided Polygon Barrel ECAL
//--------------------------------------------------------------------------------
// Author: Ji-Yuan CHEN (SJTU; jy_chen@sjtu.edu.cn)
//--------------------------------------------------------------------------------
//
// ECAL with short crystals.  Not crossed-bar structure.
// 32 modules cover a phi range of 2pi.
//
// ‘Positive’ and ‘negative’ trapezia:
//       ______________             __________
//       \            /            /          \        ↑ R (pointing outwards)
//        \__________/            /____________\       │
//          Positive                 Negative        z ⊗ ——→ φ
// (Inner base < outer base)     (Inner > outer)
//
// The inner radius, module thickness, barrel length, etc. are directly read from XML file.
// Dead material: ESR (wrapper), carbon fibre (mechanical structure), SiPM, PCB, Cu (cooling material).
//
// Layout: 32 modules in phi direction
//         → 15 blocks in z direction;
//           → 24 (?) layers in R direction.
// For a schematic diagram, see  <https://indico.ihep.ac.cn/event/22331/contributions/155518/attachments/77650/96399/Polygon32-CrossSection.pdf>  (the numbers have been modified).
//================================================================================

#include "DD4hep/DetFactoryHelper.h"
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/Segmentation.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;

#define MYDEBUG(x) cout << __FILE__ << ":" << __LINE__ << ": " << x << endl;
#define MYDEBUGVAL(x) cout << __FILE__ << ":" << __LINE__ << ": " << #x << ": " << x << endl;

using dd4hep::Ref_t;
using dd4hep::Detector;
using dd4hep::SensitiveDetector;
using dd4hep::pi;
using dd4hep::degree;
using dd4hep::DetElement;
using dd4hep::Volume;
using dd4hep::PolyhedraRegular;
using dd4hep::Material;
using dd4hep::PlacedVolume;
using dd4hep::Position;
using dd4hep::Trapezoid;
using dd4hep::Box;
using dd4hep::SubtractionSolid;
using dd4hep::Transform3D;
using dd4hep::RotationZ;
using dd4hep::RotationX;
using dd4hep::_toString;

static Ref_t create_detector(Detector& theDetector, xml_h e, SensitiveDetector sens)
{
    xml_det_t x_det = e;

    string det_name = x_det.nameStr();
    string det_type = x_det.typeStr();
    const int detid = x_det.id();
    MYDEBUGVAL(det_name)
    MYDEBUGVAL(det_type)
    MYDEBUGVAL(detid)

    // To prevent overlapping
    const double boundary_safety_barrel = theDetector.constant<double>("boundary_safety_barrel");

    // Global geometry
    const double r_in = theDetector.constant<double>("ecalbarrel_inner_radius");
    const double r_out_max = theDetector.constant<double>("ecalbarrel_outer_radius");
    const double h0 = theDetector.constant<double>("ecalbarrel_thickness");
    const double Z0 = theDetector.constant<double>("ecalbarrel_zlength");
    const int Nmodule = theDetector.constant<int>("Nmodule");    // 32 modules
    const int Nblock_z = theDetector.constant<int>("Nblock_z");    // Block number in z direction
    const double rotation_angle = theDetector.constant<double>("module_rotation");    // Angle between the leg and the radius through its mid-point.
    const double angle = 2 * pi / Nmodule;

    // Unit size
    const double crystal_r = theDetector.constant<double>("crystal_r");
    const double crystal_phi = theDetector.constant<double>("crystal_phi");
    const double crystal_z_barrel = theDetector.constant<double>("crystal_z_barrel");
    const double block_z = Z0 / Nblock_z;    // Length of a block in z direction

    MYDEBUGVAL(block_z)

    const double esr_thickness_barrel = theDetector.constant<double>("esr_thickness_barrel");    // Wrapper
    const double sipm_r = theDetector.constant<double>("sipm_r");
    const double sipm_phi = theDetector.constant<double>("sipm_phi");
    const double sipm_z_barrel = theDetector.constant<double>("sipm_z_barrel");
    const double pcb_thickness_barrel = theDetector.constant<double>("pcb_thickness_barrel");
    const double cu_thickness_barrel = theDetector.constant<double>("cu_thickness_barrel");    // Cooling material: Cu
    const double fibre_thickness_barrel = theDetector.constant<double>("fibre_thickness_barrel");    // Mechanical structure: carbon fibre
    const double collection_width = theDetector.constant<double>("collection_width");
    const double collection_thickness = theDetector.constant<double>("collection_thickness");

    const double cell_r = crystal_r + 4 * boundary_safety_barrel + 2 * esr_thickness_barrel + fibre_thickness_barrel;
    const double cell_phi = crystal_phi + 4 * boundary_safety_barrel + 2 * esr_thickness_barrel + fibre_thickness_barrel;
    const double cell_z = crystal_z_barrel + 4 * boundary_safety_barrel + 2 * esr_thickness_barrel + fibre_thickness_barrel;

    const double layer_thickness = cell_r + sipm_r + pcb_thickness_barrel + cu_thickness_barrel + 4 * boundary_safety_barrel;
    MYDEBUGVAL(layer_thickness)

    // Adjustments for the positive trapezia
    const double height_leg_angle_pos = rotation_angle + 0.5 * angle;    // Angle between <height> and <leg> of the positive trapezia
    const double height_leg_deviation_pos = 0.5 * h0 * tan(height_leg_angle_pos);    // Deviation because of the angle between <height> and <leg> (about the mid-point of the leg)
    const double height_rad_deviation = 0.5 * h0 * tan(0.5 * angle);    // Deviation because of the angle between <height> and <radius through the mid-point of the leg> (about the mid-point of the leg)
    const double leg_rad_deviation_pos = height_leg_deviation_pos - height_rad_deviation;    // Deviation because of the angle between <leg> and <radius through the mid-point of the leg> (about the mid-point of the leg). Or equivalently, deviation of the inner base due to rotation.

    // Adjustments for the negative trapezia
    const double height_leg_angle_neg = rotation_angle - 0.5 * angle;    // Angle between <height> and <leg> of the negative trapezia
    const double height_leg_deviation_neg = 0.5 * h0 * tan(height_leg_angle_neg);    // Deviation because of the angle between <height> and <leg> (about the mid-point of the leg)
    const double leg_rad_deviation_neg = height_leg_deviation_neg + height_rad_deviation;    // Deviation because of the angle between <leg> and <radius through the mid-point of the leg> (about the mid-point of the leg)

    const double dim_in_pos = r_in * tan(0.5 * angle) - leg_rad_deviation_pos;
    const double dim_out_pos = dim_in_pos + h0 * tan(height_leg_angle_pos);
    const double dim_in_neg = r_in * tan(0.5 * angle) + leg_rad_deviation_neg;
    const double dim_out_neg = dim_in_neg - h0 * tan(height_leg_angle_neg);
    const double dim_y = 0.5 * Z0;
    const double dim_z = 0.5 * h0;
    const double r0 = r_in + 0.5 * h0;    // Rotation radius
    const double r_out = sqrt(pow(r_in + h0, 2) + pow(dim_out_pos, 2));

    if (r_out > r_out_max)
        throw "Outer radius of barrel ECAL exceeds assigned maximum value!";

    MYDEBUGVAL(dim_in_pos)
    MYDEBUGVAL(dim_out_pos)
    MYDEBUGVAL(dim_in_neg)
    MYDEBUGVAL(dim_out_neg)

    const int Nlayers = ((int) (h0 / layer_thickness) * layer_thickness + collection_thickness <= h0) ? (int) (h0 / layer_thickness) : (int) (h0 / layer_thickness) - 1;
    const int Ncell_z = (int) (block_z / cell_z);    // Crystal number along z direction in each block
    int Ncell_phi_pos;    // Crystal number along phi direction in each positive trapezium block
    int Ncell_phi_neg;    // Crystal number along phi direction in each negative trapezium block

    MYDEBUGVAL(Nlayers)
    MYDEBUGVAL(Ncell_z)

    // Materials
    Material mat_air(theDetector.material("Air"));
    Material mat_sensitive(theDetector.material( x_det.materialStr() ));
    Material mat_ESR(theDetector.material("G4_ESR"));
    Material mat_SiPM(theDetector.material("G4_Si"));
    Material mat_PCB(theDetector.material("PCB"));
    Material mat_Cu(theDetector.material("G4_Cu"));
    Material mat_CF(theDetector.material("CarbonFiber"));

    // Define the detector and mother volumes (world)
    DetElement ECAL(det_name, detid);
    Volume motherVol = theDetector.pickMotherVolume(ECAL);

    // Create a tube-like envelope to represent the whole detector volume
    PolyhedraRegular envelope(Nmodule, 0.5 * angle, r_in, r_out, Z0);
    Volume envelopeVol(det_name, envelope, mat_air);
    PlacedVolume envelopePlv = motherVol.placeVolume(envelopeVol, Position(0, 0, 0));
    envelopePlv.addPhysVolID("system", detid);
    envelopeVol.setVisAttributes(theDetector, "SeeThrough");
    ECAL.setPlacement(envelopePlv);
    DetElement blockdet(ECAL, "module", detid);

    // Positive trapezium module
    Trapezoid module_pos(dim_in_pos, dim_out_pos, dim_y, dim_y, dim_z);
    Volume module_pos_vol("module_pos_vol", module_pos, mat_air);
    module_pos_vol.setVisAttributes(theDetector, "CyanVis");

    // Negative trapezium module
    Trapezoid module_neg(dim_in_neg, dim_out_neg, dim_y, dim_y, dim_z);
    Volume module_neg_vol("module_neg_vol", module_neg, mat_air);
    module_neg_vol.setVisAttributes(theDetector, "CyanVis");

    // Positive block
    Trapezoid block_pos(dim_in_pos, dim_out_pos, 0.5 * block_z, 0.5 * block_z, dim_z);
    Volume block_pos_vol("block_pos_vol", block_pos, mat_CF);
    block_pos_vol.setVisAttributes(theDetector, "CyanVis");

    // Negative block
    Trapezoid block_neg(dim_in_neg, dim_out_neg, 0.5 * block_z, 0.5 * block_z, dim_z);
    Volume block_neg_vol("block_neg_vol", block_neg, mat_CF);
    block_neg_vol.setVisAttributes(theDetector, "CyanVis");

    // Crystal, ESR, SiPM, carbon fibre and collection board
    Volume crystal("crystal", Box(0.5 * crystal_phi, 0.5 * crystal_z_barrel, 0.5 * crystal_r), mat_sensitive);    // Order: phi → z → R.
    crystal.setVisAttributes(theDetector, "SeeThrough");
    crystal.setSensitiveDetector(sens);

    Box esr_out(0.5 * crystal_phi + esr_thickness_barrel + boundary_safety_barrel, 0.5 * crystal_z_barrel + esr_thickness_barrel + boundary_safety_barrel, 0.5 * crystal_r + esr_thickness_barrel + boundary_safety_barrel);
    Box esr_in(0.5 * crystal_phi + boundary_safety_barrel, 0.5 * crystal_z_barrel + boundary_safety_barrel, 0.5 * crystal_r + boundary_safety_barrel);
    Volume esr("esr", SubtractionSolid(esr_out, esr_in, Position(0, 0, 0)), mat_ESR);
    esr.setVisAttributes(theDetector, "SeeThrough");

    Volume SiPM("SiPM", Box(0.5 * sipm_phi, 0.5 * sipm_z_barrel, 0.5 * sipm_r), mat_SiPM);
    SiPM.setVisAttributes(theDetector, "SeeThrough");

    Box cf_out(0.5 * cell_phi, 0.5 * cell_z, 0.5 * cell_r);
    Box cf_in(0.5 * (cell_phi - fibre_thickness_barrel), 0.5 * (cell_z - fibre_thickness_barrel), 0.5 * (cell_r - fibre_thickness_barrel));
    Volume cf("cf", SubtractionSolid(cf_out, cf_in, Position(0, 0, 0)), mat_CF);
    cf.setVisAttributes(theDetector, "SeeThrough");

    Volume collection("collection", Box(0.5 * collection_width, 0.5 * collection_thickness, 0.5 * collection_width), mat_PCB);
    collection.setVisAttributes(theDetector, "SeeThrough");

    // Positions
    const double crystal_pos_r = -0.5 * layer_thickness + 0.5 * cell_r;
    const double esr_pos_r = crystal_pos_r;
    const double cf_pos_r = esr_pos_r;
    const double sipm_pos_r = cf_pos_r + 0.5 * cell_r + boundary_safety_barrel + 0.5 * sipm_r;
    const double pcb_pos_r = sipm_pos_r + 0.5 * sipm_r + boundary_safety_barrel + 0.5 * pcb_thickness_barrel;
    const double cu_pos_r = pcb_pos_r + 0.5 * (pcb_thickness_barrel + cu_thickness_barrel);

    // Loop for placing the crystals in one positive trapezium block
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        const double layer_length_pos = dim_in_pos + (ilayer - 1) * layer_thickness * tan(height_leg_angle_pos);
        Ncell_phi_pos = (int) (2 * layer_length_pos / cell_phi);
        const double layer_phi_pos = Ncell_phi_pos * cell_phi;

        Volume slice_pos("slice_pos", Box(0.5 * layer_phi_pos, 0.5 * block_z, 0.5 * layer_thickness), mat_air);
        slice_pos.setVisAttributes(theDetector, "SeeThrough");
        string slicename_pos = "Slice_pos_" + to_string(ilayer);
        DetElement sd_pos(blockdet, slicename_pos, detid);

        Volume slice_pcb_pos("slice_pcb_pos", Box(0.5 * layer_phi_pos, 0.5 * block_z, 0.5 * pcb_thickness_barrel), mat_PCB);
        slice_pcb_pos.setVisAttributes(theDetector, "SeeThrough");

        Volume slice_cu_pos("slice_cu_pos", Box(0.5 * layer_phi_pos, 0.5 * block_z, 0.5 * cu_thickness_barrel), mat_Cu);
        slice_cu_pos.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit_pos = slice_pos.placeVolume(slice_pcb_pos, Position(0, 0, pcb_pos_r));
        pcb_unit_pos.addPhysVolID("layer", ilayer);

        PlacedVolume cu_unit = slice_pos.placeVolume(slice_cu_pos, Position(0, 0, cu_pos_r));
        cu_unit.addPhysVolID("layer", ilayer);

        for (int iz = 1; iz <= Ncell_z; ++iz)
            for (int iphi = 1; iphi <= Ncell_phi_pos; ++iphi)
            {
                PlacedVolume crystal_unit_pos = slice_pos.placeVolume(crystal,
                        Position((-0.5 * (Ncell_phi_pos + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 crystal_pos_r));
                PlacedVolume esr_unit_pos = slice_pos.placeVolume(esr,
                        Position((-0.5 * (Ncell_phi_pos + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 esr_pos_r));
                PlacedVolume cf_unit_pos = slice_pos.placeVolume(cf,
                        Position((-0.5 * (Ncell_phi_pos + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 cf_pos_r));
                PlacedVolume sipm_unit_pos = slice_pos.placeVolume(SiPM,
                        Position((-0.5 * (Ncell_phi_pos + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 sipm_pos_r));

                crystal_unit_pos.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);
                esr_unit_pos.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);
                cf_unit_pos.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);
                sipm_unit_pos.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);

                string crystal_name_pos = "Crystal_pos_" + to_string(ilayer) + "_" + to_string(iphi) + "_" + to_string(iz);
                DetElement unit_pos(sd_pos, crystal_name_pos, detid);
                unit_pos.setPlacement(crystal_unit_pos);
            }

        PlacedVolume plv = block_pos_vol.placeVolume(slice_pos, Position(0, 0, (ilayer - 0.5) * layer_thickness - dim_z));
        plv.addPhysVolID("layer", ilayer);
        sd_pos.setPlacement(plv);
    }

    // Loop for placing the crystals in one negative trapezium block
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        const double layer_length_neg = dim_in_neg - ilayer * layer_thickness * tan(height_leg_angle_neg);
        Ncell_phi_neg = (int) (2 * layer_length_neg / cell_phi);
        const double layer_phi_neg = Ncell_phi_neg * cell_phi;

        Volume slice_neg("slice_neg", Box(0.5 * layer_phi_neg, 0.5 * block_z, 0.5 * layer_thickness), mat_air);
        slice_neg.setVisAttributes(theDetector, "SeeThrough");
        string slicename_neg = "Slice_neg_" + to_string(ilayer);
        DetElement sd_neg(blockdet, slicename_neg, detid);

        Volume slice_pcb_neg("slice_pcb_neg", Box(0.5 * layer_phi_neg, 0.5 * block_z, 0.5 * pcb_thickness_barrel), mat_PCB);
        slice_pcb_neg.setVisAttributes(theDetector, "SeeThrough");

        Volume slice_cu_neg("slice_cu_neg", Box(0.5 * layer_phi_neg, 0.5 * block_z, 0.5 * cu_thickness_barrel), mat_Cu);
        slice_cu_neg.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit_neg = slice_neg.placeVolume(slice_pcb_neg, Position(0, 0, pcb_pos_r));
        pcb_unit_neg.addPhysVolID("layer", ilayer);

        PlacedVolume cu_unit = slice_neg.placeVolume(slice_cu_neg, Position(0, 0, cu_pos_r));
        cu_unit.addPhysVolID("layer", ilayer);

        for (int iz = 1; iz <= Ncell_z; ++iz)
            for (int iphi = 1; iphi <= Ncell_phi_neg; ++iphi)
            {
                PlacedVolume crystal_unit_neg = slice_neg.placeVolume(crystal,
                        Position((-0.5 * (Ncell_phi_neg + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 crystal_pos_r));
                PlacedVolume esr_unit_neg = slice_neg.placeVolume(esr,
                        Position((-0.5 * (Ncell_phi_neg + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 esr_pos_r));
                PlacedVolume cf_unit_neg = slice_neg.placeVolume(cf,
                        Position((-0.5 * (Ncell_phi_neg + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 cf_pos_r));
                PlacedVolume sipm_unit_neg = slice_neg.placeVolume(SiPM,
                        Position((-0.5 * (Ncell_phi_neg + 1) + iphi) * cell_phi,
                                 (-0.5 * (Ncell_z + 1) + iz) * cell_z,
                                 sipm_pos_r));

                crystal_unit_neg.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);
                esr_unit_neg.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);
                cf_unit_neg.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);
                sipm_unit_neg.addPhysVolID("layer", ilayer).addPhysVolID("phi", iphi).addPhysVolID("z", iz);

                string crystal_name_neg = "Crystal_neg_" + to_string(ilayer) + "_" + to_string(iphi) + "_" + to_string(iz);
                DetElement unit_neg(sd_neg, crystal_name_neg, detid);
                unit_neg.setPlacement(crystal_unit_neg);
            }

        PlacedVolume plv = block_neg_vol.placeVolume(slice_neg, Position(0, 0, (ilayer - 0.5) * layer_thickness - dim_z));
        plv.addPhysVolID("layer", ilayer);
        sd_neg.setPlacement(plv);
    }

    // Loop for placing the blocks in a module
    for (int iz = 1; iz <= Nblock_z; ++iz)
    {
        PlacedVolume plv_collection_pos = block_pos_vol.placeVolume(collection, Position(0, 0, -0.5 * dim_z + Nlayers * layer_thickness + collection_thickness));
        plv_collection_pos.addPhysVolID("stave", iz);

        PlacedVolume plv_pos = module_pos_vol.placeVolume(block_pos_vol, Position(0, dim_y - (iz - 0.5) * block_z, 0));
        plv_pos.addPhysVolID("stave", iz);
        DetElement sd_pos(blockdet, _toString(iz, "block_pos_%3d"), detid);
        sd_pos.setPlacement(plv_pos);

        PlacedVolume plv_collection_neg = block_neg_vol.placeVolume(collection, Position(0, 0, -0.5 * dim_z + Nlayers * layer_thickness + collection_thickness));
        plv_collection_neg.addPhysVolID("stave", iz);

        PlacedVolume plv_neg = module_neg_vol.placeVolume(block_neg_vol, Position(0, dim_y - (iz - 0.5) * block_z, 0));
        plv_neg.addPhysVolID("stave", iz);
        DetElement sd_neg(blockdet, _toString(iz, "block_neg_%3d"), detid);
        sd_neg.setPlacement(plv_neg);
    }

    // Loop for placing the modules
    for (int i = 0; i < Nmodule; ++i)
    {
        const double m_rot = i * angle;
        const double posx = -r0 * sin(m_rot);
        const double posy = r0 * cos(m_rot);

        Transform3D transform(RotationZ(m_rot) * RotationX(-0.5 * pi), Position(posx, posy, 0.0));
        PlacedVolume plv = (i % 2 == 0) ? envelopeVol.placeVolume(module_pos_vol, transform) : envelopeVol.placeVolume(module_neg_vol, transform);
        plv.addPhysVolID("module", i);

        DetElement sd(ECAL, _toString(i, "module%3d"), detid);
        sd.setPlacement(plv);
    }

    sens.setType("calorimeter");

    MYDEBUG("create_detector FINISHED.");
    return ECAL;
}

DECLARE_DETELEMENT(CRDEcalBarrel_Short_v02, create_detector)
