/*! setup_interface.cpp
 * 
 *
 *  Created on: Jul 3, 2013
 *      Author: Beverly Sanders
 */

//#include "setup_interface.h"
#include <assert.h>
#include "io_utils.h"
#include "setup_writer.h"
#include "setup_reader.h"
#include "setup_reader_binary.h"
#include "sip.h"

setup::SetupWriter *writer;


#ifdef __cplusplus
extern "C" {
#endif

void init_setup(const char * job_name){
	   setup::OutputStream * file;
	   std::string name = std::string(job_name);
//	   if (sip::SETUP_FILE_TYPE_IS_BINARY){
		   setup::BinaryOutputFile *bfile = new setup::BinaryOutputFile(name + ".dat");
		   file = bfile;
//	   }
	   writer = new setup::SetupWriter(name,file);
   }

void finalize_setup(){
	writer->write_header_file();
	writer->write_data_file();
	delete writer;
}

void set_constant(const char * name, int value){
	   writer->addPredefinedIntData(std::string(name), value);
}

void set_scalar(const char * name, double value){
	writer->addPredefinedScalar(std::string(name), value);
}

void add_sial_program(const char * name){
	writer->addSialProgram(std::string(name));
}
/*!  Specify segment sizes for the aoindex type */
void set_aoindex_info(int num_segments, int *segment_sizes){
	writer->addSegmentInfo(sip::aoindex, num_segments, segment_sizes);
}

/*!  Specify  segment sizes for the moaindex type */
void set_moaindex_info(int num_segments, int *segment_sizes){
	writer->addSegmentInfo(sip::moaindex, num_segments, segment_sizes);
}

/*!  Specify  segment sizes for the mobindex type */
void set_mobindex_info(int num_segments, int *segment_sizes){
	writer->addSegmentInfo(sip::mobindex, num_segments, segment_sizes);
}

 /*!  Specify segment sizes for the moindex type */
void set_moindex_info(int num_segments, int *segment_sizes){
	writer->addSegmentInfo(sip::moindex, num_segments, segment_sizes);
}

/*!  Specify static predefined arrays */
void set_predefined_scalar_array(const char *static_array_name, int numDims, int *dims, double *vals){
	writer->addPredefinedContiguousArray(std::string(static_array_name), numDims, dims, vals);
}

/*!  Specify static predefined integer arrays */
void set_predefined_integer_array(const char *static_array_name, int numDims, int *dims, int *vals){
	writer->addPredefinedIntegerContiguousArray(std::string(static_array_name), numDims, dims, vals);


    // Debug
	/*
    int numElems = 1;
    for (int i=0; i<numDims; i++)
        numElems *= dims[i];
    std::cout << static_array_name << " [" << dims[0];
    for (int i=1; i<numDims; i++)
        std :: cout <<", "<< dims[i];
    std::cout << "], {" << vals[0];
    for (int i=1; i<numElems; i++)
        std::cout << ", " << vals[i];
    std::cout << std :: endl;
	*/
}

/*!  Specify sial config parameters */
void set_config_info(const char* sialfile, const char* key, const char* value){
	writer->addSialFileConfigInfo(std::string(sialfile), std::string(key), std::string(value));
	// Debug
//	std::cout << sialfile << std::endl;
//	std::cout << key << std::endl;
//	std::cout << value << std::endl;
}


void dump_file(const char * name){
	std::string fname = std::string(name);
	setup::BinaryInputFile bfile (fname);
	setup::SetupReaderBinary reader(bfile);
	reader.dump_data();
}

#ifdef __cplusplus
}
#endif

//void set_static_array(const char *static_array_name, int numDims, int *dims, double *vals){
//	 assert(0);
//	  setup::writer.addStaticArray_(std::string(static_array_name), numDims, dims, vals);
//}
