/*! io_utils.cpp  Contains routines to read and write primitive types and arrays from
 * binary and text files.
 * 
 *
 *  Created on: Jul 7, 2013
 *      Author: Beverly Sanders
 */


#include "io_utils.h"
#include <stdexcept>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

namespace setup{

/* Default constructors and destructors for OutputFile */
OutputStream::OutputStream():stream_(NULL){}

OutputStream::~OutputStream(){
	if(stream_) {
		//stream_->close();
		delete stream_;
	}
}

/* BinaryOutputFile */
BinaryOutputFile::BinaryOutputFile(const std::string& name) : file_name_(name){
	file_stream_ = new std::ofstream(name.c_str(), std::ios::binary | std::ios::trunc);
	stream_ = file_stream_;
	if (!file_stream_->is_open()){
		std::cout << stream_->eof() << "\t" << stream_->fail() << "\t" << stream_->bad();
		std :: cerr << "File "<< name  <<" could not be opened !";
		exit(-1);
	}
}

BinaryOutputFile::~BinaryOutputFile(){
	if (file_stream_->is_open())
		file_stream_->close();
}

void BinaryOutputStream::write_string(const std::string& aString) {
        //trim trailing spaces
        size_t endpos = aString.find_last_not_of(" ");
        std::string trimmed = (endpos == std::string::npos) ? "" : aString.substr(0,endpos+1);
	int length = (int) trimmed.length() + 1; //extra for null
	write_int(length);
//std::cout << "writing string " << trimmed << ", length=" << length << std::endl;
	stream_->write(trimmed.c_str(), length);
}

void BinaryOutputStream::write_int( int value) {
	stream_->write(reinterpret_cast<char *>(&value), sizeof(int));
}

void BinaryOutputStream::write_int_array(int size, int  values[]) {
	write_int(size);
	stream_->write(reinterpret_cast<char *>(values), sizeof(int) * size);
}

void BinaryOutputStream::write_double(double value) {
	stream_->write(reinterpret_cast<char *>(&value), sizeof(double));
}

void BinaryOutputStream::write_double_array(int size, double values[]) {
	write_int(size);
	stream_->write( reinterpret_cast<char *>(values), sizeof(double) * size);
}

void BinaryOutputStream::write_size_t_val(size_t value) {
	stream_->write( reinterpret_cast<char *>(&value), sizeof(size_t));
}

BinaryOutputByteStream::BinaryOutputByteStream(char *char_stream, int size){
	char_stream_ = new CharStreamBuf(char_stream, char_stream + size);
	this->stream_ = new std::ostream(char_stream_);

}

BinaryOutputByteStream::~BinaryOutputByteStream(){
	delete char_stream_;
}


InputStream::InputStream():stream_(NULL){}

InputStream::~InputStream(){
		if ((stream_)) {
		  //if (stream->is_open()){  stream->close();}
		  delete stream_;
	}
}
/* BinaryInputFile */
BinaryInputFile::BinaryInputFile(const std::string& name):file_name_(name){
	file_stream_ = new std::ifstream(name.c_str(), std::ifstream::binary);
	stream_ = file_stream_;
	if (!file_stream_->is_open()){
		std::cout << stream_->eof() << "\t" << stream_->fail() << "\t" << stream_->bad();
		std :: cerr << "File "<< name  <<" could not be opened !";
		exit(-1);
	}
	//assert (file->is_open());  //TODO  better error handling
}
BinaryInputFile::~BinaryInputFile(){
	if (file_stream_->is_open()){  file_stream_->close();}
}

std::string BinaryInputFile::get_file_name(){
		return file_name_;
}

std::string BinaryInputStream::read_string() {
	int length = read_int();
	char *chars = new char[length+1];
	stream_-> read(chars, length);
    CHECK(stream_->good(), std::string("malformed input "));
	chars[length]= '\0';
	std::string s(chars);
	delete [] chars;
	return s;
}

int BinaryInputStream::read_int() {
	int value;
	stream_->read( reinterpret_cast<char *>(&value), sizeof(value));
	CHECK(stream_->good(), std::string( "error in read_int of input "));
	return value;
}

double BinaryInputStream::read_double() {
	double value;
	stream_->read( reinterpret_cast<char *>(&value), sizeof(value));
	CHECK(stream_->good(), std::string( "error in read_double of input "));
	return value;
}

int * BinaryInputStream::read_int_array(int * size) {
	int sizec = read_int();
	int * values = new int[sizec];
	stream_->read( reinterpret_cast<char *>(values), sizeof(int) * sizec);
	*size = sizec;
	CHECK(stream_->good(), std::string( "error in read_int_array of input "));
	return values;
}

double * BinaryInputStream::read_double_array(int *size){
	int sizec = read_int();
	double * values = new double[sizec];
	stream_->read( reinterpret_cast<char *>(values), sizeof(double) * sizec);
	CHECK(stream_->good(), std::string( "error in read_double_array of input "));
	*size = sizec;
	return values;
}

std::string * BinaryInputStream::read_string_array(int * size){
	int sizec = read_int();
	std::string * strings = new std::string[sizec];
	for (int i = 0; i < sizec; ++i){
		strings[i] = read_string();
	}
	*size = sizec;
	CHECK(stream_->good(), std::string( "error in read_int_array of input "));
	return strings;
}

//TODO figure out what is going on with int vs size_t
double * BinaryInputStream::read_double_array_siox(int *size){
	int sizec = read_int();
//	std::cout << "size of double array " << sizec << std::endl;
	double * values = new double[sizec];
	stream_->read( reinterpret_cast<char *>(values), sizeof(double) * sizec);
	*size = sizec;
//	std::cout << "returning from read_double_array_siox" << std::endl;
	return values;
}

size_t BinaryInputStream::read_size_t(){
	size_t value;
	stream_->read( reinterpret_cast<char *>(&value), sizeof(value));
	return value;
}

BinaryInputByteStream::BinaryInputByteStream(char* char_stream, int size){
	char_stream_ = new CharStreamBuf(char_stream, char_stream + size);
	this->stream_ = new std::istream(char_stream_);
}

BinaryInputByteStream::~BinaryInputByteStream(){
	delete char_stream_;
}


} /*namespace setup*/
