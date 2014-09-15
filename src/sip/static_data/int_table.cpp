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

IntTable::IntTable(const IntTable& other):
		values_ (other.values_),
		attributes_(other.attributes_),
		name_slot_map_(other.name_slot_map_),
		slot_name_map_(other.slot_name_map_)
		{
}

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


/** returns the value associated with the given slot */
int IntTable::value(int slot) const {
	try {
		return values_.at(slot);
	} catch (const std::out_of_range& oor) {
		fail("out of range error when looking up value in int table");
	}
	return 0.0; //should never get here
}
/** set the indicated int to the given value */
void IntTable::set_value(int slot, int value) {
	values_[slot] = value;
}
/** returns the name associated with the given slot */
std::string IntTable::name(int slot) const {
	try {
		return slot_name_map_.at(slot);
	} catch (const std::out_of_range& oor) {
		fail("out of range error when looking up name associated with a slot in the int table");
	}
	return "error";  //should never get here
}
/** returns the slot associated with the given name */
int IntTable::slot(std::string name) const {
	try {
		return name_slot_map_.at(name);
	} catch (const std::out_of_range& oor) {
		fail("out of range error when looking up int tables slot  associated with name");
	}
	return -1; //should never get here
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
