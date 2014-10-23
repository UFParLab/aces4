/*
 * profile_timer_store.cpp
 *
 *  Created on: Oct 4, 2014
 *      Author: jindal
 */

#include "profile_timer_store.h"

#include <sqlite3.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>

#include "sip.h"

namespace sip {

const int ProfileTimerStore::MAX_BLOCK_OPERANDS = 6;

// Names of SQLITE3 TABLES
const std::string ProfileTimerStore::block_table[] = {
	"zero_block_table",
	"one_block_table",
	"two_block_table",
	"three_block_table",
	"four_block_table",
	"five_block_table",
	"six_block_table"
};

// CHARACTER Columns
const std::string ProfileTimerStore::opcode_column("opcode");
const int ProfileTimerStore::max_opcode_size = 32;

// INTEGER Columns
const std::string ProfileTimerStore::rank_prefix("rank");
const std::string ProfileTimerStore::block_prefix("block");
const std::string ProfileTimerStore::indices_prefix("index");
const std::string ProfileTimerStore::segment_prefix("segment");

// BIGINT COLUMN
const std::string ProfileTimerStore::tottime_column("totaltime");

// BIGINT COLUMN
const std::string ProfileTimerStore::count_column("count");

void ProfileTimerStore::sip_sqlite3_error(int rc) const {
	// Print error message and exit if invalid
	std::stringstream ss;
	ss << "Error in ProfileTimerStore : ";
	ss << sqlite3_errstr(rc) << std::endl;
	throw std::runtime_error(ss.str());
}

/**
 * This methods creates a table for zero blocks parameters, 1 block, 2 blocks...6 blocks.
 * The table columns for 1 block are named like so:
 * opcode | block1index1 | block1index2 | ... | block1segment1 | block1segment2 | ... | totaltime | count
 * @param num_blocks
 */
void ProfileTimerStore::create_table(int num_blocks) {
	std::stringstream create_table_ss;
	create_table_ss << "CREATE TABLE IF NOT EXISTS "<< block_table[num_blocks] << "("
			        << opcode_column << " CHARACTER(" << max_opcode_size << ") NOT NULL, ";

	for (int bnum = 0; bnum < num_blocks; bnum++){
		create_table_ss << rank_prefix << bnum << " INTEGER NOT NULL,";
	}

	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int index = 0; index < MAX_RANK; index++) {
			create_table_ss << block_prefix << bnum << indices_prefix << index << " INTEGER  NOT NULL,";
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int segment = 0; segment < MAX_RANK; segment++) {
			create_table_ss << block_prefix << bnum << segment_prefix << segment << " INTEGER  NOT NULL,";
		}
	}

	create_table_ss << tottime_column << " BIGINT  NOT NULL,"	// in micro-seconds
			 	    << count_column << " BIGINT  NOT NULL,";

	// Create the uniqueness constraint over all columns except the totaltime & count columns
	create_table_ss << "CONSTRAINT opcode_operands_unique UNIQUE (";
	create_table_ss << opcode_column;
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int index = 0; index < MAX_RANK; index++) {
			create_table_ss << ", "<< block_prefix << bnum << indices_prefix << index ;
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int segment = 0; segment < MAX_RANK; segment++) {
			create_table_ss <<", " << block_prefix << bnum << segment_prefix << segment;
		}
	}
	create_table_ss << ")";  // End of Primary Key
	create_table_ss << ");"; // End of create table

	SIP_LOG(std::cout << "In create_table, Executing create table query " << create_table_ss.str() << std::endl);


	sqlite3_stmt* create_table_stmt;
	std::string create_table_sql(create_table_ss.str());

	int rc = sqlite3_prepare_v2(db_, create_table_sql.c_str(), create_table_sql.length(), &create_table_stmt, NULL);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	rc = sqlite3_step(create_table_stmt);
	if (rc != SQLITE_DONE)
		sip_sqlite3_error(rc);

	rc = sqlite3_finalize(create_table_stmt);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

}

void ProfileTimerStore::append_where_clause(std::stringstream& ss, const ProfileTimer::Key& opcode_operands) const {
	int num_blocks = opcode_operands.blocks_.size();
	ss << " WHERE " << opcode_column << "='" << opcode_operands.opcode_ << "'";

	for (int bnum = 0; bnum < num_blocks; bnum++) {
		int rank_to_search = opcode_operands.blocks_[bnum].rank_;
		ss << " and " << rank_prefix << bnum << " = " << rank_to_search;
	}

	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int index = 0; index < MAX_RANK; index++) {
			int index_to_search = opcode_operands.blocks_[bnum].index_ids_[index];
			ss << " and " << block_prefix << bnum << indices_prefix << index << " = " << index_to_search;
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int segment = 0; segment < MAX_RANK; segment++) {
			int segment_to_search = opcode_operands.blocks_[bnum].segment_sizes_[segment];
			ss << " and " << block_prefix << bnum << segment_prefix << segment << " = " << segment_to_search;
		}
	}
}

ProfileTimerStore::ProfileTimerStore(const std::string &db_location):
	db_location_(db_location), db_(NULL){

	int rc = sqlite3_open(db_location_.c_str(), &db_);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	// create the tables for 0 block operands, 1 block operands, .... 6 block operands
	for (int num_blocks = 0; num_blocks <=6; num_blocks++)
		create_table(num_blocks);

}

ProfileTimerStore::~ProfileTimerStore() {
	// Close the database.
	int rc = sqlite3_close(db_);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);
}

void ProfileTimerStore::save_to_store(const ProfileTimer::Key& opcode_operands, const std::pair<long, long>& const_time_count_pair){

	// TODO Upgrade to bound prepared statements - http://www.sqlite.org/c3ref/bind_blob.html

	std::pair<long, long> time_count_pair = const_time_count_pair;

	bool do_insert = false;

	// Get value from store if something exists & merge.
	try {
		std::pair<long, long> prev_timer_count_pair = get_from_store(opcode_operands);
		time_count_pair.first += prev_timer_count_pair.first;
		time_count_pair.second += prev_timer_count_pair.second;
	} catch (const std::invalid_argument& e){
		do_insert = true;
		// Do nothing
	}

	std::stringstream ss;	// Will contain the query
	int num_blocks = opcode_operands.blocks_.size();
	if (!do_insert) {
		ss << "UPDATE " << block_table[num_blocks];
		ss << " SET " << tottime_column << " = " << time_count_pair.first << ", "
			          << count_column << " = " << time_count_pair.second << " ";
		append_where_clause(ss, opcode_operands);
		ss << ";";
	} else {
		// Construct SQL Query String
		check (num_blocks <= MAX_BLOCK_OPERANDS, "Num of block operands exceeds maximum allowed");
		ss << "INSERT INTO " << block_table[num_blocks] << "(";
		ss << opcode_column ;

		for (int bnum = 0; bnum < num_blocks; bnum++) {
			ss << ", " << rank_prefix << bnum ;
		}
		for (int bnum = 0; bnum < num_blocks; bnum++) {
			for (int index = 0; index < MAX_RANK; index++) {
				ss << ", " << block_prefix << bnum << indices_prefix << index;
			}
		}
		for (int bnum = 0; bnum < num_blocks; bnum++) {
			for (int segment = 0; segment < MAX_RANK; segment++) {
				ss << ", " << block_prefix << bnum << segment_prefix << segment;
			}
		}
		ss << ", " << tottime_column;
		ss << ", " << count_column;
		ss << ")";

		// Values to put into database
		ss << " VALUES (";
		ss << "'" << opcode_operands.opcode_ << "'";
		const std::vector<ProfileTimer::BlockInfo> & blocks = opcode_operands.blocks_;

		for (int bnum = 0; bnum < num_blocks; bnum++) {
			ss << ", " << opcode_operands.blocks_[bnum].rank_;
		}

		for (int bnum = 0; bnum < num_blocks; bnum++) {
			for (int index = 0; index < MAX_RANK; index++) {
				ss << ", " << opcode_operands.blocks_[bnum].index_ids_[index];
			}
		}
		for (int bnum = 0; bnum < num_blocks; bnum++) {
			for (int segment = 0; segment < MAX_RANK; segment++) {
				int segment_to_search = opcode_operands.blocks_[bnum].segment_sizes_[segment];
				ss << ", " << opcode_operands.blocks_[bnum].segment_sizes_[segment];
			}
		}
		ss << ", " << time_count_pair.first;
		ss << ", " << time_count_pair.second;
		ss << ");";
		// ss now contains the complete query.
	}

	SIP_LOG(std::cout << "In save_to_store, Executing Insert query " << ss.str() << std::endl);

	std::string save_to_store_sql(ss.str());
	sqlite3_stmt* save_to_store_stmt;

	// Execute the SQL query string and insert the values into the database
	int rc = sqlite3_prepare_v2(db_, save_to_store_sql.c_str(), save_to_store_sql.length(), &save_to_store_stmt, NULL);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	rc = sqlite3_step(save_to_store_stmt);
	if (rc != SQLITE_DONE)
		sip_sqlite3_error(rc);

	rc = sqlite3_finalize(save_to_store_stmt);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

}

std::pair<long, long> ProfileTimerStore::get_from_store(const ProfileTimer::Key& opcode_operands) const {

	// TODO Upgrade to bound prepared statements - http://www.sqlite.org/c3ref/bind_blob.html
	// Specially needed in this routine, since it will be called by the modeling interpreter.

	// Construct SQL Query String
	int num_blocks = opcode_operands.blocks_.size();
	check (num_blocks <= MAX_BLOCK_OPERANDS, "Num of block operands exceeds maximum allowed");
	std::stringstream ss;
	ss << "SELECT "<< tottime_column << ", " << count_column  <<" FROM "
	   << block_table[num_blocks];

	append_where_clause(ss, opcode_operands);

	ss << ";";
	// ss now contains the complete query.

	SIP_LOG(std::cout << "In get_from_store, Executing Select query " << ss.str() << std::endl);

	std::string get_from_store_sql(ss.str());
	sqlite3_stmt* get_from_store_stmt;

	// Execute the SQL query string and get total time and count from the results.
	int rc = sqlite3_prepare_v2(db_, get_from_store_sql.c_str(), get_from_store_sql.length(), &get_from_store_stmt, NULL);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	long totaltime = -1;
	long count = -1;
	int row_count = 0;
	while (sqlite3_step(get_from_store_stmt) == SQLITE_ROW){
		totaltime = sqlite3_column_int64(get_from_store_stmt, 0); 	// 1st column is tottime_column
		count = sqlite3_column_int64(get_from_store_stmt, 1);		// 2nd column is count_column
		row_count++;
	}

	// If no rows were returned, throw an exception.
	if (row_count < 1){
		std::stringstream err_ss;
		err_ss << "There were no rows for key : " << opcode_operands;
		rc = sqlite3_finalize(get_from_store_stmt);
		if (rc != SQLITE_OK)
			sip_sqlite3_error(rc);
		throw std::invalid_argument(err_ss.str());
	}

	// Make sure there is just one row in the result.
	if (row_count > 1){
		std::stringstream err_ss;
		err_ss << "There are " << row_count << " rows for Key " << opcode_operands;
		rc = sqlite3_finalize(get_from_store_stmt);
		if (rc != SQLITE_OK)
			sip_sqlite3_error(rc);
		//fail(ss.str());
		throw std::out_of_range(err_ss.str());
	}

	rc = sqlite3_finalize(get_from_store_stmt);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	return std::make_pair(totaltime, count);
}


ProfileTimerStore::ProfileStoreMap_t ProfileTimerStore::read_all_data() const{
	std::map<ProfileTimer::Key, std::pair<long, long> > all_rows_map;

	for (int operands=0; operands<=MAX_BLOCK_OPERANDS; ++operands){
		std::stringstream query_ss;
		query_ss << "SELECT * FROM " << block_table[operands];
		std::string sql_query(query_ss.str());

		sqlite3_stmt* get_all_rows_from_table;
		// Execute the SQL query string and get total time and count from the results.
		int rc = sqlite3_prepare_v2(db_, sql_query.c_str(), sql_query.length(), &get_all_rows_from_table, NULL);
		if (rc != SQLITE_OK)
			sip_sqlite3_error(rc);

		while (sqlite3_step(get_all_rows_from_table) == SQLITE_ROW){
			int col_num = 0;
			// Get opcode
			std::string opcode(reinterpret_cast<const char*>(sqlite3_column_text(get_all_rows_from_table, col_num++)));

			// Get ranks
			int ranks[MAX_RANK];
			for (int bnum=0; bnum<operands; ++bnum){
				ranks[bnum] = sqlite3_column_int(get_all_rows_from_table, col_num++);
			}

			// Get indices
			int indices[MAX_RANK][MAX_RANK];
			for (int bnum = 0; bnum < operands; bnum++) {
				for (int index = 0; index < MAX_RANK; index++) {
					indices[bnum][index] = sqlite3_column_int(get_all_rows_from_table, col_num++);
				}
			}

			// Get segment sizes
			int segments[MAX_RANK][MAX_RANK];
			for (int bnum = 0; bnum < operands; bnum++) {
				for (int segment = 0; segment < MAX_RANK; segment++) {
					segments[bnum][segment] = sqlite3_column_int(get_all_rows_from_table, col_num++);
				}
			}

			// Get total time & counts
			long totaltime = sqlite3_column_int64(get_all_rows_from_table, col_num++);
			long count = sqlite3_column_int64(get_all_rows_from_table, col_num++);

			std::pair<long, long> time_count_pair = std::make_pair(totaltime, count);
			std::vector<ProfileTimer::BlockInfo> block_infos;
			for (int i=0; i<operands; ++i){
				ProfileTimer::BlockInfo block_info (ranks[i], indices[i], segments[i]);
				block_infos.push_back(block_info);
			}
			ProfileTimer::Key key(opcode, block_infos);

			typedef std::map<ProfileTimer::Key, std::pair<long, long> >::iterator MapIterator_t;
			std::pair<MapIterator_t, bool> returned = all_rows_map.insert(std::make_pair(key, time_count_pair));
			sip::check(returned.second, "Trying to insert duplicate element from database into map in ProfileTimerStore");
		}
		rc = sqlite3_finalize(get_all_rows_from_table);
		if (rc != SQLITE_OK)
			sip_sqlite3_error(rc);
	}
	return all_rows_map;
}


void ProfileTimerStore::backup_to_other(const ProfileTimerStore& other){
	sqlite3_backup *pBackup;  	/* Backup object used to copy data */
	sqlite3 *pTo = other.db_;	/* Database to copy to (pFile or pInMemory) */
	sqlite3 *pFrom = this->db_; /* Database to copy from (pFile or pInMemory) */

	pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
	if( pBackup ){
	  sqlite3_backup_step(pBackup, -1);
	  sqlite3_backup_finish(pBackup);
	}
	int rc = sqlite3_errcode(pTo);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);
}

void ProfileTimerStore::merge_from_other(const ProfileTimerStore& other){
	ProfileStoreMap_t other_map = other.read_all_data();
	for (ProfileStoreMapIterator_t it = other_map.begin(); it != other_map.end(); ++it){
		ProfileStoreMapIterator_t found_it = other_map.find(it->first);

		// If the record was found, merge it and put it in
		if (found_it != other_map.end()){
			long new_total_time = it->second.first + found_it->second.first;
			long new_count = it->second.second + found_it->second.second;
			std::pair<long, long> new_timer_count_pair = std::make_pair(new_total_time, new_count);
			this->save_to_store(it->first, new_timer_count_pair);

		} else { // Otherwise just put it in
			this->save_to_store(it->first, it->second);
		}
	}
}


} /* namespace sip */
