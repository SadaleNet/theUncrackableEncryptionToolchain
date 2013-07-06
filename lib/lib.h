#ifndef LIB_H_LOCAL
#define LIB_H_LOCAL

#include "boost.h"
#include "uni.h"

//Don't know where should I place this function in. Let's put it in this file, lol. :P
size_t strToSizeT(const uni::string& str){
	double ret = 0;
	for(size_t i=0; i<str.size(); i++)
		ret += static_cast<double>(str[i]-'0')*std::pow(10, str.size()-1-i);
	return static_cast<size_t>(ret);
}

#ifdef __MINGW32__
#include "mingwSupp.h"
#endif

#include "secureRandomDevice.h"
#include "piper/piper.h"

#include "native.h"

#endif
