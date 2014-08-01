/*
 * IntTable.h
 *
 *  Created on: Aug 3, 2013
 *      Author: basbas
 */

#ifndef SIP_INTTABLE_H_
#define SIP_INTTABLE_H_

#include "sip.h"
#include <map>
#include <vector>
#include <iostream>
#include <exception>
#include "setup_reader.h"
#include <stdexcept>

namespace sip {

//TODO  this is no longer constant.  Need to move values_ array to a separate class in dynamic_data
class IntTable {
public:
	IntTable();
	~IntTable();
	static void read(IntTable&, setup::InputStream&, setup::SetupReader&);
	/** returns the value associated with the given slot */
	int value(int slot) const {
		try{
		return values_.at(slot);
		}
		  catch (const std::out_of_range& oor) {
		    fail("out of range error when looking up value in int table");
		  }
return 0.0; //should never get here
		}
	/** set the indicated int to the given value */
	void set_value(int slot, int value){ values_[slot]=value; }
	/** returns the name associated with the given slot */
	std::string name(int slot) const {
		try{
			return slot_name_map_.at(slot);
	}
	    catch(const std::out_of_range& oor) {
		    fail("out of range error when looking up name associated with a slot in the int table");
		  }
	    return "error";  //should never get here
}
	/** returns the slot associated with the given name */
//	int slot(std::string name) const{
//		try {return name_slot_map_.at(name);}
//		catch (std::exception& e){
//			std::cerr << "exception "<< e.what() << "thrown in " << __FILE__ << " at line " << __LINE__ << std::endl;
//			fail("lookup of int " + name + " failed.  It is not in the int table");
//		}
//		return -1;
//	}
	int slot(std::string name) const {
		try
		{ return name_slot_map_.at(name);}
		catch (const std::out_of_range& oor) {
		    fail("out of range error when looking up int tables slot  associated with name");
		  }
		return -1; //should never get here
	}

	friend std::ostream& operator<<(std::ostream&, const IntTable &);
	friend class SipTables;
private:
    void clear();
	std::vector<int> values_;  //maps slot to value
	std::vector<int> attributes_; //maps slot to attribute
	std::map<std::string, int> name_slot_map_;  //maps name to slot
	std::vector<std::string> slot_name_map_;  //maps slot to name
	DISALLOW_COPY_AND_ASSIGN(IntTable);
};

} /* namespace sip */
#endif /* SIP_INTTABLE_H_ */

