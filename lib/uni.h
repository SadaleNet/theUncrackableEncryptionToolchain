#ifndef UNI_H
#define UNI_H

#include "boost.h"
#include <string>
///#include <regex> //Change to this one once regex is implemented in GCC

namespace uni{
	/*damn it! Windows uses UTF-16 encoding. Y U NO USE UTF-8?*/
	#ifdef __WIN32__
	///It seems that boost::regex_match() doesn't like wchar_t. It crashes. Due to time constraint, let left out unicode support for Windows. Sorry, Windows users. :(
	///TODO: unicode support.
	/*typedef wchar_t char_t;
	//typedef std::wstring string;
	class string: public std::wstring{
		private:
			typedef std::wstring super;
			typedef char_t CharT;
			typedef std::allocator<CharT> Allocator;
			std::string toStr() const{
				std::string dummy(this->size(), '\0');
				for(size_t i=0; i<this->size(); i++){
					dummy[i] = this->at(i);
				}
				return dummy;
			}
		public:
			string(size_type count, CharT ch, const Allocator& alloc = Allocator()):super(count, ch, alloc){}
			//string(const string& other, size_type pos, size_type count = super::npos, const Allocator& alloc = Allocator()):super(count, pos, count, alloc){}
			string(const CharT* s, size_type count, const Allocator& alloc = Allocator()):super(s, count, alloc){}
			string(const CharT* s, const Allocator& alloc = Allocator()):super(s, alloc){}
			//string(super::InputIt first, super::InputIt last, const Allocator& alloc = Allocator()):super(first, last, alloc){}
			string() = default;
			string(const string&) = default;
			string(std::string str){
				this->resize(str.size());
				for(size_t i=0; i<str.size(); i++){
					this->push_back(str[i]);
				}
			}
			string(std::wstring str):super(str){}
			string(const char* s){
				size_t length = std::strlen(s);
				this->resize(length);
				std::copy_n(s, length, &(this->operator[](0)));
			}
			operator const std::string() const{ return toStr(); }
			operator const std::wstring() const{ return &super::operator[](0); }
			friend uni::string operator+(const uni::string& a, const uni::string& b){
				uni::string ret(a);
				ret.append(b);
				return ret;
			}
			uni::char_t& operator[]( size_t pos ){ return super::operator[](pos); }
			const uni::char_t& operator[]( size_t pos ) const{
				return super::operator[](pos);
			}
			friend std::ostream& operator<< (std::ostream& stream, const string& str) {
				stream<<"WSTR["<<str.toStr()<<"]";
				return stream;
			}
			size_type find(const char* s, size_type pos = 0) const{ return super::find(string(s), pos); }
			size_type find(const char s, size_type pos = 0) const{ return super::find(s, pos); }
			size_type find(const uni::string& str, size_type pos = 0) const{ return super::find(str.c_str(), pos); }
	};
	typedef std::wregex regex;
	typedef std::wsmatch match_results;*/

	typedef char char_t;
	typedef std::string string;
	typedef std::regex regex;
	typedef std::smatch match_results;
	#elif defined(__unix__)||defined(__APPLE__)
	typedef char char_t;
	typedef std::string string;
	typedef std::regex regex;
	typedef std::smatch match_results;
	#endif
	/*class string: public string_inner{
		public:
		//friend string operator uni::char_t*(){ return &this[0]; }
		operator uni::char_t*(){ return &((*this)[0]); }
	};*/
}

#endif
