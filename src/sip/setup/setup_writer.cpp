/** phase1_writer.cpp
 *
 *  Created on: Jun 23, 2013
 *      Author: Bev
 */

//TODO  Memory leak on array allocated in addSegmentInfowq


#include "setup_writer.h"
#include "json/json.h"
#include <algorithm>
#include <assert.h>
#include <cctype>

namespace setup {

// Debugging Method
void printArray(std::string prefix, std::string name, int rank, int *dims, double *data){
	// Debug
	std :: cout << prefix << name << ", (" << dims[0];
	for (int i=1; i<rank ; i++){
		std :: cout << ", " << dims[i];
	}
	int num_elems = 1;
	for (int i=0; i<rank; i++){
		num_elems *= dims[i];
	}
	std :: cout << "), [" << num_elems << "], {" << data[0];
	for (int i=1; i<num_elems; i++){
		std :: cout << ", " << data[i];
	}
	std :: cout <<"}" << std :: endl;
}


SetupWriter::SetupWriter(std::string jobname, OutputStream* file):
		jobname_(jobname),
		file(file) {
}

SetupWriter::~SetupWriter(){
    for (SegSizeArrayIterator iter = segments_.begin(); iter != segments_.end(); ++iter){
    	delete [] iter->second.second;
    }
    for (PredefIntArrayIterator iter = predef_int_arr_.begin(); iter != predef_int_arr_.end(); ++iter){
    	delete [] iter->second.second.first;
    	delete [] iter->second.second.second;
    }
    for (PredefArrayIterator iter = predef_arr_.begin(); iter != predef_arr_.end(); ++iter){
    	delete [] iter->second.second.first;
    	delete [] iter->second.second.second;
    }
	delete file;
}

void SetupWriter::addPredefinedIntHeader(std::string name, int val) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (header_constants_.count(name)){
		std::cerr << "Duplicate predefined int " << name << std::endl;
	}
	header_constants_[name] = val;
}

void SetupWriter::addPredefinedIntData(std::string name, int val) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (data_constants_.count(name)){
		std::cerr << "Duplicate predefined int " << name << std::endl;
	}
	if (header_constants_.count(name)){
	    std::cerr << "Duplicate predefined int " << name << std::endl;
	}
	data_constants_[name] = val;
}

void SetupWriter::addSialProgram(std::string name) {
	sial_programs_.push_back(name);
}

void SetupWriter::addSegmentInfo(sip::IndexType_t index_type_num, int num_segments, int * segment_sizes){
	if (segments_.count(index_type_num)){
		std::cerr << "Duplicate segments array for " << index_type_num << std::endl;
	}
	// Put copy of array into table
	int * alloc_segment_sizes = new int[num_segments];
	std::copy(segment_sizes, segment_sizes+num_segments, alloc_segment_sizes);

	segments_[index_type_num] = std::pair<int, int*>(num_segments, alloc_segment_sizes);
}

void SetupWriter::addPredefinedScalar(std::string name, double value){
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (scalars_.count(name)){
		std::cerr << "Duplicate predefined scalar " << name << std::endl;
	}
	scalars_[name] = value;
}


void SetupWriter::addPredefinedContiguousArray(std::string name, int rank, int * dims,
			double * data){

	//printArray("before adding to DS", name, rank, dims, data);

	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (predef_arr_.count(name)){
		std::cerr << "Duplicate predefined array " << name << std::endl;
	}
	// Put copy of array into table
	int num_elems = 1;
	for (int i=0; i<rank; i++){
		num_elems *= dims[i];
	}
	double * alloc_data = new double[num_elems];
	std::copy(data, data+num_elems, alloc_data);
	int * alloc_dims = new int[rank];
	std::copy(dims, dims+rank, alloc_dims);

	std::pair<int *, double *> dataPair = std::pair<int *, double *>(alloc_dims, alloc_data);
	predef_arr_[name] = std::pair<int, std::pair<int *, double *> >(rank, dataPair);


	//printArray("After adding to DS", name, predef_arr_[name].first, predef_arr_[name].second.first, predef_arr_[name].second.second);

}

void SetupWriter::addPredefinedIntegerContiguousArray(std::string name, int rank, int * dims,
			int * data){

	//printArray("before adding to DS", name, rank, dims, data);

	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	if (predef_arr_.count(name)){
		std::cerr << "Duplicate predefined array " << name << std::endl;
	}
	// Put copy of array into table
	int num_elems = 1;
	for (int i=0; i<rank; i++){
		num_elems *= dims[i];
	}
	int * alloc_data = new int[num_elems];
	std::copy(data, data+num_elems, alloc_data);
	int * alloc_dims = new int[rank];
	std::copy(dims, dims+rank, alloc_dims);

	std::pair<int *, int *> dataPair = std::pair<int *, int *>(alloc_dims, alloc_data);
	predef_int_arr_[name] = std::pair<int, std::pair<int *, int *> >(rank, dataPair);


	//printArray("After adding to DS", name, predef_arr_[name].first, predef_arr_[name].second.first, predef_arr_[name].second.second);

}


void SetupWriter::addSialFileConfigInfo(std::string sialfile, std::string key, std::string value){
	std::transform(sialfile.begin(), sialfile.end(), sialfile.begin(), ::tolower);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (configs_.count(sialfile) == 0){
		std::map<std::string, std::string> configMap;
		configMap[key] = value;
		configs_[sialfile] = configMap;
	} else {
		std::map<std::string, std::string> &configMap = configs_[sialfile];
		configMap[key] = value;
		configs_[sialfile] = configMap;
	}
}


//TODO  add timestamp
void SetupWriter::write_header_file(){
	//write predefined ints to header file
	std::ofstream header;
	std::string header_name = jobname_ + ".h";
	header.open(header_name.data(), std::ios::trunc);
	header << "#ifndef SETUP_HEADER_H_" << std::endl;
	header << "#define SETUP_HEADER_H_" << std::endl;
	for (PredefInt::iterator it=header_constants_.begin();
			it!= header_constants_.end(); ++it){
		 header << "#define " << it->first << " " << it->second << std::endl;
	}
	header << "#endif //SETUP_HEADER_H_" << std::endl;
	header.close();
}

void SetupWriter::write_data_file() {
	file -> write_int(sip::SETUP_MAGIC);
	file -> write_int(sip::SETUP_VERSION);
	//write sial program names
	int num_sial_programs = sial_programs_.size();
	file -> write_int(num_sial_programs);
	for (SialProg::iterator it=sial_programs_.begin();
			it!= sial_programs_.end(); ++it){
		 file -> write_string(*it);
	}
	//write predefined ints
	int nints = data_constants_.size();
	file->write_int(nints);
	for (PredefInt::iterator it=data_constants_.begin();
			it!= data_constants_.end(); ++it){
		 file -> write_string(it->first);
		 file -> write_int(it->second);
	}
	//write predefined scalars
	int nscalars = scalars_.size();
	file->write_int(nscalars);
	for (PredefScalar::iterator it=scalars_.begin();
			it!= scalars_.end(); ++it){
		 file -> write_string(it->first);
		 file -> write_double(it->second);
	}
	//write segment info
    int num_segment_size_arrays = segments_.size();
	file->write_int(num_segment_size_arrays);
	for (SegSizeArray::iterator it=segments_.begin();
			it!= segments_.end(); ++it){
		 file -> write_int(it->first);  // index type
		 int nsegs = it->second.first; //num_segments
		 file -> write_int_array(nsegs, it->second.second); //array of seg sizes
	}
	//write predefined contiguous array
	int num_predef_arrays = predef_arr_.size();
	file->write_int(num_predef_arrays);
	for (PredefArrMap::iterator it = predef_arr_.begin(); it != predef_arr_.end(); ++it){
		// Array Name
		file->write_string(it->first);
		// Array Rank
		int array_rank = it->second.first;
		file->write_int(array_rank);
		// Array Dimensions
		int * array_dims = it->second.second.first;
		file->write_int_array(array_rank, array_dims);
		// Array Data
		int num_data_elems = 1;
		for (int i=0; i<array_rank; i++){
			num_data_elems *= array_dims[i];
		}
		double * array_data = it->second.second.second;
		file->write_double_array(num_data_elems, array_data);

		//printArray("To file", it->first, array_rank,array_dims, array_data);

	}

    // Write integer arrays
	int num_predef_integer_arrays = predef_int_arr_.size();
	file->write_int(num_predef_integer_arrays);
	for (PredefIntArrMap::iterator it = predef_int_arr_.begin(); it != predef_int_arr_.end(); ++it){
		// Array Name
		file->write_string(it->first);
		// Array Rank
		int array_rank = it->second.first;
		file->write_int(array_rank);
		// Array Dimensions
		int * array_dims = it->second.second.first;
		file->write_int_array(array_rank, array_dims);
		// Array Data
		int num_data_elems = 1;
		for (int i=0; i<array_rank; i++){
			num_data_elems *= array_dims[i];
		}
		int * array_data = it->second.second.second;
		file->write_int_array(num_data_elems, array_data);

		//printArray("To file", it->first, array_rank,array_dims, array_data);

	}
	// Write configuration per sial file
	int num_sialfile_configs = configs_.size();
	file->write_int(num_sialfile_configs);
	for (FileConfigMap::iterator it = configs_.begin(); it != configs_.end(); ++it){
		// Sial file name
		file->write_string(it->first);
		// Key value map
		std::map<std::string, std::string> &kvConfig = it->second;
		int num_config_elems = kvConfig.size();
		file->write_int(num_config_elems);
		for (KeyValueMap::iterator it2 = kvConfig.begin(); it2 != kvConfig.end(); ++it2){
			file->write_string(it2->first);
			file->write_string(it2->second);
		}
	}
}


// TODO Test
std::string SetupWriter::get_json_string(){
	Json::Value root;

	// Sial Programs
	{
		Json::Value& sial_programs_root = root["SialPrograms"];
		SialProg::const_iterator it = sial_programs_.begin();
		for (; it != sial_programs_.end(); ++it)
			sial_programs_root.append(*it);
	}


	// Predefined Integers
	{
		Json::Value& predef_int_root = root["PredefinedIntegers"];
		PredefInt::const_iterator it = data_constants_.begin();
		for(; it != data_constants_.end(); ++it)
			predef_int_root[it->first] = it->second;
	}

	// Predefined Scalars
	{
		Json::Value& predef_scalar_root = root["PredefinedScalars"];
		PredefScalar::const_iterator it = scalars_.begin();
		for(; it != scalars_.end(); ++it)
			predef_scalar_root[it->first] = it->second;
	}

	// Segments sizes
	{
		Json::Value& segsize_root = root["SegmentSizeArrays"];
		SegSizeArray::const_iterator it = segments_.begin();
		for (; it != segments_.end(); ++it){
			std::string index_type_name = sip::index_type_name(sip::intToIndexType_t(it->first));
			Json::Value& seg_root = segsize_root[index_type_name];
			int count = it->second.first;
			int *array = it->second.second;
			for (int i=0; i<count; i++)
				seg_root.append(array[i]);
		}
	}
	
	// Predefined Contiguous Arrays
	{
		Json::Value& predefined_scalar_arrays_root = root["PredefinedScalarArrays"];
		PredefArrMap::const_iterator it = predef_arr_.begin();
		for (; it != predef_arr_.end(); ++it){
			Json::Value& array_root = predefined_scalar_arrays_root[it->first];
			const std::pair<int, std::pair<int*, double*> > &array_info = it->second;
			int rank = array_info.first;
			int *dims = array_info.second.first;
			double *data = array_info.second.second;
			int num_data_elems = 1;
			for (int i=0; i<rank; i++){
				num_data_elems *= dims[i];
			}
			array_root["Rank"] = rank;
			Json::Value& dims_root = array_root["Dims"];
			for (int i=0; i<rank; i++)
				dims_root.append(dims[i]);
			Json::Value& data_root = array_root["Data"];
			for (int i=0; i<num_data_elems; i++)
				data_root.append(data[i]);
		}
	}


	// Predefined Integer Arrays
	{
		Json::Value& predefined_integer_arrays_root = root["PredefinedScalarArrays"];
		PredefIntArrMap::const_iterator it = predef_int_arr_.begin();
		for (; it != predef_int_arr_.end(); ++it){
			Json::Value& array_root = predefined_integer_arrays_root[it->first];
			const std::pair<int, std::pair<int*, int*> > &array_info = it->second;
			int rank = array_info.first;
			int *dims = array_info.second.first;
			int *data = array_info.second.second;
			int num_data_elems = 1;
			for (int i=0; i<rank; i++){
				num_data_elems *= dims[i];
			}
			array_root["Rank"] = rank;
			Json::Value& dims_root = array_root["Dims"];
			for (int i=0; i<rank; i++)
				dims_root.append(dims[i]);
			Json::Value& data_root = array_root["Data"];
			for (int i=0; i<num_data_elems; i++)
				data_root.append(data[i]);
		}
	}


	// Sial Configuration per file
	{
		Json::Value& sial_config_root = root["SialConfiguration"];
		FileConfigMap::iterator it = configs_.begin();
		for (; it != configs_.end(); ++it){
			Json::Value& sial_file_root = sial_config_root[it->first];
			KeyValueMap::const_iterator kv_it = it->second.begin();
			for (; kv_it != it->second.end(); ++kv_it){
				sial_file_root[kv_it->first] = kv_it->second;
			}
		}
	}

	Json::StyledWriter writer;
	return writer.write(root);
}


}/* namespace setup */


