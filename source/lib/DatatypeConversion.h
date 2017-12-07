#ifndef _MERGETIFF_DATATYPE_CONVERSION
#define _MERGETIFF_DATATYPE_CONVERSION

#include <gdal.h>
#include <stdint.h>
#include <stdexcept>

namespace mergetiff {
namespace DatatypeConversion {

template <typename PrimitiveTy>
inline GDALDataType primitiveToGdal() {
	throw std::runtime_error("unsupported primitive type");
}

#define _P2G_SPECIALISATION(PrimitiveTy, GdalTy) template<> GDALDataType primitiveToGdal<PrimitiveTy>() { return GdalTy; }
_P2G_SPECIALISATION(uint8_t,  GDT_Byte)
_P2G_SPECIALISATION(uint16_t, GDT_UInt16)
_P2G_SPECIALISATION(int16_t,  GDT_Int16)
_P2G_SPECIALISATION(uint32_t, GDT_UInt32)
_P2G_SPECIALISATION(int32_t,  GDT_Int32)
_P2G_SPECIALISATION(float,    GDT_Float32)
_P2G_SPECIALISATION(double,   GDT_Float64)

} //End namespace DatatypeConversion
} //End namespace mergetiff

#endif
