/*
 * sip_server.h
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#ifndef SIP_SERVER_H_
#define SIP_SERVER_H_

#include "sip_tables.h"
#include "blocks.h"
#include "data_distribution.h"
#include "persistent_array_manager.h"
#include "sip_mpi_data.h"

namespace sip {

class SIPServer {
public:
	SIPServer(SipTables&, DataDistribution&, SIPMPIAttr &, PersistentArrayManager&, PersistentArrayManager&);
	~SIPServer();

	typedef std::map<BlockId, Block::BlockPtr> IdBlockMap;
	typedef IdBlockMap* IdBlockMapPtr;
	typedef std::vector<IdBlockMapPtr> BlockMap;

	/**
	 * The servers run this loop to serve up blocks
	 */
	void run();

private:

	/**
	 * MPI Attributes of the SIP for this rank
	 */
	SIPMPIAttr & sip_mpi_attr_;

	/**
	 * Data distribution scheme
	 */
	DataDistribution &data_distribution_;

	/**
	 * Reference to the static Sip Tables data.
	 */
	SipTables &sip_tables_;

	/**
	 * persistent block manager instance to read data from previous sial file.
	 */
	PersistentArrayManager& pbm_read_;

	/**
	 * persistent block manager instance to read data from previous sial file.
	 */
	PersistentArrayManager& pbm_write_;

	/**
	 * Stores distributed blocks
	 */
	BlockMap block_map_;

	/**
	 * Last pardo section for which a request was served.
	 * The invariant for correctness is
	 * section(any_incoming_request) > section_number_
	 */
	int section_number_;

	/**
	 * Auxiliary structure to keep track of data for pending puts & put_accumulates
	 */
	class TagInfo {
	public:
		int from;
		int section_number;
		int message_number;
		TagInfo(int f, int s, int m): from(f), section_number(s), message_number(m){}
		bool operator< (const TagInfo &other) const {
			if (from < other.from) return true;
			if (section_number < other.section_number) return true;
			if (message_number < other.message_number) return true;
			return false;
		}
	};

	/**
	 * Keeps track of block data for pending puts & put_accumulates.
	 */
	typedef std::pair<BlockId, Block::BlockPtr> IdBlockPair;
	typedef std::map<TagInfo, IdBlockPair> TagInfoIdBlockPairMap;
	TagInfoIdBlockPairMap outstanding_put_data_map_;


	/**
	 * Deletes all blocks from given array
	 * @param array_id
	 */
	void delete_array(int array_id);

	/**
	 * Get array id from a blocking receive from given rank.
	 * @param rank
	 * @param tag
	 * @return
	 */
	int get_int_from_rank(const int rank, int *tag);

	/**
	 * Send integer to other servers (from master server)
	 * @param to_send
	 * @param tag
	 */
	void send_to_other_servers(int to_send, int tag);

	/**
	 * Send char* to other servers (from master server)
	 * @param str [in]
	 * @param len [in]
	 * @param tag [in]
	 */
	void send_to_other_servers(const char *str, int len, int tag);

//	/**
//	 * Gets a string from the given rank.
//	 * @param rank [in]
//	 * @param size [out]
//	 */
//	std::string get_string_from(int rank, int &size);

	/**
	 * Any work to be done before server exits
	 */
	void post_program_processing();

	/**
	 * Restores a persistent array
	 * @param label
	 * @param array_id
	 */
	void restore_persistent_array(const std::string& label,
			int array_id);

	/**
	 * Print an object of SIPServer
	 * @param os
	 * @param obj
	 * @return
	 */
	friend std::ostream& operator<<(std::ostream& os, const SIPServer& obj);

	// Handles messages received by the server
	void handle_GET(int mpi_source);
	void handle_PUT(int mpi_source);
	void handle_PUT_DATA(int mpi_source, int size, int tag);
	void handle_PUT_ACCUMULATE(int mpi_source);
	void handle_PUT_ACCUMULATE_DATA(int mpi_source, int size, int tag);
	void handle_DELETE(int mpi_source);
	void handle_BARRIER();
	void handle_END_PROGRAM(int mpi_source);
	void handle_SAVE_PERSISTENT(int mpi_source);
	void handle_RESTORE_PERSISTENT(int mpi_source);
};

} /* namespace sip */

#endif /* SIP_SERVER_H_ */
