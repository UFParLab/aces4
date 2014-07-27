/*
 * IntTable.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: basbas
 */

#include "int_table.h"

namespace sip {

IntTable::IntTable() { /*filled in by calling reader */}

IntTable::~IntTable() {}

void IntTable::read(IntTable& intTable, setup::InputStream& siox_file, setup::SetupReader& setup) {
	int n = siox_file.read_int(); //number of entries
	for (int i = 0; i < n; ++i) {
		int value = siox_file.read_int();
		int attribute = siox_file.read_int();
		intTable.attributes_.push_back(attribute);
		std::string name = siox_file.read_string();
//		std::cout<<"name:"<<name;
		intTable.name_slot_map_[name] = i;
		intTable.slot_name_map_.push_back(name);
		if(is_predefined_attr(attribute)){
			value = setup.predefined_int(name);
		}
		intTable.values_.push_back(value);
	}
}



void IntTable::clear(){
   values_.clear();
   name_slot_map_.clear();
   slot_name_map_.clear();

}

std::ostream& operator<<(std::ostream& os, const IntTable & table) {
	std::map<std::string, int>::const_iterator it;
	for (it = table.name_slot_map_.begin(); it != table.name_slot_map_.end();
			++it) {
		std::string name = it->first;
		int index = it->second;
		int value = table.values_[index];
		int attribute = table.attributes_[index];
		os << index << ":" << (is_predefined_attr(attribute)? "predefined ":"")
				<< name << "=" << value  << std::endl;
	}
	return os;
}

} /* namespace sip */
