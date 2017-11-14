#include "../lib/DatasetManagement.h"
#include "../lib/Utility.h"
using mergetiff::DatasetManagement;
using mergetiff::GDALDatasetRef;
using mergetiff::Utility;

#include <string>
#include <vector>
#include <iostream>
using std::string;
using std::vector;
using std::clog;
using std::endl;

int main (int argc, char* argv[])
{
	try
	{
		//Check that the required command-line arguments have been supplied
		if (argc > 3 && argc % 2 == 0)
		{
			string outputFile = argv[1];
			vector<GDALDatasetRef> datasets;
			vector<GDALRasterBand*> bands;
			
			//Iterate over each of the input datasets
			for (int i = 2; i < argc; i += 2)
			{
				//Attempt to open the dataset
				datasets.emplace_back(DatasetManagement::openDataset(argv[i]));
				
				//Determine if we are including any of the bands from the current dataset
				string bandStr = argv[i+1];
				if (bandStr != "-")
				{
					try
					{
						//Determine which bands have been requested
						vector<string> bandStrs = Utility::strSplit(bandStr, ",");
						vector<unsigned int> bandIndices;
						for (auto band : bandStrs) {
							bandIndices.push_back(std::stoi(band));
						}
						
						//Attempt to retrieve the bands and add them to our list
						vector<GDALRasterBand*> datasetBands = DatasetManagement::getRasterBands(datasets.back(), bandIndices);
						bands.insert(bands.end(), datasetBands.begin(), datasetBands.end());
					}
					catch (std::invalid_argument&) {
						throw std::runtime_error("invalid band specifier string");
					}
				}
			}
			
			//Attempt to create the merged dataset
			DatasetManagement::createMergedDataset(outputFile, datasets[0], bands);
			clog << "Created merged dataset \"" << outputFile << "\"." << endl;
		}
		else
		{
			clog << "Usage:" << endl;
			clog << "mergetiff <OUT.TIF> <IN1.TIF> <BAND1,BAND2,BAND3> [<IN2.TIF> <BAND1,BAND2,BAND3>]" << endl;
		}
		
		return 0;
	}
	catch (std::runtime_error& e)
	{
		clog << "Error: " << e.what() << endl;
		return 1;
	}
}
