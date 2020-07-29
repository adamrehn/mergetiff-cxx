`mergetiff` GeoTiff Raster Merging Tool
=======================================

This is a native C++ port of my Python library [mergetiff](https://github.com/adamrehn/mergetiff). This implementation contains a header-only C++11 library and associated command-line tool called `mergetiff` that provides functionality to merge raster bands from multiple GeoTiff files into a single dataset. Metadata (including geospatial reference and projection data) will be copied from the first input dataset (when using the command-line tool) or from the dataset passed as the second argument to the `DatasetManagement::createMergedDataset()` function.

Command-line usage is identical to that of the Python version, see [the relevant section of the Python version's README](https://github.com/adamrehn/mergetiff#using-the-command-line-tool) for details.

The C++ version of the library also includes additional convenience functionality for working with the [C API entrypoints to the GDAL command-line utilities](https://gdal.org/api/gdal_utils.html), which are unnecessary in the Python version of the library due to the excellent SWIG bindings provided by the GDAL developers.


Contents
--------

- [Requirements](#requirements)
- [Building from source](#building-from-source)


Requirements
------------

- A modern, C++11-compliant compiler (Clang, GCC, Visual Studio 2015 or newer)
- [CMake](https://cmake.org/) 3.8 or newer
- GDAL 2.0 or newer


Building from source
---------------------------------------

To build, use the standard CMake invocation:

```
mkdir build && cd build
cmake ..
cmake --build .
```

Under Windows, it may be necessary to specify the location of the GDAL library and headers. This can be done through the `GDAL_DIR` variable:

```
mkdir build && cd build
cmake -A x64 -DGDAL_DIR="path/to/gdal" ..
cmake --build . --config Release
```
