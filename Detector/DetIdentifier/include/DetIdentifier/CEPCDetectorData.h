//==========================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     :
//
//==========================================================================
#ifndef CEPC_DETECTORDATA_H
#define CEPC_DETECTORDATA_H

#include "DDRec/DetectorData.h"
#include <boost/io/ios_state.hpp>

namespace dd4hep {
  namespace rec {
    /** Simple data structure with key parameters for
     *  reconstruction of a cylindrical detector
     *
     * @author 
     * @date Sep, 02 2024
     * @version $Id: $
     */
    struct CylindricalStruct {
      ///  The half length (z) of the support shell (w/o gap) - 0. if no shell exists.
      double zHalfShell;
      ///  The length of the gap in mm (gap position at z=0).
      double  gapShell;
      ///  The inner radius of the support shell.
      double rInnerShell;
      ///  The outer radius of the support shell.
      double rOuterShell;

      /**Internal helper struct for defining the layer layout. Layers are defined
       * with a sensitive part and a support part.
       */
      struct LayerLayout {
	// default c'tor with zero initialization
        LayerLayout() :
	  id(0),
	  zHalf(0),
	  radiusSensitive(0),
          thicknessSensitive(0),
          radiusSupport(0),
          thicknessSupport(0),
	  phi0(0),
	  rgap(0),
	  dphi(0),  
          offset(0) {
	}
	int id;
	/// half length
	double zHalf;
	/// inner radius
	double radiusSensitive;
	/// thickness
	double thicknessSensitive;
	///
	double radiusSupport;
	///
	double thicknessSupport;
	///
	double width;
	///
	double phi0;
	///
	double rgap;
	///
	double dphi;
	///
	double offset;
      };
      std::vector<LayerLayout> layers;
    };
    typedef StructExtension<CylindricalStruct> CylindricalData;

    static std::ostream& operator<<( std::ostream& io , const CylindricalData& d ) {
      boost::io::ios_base_all_saver ifs(io);

      io <<  " -- CylindricalData: "  << std::scientific << std::endl;
      io <<  " zHalfShell  : " <<  d.zHalfShell  << std::endl;
      io <<  " gapShell  : " <<  d.gapShell  << std::endl;
      io <<  " rInnerShell  : " <<  d.rInnerShell  << std::endl;
      io <<  " rOuterShell  : " <<  d.rOuterShell  << std::endl;

      std::vector<CylindricalData::LayerLayout> layers = d.layers;

      io <<  " Layers : " << std::endl
         <<  "  phi0   zHalf   width  radiusSensitive thicknessSensitive radiusSupport thicknessSupport radialgap deltaphi" << std::endl;

      for(unsigned i=0,N=layers.size(); i<N; ++i){

	CylindricalData::LayerLayout l = layers[i];

        io << " " << l.phi0
	   << " " << l.zHalf
	   << " " << l.width
           << " " << l.radiusSensitive
           << " " << l.thicknessSensitive
	   << " " << l.radiusSupport
	   << " " << l.thicknessSupport
	   << " " << l.rgap
	   << " " << l.dphi
           << std::endl;
      }

      return io;
    };

    /** Simple data structure with key parameters for
     *  reconstruction of a composite of cylindrical and planar detector
     *
     * @author
     * @date Sep, 02 2024
     * @version $Id: $
     */
    struct CompositeStruct {
      ///  The half length (z) of the support shell (w/o gap) - 0. if no shell exists.
      double zHalfShell;
      ///  The length of the gap in mm (gap position at z=0).
      double  gapShell;
      ///  The inner radius of the support shell.
      double rInnerShell;
      ///  The outer radius of the support shell.
      double rOuterShell;

      std::vector<CylindricalStruct::LayerLayout> layersBent;
      std::vector<ZPlanarStruct::LayerLayout> layersPlanar;
    };
    typedef StructExtension<CompositeStruct> CompositeData;

    static std::ostream& operator<<( std::ostream& io , const CompositeData& d ) {
      boost::io::ios_base_all_saver ifs(io);

      io <<  " -- CompositeData: "  << std::scientific << std::endl;
      io <<  " zHalfShell  : " <<  d.zHalfShell  << std::endl;
      io <<  " gapShell  : " <<  d.gapShell  << std::endl;
      io <<  " rInnerShell  : " <<  d.rInnerShell  << std::endl;
      io <<  " rOuterShell  : " <<  d.rOuterShell  << std::endl;

      std::vector<CylindricalData::LayerLayout> layersBent = d.layersBent;

      io <<  " Bent Layers : " << std::endl
         <<  "  phi0   zHalf  width  radiusSensitive thicknessSensitive radiusSupport thicknessSupport radialgap deltaphi" << std::endl;

      for(unsigned i=0,N=layersBent.size(); i<N; ++i){
	CylindricalData::LayerLayout l = layersBent[i];

	io << " " << l.phi0
           << " " << l.zHalf
	   << " " << l.width
           << " " << l.radiusSensitive
           << " " << l.thicknessSensitive
           << " " << l.radiusSupport
           << " " << l.thicknessSupport
           << " " << l.rgap
           << " " << l.dphi
           << std::endl;
      }

      std::vector<ZPlanarData::LayerLayout> layersPlanar = d.layersPlanar;

      io <<  " Planar Layers : " << std::endl
         <<  "  nLadder phi0     nSensors    lengthSensor distSupport  thickSupport  offsetSupport widthSupport zHalfSupport distSense "
	 <<  " thickSense   offsetSense   widthSense  zHalfSense" << std::endl;

      for (unsigned i=0,N=layersPlanar.size(); i<N; ++i) {
        ZPlanarData::LayerLayout l = layersPlanar[i];

        io << " " << l.ladderNumber
           << " " << l.phi0
           << " " << l.sensorsPerLadder
           << " " << l.lengthSensor
           << " " << l.distanceSupport
           << " " << l.thicknessSupport
           << " " << l.offsetSupport
           << " " << l.widthSupport
           << " " << l.zHalfSupport
           << " " << l.distanceSensitive
           << " " << l.thicknessSensitive
           << " " << l.offsetSensitive
           << " " << l.widthSensitive
           << " " << l.zHalfSensitive
           << std::endl ;
      }

      return io;
    };

    /** Simple data structure with key parameters for
     *  zdisk by multi rings
     *
     * @author
     * @date Feb, 26, 2025
     * @version $Id: $
     */
    struct MultiRingsZDiskStruct {
      /// width of the strips (if applicable )
      double widthStrip;
      /// length of the strips (if applicable )
      double lengthStrip;
      /// strip pitch  (if applicable )
      double pitchStrip;
      /// strip stereo angle  (if applicable )
      double angleStrip;

      /// enum for encoding the sensor type in typeFlags
      struct SensorType{
	enum {
	  DoubleSided=0,
	  Pixel
	};
      };

      /** Internal helper struct for defining the layer layout. Layers are defined
       *  with a sensitive part, flex/cable/service part and a support part.
       */
      struct Ring {
	Ring() :
	  petalNumber(0),
	  sensorsPerPetal(0),
	  phi0(0),
	  phiOffsetOdd(0),
	  distance(0),
	  widthInner(0),
	  widthOuter(0),
	  length(0),
	  thicknessSensitive(0),
	  thicknessGlue(0),
	  thicknessService(0) {
	}

	int    petalNumber;
	int    sensorsPerPetal;
	double phi0;
	double phiOffsetOdd;
	double distance;
	double widthInner;
	double widthOuter;
	double length;
	double thicknessSensitive;
	double thicknessGlue;
	double thicknessService;
      };

      struct LayerLayout {
	LayerLayout() :
	  typeFlags(0),
	  alphaPetal(0),
	  zPosition(0),
	  zOffsetSupport(0),
	  rminSupport(0),
	  rmaxSupport(0),
	  thicknessSupport(0) {
	}
	std::bitset<32> typeFlags;
	double alphaPetal;
	double zPosition;
	double zOffsetSupport;
	double rminSupport;
	double rmaxSupport;
	double thicknessSupport;

	std::vector<Ring> rings;
      };

      std::vector<LayerLayout> layers;
    };
    typedef StructExtension<MultiRingsZDiskStruct> MultiRingsZDiskData;

    static std::ostream& operator<<( std::ostream& io , const MultiRingsZDiskData& d ) {
      boost::io::ios_base_all_saver ifs(io);

      io <<  " -- MultiRingsZDiskData: "  << std::scientific << std::endl;
      io <<  "  widthStrip  : " <<  d.widthStrip  << std::endl;
      io <<  "  lengthStrip : " <<  d.lengthStrip << std::endl;
      io <<  "  pitchStrip  : " <<  d.pitchStrip  << std::endl;
      io <<  "  angleStrip  : " <<  d.angleStrip  << std::endl;

      std::vector<MultiRingsZDiskData::LayerLayout> layers = d.layers;
      io << " Layers : alphaPetal  zPosition    d p   zOffsetSup  rminSupport  rmaxSupport thicknessSupport nRing" << std::endl;
      io << "               np    ns  phi0    phiOffsetOdd distance widthInner widthOuter length   thicknessSensitive thicknessGlue thicknessService" << std::endl;   

      for (unsigned i=0,N=layers.size(); i<N; ++i) {
	MultiRingsZDiskData::LayerLayout l = layers[i];

	std::vector<MultiRingsZDiskData::Ring> rings = l.rings;
	io << "        " << l.alphaPetal
	   << " " << l.zPosition
	   << " " << l.typeFlags[ MultiRingsZDiskData::SensorType::DoubleSided ]
	   << " " << l.typeFlags[ MultiRingsZDiskData::SensorType::Pixel ]
	   << " " << l.zOffsetSupport
	   << " " << l.rminSupport
	   << " " << l.rmaxSupport
	   << " " << l.thicknessSupport
	   << " " << rings.size()
	   << std::endl;

	for (unsigned ir=0,NR=rings.size(); ir<NR; ++ir) {
	  MultiRingsZDiskData::Ring r = rings[ir];

	  io << "               " << r.petalNumber
	     << " " << r.sensorsPerPetal
	     << " " << r.phi0
	     << " " << r.phiOffsetOdd
	     << " " << r.distance
	     << " " << r.widthInner
	     << " " << r.widthOuter
	     << " " << r.length
	     << " " << r.thicknessSensitive
	     << " " << r.thicknessGlue
	     << " " << r.thicknessService
	     << std::endl;
	}
      }

      return io;
    };

    /** Simple data structure with key parameters for
     *  reconstruction of long crystal bar ECAL
     *
     * @author Weizheng Song
     * @date Nov, 20, 2024
     * @version $Id: $
     */
    struct ECALModuleInfoStruct {
      int moduleNumber;
      int staveNumber;
      int partNumber;

      struct LayerInfo {
	LayerInfo() :
	  dlayerNumber(-1),
	  slayerNumber(-1),
	  barNumber(-1){
	}
	int dlayerNumber;
	int slayerNumber;
	int barNumber;
      };
      std::vector<LayerInfo> LayerInfos;
    };
    typedef StructExtension<ECALModuleInfoStruct> ECALModuleInfoData;

    struct ECALSystemInfoStruct {
      int systemNumber;

      std::vector<ECALModuleInfoStruct> ModuleInfos;
    };
    typedef StructExtension<ECALSystemInfoStruct> ECALSystemInfoData;
  }
}

#endif
