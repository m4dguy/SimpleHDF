
#include <vector>
#include <string>

#include <H5Cpp.h>

/**
 * Stupid utility stuff for getting H5 file and dataspace information.
 * Single header without other dependencies (except H5Cpp.h).
 */
namespace simpleHDF{
	/**
	 * Container for DataSpace dimensions.
	 * Remember to manually delete the "sizes" member to avoid memory leaks!
	 */
	struct SpaceDimensions{
		unsigned int dimensions = 0;
		std::vector<hsize_t> sizes;
	};

	/**
	 * Retrieve string representation of a dataset's datatype.
	 * @param: dataset Input DataSet.
	 * @param: std::string with text representation of object type class.
	 */
	std::string getDatasetType(H5::DataSet& dataset){
		H5T_class_t typeclass = dataset.getTypeClass();
		switch (typeclass){
			case H5T_ARRAY:
				return std::string("H5T_ARRAY");
			case H5T_INTEGER: 
				return std::string("H5T_INTEGER");
			case H5T_FLOAT: 
				return std::string("H5T_FLOAT");
			case H5T_STRING:
				return std::string("H5T_STRING");
			default: 
				return std::string("UNKNOWN");
		}		
	}

	/**
	 * Operator function for getAttributes().
	 */
	void attrOperator(H5::H5Location&, std::string attrName, void* opdata){
		std::vector<std::string> *vec = (std::vector<std::string> *) opdata;
		vec->push_back(attrName);
	}

	/**
	 * Retrieve a list of the DataSet's attribute names.
	 * @param loc H5Location from which attributes need be retrieved.
	 * @return std::vector containing the DataSet's attribute name.
	 */
	std::vector<std::string> getAttributes(H5::H5Location& loc){
		uint i = 0;
		herr_t err = 0;
		std::vector<std::string> attributes;
		attributes.reserve(loc.getNumAttrs());
		loc.iterateAttrs(attrOperator, &i, &attributes);
		return attributes;
	}

	/**
	 * Get rank and each single dataspace dimensions.
	 * @param space DataSpace to check for info.
	 */ 
	SpaceDimensions getDataSpaceDimensions(H5::DataSpace& space){
		SpaceDimensions sd;
		sd.dimensions = space.getSimpleExtentNdims();
		sd.sizes.resize(sd.dimensions);
		space.getSimpleExtentDims(sd.sizes.data());
		return sd;
	}

}
