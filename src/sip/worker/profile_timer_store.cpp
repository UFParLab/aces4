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

#include "sip.h"

namespace sip {

const int ProfileTimerStore::MAX_BLOCK_OPERANDS = 5;

// Names of SQLITE3 TABLES
const std::string ProfileTimerStore::block_table[] = {
	"zero_block_table",
	"one_block_table",
	"two_block_table",
	"three_block_table",
	"four_block_table",
	"five_block_table"
};

// CHARACTER Columns
const std::string ProfileTimerStore::opcode_column("opcode");
const int ProfileTimerStore::max_opcode_size = 32;

// INTEGER Columns
const std::string ProfileTimerStore::block_prefix("block");
const std::string ProfileTimerStore::indices_prefix("index");
const std::string ProfileTimerStore::segment_prefix("segment");

// DOUBLE PRECISION COLUMN
const std::string ProfileTimerStore::tottime_column("totaltime");

// BIGINT COLUMN
const std::string ProfileTimerStore::count_column("count");

void ProfileTimerStore::sip_sqlite3_error(int rc) {
	// Print error message and exit if invalid
	std::stringstream ss;
	ss << "Error in ProfileTimerStore : ";
	ss << sqlite3_errstr(rc) << std::endl;
	throw std::runtime_error(ss.str());
}

/**
 * This methods creates a table for zero blocks parameters, 1 block, 2 blocks...5 blocks.
 * The table columns for 1 block are named like so:
 * opcode | block1index1 | block1index2 | ... | block1segment1 | block1segment2 | ... | totaltime | count
 * @param num_blocks
 */
void ProfileTimerStore::create_table(int num_blocks) {
	std::stringstream create_table_ss;
	create_table_ss << "CREATE TABLE IF NOT EXISTS "<< block_table[num_blocks] << "("
			        << opcode_column << " CHARACTER(" << max_opcode_size << "), ";

	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int index = 0; index < MAX_RANK; index++) {
			create_table_ss << block_prefix << bnum << indices_prefix << index << " INTEGER,";
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		for (int segment = 0; segment < MAX_RANK; segment++) {
			create_table_ss << block_prefix << bnum << segment_prefix << segment << " INTEGER,";
		}
	}

	create_table_ss << tottime_column << " DOUBLE PRECISION,"
			 	    << count_column << " BIGINT,";

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
	create_table_ss << ")";  // End of Constraint
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

ProfileTimerStore::ProfileTimerStore(const std::string &db_location):
	db_location_(db_location), db_(NULL){

	int rc = sqlite3_open(db_location_.c_str(), &db_);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	// create the tables for 0 block operands, 1 block operands, .... 5 block operands
	for (int num_blocks = 0; num_blocks <=5; num_blocks++)
		create_table(num_blocks);

}

ProfileTimerStore::~ProfileTimerStore() {
	// Close the database.
	int rc = sqlite3_close(db_);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);
}

void ProfileTimerStore::save_to_store(const ProfileTimer::Key& opcode_operands, const std::pair<double, int>& time_count_pair){

	// TODO Upgrade to bound prepared statements - http://www.sqlite.org/c3ref/bind_blob.html

	// Construct SQL Query String
	int num_blocks = opcode_operands.blocks_.size();
	check (num_blocks <= MAX_BLOCK_OPERANDS, "Num of block operands exceeds maximum allowed");
	std::stringstream ss;
	ss << "INSERT INTO " << block_table[num_blocks] << "(";
	ss << opcode_column ;
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		int rank = opcode_operands.blocks_[bnum].rank_;
		for (int index = 0; index < rank; index++) {
			ss << ", " << block_prefix << bnum << indices_prefix << index;
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		int rank = opcode_operands.blocks_[bnum].rank_;
		for (int segment = 0; segment < rank; segment++) {
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
		int rank = opcode_operands.blocks_[bnum].rank_;
		for (int index = 0; index < rank; index++) {
			ss << ", " << opcode_operands.blocks_[bnum].index_ids_[index];
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		int rank = opcode_operands.blocks_[bnum].rank_;
		for (int segment = 0; segment < rank; segment++) {
			int segment_to_search = opcode_operands.blocks_[bnum].segment_sizes_[segment];
			ss << ", " << opcode_operands.blocks_[bnum].segment_sizes_[segment];
		}
	}
	ss << ", " << time_count_pair.first;
	ss << ", " << time_count_pair.second;
	ss << ");";
	// ss now contains the complete query.

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

std::pair<double, int> ProfileTimerStore::get_from_store(const ProfileTimer::Key& opcode_operands){

	// TODO Upgrade to bound prepared statements - http://www.sqlite.org/c3ref/bind_blob.html
	// Specially needed in this routine, since it will be called by the modeling interpreter.

	// Construct SQL Query String
	int num_blocks = opcode_operands.blocks_.size();
	check (num_blocks <= MAX_BLOCK_OPERANDS, "Num of block operands exceeds maximum allowed");
	std::stringstream ss;
	ss << "SELECT "<< tottime_column << ", " << count_column  <<" FROM "
	   << block_table[num_blocks]
	   << " WHERE "<< opcode_column << "='" << opcode_operands.opcode_ << "'";
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		int rank = opcode_operands.blocks_[bnum].rank_;
		for (int index = 0; index < rank; index++) {
			int index_to_search = opcode_operands.blocks_[bnum].index_ids_[index];
			ss << " and " << block_prefix << bnum << indices_prefix << index << " = " << index_to_search;
		}
	}
	for (int bnum = 0; bnum < num_blocks; bnum++) {
		int rank = opcode_operands.blocks_[bnum].rank_;
		for (int segment = 0; segment < rank; segment++) {
			int segment_to_search = opcode_operands.blocks_[bnum].segment_sizes_[segment];
			ss << " and "<< block_prefix << bnum << segment_prefix << segment << " = " << segment_to_search;
		}
	}
	ss << ";";
	// ss now contains the complete query.

	SIP_LOG(std::cout << "In get_from_store, Executing Select query " << ss.str() << std::endl);

	std::string get_from_store_sql(ss.str());
	sqlite3_stmt* get_from_store_stmt;

	// Execute the SQL query string and get total time and count from the results.
	int rc = sqlite3_prepare_v2(db_, get_from_store_sql.c_str(), get_from_store_sql.length(), &get_from_store_stmt, NULL);
	if (rc != SQLITE_OK)
		sip_sqlite3_error(rc);

	double totaltime = -1.0;
	long count = -1;
	int row_count = 0;
	while (sqlite3_step(get_from_store_stmt) == SQLITE_ROW){
		totaltime = sqlite3_column_double(get_from_store_stmt, 0); 	// 1st column is tottime_column
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
		throw std::out_of_range(err_ss.str());
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



} /* namespace sip */
