/*
 * rank_distribution.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: njindal
 */

#include <rank_distribution.h>

#include <assert.h>

namespace sip {

bool RankDistribution::is_server(int rank, int size){
	return three_to_one_server(rank, size);
}

bool RankDistribution::three_to_one_server(int rank, int size){

	assert (size >= 2);

	if (size >= 3){
		if (2 == rank % 3)
			return true;
		else
			return false;
	} else {
		if (1 == rank)
			return true;
		else
			return false;
	}
}



} /* namespace sip */
