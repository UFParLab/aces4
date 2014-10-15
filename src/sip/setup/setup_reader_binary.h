/*! setup_reader_binary.h
 * 
 *
 *  Created on: Jul 3, 2013
 *      Author: Beverly Sanders
 */

#ifndef SETUPREADERBINARY_H_
#define SETUPREADERBINARY_H_

#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include "sip.h"
#include "io_utils.h"
#include "array_constants.h"
#include "block.h"
#include "setup_reader.h"
#define MAX_PRINT_ELEMS 512

namespace sip {
class IndexTable;
class ContiguousArrayManager;
}

namespace setup {


class SetupReaderBinary : public SetupReader {

public:

	static SetupReaderBinary* get_empty_reader();

	SetupReaderBinary(InputStream &);
	virtual ~SetupReaderBinary();

	void dump_data();  	//for debugging;
	void dump_data(std::ostream&);  //for debugging;


	/** performs sanity checks on input
	 * right now, this just checks for the existence of a sial program name
	 */
	virtual bool aces_validate();


private:

	/**
	 * Constructs SetupReaderBinary instance, optionally calling "read()" of the InputStream reference.
	 * @param
	 * @param
	 */
	SetupReaderBinary(InputStream &, bool);

    void read();

	void read_and_check_magic();
	void read_and_check_version();
	void read_sial_programs();
    void read_predefined_ints();
    void read_predefined_scalars();
    void read_segment_sizes();
    void read_predefined_arrays();
    void read_predefined_integer_arrays();
    void read_sialfile_configs();

	void dump_predefined_int_map(std::ostream&);
	void dump_predefined_scalar_map(std::ostream&);
	void dump_sial_prog_list(std::ostream&);
	void dump_segment_map(std::ostream&);
	void dump_predefined_map(std::ostream&);

	InputStream& stream_;

	friend std::ostream& operator<<(std::ostream&, const SetupReaderBinary &);

	friend class sip::IndexTable;
	friend class sip::ContiguousArrayManager;

	DISALLOW_COPY_AND_ASSIGN(SetupReaderBinary);


};

} /* namespace setup */

#endif /* SETUPREADERBINARY_H_ */
