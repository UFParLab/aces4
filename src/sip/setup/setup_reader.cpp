/*! SetupReader.cpp
 * 
 *
 *  Created on: Jul 3, 2013
 *      Author: Beverly Sanders
 */

#include "config.h"
#include "setup_reader.h"
#include "assert.h"
#include "block.h"
#include "block_shape.h"
#ifdef HAVE_JSON
#include "json/json.h"
#endif
#include <stdexcept>
#include <sstream>
#include "memory_tracker.h"

namespace setup {


SetupReader* SetupReader::get_empty_reader(){
	char *c = new char[1];
	BinaryInputByteStream bis(c, 1);
	SetupReader *sr = new SetupReader(bis, false);
	return sr;
}

#ifdef HAVE_JSON
SetupReader::SetupReader(InputStream & stream) : binary_stream_(&stream), json_stream_(NULL) {
	read_binary();
}

SetupReader::SetupReader(InputStream & stream, bool to_read) : binary_stream_(&stream), json_stream_(NULL){
	if (to_read)
		read_binary();
}

SetupReader::SetupReader(std::istream& stream): binary_stream_(NULL), json_stream_(&stream){
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( *json_stream_, root );
	if ( !parsingSuccessful ){
		std::stringstream err;
		err << "Parsing of input json file failed !" << reader.getFormattedErrorMessages();
		sip::fail (err.str());
	}

	// Sial Programs
	{
		Json::Value& sial_programs_root = root["SialPrograms"];
		for (int i = 0; i < sial_programs_root.size(); ++i) {
			sial_prog_list_.push_back(sial_programs_root[i].asString());
		}
	}

	// Predefined Scalars
	{
		Json::Value& predef_scalar_root = root["PredefinedIntegers"];

		Json::ValueIterator it = predef_scalar_root.begin();
		for (; it != predef_scalar_root.end(); ++it){
			Json::Value key = it.key();
			Json::Value& val = predef_scalar_root[key.asString()];
			predefined_int_map_.insert(std::make_pair(key.asString(), val.asInt()));
		}
	}

	// Predefined Integers
	{
		Json::Value& predef_int_root = root["PredefinedScalars"];

		Json::ValueIterator it = predef_int_root.begin();
		for (; it != predef_int_root.end(); ++it){
			Json::Value key = it.key();
			Json::Value& val = predef_int_root[key.asString()];
			predefined_scalar_map_.insert(std::make_pair(key.asString(), val.asDouble()));
		}
	}

	// Segment Sizes
	{
		Json::Value& segsize_root = root["SegmentSizeArrays"];
		Json::ValueIterator it = segsize_root.begin();
		for(; it != segsize_root.end(); ++it){
			Json::Value key = it.key();
			Json::Value& seg_root = segsize_root[key.asString()];
			std::vector<int> segsizes_vec;
			for (Json::ArrayIndex i=0; i<seg_root.size(); i++){
				segsizes_vec.push_back(seg_root[i].asInt());
			}
			sip::IndexType_t index_type = sip::map_index_type(key.asString());
			segment_map_.insert(std::make_pair(index_type, segsizes_vec));
		}
	}

	// Predefined Contiguous Arrays
	{
		Json::Value& predefined_scalar_arrays_root = root["PredefinedScalarArrays"];
		Json::ValueIterator it = predefined_scalar_arrays_root.begin();
		for (; it != predefined_scalar_arrays_root.end(); ++it){
			Json::Value key = it.key();
			std::string array_name = key.asString();
			Json::Value& array_root = predefined_scalar_arrays_root[array_name];
			int rank = array_root["Rank"].asInt();

			// Construct segment sizes array for block shape
			sip::segment_size_array_t dims;
			Json::Value& dims_root = array_root["Dims"];
			for (Json::ArrayIndex i=0; i<dims_root.size(); ++i){
				dims[i] = (dims_root[i].asInt());
			}
			for (int i=dims_root.size(); i<MAX_RANK; ++i){
				dims[i] = sip::unused_index_segment_size;
			}

			// Construct data array for block data
			Json::Value& data_root = array_root["Data"];
			double *data = new double[data_root.size()];
			for (Json::ArrayIndex i=0; i<data_root.size(); ++i){
				data[i] = data_root[i].asDouble();
			}

			sip::BlockShape shape(dims, rank);
			sip::Block::BlockPtr block = new sip::Block(shape, data);

			PredefContigArray array_info;
			array_info.first = rank;
			array_info.second = block;
			name_to_predefined_contiguous_array_map_.insert(std::make_pair(array_name, array_info));
		}
	}

	// Predefined Integer Arrays
	{
		Json::Value& predefined_integer_arrays_root = root["PredefinedIntegerArrays"];
		Json::ValueIterator it = predefined_integer_arrays_root.begin();
		for (; it != predefined_integer_arrays_root.end(); ++it){
			Json::Value key = it.key();
			std::string array_name = key.asString();
			Json::Value& array_root = predefined_integer_arrays_root[array_name];
			int rank = array_root["Rank"].asInt();

			// Construct segment sizes array for block shape
			int* dims = new int[MAX_RANK];
			Json::Value& dims_root = array_root["Dims"];
			for (Json::ArrayIndex i=0; i<dims_root.size(); ++i){
				dims[i] = (dims_root[i].asInt());
			}
			for (int i=dims_root.size(); i<MAX_RANK; ++i){
				dims[i] = sip::unused_index_segment_size;
			}
			// Construct data array for block data
			Json::Value& data_root = array_root["Data"];
			int *data = new int[data_root.size()];
			for (Json::ArrayIndex i=0; i<data_root.size(); ++i){
				data[i] = data_root[i].asDouble();
			}

			PredefIntArray array_info;
			array_info.rank = rank;
			array_info.dims = dims;
			array_info.data = data;
			predef_int_arr_.insert(std::make_pair(array_name, array_info));
		}
	}

	// Sial Configuration per file
	{
		Json::Value& sial_config_root = root["SialConfiguration"];
		Json::ValueIterator it = sial_config_root.begin();
		for (; it != sial_config_root.end(); ++it){
			Json::Value key = it.key();
			std::string sial_file_name = key.asString();
			Json::Value& sial_file_root = sial_config_root[sial_file_name];
			Json::ValueIterator sit = sial_file_root.begin();
			KeyValueMap kv_map;
			for (; sit != sial_file_root.end(); ++sit){
				Json::Value key = sit.key();
				Json::Value val = sial_file_root[key.asString()];
				kv_map.insert(std::make_pair(key.asString(), val.asString()));
			}
			configs_.insert(std::make_pair(sial_file_name, kv_map));
		}
	}

}



std::string SetupReader::get_json_string() {
	Json::Value root;

	// Sial Programs
	{
		Json::Value& sial_programs_root = root["SialPrograms"];
		SialProgList::const_iterator it = sial_prog_list_.begin();
		for (; it != sial_prog_list_.end(); ++it)
			sial_programs_root.append(*it);
	}


	// Predefined Integers
	{
		Json::Value& predef_int_root = root["PredefinedIntegers"];
		PredefIntMap::const_iterator it = predefined_int_map_.begin();
		for(; it != predefined_int_map_.end(); ++it)
			predef_int_root[it->first] = it->second;
	}

	// Predefined Scalars
	{
		Json::Value& predef_scalar_root = root["PredefinedScalars"];
		PredefScalarMap::const_iterator it = predefined_scalar_map_.begin();
		for(; it != predefined_scalar_map_.end(); ++it)
			predef_scalar_root[it->first] = it->second;
	}

	// Segments sizes
	{
		Json::Value& segsize_root = root["SegmentSizeArrays"];
		SetupSegmentInfoMap::const_iterator it = segment_map_.begin();
		for (; it != segment_map_.end(); ++it){
			std::string index_type_name = sip::index_type_name(sip::intToIndexType_t(it->first));
			Json::Value& seg_root = segsize_root[index_type_name];
			std::vector<int>::const_iterator segments_it = it->second.begin();
			for (; segments_it != it->second.end(); ++segments_it)
				seg_root.append(*segments_it);
		}
	}

	// Predefined Contiguous Arrays
	{
		Json::Value& predefined_scalar_arrays_root = root["PredefinedScalarArrays"];
		NamePredefinedContiguousArrayMapIterator it = name_to_predefined_contiguous_array_map_.begin();
		for (; it != name_to_predefined_contiguous_array_map_.end(); ++it){
			Json::Value& array_root = predefined_scalar_arrays_root[it->first];
			PredefContigArray &array_info = it->second;
			int rank = array_info.first;
			sip::Block::BlockPtr block = array_info.second;
			double *data = block->get_data();
			sip::BlockShape block_shape = block->shape();
			int num_data_elems = block_shape.num_elems();
			array_root["Rank"] = rank;
			Json::Value& dims_root = array_root["Dims"];
			for (int i=0; i<rank; i++)
				dims_root.append(block_shape.segment_sizes_[i]);
			Json::Value& data_root = array_root["Data"];
			for (int i=0; i<num_data_elems; i++)
				data_root.append(data[i]);
		}
	}


	// Predefined Integer Arrays
	{
		Json::Value& predefined_integer_arrays_root = root["PredefinedIntegerArrays"];
		PredefIntArrMap::const_iterator it = predef_int_arr_.begin();
		for (; it != predef_int_arr_.end(); ++it){
			Json::Value& array_root = predefined_integer_arrays_root[it->first];
			const PredefIntArray &array_info = it->second;
			int rank = array_info.rank;
			int *dims = array_info.dims;
			int *data = array_info.data;
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
#else
SetupReader::SetupReader(InputStream & stream) : binary_stream_(&stream) {
	read_binary();
}

SetupReader::SetupReader(InputStream & stream, bool to_read) : binary_stream_(&stream){
	if (to_read)
		read_binary();
}
#endif //HAVE_JSON

SetupReader::~SetupReader() {
	for(PredefIntArrayIterator iter = predef_int_arr_.begin(); iter != predef_int_arr_.end(); ++iter){
		SIP_LOG(std::cout<<"From SetupReader, freeing "<<iter->first<<std::endl);
		delete [] iter->second.dims;  //dims
		delete [] iter->second.data; //data
	}
//    for (PredefArrayIterator iter = predef_arr_.begin(); iter != predef_arr_.end(); ++iter){
//    	SIP_LOG(std::cout<<"From SetupReader, freeing "<<iter->first<<std::endl);
//    	delete [] iter->second.second.first;
//    	delete [] iter->second.second.second;
//    }
    for (NamePredefinedContiguousArrayMapIterator iter = name_to_predefined_contiguous_array_map_.begin();
    		iter != name_to_predefined_contiguous_array_map_.end(); ++iter){
    	SIP_LOG(std::cout<<"From SetupReader, freeing "<<iter->first<<std::endl);
    	CHECK(iter->second.second, "attempting to delete NULL block in ~SetupReader");
		delete iter->second.second;
		iter->second.second = NULL;
    }
    name_to_predefined_contiguous_array_map_.clear();
}

void SetupReader::read_binary() {
	read_and_check_magic();
	read_and_check_version();
	read_sial_programs();
	read_predefined_ints();
	read_predefined_scalars();
	read_segment_sizes();
	read_predefined_arrays();
	read_predefined_integer_arrays();
	read_sialfile_configs();
}

std::ostream& operator<<(std::ostream& os, const SetupReader & obj) {
	os << "Sial program list:" << std::endl;
	SetupReader::SialProgList::const_iterator it;
	for (it = obj.sial_prog_list_.begin(); it != obj.sial_prog_list_.end();
			++it) {
		os << *it << std::endl;
	}
	os << "Predefined int map:" << std::endl;
	SetupReader::PredefIntMap::const_iterator iti;
	for (iti = obj.predefined_int_map_.begin();
			iti != obj.predefined_int_map_.end(); ++iti) {
		os << iti->first << "=" << iti->second << std::endl;
	}

	os << "Predefined scalar map:" << std::endl;
	SetupReader::PredefScalarMap::const_iterator its;
	for (its = obj.predefined_scalar_map_.begin();
			its != obj.predefined_scalar_map_.end(); ++its) {
		os << its->first << "=" << its->second << std::endl;
	}
	os << "Segment table info:" << std::endl;
	SetupReader::SetupSegmentInfoMap::const_iterator itg;
	for (itg = obj.segment_map_.begin(); itg != obj.segment_map_.end(); ++itg) {
		os << index_type_name(itg->first) << ":[";
		std::vector<int>::const_iterator sit;
		for (sit = (itg->second).begin(); sit != (itg->second).end(); ++sit) {
			os << (sit == (itg->second).begin()?"":",") << *sit;
		}
		os << ']' << std::endl;
	}

	os << "Predefined arrays:" << std::endl;
	SetupReader::NamePredefinedContiguousArrayMap::const_iterator itp = obj.name_to_predefined_contiguous_array_map_.begin();
	for (; itp != obj.name_to_predefined_contiguous_array_map_.end(); ++itp){
		int rank = itp->second.first;
		sip::BlockShape shape = itp->second.second->shape();
		double * data = itp->second.second->get_data();
		int num_elems = shape.num_elems();
		os << itp->first << ":{ rank : ";
		os << rank << " } (";
		os << shape.segment_sizes_[0];
		for (int i=1; i<rank; i++){
			os <<" ," << shape.segment_sizes_[i];
		}
		os << "), [";
		os << data [0];
		for (int i=1; i<num_elems && i < MAX_PRINT_ELEMS; i++){
			os << ", " << data[i];
		}
		os << "]" << std::endl;
	}

	os << "Predefined integer arrays:" << std::endl;
	SetupReader::PredefIntArrMap::const_iterator itpi;
	for (itpi = obj.predef_int_arr_.begin(); itpi != obj.predef_int_arr_.end(); ++itpi){
		int rank = itpi->second.rank;
		int * dims = itpi->second.dims;
		int * data = itpi->second.data;
		int num_elems = 1;
		for (int i = 0; i < rank; i++) {
			num_elems *= dims[i];
		}
		os << itpi->first << ":{ rank : ";
		os << rank << " } (";
		os << dims[0];
		for (int i=1; i<rank; i++){
			os <<" ," << dims[i];
		}
		os << "), [";
		os << data [0];
		for (int i=1; i<num_elems && i < MAX_PRINT_ELEMS; i++){
			os << ", " << data[i];
		}
		os << "]" << std::endl;
	}

	os << "Sial stream configurations:" << std::endl;
	SetupReader::FileConfigMap::const_iterator itc;
	for (itc = obj.configs_.begin(); itc != obj.configs_.end(); ++itc){
		std::string fileName = itc->first;
		os << "[" << fileName << "]" << "{";
		SetupReader::KeyValueMap kvMap = itc->second;
		SetupReader::KeyValueMap::const_iterator itkv;
		itkv = kvMap.begin();
		if (itkv != kvMap.end()){
			std::string key = itkv->first;
			std::string val = itkv->second;
			os << key <<":"<< val<< ", ";
			++itkv;
		}
		for (; itkv != kvMap.end(); ++itkv){
			std::string key = itkv->first;
			std::string val = itkv->second;
			os << ", "<<key <<":"<< val<< ", ";
		}
		os << "}";
	}

	os << std::endl;
	return os;
}

void SetupReader::dump_data() {
	dump_data(std::cout);
}

void SetupReader::dump_data(std::ostream& os) {
	std::cout << this;
}

int SetupReader::predefined_int(const std::string& name) {
	PredefIntMap::iterator it = predefined_int_map_.find(name);
	if (it == predefined_int_map_.end()){
		throw std::out_of_range("Could not find predefined integer : " + name);
	}
	return it->second;
}

double SetupReader::predefined_scalar(const std::string& name) {
	PredefScalarMap::iterator it = predefined_scalar_map_.find(name);
	if (it == predefined_scalar_map_.end()){
		throw std::out_of_range("Could not find predefined scalar : " + name);
	}
	return it->second;
}

PredefContigArray SetupReader::predefined_contiguous_array(const std::string& name){
	NamePredefinedContiguousArrayMap::iterator it = name_to_predefined_contiguous_array_map_.find(name);
	if (it == name_to_predefined_contiguous_array_map_.end()){
		throw std::out_of_range("Could not find predefined contiguous array : " + name);
	}
	return it->second;
}

PredefIntArray SetupReader::predefined_integer_array(const std::string& name){
	PredefIntArrMap::iterator it = predef_int_arr_.find(name);
	if (it == predef_int_arr_.end()){
		throw std::out_of_range("Could not find predefined integer array : " + name);
	}
	return it->second;
}

bool SetupReader::aces_validate(){
	return sip::input_warn(sial_prog_list_.size() > 0,"setup file is missing sial program" );
}
void SetupReader::read_and_check_magic() {
	int fmagic = binary_stream_->read_int();
	CHECK(fmagic == sip::SETUP_MAGIC,
			std::string("setup data file has incorrect magic number"));
}

void SetupReader::read_and_check_version() {
	int fversion = binary_stream_->read_int();
	CHECK(fversion == sip::SETUP_VERSION,
			std::string("setup data file has incorrect version"));
}

void SetupReader::read_sial_programs() {
	int n = binary_stream_->read_int();
	for (int i = 0; i < n; ++i) {
		std::string prog = binary_stream_->read_string();
		sial_prog_list_.push_back(prog);
	}
}

void SetupReader::read_predefined_ints() {
	int n = binary_stream_->read_int();
	for (int i = 0; i < n; ++i) {
		std::string name = binary_stream_->read_string();
		int value = binary_stream_->read_int();
		predefined_int_map_[name] = value;
	}
}

void SetupReader::read_predefined_scalars() {
	int n = binary_stream_->read_int();
	for (int i = 0; i < n; ++i) {
		std::string name = binary_stream_->read_string();
		double value = binary_stream_->read_double();
		predefined_scalar_map_[name] = value;
	}
}

void SetupReader::read_segment_sizes() {
	int n = binary_stream_->read_int();  //number of segment size entries
	for (int i = 0; i < n; ++i) {
		sip::IndexType_t index_type = sip::intToIndexType_t(binary_stream_->read_int());
		int num_segments;
		int * seg_sizes = binary_stream_->read_int_array(&num_segments);
		segment_map_[index_type] = std::vector<int>(seg_sizes,
				seg_sizes + num_segments);
		delete [] seg_sizes;
	}
}

void SetupReader::read_predefined_arrays(){
	int n = binary_stream_->read_int();
	for (int i = 0; i < n; i++) {
		// Name of array
		std::string name = binary_stream_->read_string();
		// Rank of array
		int rank = binary_stream_->read_int();
		// Dimensions
		int * dims = binary_stream_->read_int_array(&rank);
		// Number of elements
		int num_data_elems = 1;
		for (int i=0; i<rank; i++){
			num_data_elems *= dims[i];
		}
		double * data = binary_stream_->read_double_array(&num_data_elems);
		sip::MemoryTracker::global->inc_allocated(num_data_elems);
//		std::pair<int *, double *> dataPair = std::pair<int *, double *>(dims, data);
//		predef_arr_[name] = std::pair<int, std::pair<int *, double *> >(rank, dataPair);
//
		sip::segment_size_array_t dim_sizes;
		std::copy(dims+0, dims+rank,dim_sizes);
		std::fill(dim_sizes+rank, dim_sizes+MAX_RANK, 1);
		sip::BlockShape shape(dim_sizes, rank);
		delete [] dims;
//
//		double * data2 = new double[num_data_elems];
//		std::copy(data+0, data+num_data_elems, data2);
//		name_to_predefined_contiguous_array_map_[name] = new sip::Block(shape, data2);
		std::pair<int, sip::Block::BlockPtr> rank_blockptr_pair(rank, new sip::Block(shape, data));
		std::pair<NamePredefinedContiguousArrayMap::iterator, bool> ret =
				name_to_predefined_contiguous_array_map_.insert(make_pair(name, rank_blockptr_pair));
		CHECK(ret.second, "Trying to read another array by name : " + name);

	}
}

void SetupReader::read_predefined_integer_arrays(){
	int n = binary_stream_->read_int();
	for (int i = 0; i < n; i++) {
		// Name of array
		std::string name = binary_stream_->read_string();
		// Rank of array
		int rank = binary_stream_->read_int();
		// Dimensions
		int * dims = binary_stream_->read_int_array(&rank);
		// Number of elements
		int num_data_elems = 1;
		for (int i=0; i<rank; i++){
			num_data_elems *= dims[i];
		}
		//read the elements
		int * data = binary_stream_->read_int_array(&num_data_elems);

		PredefIntArray predef_int_array = { rank, dims, data};
		//std::pair<int *, int *> dataPair = std::pair<int *, int *>(dims, data);
		//predef_int_arr_[name] = std::pair<int, std::pair<int *, int *> >(rank, dataPair);
		std::pair<PredefIntArrMap::iterator, bool> ret =
				predef_int_arr_.insert (make_pair(name, predef_int_array));
		CHECK(ret.second, "Trying to read another array by name : " + name);

	}
}

void SetupReader::read_sialfile_configs(){
	int num_sialfiles = binary_stream_->read_int();
	for (int i=0; i<num_sialfiles; i++){
		// Name of Sial File
		std::string sialfile = binary_stream_->read_string();
		// Number of config entries
		int num_entries = binary_stream_->read_int();
		KeyValueMap kvMap;
		for (int j=0; j<num_entries; j++){
			std::string key = binary_stream_->read_string();
			std::string val = binary_stream_->read_string();
			kvMap[key] = val;
		}
		configs_[sialfile] = kvMap;
	}
}



} /* namespace setup */
