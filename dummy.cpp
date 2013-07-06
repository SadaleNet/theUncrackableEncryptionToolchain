#include "lib/lib.h"

const char* helpHeader = R"(This is a dummy program of this toolchain. It is can be used to a) convert external stdin to the format that is suitable for our toolchain and vice versa b) copy files. )";
const char* helpFooter = "";

std::vector<std::shared_ptr<Argument>> arguments = {std::make_shared<InputArgument>("src"),
										std::make_shared<OutputArgument>("dest")};
std::vector<std::shared_ptr<Argument>> specialArguments = {std::make_shared<RecirectArgument>("redir"),
										std::make_shared<OptionArgument>("opt")};
uint32_t enabledOptions = SHOW_HELP|FORCE_OVERWRITE;

void init(){}
void preProcess(){}
