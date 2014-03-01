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
#include "setup_reader.h"

namespace sip {

class IntTable {
public:
	IntTable();
	~IntTable();
	static void read(IntTable&, setup::InputStream&, setup::SetupReader&);
	/** returns the value associated with the given slot */
	int value(int);
	/** returns the name associated with the given slot */
	std::string name(int); //returns the name associated with indicated location in the int table
	/** returns the index associated with the given name */
	int index(std::string);

	friend std::ostream& operator<<(std::ostream&, const IntTable &);
	friend class SipTables;
private:
    void clear();
	std::vector<int> entries_;  //maps index to value
	std::map<std::string, int> name_entry_map_;  //maps name to entry index
	std::vector<std::string> entryindex_name_map_;  //maps entry index to name
	DISALLOW_COPY_AND_ASSIGN(IntTable);
};

} /* namespace sip */
#endif /* SIP_INTTABLE_H_ */
