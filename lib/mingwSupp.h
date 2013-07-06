/*This is TRWTF. There are several bug in MinGW 4.7, lefting many standard library functions umimplemented. This file implements the unimplemented functins.
EDIT: It seems that these functions are new in C++11. No wonder why they haven't implement it.*/

#ifndef MINGW_SUPP_H
#define MINGW_SUPP_H
#include <sstream>
	namespace std{
		template<class T>
		string to_string(T a){ //WTF? A bug in MinGW left this function umimplemted. http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52015
			std::stringstream sstrm;
			sstrm << a;
			return std::string(sstrm.str());
		}
	}

#endif
