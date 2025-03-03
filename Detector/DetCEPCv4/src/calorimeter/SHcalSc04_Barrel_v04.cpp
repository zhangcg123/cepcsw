//====================================================================
//  DD4hep Geometry driver for HcalBarrel
//--------------------------------------------------------------------
//  S.Lu, DESY
//  F. Gaede, DESY :  v04 : prepare for multi segmentation
//     18.04.2017            - copied from SHcalSc04_Barrel_v01.cpp
//                           - add optional parameter <subsegmentation key="" value=""/>
//                             defines the segmentation to be used in reconstruction
//====================================================================
#include "DD4hep/Printout.h"
#include "DD4hep/DetFactoryHelper.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/BitField64.h"
#include "DDSegmentation/TiledLayerGridXY.h"
#include "DDSegmentation/Segmentation.h"
#include "DDSegmentation/MultiSegmentation.h"
#include "LcgeoExceptions.h"

#include <iostream>
#include <vector>

using namespace std;

using dd4hep::_toString;
using dd4hep::Box;
using dd4hep::BUILD_ENVELOPE;
using dd4hep::Detector;
using dd4hep::DetElement;
using dd4hep::IntersectionSolid;
using dd4hep::Material;
using dd4hep::PlacedVolume;
using dd4hep::Position;
using dd4hep::Readout;
using dd4hep::Ref_t;
using dd4hep::Rotation3D;
using dd4hep::RotationZ;
using dd4hep::RotationZYX;
using dd4hep::SensitiveDetector;
using dd4hep::Transform3D;
using dd4hep::Trapezoid;
using dd4hep::Tube;
using dd4hep::Volume;

using dd4hep::rec::LayeredCalorimeterData;

// After reading in all the necessary parameters.
// To check the radius range and the space for placing the total layers
static bool validateEnvelope(double rInner, double rOuter, double radiatorThickness, double layerThickness, int layerNumber, int nsymmetry)
{

  bool Error = false;
  bool Warning = false;
  double spaceAllowed = rOuter * cos(M_PI / nsymmetry) - rInner;
  double spaceNeeded = (radiatorThickness + layerThickness) * layerNumber;
  double spaceToleranted = (radiatorThickness + layerThickness) * (layerNumber + 1);
  double rOuterRecommaned = (rInner + spaceNeeded) / cos(M_PI / 16.);
  int layerNumberRecommaned = floor((spaceAllowed) / (radiatorThickness + layerThickness));

  if (spaceNeeded > spaceAllowed)
  {
    printout(dd4hep::ERROR, "SHcalSc04_Barrel_v04", " Layer number is more than it can be built! ");
    Error = true;
  }
  else if (spaceToleranted < spaceAllowed)
  {
    printout(dd4hep::WARNING, "SHcalSc04_Barrel_v04", " Layer number is less than it is able to build!");
    Warning = true;
  }
  else
  {
    printout(dd4hep::DEBUG, "SHcalSc04_Barrel_v04", " has been validated and start to build it.");
    Error = false;
    Warning = false;
  }

  if (Error)
  {
    cout << "\n ============> First Help Documentation <=============== \n"
         << " When you see this message, that means you are crashing the module. \n"
         << " Please take a cup of cafe, and think about what you want to do! \n"
         << " Here are few FirstAid# for you. Please read them carefully. \n"
         << " \n"
         << " ###  FirstAid 1: ###\n"
         << " If you want to build HCAL within the rInner and rOuter range, \n"
         << " please reduce the layer number to " << layerNumberRecommaned << " \n"
         << " with the current layer thickness structure. \n"
         << " \n"
         << " You may redisgn the layer structure and thickness, too. \n"
         << " \n"
         << " ###  FirstAid 2: ###\n"
         << " If you want to build HCAL with this layer number and the layer thickness, \n"
         << " you have to update rOuter to " << rOuterRecommaned * 10. << "*mm \n"
         << " and to inform other subdetector, you need this space for building your design. \n"
         << " \n"
         << " ###  FirstAid 3: ###\n"
         << " Do you think that you are looking for another type of HCAL driver? \n"
         << " \n"
         << endl;
    throw lcgeo::GeometryException("SHcalSc04_Barrel: Error: Layer number is more than it can be built!");
  }
  else if (Warning)
  {
    cout << "\n ============> First Help Documentation <=============== \n"
         << " When you see this warning message, that means you are changing the module. \n"
         << " Please take a cup of cafe, and think about what you want to do! \n"
         << " Here are few FirstAid# for you. Please read them carefully. \n"
         << " \n"
         << " ###  FirstAid 1: ###\n"
         << " If you want to build HCAL within the rInner and rOuter range, \n"
         << " You could build the layer number up to " << layerNumberRecommaned << " \n"
         << " with the current layer thickness structure. \n"
         << " \n"
         << " You may redisgn the layer structure and thickness, too. \n"
         << " \n"
         << " ###  FirstAid 2: ###\n"
         << " If you want to build HCAL with this layer number and the layer thickness, \n"
         << " you could reduce rOuter to " << rOuterRecommaned * 10. << "*mm \n"
         << " and to reduce the back plate thickness, which you may not need for placing layer. \n"
         << " \n"
         << " ###  FirstAid 3: ###\n"
         << " Do you think that you are looking for another type of HCAL driver? \n"
         << " \n"
         << endl;
    return Warning;
  }
  else
  {
    return true;
  }
}

static Ref_t create_detector(Detector &theDetector, xml_h element, SensitiveDetector sens)
{

  double boundarySafety = 0.0001;

  xml_det_t x_det = element;
  string det_name = x_det.nameStr();
  int det_id = x_det.id();
  DetElement sdet(det_name, det_id);

  // --- create an envelope volume and position it into the world ---------------------

  Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, element, sdet);

  dd4hep::xml::setDetectorTypeFlag(element, sdet);

  if (theDetector.buildType() == BUILD_ENVELOPE)
    return sdet;

  //-----------------------------------------------------------------------------------

  xml_comp_t x_staves = x_det.staves(); // absorber
  Material stavesMaterial = theDetector.material(x_staves.materialStr());
  Material air = theDetector.air();

  PlacedVolume pv;

  sens.setType("calorimeter");

  //====================================================================
  //
  // Read all the constant from ILD_o1_v05.xml
  // Use them to build HcalBarrel
  //
  //====================================================================
  double Hcal_inner_radius = theDetector.constant<double>("Hcal_inner_radius");
  double Hcal_outer_radius = theDetector.constant<double>("Hcal_outer_radius");
  double Hcal_half_length = theDetector.constant<double>("Hcal_half_length");
  int Hcal_inner_symmetry = theDetector.constant<int>("Hcal_inner_symmetry");
  int Hcal_outer_symmetry = 0; // Fixed shape for Tube, and not allow to modify from compact xml
  double Hcal_cell_size = theDetector.constant<double>("Hcal_cell_size");
  double Hcal_cell_size_abnormal = theDetector.constant<double>("Hcal_cell_size_abnormal");
  double Hcal_scintillator_air_gap = theDetector.constant<double>("Hcal_scintillator_air_gap");
  double Hcal_scintillator_ESR_thickness = theDetector.constant<double>("Hcal_scintillator_ESR_thickness");
  double Hcal_scintillator_thickness = theDetector.constant<double>("Hcal_scintillator_thickness");
  double Hcal_radiator_thickness = theDetector.constant<double>("Hcal_radiator_thickness");
  double Hcal_chamber_thickness = theDetector.constant<double>("Hcal_chamber_thickness");
  double Hcal_back_plate_thickness = theDetector.constant<double>("Hcal_back_plate_thickness");
  double Hcal_lateral_plate_thickness = theDetector.constant<double>("Hcal_lateral_structure_thickness");
  double Hcal_stave_gaps = theDetector.constant<double>("Hcal_stave_gaps");
  // double Hcal_modules_gap = theDetector.constant<double>("Hcal_modules_gap");
  double Hcal_middle_stave_gaps = theDetector.constant<double>("Hcal_middle_stave_gaps");
  double Hcal_layer_air_gap = theDetector.constant<double>("Hcal_layer_air_gap");
  // double      Hcal_cells_size                  = theDetector.constant<double>("Hcal_cells_size");

  int Hcal_nlayers = theDetector.constant<int>("Hcal_nlayers");

  //=================================================================================
  //  if Hcal_outer_radius stands for the radius of circumcircle, comment next line 
  Hcal_outer_radius /= cos(M_PI / Hcal_inner_symmetry);
  //=================================================================================
  printout(dd4hep::INFO, "SHcalSc04_Barrel_v04", "Hcal_inner_radius : %e   - Hcal_outer_radius %e ", Hcal_inner_radius, Hcal_outer_radius);
  validateEnvelope(Hcal_inner_radius, Hcal_outer_radius, Hcal_radiator_thickness, Hcal_chamber_thickness, Hcal_nlayers, Hcal_inner_symmetry);

  // fg: this is a bit tricky: we first have to check whether a multi segmentation is used and then, if so, we
  //     will check the <subsegmentation key="" value=""/> element, for which subsegmentation to use for filling the
  //     DDRec:LayeredCalorimeterData information.
  //     Additionally, we need to figure out if there is a TiledLayerGridXY instance defined -
  //     and in case, there is more than one defined, we need to pick the correct one specified via the
  //     <subsegmentation key="" value=""/> element.
  //     This involves a lot of casting:  review the API in DD4hep !!

  // check if we have a multi segmentation :

  //========== fill data for reconstruction ============================
  LayeredCalorimeterData *caloData = new LayeredCalorimeterData;
  caloData->layoutType = LayeredCalorimeterData::BarrelLayout;
  caloData->inner_symmetry = Hcal_inner_symmetry;
  caloData->outer_symmetry = Hcal_outer_symmetry;
  caloData->phi0 = 0; // fg: also hardcoded below

  /// extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
  caloData->extent[0] = Hcal_inner_radius;
  caloData->extent[1] = Hcal_outer_radius;
  caloData->extent[2] = 0.; // Barrel zmin is "0" by default.
  caloData->extent[3] = Hcal_half_length;

  //====================================================================
  //
  // general calculated parameters
  //
  //====================================================================

  double Hcal_total_dim_y = Hcal_nlayers * (Hcal_radiator_thickness + Hcal_chamber_thickness) + Hcal_back_plate_thickness;

  // double Hcal_y_dim1_for_x = Hcal_outer_radius * cos(M_PI / Hcal_inner_symmetry) - Hcal_inner_radius;
  double Hcal_bottom_dim_x = 2. * Hcal_inner_radius * tan(M_PI / Hcal_inner_symmetry) - Hcal_lateral_plate_thickness * 2 - Hcal_stave_gaps;
  // double Hcal_normal_dim_z = (2 * Hcal_half_length - Hcal_modules_gap) / 2.;
  double Hcal_normal_dim_z = Hcal_half_length;

  // only the middle has the steel plate.
  // double Hcal_regular_chamber_dim_z = Hcal_normal_dim_z - Hcal_lateral_plate_thickness;
  double Hcal_regular_chamber_dim_z = Hcal_normal_dim_z;

  // double Hcal_cell_dim_x            = Hcal_cells_size;
  // double Hcal_cell_dim_z            = Hcal_regular_chamber_dim_z / floor (Hcal_regular_chamber_dim_z/Hcal_cell_dim_x);

  // ========= Create Hcal Barrel stave   ====================================
  //  It will be the volume for palcing the Hcal Barrel Chamber(i.e. Layers).
  //  Itself will be placed into the world volume.
  // ==========================================================================

  // stave modules shaper parameters
  double BHX = (Hcal_bottom_dim_x + Hcal_lateral_plate_thickness * 2 + Hcal_stave_gaps) / 2.;
  double THX = (Hcal_total_dim_y + Hcal_inner_radius) * tan(M_PI / Hcal_inner_symmetry);
  double YXH = Hcal_total_dim_y / 2.;
  // double DHZ = (Hcal_normal_dim_z - Hcal_lateral_plate_thickness) / 2.;
  double DHZ = Hcal_normal_dim_z;

  Trapezoid stave_shaper(THX, BHX, DHZ, DHZ, YXH);

  Tube solidCaloTube(0, Hcal_outer_radius, DHZ + boundarySafety);

  RotationZYX mrot(0, 0, M_PI / 2.);

  Rotation3D mrot3D(mrot);
  Position mxyzVec(0, 0, (Hcal_inner_radius + Hcal_total_dim_y / 2.));
  Transform3D mtran3D(mrot3D, mxyzVec);

  IntersectionSolid barrelModuleSolid(stave_shaper, solidCaloTube, mtran3D);

  Volume EnvLogHcalModuleBarrel(det_name + "_module", barrelModuleSolid, stavesMaterial);

  EnvLogHcalModuleBarrel.setAttributes(theDetector, x_staves.regionStr(), x_staves.limitsStr(), x_staves.visStr());

  // stave modules lateral plate shaper parameters
  // double BHX_LP = BHX;
  // double THX_LP = THX;
  // double YXH_LP = YXH;

  // // build lateral palte here to simulate lateral plate in the middle of barrel.
  // double DHZ_LP = Hcal_lateral_plate_thickness / 2.0;

  // Trapezoid stave_shaper_LP(THX_LP, BHX_LP, DHZ_LP, DHZ_LP, YXH_LP);

  // Tube solidCaloTube_LP(0, Hcal_outer_radius, DHZ_LP + boundarySafety);

  // IntersectionSolid Module_lateral_plate(stave_shaper_LP, solidCaloTube_LP, mtran3D);

  // Volume EnvLogHcalModuleBarrel_LP(det_name + "_Module_lateral_plate", Module_lateral_plate, stavesMaterial);

  // EnvLogHcalModuleBarrel_LP.setAttributes(theDetector, x_det.regionStr(), x_det.limitsStr(), x_det.visStr());

#ifdef SHCALSC04_DEBUG
  std::cout << " ==> Hcal_outer_radius: " << Hcal_outer_radius << std::endl;
#endif

  //====================================================================
  //
  // Chambers in the HCAL BARREL
  //
  //====================================================================
  // Build Layer Chamber fill with air, which include the tolarance space at the two side
  // place the slice into the Layer Chamber
  // Place the Layer Chamber into the Stave module
  // place the Stave module into the asembly Volume
  // place the module middle lateral plate into asembly Volume

  double x_halflength; // dimension of an Hcal barrel layer on the x-axis
  double y_halfheight; // dimension of an Hcal barrel layer on the y-axis
  double z_halfwidth;  // dimension of an Hcal barrel layer on the z-axis
  x_halflength = 0.;   // Each Layer the x_halflength has new value.
  y_halfheight = Hcal_chamber_thickness / 2.;
  z_halfwidth = Hcal_regular_chamber_dim_z;

  double xOffset = 0.; // the x_halflength of a barrel layer is calculated as a
  // barrel x-dimension plus (bottom barrel) or minus
  //(top barrel) an x-offset, which depends on the angle M_PI/Hcal_inner_symmetry

  double xShift = 0.; // Geant4 draws everything in the barrel related to the
  // center of the bottom barrel, so we need to shift the layers to
  // the left (or to the right) with the quantity xShift

  // read sensitive cell parameters and create it

  // xml_comp_t x_scintillator = x_det.solid();
  // Material sensitiveMaterial = theDetector.material(x_scintillator.materialStr());
  // xml_comp_t x_ESR = x_det.shape();
  // Material ladderMaterial = theDetector.material(x_ESR.materialStr());

  Volume cell_vol;
  Volume scintillator_vol;
  Volume cell_vol_abnormal;
  Volume scintillator_vol_abnormal;
  xml_comp_t x_layer_tmp(x_det.child(_U(layer)));
  for (xml_coll_t k(x_layer_tmp, _U(slice)); k; ++k)
  {
    xml_comp_t x_slice = k;
    if (x_slice.isSensitive())
    {
      // child elements: ladder and sensitive
      xml_comp_t x_sensitive(x_slice.child(_U(sensitive)));
      xml_comp_t x_ladder(x_slice.child(_U(ladder)));
      Material sensitiveMaterial = theDetector.material(x_sensitive.materialStr());
      Material ladderMaterial = theDetector.material(x_ladder.materialStr());

      // Store scintillator thickness
      double cell_thickness = Hcal_scintillator_thickness + 2 * Hcal_scintillator_ESR_thickness;

      double cell_size = 2 * Hcal_scintillator_ESR_thickness + Hcal_cell_size;
      // place sensitive cell into ESR cell
      string cell_name = "cell";
      string scintillator_name = cell_name + "_scintillator";
      // create sensitive cell with ESR
      Box cell_box(cell_size / 2., cell_size / 2., cell_thickness / 2.);
      // string cell_name=
      // Volume cell_vol_tmp(cell_name, cell_box, ladderMaterial);
      cell_vol = Volume(cell_name, cell_box, ladderMaterial);
      cell_vol.setAttributes(theDetector, x_ladder.regionStr(), x_ladder.limitsStr(), x_ladder.visStr());
      Box scintillator_box(Hcal_cell_size / 2., Hcal_cell_size / 2., Hcal_scintillator_thickness / 2.);
      // Volume scintillator_vol_tmp(scintillator_name, scintillator_box, sensitiveMaterial);
      scintillator_vol = Volume(scintillator_name, scintillator_box, sensitiveMaterial);
      scintillator_vol.setAttributes(theDetector, x_sensitive.regionStr(), x_sensitive.limitsStr(), x_sensitive.visStr());
      scintillator_vol.setSensitiveDetector(sens);
      cell_vol.placeVolume(scintillator_vol, Position(0., 0., 0.));
      // cout << x_ladder.visStr() << "   " << x_sensitive.visStr() << endl;
      ///////////////////////////////
      // abnormal cell             //
      ///////////////////////////////
      double cell_size_abnormal = 2 * Hcal_scintillator_ESR_thickness + Hcal_cell_size_abnormal;
      // place sensitive cell into ESR cell
      string cell_name_abnormal = "cell_abnormal";
      string scintillator_name_abnormal = cell_name_abnormal + "_scintillator_abnormal";
      // create sensitive cell with ESR
      Box cell_box_abnormal(cell_size_abnormal / 2., cell_size / 2., cell_thickness / 2.);
      // string cell_name=
      // Volume cell_vol_tmp_abnormal(cell_name_abnormal, cell_box_abnormal, ladderMaterial);
      cell_vol_abnormal = Volume(cell_name_abnormal, cell_box_abnormal, ladderMaterial);
      cell_vol_abnormal.setAttributes(theDetector, x_ladder.regionStr(), x_ladder.limitsStr(), x_ladder.visStr());
      Box scintillator_box_abnormal(Hcal_cell_size_abnormal / 2., Hcal_cell_size / 2., Hcal_scintillator_thickness / 2.);
      // Volume scintillator_vol_tmp_abnormal(scintillator_name_abnormal, scintillator_box_abnormal, sensitiveMaterial);
      scintillator_vol_abnormal = Volume(scintillator_name_abnormal, scintillator_box_abnormal, sensitiveMaterial);
      scintillator_vol_abnormal.setAttributes(theDetector, x_sensitive.regionStr(), x_sensitive.limitsStr(), x_sensitive.visStr());
      scintillator_vol_abnormal.setSensitiveDetector(sens);
      cell_vol_abnormal.placeVolume(scintillator_vol_abnormal, Position(0., 0., 0.));
    }
  }

  // sensitive cell creating done

  //-------------------- start loop over HCAL layers ----------------------
  // int n_x_layer_1 = 0;
  for (int layer_id = 1; layer_id <= (Hcal_nlayers); layer_id++)
  {

    double TanPiDiv8 = tan(M_PI / Hcal_inner_symmetry);
    double x_total = 0.;
    // double x_halfLength;
    x_halflength = 0.;

    // int layer_id = 0;

    // if ((layer_id < Hcal_nlayers) || (layer_id > Hcal_nlayers && layer_id < (2 * Hcal_nlayers)))
    //   layer_id = layer_id % Hcal_nlayers;
    // else if ((layer_id == Hcal_nlayers) || (layer_id == 2 * Hcal_nlayers))
    //   layer_id = Hcal_nlayers;

    //---- bottom barrel------------------------------------------------------------
    // if (layer_id * (Hcal_radiator_thickness + Hcal_chamber_thickness) < (Hcal_outer_radius * cos(M_PI / Hcal_inner_symmetry) - Hcal_inner_radius))
    // {
    xOffset = (layer_id * Hcal_radiator_thickness + (layer_id - 1) * Hcal_chamber_thickness) * TanPiDiv8;

    x_total = Hcal_bottom_dim_x - Hcal_middle_stave_gaps + xOffset * 2;
    x_halflength = x_total - 2 * Hcal_layer_air_gap;
    // x_halfLength = x_halflength / 2.;
    // }
    // else
    // { //----- top barrel -------------------------------------------------
    //   double y_layerID = layer_id * (Hcal_radiator_thickness + Hcal_chamber_thickness) + Hcal_inner_radius;
    //   double ro_layer = Hcal_outer_radius - Hcal_radiator_thickness;

    //   x_total = sqrt(ro_layer * ro_layer - y_layerID * y_layerID);

    //   x_halflength = x_total - Hcal_middle_stave_gaps;

    //   x_halfLength = x_halflength / 2.;

    //   xOffset = (layer_id * Hcal_radiator_thickness + (layer_id - 1) * Hcal_chamber_thickness - Hcal_y_dim1_for_x) / TanPiDiv8 + Hcal_chamber_thickness / TanPiDiv8;
    // }

    x_halflength = x_halflength / 2.;

    LayeredCalorimeterData::Layer caloLayer;
    caloLayer.cellSize0 = Hcal_cell_size;
    caloLayer.cellSize1 = Hcal_cell_size;

    //--------------------------------------------------------------------------------
    //  build chamber box, with the calculated dimensions
    //-------------------------------------------------------------------------------
    printout(dd4hep::DEBUG, "SHcalSc04_Barrel_v04", " \n Start to build layer chamber - layer_id: %d", layer_id);
    printout(dd4hep::DEBUG, "SHcalSc04_Barrel_v04", " chamber x:y:z:  %e:%e:%e", x_halflength * 2., z_halfwidth * 2., y_halfheight * 2.);

    // check if x_halflength (scintillator length) is divisible with x_integerTileSize

    Box ChamberSolid((x_halflength + Hcal_layer_air_gap), // x + air gaps at two side, do not need to build air gaps individualy.
                     z_halfwidth,                         // z attention!
                     y_halfheight);                       // y attention!

    string ChamberLogical_name = det_name + _toString(layer_id, "_layer%d");

    Volume ChamberLogical(ChamberLogical_name, ChamberSolid, air);
    ChamberLogical.setAttributes(theDetector, x_layer_tmp.regionStr(), x_layer_tmp.limitsStr(), x_layer_tmp.visStr());

    double layer_thickness = y_halfheight * 2.;

    double nRadiationLengths = 0.;
    double nInteractionLengths = 0.;
    double thickness_sum = 0;

    nRadiationLengths = Hcal_radiator_thickness / (stavesMaterial.radLength());
    nInteractionLengths = Hcal_radiator_thickness / (stavesMaterial.intLength());
    thickness_sum = Hcal_radiator_thickness;

    //====================================================================
    // Create Hcal Barrel Chamber without radiator
    // Place into the Hcal Barrel stave, after each radiator
    //====================================================================
    xml_coll_t c(x_det, _U(layer));
    xml_comp_t x_layer = c;
    string layer_name = det_name + _toString(layer_id, "_layer%d");

    // Create the slices (sublayers) within the Hcal Barrel Chamber.
    double slice_pos_z = layer_thickness / 2.;
    int slice_number = 0;

    for (xml_coll_t k(x_layer, _U(slice)); k; ++k)
    {
      xml_comp_t x_slice = k;
      string slice_name = layer_name + _toString(slice_number, "_slice%d");
      double slice_thickness = x_slice.thickness();
      Material slice_material = theDetector.material(x_slice.materialStr());
      DetElement slice(layer_name, _toString(slice_number, "slice%d"), x_det.id());

      slice_pos_z -= slice_thickness / 2.;

      // Slice volume & box
      Volume slice_vol(slice_name, Box(x_halflength, z_halfwidth, slice_thickness / 2.), slice_material);
      slice_vol.setAttributes(theDetector, x_slice.regionStr(), x_slice.limitsStr(), x_slice.visStr());

      nRadiationLengths += slice_thickness / (2. * slice_material.radLength());
      nInteractionLengths += slice_thickness / (2. * slice_material.intLength());
      thickness_sum += slice_thickness / 2;
      if (x_slice.isSensitive())
      {
        // Store "inner" quantities
        caloLayer.inner_nRadiationLengths = nRadiationLengths;
        caloLayer.inner_nInteractionLengths = nInteractionLengths;
        caloLayer.inner_thickness = thickness_sum;
        // Store scintillator thickness
        caloLayer.sensitive_thickness = Hcal_scintillator_thickness;

        double cell_size = 2 * Hcal_scintillator_ESR_thickness + Hcal_cell_size;
        double cell_size_abnormal = 2 * Hcal_scintillator_ESR_thickness + Hcal_cell_size_abnormal;
        double cell_x_offset = 0, cell_z_offset = 0, cell_y_offset = 0;
        // place cell into slice one by one
        double total_cell_size = cell_size + Hcal_scintillator_air_gap;                   // include air gap
        double total_cell_size_abnormal = cell_size_abnormal + Hcal_scintillator_air_gap; // include air gap
        int n_x = (2 * x_halflength) / total_cell_size;
        int n_z = (2 * z_halfwidth) / total_cell_size;
        int nx_abnormal = (2 * x_halflength - n_x * total_cell_size) / (total_cell_size_abnormal - total_cell_size);
        n_x -= nx_abnormal;
        // if(layer_id==1){n_x_layer_1=n_x;}
        // else{
        //   n_x=(n_x-n_x_layer_1)/2;
        //   n_x=n_x*2+n_x_layer_1;
        // }
        for (int index_cell_z = 0; index_cell_z < n_z; index_cell_z++)
        {
          cell_x_offset = (n_x * total_cell_size + nx_abnormal * total_cell_size_abnormal) / 2.;
          cell_z_offset = (n_z / 2. - index_cell_z - 0.5) * total_cell_size;
          for (int index_cell_x = 0; index_cell_x < n_x; index_cell_x++)
          {
            cell_x_offset -= total_cell_size / 2.;
            int cell_number = index_cell_x + 100 * index_cell_z;
            string cell_name = slice_name + _toString(cell_number, "_cell%d");
            string scintillator_name = cell_name + "_scintillator";
            // cell_y_offset = slice_thickness / 2.;
            PlacedVolume cell_phv = slice_vol.placeVolume(cell_vol, Position(cell_x_offset, cell_z_offset, cell_y_offset));
            cell_phv.addPhysVolID("tile", cell_number);

            DetElement cell(cell_name, _toString(cell_number, "cell%d"), cell_number);
            cell.setPlacement(cell_phv);
            cell_x_offset -= total_cell_size / 2.;
          }
          for (int index_cell_x = 0; index_cell_x < nx_abnormal; index_cell_x++)
          {
            cell_x_offset -= total_cell_size_abnormal / 2.;
            int cell_number = index_cell_x + n_x + 100 * index_cell_z;
            string cell_name = slice_name + _toString(cell_number, "_cell%d");
            string scintillator_name = cell_name + "_scintillator";
            // cell_y_offset = slice_thickness / 2.;
            PlacedVolume cell_phv = slice_vol.placeVolume(cell_vol_abnormal, Position(cell_x_offset, cell_z_offset, cell_y_offset));
            cell_phv.addPhysVolID("tile", cell_number);

            DetElement cell(cell_name, _toString(cell_number, "cell%d"), cell_number);
            cell.setPlacement(cell_phv);
            cell_x_offset -= total_cell_size_abnormal / 2.;
          }
        }
        //printf("layerID: %2d, x_length: %3.3lf, x_cell: %3.3lf, x_cell_abnormal: %3.3lf, x_dead: %3.3lf, z_length: %3.3lf,z_cell: %3.3lf, z_dead: %3.3lf,\n", layer_id,  2 * x_halflength,  n_x * total_cell_size,  nx_abnormal * total_cell_size_abnormal,  2 * x_halflength - n_x * total_cell_size - nx_abnormal * total_cell_size_abnormal,  2 * z_halfwidth,  n_z * total_cell_size,  2 * z_halfwidth - n_z * total_cell_size);
        // Reset counters to measure "outside" quantitites
        nRadiationLengths = 0.;
        nInteractionLengths = 0.;
        thickness_sum = 0.;
        // }
      }
      nRadiationLengths += slice_thickness / (2. * slice_material.radLength());
      nInteractionLengths += slice_thickness / (2. * slice_material.intLength());
      thickness_sum += slice_thickness / 2;

      // Set region, limitset, and vis.
      // slice_vol.setAttributes(theDetector, x_slice.regionStr(), x_slice.limitsStr(), x_slice.visStr());
      // slice PlacedVolume
      PlacedVolume slice_phv = ChamberLogical.placeVolume(slice_vol, Position(0., 0., slice_pos_z));

      slice_phv.addPhysVolID("layer", layer_id);
      slice.setPlacement(slice_phv);
      // Increment z position for next slice.
      slice_pos_z -= slice_thickness / 2.;
      // Increment slice number.
      ++slice_number;
    }

    // Store "outer" quantities
    caloLayer.outer_nRadiationLengths = nRadiationLengths;
    caloLayer.outer_nInteractionLengths = nInteractionLengths;
    caloLayer.outer_thickness = thickness_sum;

    //---------------------------  Chamber Placements -----------------------------------------

    double chamber_x_offset, chamber_y_offset, chamber_z_offset;
    chamber_x_offset = xShift;

    chamber_z_offset = 0;

    chamber_y_offset = -(-Hcal_total_dim_y / 2. + (layer_id - 1) * (Hcal_chamber_thickness + Hcal_radiator_thickness) + Hcal_radiator_thickness + Hcal_chamber_thickness / 2.);

    pv = EnvLogHcalModuleBarrel.placeVolume(ChamberLogical,
                                            Position(chamber_x_offset, chamber_z_offset, chamber_y_offset));

    //-----------------------------------------------------------------------------------------
    if (layer_id <= Hcal_nlayers)
    { // add the first set of layers to the reconstruction data

      caloLayer.distance = Hcal_inner_radius + Hcal_total_dim_y / 2.0 - chamber_y_offset - caloLayer.inner_thickness;
      // Will be added later at "DDMarlinPandora/DDGeometryCreator.cc:226" to get center of sensitive element

      caloLayer.absorberThickness = Hcal_radiator_thickness;

      caloData->layers.push_back(caloLayer);
    }
    //-----------------------------------------------------------------------------------------

  } // end loop over HCAL nlayers;

  //====================================================================
  // Place HCAL Barrel stave module into the envelope
  //====================================================================
  double stave_phi_offset, module_z_offset = 0;

  double Y = Hcal_inner_radius + Hcal_total_dim_y / 2.;

  stave_phi_offset = M_PI / Hcal_inner_symmetry - M_PI / 2.;

  //-------- start loop over HCAL BARREL staves ----------------------------

  for (int stave_id = 1; stave_id <= Hcal_inner_symmetry; stave_id++)
  {
    double phirot = stave_phi_offset;
    RotationZYX srot(0, phirot, M_PI * 0.5);
    Rotation3D srot3D(srot);

    // for (int module_id = 1; module_id <= 2; module_id++)
    // {
    Position sxyzVec(-Y * sin(phirot), Y * cos(phirot), module_z_offset);
    Transform3D stran3D(srot3D, sxyzVec);

    // Place Hcal Barrel volume into the envelope volume
    pv = envelope.placeVolume(EnvLogHcalModuleBarrel, stran3D);
    pv.addPhysVolID("stave", stave_id);
    // pv.addPhysVolID("module", module_id);
    pv.addPhysVolID("system", det_id);

    // const int staveCounter = (stave_id - 1) * 2 + module_id - 1;
    const int staveCounter = stave_id - 1;
    DetElement stave(sdet, _toString(staveCounter, "stave%d"), staveCounter);
    stave.setPlacement(pv);

    // Position xyzVec_LP(-Y * sin(phirot), Y * cos(phirot), lateral_plate_z_offset);
    // Transform3D tran3D_LP(srot3D, xyzVec_LP);
    // pv = envelope.placeVolume(EnvLogHcalModuleBarrel_LP, tran3D_LP);

    // module_z_offset = -module_z_offset;
    // lateral_plate_z_offset = -lateral_plate_z_offset;
    // }

    stave_phi_offset -= M_PI * 2.0 / Hcal_inner_symmetry;
  } //-------- end loop over HCAL BARREL staves ----------------------------

  sdet.addExtension<LayeredCalorimeterData>(caloData);

  return sdet;
}

DECLARE_DETELEMENT(SHcalSc04_Barrel_v04, create_detector)
