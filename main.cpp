
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <H5Cpp.h>

#include <string>

#include "cv_hdf.hpp"


/**
 *
 * Open file HDF file and dataset group.
 * Show images in group and allow navigation with trackbar.
 * Works with 3D data, not tested on higher-dimensional data yet.
 *
 * Usage:
 * Launch via commandline.
 * First commandline parameter points to file
 * Second parameter denotes dataset/ group
 * Third parameter (optional) is destination to an output file. The last displayed file will be written into output.
 *
 */

cv::Mat src, dst;
H5::DataSet dataset;
const double SCALE = 0.25;

void onTrackbar(int slice, void*)
{
	cv::simpleHDF::readSlice(dataset, slice, src);

	// rescale pixel values for display
	double min, max;
	cv::minMaxIdx(src, &min, &max);

	src -= min;
	src *= (255. / max);

	//src = cv::min(src, 255.);
	//src = cv::max(src, 0.);
	src.convertTo(dst, CV_8UC1);

	int width = src.size().width;
	int height = src.size().height;
	cv::resize(dst, dst, cv::Size((int)(width * SCALE), (int)(height * SCALE)));	

	// here are some sample lines...
	//cv::medianBlur(dst, dst, 5);
	//cv::equalizeHist(dst, dst);

	cv::imshow("HDF", dst);
}


void dumpSlices(H5::DataSet& dataset, const uint startslice = 0, const uint endslice = 10){
	cv::Mat dst;
	std::string outfile;
	for (uint i = startslice; i < endslice; ++i){
		outfile = "spim_" + std::to_string(i) + ".png";
		cv::simpleHDF::readSlice(dataset, i, dst);
		dst = cv::min(dst, 255.);
		dst = cv::max(dst, 0.);
		dst.convertTo(dst, CV_8UC1);
		cv::imwrite(outfile.c_str(), dst);
		printf("dumping %s\n", outfile.c_str());
	}
}


int main(int argc, char** argv)
{
	#ifdef DEBUG
		H5::Exception::dontPrint();
	#endif

	if (argc < 3){
		printf("Please provide a file and a group name\n");
		printf("Exiting...\n");
		return 0;
	}

	const char *FILE_NAME = argv[1];
	const char *DATASET_NAME = argv[2];
	const char *OUT_FILE = NULL;

	if (argc == 4){
		printf("The last displayed frame will be dumped\n");
		printf("Location will be:\n%s\n", argv[3]);
		OUT_FILE = argv[3];
	}

	// trackbar variables
	int slice = 0;
	int maxSlice = 0;

	// H5 initialization
	H5::H5File file(FILE_NAME, H5F_ACC_RDONLY);
	dataset = file.openDataSet(DATASET_NAME);

	// getting dataset dimensions
	H5::DataSpace dataspace = dataset.getSpace();
	int dimCount = dataspace.getSimpleExtentNdims();
	hsize_t *dims = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
	hsize_t *maxsize = (hsize_t *)malloc(dimCount * sizeof(hsize_t));
	dataspace.getSimpleExtentDims(dims, maxsize);
	maxSlice = (int)dims[0]-1;

	dst = cv::Mat(src.size(), CV_8UC1);
	cv::namedWindow("HDF");
	cv::createTrackbar("Slice", "HDF", &slice, maxSlice, onTrackbar);	
	onTrackbar(slice, 0);
	cv::waitKey(0);

	//dumpSlices(dataset, 0, 50);

	dataset.close();
	file.close();

	free(dims);
	free(maxsize);

	if (OUT_FILE) {
		cv::imwrite(OUT_FILE, src);
	}

	return 0;
}
