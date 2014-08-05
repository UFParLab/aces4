/*
 * IntTable.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: basbas
 */

#include "int_table.h"
#include <stdexcept>

namespace sip {

IntTable::IntTable() { /*filled in by calling reader */}

IntTable::~IntTable() {}

void IntTable::read(IntTable& intTable, setup::InputStream& siox_file, setup::SetupReader& setup) {
	int n = siox_file.read_int(); //number of entries
	for (int i = 0; i < n; ++i) {
		std::string name = siox_file.read_string();
//		std::cout<<"name:"<<name;
		intTable.name_entry_map_[name] = i;
		intTable.entryindex_name_map_.push_back(name);
		int value = setup.predefined_int(name);
		intTable.entries_.push_back(value);
	}
}


int IntTable::value(int index) const{
	return entries_.at(index);
}
std::string IntTable::name(int index) const{
	return entryindex_name_map_.at(index);
}
int IntTable::index(std::string name) const{
	std::map<std::string, int>::const_iterator it = name_entry_map_.find(name);
	if (it == name_entry_map_.end()){
		throw std::out_of_range("Could not find index : " + name);
	}
	return it->second;
}

void IntTable::clear(){
   entries_.clear();
   name_entry_map_.clear();
   entryindex_name_map_.clear();

}

std::ostream& operator<<(std::ostream& os, const IntTable & table) {
	std::map<std::string, int>::const_iterator it;
	for (it = table.name_entry_map_.begin(); it != table.name_entry_map_.end();
			++it) {
		std::string name = it->first;
		int index = it->second;
		int value = table.entries_[index];
		os << index << ":" << name << "=" << value  << std::endl;
	}
	return os;
}

} /* namespace sip */
