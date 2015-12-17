/*
 * array_file.h
 *
 *  Created on: Aug 10, 2015
 *      Author: basbas
 */

#ifndef ARRAY_FILE_H_
#define ARRAY_FILE_H_
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <mpi.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "sip.h"
#include "chunk.h"
#include "data_distribution.h"
#include "counter.h"


namespace sip {

class Chunk;
class ChunkManager;
/** Encapsulates file IO for server disk backing and persistence.
 *
 * Responsible for converting array names and persistent array labels to file names.
 *
 * A temporary file for distributed/served array A is opened when the DiskBackedBlockManager
 * is constructed.  It's name is constructed from a job-specific prefix, the array name,
 * the current sial program number, and a fixed suffix, ".arr".  This file does not have
 * corresponding index file.  At the end of the current sial program, this file is
 * either deleted, or if it was marked persistent, an index file is created and this file
 * is renamed with a name with the .parr suffix constructed from the given label
 *
 * If the array has been marked persistent, then this class holds its label.  When the ArrayFile
 * object is destructed (which happens at the end of the sial program) the file is closed.
 * The temporary files are either deleted
 * (if they are not persistent) or, an index is created, and the temporary file is renamed.
 * The exception is if the save_on_close parameter is true. In that case, which is intended
 * to be used for testing, the temporary, non-persistent file is not deleted.
 *
 * The persistent file has a filename constructed from the job-specific prefix, the label,
 * the current sial program number, and a fixed suffix, ".parr".  The corresponding index file
 * has the same name except the suffix is ".arr_index".
 *
 * The data file format
 *
 * <file> ::= <header> <data>
 *
 *
 * <header> ::=  <header_val_t chunk_size> <header_val_t num_servers>
 * <data> ::= <double>*
 *
 *
 * <index> ::=  <index_type><number of MPI_Offset values in index><MPI_Offset>*
 *
 * <index_type> ::= DENSE_INDEX | SPARSE_INDEX
 *
 *
 *
 * Each server is allocated disk space in chunk_size sized chunks (chunk_size is in units of
 * double) in a round robin fashion.
 *
 * Invariant:  data file view is set for reading and writing data.  Thus methods that change the view must
 * reset it to the view for data.
 *
 * Invariant:  every file with suffix parr has a matching index file with suffix .parr_index
 *
 * Invariant:  only the process with comm_rank()==0 performs non-collective
 *   operations on files
 *
 * Preconditions:  init_statics should be called before any instance of this class is instantiated.
 *
 * The prog_num should be the number of the sial program so is increasing and
 * unique for each sial program in the run. This property is required as it allows the sip to find the
 * most recent file with a given prefix and label. It must also have the same value at all ranks.
 *
 * The prefix must be common at all ranks and constant throughout the entire job.
 * It is best if it is unique for each run of the program.  Constructing it from the
 * time-stamp is recommended.
 *
 *
 *
 *
 *
 */
class ArrayFile {
public:

	/** these definitions define the types of values in the header and index.
	 *
	 * Note header_val_t and index_val_t are defined to be c++ types,
	 * as is MPI_Offset  The latter's size is supposed to be large enough to handle
	 * the largest files supported on the given system.
	 *
	 * In contrast MPI_OFFSET_VAL_T and MPI_HEADER_VAL_T are MPI_Datatype (and
	 * can be passed as type arguments to MPI routines.)
	 *
	 * The c++ types must match the MPI_Datatype.  For example, if the c++
	 * type is int, the MPI_Datatype should be MPI_INT.
	 *
	 * WARNING:  MPI_Offset is implementation/configuration specific.
	 * TODO fix more generally
	 */

	typedef int header_val_t;
#define  MPI_HEADER_VAL_T MPI_INT

	typedef MPI_Offset offset_val_t;
#define  MPI_OFFSET_VAL_T  MPI_LONG_LONG

	const static int NUM_VALS_IN_HEADER = 2;
	const static int NUM_VALS_IN_INDEX_HEADER = 2;
	const static int DENSE_INDEX = 77;
	const static int SPARSE_INDEX = 88;

	const static offset_val_t ABSENT_BLOCK_OFFSET;

	const static std::string& PERSISTENT_SUFFIX;
	const static std::string& INDEX_SUFFIX;
	const static std::string& TEMP_SUFFIX;
	/**
	 * Creates a temporary file to back blocks in an array.
	 * It is an error if a file with the constructed name already exists.
	 *
	 * This is a collective operation among the given communicator.
	 *
	 * @param chunk_size
	 * @param file_name
	 * @param comm
	 *
	 *
	 * TODO add more info about array to header and check when reopened.
	 */

	ArrayFile(header_val_t chunk_size,
			const std::string& name, const MPI_Comm& comm,
			bool save_after_close = false);

	/**
	 *
	 * The file is closed.
	 *
	 * If the array is not persistent, the backing file
	 * and its associated index file, if there is one,
	 * are deleted (unless save_after_close_ is true)
	 *
	 * If the array is persistent and not was_restored_
	 * the following preconditions (which
	 * can be established by calling finalize_persisent) apply:
	 * 1. the header has been written
	 * 2. the index file exists with name *.label.#.parr_array
	 * The temp file is closed and renamed to *.label.curr#.parr
	 *
	 * If the array is persistent and was_restored_
	 * the following preconditions apply:
	 * 1.  the backing file has name *.l0.#.parr and the
	 * for some l0
	 * 2.  the index file exists with name *.l0.#.parr_index
	 * The backing file is closed and renamed to *.label.curr#.parr
	 * The index file is closed and renamed to *.label.curr#.parr_index
	 *
	 * This is a collective operation
	 */
	~ArrayFile();





	/**
	 * Writes the given chunk to disk.
	 *
	 * This is NOT a collective operation.
	 *
	 * @param chunk
	 */
	void chunk_write(const Chunk & chunk);

	/**
	 * Collectively writes the given chunk to disk.
	 *
	 * This is a collective operation:  all members of the comm write a chunk in same operation.
	 * (Naming adopted from MPI)
	 *
	 * @param chunk
	 */
	void chunk_write_all(const Chunk & chunk);


	/**
	 * Operation that can be used with chunk_write_all if this server does not
	 * have anything to write.
	 */
	void chunk_write_all_nop() const;

	/**
	 * Reads the data for the given chunk from disk.
	 * Precondition:  chunk memory has been allocated
	 *
	 * This is NOT a collective operation
	 *
	 * @param chunk
	 */
	void chunk_read(Chunk& chunk);

	/**
	 * Collectively reads the data for the given chunk from disk
	 * Precondition:  chunk memory has been allocated
	 *
	 * This is a collective operation
	 *
	 * @param chunk
	 */
	void chunk_read_all(Chunk & chunk);


	/**
	 * Operation that can be used with chunk_read_all if this server does not hav
	 * anything to read
	 */
	void chunk_read_all_nop();



//	//const_cast<char *>(file_name.c_str())
//	bool rename_persistent(){
//		if (comm_rank() == 0){
//
//		std::string name = file_name();
//		std::string new_name = persistent_file_name();
//		if (name.compare(new_name)==0) return false;
//		int err = std::remove(const_cast<char *>(new_name.c_str()));
//		//note that here the error is that the remove succeeded.
//		WARN(err != 0, std::string("Deleted existing persistent file ") + new_name);
////		std::cout << "renaming " << name << " to " << new_name << std::endl << std::flush;
//		err = std::rename(const_cast<char*>(name.c_str()),
//				const_cast<char*>(new_name.c_str()));
//		if (err != 0){
//			perror("Error renaming persistent array");
//			CHECK(false,"");
//		}
//		return true;
//		}
//		return false;
//	}


////TODO  fix this to work with time stamps.  For now, just initialize
//			static void init_statics(std::string filename_prefix,
//					int prog_num);


			struct Stats {
				MPICounter chunks_written_;
				MPICounter chunks_restored_;

				explicit Stats(const MPI_Comm& comm) :
						chunks_written_(comm), chunks_restored_(comm) {
				}

				std::ostream& gather_and_print_statistics(std::ostream& os, ArrayFile* parent) {
					chunks_written_.gather();
					chunks_restored_.gather();
					if (SIPMPIAttr::get_instance().is_company_master()) {
						os << "chunks_written"<< std::endl;
						os << chunks_written_;
						os << "chunks_restored"<< std::endl;
						os << chunks_restored_ << std::endl;
					}
					return os;
				}
			};

			/**
			 * deletes all files with suffix .arr, .parr, and .parr_index
			 * from the current directory.
			 *
			 * This should only be called by a single process
			 */
			static void clean_directory(){
   				if (SIPMPIAttr::get_instance().global_rank()==0) {
				DIR *dp;
				struct dirent *ep;
				dp = opendir("./");
				if (dp == NULL) {
					perror("Could not open current directory for clean_directory operation.");
					return; //print the error message, but leave error handling to caller
				}
				for (ep = readdir(dp); ep ; ep = readdir(dp)){
					std::string file_name = ep->d_name;;
					std::vector<std::string> file_name_parts;
					parse_file_name(file_name, file_name_parts);
					std::vector<std::string>::reverse_iterator it = file_name_parts.rbegin();
                    if (*it == PERSISTENT_SUFFIX || *it == INDEX_SUFFIX || *it == TEMP_SUFFIX){

        					int err = std::remove(
        							const_cast<char *>(file_name.c_str()));
//        					if (err != 0) {
//        						perror(("Error deleting file" + file_name).c_str());
//        						//TODO something sensible here
//        					}
        				}
                    }
				}
			}
			friend DiskBackedBlockMap;
			friend ChunkManager;
			friend std::ostream& operator<<(std::ostream& os,
					const ArrayFile& obj);
			void init_statics(std::string filename_prefix, int prog_num);

			/***************************/
		private:

			const MPI_Comm& comm_;
			header_val_t chunk_size_;
			std::string name_;  //name of array

			MPI_File fh_; //handle to opened file backing_file_name_
			std::string backing_file_name_;

			bool was_restored_; //indicates if array was restored from persistent
			std::string index_file_name_; //"" if index_fh_ == NULL,
			      // or name of opened index file.

//			header_val_t num_blocks_;  //if dense, this is the number of possible blocks, if sparse,
			                           //this is the number of actual blocks

			bool is_persistent_; //initially false, set when array marked persistent.
			std::string label_;  //if (is_persistent_, then this is the label),
			                     //otherwise ""

			bool save_after_close_;  //used in tests.
			Stats stats_;
/*******************************/
			/**
			 * make file name for temp file for array.  The suffix is ".arr"
			 */
			std::string make_temp_file_name(std::string& name) const {
				std::stringstream ss;
				ss << JobControl::global->get_job_id() << '.' << name << '.' << JobControl::global->get_program_num() << '.' << TEMP_SUFFIX;
				return ss.str();
			}
/**
 * make file name for persistent file with given label.  The suffix is ".parr"
 */

			std::string make_persistent_file_name(
					std::string& label) const {
				std::stringstream ss;
				ss << JobControl::global->get_job_id() << '.' << label << '.' << JobControl::global->get_program_num() << '.' << PERSISTENT_SUFFIX;
				return ss.str();
			}

			/**
			 * //make file name for index file with given label.  The suffix is ".parr_index"
			 */

			std::string make_index_file_name(std::string& label) const {
				std::stringstream ss;
				ss << JobControl::global->get_job_id() << '.' << label << '.' << JobControl::global->get_program_num() << '.' << INDEX_SUFFIX;
				return ss.str();
			}

			/**
			 * Finds and opens a persistent file with the given label and reads its header.
			 * Finds and opens the corresponding index file and reads the index.
			 * Closes and deletes the temp file for this array that was created during initialization.
			 *
			 * It is the callers job to process the index and add entries to the array's map.
			 *
			 * This is a collective operation
			 *
			 * @param[in] label
			 * @param[out] index_type
			 * @param[out] num blocks in index
			 * @param[out] index
			 *
			 */
			void open_persistent_file(const std::string& label, offset_val_t& index_type, offset_val_t& num_blocks, std::vector<offset_val_t>& index);

			/**
			 * Rank 0 writes header.  Other ranks just return.
			 */
			void write_index_header(MPI_File& index_fh, offset_val_t index_type, offset_val_t num_blocks){

			}

			/**
			 * Rank 0 reads and broadcasts index header.  Header data returned in reference parameters
			 *
			 * @param[in] index_fh
			 * @param[out] index_type
			 * @param[out] num_blocks   number of entries in index
			 *
			 * This is a collective operation.
			 */
			void read_index_header(MPI_File& index_fh, offset_val_t& index_type, offset_val_t& index_size){

			}
/**
 * Comm rank 0 process opens and reads index file, and broadcast contents
 * to other in comm.
 *
 * The index type  is read from the header.  The number of values read can
 * be obtained from the size of the index vector.
 *
 * This file simply reads the values in the index file.  It does not interpret
 * them or handle the different formats.
 *
 * @param[in] index_file_name
 * @param[out]  index_type   index type read from file header
 * @param index[inout]  empty vector is passed in.  This method resizes and fills
 *           the vector with the data from the index file.
 *
 *
 */
			void read_index(const std::string& index_file_name, offset_val_t& index_size, offset_val_t& index_type, std::vector<offset_val_t>& index);

			/**
			 * Takes an index in the form of an array with an entry for every possible
			 * block in this array.  Entries corresponding to locally owned blocks
			 * contain their file offset.  The value for non-locally owned blocks is
			 * DiskBackedBlockMap::ABSENT_BLOCK_OFFSET, which is required to be less than
			 * any valid offset (i.e. <0).
			 *
			 * Reduces the results to obtain a global index.
			 *
			 * The master server then writes the index to the index file.
			 *
			 * This is a collective operation
			 *
			 * @param index
			 */
			void write_dense_index(std::vector<ArrayFile::offset_val_t>& index);

//			/**
//			 * Reads the index of an array file and broadcast to other servers.
//			 *
//			 * Preconditions:
//			 *
//			 * the headers have already been read, so num_blocks_ and chunk_size_ are
//			 * initialized.
//			 *
//			 * index has been allocated.
//			 *
//			 *This is a collective operation
//			 *
//			 * @param index
//			 */
//			 void read_dense_index(std::vector<offset_val_t> index, header_val_t size) {
////				CHECK(size == num_blocks_,
////						"array passed to read_index may not have correct size");
//				MPI_Offset displacement = NUM_VALS_IN_HEADER
//				* sizeof(header_val_t);
//				int err = MPI_File_set_view(fh_, displacement,
//						MPI_OFFSET_VAL_T,
//						MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
//				MPI_Status status;
//				if (comm_rank() == 0) {
//					err = MPI_File_read_at(fh_, 0, &index.front(), size,
//							MPI_OFFSET_VAL_T, &status);
//					CHECK(err == MPI_SUCCESS, "failure reading file index");
//				}
//				err = MPI_Bcast(&index.front(), size, MPI_OFFSET_VAL_T, 0,
//						comm_);
//				CHECK(err == MPI_SUCCESS, "index broadcast failed");
//				set_view_for_data();
//			}
//
//		//	void read_sparse_index(offset_val_t* index, header_val_t size) {
//		//		CHECK(size == num_blocks_,
//		//				"array passed to read_index may not have correct size");
//		//		MPI_Offset displacement = NUM_VALS_IN_HEADER
//		//		* sizeof(header_val_t);
//		//		int err = MPI_File_set_view(fh_, displacement,
//		//				MPI_OFFSET_VAL_T,
//		//				MPI_OFFSET_VAL_T, "native", MPI_INFO_NULL);
//		//		MPI_Status status;
//		//		if (comm_rank() == 0) {
//		//			err = MPI_File_read_at(fh_, 0, index, num_blocks_,
//		//					MPI_OFFSET_VAL_T, &status);
//		//			CHECK(err == MPI_SUCCESS, "failure reading file index");
//		//		}
//		//		err = MPI_Bcast(index, num_blocks_, MPI_OFFSET_VAL_T, 0,
//		//				comm_);
//		//		CHECK(err == MPI_SUCCESS, "index broadcast failed");
//		//		set_view_for_data();
//		//	}

			void write_sparse_index(std::vector<offset_val_t>& index);
			void mark_persistent(const std::string& label);
			void close_and_rename_persistent();

			//read size values into the given buffer starting at offset doubles relative to the data view
			void read_doubles(double * data, size_t size,
					MPI_Offset offset) {
				MPI_Status status;
				int err = MPI_File_read_at(fh_, offset, data, size,
						MPI_DOUBLE, &status);
				CHECK(err == MPI_SUCCESS, "chunk_read_all failed");
			}

			void sync() {
				MPI_File_sync (fh_);
			}

			int comm_rank() const;

			int comm_size() const {
				int size;
				MPI_Comm_size(comm_, &size);
				return size;
			}

			/**
			 * This is a collective operation to set the view to write unsigned values.
			 * Then the communicator's master writes the header data.
			 * This is followed by a collective operation to set the view for
			 * writing data (i.e. set_view_for_data())
			 *
			 * This is a collective operation
			 *
			 */
			void write_header();



			/**
			 * This is a collective operation to read the header.  In this implementation,
			 */
			void read_header();

		//	/**
		//	 * Causes failure if the number of blocks or chunk size exceed the current implementation limits.
		//	 */
		//	void check_implementation_limits() {
		//		CHECK(
		//				num_blocks_
		//				< std::numeric_limits<offset_val_t>::max()
		//				/ chunk_size_,
		//				"implementation limit encountered:  type used to represent offsets in the index file may not be big enough");
		//		CHECK(num_blocks_ < std::numeric_limits<int>::max(),
		//				"implemenation limit enountered:  number of blocks too large for int used when writing index file");
		//	}


			/**
			 * Sets the MPI_IO file view to skip the header and count doubles (instead of bytes)
			 *
			 * This is a collective operation.
			 */
			void set_view_for_data();

			/**
			 * Precondition:  old_file_name exists
			 *
			 * @param old_file_name
			 * @param new_file_name
			 * @return
			 *
			 * If no file with new_file_name exists, rename old_file_name to new_file_name and return
			 * true.
			 *
			 * Otherwise do nothing and return false.
			 */
			static bool rename(std::string& old_file_name,
					std::string& new_file_name) {
				if (file_exists(new_file_name)) {
					return false;
				}
				int err = std::rename(
						const_cast<char*>(old_file_name.c_str()),
						const_cast<char*>(new_file_name.c_str()));
				if (err != 0) {
					perror("Error renaming file");
					CHECK(false,
							old_file_name + std::string(" to ")
							+ new_file_name);
				}
				return true;
			}

			static void parse_file_name(const std::string& file_name,
					std::vector<std::string>& file_name_parts) {
				file_name_parts.clear();
				size_t start = 0;
				size_t end = file_name.find('.');
				while (true) {
					if (end == std::string::npos) {
						file_name_parts.push_back(file_name.substr(start, end));
						return;
					}
					file_name_parts.push_back(file_name.substr(start, end - start));
					start = end + 1;  //position past the dot.
					end = file_name.find('.', start); //position of next dot in filename, or npos if none.
				}
			}




			/**
			 * Opens the current directory and finds the name of a
			 * file whose name was constructed from the given label,
			 * or the empty string if no matching file was found and
			 * whose program number entry is less than the current_prog_num.
			 * If there are multiple such matching files, it takes the latest one.
			 *
			 *
			 * @param label
			 * @return
			 */
			static std::string most_recent_matching_file(const std::string& jobid,
					const std::string& label, int current_prog_num, const std::string& suffix) {
				std::string found("");
				int found_prog_num = 0;
				DIR *dp;
				struct dirent *ep;
				dp = opendir("./");
				if (dp == NULL) {
					perror("Could not open current directory");
					return found; //print the error message, but leave error handling to caller
				}
				for (ep = readdir(dp); ep ; ep = readdir(dp)){
					std::string file_name = ep->d_name;;
					std::vector<std::string> file_name_parts;
					parse_file_name(file_name, file_name_parts);
					std::vector<std::string>::iterator it = file_name_parts.begin();
					if (*it != jobid) continue;  //prefix is wrong, doesn't belong to this job
					++it;
					if (*it != label) continue; //doesn't match label
					++it;
					int filename_prog_num;
					//try to convert the prog num part of the filename to an int.
					std::istringstream is(*it);
                    is >> filename_prog_num;
                    if (is.rdstate() & std::istringstream::failbit != 0) continue; //part could not be converted to int
                    if (filename_prog_num >= current_prog_num) continue;
                    ++it;
                    if (*it != suffix) continue; //suffix is wrong
                    ++it;
                    if (it != file_name_parts.end()) continue;  //more stuff that shouldn't be there after suffix
                    //this looks like a good file, check if this is the most recent.
                    if (filename_prog_num > found_prog_num) found_prog_num = filename_prog_num; //this file is more recent
                    found = file_name;
                    found_prog_num = filename_prog_num;
				}
				return found;
			}

					static bool file_exists(const std::string& file_name);

//					struct Stats {
//
//						MPICounter chunks_written_;
//						MPICounter chunks_restored_;
//
//						explicit Stats(const MPI_Comm& comm) :
//								chunks_written_(comm), chunks_restored_(comm) {
//						}
//
//						std::ostream& gather_and_print_statistics(std::ostream& os,
//								ArrayFile* parent) {
//							chunks_written_.gather();
//							chunks_restored_.gather();
//							if (SIPMPIAttr::get_instance().is_company_master()) {
//								os << "chunks_written" << std::endl;
//								os << chunks_written_;
//								os << "chunks_restored" << std::endl;
//								os << chunks_restored_ << std::endl;
//							}
//							return os;
//						}



//	static std::string make_persistent_file_name() const{
//		std::stringstream ss;
//		int phase = JobControl::global->get_program_num();
//		ss << "server." << name_ << "." << phase << ".arr";
//		return ss.str();
//	}



			void delete_file(const std::string& file_name) {
				//server master deletes file from the file system
				if (comm_rank() == 0) {
					int err = std::remove(
							const_cast<char *>(file_name.c_str()));
//					if (err != 0) {
//						perror(("Error deleting file" + file_name).c_str());
//						//TODO something sensible here
//					}
				}
			}



			DISALLOW_COPY_AND_ASSIGN(ArrayFile);
			}
			;

		} /* namespace sip */
//TODO  fix this to work with time stamps.  For now, just initialize
			void init_statics(std::string filename_prefix,
					int prog_num);
#endif /* ARRAY_FILE_H_ */
