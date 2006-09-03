//***************************************************
// DFDCGeometry - temporary geometry class for FDC.
// Author: Craig Bookwalter (craigb at jlab.org)
// Date:   March 2006
//***************************************************

#ifndef DFDCGEOMETRY_H
#define DFDCGEOMETRY_H

#define WIRES_PER_PLANE       121
#define WIRE_SPACING          1.0
#define U_OF_WIRE_ZERO        (-(WIRES_PER_PLANE-1.)*WIRE_SPACING/2)

#include <math.h>

#include <JANA/JObject.h>
#include "DFDCHit.h"

///
/// class DFDCGeometry: definition of a class providing basic geometry
/// methods for FDC reconstruction.
///
class DFDCGeometry : public JObject {
	public:
		///
		/// DFDCGeometry::gLayer():
		/// Compute the global layer (detection layer 1-24) number based on module and layer
		///
		static inline int gLayer(const DFDCHit* h) {
			return 3*(h->module - 1) + (h->layer - 1) + 1;
		}		
		
		///
		/// DFDCGeometry::gPlane():
		/// Compute the global plane (1-74) number based on module, layer, and plane
		///
		static inline int gPlane(const DFDCHit* h) {
			return 9*(h->module-1) + 3*(h->layer-1) + (h->plane-1) + 1;
		}
		
		/// 
		/// DFDCGeometry::getLayerZ():
		/// Get the Z position of a layer
		///
		static float getLayerZ(const DFDCHit* h) {
			if (h->gPlane <= 18)  
				return 227.5 + h->gPlane*0.5;			// Thing 1
			if (h->gPlane <= 36)	   
				return 283.5 + (h->gPlane - 18)*0.5;	// Thing 2
			if (h->gPlane <= 54)	   
				return 358.5 + (h->gPlane - 36)*0.5;	// Thing 3
			return 394.5 + (h->gPlane - 540*0.5); 		// Thing 4
		}
		
		///		
		/// DFDCGeometry::getWireR():
		/// Return X coordinate of a wire
		/// 
		static inline float getWireR(const DFDCHit* h) {
		  return WIRE_SPACING*(h->element-0.5)+U_OF_WIRE_ZERO;
	  
		}
		
		///
		/// DFDCGeometry::getStripR():
		/// Return coordinate in U or V space of a strip
		///
		static inline float getStripR(const DFDCHit* h) {
			return (h->element - 119.5)*0.5;
		}

		///
		/// DFDCGeometry::getXLocalStrips()
		///  Compute the x-coordinate in the layer coordinate system
		/// from the strip data.
		///
		static inline float getXLocalStrips(float u, float v){
		  return 0.5*(u+v-240.0)/sqrt(2.);
		}

		///
		/// DFDCGeometry::getYLocalStrips()
		///  Compute the y-coordinate in the layer coordinate system
		/// from the strip data
		///
		static inline float getYLocalStrips(float u, float v){
		  return  0.5*(u-v)/sqrt(2.);
		} 
	
		///
		/// DFDCGeometry::getLayerRotation():
		/// Compute the rotation of a detection layer (0, 60, -60)
		///
		static inline float getLayerRotation(const int gLayer) {
			switch ((gLayer-1) % 3) {
				case 0:
					return 0.0;
				case 1:
					return 3.1415926 / 3;
				case 2:
					return -3.1415926 / 3;
				return 0.0;
			}
		}
};

#endif // DFDCGEOMETRY_H
