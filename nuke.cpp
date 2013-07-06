///TODO: If the program is terminated while the file is being nuked, the size of the file will become zero. If the first overwrite is incompleted, data inside the file may leaks.

#include "lib/lib.h"

const char* helpHeader = "This program overwrite files with random data multiple times to prevent it from being recovered.";
const char* helpFooter = "Please notice that this utility would not work in case the file content is journaled by the filesystem.";

std::vector<std::shared_ptr<Argument>> arguments = {std::make_shared<NukeArgument>("file")};
std::vector<std::shared_ptr<Argument>> specialArguments = {std::make_shared<RecirectArgument>("redir"),
										std::make_shared<OptionArgument>("opt")};
uint32_t enabledOptions = SHOW_HELP|FORCE_OVERWRITE;

void init(){
	appendOption( uni::string(R"(^n=\d+)"), [](const uni::string paramStr){
		std::static_pointer_cast<NukeArgument>(arguments[0])->overwrite_n = strToSizeT(paramStr.substr(paramStr.find('=')+1));
		if(std::static_pointer_cast<NukeArgument>(arguments[0])->overwrite_n==0) throw std::invalid_argument("You have to overwrite the file at least once.");
	}, "Overwrite the file, including the file name \\d+ times. (default=3)");
}

void preProcess(){
	if(!forceOverwrite&&!prompt("Are you sure that you want to nuke the file(s)? This cannot be undone.", false)){
		std::cerr<<"Operation canceled."<<std::endl;
		std::exit(0);
	}
	std::cerr<<"Nuking files..."<<std::endl;
	forceOverwrite = ENABLED;
	///TODO: an option to specify the time of overwrite.
}
