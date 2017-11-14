#ifndef _MERGETIFF_DATASET_MANAGEMENT
#define _MERGETIFF_DATASET_MANAGEMENT

#include "DatatypeConversion.h"
#include "DriverOptions.h"
#include "GDALDatasetRef.h"
#include "RasterData.h"
#include "RasterIO.h"

#include <algorithm>
#include <stdexcept>
#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

namespace mergetiff {

class DatasetManagement
{
	public:
		
		//Opens a GDAL GeoTiff dataset
		static inline GDALDatasetRef openDataset(const string& filename)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Attempt to open the input dataset
			ArgsArray drivers({"GTiff"});
			ArgsArray options({"NUM_THREADS=ALL_CPUS"});
			ArgsArray siblings;
			GDALDataset* dataset = (GDALDataset*)(GDALOpenEx(filename.c_str(), GDAL_OF_RASTER | GDAL_OF_READONLY | GDAL_OF_VERBOSE_ERROR, drivers.get(), options.get(), siblings.get()));
			
			//Verify that we were able to open the dataset
			if (!dataset) {
				throw std::runtime_error("failed to open input dataset \"" + filename + "\"");
			}
			
			return GDALDatasetRef(dataset);
		}
		
		//Retrieves the specified raster bands of a GDAL dataset
		static inline vector<GDALRasterBand*> getRasterBands(GDALDatasetRef& dataset, const vector<unsigned int>& bandIndices)
		{
			//Verify that all of the requested band indices are valid
			unsigned int maxBand = *(std::max_element(bandIndices.begin(), bandIndices.end()));
			if (maxBand > (unsigned int)(dataset->GetRasterCount())) {
				throw std::runtime_error("invalid band index " + std::to_string(maxBand));
			}
			
			//Retrieve each of the requested bands
			vector<GDALRasterBand*> bands;
			for (auto index : bandIndices) {
				bands.push_back(dataset->GetRasterBand(index));
			}
			
			return bands;
		}
		
		//Retrieves all of the raster bands of a GDAL dataset
		static inline vector<GDALRasterBand*> getAllRasterBands(GDALDatasetRef& dataset)
		{
			vector<GDALRasterBand*> bands;
			for (int index = 1; index <= dataset->GetRasterCount(); ++index) {
				bands.push_back(dataset->GetRasterBand(index));
			}
			
			return bands;
		}
		
		//Opens a dataset and reads all of its raster data into a RasterData object
		template <typename PrimitiveTy>
		static inline RasterData<PrimitiveTy> rasterFromFile(const string& filename, const vector<unsigned int>& bands = vector<unsigned int>())
		{
			//Attempt to open the dataset
			GDALDatasetRef dataset = DatasetManagement::openDataset(filename);
			
			//Perform the raster I/O
			return DatasetManagement::rasterFromDataset<PrimitiveTy>(dataset, bands);
		}
		
		//Writes the raster data from a RasterData object to an image file
		template <typename PrimitiveTy>
		static inline GDALDatasetRef rasterToFile(const string& filename, const RasterData<PrimitiveTy>& data)
		{
			GDALDataType dtype = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			ArgsArray options = DriverOptions::geoTiffOptions(dtype);
			return DatasetManagement::datasetFromRaster(data, false, "GTiff", filename, options);
		}
		
		//Reads all of the raster data from a dataset into a RasterData object
		template <typename PrimitiveTy>
		static inline RasterData<PrimitiveTy> rasterFromDataset(GDALDatasetRef& dataset, const vector<unsigned int>& bands = vector<unsigned int>())
		{
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			return RasterIO::readDataset<PrimitiveTy>(dataset, expectedType, bands);
		}
		
		//Creates a GDAL dataset from the supplied raster data (defaults to an in-memory dataset containing a copy of the raster data)
		template <typename PrimitiveTy>
		static inline GDALDatasetRef datasetFromRaster(const RasterData<PrimitiveTy>& data, bool forceGrayInterp = false, const string& driver = "MEM", const string& filename = "", ArgsArray options = ArgsArray())
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Attempt to retrieve a reference to the requested GDAL driver
			GDALDriver* gdalDriver = ((GDALDriver*)GDALGetDriverByName(driver.c_str()));
			if (gdalDriver == nullptr) {
				throw std::runtime_error("failed to retrieve the GDAL \"" + driver + "\" driver handle");
			}
			
			//Attempt to create the output dataset
			GDALDataType gdalType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			GDALDataset* datasetPtr = gdalDriver->Create(
				filename.c_str(),
				data.cols(),
				data.rows(),
				data.channels(),
				gdalType,
				options.get()
			);
			
			//Verify that we were able to create the dataset
			if (datasetPtr == nullptr) {
				throw std::runtime_error("failed to create dataset from raster using \"" + driver + "\" driver");
			}
			
			//Attempt to copy the raster data into the dataset
			GDALDatasetRef dataset(datasetPtr);
			if (RasterIO::writeDataset(dataset, data) == false) {
				throw std::runtime_error("failed to copy raster data into dataset");
			}
			
			//Set the colour interpretation for each of the raster bands
			for (int index = 1; index < data.channels(); ++index)
			{
				GDALRasterBand* band = dataset->GetRasterBand(index+1);
				DatasetManagement::setColourInterpretation(band, index, data.channels(), forceGrayInterp);
			}
			
			return dataset;
		}
		
		//Creates a GDAL in-memory dataset that wraps the supplied raster data without copying it
		template <typename PrimitiveTy>
		static inline GDALDatasetRef wrapRasterData(const RasterData<PrimitiveTy>& data, bool forceGrayInterp = false)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Build the "filename" that will specify the options for the MEM driver
			GDALDataType dtype = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			string dtypeName = GDALGetDataTypeName(dtype);
			char ptrStrBuf[256];
			int ptrStrLen = CPLPrintPointer(ptrStrBuf, (void*)(data.getBuffer()), 256);
			string ptrStr = string(ptrStrBuf, ptrStrLen);
			string filename = string("MEM:::") +
				"DATAPOINTER=" + ptrStr +
				",PIXELS=" + std::to_string(data.cols()) +
				",LINES=" + std::to_string(data.rows()) +
				",BANDS=" + std::to_string(data.channels()) +
				",DATATYPE=" + dtypeName +
				",PIXELOFFSET=" + std::to_string(data.channels() * sizeof(PrimitiveTy)) +
				",LINEOFFSET=" + std::to_string(data.cols() * data.channels() * sizeof(PrimitiveTy)) +
				",BANDOFFSET=" + std::to_string(sizeof(PrimitiveTy));
			
			//Attempt to open the dataset
			GDALDataset* dataset = (GDALDataset*)(GDALOpen(filename.c_str(), GA_ReadOnly));
			
			//Verify that we were able to open the dataset
			if (dataset == nullptr) {
				throw std::runtime_error("failed to open in-memory dataset wrapping raster data");
			}
			
			//Set the colour interpretation for each of the raster bands
			for (int index = 1; index < data.channels(); ++index)
			{
				GDALRasterBand* band = dataset->GetRasterBand(index+1);
				DatasetManagement::setColourInterpretation(band, index, data.channels(), forceGrayInterp);
			}
			
			return GDALDatasetRef(dataset);
		}
		
		//Creates a merged dataset containing all of the supplied raster bands along with the metadata from the specified dataset
		template <typename PrimitiveTy>
		static inline GDALDatasetRef createMergedDatasetForType(const string& filename, GDALDatasetRef& metadataDataset, vector<GDALRasterBand*> rasterBands)
		{
			//Register all GDAL drivers
			GDALAllRegister();
			
			//Verify that all of the supplied raster bands have the correct datatype
			GDALDataType expectedType = DatatypeConversion::primitiveToGdal<PrimitiveTy>();
			for (auto band : rasterBands)
			{
				if (band->GetRasterDataType() != expectedType) {
					throw std::runtime_error("invalid datatype in one or more raster bands");
				}
			}
			
			//Attempt to retrieve a reference to the GeoTiff GDAL driver
			GDALDriver* tiffDriver = ((GDALDriver*)GDALGetDriverByName("GTiff"));
			if (tiffDriver == nullptr) {
				throw std::runtime_error("failed to retrieve the GDAL GeoTiff driver handle");
			}
			
			//Attempt to create the output dataset
			ArgsArray options = DriverOptions::geoTiffOptions(expectedType);
			GDALDataset* dataset = tiffDriver->Create(
				filename.c_str(),
				rasterBands[0]->GetXSize(),
				rasterBands[0]->GetYSize(),
				rasterBands.size(),
				expectedType,
				options.get()
			);
			
			//Verify that we were able to create the dataset
			if (dataset == nullptr) {
				throw std::runtime_error("failed to open output dataset \"" + filename + "\"");
			}
			
			//If a dataset was specified to copy metadata from, do so
			if (metadataDataset)
			{
				//Extract the list of metadata domains
				char** domains = metadataDataset->GetMetadataDomainList();
				
				//Check if there are any metadata domains
				if (domains == nullptr)
				{
					//No domains, simply copy the metadata for the default domain
					dataset->SetMetadata(metadataDataset->GetMetadata());
				}
				else
				{
					//Copy the metadata for each domain
					char** currDomain = domains;
					while (*currDomain != nullptr)
					{
						dataset->SetMetadata(metadataDataset->GetMetadata(*currDomain), *currDomain);
						currDomain++;
					}
					
					//Free the domain list
					CSLDestroy(domains);
				}
				
				//Copy projection
				dataset->SetProjection(metadataDataset->GetProjectionRef());
				
				//Copy affine GeoTransform
				double padfTransform[6];
				if (metadataDataset->GetGeoTransform(padfTransform) != CE_Failure) {
					dataset->SetGeoTransform(padfTransform);
				}
				
				//Copy GCPs
				if (metadataDataset->GetGCPCount() > 0)
				{
					dataset->SetGCPs(
						metadataDataset->GetGCPCount(),
						metadataDataset->GetGCPs(),
						metadataDataset->GetGCPProjection()
					);
				}
			}
			
			//Copy each of the input raster bands
			for (unsigned int index = 0; index < rasterBands.size(); ++index)
			{
				//Retrieve the input and output bands
				GDALRasterBand* inputBand = rasterBands[index];
				GDALRasterBand* outputBand = dataset->GetRasterBand(index+1);
				
				//Copy the band data
				RasterData<PrimitiveTy> bandData = RasterIO::readBand<PrimitiveTy>(inputBand, expectedType);
				RasterIO::writeBand(outputBand, bandData);
				
				//Copy the "no data" sentinel value, if any
				int hasNoDataValue = 0;
				double noDataValue = inputBand->GetNoDataValue(&hasNoDataValue);
				if (hasNoDataValue) {
					outputBand->SetNoDataValue(noDataValue);
				}
				
				//Copy the colour interpretation value, if any
				GDALColorInterp colourInterp = inputBand->GetColorInterpretation();
				if (colourInterp != GCI_Undefined) {
					outputBand->SetColorInterpretation(colourInterp);
				}
			}
			
			return GDALDatasetRef(dataset);
		}
		
		//Helper function for createMergedDatasetForType() to automatically provide the correct template argument
		static inline GDALDatasetRef createMergedDataset(const string& filename, GDALDatasetRef& metadataDataset, vector<GDALRasterBand*> rasterBands)
		{
			GDALDataType dtype = rasterBands[0]->GetRasterDataType();
			
			#define _CREATE_MERGED(GdalTy, PrimitiveTy) case GdalTy: return DatasetManagement::createMergedDatasetForType<PrimitiveTy>(filename, metadataDataset, rasterBands)
			switch (dtype)
			{
				_CREATE_MERGED(GDT_Byte,    uint8_t);
				_CREATE_MERGED(GDT_Int16,   int16_t);
				_CREATE_MERGED(GDT_UInt16,  uint16_t);
				_CREATE_MERGED(GDT_Int32,   int32_t);
				_CREATE_MERGED(GDT_UInt32,  uint32_t);
				_CREATE_MERGED(GDT_Float32, float);
				_CREATE_MERGED(GDT_Float64, double);
				
				default:
					throw std::runtime_error("unsupported GDAL datatype");
			}
			#undef _CREATE_MERGED
		}
		
	protected:
		
		//Helper function to set the colour interpretation for a raster band
		static inline void setColourInterpretation(GDALRasterBand* band, int bandIndex, int totalChannels, bool forceGrayInterp)
		{
			if (forceGrayInterp == false && totalChannels >= 3 && bandIndex <= 3)
			{
				static const vector<GDALColorInterp> interps = {
					GCI_RedBand,
					GCI_GreenBand,
					GCI_BlueBand,
					GCI_AlphaBand
				};
				
				band->SetColorInterpretation(interps[bandIndex]);
			}
			else
			{
				//Use grayscale for all bands if forceGrayInterp is true, as
				//well as all bands beyond the fourth raster band regardless
				band->SetColorInterpretation(GCI_GrayIndex);
			}
		}
};

} //End namespace mergetiff

#endif
