//================================================================================
// Description — End-Cap AHCAL
//--------------------------------------------------------------------------------
// Author: Ji-Yuan CHEN (SJTU; jy_chen@sjtu.edu.cn)
//--------------------------------------------------------------------------------
//
// End-cap AHCAL with 40×40×3 mm³ scintillator tiles.
// Adding the dead materials (cassette, PCB and electronic components, ESR wrapper and steel absorber) and including the air gaps, the size of each cell becomes 40.3×40.3×27.2 mm³.
//
// The inner radius, end-cap thickness, unit size, etc. are directly read from XML file.
//
// Structure in a layer: Cassette → ESR → scintillator → ESR → PCB (including electronic components) → cassette → steel absorber.
//
// Default layout: The absorber is designed as a whole with some space left for the sensitive units (the cross-section is not a polygon); steel frame on the edges of each module; 16 modules (called 'staves' in this program), 48 layers; 72 rows in r direction, with uniform granularity.  For a schematic diagram, see Page 3 of  <https://indico.ihep.ac.cn/event/22010/contributions/153187/attachments/77666/96430/2024_0324_Calorimeter_Endcaps.pdf>  (the design on the left; the numbers have been modified).
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
#include <cassert>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;

#define MYDEBUG(x) cout << __FILE__ << ":" << __LINE__ << ": " << x << endl;
#define MYDEBUGVAL(x) cout << __FILE__ << ":" << __LINE__ << ": "<< #x << ": " << x << endl;

using dd4hep::Ref_t;
using dd4hep::Detector;
using dd4hep::SensitiveDetector;
using dd4hep::pi;
using dd4hep::mm;
using dd4hep::Material;
using dd4hep::DetElement;
using dd4hep::Volume;
using dd4hep::PolyhedraRegular;
using dd4hep::Tube;
using dd4hep::UnionSolid;
using dd4hep::Trapezoid;
using dd4hep::Box;
using dd4hep::SubtractionSolid;
using dd4hep::PlacedVolume;
using dd4hep::Position;
using dd4hep::RotationX;
using dd4hep::_toString;
using dd4hep::Transform3D;
using dd4hep::RotationZ;
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
    const double r_in = theDetector.constant<double>("hcalendcap_inner_radius");
    const double r_out = theDetector.constant<double>("hcalendcap_outer_radius");
    const double module_thickness = theDetector.constant<double>("hcalendcap_thickness");
    const double pos_z = theDetector.constant<double>("hcalendcap_z");
    const int Nmodules = theDetector.constant<int>("Nmodules");
    const double angle = 2 * pi / Nmodules;

    // Unit size
    const double scintillator_xy = theDetector.constant<double>("scintillator_xy");
    const double scintillator_z = theDetector.constant<double>("scintillator_z");
    const double wrapped_scintillator_xy = theDetector.constant<double>("wrapped_scintillator_xy");
    const double wrapped_scintillator_z = theDetector.constant<double>("wrapped_scintillator_z");

    // Dead materials
    const double cassette_thickness = theDetector.constant<double>("cassette_thickness"); 
    const double esr_thickness = theDetector.constant<double>("esr_thickness");    // Wrapper
    [[maybe_unused]] const double sipm_xy = theDetector.constant<double>("sipm_xy");
    [[maybe_unused]] const double sipm_z = theDetector.constant<double>("sipm_z");
    const double pcb_thickness = theDetector.constant<double>("pcb_thickness"); 
    const double absorber_thickness = theDetector.constant<double>("absorber_thickness"); 
    const double air_gap_xy = 0.5 * (wrapped_scintillator_xy - scintillator_xy) - esr_thickness;
    [[maybe_unused]] const double air_gap_z = 0.5 * (wrapped_scintillator_z - scintillator_z) - esr_thickness;

    // Odd-shaped cells
    const int Nodd = theDetector.constant<int>("Nodd");
    const double short_elongation_1 = theDetector.constant<double>("short_elongation_1");
    const double short_elongation_2 = theDetector.constant<double>("short_elongation_2");
    const double short_elongation_3 = theDetector.constant<double>("short_elongation_3");
    const double short_elongation_4 = theDetector.constant<double>("short_elongation_4");
    const double short_elongation_5 = theDetector.constant<double>("short_elongation_5");

    const double elongation_angle_esr_out = wrapped_scintillator_xy * tan(0.5 * angle);    // Elongation of the outer edge of ESR: long side - short side
    const double elongation_angle_esr_in = (wrapped_scintillator_xy - esr_thickness / cos(0.5 * angle)) * tan(0.5 * angle);    // Elongation of the inner edge of ESR: long side - short side
    const double elongation_angle_sc = scintillator_xy * tan(0.5 * angle);    // Elongation of the scintillator: long side - short side
    const vector<double> short_elongation_esr_out = { short_elongation_1, short_elongation_2, short_elongation_3, short_elongation_4, short_elongation_5 };
    assert(Nodd == short_elongation_esr_out.size());

    vector<double> short_elongation_sc(short_elongation_esr_out.size()), long_elongation_sc(short_elongation_esr_out.size()), short_elongation_esr_in(short_elongation_esr_out.size()), long_elongation_esr_in(short_elongation_esr_out.size()), long_elongation_esr_out(short_elongation_esr_out.size());
    for (int i = 0; i < Nodd; ++i)
    {
        short_elongation_sc.at(i) = short_elongation_esr_out.at(i) - (air_gap_xy + esr_thickness) / cos(0.5 * angle);
        long_elongation_sc.at(i) = short_elongation_sc.at(i) + elongation_angle_sc;
        short_elongation_esr_in.at(i) = short_elongation_esr_out.at(i) - esr_thickness / cos(0.5 * angle);
        long_elongation_esr_in.at(i) = short_elongation_esr_in.at(i) + elongation_angle_esr_in;
        long_elongation_esr_out.at(i) = short_elongation_esr_out.at(i) + elongation_angle_esr_out;
    }

    // Mechanical structure
    const double inner_structure_thickness = theDetector.constant<double>("inner_structure_thickness");
    [[maybe_unused]] const double outer_structure_width = theDetector.constant<double>("outer_structure_width");
    [[maybe_unused]] const double outer_structure_thickness = theDetector.constant<double>("outer_structure_thickness");
    const double frame_thickness = theDetector.constant<double>("frame_thickness");

    const double layer_thickness = wrapped_scintillator_z + pcb_thickness + 2 * cassette_thickness + absorber_thickness + 5 * boundary_safety;
    const double non_absorber_thickness = wrapped_scintillator_z + pcb_thickness + 3 * boundary_safety;

    MYDEBUGVAL(layer_thickness)

    // Module size
    const double dim_in_frame = (r_in + inner_structure_thickness + frame_thickness) * tan(0.5 * angle);
    const double dim_out_frame = r_out * sin(0.5 * angle);
    const double dim_in = dim_in_frame - frame_thickness / cos(0.5 * angle);
    const double dim_out = dim_out_frame - frame_thickness / cos(0.5 * angle);
    const double height = r_out * cos(0.5 * angle) - r_in - inner_structure_thickness;
    const double r0 = r_in + inner_structure_thickness + 0.5 * height;    // Rotation radius for placing the modules

    const int Nrows = (int) (height / (wrapped_scintillator_xy + boundary_safety));
    const int Nlayers = (int) (module_thickness / layer_thickness);
    int Ncells_phi;

    MYDEBUGVAL(Nrows)
    MYDEBUGVAL(Nlayers)

    // Materials
    Material mat_air(theDetector.material("Air"));
    Material mat_PCB(theDetector.material("PCB"));
    [[maybe_unused]] Material mat_SiPM(theDetector.material("G4_Si"));
    Material mat_sensitive(theDetector.material( x_det.materialStr() ));
    Material mat_ESR(theDetector.material("G4_ESR"));
    Material mat_steel(theDetector.material("Steel235"));

    // The detector and mother volumes (world)
    DetElement AHCAL(det_name, det_id);
    Volume motherVol = theDetector.pickMotherVolume(AHCAL);

    // Create two tube-like envelopes to represent the end-cap volumes
//    PolyhedraRegular envelope_side(Nmodules, 0.5 * angle, r_in + inner_structure_thickness, r_out, module_thickness);
    Tube envelope_side(r_in, r_out, 0.5 * module_thickness);
    UnionSolid envelope(envelope_side, envelope_side, Position(0, 0, 2 * pos_z));
    Volume envelopeVol(det_name, envelope, mat_steel);
    PlacedVolume envelopePlv = motherVol.placeVolume(envelopeVol, Position(0, 0, -pos_z));
    envelopePlv.addPhysVolID("system", det_id);
    envelopeVol.setVisAttributes(theDetector, "SeeThrough");
    AHCAL.setPlacement(envelopePlv);
    DetElement stave_det(AHCAL, "sector", det_id);

    // Module (stave)
    Trapezoid stave(dim_in, dim_out, 0.5 * module_thickness, 0.5 * module_thickness, 0.5 * height);
    Volume stave_vol("stave_vol", stave, mat_steel);
    stave_vol.setVisAttributes(theDetector, "BlueVis");

    // Layer with only wrapped scintillators, electronic components and PCB
    Trapezoid layer(dim_in, dim_out, 0.5 * non_absorber_thickness, 0.5 * non_absorber_thickness, 0.5 * height);
    Volume layer_vol("layer_vol", layer, mat_steel);
    layer_vol.setVisAttributes(theDetector, "SeeThrough");

    // Scintillator, ESR, SiPM, PCB, cassette, and absorber
    Box normal_scintillator(0.5 * scintillator_xy, 0.5 * scintillator_z, 0.5 * scintillator_xy);
    Volume scintillator("scintillator", normal_scintillator, mat_sensitive);    // Order: phi → z → r
    scintillator.setVisAttributes(theDetector, "SeeThrough");
    scintillator.setSensitiveDetector(sens);

    [[maybe_unused]] Volume sipm("SiPM", Box(0.5 * sipm_xy, 0.5 * sipm_xy, 0.5 * sipm_z), mat_SiPM);
    sipm.setVisAttributes(theDetector, "CyanVis");

    Box esr_out(0.5 * wrapped_scintillator_xy, 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_xy);
    Box esr_in(0.5 * wrapped_scintillator_xy - esr_thickness, 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);
    Volume esr("esr", SubtractionSolid(esr_out, esr_in, Position(0, 0, 0)), mat_ESR);
    esr.setVisAttributes(theDetector, "SeeThrough");

    // Odd-shaped scintillators
    Trapezoid odd_add_0_scintillator(0, long_elongation_sc.at(0) - short_elongation_sc.at(0), 0.5 * scintillator_z, 0.5 * scintillator_z, 0.5 * scintillator_xy);
    Trapezoid odd_add_45_scintillator(short_elongation_sc.at(1), long_elongation_sc.at(1), 0.5 * scintillator_z, 0.5 * scintillator_z, 0.5 * scintillator_xy);
    Trapezoid odd_add_9_scintillator(short_elongation_sc.at(2), long_elongation_sc.at(2), 0.5 * scintillator_z, 0.5 * scintillator_z, 0.5 * scintillator_xy);
    Trapezoid odd_add_14_scintillator(short_elongation_sc.at(3), long_elongation_sc.at(3), 0.5 * scintillator_z, 0.5 * scintillator_z, 0.5 * scintillator_xy);
    Trapezoid odd_add_17_scintillator(short_elongation_sc.at(4), long_elongation_sc.at(4), 0.5 * scintillator_z, 0.5 * scintillator_z, 0.5 * scintillator_xy);

    Box odd_scintillator_0_rect(0.5 * (scintillator_xy + short_elongation_sc.at(0)), 0.5 * scintillator_z, 0.5 * scintillator_xy);

    Volume odd_0_scintillator("odd_0_scintillator", UnionSolid(odd_scintillator_0_rect, odd_add_0_scintillator, Position(0.5 * (scintillator_xy + short_elongation_sc.at(0)), 0, 0)), mat_sensitive);
    odd_0_scintillator.setVisAttributes(theDetector, "SeeThrough");
    odd_0_scintillator.setSensitiveDetector(sens);

    Volume odd_45_scintillator("odd_45_scintillator", UnionSolid(normal_scintillator, odd_add_45_scintillator, Position(0.5 * scintillator_xy, 0, 0)), mat_sensitive);
    odd_45_scintillator.setVisAttributes(theDetector, "SeeThrough");
    odd_45_scintillator.setSensitiveDetector(sens);

    Volume odd_9_scintillator("odd_9_scintillator", UnionSolid(normal_scintillator, odd_add_9_scintillator, Position(0.5 * scintillator_xy, 0, 0)), mat_sensitive);
    odd_9_scintillator.setVisAttributes(theDetector, "SeeThrough");
    odd_9_scintillator.setSensitiveDetector(sens);

    Volume odd_14_scintillator("odd_14_scintillator", UnionSolid(normal_scintillator, odd_add_14_scintillator, Position(0.5 * scintillator_xy, 0, 0)), mat_sensitive);
    odd_14_scintillator.setVisAttributes(theDetector, "SeeThrough");
    odd_14_scintillator.setSensitiveDetector(sens);

    Volume odd_17_scintillator("odd_17_scintillator", UnionSolid(normal_scintillator, odd_add_17_scintillator, Position(0.5 * scintillator_xy, 0, 0)), mat_sensitive);
    odd_17_scintillator.setVisAttributes(theDetector, "SeeThrough");
    odd_17_scintillator.setSensitiveDetector(sens);

    // Odd-shaped ESR
    // Outer edge
    Trapezoid odd_add_0_esr_out(short_elongation_esr_out.at(0), long_elongation_esr_out.at(0), 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_xy);
    Trapezoid odd_add_45_esr_out(short_elongation_esr_out.at(1), long_elongation_esr_out.at(1), 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_xy);
    Trapezoid odd_add_9_esr_out(short_elongation_esr_out.at(2), long_elongation_esr_out.at(2), 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_xy);
    Trapezoid odd_add_14_esr_out(short_elongation_esr_out.at(3), long_elongation_esr_out.at(3), 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_xy);
    Trapezoid odd_add_17_esr_out(short_elongation_esr_out.at(4), long_elongation_esr_out.at(4), 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_z, 0.5 * wrapped_scintillator_xy);

    UnionSolid odd_0_esr_out(esr_out, odd_add_0_esr_out, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_45_esr_out(esr_out, odd_add_45_esr_out, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_9_esr_out(esr_out, odd_add_9_esr_out, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_14_esr_out(esr_out, odd_add_14_esr_out, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_17_esr_out(esr_out, odd_add_17_esr_out, Position(0.5 * wrapped_scintillator_xy, 0, 0));

    // Inner edge
    Trapezoid odd_add_0_esr_in(0, long_elongation_esr_in.at(0) - short_elongation_esr_in.at(0), 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);
    Trapezoid odd_add_45_esr_in(short_elongation_esr_in.at(1), long_elongation_esr_in.at(1), 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);
    Trapezoid odd_add_9_esr_in(short_elongation_esr_in.at(2), long_elongation_esr_in.at(2), 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);
    Trapezoid odd_add_14_esr_in(short_elongation_esr_in.at(3), long_elongation_esr_in.at(3), 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);
    Trapezoid odd_add_17_esr_in(short_elongation_esr_in.at(4), long_elongation_esr_in.at(4), 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);

    Box odd_esr_in_0_rect(0.5 * (wrapped_scintillator_xy - 2 * esr_thickness + short_elongation_esr_in.at(0)), 0.5 * wrapped_scintillator_z - esr_thickness, 0.5 * wrapped_scintillator_xy - esr_thickness);

    UnionSolid odd_0_esr_in(odd_esr_in_0_rect, odd_add_0_esr_in, Position(0.5 * (wrapped_scintillator_xy + short_elongation_esr_in.at(0)), 0, 0));
    UnionSolid odd_45_esr_in(esr_in, odd_add_45_esr_in, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_9_esr_in(esr_in, odd_add_9_esr_in, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_14_esr_in(esr_in, odd_add_14_esr_in, Position(0.5 * wrapped_scintillator_xy, 0, 0));
    UnionSolid odd_17_esr_in(esr_in, odd_add_17_esr_in, Position(0.5 * wrapped_scintillator_xy, 0, 0));

    // Odd-shaped ESR, from the subtraction of outer and inner frames
    Volume odd_0_esr("odd_0_esr", SubtractionSolid(odd_0_esr_out, odd_0_esr_in, Position(0, 0, 0)), mat_ESR);
    Volume odd_45_esr("odd_45_esr", SubtractionSolid(odd_45_esr_out, odd_45_esr_in, Position(0, 0, 0)), mat_ESR);
    Volume odd_9_esr("odd_9_esr", SubtractionSolid(odd_9_esr_out, odd_9_esr_in, Position(0, 0, 0)), mat_ESR);
    Volume odd_14_esr("odd_14_esr", SubtractionSolid(odd_14_esr_out, odd_14_esr_in, Position(0, 0, 0)), mat_ESR);
    Volume odd_17_esr("odd_17_esr", SubtractionSolid(odd_17_esr_out, odd_17_esr_in, Position(0, 0, 0)), mat_ESR);

    // Positions
    const double scintillator_pos_z = -0.5 * non_absorber_thickness + boundary_safety + 0.5 * wrapped_scintillator_z;
    const double esr_pos_z = scintillator_pos_z;
    const double pcb_pos_z = esr_pos_z + 0.5 * wrapped_scintillator_z + boundary_safety + 0.5 * pcb_thickness;

    // Loop for placing the units in a layer
    for (int ir = 1; ir <= Nrows; ++ir)
    {
        const double layer_length = dim_in + (ir - 1) * wrapped_scintillator_xy * tan(0.5 * angle);
        Ncells_phi = (int) (2 * layer_length / wrapped_scintillator_xy);
        const double layer_phi = Ncells_phi * wrapped_scintillator_xy;
        const double single_elongation = layer_length - 0.5 * layer_phi;

        Volume slice("slice", Trapezoid(layer_length, layer_length + elongation_angle_esr_out, 0.5 * non_absorber_thickness, 0.5 * non_absorber_thickness, 0.5 * (wrapped_scintillator_xy + boundary_safety)), mat_air);
        slice.setVisAttributes(theDetector, "SeeThrough");
        string slicename = "Slice_" + to_string(ir);
        DetElement sd(stave_det, slicename, det_id);

        Volume slice_pcb("slice_pcb", Box(0.5 * layer_phi, 0.5 * pcb_thickness, 0.5 * wrapped_scintillator_xy), mat_PCB);
        slice_pcb.setVisAttributes(theDetector, "SeeThrough");

        PlacedVolume pcb_unit = slice.placeVolume(slice_pcb, Position(0, pcb_pos_z, 0));
        pcb_unit.addPhysVolID("row", ir);

        for (int iphi = 1; iphi <= Ncells_phi; ++iphi)
        {
            PlacedVolume scintillator_unit;
            PlacedVolume esr_unit;

            Position position_scintillator((-0.5 * (Ncells_phi + 1) + iphi) * wrapped_scintillator_xy, scintillator_pos_z, 0);
            Position position_esr((-0.5 * (Ncells_phi + 1) + iphi) * wrapped_scintillator_xy, esr_pos_z, 0);

            Transform3D transform_scintillator(RotationZ(pi), position_scintillator);
            Transform3D transform_esr(RotationZ(pi), position_esr);

            if (iphi == 1)
            {
                if (single_elongation >= short_elongation_esr_out.at(4))
                {
                    scintillator_unit = slice.placeVolume(odd_17_scintillator, transform_scintillator);
                    esr_unit = slice.placeVolume(odd_17_esr, transform_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(3))
                {
                    scintillator_unit = slice.placeVolume(odd_14_scintillator, transform_scintillator);
                    esr_unit = slice.placeVolume(odd_14_esr, transform_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(2))
                {
                    scintillator_unit = slice.placeVolume(odd_9_scintillator, transform_scintillator);
                    esr_unit = slice.placeVolume(odd_9_esr, transform_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(1))
                {
                    scintillator_unit = slice.placeVolume(odd_45_scintillator, transform_scintillator);
                    esr_unit = slice.placeVolume(odd_45_esr, transform_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(0))
                {
//                    scintillator_unit = slice.placeVolume(odd_0_scintillator, transform_scintillator);
                    scintillator_unit = slice.placeVolume(odd_0_scintillator, Transform3D(RotationZ(pi),
                                Position((-0.5 * (Ncells_phi + 1) + iphi) * wrapped_scintillator_xy - 0.5 * short_elongation_esr_in.at(0),
                                         scintillator_pos_z,
                                         0)));
                    esr_unit = slice.placeVolume(odd_0_esr, transform_esr);
                }
            }
            else if (iphi == Ncells_phi)
            {
                if (single_elongation >= short_elongation_esr_out.at(4))
                {
                    scintillator_unit = slice.placeVolume(odd_17_scintillator, position_scintillator);
                    esr_unit = slice.placeVolume(odd_17_esr, position_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(3))
                {
                    scintillator_unit = slice.placeVolume(odd_14_scintillator, position_scintillator);
                    esr_unit = slice.placeVolume(odd_14_esr, position_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(2))
                {
                    scintillator_unit = slice.placeVolume(odd_9_scintillator, position_scintillator);
                    esr_unit = slice.placeVolume(odd_9_esr, position_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(1))
                {
                    scintillator_unit = slice.placeVolume(odd_45_scintillator, position_scintillator);
                    esr_unit = slice.placeVolume(odd_45_esr, position_esr);
                }
                else if (single_elongation >= short_elongation_esr_out.at(0))
                {
//                    scintillator_unit = slice.placeVolume(odd_0_scintillator, position_scintillator);
                    scintillator_unit = slice.placeVolume(odd_0_scintillator,
                                Position((-0.5 * (Ncells_phi + 1) + iphi) * wrapped_scintillator_xy + 0.5 * short_elongation_esr_in.at(0),
                                         scintillator_pos_z,
                                         0));
                    esr_unit = slice.placeVolume(odd_0_esr, position_esr);
                }
            }
            else
            {
                scintillator_unit = slice.placeVolume(scintillator, position_scintillator);
                esr_unit = slice.placeVolume(esr, position_esr);
            }

            scintillator_unit.addPhysVolID("row", ir).addPhysVolID("phi", iphi);
            esr_unit.addPhysVolID("row", ir).addPhysVolID("phi", iphi);

            string scintillator_name = "Scintillator_" + to_string(ir) + "_" + to_string(iphi);
            DetElement unit(sd, scintillator_name, det_id);
            unit.setPlacement(scintillator_unit);
        }

        PlacedVolume plv = layer_vol.placeVolume(slice, Position(0, 0, (-0.5 * (Nrows + 1) + ir) * (wrapped_scintillator_xy + boundary_safety)));
        plv.addPhysVolID("row", ir);
        sd.setPlacement(plv);
    }

    // Loop for placing the layers in a module
    for (int ilayer = 1; ilayer <= Nlayers; ++ilayer)
    {
        PlacedVolume plv = stave_vol.placeVolume(layer_vol, Position(0, (ilayer - 1) * layer_thickness + cassette_thickness + 0.5 * non_absorber_thickness - 0.5 * module_thickness, 0));
        plv.addPhysVolID("layer", ilayer);
        DetElement sd(stave_det, _toString(ilayer, "layer_%3d"), det_id);
        sd.setPlacement(plv);
    }

    // Loop for placing the modules
    for (int i = 0; i < Nmodules; ++i)
    {
        const double m_rot = i * angle;
        const double posx = -r0 * sin(m_rot);
        const double posy = r0 * cos(m_rot);

        Transform3D transform_neg(RotationZ(m_rot) * RotationX(-0.5 * pi), Position(posx, posy, 0));
        Transform3D transform_pos(RotationZ(m_rot) * RotationY(pi) * RotationX(-0.5 * pi), Position(posx, posy, 2 * pos_z));
        PlacedVolume plv_neg = envelopeVol.placeVolume(stave_vol, transform_neg);
        PlacedVolume plv_pos = envelopeVol.placeVolume(stave_vol, transform_pos);
        plv_neg.addPhysVolID("stave", i);
        plv_pos.addPhysVolID("stave", i + Nmodules);
        DetElement sd_neg(AHCAL, _toString(i, "sector%3d"), det_id);
        DetElement sd_pos(AHCAL, _toString(i + Nmodules, "sector%3d"), det_id);
        sd_neg.setPlacement(plv_neg);
        sd_pos.setPlacement(plv_pos);
    }

    sens.setType("calorimeter");

    MYDEBUG("create_detector FINISHED.");
    return AHCAL;
}

DECLARE_DETELEMENT(SHcalSc04_Endcaps_v02, create_detector)
