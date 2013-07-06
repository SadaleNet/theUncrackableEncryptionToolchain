#include "lib/lib.h"

///TODO: obfuscate the creation data. Create a OutputArgument for this purpose? something like SecureOutputArgument?

const char* helpHeader = "This program generates file(s) with random data.";
const char* helpFooter = "";

size_t fileSize;
class SizeArgument:public Argument{
	private:
	public:
		SizeArgument(const char* name):Argument(name){}
		virtual void validateParam(std::shared_ptr<const Param_t> param){
			if(!regexMatch(param->value, R"(\d+[kmgKMG]?[bB]?)"))
				throw std::invalid_argument("invalid size.");
		}
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return DECIDED_BY_OTHER_PARAM; }
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> param) override{
			uni::match_results regexSubexpr;
			std::regex_match(param->value, regexSubexpr, uni::regex(uni::string(R"((\d+)([kmgKMG]?)([bB]?))")));
			size_t base = 1024;
			if(regexSubexpr[3].length()!=0) base = 1000;
			size_t exponent;
			switch(uni::string(regexSubexpr[2]).front()){
				case 'k': case 'K': exponent = 1; break;
				case 'm': case 'M': exponent = 2; break;
				case 'g': case 'G': exponent = 3; break;
				default: exponent = 0; break;
			}
			fileSize = strToSizeT(regexSubexpr[1].str())*pow(base, exponent);
			return END_PROCESSING_BLOCK;
		}
		virtual const char* getArgCode() override{ return "s"; }
		virtual const char* getArgDescription() override{ return "size: size in bytes. K=1024, M=1024*1024, G=1024*1024*1024, KB=1000, MB=1000*1000, GB=1000*1000*1000"; }
};
class OutputKeyArgument:public OutputArgument{
	public:
		OutputKeyArgument(const char* name):OutputArgument(name){}
		virtual bool predictableDataLen() override{ return true; }
		virtual uint64_t getDataLen() override{ return fileSize; }
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return REPEAT_PROCESSING_BLOCK; }
		virtual std::vector<char> genData() override{
			size_t toBeWritten = fileSize-dataSizeWritten<CHUNK_SIZE? fileSize-dataSizeWritten: CHUNK_SIZE;
			return SecureRandomDevice<>(toBeWritten).genVector();
		}
};

std::vector<std::shared_ptr<Argument>> arguments = {std::make_shared<SizeArgument>("size"),
										std::make_shared<OutputKeyArgument>("outKey")};
std::vector<std::shared_ptr<Argument>> specialArguments = {std::make_shared<RecirectArgument>("redir"),
										std::make_shared<OptionArgument>("opt")};
uint32_t enabledOptions = SHOW_HELP|FORCE_OVERWRITE;

void init(){}
void preProcess(){}
