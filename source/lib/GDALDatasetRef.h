#ifndef _MERGETIFF_GDAL_DATASET_REF
#define _MERGETIFF_GDAL_DATASET_REF

#include "LibrarySettings.h"

#include <gdal.h>
#include <gdal_priv.h>

namespace mergetiff {

//Frees a GDAL dataset handle by calling GDALClose() on it
class GDALDatasetDeleter
{
	public:
		inline void operator()(GDALDataset* d) {
			GDALClose(d);
		}
};

//Smart pointer type for GDAL datasets
typedef MERGETIFF_SMART_POINTER_TYPE<GDALDataset, GDALDatasetDeleter> GDALDatasetRef;

} //End namespace mergetiff

#endif
