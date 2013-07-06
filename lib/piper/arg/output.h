/*piper/arg/output.h
This file holds the definition and the implementation of the class OutputArgument and DestructveOutputArgument.
OutputArgument is a class that process the buffer that is previously loaded in InputArgument.
*/

///TODO: create directory automatically when it does not exist. e.g. output to ./dir/file, where dir does not exist.

class OutputArgument:public GenerativeArgument{
	private:
		/*define a super keyword*/
		typedef GenerativeArgument super;
	protected:
		/*holds a vector on pointers that's an InputArgument in argument[]*/
		std::vector<InputArgument*> inputArgPtr;
		/*This function is to be derived. If the data length is predictable by inputArgPtr[i]->getDataLen(), this function returns true. false else.*/
		virtual bool predictableDataLen() override{ return (getDataLen()!=std::numeric_limits<uint64_t>::max()); }
		/*This function is to be derived. If predictableDataLen() is true, this function returns the length of the data. Otherwise, this function should never be called.*/
		virtual uint64_t getDataLen() override{ return inputArgPtr[0]->getDataLen(); }
		/*This function is to be derived.
			This process the buffer by using the following functions:
				inputArgPtr[i]->getBuffer();
				inputArgPtr[i]->getDataSizeRead();
				inputArgPtr[i]->getDataChunkSize();
				inputArgPtr[i]->getDataLen();
			This function returns a processed buffer.*/
		virtual std::vector<char> genData() override{ return inputArgPtr[0]->getBuffer(); }
	public:
		/*constructor*/
		OutputArgument(const char* name):GenerativeArgument(name){}
		/*loads inputArgPtr. Push all arguments[] with the class of InputArgument() and its derived classes into inputArgPtr. */
		virtual void setupArguments() override;
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> p) override;

		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "out"; }
		virtual const char* getArgDescription() override{ return "output. process the buffer in input, then write it to stdout or file(s), or both."; }
};

void OutputArgument::setupArguments(){
	for(size_t i=0;i<arguments.size();i++){ ///TODO: use range-based for?
		InputArgument* dummy = dynamic_cast<InputArgument*>(arguments[i].get());
		if(dummy!=nullptr)
			inputArgPtr.push_back(dummy);
	}
}

std::shared_ptr<Param_t> OutputArgument::loadParam(const uni::string& str, std::shared_ptr<Param_t> p=nullptr){
	p = super::loadParam(str, p);
	std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
	param->pipeInfo |= GENERATE;
	return p;
}

class DestructveOutputArgument:public OutputArgument{
	private:
		typedef OutputArgument super;
		size_t fileSize;
	public:
		DestructveOutputArgument(const char* name):OutputArgument(name){}
		virtual void validateParam(std::shared_ptr<const Param_t> p) override{
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
			if(!(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))) || (param->pipeInfo&REDIRECT))
				throw std::invalid_argument("only {i} and file are allowed for DestructveOutputArgument.");
		}
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return REPEAT_PROCESSING_BLOCK; }
		/*set the initial value of PARAM_PROC_STATUS*/
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override{
			dataSizeWritten = 0;
			std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
			preventOverwrite = false;
			if(param->pipeInfo&(SRC_DEST&PIPE_FILENAME))
				param->value = readFileNameFromStdin();
			fileSize = boost::filesystem::file_size(param->value);
			file.open(param->value, std::ios_base::out|std::ios_base::binary);
			if(!file) throw std::invalid_argument(std::string("cannot open the file '")+param->value+"'.");
			return DECIDED_BY_OTHER_PARAM;
		}
		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "Dout"; }
		virtual const char* getArgDescription() override{ return "Descturtive output. Writes to file. Only {i} and file are allowed."; }
		/*resource getter*/
		size_t getFileSize() const{ return fileSize; }
};

class NukeArgument:public DestructveOutputArgument{
	private:
		typedef DestructveOutputArgument super;
		size_t overwriteIteration;
		uint64_t fileSize;
	public:
		size_t overwrite_n;
		NukeArgument(const char* name):DestructveOutputArgument(name),overwriteIteration(0),overwrite_n(3){}
		virtual bool predictableDataLen() override{ return true; }
		virtual uint64_t getDataLen() override{ return fileSize; }
		virtual std::vector<char> genData() override{
			size_t toBeWritten = getDataLen()<CHUNK_SIZE+dataSizeWritten ? getDataLen()-dataSizeWritten : CHUNK_SIZE;
			return SecureRandomDevice<>(toBeWritten).genVector();
		}
		PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override{
			if(overwriteIteration==0){
				super::preProcessParam(p);
				fileSize = getFileSize()+SecureRandomDevice<uint32_t>(1).gen()[0]%CHUNK_SIZE;
				fileSize *= 1+static_cast<float>(SecureRandomDevice<uint16_t>(1).gen()[0])/65535;
			}else{
				/*trick the super::preProcessParam(p) to think that it's not a {i} even if it is to prevent it from reading from std:::cin multiple times.*/
				std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
				uint32_t oldPipeInfo = param->pipeInfo;
				param->pipeInfo &= ~SRC_DEST;
				param->pipeInfo |= SRC_DEST&PLAIN_FILE;
				super::preProcessParam(p);
				param->pipeInfo = oldPipeInfo;
			}
			return DECIDED_BY_OTHER_PARAM;
		}
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override{
			super::postProcessParam(p);
			const std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);

			/*check whether it have overwritten for enough time.*/
			overwriteIteration++;
			assert(overwriteIteration<=overwrite_n);
			if(overwriteIteration==overwrite_n){
				/*wipe the filename after the last overwrite iteration has completed.*/
				boost::filesystem::path path = param->value;
				for(size_t i=0;i<overwrite_n;i++){
					boost::filesystem::path oldPath = path;
					size_t fileNameLen = path.filename().string<std::basic_string<uni::char_t>>().size();
					/*generate a new filename.*/
					std::vector<uni::char_t> newFileName = SecureRandomDevice<uni::char_t>(fileNameLen+1).genVector();
					RandomFilter(R"(\w)").filter(newFileName);
					newFileName[fileNameLen] = '\0';
					/*change the file name in the path of the file.*/
					path.remove_filename();
					path /= newFileName;
					/*rename the file*/
					boost::filesystem::rename(oldPath.string<std::basic_string<uni::char_t>>(), path.string<std::basic_string<uni::char_t>>());
				}
				/*finally, remove the file and reset overwriteIteration.*/
				boost::filesystem::remove(path.string<std::basic_string<uni::char_t>>());
				overwriteIteration = 0;
				return END_BLOCK;
			}
			return REPEAT_BLOCK;
		}
};
