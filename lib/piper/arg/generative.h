/*piper/arg/generative.h
Definition of generativeArgument
*/

class GenerativeArgument:public FileArgument{
	private:
		/*define a super keyword*/
		typedef FileArgument super;
	protected:
		/*buffer to store the data write/read(in some derived classes like InputArgument)*/
		std::vector<char> buffer;
		/*the file object to be opened/closed.*/
		std::fstream file;
		/*the flags to be appended.*/
		std::ios_base::openmode extraFileOpenFlag;
		/*used for prompting overwrite*/
		bool preventOverwrite;
		/*the size of the data have written in total.*/
		uint64_t dataSizeWritten;
		/*write the processed buffer out.*/
		virtual void writeData(std::shared_ptr<const FileParam_t> param, std::vector<char>& buffer){
			if(buffer.size()==0) return;
			if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){
				file.write(buffer.data(), buffer.size());
				file.flush();
			}
			dataSizeWritten += buffer.size();
		}
		/*This function creates non-existent directory for writing the file.
		e.g. The program is going to open the file ./dir/file to write data on it, where dir does not exist.
		Then this function creates the directory ./dir */
		void createNecessaryDirectories (const uni::string fileName) final{
			#ifdef __WIN32__
			fileName.substr(0, fileName.find('\\')); //remove the file name.
			#elif defined(__unix__)||defined(__APPLE__)
			fileName.substr(0, fileName.find('/')); //remove the file name.
			#endif
			size_t pos = 0;
			while(
				#ifdef __WIN32__
				(pos = std::min(fileName.find('/', pos+1), fileName.find('\\', pos+1)))
				#elif defined(__unix__)||defined(__APPLE__)
				(pos = fileName.find('/', pos+1))
				#endif
				!= uni::string::npos){
				uni::string currentDir = fileName.substr(0, pos);
				if(boost::filesystem::exists(currentDir)){
					/*check whether dirs in dir/dir/dir/file itself is not a directory. */
					if(!boost::filesystem::is_directory(currentDir))
						throw std::runtime_error(std::string(currentDir)+" is not a directory.");
				}else{
					boost::filesystem::create_directory(currentDir);
				}
			}
		}
		/*This function is to be derived. If the data length is predictable by inputArgPtr[i]->getDataLen(), this function returns true. false else.*/
		virtual bool predictableDataLen(){ return false; }
		/*This function is to be derived. If predictableDataLen() is true, this function returns the length of the data. Otherwise, this function should never be called.*/
		virtual uint64_t getDataLen(){ return 0; }
		/*This function is to be derived.
			This process the buffer by using the following functions:
				inputArgPtr[i]->getBuffer();
				inputArgPtr[i]->getDataSizeRead();
				inputArgPtr[i]->getDataChunkSize();
				inputArgPtr[i]->getDataLen();
			This function returns a processed buffer.*/
		virtual std::vector<char> genHeader(){ return std::vector<char>(); }
		virtual std::vector<char> genData(){ return std::vector<char>(); }
		virtual std::vector<char> genFooter(){ return std::vector<char>(); }
	public:
		/*constructor*/
		GenerativeArgument(const char* name):FileArgument(name){}
		/*set the initial value of PARAM_PROC_STATUS*/
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return predictableDataLen()?  REPEAT_PROCESSING_BLOCK: DECIDED_BY_OTHER_PARAM; }
		void validateParam(std::shared_ptr<const Param_t> p);
		/* open the file/initialize std::cout*/
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override;
		/*process the buffer, then write it out. Then redirect it.*/
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) override;
		/*close the file is it's a file param.*/
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override;
		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "out"; }
		virtual const char* getArgDescription() override{ return "output. process the buffer in input, then write it to stdout or file(s), or both."; }
};

void GenerativeArgument::validateParam(std::shared_ptr<const Param_t> p){
	super::validateParam(p);
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	//Check whether {[(i)]} or {[(r)]} is used. If yes, throw an exception.
	if(param->pipeInfo&(SRC_DEST&(PIPE_PLAIN|PIPE_LEN_DATA))){
		//if(param->value.find("i")!=uni::string::npos)
			throw std::invalid_argument("param [(i)] is not allowed to be used as a GenerativeArgument");
	}

	assert(!(param->pipeInfo&(SRC_DEST&(PIPE_PLAIN|PIPE_LEN_DATA))));
	assert(!(param->pipeInfo&(REDIRECT&PLAIN_FILE)));

}

PARAM_PROC_STATUS GenerativeArgument::preProcessParam(std::shared_ptr<Param_t> p){
	dataSizeWritten = 0;
	/*open the file.*/
	std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
	assert(param->pipeInfo&GENERATE);
	preventOverwrite = false;
	if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){ //the param is file or {flie}
		if(param->pipeInfo&(SRC_DEST&PIPE_FILENAME))
			param->value = readFileNameFromStdin();
		/*If the file exists, prompt for overwrite.*/
		if(boost::filesystem::exists(param->value)){
			/*If forceOverwrite!=ENABLED, do NOT prevent overwrite.*/
			if(forceOverwrite!=ENABLED){
				/*forceOverwrite==DISABLED or the user disagree to overwrite file on prompt, set preventOverwrite to true.*/
				if(forceOverwrite==DISABLED||!prompt(uni::string("The file '")+param->value+"' exists. Overwrite?", false)){
					std::cerr<<std::string("file '")+param->value+"' skipped."<<std::endl;
					preventOverwrite = true;
					return DECIDED_BY_OTHER_PARAM;
				}
			}
		}
		createNecessaryDirectories(param->value);
		file.open(param->value, std::ios_base::out|std::ios_base::binary|extraFileOpenFlag);
		if(!file) throw std::invalid_argument(std::string("cannot open the file '")+param->value+"'.");
	}

	/*redirect the header. write the data/file name length to std::cout if the param is a [] or {}*/
	redirectHeader(param);
	/*write the header of the file*/
	if(!preventOverwrite){
		std::vector<char> header = genHeader();
		writeData(param, header);
		redirectData(param, header.data(), header.size());
	}
	return DECIDED_BY_OTHER_PARAM;
}

PARAM_PROC_STATUS GenerativeArgument::processParam(std::shared_ptr<const Param_t> p){
	if(preventOverwrite) return DECIDED_BY_OTHER_PARAM;
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	assert(param->pipeInfo&GENERATE);
	/*process the buffer*/
	buffer = genData();
	/*write the processed buffer out.*/
	writeData(param, buffer);
	/*redirect the processed buffer to std::cout*/
	redirectData(param, buffer.data(), buffer.size());

	/*return a suitable PARAM_PROC_STATUS*/
	if(predictableDataLen()){
		assert(dataSizeWritten<=getDataLen());
		if(dataSizeWritten<getDataLen()) return REPEAT_PROCESSING_BLOCK;
		else return END_PROCESSING_BLOCK;
	}
	return DECIDED_BY_OTHER_PARAM;
}

BLOCK_PROC_STATUS GenerativeArgument::postProcessParam(std::shared_ptr<const Param_t> p){
	if(preventOverwrite) return END_BLOCK;
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	assert(param->pipeInfo&GENERATE);
	/*write the footer of the file*/
	std::vector<char> footer = genFooter();
	writeData(param, footer);
	redirectData(param, footer.data(), footer.size());
	/*close the file*/
	if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME)))
		file.close();
	redirectFooter(param);
	return END_BLOCK;
}
