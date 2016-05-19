/*
 * dump_array_file.cpp
 *
 *  Created on: Aug 25, 2015
 *      Author: basbas
 */

#include <fstream>
#include <string>

#include "array_file.h"

int read_int(std::fstream* stream);
double read_double(std::fstream* stream);
sip::ArrayFile::offset_val_t read_offset(std::fstream* stream);
sip::ArrayFile::header_val_t read_header_val(std::fstream* stream);

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << "file" << std::endl;
		return 1;
	}
	std::string filename(argv[1]);

	std::fstream fs;
	fs.open (filename.c_str(), std::fstream::in | std::fstream::binary );



//print header
	sip::ArrayFile::header_val_t num_blocks = read_header_val(&fs);
	std::cout << "num_blocks=" << num_blocks << std::endl;
	sip::ArrayFile::header_val_t chunk_size = read_header_val(&fs);
    std::cout << "chunk_size=" << chunk_size << std::endl;
	sip::ArrayFile::header_val_t num_servers = read_header_val(&fs);
    std::cout << "num_servers=" << num_servers << std::endl;
    std::cout << std::endl;

//print index
    std::cout << "index=" << std::endl;
    for (int i = 0; i < num_blocks; ++i){
    	std::cout << "[" << i << "]=" << read_offset(&fs) << std::endl;
    }

	  fs.close();

	  return 0;

}


int read_int(std::fstream* stream) {
	int value;
	stream->read( reinterpret_cast<char *>(&value), sizeof(value));
	if (stream->good()){
	return value;
	}
	else std::cerr <<  std::string( "error in read_int") << std::endl;
	return 0;
}

double read_double(std::fstream* stream) {
	double value;
	stream->read( reinterpret_cast<char *>(&value), sizeof(value));
	if (stream->good()){
	return value;
	}
	else std::cerr <<  std::string( "error in read_double") << std::endl;
	return 0;
}

sip::ArrayFile::offset_val_t read_offset(std::fstream* stream) {
	sip::ArrayFile::offset_val_t value;
	stream->read( reinterpret_cast<char *>(&value), sizeof(value));
	if (stream->good()){
	return value;
	}
	else std::cerr <<  std::string( "error in read_offset") << std::endl;
	return 0;
}

sip::ArrayFile::header_val_t read_header_val(std::fstream* stream) {
	sip::ArrayFile::header_val_t value;
	stream->read( reinterpret_cast<char *>(&value), sizeof(value));
	if (stream->good()){
	return value;
	}
	else std::cerr <<  std::string( "error in read_header_val ") << std::endl;
	return 0;
}
