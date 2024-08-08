//================================================================================
// Description — Short-Crystal End-Cap ECAL
//--------------------------------------------------------------------------------
// Author: Ji-Yuan CHEN (SJTU; jy_chen@sjtu.edu.cn)
//--------------------------------------------------------------------------------
//
// End-cap ECAL with short-bar crystals, 1×1×1 cm³ each.
// Each block is of size 35×35×30 cm³.  Some blocks on the edges are truncated.
// For placing more crystals, the gaps have been 'absorbed' in the blocks, and the actual size of each crystal is slightly smaller than (but still very close to) 1×1×1 cm³.
//
// The inner radius, number of modules in x or y direction, end-cap thickness, etc. are directly read from XML file.
// Dead material: ESR (wrapper), SiPM, PCB, Cu (cooling material).
//
// Structure in a layer: ESR → crystal → ESR → SiPM → PCB → Cu.
//
// Default layout: 12 blocks in x and y directions; for filling up spaces, add some smaller blocks.  For a schematic diagram, see Page 1 of  <https://indico.ihep.ac.cn/event/22010/contributions/153187/attachments/77666/96430/2024_0324_Calorimeter_Endcaps.pdf>  (the structure on the left; the numbers and detailed block structures have been modified).
//================================================================================

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/Shapes.h"
#include "DD4hep/DetType.h"
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/Segmentation.h"
#include <sstream>

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
using dd4hep::Tube;
using dd4hep::Box;
using dd4hep::SubtractionSolid;
using dd4hep::UnionSolid;
using dd4hep::Material;
using dd4hep::PlacedVolume;
using dd4hep::Position;
using dd4hep::Transform3D;
using dd4hep::RotationZ;
using dd4hep::_toString;
using dd4hep::RotationY;

static Ref_t create_detector(Detector& theDetector, xml_h e, SensitiveDetector sens)
{
    xml_det_t x_det = e;

    const string det_name = x_det.nameStr();
    const string det_type = x_det.typeStr();
    const int det_id = x_det.id();
    MYDEBUGVAL(det_name)
    MYDEBUGVAL(det_type)
    MYDEBUGVAL(det_id)

    // To prevent overlapping
    const double boundary_safety = theDetector.constant<double>("boundary_safety");

    // Global geometry
    const double r_in = theDetector.constant<double>("ecalendcap_inner_radius");
    const double r_out = theDetector.constant<double>("ecalendcap_outer_radius");
    const double block_xy_out = theDetector.constant<double>("ecalendcap_block_xy");
    const double block_z = theDetector.constant<double>("ecalendcap_thickness");
    const double pos_z = theDetector.constant<double>("ecalendcap_z");
    const int Nblock_xy = theDetector.constant<double>("Nblock_xy");    // In a sector
    const int Nsectors = theDetector.constant<double>("Nsectors");
    const double gap_narrow = theDetector.constant<double>("gap_narrow");
    const double gap_wide = theDetector.constant<double>("gap_wide");
    const double block_xy_in = block_xy_out - gap_narrow;
    const double angle = 2 * pi / Nsectors;

    // Filling the gaps
    const double block_rect_short_out = theDetector.constant<double>("ecalendcap_block_fill_rect_short");
    const double block_sq1_width = theDetector.constant<double>("ecalendcap_block_fill_sq1");
    const double block_sq2_width = theDetector.constant<double>("ecalendcap_block_fill_sq2");

    const double block_rect_short_in = block_rect_short_out - gap_narrow;
    const double block_sq1_width_in = block_sq1_width - gap_narrow;
    const double block_sq2_width_in = block_sq2_width - gap_narrow;

    const int Ncell_rect_short = theDetector.constant<double>("Ncell_rect_short");
    const int Ncell_sq1_xy = theDetector.constant<double>("Ncell_sq1_xy");
    const int Ncell_sq2_xy = theDetector.constant<double>("Ncell_sq2_xy");

    // Unit size
    const int Ncell_xy = theDetector.constant<int>("Ncell_xy");
    const double crystal_z = theDetector.constant<double>("crystal_z");
    const double cell_xy = block_xy_in / Ncell_xy;

    const double esr_thickness = theDetector.constant<double>("esr_thickness");    // Wrapper
    const double sipm_x = theDetector.constant<double>("sipm_x");
    const double sipm_y = theDetector.constant<double>("sipm_y");
    const double sipm_z = theDetector.constant<double>("sipm_z");
    const double pcb_thickness = theDetector.constant<double>("pcb_thickness");
    const double cu_thickness = theDetector.constant<double>("cu_thickness");    // Cooling material: Cu
    const double fibre_thickness = theDetector.constant<double>("fibre_thickness");    // Mechanical structure: carbon fibre

    const double crystal_xy = cell_xy - 4 * boundary_safety - 2 * esr_thickness - fibre_thickness;
    const double cell_z = crystal_z + 4 * boundary_safety + 2 * esr_thickness + fibre_thickness;

    const double layer_thickness = cell_z + sipm_z + pcb_thickness + cu_thickness + 4 * boundary_safety;
    const int Nlayers = (int) (block_z / layer_thickness);

    MYDEBUGVAL(layer_thickness)
    MYDEBUGVAL(Nlayers)

    // Materials
    Material mat_air(theDetector.material("Air"));
    Material mat_sensitive(theDetector.material( x_det.materialStr() ));
    Material mat_ESR(theDetector.material("G4_ESR"));
    Material mat_SiPM(theDetector.material("G4_Si"));
    Material mat_PCB(theDetector.material("PCB"));
    Material mat_Cu(theDetector.material("G4_Cu"));
    Material mat_CF(theDetector.material("CarbonFiber"));

    // Define the detector and mother volumes (world)
    DetElement ECAL(det_name, det_id);
    Volume motherVol = theDetector.pickMotherVolume(ECAL);

    // Create two tube-like envelopes to represent the end-cap volumes
    Tube envelope_tube(0, r_out, 0.5 * block_z + boundary_safety);
    Box envelope_box(r_in, r_in, block_z);
    SubtractionSolid envelope_side(envelope_tube, envelope_box, Position(0, 0, 0));
    UnionSolid envelope(envelope_side, envelope_side, Position(0, 0, 2 * pos_z));
    Volume envelopeVol(det_name, envelope, mat_air);
    PlacedVolume envelopePlv = motherVol.placeVolume(envelopeVol, Position(0, 0, -pos_z));
    envelopePlv.addPhysVolID("system", det_id);
    envelopeVol.setVisAttributes(theDetector, "SeeThrough");
    ECAL.setPlacement(envelopePlv);
    DetElement blockdet(ECAL, "box", det_id);

    // Sector
    Tube sector_tube(0, r_out, 0.5 * block_z + boundary_safety, 0, 0.5 * pi);
    SubtractionSolid sector(sector_tube, envelope_box, Position(0, 0, 0));
    Volume sector_vol("sector_vol", sector, mat_air);
    sector_vol.setVisAttributes(theDetector, "GreenVis");

    // Main block
    Box block(0.5 * block_xy_out, 0.5 * block_xy_out, 0.5 * block_z + boundary_safety);
    Volume block_vol("block_vol", block, mat_air);
    block_vol.setVisAttributes(theDetector, "GreenVis");

    Box block_cf_in(0.5 * block_xy_in, 0.5 * block_xy_in, 0.5 * block_z);
    Volume block_cf("block_cf", SubtractionSolid(block, block_cf_in, Position(0, 0, 0)), mat_CF);
    block_cf.setVisAttributes(theDetector, "GreenVis");

    // Rectangle block for filling the space
    Box block_rect(0.5 * block_xy_out, 0.5 * block_rect_short_out, 0.5 * block_z + boundary_safety);
    Volume block_rect_vol("block_rect_vol", block_rect, mat_air);
    block_rect_vol.setVisAttributes(theDetector, "GreenVis");

    Box block_cf_rect_in(0.5 * block_xy_in, 0.5 * block_rect_short_out - gap_narrow, 0.5 * block_z);
    Volume block_cf_rect("block_cf_rect", SubtractionSolid(block_rect, block_cf_rect_in, Position(0, 0, 0)), mat_CF);
    block_cf_rect.setVisAttributes(theDetector, "GreenVis");

    // Square blocks for filling the space
    // Square 1
    Box block_sq1(0.5 * block_sq1_width, 0.5 * block_sq1_width, 0.5 * block_z + boundary_safety);
    Volume block_sq1_vol("block_sq1_vol", block_sq1, mat_air);
    block_sq1_vol.setVisAttributes(theDetector, "GreenVis");

    Box block_cf_sq1_in(0.5 * block_sq1_width_in, 0.5 * block_sq1_width_in, 0.5 * block_z);
    Volume block_cf_sq1("block_cf_sq1", SubtractionSolid(block_sq1, block_cf_sq1_in, Position(0, 0, 0)), mat_CF);
    block_cf_sq1.setVisAttributes(theDetector, "GreenVis");

    // Square 2
    Box block_sq2(0.5 * block_sq2_width, 0.5 * block_sq2_width, 0.5 * block_z + boundary_safety);
    Volume block_sq2_vol("block_sq2_vol", block_sq2, mat_air);
    block_sq2_vol.setVisAttributes(theDetector, "GreenVis");

    Box block_cf_sq2_in(0.5 * block_sq2_width_in, 0.5 * block_sq2_width_in, 0.5 * block_z);
    Volume block_cf_sq2("block_cf_sq2", SubtractionSolid(block_sq2, block_cf_sq2_in, Position(0, 0, 0)), mat_CF);
    block_cf_sq2.setVisAttributes(theDetector, "GreenVis");

    // Crystal, SiPM, ESR, and carbon fibre
    Volume crystal("crystal", Box(0.5 * crystal_xy, 0.5 * crystal_xy, 0.5 * crystal_z), mat_sensitive);
    crystal.setVisAttributes(theDetector, "SeeThrough");
    crystal.setSensitiveDetector(sens);

    Volume sipm("SiPM", Box(0.5 * sipm_x, 0.5 * sipm_y, 0.5 * sipm_z), mat_SiPM);
    sipm.setVisAttributes(theDetector, "SeeThrough");

    Box esr_out(0.5 * crystal_xy + esr_thickness + boundary_safety, 0.5 * crystal_xy + esr_thickness + boundary_safety, 0.5 * crystal_z + esr_thickness + boundary_safety);
    Box esr_in(0.5 * crystal_xy + boundary_safety, 0.5 * crystal_xy + boundary_safety, 0.5 * crystal_z + boundary_safety);
    Volume esr("esr", SubtractionSolid(esr_out, esr_in, Position(0, 0, 0)), mat_ESR);
    esr.setVisAttributes(theDetector, "SeeThrough");

    Box cf_out(0.5 * cell_xy, 0.5 * cell_xy, 0.5 * cell_z);
    Box cf_in(0.5 * (cell_xy - fibre_thickness), 0.5 * (cell_xy - fibre_thickness), 0.5 * (cell_z - fibre_thickness));
    Volume cf("cf", SubtractionSolid(cf_out, cf_in, Position(0, 0, 0)), mat_CF);
    cf.setVisAttributes(theDetector, "SeeThrough");

    // Positions
    const double crystal_pos_z = -0.5 * layer_thickness + 0.5 * cell_z;
    const double esr_pos_z = crystal_pos_z;
    const double cf_pos_z = esr_pos_z;
    const double sipm_pos_z = cf_pos_z + 0.5 * cell_z + boundary_safety + 0.5 * sipm_z;
    const double pcb_pos_z = sipm_pos_z + 0.5 * sipm_z + boundary_safety + 0.5 * pcb_thickness;
    const double cu_pos_z = pcb_pos_z + 0.5 * (pcb_thickness + cu_thickness);

    // Loop for placing the units in a block
    // Normal block
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        Volume slice("slice", Box(0.5 * block_xy_in, 0.5 * block_xy_in, 0.5 * layer_thickness), mat_air);
        slice.setVisAttributes(theDetector, "SeeThrough");
        string slicename = "Slice_" + to_string(ilayer);
        DetElement sd(blockdet, slicename, det_id);

        Volume slice_pcb("slice_pcb", Box(0.5 * block_xy_in, 0.5 * block_xy_in, 0.5 * pcb_thickness), mat_PCB);
        slice_pcb.setVisAttributes(theDetector, "SeeThrough");

        Volume slice_cu("slice_cu", Box(0.5 * block_xy_in, 0.5 * block_xy_in, 0.5 * cu_thickness), mat_Cu);
        slice_cu.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit = slice.placeVolume(slice_pcb, Position(0, 0, pcb_pos_z));
        pcb_unit.addPhysVolID("layer", ilayer);

        PlacedVolume cu_unit = slice.placeVolume(slice_cu, Position(0, 0, cu_pos_z));
        cu_unit.addPhysVolID("layer", ilayer);

        for (int ix = 1; ix <= Ncell_xy; ++ix)
            for (int iy = 1; iy <= Ncell_xy; ++iy)
            {
                PlacedVolume crystal_unit = slice.placeVolume(crystal,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_xy_in + iy * cell_xy,
                                 crystal_pos_z));
                PlacedVolume esr_unit = slice.placeVolume(esr,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_xy_in + iy * cell_xy,
                                 esr_pos_z));
                PlacedVolume cf_unit = slice.placeVolume(cf,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_xy_in + iy * cell_xy,
                                 cf_pos_z));
                PlacedVolume sipm_unit = slice.placeVolume(sipm,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_xy_in + iy * cell_xy,
                                 sipm_pos_z));

                crystal_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                esr_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                cf_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                sipm_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);

                string crystal_name = "Crystal_" + to_string(ilayer) + "_" + to_string(ix) + "_" + to_string(iy);
                DetElement unit(sd, crystal_name, det_id);
                unit.setPlacement(crystal_unit);
            }

        PlacedVolume plv = block_vol.placeVolume(slice, Position(0, 0, (ilayer - 0.5) * layer_thickness - 0.5 * block_z + boundary_safety));
        plv.addPhysVolID("layer", ilayer);
        sd.setPlacement(plv);
    }

    // Rectangle block
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        Volume slice("slice", Box(0.5 * block_xy_in, 0.5 * block_rect_short_in, 0.5 * layer_thickness), mat_air);
        slice.setVisAttributes(theDetector, "SeeThrough");
        string slicename = "Rect_Slice_" + to_string(ilayer);
        DetElement sd(blockdet, slicename, det_id);

        Volume slice_pcb("slice_pcb", Box(0.5 * block_xy_in, 0.5 * block_rect_short_in, 0.5 * pcb_thickness), mat_PCB);
        slice_pcb.setVisAttributes(theDetector, "SeeThrough");

        Volume slice_cu("slice_cu", Box(0.5 * block_xy_in, 0.5 * block_rect_short_in, 0.5 * cu_thickness), mat_Cu);
        slice_cu.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit = slice.placeVolume(slice_pcb, Position(0, 0, pcb_pos_z));
        pcb_unit.addPhysVolID("layer", ilayer);

        PlacedVolume cu_unit = slice.placeVolume(slice_cu, Position(0, 0, cu_pos_z));
        cu_unit.addPhysVolID("layer", ilayer);

        for (int ix = 1; ix <= Ncell_xy; ++ix)
            for (int iy = 1; iy <= Ncell_rect_short; ++iy)
            {
                PlacedVolume crystal_unit = slice.placeVolume(crystal,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_rect_short_in + iy * cell_xy,
                                 crystal_pos_z));
                PlacedVolume esr_unit = slice.placeVolume(esr,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_rect_short_in + iy * cell_xy,
                                 esr_pos_z));
                PlacedVolume cf_unit = slice.placeVolume(cf,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_rect_short_in + iy * cell_xy,
                                 cf_pos_z));
                PlacedVolume sipm_unit = slice.placeVolume(sipm,
                        Position(-0.5 * block_xy_in + ix * cell_xy,
                                 -0.5 * block_rect_short_in + iy * cell_xy,
                                 sipm_pos_z));

                crystal_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                esr_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                cf_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                sipm_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);

                string crystal_name = "Crystal_" + to_string(ilayer) + "_" + to_string(ix) + "_" + to_string(iy);
                DetElement unit(sd, crystal_name, det_id);
                unit.setPlacement(crystal_unit);
            }

        PlacedVolume plv = block_rect_vol.placeVolume(slice, Position(0, 0, (ilayer - 0.5) * layer_thickness - 0.5 * block_z + boundary_safety));
        plv.addPhysVolID("layer", ilayer);
        sd.setPlacement(plv);
    }

    // Square block 1
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        Volume slice("slice", Box(0.5 * block_sq1_width, 0.5 * block_sq1_width, 0.5 * layer_thickness), mat_air);
        slice.setVisAttributes(theDetector, "SeeThrough");
        string slicename = "Sq1_Slice_" + to_string(ilayer);
        DetElement sd(blockdet, slicename, det_id);

        Volume slice_pcb("slice_pcb", Box(0.5 * block_sq1_width, 0.5 * block_sq1_width, 0.5 * pcb_thickness), mat_PCB);
        slice_pcb.setVisAttributes(theDetector, "SeeThrough");

        Volume slice_cu("slice_cu", Box(0.5 * block_sq1_width, 0.5 * block_sq1_width, 0.5 * cu_thickness), mat_Cu);
        slice_cu.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit = slice.placeVolume(slice_pcb, Position(0, 0, pcb_pos_z));
        pcb_unit.addPhysVolID("layer", ilayer);

        PlacedVolume cu_unit = slice.placeVolume(slice_cu, Position(0, 0, cu_pos_z));
        cu_unit.addPhysVolID("layer", ilayer);

        for (int ix = 1; ix <= Ncell_sq1_xy; ++ix)
            for (int iy = 1; iy <= Ncell_sq1_xy; ++iy)
            {
                PlacedVolume crystal_unit = slice.placeVolume(crystal,
                        Position(-0.5 * block_sq1_width + ix * cell_xy,
                                 -0.5 * block_sq1_width + iy * cell_xy,
                                 crystal_pos_z));
                PlacedVolume esr_unit = slice.placeVolume(esr,
                        Position(-0.5 * block_sq1_width + ix * cell_xy,
                                 -0.5 * block_sq1_width + iy * cell_xy,
                                 esr_pos_z));
                PlacedVolume cf_unit = slice.placeVolume(cf,
                        Position(-0.5 * block_sq1_width + ix * cell_xy,
                                 -0.5 * block_sq1_width + iy * cell_xy,
                                 cf_pos_z));
                PlacedVolume sipm_unit = slice.placeVolume(sipm,
                        Position(-0.5 * block_sq1_width + ix * cell_xy,
                                 -0.5 * block_sq1_width + iy * cell_xy,
                                 sipm_pos_z));

                crystal_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                esr_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                cf_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                sipm_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);

                string crystal_name = "Crystal_" + to_string(ilayer) + "_" + to_string(ix) + "_" + to_string(iy);
                DetElement unit(sd, crystal_name, det_id);
                unit.setPlacement(crystal_unit);
            }

        PlacedVolume plv = block_sq1_vol.placeVolume(slice, Position(0, 0, (ilayer - 0.5) * layer_thickness - 0.5 * block_z + boundary_safety));
        plv.addPhysVolID("layer", ilayer);
        sd.setPlacement(plv);
    }

    // Square block 2
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        Volume slice("slice", Box(0.5 * block_sq2_width, 0.5 * block_sq2_width, 0.5 * layer_thickness), mat_air);
        slice.setVisAttributes(theDetector, "SeeThrough");
        string slicename = "Sq2_Slice_" + to_string(ilayer);
        DetElement sd(blockdet, slicename, det_id);

        Volume slice_pcb("slice_pcb", Box(0.5 * block_sq2_width, 0.5 * block_sq2_width, 0.5 * pcb_thickness), mat_PCB);
        slice_pcb.setVisAttributes(theDetector, "SeeThrough");

        Volume slice_cu("slice_cu", Box(0.5 * block_sq2_width, 0.5 * block_sq2_width, 0.5 * cu_thickness), mat_Cu);
        slice_cu.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit = slice.placeVolume(slice_pcb, Position(0, 0, pcb_pos_z));
        pcb_unit.addPhysVolID("layer", ilayer);

        PlacedVolume cu_unit = slice.placeVolume(slice_cu, Position(0, 0, cu_pos_z));
        cu_unit.addPhysVolID("layer", ilayer);

        for (int ix = 1; ix <= Ncell_sq2_xy; ++ix)
            for (int iy = 1; iy <= Ncell_sq2_xy; ++iy)
            {
                PlacedVolume crystal_unit = slice.placeVolume(crystal,
                        Position(-0.5 * block_sq2_width + ix * cell_xy,
                                 -0.5 * block_sq2_width + iy * cell_xy,
                                 crystal_pos_z));
                PlacedVolume esr_unit = slice.placeVolume(esr,
                        Position(-0.5 * block_sq2_width + ix * cell_xy,
                                 -0.5 * block_sq2_width + iy * cell_xy,
                                 esr_pos_z));
                PlacedVolume cf_unit = slice.placeVolume(cf,
                        Position(-0.5 * block_sq2_width + ix * cell_xy,
                                 -0.5 * block_sq2_width + iy * cell_xy,
                                 cf_pos_z));
                PlacedVolume sipm_unit = slice.placeVolume(sipm,
                        Position(-0.5 * block_sq2_width + ix * cell_xy,
                                 -0.5 * block_sq2_width + iy * cell_xy,
                                 sipm_pos_z));

                crystal_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                esr_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                cf_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);
                sipm_unit.addPhysVolID("layer", ilayer).addPhysVolID("x", ix).addPhysVolID("y", iy);

                string crystal_name = "Crystal_" + to_string(ilayer) + "_" + to_string(ix) + "_" + to_string(iy);
                DetElement unit(sd, crystal_name, det_id);
                unit.setPlacement(crystal_unit);
            }

        PlacedVolume plv = block_sq2_vol.placeVolume(slice, Position(0, 0, (ilayer - 0.5) * layer_thickness - 0.5 * block_z + boundary_safety));
        plv.addPhysVolID("layer", ilayer);
        sd.setPlacement(plv);
    }

    int istave = 0;

    // Loop for placing the blocks in a module
    // This session is not flexible enough...
    for (int ix = 1; ix <= Nblock_xy; ++ix)
        for (int iy = 1; iy <= Nblock_xy; ++iy)
        {
            const double distance = sqrt(pow((ix - 0.5) * block_xy_out, 2) + pow((iy - 0.5) * block_xy_out, 2));
            if (distance < r_in || distance > r_out)
                continue;

            ++istave;

            PlacedVolume plv_block_cf;
            PlacedVolume plv_block;

            if (ix == 2 && iy == Nblock_xy)
            {
                plv_block_cf = block_rect_vol.placeVolume(block_cf_rect, Position(0, 0, 0));
                plv_block = sector_vol.placeVolume(block_rect_vol,
                        Position(0.5 * gap_wide - gap_narrow + (ix - 0.5) * block_xy_out,
                                 0.5 * gap_wide - gap_narrow + (iy - 1) * block_xy_out + 0.5 * block_rect_short_out,
                                 0));
            }
            else if ((ix == 3 && iy == Nblock_xy) || (ix == Nblock_xy && iy == 3))
            {
                plv_block_cf = block_sq1_vol.placeVolume(block_cf_sq1, Position(0, 0, 0));
                plv_block = sector_vol.placeVolume(block_sq1_vol,
                        Position(0.5 * gap_wide - gap_narrow + (ix - 1) * block_xy_out + 0.5 * block_sq1_width,
                                 0.5 * gap_wide - gap_narrow + (iy - 1) * block_xy_out + 0.5 * block_sq1_width,
                                 0));
            }
            else if ((ix == 4 && iy == 5) || (ix == 5 && iy == 4))
            {
                plv_block_cf = block_sq2_vol.placeVolume(block_cf_sq2, Position(0, 0, 0));
                plv_block = sector_vol.placeVolume(block_sq2_vol,
                        Position(0.5 * gap_wide - gap_narrow + (ix - 1) * block_xy_out + 0.5 * block_sq2_width,
                                 0.5 * gap_wide - gap_narrow + (iy - 1) * block_xy_out + 0.5 * block_sq2_width,
                                 0));
            }
            else if (ix == Nblock_xy && iy == 2)
            {
                plv_block_cf = block_rect_vol.placeVolume(block_cf_rect, Position(0, 0, 0));
                Transform3D transform(RotationZ(0.5 * pi),
                        Position(0.5 * gap_wide - gap_narrow + (ix - 1) * block_xy_out + 0.5 * block_rect_short_out,
                                 0.5 * gap_wide - gap_narrow + (iy - 0.5) * block_xy_out,
                                 0));
                plv_block = sector_vol.placeVolume(block_rect_vol, transform);
            }
            else
            {
                plv_block_cf = block_vol.placeVolume(block_cf, Position(0, 0, 0));
                plv_block = sector_vol.placeVolume(block_vol,
                        Position(0.5 * gap_wide - gap_narrow + (ix - 0.5) * block_xy_out,
                                 0.5 * gap_wide - gap_narrow + (iy - 0.5) * block_xy_out,
                                 0));
            }
            plv_block_cf.addPhysVolID("stave", istave);
            plv_block.addPhysVolID("stave", istave);
            DetElement sd(blockdet, _toString(istave, "block_%3d"), det_id);
            sd.setPlacement(plv_block);
        }

    MYDEBUGVAL(istave)

    // Loop for placing the modules
    for (int i = 0; i < Nsectors; ++i)
    {
        const double s_rot = i * angle;
        Transform3D transform_neg(RotationZ(s_rot) * RotationY(pi), Position(0, 0, 0));
        Transform3D transform_pos(RotationZ(s_rot + angle), Position(0, 0, 2 * pos_z));
        PlacedVolume plv_neg = envelopeVol.placeVolume(sector_vol, transform_neg);
        PlacedVolume plv_pos = envelopeVol.placeVolume(sector_vol, transform_pos);
        plv_neg.addPhysVolID("module", i);
        plv_pos.addPhysVolID("module", i + Nsectors);
        DetElement sd_neg(ECAL, _toString(i, "sector%3d"), det_id);
        DetElement sd_pos(ECAL, _toString(i + Nsectors, "sector%3d"), det_id);
        sd_neg.setPlacement(plv_neg);
        sd_pos.setPlacement(plv_pos);
    }

    sens.setType("calorimeter");

    MYDEBUG("create_detector FINISHED.")
    return ECAL;
}

DECLARE_DETELEMENT(CRDEcalEndcap_Short_v01, create_detector)
