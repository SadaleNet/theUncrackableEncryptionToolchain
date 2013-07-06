/*This is a wrapper library to wrap the regex functions in boost to std.
This is written because GCC have not implement regex library yet.
Damn it! I've wasted a few hours on finding the reason of that my programs keep throwing std::regex_error,
it turned out to be the fault of the compiler.*/

#include <boost/regex.hpp>
namespace std{
	using namespace boost; //The namespace might contain unrelavant implementation identifiers. However, we don't need to worry about it because this is under the namespace std. This won't pollute the global namespace.
}

#include <boost/filesystem.hpp>
/*#ifndef BOOST_FILESYSTEM_IMPLEMENT_THE_UNIMPLEMENTED
#define BOOST_FILESYSTEM_IMPLEMENT_THE_UNIMPLEMENTED
namespace boost{
  namespace filesystem{
	//This function is not implemented as in boost 1.46.1
	///TODO: check the version of boost that implements copy() and copy_directory().
	#if BOOST_VERSION <= 104601
  	void copy(const path& from, const path& to){
  		std::string command;
		#ifdef __WIN32__
			command = std::string("copy /b /y ")+from.string<std::basic_string<uni::char_t>>()+" "+to.string<std::basic_string<uni::char_t>>();
		#elif defined(__unix__)||defined(__APPLE__)
			command = std::string("cp -f ")+from.string<std::basic_string<uni::char_t>>()+" "+to.string<std::basic_string<uni::char_t>>();
		#endif
		std::system(command.c_str());
  	}
  	void copy_directory(const path& from, const path& to){
  		std::string command;
		std::system((std::string("mkdir ")+to.string<std::basic_string<uni::char_t>>()).c_str());
		#ifdef __WIN32__
			command = std::string("copy /b /y ")+from.string<std::basic_string<uni::char_t>>()+" "+to.string<std::basic_string<uni::char_t>>();
		#elif defined(__unix__)||defined(__APPLE__)
			command = std::string("cp -Rf ")+from.string<std::basic_string<uni::char_t>>()+"/* "+to.string<std::basic_string<uni::char_t>>();
		#endif
		std::system(command.c_str());
  	}
  	#endif
  }
}


#endif
*/
