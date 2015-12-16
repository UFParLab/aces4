//TODO  array table contains fortran index of scalar table.  Consider whether this is a good idea.

/**
 * sioreader.cpp
 *
 *A SioxReader object is associated with an InputStream pointing to a .siox file on construction
 *
 *SioxReader is the location that knows of the overall format of .siox files, and delegates
 *reading individual tables that have associated classes to those classes.
 *
 *The read method takes a SipTables object to initialize.
 *  Created on: Jul 14, 2013
 *      Author: basbas
 */

#include "config.h"
#include "siox_reader.h"
#include "io_utils.h"
#include "sip_tables.h"  //this brings in header files for various tables.

namespace sip {

/** Constructor for class that reads .siox files
 * @param tables points to instance of SipTables, which should already be instantiated.
 *     tables is not managed by this class.
 * @param file reference to InputStream (Binary) InputStream associated with .siox file
 * @param setup the SetupReader object used to initialize predefined constants, etc.
 *
 */
SioxReader::SioxReader(SipTables& tables, setup::InputStream& file, setup::SetupReader& setup):
	file(file), setup(setup), tables(tables){
	read();
}

SioxReader::~SioxReader() {}
/*   Format of .siox file
*   <siox> ::= <Header> <IntConstantTable> <IndexTable> <ArrayTable> <OpTable> <ScalarTable> <SpecialTable> <StringLitTable>
*/
void SioxReader::read(){
    read_and_check_header();
	read_int_table(); //TODO generalize.  These are called constants in the compiler, but should be done for now.
    read_index_table();
    read_array_table();
    read_op_table();
    read_scalar_table();  //requires array table already read
    read_special_instruction_table();
    read_string_literal_table_();
    //tables->setup_reader_ = setup;
}

double * SioxReader::contiguous_array_data(std::string name){
    WARN(false, "implement SioxReader::get_contiguous_array_data");
    return NULL;
}

void SioxReader::read_and_check_header(){
	int magic = file . read_int();
	CHECK(magic == SIOX_MAGIC, std::string("siox file has the wrong magic number"));
	int version = file . read_int(); //version
	CHECK(version == SIOX_VERSION, "siox file has the wrong version number");
	int max_dim = file . read_int(); //release
	CHECK(max_dim == MAX_RANK, "siox file has the inconsistent value for MAX_RANK number");
	}

void SioxReader::read_int_table(){
	IntTable::read(tables.int_table_, file, setup);
}

void SioxReader::read_index_table(){
    tables.index_table_.init(file, setup, tables.int_table_);
}

void SioxReader::read_array_table(){
	tables.array_table_.init(file);
}

void SioxReader::read_op_table(){
    OpTable::read(tables.op_table_, file);
}

void SioxReader::read_scalar_table(){
	int size;
	double * tmp = file.read_double_array_siox(&size);
	if (size > 0) {
		tables.scalar_table_.insert(tables.scalar_table_.begin(), tmp, tmp+size);
	}
	delete [] tmp;
	//traverse array table to set values of predefined scalars
	std::vector<sip::ArrayTableEntry>::iterator it = tables.array_table_.entries_.begin();
	for (; it != tables.array_table_.entries_.end(); ++it){
		int attr = it->array_type_;
		if(sip::is_predefined_scalar_attr(attr)) {
			std::string name = it->name_;
			double value = setup.predefined_scalar(name);
			int scalar_table_index = it->scalar_selector_;
			tables.scalar_table_[scalar_table_index] = value;
		}
	}
}


void SioxReader::read_special_instruction_table(){
	SIP_LOG(std::cout<< " in void SioxReader::read_special_instruction_table()" << std::endl);
	sip::SpecialInstructionManager::read(tables.special_instruction_manager_, file);
}


void SioxReader::read_string_literal_table_(){
	int size;
	size =  file.read_int();
	for (int i = 0; i < size; i++){
		std::string s = file.read_string();
		tables.string_literal_table_.push_back(s);
	}
}

} /*namespace sip*/
