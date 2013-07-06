#include "lib/lib.h"

const char* helpHeader = R"(This program injects data of src into dest to hide the data of the file, or extract the previously appended data from src to dest in extraction mode. )";
const char* helpFooter = "Pipe note: Since this program uses append file opening mode, redirection of () and [] of dest are not allowed";

bool discardMode = false, extractMode = false;

uint64_t extractedFileSize;

class SourceArgument:public SecureStreamInputArgument{
	private:
		typedef SecureStreamInputArgument super;
	public:
		SourceArgument(const char* name):SecureStreamInputArgument(name){}
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override{
			BLOCK_PROC_STATUS ret = super::postProcessParam(p);
			if(!nukeMode&&extractMode&&discardMode){ /*restore the file to its original state. i.e. discard the data that is previously injected. If extraction mode is enabled.*/
				std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
				if(param->pipeInfo&SRC_DEST&(PIPE_PLAIN|PIPE_LEN_DATA))
					throw std::runtime_error("discard mode cannot be used on non-file input.");
				///TODO: Should wipe the content to be discarded. It is not known that whether the content to be discarded is encrypted or not.
				boost::filesystem::resize_file(param->value, boost::filesystem::file_size(param->value)-extractedFileSize-sizeof(uint64_t));
			}
			return ret;
		}
};

uint64_t outputSize;
class InjectArgument:public OutputArgument{
	private:
		typedef OutputArgument super;
	protected:
		virtual bool predictableDataLen() override{ return true; }
		virtual uint64_t getDataLen() override{ return outputSize; }
	public:
		InjectArgument(const char* name):OutputArgument(name){	}
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> p=nullptr) override{
			if(!extractMode)
				extraFileOpenFlag = std::ios_base::app;
			return super::loadParam(str, p);
		}
		virtual void validateParam(std::shared_ptr<const Param_t> p) override{
			super::validateParam(p);
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
			if(param->pipeInfo&(REDIRECT&(PIPE_PLAIN|PIPE_LEN_DATA)))
				throw std::invalid_argument("redirection is not allowed due to the append nature of the output.");
			else if(discardMode&&extractMode)
				throw std::invalid_argument("discard mode cannot be used with extract mode.");
			else if(discardMode&&nukeMode)
				throw std::invalid_argument("discard cannot be used with nuke mode.");
		}
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override{
			outputSize = std::numeric_limits<uint64_t>::max();
			return super::preProcessParam(p);
		}
		virtual std::vector<char> genHeader() override{
			if(extractMode){
				SecureStreamInputArgument* SRC = static_cast<SecureStreamInputArgument*>(inputArgPtr[0]);
				/*reads as much as possible*/
				std::vector<char> buffer = SRC->readBuffer(std::numeric_limits<uint64_t>::max());
				if(buffer.size()<sizeof(uint64_t))
					throw std::runtime_error("invalid src file. The file is too short.");
				extractedFileSize = *reinterpret_cast<uint64_t*>(&buffer[buffer.size()-sizeof(uint64_t)]);
				if(buffer.size()<extractedFileSize+sizeof(uint64_t))
					throw std::runtime_error("invalid src file. The file is too short.");
				return std::vector<char>(&buffer[buffer.size()-sizeof(uint64_t)-extractedFileSize], &buffer[buffer.size()-sizeof(uint64_t)]);
			}
			return std::vector<char>();
		}
		virtual std::vector<char> genData() override{
			/*in injection mode, read and write data by chunk. In extraction mode, read all data into buffer, then extract the data we want and return it.*/
			if(!extractMode){
				SecureStreamInputArgument* SRC = static_cast<SecureStreamInputArgument*>(inputArgPtr[0]);
				std::vector<char> dataBuffer = SRC->readBuffer(CHUNK_SIZE);
				if(dataBuffer.size()==0){
					outputSize = dataSizeWritten;
					return std::vector<char>();
				}
				return dataBuffer;
			}else{
				outputSize = dataSizeWritten;
			}
			return std::vector<char>();
		}
		virtual std::vector<char> genFooter() override{
			if(extractMode) return std::vector<char>();
			std::vector<char> dataLenVector(sizeof(uint64_t));
			std::copy_n(reinterpret_cast<char*>(&dataSizeWritten), sizeof(uint64_t), dataLenVector.begin());
			return dataLenVector;
		}
};

std::vector<std::shared_ptr<Argument>> arguments = {std::make_shared<SourceArgument>("src"),
										std::make_shared<InjectArgument>("dest")};
std::vector<std::shared_ptr<Argument>> specialArguments = {std::make_shared<RecirectArgument>("redir"),
										std::make_shared<OptionArgument>("opt")};
uint32_t enabledOptions = SHOW_HELP|FORCE_OVERWRITE|NUKE_INPUT_FILE;

void init(){
	appendOption('d', discardMode, "discard mode. can only be used with extraction mode. If enabled, it discards the content that is previously injected. CAUTION: a) This does NOT nuke the discarded content b) Using this flag with a non-injected file may corrupt the file.");
	appendOption('e', extractMode, "extraction mode. extract the file src to dest.");
}
void preProcess(){}
