#ifndef _MERGETIFF_GDAL_DATASET_REF
#define _MERGETIFF_GDAL_DATASET_REF

#include <memory>
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
typedef std::unique_ptr<GDALDataset, GDALDatasetDeleter> GDALDatasetRef;

} //End namespace mergetiff

#endif
