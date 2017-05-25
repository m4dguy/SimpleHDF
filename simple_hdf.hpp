
#include <H5Cpp.h>

#ifndef SIMPLE_HDF_HPP
#define SIMPLE_HDF_HPP

#define IMGRANK 2
#define uint unsigned int

/**
 * Lightweight header for reading HDF slices.
 * Light on dependencies (only H5Cpp.h required), vegan and gluten-free.
 */
namespace simpleHDF{
	/**
	 * Struct holding single HDF slice.
	 */
	struct HDFSlice{
		// raw data
		void* data;
		// row length
		size_t width;
		// column length
		size_t height;
		// size in bytes of each element
		size_t elementSize;
	};
	
	/**
	 * Get size of single element in DataSet in bytes.
	 * To be used for memory allocation.
	 * @param dataset DataSet to check.
	 * @return Size of single DataSet element.
	 */
	size_t elementSize(H5::DataSet& dataset){
		size_t bits = 0;
		size_t bitdepth = 0;
		H5::DataType type = dataset.getDataType();
		bitdepth = type.getSize();

		H5T_class_t dataType = dataset.getTypeClass();
		switch (dataType){
		case H5T_INTEGER:
			bits = sizeof(char);
			break;
		case H5T_FLOAT:
			bits = sizeof(float);
			break;
		default:
			return -1;
		}
		return bits * bitdepth;
	}

	/**
	 * Create buffer struct for slice reading from given dataset.
	 * @param dataset DataSet to read from.
	 * @param widthDimension Data space dimension for travering rows.
	 * @param heightDimension Data space dimension for traversing columns.
	 * @return Fully initialized HDFSlice struct for immediate use with @readSlice.
	 */
	HDFSlice createBuffer(H5::DataSet& dataset, const uint widthDimension = 1, const uint heightDimension = 2){
		HDFSlice slice;
		try{
			H5::DataSpace dataspace = dataset.getSpace();

			int dimCount = dataspace.getSimpleExtentNdims();
			hsize_t *dims = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
			hsize_t *maxsize = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
			dataspace.getSimpleExtentDims(dims, maxsize);
			
			slice.width = (size_t)dims[widthDimension];
			slice.height = (size_t)dims[heightDimension];
			slice.elementSize = elementSize(dataset);
			slice.data = malloc(slice.width * slice.height * slice.elementSize);

			if (!slice.data){
				printf("unable to allocate buffer memory");
				return slice;
			}
		}
		catch (H5::FileIException error){
			error.printError();
		}
		catch (H5::DataSetIException error){
			error.printError();
		}
		catch (H5::DataSpaceIException error){
			error.printError();
		}
		catch (H5::DataTypeIException error){
			error.printError();
		}
		return slice;
	}

	/**
	 * Read single slice from image stack into buffer.
	 * @param dataset DataSet to read from.
	 * @param target Target HDFSlice to read data into. Must be fully initialized before reading.
	 * @param slice Index of slice to read.
	 * @param sliceDimension Data space dimension for traversing slices.
	 * @param widthDimension Data space dimension for travering rows.
	 * @param heightDimension Data space dimension for traversing columns.
	 */
	void readSlice(H5::DataSet& dataset, HDFSlice& target, const int slice, const uint sliceDimension = 0, const uint widthDimension = 1, const uint heightDimension = 2){
		try{
			H5::DataSpace dataspace = dataset.getSpace();

			int dimCount = dataspace.getSimpleExtentNdims();
			hsize_t* dims = (hsize_t*) malloc(dimCount * sizeof(hsize_t));
			hsize_t* maxsize = (hsize_t*) malloc(dimCount * sizeof(hsize_t));
			dataspace.getSimpleExtentDims(dims, maxsize);

			hsize_t* offset = (hsize_t*)calloc(dimCount, sizeof(hsize_t));
			offset[sliceDimension] = slice;
			hsize_t* count = (hsize_t*)malloc(dimCount * sizeof(hsize_t));
			count[sliceDimension] = 1;
			count[widthDimension] = target.width;
			count[heightDimension] = target.height;
			dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

			hsize_t* offset_out = (hsize_t*) calloc(IMGRANK, sizeof(hsize_t));
			hsize_t* count_out = (hsize_t*) malloc(IMGRANK * sizeof(hsize_t));
			count_out[0] = target.width;
			count_out[1] = target.height;

			H5::DataSpace memspace(IMGRANK, count_out);
			memspace.selectHyperslab(H5S_SELECT_SET, count_out, offset_out);

			dataset.read(target.data, H5::PredType::NATIVE_INT16, memspace, dataspace);
			dataspace.close();
			
			free(dims);
			free(maxsize);

			free(offset);
			free(count);

			free(offset_out);
			free(count_out);
		}
		catch (H5::FileIException error){
			error.printError();
		}
		catch (H5::DataSetIException error){
			error.printError();
		}
		catch (H5::DataSpaceIException error){
			error.printError();
		}
		catch (H5::DataTypeIException error){
			error.printError();
		}
	}
}
#endif
