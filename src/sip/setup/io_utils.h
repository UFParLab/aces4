/*! io_utils.h
 * 
 *
 *  Created on: Jul 7, 2013
 *      Author: Beverly Sanders
 */

#ifndef IO_UTILS_H_
#define IO_UTILS_H_

#include <string>
#include <iostream>
#include <fstream>
#include "sip.h"


namespace setup{

	/* Output Classes ============================================ */

   class OutputStream {
	public:
		virtual void write_string(const std::string&)=0;
		virtual void write_int(int)=0;
		virtual void write_int_array(int size, int[] )=0;
		virtual void write_double(double)=0;
		virtual void write_double_array(int size, double[])=0;
		virtual void write_size_t_val(size_t)=0;
		virtual ~OutputStream();
	protected:
		OutputStream();  //must use subclass
		std::ostream *stream_;
	private:
		DISALLOW_COPY_AND_ASSIGN(OutputStream);
   };

   class BinaryOutputStream : public OutputStream {
   public:
		virtual void write_string(const std::string&);
		virtual void write_int(int);
		virtual void write_int_array(int size, int[] );
		virtual void write_double(double);
		virtual void write_double_array(int size, double[]);
		virtual void write_size_t_val(size_t);
   };

	class BinaryOutputFile: public BinaryOutputStream{
	public:
		std::string get_file_name();

		BinaryOutputFile(const std::string&);
		virtual~BinaryOutputFile();
	private:
		std::string file_name_;
		std::ofstream *file_stream_;
		DISALLOW_COPY_AND_ASSIGN(BinaryOutputFile);
	};

	class BinaryOutputByteStream : public BinaryOutputStream{
	public:
		BinaryOutputByteStream(char* char_stream, int size);
		virtual ~BinaryOutputByteStream();
	private:
		struct CharStreamBuf : std::streambuf
		{
			CharStreamBuf(char* begin, char* end) {
				this->setg(begin, begin, end);
			}
		};
		CharStreamBuf *char_stream_;
		DISALLOW_COPY_AND_ASSIGN(BinaryOutputByteStream);
	};


	/* Input Classes ============================================ */

	class InputStream {
	public:

		virtual std::string read_string()=0;
		virtual int read_int()=0;
		virtual double read_double()=0;
		virtual int * read_int_array(int * size)=0;
		virtual double * read_double_array(int *size)=0;
		virtual double * read_double_array_siox(int *size)= 0;
		virtual size_t read_size_t()=0;
		virtual std::string * read_string_array(int * size)=0;
		virtual ~InputStream();
	protected:
		InputStream();
		std::istream *stream_;
	private:
		DISALLOW_COPY_AND_ASSIGN(InputStream);
	};

//	class EmptyInputStream : public InputStream {
//		virtual std::string read_string(){return std::string("");}
//		virtual int read_int(){return -1;}
//		virtual double read_double(){return -1;}
//		virtual int * read_int_array(int * size){return NULL;}
//		virtual double * read_double_array(int *size){return NULL;}
//		virtual double * read_double_array_siox(int *size){return NULL;};
//		virtual size_t read_size_t(){return 0;}
//		virtual std::string * read_string_array(int * size){return NULL;}
//	};

	class BinaryInputStream : public InputStream {
	public:
		virtual std::string read_string();
		virtual int read_int();
		virtual double read_double();
		virtual int * read_int_array(int * size);
		virtual double * read_double_array(int *size);
		virtual double * read_double_array_siox(int *size);
		virtual size_t read_size_t();
		virtual std::string * read_string_array(int * size);
	};

	class BinaryInputFile : public BinaryInputStream{
	public:
		virtual bool is_open(){
			return file_stream_!= NULL && stream_ !=NULL && file_stream_ -> is_open();
		}

		std::string get_file_name();
		BinaryInputFile(const std::string&);
        virtual ~BinaryInputFile();
	private:
        std::string file_name_;
        std::ifstream *file_stream_;
		DISALLOW_COPY_AND_ASSIGN(BinaryInputFile);
	};


	class BinaryInputByteStream : public BinaryInputStream {
	public:
		BinaryInputByteStream(char* char_stream, int size);
		virtual ~BinaryInputByteStream();
	private:
		// From http://stackoverflow.com/questions/7781898/get-an-istream-from-a-char
		struct CharStreamBuf : std::streambuf
		{
			CharStreamBuf(char* begin, char* end) {
		        this->setg(begin, begin, end);
		    }
		};
		CharStreamBuf *char_stream_;
		DISALLOW_COPY_AND_ASSIGN(BinaryInputByteStream);
	};


} /*namespace setup */

#endif /* IO_UTILS_H_ */
