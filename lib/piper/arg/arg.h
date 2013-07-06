/*
This directory holds the Argument class and its derived classes.
Here is the inheritance chart and the name of the file that contain the class.
Class											File
Argument										arg.h(this file)
	FileArgument								file.h
		GenerativeArgument*+					generative.h
			InputArgument*+						input.h
				SecureInputArgument@			input.h
				RedirectArgument				redir.h
				StreamInputArgument@			input.h
					SecureStreamInputArgument@	input.h
			OutputArgument*+					output.h
				DestructveOutputArgument@		output.h
					NukeArgument@				output.h
		FileNameArgument@						fileName.h
	OptionArgument

*These classes are depended on each other.
+requires re-documentation due to updates on the code.
@lacks documentation.

The documentation of each classes are available in corresponding file, inside the declaration of the class.
--------------------------
piper/arg/arg.h
This file holds:
	1) the definition and the implementation of the class Argument.
	2) the declaration of the variables std::shared_ptr<Argument> arguments[] and std::shared_ptr<Argument> specialArguments[]
	3) a structured #include block to load all arguments in this directory.
	4) functions that might be useful for classes.
Argument is a base class of all Argument classes.
The detailed documentation of the class Argument can be found below.

*/
/**TODO: document definition of argument and param, and block.*/

#ifndef ARG_H
#define ARG_H

#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ios>
#include <map>

#include <boost/filesystem.hpp>

/*Check whether the regex expr is found in param.*/
bool regexMatch(const uni::string& param, const uni::string expr){ //TODO: slow! This would repeatly create uni::regex()
	return std::regex_match(param, uni::regex(expr));
}
bool regexSearch(const uni::string& param, const uni::string expr){ //TODO: slow! This would repeatly create uni::regex()
	return std::regex_search(param, uni::regex(expr));
}

/*stdinOccupied is set to true if it is used for receiving data via pipe.*/
bool stdinOccupied = false;
/*prompt users of doing something.*/
bool prompt(std::string message, bool defaultValue){
	/*print the prompt message*/
	std::cerr<<message<<(defaultValue?"[Y/n]":"[y/N]")<<": ";
	/*If std::cin is occupied, tell the user and return the defaultValue*/
	if(stdinOccupied){
		std::cerr<<(defaultValue?'Y':'N')<<'*'<<std::endl;
		std::cerr<<"*[std::cin is occupied. Choosing the default value]"<<std::endl;
		return defaultValue;
	}
	/*wait for the user to type <Enter>, <y>, <Y>, <n> or <N> and return an approapiate value.*/
	char input[2]; //size of 2 for a character with \0
	do{
		input[0] = '\0';
		std::cin.getline(input, 2);

		if(input[0]=='y'||input[0]=='Y') return true;
		else if(input[0]=='n'||input[0]=='N') return false;
		else if(input[0]!='\0') std::cerr<<"Invalid input."<<std::endl;
	}while(input[0]!='\0'); //repeat if the user have typed something that is neither started with y or n
	return defaultValue;
}
/*These enums are used as a return value of processParam(). Please read processParam().*/
enum PARAM_PROC_STATUS{
	REPEAT_PROCESSING_BLOCK,
	END_PROCESSING_BLOCK,
	DECIDED_BY_OTHER_PARAM
};

enum BLOCK_PROC_STATUS{
	REPEAT_BLOCK,
	END_BLOCK
};

class Argument{
	protected:
		/*holds the name of the Argument, which is shown with -h option.*/
		std::string name;
	public:
		PARAM_PROC_STATUS status;
		/*holds the number of the Argument constructed.*/
		/*Constructor. Takes an const uni::char_t* as a name, which is shown with -h option.*/
		Argument(const char* name){ this->name = name; }
		/*only called for non-special arguments. This is called once only.*/
		virtual void setupArguments(){};
		/*special arguments have to be triggered through this function. This function is called for all special arguments, before loadParam() is called. A uni::string that contains the content of the param is passed to this function. If this function returns true, the argument is triggered. Then this argument will be appended the the block(a group of param).*/
		virtual bool triggerArgument(const uni::string&){ return false; };
		/*If no special argument is triggered, the param will be loaded according to arguments[]. This function creates an object Param_t, which will be used to store information of the parameter. Information will be loaded into the Param_t. Then, the Param_t object created is returned. This function may throw an exception if there is a syntax error on the param. the parameter Param_t is used for inheritance.
		NOTE: do NOT change nullptr to std::make_shared<Param_t>() because this will make the default argument inherited to its derived classes. Usually, the derived class want the default argument to be std::make_shared<ParamOfTheDerivedClass_t>()*/
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> param=nullptr){
			if(param==nullptr) param = std::make_shared<Param_t>();
			param->value = str;
			return param;
		};
		/*After loadParam() is called, this function is called. The Param_t object is passed to this function to check whether the param is legit. If the param is syntactically correct but logically wrong, it throws an exception.*/
		virtual void validateParam(std::shared_ptr<const Param_t>){}
		/*This function also set global variables like stdinOccupied.*/
		virtual void setGlobalVariables(std::shared_ptr<const Param_t>){}
		/*This function is only called for both non-special argument and specialArgumnt[]. This return a value to indicate how far should argPtr be advanced. The return value can be either positive or negative.*/
		virtual void advanceArgPtr(size_t& argPtr){ argPtr++; }
		/*this function loads and validate Param_t objects in a block. A vector of Param_t and the id/offset of the block is passed to this function. This function can be used to: a) change the order of Param_t objects in the vector so that the order of (pre| |post)ProcessParam() can be changed. b) check whether the block is legit. e.g. only one regex master is allowed in one block.*/
		virtual void loadAndValidateBlock(std::vector<std::shared_ptr<Param_t>>, size_t){};
		/*This function set initial status of PARAM_PROC_STATUS. The description of PARAM_PROC_STATUS is available in core.h:loadParam()*/
		virtual PARAM_PROC_STATUS initParamProcStatus() = 0;
		/*This function is called before processParam() is called. It can be used to open a file. Please notice that the argument is NOT const.*/
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t>){ return status; };
		/*As its name suggests, this function is used to process the Param_t loaded. The Param_t object that is previously loaded is passed to this function. Param can be processed in chunk. */
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t>) = 0;
		/*This function is called after processParam() is called. It can be used to close a file.*/
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t>){ return END_BLOCK; };

		/*This function is called before pre-/-/pro-processParam() is called.*/
		virtual BLOCK_PROC_STATUS preProcessBlock(std::vector<std::shared_ptr<Param_t>>, size_t){ return REPEAT_BLOCK; };
		/*This function is called after pre-/-/pro-processParam() is called.*/
		virtual BLOCK_PROC_STATUS postProcessBlock(std::vector<std::shared_ptr<Param_t>>, size_t){ return END_BLOCK; };

		/*used for help message*/
		virtual const char* getArgCode() = 0;
		virtual const char* getArgDescription() = 0;

		/*member getters*/
		const std::string getName(){ return name; }

};

extern std::vector<std::shared_ptr<Argument>> arguments;
extern std::vector<std::shared_ptr<Argument>> specialArguments; //have to be triggered through triggerArgument()

#include "option.h"

#include "file.h"
#include "generative.h"
#include "input.h"
#include "output.h"
#include "redirect.h"
#include "fileName.h"

#endif
