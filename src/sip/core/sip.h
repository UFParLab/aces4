/*! sip.h  SIP-wide definitions for coding standards, error handling, and various constants.
 *
 * Should be included in almost every program.
 * 
 *
 *  Created on: Jul 6, 2013
 *      Author: Beverly Sanders
 */

#ifndef SIP_H_
#define SIP_H_

#include "config.h"
#include <string>
#include <vector>
#include <map>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#define SETUP_FILE_TYPE_IS_BINARY 1

#ifdef SIP_DEVEL
	#define SIP_LOG(x) x
	#ifdef HAVE_MPI
		#define SIP_MASTER_LOG(x) if(sip::SIPMPIAttr::get_instance().global_rank() == 0) x;
		#define SIP_MASTER(x) SIP_MASTER_LOG(x)
	#else
		#define SIP_MASTER_LOG(x) x
		#define SIP_MASTER(x) x
	#endif
#else
	#define SIP_LOG(x) ;
	#define SIP_MASTER_LOG(x) ;
	#define SIP_MASTER(x) x
#endif


void sip_abort();

namespace sip {

/*!The values that should appear at the beginning of each data file */
extern const int SETUP_MAGIC;
extern const int SETUP_VERSION;

/*! The values that should appear at the beginning of each .siox file */
extern const int SIOX_MAGIC;
extern const int SIOX_VERSION;
extern const int SIOX_RELEASE;

typedef std::vector<double> ScalarTable;
typedef std::vector<std::string> StringLiteralTable;
typedef std::map<std::string, int> NameIndexMap;

extern const int MAX_OMP_THREADS;
/*!  Checks given condition.  If not satisfied, prints given message on cerr and aborts the computation.
 * In the parallel version of aces, this should call mpi_abort
 */
void check(bool, std::string, int line = 0);

/*!  Checks given condition.  If not satisfied, prints a warning message on cerr and then continues the computation */
bool check_and_warn(bool, std::string, int line = 0);

/*! fails with message */
void fail(std::string, int line = 0);

}//namespace sip

#endif /* SIP_H_*/
