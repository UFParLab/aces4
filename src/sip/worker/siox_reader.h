/*
 * sioreader.h  This class reads the .sio file and constructs a representation
 * of it.
 *
 * All this file does is load the contents.  Symbolic constants, etc. are not
 * initialized by this class.
 *
 *  Created on: Jul 14, 2013
 *      Author: basbas
 */

#ifndef MASTER_SIOX_READER_H_
#define MASTER_SIOX_READER_H_



#include "sip.h"
#include <map>
#include <vector>
#include <iostream>
#include "setup_reader.h"
#include "io_utils.h"
#include "siox_reader.h"

namespace setup {
class InputStream;
}


namespace sip {

class SipTables;


class SioxReader {
public:

	SioxReader(SipTables&, setup::InputStream&, setup::SetupReader&);
	virtual ~SioxReader();

	double * contiguous_array_data(std::string);

	friend std::ostream& operator<<(std::ostream&, const SioxReader &);

private:
	SipTables& tables;  //for convenience
    setup::InputStream&  file;
    setup::SetupReader& setup;

	void read();

    void read_and_check_header();
    void read_index_table();
    void read_array_table();
    void read_op_table();
    void read_scalar_table();
    void read_int_table();
    void read_special_instruction_table();
    void read_string_literal_table_();
    void read_index_symbols();
    void read_array_symbols();


	DISALLOW_COPY_AND_ASSIGN(SioxReader);

};

} /* namespace master */


#endif /* MASTER_SIOX_READER_H_ */



