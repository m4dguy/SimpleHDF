
#include <H5Cpp.h>

#define IMGRANK 2
#define uint unsigned int

/**
 * Lightweight header for reading HDF slices.
 * Light on dependencies (only H5Cpp.h required), vegan and gluten-free.
 */
namespace simpleHDF{
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
		size_t bitdepth = type.getSize();

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
	 * Read single slice from image stack into buffer.
	 * @param dataset DataSet to read from.
	 * @param slice Index of slice to read.
	 * @return Pointer to read data (uint16_t). NULL if reading fails.
	 */
	void *readSlice(H5::DataSet& dataset, const int slice, const uint sliceDimension=0, const uint widthDimension=1, const uint heightDimension=2){
		void *buffer = NULL;

		try{
			H5::DataSpace dataspace = dataset.getSpace();

			int dimCount = dataspace.getSimpleExtentNdims();
			hsize_t *dims = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
			hsize_t *maxsize = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
			dataspace.getSimpleExtentDims(dims, maxsize);
			const int width = (int)dims[widthDimension];
			const int height = (int)dims[heightDimension];

			buffer = malloc(width * height * elementSize(dataset));
			if (!buffer){
				printf("unable to allocate buffer memory");
				return NULL;
			}

			hsize_t *offset = (hsize_t*)calloc(dimCount, sizeof(hsize_t));
			offset[sliceDimension] = slice;
			hsize_t *count = (hsize_t*)malloc(dimCount * sizeof(hsize_t));
			count[sliceDimension] = 1;
			count[widthDimension] = width;
			count[heightDimension] = height;
			dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

			hsize_t *offset_out = (hsize_t*)calloc(IMGRANK, sizeof(hsize_t));
			hsize_t *count_out = (hsize_t*)malloc(IMGRANK * sizeof(hsize_t));
			count_out[0] = width;
			count_out[1] = height;

			H5::DataSpace memspace(IMGRANK, count_out);
			memspace.selectHyperslab(H5S_SELECT_SET, count_out, offset_out);

			dataset.read(buffer, H5::PredType::NATIVE_INT16, memspace, dataspace);
			dataspace.close();
			
			free(dims);
			free(maxsize);

			free(offset);
			free(count);

			free(offset_out);
			free(count_out);

			return buffer;
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
		return buffer;
	}
}