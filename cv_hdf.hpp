
#include <opencv2/core/core.hpp>

#include <H5Cpp.h>

#ifndef SIMPLE_HDF_CV_HPP
#define SIMPLE_HDF_CV_HPP

#define IMGRANK 2
#define uint unsigned int

/**
 * Utility stuff for reading SPIM data into OpenCV's Mat datastructure.
 * Depends on OpenCV and H5.
 */
namespace cv{
	namespace simpleHDF{
		/**
		 * Get OpenCV datatype corresponding to dataset's datatype or else -1.
		 * @param dataset Dataset to check.
		 * @return CV_Type of DataSet or else -1.
		 */
		int getCvType(H5::DataSet& dataset){
			int typesize, channels;
			H5T_sign_t signtype;

			std::string subclass("");
			if (dataset.attrExists("IMAGE_SUBCLASS")){
				H5::Attribute imgsubclass = dataset.openAttribute("IMAGE_SUBCLASS");
				imgsubclass.read(imgsubclass.getDataType(), subclass);
			} else{
				return -1;
			}

			channels = (subclass == "IMAGE_GRAYSCALE")? 1 : 3;
			switch (dataset.getTypeClass()){
				case H5T_INTEGER:
					typesize = dataset.getIntType().getSize();
					signtype = dataset.getIntType().getSign();
					switch (signtype){
						case H5T_SGN_2:
							switch (typesize){
								case 1:
									return CV_MAKETYPE(CV_8S, channels);
								case 2:
									return CV_MAKETYPE(CV_16S, channels);
								case 4:
									return CV_MAKETYPE(CV_32S, channels);
								default:
									return -1;
							}
						case H5T_SGN_NONE:
							switch (typesize){
								case 1:
									return CV_MAKETYPE(CV_8U, channels);
								case 2:
									return CV_MAKETYPE(CV_16U, channels);
								default:
									return -1;
								}
						default:
							return -1;
					}					
				case H5T_FLOAT:
					typesize = dataset.getFloatType().getSize();
					switch (typesize){
						case 1:
							return CV_MAKETYPE(CV_32F, channels);
						case 2:
							return CV_MAKETYPE(CV_64F, channels);
						default:
							return -1;
					}
				default:
					return -1;
			}
		}

		/**
		 * Read single slice from image stack into cv::Mat.
		 * @param dataset Target dataset to read from.
		 * @param slice Index of slice to read.
		 * @param dst cv::Mat to read into.
		 * @return Error code. 0 if everything went well, -1 else.
		 */
		int readSlice(H5::DataSet& dataset, const int slice, cv::Mat& dst, const uint sliceDimension=0, const uint widthDimension=1, const uint heightDimension=2){
			try{
				H5::DataSpace dataspace = dataset.getSpace();

				// get dataspace dimensions (needed for buffer creation)
				int dimCount = dataspace.getSimpleExtentNdims();
				hsize_t *dims = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
				hsize_t *maxsize = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
				dataspace.getSimpleExtentDims(dims, maxsize);
				const int width = (int)dims[widthDimension];
				const int height = (int)dims[heightDimension];
				const int type = getCvType(dataset);

				//dynamically adapt size of cv::Mat to match slice size
				if ((dst.size().width != width) || (dst.size().height != height) || (dst.type() != type)){
					dst.release();
					dst = cv::Mat(cv::Size(width, height), type);
				}

				void *buffer = (void*)dst.data;

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

				free(count);
				free(offset);

				free(count_out);
				free(offset_out);

				return 0;
			}
			catch (H5::FileIException error){
				error.printError();
				return -1;
			}
			catch (H5::DataSetIException error)
			{
				error.printError();
				return -1;
			}
			catch (H5::DataSpaceIException error){
				error.printError();
				return -1;
			}
			catch (H5::DataTypeIException error){
				error.printError();
				return -1;
			}
		}
	}
}
#endif
