enum TriState{ DEFAULT, DISABLED, ENABLED };

struct Option_t{
	uni::string regex;
	std::function<void(const uni::string&)> func;
	const char* description; //if the string is empty, it means that it's an alias of another option. The alias option is not shown in the help then.
	bool terminate;
};
//std::map<uni::string, Option_t> options;
std::vector<Option_t> options;
template<class T>
uni::char_t toUpper(T a){ return static_cast<uni::char_t>(std::toupper(a)); }
template<class T>
uni::char_t toLower(T a){ return static_cast<uni::char_t>(std::tolower(a)); }

void appendOption(uni::char_t flag, TriState& target, const char* enableDescription, const char* disableDescription){
	assert(std::isalpha(flag));
	/*set a function to enable option if the option is lower case.*/
	options.push_back( {uni::string(1,toLower(flag)), [&target](const uni::string&){ target = ENABLED; }, enableDescription, false} );
	/*set a function to enable option if the option is upper case.*/
	options.push_back( {uni::string(1,toUpper(flag)), [&target](const uni::string&){ target = DISABLED; }, disableDescription, false} );
}

void appendOption(uni::char_t flag, std::function<void(const uni::string&)> func, const char* description, bool terminate=false){
	if(std::isalpha(flag)){
		options.push_back( {uni::string(1,toLower(flag)), func, description, terminate} );
		options.push_back( {uni::string(1,toUpper(flag)), func, nullptr, terminate} ); //set desciption to "" to prevent showing it in help.
	}else{
		options.push_back( {uni::string(1,toUpper(flag)), func, description, terminate} );
	}
}

void appendOption(uni::char_t flag, bool& target, const char* description, bool terminate=false){
	appendOption(flag, [&target](const uni::string&){ target = true; }, description, terminate);
}

void appendOption(uni::string regex, std::function<void(const uni::string&)> func, const char* description, bool terminate=false){
	options.push_back( {regex, func, description, terminate} );
}


extern const char* helpHeader;
extern const char* helpFooter;
extern uni::string programPath;

void showHelp(const uni::string&){
	/*Show the header of the help defined by the program.*/
	std::cerr<<helpHeader<<std::endl<<std::endl;

	/*show the name of the program and its usage.*/
	std::cerr<<"Usage: "<<programPath;
	for(auto& i:arguments){
		std::cerr<<'\t';
		std::cerr<<i->getName();
	}
	std::cerr<<std::endl;

	std::cerr<<"Type:  "<<std::string(programPath.size(), ' ');
	for(auto& i:arguments) std::cerr<<'\t'<<i->getArgCode();
	std::cerr<<std::endl;
	if(specialArguments.size()>0){
		std::cerr<<"Available special arguments:";
		for(auto& i:specialArguments) std::cerr<<' '<<i->getArgCode();
		std::cerr<<std::endl;
	}

	std::cerr<<"Argument type description: "<<std::endl;
	for(auto& i:arguments) std::cerr<<'\t'<<i->getArgCode()<<'\t'<<i->getArgDescription()<<std::endl;
	for(auto& i:specialArguments) std::cerr<<'\t'<<i->getArgCode()<<'\t'<<i->getArgDescription()<<std::endl;

	/*Show the available options*/
	std::cerr<<"Available options:"<<std::endl;
	for(auto& i:options){
		if(i.description==nullptr) continue;
		std::cerr<<'\t'<<'-'<<i.regex<<'\t'<<i.description<<std::endl;
	}

	/*Show the footer of the help defined by the program.*/
	std::cerr<<std::endl<<helpFooter<<std::endl;
}
TriState forceOverwrite = DEFAULT;
bool nukeMode = false;

enum EnabledOptions{
	DISABLE_ALL_OPTIONS=0x0,
	SHOW_HELP=0x1,
	FORCE_OVERWRITE=0x2,
	NUKE_INPUT_FILE=0x4
};
extern uint32_t enabledOptions;
std::map<EnabledOptions, std::function<void()>> enumToOption = {
{ SHOW_HELP, [](){ appendOption('h', showHelp, "show this help message", true); } },
{ FORCE_OVERWRITE, [](){ appendOption('f', forceOverwrite, "force overwrite/nuke", "prevent overwrite/nuke"); } },
{ NUKE_INPUT_FILE, [](){ appendOption('n', [](const uni::string&){
		if(!forceOverwrite&&!prompt("You are about to nuke the source file. This is not revisible. Are you sure?", false)){ ///TODO: stdinOccupied is not set at the movement that this function is called. It is because setGlobalVariables() of parameter with {[(i)]} is usually called after this param is processed. If the data from pipe starts with y, then, oh no. The file will be nuked. Need to find a way to fix it.
			std::cerr<<"Operation canceled."<<std::endl;
			std::exit(0);
		}
		nukeMode = true; }, "nuke the source file after the operation is finished."); } },
};

class OptionArgument:public Argument{
	private:
		/*define a super keyword*/
		typedef Argument super;
	public:
		OptionArgument(const char* name):Argument(name){
			/*call the functions in enumToOption if a flag is enabled.*/
			for(const auto& i:enumToOption){
				if(enabledOptions&i.first)
					i.second();
			}
		}
		virtual bool triggerArgument(const uni::string& str) override{ return regexMatch(str, R"(-.*)"); }
		virtual void advanceArgPtr(size_t&) override{}
		/*set the initial value of PARAM_PROC_STATUS*/
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> param=nullptr) override{
			param = super::loadParam(str);
			/*sort the vector options.*/
			std::sort(options.begin(), options.end(), [](const Option_t& a,const Option_t& b){ return a.regex.size() < b.regex.size(); }); ///TODO: longer regex expression != longer the expression is matched.
			/*remove the - prefix in the option.*/
			param->value = param->value.substr(1);
			for(auto& i:options){
				if(regexSearch(param->value, i.regex.c_str())){
					i.func(param->value);
					if(i.terminate) std::exit(0);
				}
			}
			return param;
		}
		virtual void validateParam(std::shared_ptr<const Param_t> param) override{
			bool matched = false;
			for(auto& i:options){
				if(regexSearch(param->value, i.regex.c_str())){
					matched = true;
					break;
				}
			}
			if(!matched)
				throw std::invalid_argument(uni::string("invalid option -")+param->value);
		}
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return END_PROCESSING_BLOCK; }
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t>) override{ return status; }
		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "opt"; }
		virtual const char* getArgDescription() override{ return "option. Triggered by '-*', where * is an arbitrary number of characters."; }
};

