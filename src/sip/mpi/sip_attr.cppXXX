/*
 * sip_attr.cpp
 *
 *  Created on: July 18, 2014
 *      Author: bsanders
 */

#include "config.h"
#include <sip_attr.h>
#include <iostream>


namespace sip {

SIPAttr* SIPAttr::instance_ = NULL;
bool SIPAttr::destroyed_ = false;

SIPAttr& SIPAttr::get_instance() {
	if (destroyed_)
		sip::fail("SIPAttr instance has been destroyed !");
	if (instance_ == NULL)
		instance_ = new SIPAttr();
	return *instance_;
}

void SIPAttr::cleanup() {
	delete instance_;
	destroyed_ = true;
}

SIPAttr::SIPAttr() {


}



std::ostream& operator<<(std::ostream& os, const SIPAttr& obj){
	os << "SIP Attributes [";
	os << "rank : 0" ;
	os << ", size : 1" ;
	os << ", is server? false: " ;
	os << ", company rank 0: " ;
	os << ", company size 1: " ;
	os << ", is master? : true"  ;
    os << ", server_master false: " ;
    os << ", worker master true: " << "]";
    os << std::endl;
	return os;
}


SIPAttr::~SIPAttr() {

}

} /* namespace sip */
