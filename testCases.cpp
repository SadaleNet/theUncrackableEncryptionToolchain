#include <cassert>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <utility>
#include <functional>
#include <array>
#include <vector>
//from lib.h
#include "lib/boost.h"
#include "lib/uni.h"
#ifdef __MINGW32__
#include "lib/mingwSupp.h"
#endif
#include "lib/secureRandomDevice.h"
#include "lib/native.h"

#define assertWithMsg(func)\
	std::cout<<"Testing\t"<<#func<<"()...\t";\
	if(!func()){\
		std::cout<<"FAILED"<<std::endl;\
		std::exit(-1);\
	}\
	std::cout<<"Passed"<<std::endl;

bool randomTest(){
	const int SAMPLE_N = 512;
	SecureRandomDevice<> randomBlock(SAMPLE_N);
	char* dummy = randomBlock.gen();
	for(int i=1;i<SAMPLE_N;i++){
		if(dummy[i]!=dummy[i-1])
			break;
		if(i==SAMPLE_N-1) //create a robust test? Ain't nobody got time for dat? lol.
			return false;
	}
	return true;
}

bool randomFilterTest(){
	const int SAMPLE_N = 128*1024;
	SecureRandomDevice<unsigned int> randomBlock(SAMPLE_N);
	std::vector<std::pair<std::string,std::function<bool(unsigned int&)>>> regexCases =
	{
		{ R"([a-z])", [](unsigned int& i){ return 'a'<=i&&i<='z';}},
		{ R"([A-Z])", [](unsigned int& i){ return 'A'<=i&&i<='Z';}},
		{ R"([\d!])", [](unsigned int& i){ return ('0'<=i&&i<='9')||(i=='!');}},
		{ R"([01])", [](unsigned int& i){ return i=='0'||i=='1';}},
		//{ "[]", [](unsigned int& i){ return 'A'<=i&&i<='Z';}},
	};
	for(auto& i:regexCases){
		//print message
		std::cout<<i.first<<"\t";
		//generate random block
		unsigned int* dummy = randomBlock.gen();
		//convert it into std::vector and filter it.
		std::vector<unsigned int> randomChars(dummy, dummy+SAMPLE_N);
		RandomFilter(i.first).filter<unsigned int>(randomChars);
		//If there is an character that is not included in the expression, return false.
		if(std::find_if_not(randomChars.begin(), randomChars.end(), i.second) != randomChars.end())
			return false;
	}
	return true;
}

#include "intTest.h"

int main(){
	assertWithMsg(randomTest);
	assertWithMsg(randomFilterTest);
	assertWithMsg(integrationTests);

	//assertWithMsg([](){ return false; }); //This works.

	std::cout<<"All tests were completed successfully. Hooray! :D"<<std::endl;
	return 0;
}
