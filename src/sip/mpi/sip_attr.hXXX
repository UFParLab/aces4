/*
    * sip_attr.h
 *
 *  Created on: July 19, 2014
 *      Author: bsanders
 */

#ifndef SIP_ATTR_H_
#define SIP_
#include "sip.h"


//TODO  come up with a less military naming scheme

namespace sip {

class SIPAttr {
public:

	~SIPAttr();

	static const int COMPANY_MASTER_RANK = 0;
	static SIPAttr *instance_;
	static bool destroyed_;

	// Singleton factory
	static SIPAttr& get_instance() ;

	/** Delete singleton instance */
	static void cleanup() ;


//	const std::vector<int>& server_ranks() const { return servers_; }
//	const std::vector<int>& worker_ranks() const { return workers_; }

	int num_servers() const { return 0; }
	int num_workers() const { return 1; }

	int global_rank() const { return 0; }
	int global_size() const { return 1; }

	// Each worker is in a worker company. Each server is in a server company.
	int company_rank() const { return 1; }
	int company_size() const { return 1; }
	bool is_company_master() const { return true; }

	bool is_worker() const {return true;}
	bool is_server() const {return false;}

	int worker_master() const { return 0; }
	int server_master() const { return 0; }

	int my_server(){return -1;}

	friend std::ostream& operator<<(std::ostream&, const SIPAttr&);


private:
	SIPAttr();

	DISALLOW_COPY_AND_ASSIGN(SIPAttr);

};

} /* namespace sip */

#endif /* SIP_ATTR_H_ */
