#ifndef CORE_H
#define CORE_H

#include <string>

#include "param.h"
#include "arg/arg.h"

#include <vector>
#include <stdexcept>
#include <algorithm>

uni::string programPath;

template<class ArgumentType>
void argumentProcesser(const uni::string& paramStr){
	std::shared_ptr<Argument> arg(std::make_shared<ArgumentType>("src"));
	std::shared_ptr<Param_t> param = arg->loadParam(paramStr);
	param->arg = arg;
	param->arg->validateParam(param);
	param->arg->setGlobalVariables(param);
	bool repeat = false;
	do{
		repeat = false;
		arg->status = param->arg->preProcessParam(param);
		while(arg->status==REPEAT_PROCESSING_BLOCK)
			arg->status =  param->arg->processParam(param);

		switch( param->arg->postProcessParam(param) ){
			case REPEAT_BLOCK: repeat = true; break;
			default: break; //END_BLOCK
		}
	}while(repeat);
}

std::vector<std::vector<std::shared_ptr<Param_t>>> blockVector; //paramVector[argLoop_n][arg_n]
void loadParam(int argc, uni::char_t* argv[]){
	/*Expanation of variables
		./cryptor	key	inputFile1	outputFile1
					^1	^2
		1 : argSeperaterLoop //REMOVED. Just reset argPtr to 0 after each loop.
		2 : argLoop //REMOVED. Just reset argPtr to 0 after each loop.
		argLoop_n is the number of loops that it is processing.
		argPtr stores the offset of the param that is loading.
	Expanation of terminology:
		./cryptor key1 inputFile1 outputFile1 ,, key2 inputFile2 outputFile2
		parameter are seperated by space. In this way, key1, inputFile1, etc. are said to be a param.
		block means a group of param before the argPtr is reset to argLoop. "key1 inputFile1 outputFile1" and "key2 inputFile2 outputFile2" are blocks.
		blockVector is a collection of all blocks. usage: paramVector[argLoop_n][arg_n]
	*/
	size_t argPtr = 0;
	size_t argLoop_n = 0;
	const size_t arg_n = arguments.size();
	blockVector.push_back(std::vector<std::shared_ptr<Param_t>>());

	for(const auto& i:arguments)
		i->setupArguments();

	for(int i=1;i<argc;i++){
		uni::string argStr(argv[i]);
		std::shared_ptr<Argument> arg;
		/*Check whether an optional is trigger here.*/
		bool triggeredArgument = false;
		for(auto& i:specialArguments){
			if(i->triggerArgument(argStr)){
				triggeredArgument = true;
				arg = i;
				break;
			}
		}
		/*no trigger.*/
		if(!triggeredArgument)
			arg = arguments[argPtr];
		/*arg is loaded. Now process the param.*/
		try{
			std::shared_ptr<Param_t> currentParam = arg->loadParam(argStr);
			currentParam->arg = arg;
			currentParam->arg->validateParam(currentParam);
			currentParam->arg->setGlobalVariables(currentParam);
			currentParam->arg->advanceArgPtr(argPtr);
			blockVector.back().push_back(currentParam);
		}catch(std::exception& e){
			std::cerr<<"Error in the parameter '"<<argStr<<"' :"<<std::endl;
			std::cerr<<"\t"<<e.what()<<std::endl;
			std::exit(-1);
		}

		/*a block is finished. Now, do loadAndValidateBlock() and append an empty vector to blockVector.*/
		assert(argPtr<=arg_n);
		if(argPtr==arg_n){
			try{
				for(auto& i:blockVector.back())
					i->arg->loadAndValidateBlock(blockVector.back(), blockVector.size());
			}catch(std::exception& e){
				std::cerr<<"Error in '";
				for(auto& i:blockVector.back())
					std::cerr<<i->value<<' ';
				std::cerr<<"' :"<<std::endl;
				std::cerr<<"\t"<<e.what()<<std::endl;
				std::exit(-1);
			}
			if(i!=argc-1) blockVector.push_back(std::vector<std::shared_ptr<Param_t>>());
			argLoop_n++;
			argPtr = 0;
		}
	}

	if(argPtr!=0||(argLoop_n==0&&blockVector.back().size()==0)){
		std::cerr<<"Missing parameter."<<std::endl;
		std::exit(-1);
	}
}

void processParam(){
	/*Let A = preProcessParam()
		B = processParam()
		C = postProcessParam()
		and	An = block[n]->preProcessParam()
			Bn = block[n]->processParam()
			Cn = block[n]->postProcessParam()

		***CHANGED! This info is outdated. TODO: update this documentation.

		The execution order is:
		A0 B0 A1 B1 ... An Bn repeat[n times](B0 B1 ... Bn) C0 C1 ... Cn -- (new)
		Please notice that the order is neither:
		A0 A1 ... An repeat[n times](B0 B1 ... Bn) C0 C1 ... Cn
		nor:
		A0 B0 A1 B1 repeat[n-1 times](B0 B1 ... Bn) B0 C0 B1 C1 ... Bn Cn --flag: (old)
		Ideally, (old) should be executed. However, it is impossible because for any value of n, param[n]->processParam() can be used to repeat the processParam()[i.e. triggering repeatBlockProcessParam]. It's not known that whether a param with higher value of n will trigger repeatBlockProcessParam.
		Says there are two param in the block. A0 B0 C0 is called, then A1 B1 is called that B1 triggers repeatBlockProcessParam. Then C0 have to be undone. However, it's impossible/difficult to undo it. Therefore, (new) is adopted.
	*/
	for(auto& block:blockVector){
		bool repeatBlock;
		do{
			repeatBlock = true;
			for(size_t i=0;i<block.size();i++){
				switch( block[i]->arg->preProcessBlock(block, i) ){
					case END_BLOCK: repeatBlock = false; break;
					default: break; //REPEAT_BLOCK
				}
			}
			while(repeatBlock){
				repeatBlock = false;
				std::vector<bool> processionCompleted(block.size(), false);
				bool firstIteration = true;
				for(size_t i=0;i<block.size();i++)
					block[i]->arg->status = block[i]->arg->initParamProcStatus();
				size_t breakPoint = block.size();
				size_t i = 0;
				while(i!=breakPoint){
					assert(i<block.size());
					std::shared_ptr<Param_t> param = block[i];
					std::shared_ptr<Argument> arg = param->arg;
					if(firstIteration)
						arg->status = arg->preProcessParam(param);
					if(arg->status==REPEAT_PROCESSING_BLOCK||arg->status==DECIDED_BY_OTHER_PARAM)
						arg->status = arg->processParam(param);

					if(breakPoint==block.size() && std::count_if(block.begin(), block.end(), [](std::shared_ptr<Param_t> param){ return param->arg->status==REPEAT_PROCESSING_BLOCK; })==0){
						breakPoint = i;
					}
					/*breakPoint is set. This param block is going to end. Hence, postProcessParam() should be called.*/
					if((arg->status==END_PROCESSING_BLOCK || /*arg->status==DECIDED_BY_OTHER_PARAM*/ (arg->status==DECIDED_BY_OTHER_PARAM&&breakPoint!=block.size()))&&!processionCompleted[i]){
						switch( arg->postProcessParam(param) ){
							case REPEAT_BLOCK: repeatBlock = true; break;
							default: break; //END_BLOCK
						}
						processionCompleted[i] = true;
					}
					i++;
					if(i==block.size()){
						firstIteration = false;
						i = 0;
					}
				}
				assert(std::find(processionCompleted.begin(), processionCompleted.end(), false)==processionCompleted.end());
			}

			repeatBlock = false;
			for(size_t i=0;i<block.size();i++){
				switch( block[i]->arg->postProcessBlock(block, i) ){
					case REPEAT_BLOCK: repeatBlock = true; break;
					default: break; //END_BLOCK
				}
			}
		}while(repeatBlock);
	}
}

extern void init();
extern void preProcess();
///TODO: uncomment for windows unicode support.[incompleted.]
/*
#ifdef __WIN32__
#include <io.h>
#include <wchar.h>
#include <stdlib.h>
#include <fcntl.h>
#define _UNICODE
unsigned int _CRT_fmode = _O_BINARY;
int argc;
wchar_t** argv;
wchar_t** enpv;
extern int _CRT_glob;
extern "C" void __wgetmainargs(int*,wchar_t***,wchar_t***,int,int*);
int main(){
#elif defined(__unix__)||defined(__APPLE__)*/
#ifdef __WIN32__
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
//extern "C" int _fileno(FILE* stream);
#endif
int main(int argc, char* argv[]){
//#endif
	try{
		//In Windows, set to stdin and stdout to binary mode here.
		///TODO: check whether it works.
		#ifdef __WIN32__
			_setmode(_fileno(stdin), _O_BINARY);
			_setmode(_fileno(stdout), _O_BINARY);
			/*int zero;
			__wgetmainargs(&argc, &argv, &enpv, 0, &zero);*/
		#endif
		programPath = argv[0]; //for option.h::showHelp()
		init();
		loadParam(argc, argv);
		preProcess();
		processParam();
	}catch(std::exception& e){
		std::cerr<<"Exception occured: "<<e.what()<<std::endl;
		return -1;
	}catch(...){
		std::cerr<<"An unknown exception occured"<<std::endl;
		return -1;
	}
	return 0;
}

#endif
