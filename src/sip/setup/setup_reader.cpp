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
#include <stdexcept>

namespace setup {


SetupReader* SetupReader::get_empty_reader(){
	char *c = new char[1];
	BinaryInputByteStream bis(c, 1);
	SetupReader *sr = new SetupReader(bis, false);
	return sr;
}

SetupReader::SetupReader(InputStream & stream) : stream_(stream) {
	read();
}

SetupReader::SetupReader(InputStream & stream, bool to_read) : stream_(stream) {
	if (to_read)
		read();
}

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
    	check(iter->second.second, "attempting to delete NULL block in ~SetupReader");
		delete iter->second.second;
		iter->second.second = NULL;
    }
    name_to_predefined_contiguous_array_map_.clear();
}

void SetupReader::read() {
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
		os << itg->first << ":[";
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

void SetupReader::read_and_check_magic() {
	int fmagic = stream_.read_int();
	sip::check(fmagic == sip::SETUP_MAGIC,
			std::string("setup data file has incorrect magic number"));
}

void SetupReader::read_and_check_version() {
	int fversion = stream_.read_int();
	sip::check(fversion == sip::SETUP_VERSION,
			std::string("setup data file has incorrect version"));
}

void SetupReader::read_sial_programs() {
	int n = stream_.read_int();
	for (int i = 0; i < n; ++i) {
		std::string prog = stream_.read_string();
		sial_prog_list_.push_back(prog);
	}
}

void SetupReader::read_predefined_ints() {
	int n = stream_.read_int();
	for (int i = 0; i < n; ++i) {
		std::string name = stream_.read_string();
		int value = stream_.read_int();
		predefined_int_map_[name] = value;
	}
}

void SetupReader::read_predefined_scalars() {
	int n = stream_.read_int();
	for (int i = 0; i < n; ++i) {
		std::string name = stream_.read_string();
		double value = stream_.read_double();
		predefined_scalar_map_[name] = value;
	}
}

void SetupReader::read_segment_sizes() {
	int n = stream_.read_int();  //number of segment size entries
	for (int i = 0; i < n; ++i) {
		sip::IndexType_t index_type = sip::intToIndexType_t(stream_.read_int());
		int num_segments;
		int * seg_sizes = stream_.read_int_array(&num_segments);
		segment_map_[index_type] = std::vector<int>(seg_sizes,
				seg_sizes + num_segments);
		delete [] seg_sizes;
	}
}

void SetupReader::read_predefined_arrays(){
	int n = stream_.read_int();
	for (int i = 0; i < n; i++) {
		// Name of array
		std::string name = stream_.read_string();
		// Rank of array
		int rank = stream_.read_int();
		// Dimensions
		int * dims = stream_.read_int_array(&rank);
		// Number of elements
		int num_data_elems = 1;
		for (int i=0; i<rank; i++){
			num_data_elems *= dims[i];
		}
		double * data = stream_.read_double_array(&num_data_elems);
//		std::pair<int *, double *> dataPair = std::pair<int *, double *>(dims, data);
//		predef_arr_[name] = std::pair<int, std::pair<int *, double *> >(rank, dataPair);
//
		sip::segment_size_array_t dim_sizes;
		std::copy(dims+0, dims+rank,dim_sizes);
		std::fill(dim_sizes+rank, dim_sizes+MAX_RANK, 1);
		sip::BlockShape shape(dim_sizes);
		delete [] dims;
//
//		double * data2 = new double[num_data_elems];
//		std::copy(data+0, data+num_data_elems, data2);
//		name_to_predefined_contiguous_array_map_[name] = new sip::Block(shape, data2);
		std::pair<int, sip::Block::BlockPtr> rank_blockptr_pair(rank, new sip::Block(shape, data));
		std::pair<NamePredefinedContiguousArrayMap::iterator, bool> ret =
				name_to_predefined_contiguous_array_map_.insert(make_pair(name, rank_blockptr_pair));
		sip::check(ret.second, "Trying to read another array by name : " + name);

	}
}

void SetupReader::read_predefined_integer_arrays(){
	int n = stream_.read_int();
	for (int i = 0; i < n; i++) {
		// Name of array
		std::string name = stream_.read_string();
		// Rank of array
		int rank = stream_.read_int();
		// Dimensions
		int * dims = stream_.read_int_array(&rank);
		// Number of elements
		int num_data_elems = 1;
		for (int i=0; i<rank; i++){
			num_data_elems *= dims[i];
		}
		//read the elements
		int * data = stream_.read_int_array(&num_data_elems);

		PredefIntArray predef_int_array = { rank, dims, data};
		//std::pair<int *, int *> dataPair = std::pair<int *, int *>(dims, data);
		//predef_int_arr_[name] = std::pair<int, std::pair<int *, int *> >(rank, dataPair);
		std::pair<PredefIntArrMap::iterator, bool> ret =
				predef_int_arr_.insert (make_pair(name, predef_int_array));
		sip::check(ret.second, "Trying to read another array by name : " + name);

	}
}

void SetupReader::read_sialfile_configs(){
	int num_sialfiles = stream_.read_int();
	for (int i=0; i<num_sialfiles; i++){
		// Name of Sial File
		std::string sialfile = stream_.read_string();
		// Number of config entries
		int num_entries = stream_.read_int();
		KeyValueMap kvMap;
		for (int j=0; j<num_entries; j++){
			std::string key = stream_.read_string();
			std::string val = stream_.read_string();
			kvMap[key] = val;
		}
		configs_[sialfile] = kvMap;
	}
}



} /* namespace setup */
