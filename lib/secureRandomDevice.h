#ifndef SECURE_RANDOM_GENERATOR_H
#define SECURE_RANDOM_GENERATOR_H

#include <vector>
#include "boost.h"
#include "uni.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#ifdef __WIN32__
	#include <windows.h>
	#include <wincrypt.h>
	namespace{ //encapsulation.
		/*acquire the hCryptProv right after the program starts. release it after it ends.*/
		HCRYPTPROV hCryptProv;
		class HCRYPTPROVClass{
			public:
				HCRYPTPROVClass(){
					if(!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
						throw std::runtime_error(std::string("Failure on CryptAcquireContext(). Error code:")+std::to_string(GetLastError())+" Have you enabled cryptographic service provider in Windows?");
				}
				~HCRYPTPROVClass(){ CryptReleaseContext(hCryptProv, 0); }
		} HCRYPTPROVClassObject_dummy_to_instantiate_the_class;
	}

	template <class result_type = char>
	class SecureRandomDevice{
		private:
			result_type* buffer;
			size_t bufferSize;
		public:
			explicit SecureRandomDevice(size_t bufferSize){
				this->bufferSize = bufferSize;
				buffer = new result_type[bufferSize];
			}
			~SecureRandomDevice(){ delete[] buffer; }
			result_type* gen(){
				if(!CryptGenRandom(hCryptProv, bufferSize*sizeof(result_type), (BYTE*)buffer))
					throw std::runtime_error(std::string("Failure on CryptGenRandom(). Error code:")+std::to_string(GetLastError()));
				return buffer; //Should this make a copy of buffer?
			}
			std::vector<result_type> genVector(){
				this->gen();
				std::vector<result_type> dummy(buffer, buffer+bufferSize);
				return dummy;
			}
			//Remove copy constructor and assign operator.
			SecureRandomDevice(const SecureRandomDevice&) = delete;
			void operator=(const SecureRandomDevice&) = delete;
	};
#elif defined(__unix__)||defined(__APPLE__)
	#include <fstream>
	namespace{ //encapsulation.
	/*open the file right after the program starts. Close it after the program ends.*/
		std::ifstream randomFile;
		class RandomFileClass{
			public:
				RandomFileClass(){
					randomFile.open("/dev/urandom", std::ios_base::in|std::ios::binary);
					if(!randomFile)
						throw std::runtime_error("Failure on opening /dev/urandom. It seems that this software does not supported by your operating system. Sorry about that.");
				}
				~RandomFileClass(){ randomFile.close(); }
		} randomFileObject_dummy_to_instantiate_the_class;
	}
	template <class result_type = char>
	class SecureRandomDevice{
		private:
			///TODO:the file is opened for each T in SecureRandomDevice<T>. The file object should be the same for all value of T.

			result_type* buffer;
			size_t bufferSize;

		public:
			explicit SecureRandomDevice(size_t bufferSize){
				this->bufferSize = bufferSize;
				buffer = new result_type[bufferSize];
			}
			~SecureRandomDevice(){ delete[] buffer; }
			result_type* gen(){
				randomFile.read(reinterpret_cast<char*>(buffer), bufferSize*sizeof(result_type));
				if(!randomFile)
					throw std::runtime_error("Failure on reading /dev/urandom. Please retry later. If this error presists, please contact the developer.");
				return buffer; //Should this make a copy of buffer?
			}
			std::vector<result_type> genVector(){
				this->gen();
				std::vector<result_type> dummy(buffer, buffer+bufferSize);
				return dummy;
			}
			//Remove copy constructor and assign operator.
			SecureRandomDevice(const SecureRandomDevice&) = delete;
			void operator=(const SecureRandomDevice&) = delete;
	};
#else
	#error "Sorry. This operation system is not supported."
#endif

class RandomFilter{
	private:
		std::vector<char> availChar;
	public:
		RandomFilter(std::string filter){
			try{
				//std::regex expression(filter);
				for(char i=' ';i<='~';i++){
					if(std::regex_match(&i, &i+1, std::regex(std::string(filter)))){
						availChar.push_back(static_cast<char>(i));
					}
				}
			}catch(std::exception& e){
				std::cout<<"Exception received"<<e.what()<<std::endl;
				std::cout.flush();
			}
		}
		template <class T>
		void filter(std::vector<T>& dataBuffer) const{
			if(availChar.size()==0)
				throw std::runtime_error("No character available. Ensure that the filter is initialized.");
			for(auto& i : dataBuffer)
				i = availChar.at(i%availChar.size());
		}
};

template<class T>
T genRand(){
	return *SecureRandomDevice<T>(sizeof(T)).gen();
}
#endif
