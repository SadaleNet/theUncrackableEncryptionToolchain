/*piper/arg/input.h
This file holds the definition and the implementation of the InputArgument class.
InputArgument is a class that load data as described in param.pipeInfo into buffer.
The loaded buffer will be processed by OutputArgument.
*/

class InputArgument:public GenerativeArgument{
	private:
		/*define a super keyword*/
		typedef GenerativeArgument super;
		uni::string paramStr;
	protected:
		/*the size of the data have read in total.*/
		uint64_t dataSizeRead;
		/*the length of the data. If the param is a file, it's determined when preProcessParam() is called. OItherwise, the value of dataLen is std::numeric_limits<uint64_t>::max()*/
		uint64_t dataLen;
		/*the length of data of this datablock.*/
		uint32_t dataChunkSize;
		/*see whether the param is completed. Used for PIPE_LEN_DATA in processParam()*/
		bool paramLenDataCompleted;
		/*method to resize buffer. To be overridden by SecureInputArgument*/
		virtual void resizeBuffer(size_t newSize){ buffer.resize(newSize); }
		/*method to read data form flie/std::cin*/
		virtual size_t readData(std::shared_ptr<const FileParam_t> param, size_t maxCount, size_t startPos){
			/*resize the buffer*/
			resizeBuffer(maxCount+startPos);

			/*read buffer, increases dataSizeRead, shrink buffer to fit then redirect data.*/
			if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){
				//file.sync();
				file.read(&buffer[startPos], maxCount);
				dataSizeRead += file.gcount();
				resizeBuffer(file.gcount()+startPos);
				redirectData(param, &buffer[startPos], file.gcount());
				return file.gcount();
			}else{
				pipeIStream.read(&buffer[startPos], maxCount);
				dataSizeRead += pipeIStream.gcount();
				resizeBuffer(pipeIStream.gcount()+startPos);
				redirectData(param, &buffer[startPos], pipeIStream.gcount());
			}
			return pipeIStream.gcount();
		}
		/*The file is nuked if nukeMode==true*/
		virtual bool nukable(){ return true; }
	public:
		/*A non-default constructor that takes a name. It calls the constructor of FileArgument and pass the name to it. It also set the exception flags of stream.*/
		InputArgument(const char* name):GenerativeArgument(name){}
		/*Throw an exception if:
			a) [(o)] is in the param
			b) the provided filename does not exist.*/
		virtual void validateParam(std::shared_ptr<const Param_t> p) override;
		/*If std::cin is occupied, set stdinOccupied to true.*/
		virtual void setGlobalVariables(std::shared_ptr<const Param_t> p);
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> p) override;
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return REPEAT_PROCESSING_BLOCK; }
		/*open the file/initialize std::cin/Read the len of data or filename from std::cin. Set dataLen if the param is a {} or a file. Otherwise, dataLen=std::numeric_limits<uint64_t>::max(). Resize the buffer to CHUNK_SIZE if there is no barrier.*/
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override;
		/*If the param is a (), reads until EOF, dataLastReadSize and dataSizeRead. Otherwise, read the data and load it into buffer, as well as setting dataChunkSize and increasing the value of dataSizeRead.
			Then, this function redirects the data inside the buffer.*/
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) override;
		/*If the param is a file, close it.*/
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override;

		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "in"; }
		virtual const char* getArgDescription() override{ return "input. read from either file(s) or stdin and load the it into buffer. can be used to redirect the *unprocessed* data to stdout."; }

		/*member getters, usually used for OutputArgument.*/
		virtual std::vector<char> getBuffer() const{ return buffer; }
		virtual uint64_t getDataChunkSize() const{ return dataChunkSize; }
		virtual uint64_t getDataSizeRead() const{ return dataSizeRead; }
		virtual uint64_t getDataLen() const{ return dataLen; }

};

std::shared_ptr<Param_t> InputArgument::loadParam(const uni::string& str, std::shared_ptr<Param_t> p=nullptr) override{
	paramStr = str;
	return super::loadParam(str, p);
}

void InputArgument::validateParam(std::shared_ptr<const Param_t> p){
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	if(param->pipeInfo&GENERATE) super::validateParam(p);

	FileArgument::validateParam(p);
	//Check whether {[(o)]} is used. If yes, throw an exception.
	if(!(param->pipeInfo&(SRC_DEST&PLAIN_FILE))){ //The param is neither a file or {[(file)]}
		if(param->value.find("o")!=uni::string::npos)
			throw std::invalid_argument("Param o is not allowed to be used as a InputArgument");
	}
	if((param->pipeInfo&(SRC_DEST&PLAIN_FILE)) && param->pipeInfo==NO_REGEX && !boost::filesystem::exists(param->value))
		throw std::invalid_argument(std::string("The file '")+param->value+"' does not exist.");
}

void InputArgument::setGlobalVariables(std::shared_ptr<const Param_t> p){
	super::setGlobalVariables(p);
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	if(param->pipeInfo&GENERATE) return;
	if(param->pipeInfo&(SRC_DEST&NOT_FILE))
		stdinOccupied = true;
}

PARAM_PROC_STATUS InputArgument::preProcessParam(std::shared_ptr<Param_t> p){
	paramLenDataCompleted = false;
	std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
	/*If the param generate data itself, then use preProcessParam() in GenerativeArgument.*/
	if(param->pipeInfo&GENERATE) return super::preProcessParam(p);
	/*open the file.*/
	dataLen = std::numeric_limits<uint64_t>::max();
	/*if(param->pipeInfo&(SRC_DEST&PIPE_PLAIN)){ //Do nothing!
	}else */
	if(param->pipeInfo&(SRC_DEST&PIPE_LEN_DATA)){ //if param is a [.*
		checkPipeMagicNumber(PIPE_LEN_DATA);
	}else if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){ //param is either {.* or file or {[(flie)]}
		//If param is a {}, load the filename to param->value as received from std::cin
		if(param->pipeInfo&(SRC_DEST&PIPE_FILENAME))
			param->value = readFileNameFromStdin();
		file.open(param->value, std::ios_base::in|std::ios_base::binary|extraFileOpenFlag);
		if(!file) throw std::invalid_argument(std::string("cannot not open the file '")+param->value+"'.");
		dataLen = boost::filesystem::file_size(param->value);
	}

	dataSizeRead = 0;
	/*redirect the header. Writes the len of data/file name to std::cout for param [] and {}*/
	redirectHeader(param);
	return DECIDED_BY_OTHER_PARAM;
}

PARAM_PROC_STATUS InputArgument::processParam(std::shared_ptr<const Param_t> p){
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	/*If the param generate data itself, then use processParam() in GenerativeArgument.*/
	if(param->pipeInfo&GENERATE)
		return super::processParam(p);
	/*load the file/pipe data to buffer*/
	dataChunkSize = 0;
	if(param->pipeInfo&(SRC_DEST&PIPE_PLAIN)){
		dataChunkSize = readData(param, CHUNK_SIZE, 0);
		pipeIStream.peek(); //set eof flag in the stream to true if there is no more character.
		if(pipeIStream.eof())
			return END_PROCESSING_BLOCK;
	}else if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){
		dataChunkSize = readData(param, CHUNK_SIZE, 0);
		file.peek(); //set eof flag in the file to true if there is no more character.
		if(file.eof())
			return END_PROCESSING_BLOCK;
	}else if(param->pipeInfo&(SRC_DEST&PIPE_LEN_DATA)){
		if(paramLenDataCompleted)
			return END_PROCESSING_BLOCK;
		pipeIStream.read(reinterpret_cast<char*>(&dataChunkSize), sizeof(uint32_t));
		pipeIStream.peek(); //set eof flag in the stream to true if there is no more character.
		if(dataChunkSize==0){ //readData() above will resize the buffer to zero if dataChunkSize==0.
			resizeBuffer(0);
			paramLenDataCompleted = true;
			return END_PROCESSING_BLOCK;
		}
		if(readData(param, dataChunkSize, 0)!=dataChunkSize||pipeIStream.eof())
			throw std::invalid_argument("wrong data encoding format from std::cin");
	}
	return REPEAT_PROCESSING_BLOCK;
}

template<class ArgumentType> extern void argumentProcesser(const uni::string&);
class NukeArgument;
BLOCK_PROC_STATUS InputArgument::postProcessParam(std::shared_ptr<const Param_t> p){
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	/*If the param generate data itself, then use postProcessParam() in GenerativeArgument.*/
	if(param->pipeInfo&GENERATE)
		return super::postProcessParam(p);
	//if the param is file or {o},  close the file
	if(param->pipeInfo&(PLAIN_FILE|PIPE_FILENAME)){
		file.close();
	}
	redirectFooter(param);
	try{
		if(nukable()&&nukeMode)
			argumentProcesser<NukeArgument>(param->value);
	}catch(std::exception& e){
		std::cerr<<"Error occured when nuking the file:"<<std::endl;
		std::cerr<<e.what()<<std::endl;
		std::cerr<<"The program is going to continue anyway."<<std::endl;
	}
	return END_BLOCK;
}

class SecureInputArgument:public InputArgument{
	protected:
		typedef InputArgument super;
		/*wipe the buffer if it is shrinked.*/
		virtual void resizeBuffer(size_t newSize){
			if(newSize<buffer.size())
				wipeBuffer(buffer.size());
			buffer.resize(newSize);
		}
	public:
	SecureInputArgument(const char* name):InputArgument(name){}
	/*wipe the buffer thrice*/
	void wipeBuffer(size_t begin=0){
		for(int i=0;i<3;i++) std::copy(buffer.begin()+begin, buffer.end(), SecureRandomDevice<>(buffer.size()).gen());
	}
	/*wipe the buffer after processing the data to avoid data leak.*/
	virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override{
		BLOCK_PROC_STATUS ret = super::postProcessParam(p);
		wipeBuffer(); ///TODO: change the access date of the file.
		return ret;
	}
	/*In case the program throws an exception, the destructor is called. This can wipe the buffer right before the program is exited.*/ ///TODO: verify that it's true in the standard. This is only confirmed in GCC 4.6
	~SecureInputArgument(){ wipeBuffer(); }
};

/**TODO: poorly written. better to rewrite.*/
class StreamInputArgument:public InputArgument{
	private:
		typedef InputArgument super;
		std::shared_ptr<Param_t> param;
		bool readingBuffer;
	protected:
		/*virtual size_t readData(std::shared_ptr<const FileParam_t> param, size_t maxCount, size_t) override{
			size_t oldBufferSize = buffer.size();
			//resize the buffer
			resizeBuffer(buffer.size()+maxCount);

			//append buffer, increases dataSizeRead, shrink buffer to fit then redirect data.
			if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){
				file.sync();
				file.read(&buffer[oldBufferSize], maxCount);
				dataSizeRead += file.gcount();
				resizeBuffer(oldBufferSize+file.gcount());
				redirectData(param, &buffer[buffer.size()], file.gcount());
				return file.gcount();
			}else{
				pipeIStream.read(&buffer[oldBufferSize], maxCount);
				dataSizeRead += pipeIStream.gcount();
				resizeBuffer(oldBufferSize+pipeIStream.gcount());
				redirectData(param, &buffer[buffer.size()], pipeIStream.gcount());
			}
			return pipeIStream.gcount();
		}*/
		/*virtual bool eof(std::shared_ptr<const FileParam_t> param) final{
			if(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))){
				file.peek();
				return file.eof();
			}else if(param->pipeInfo&(SRC_DEST&PIPE_LEN_DATA)){
				return paramLenDataCompleted;
			}
			//param->pipeInfo&(SRC_DEST&PIPE_FILENAME)
			pipeIStream.peek();
			return pipeIStream.eof();
		}*/
	public:
		StreamInputArgument(const char* name):InputArgument(name){}
		virtual std::vector<char> readBuffer(size_t maxCount){
			if(maxCount==0) return std::vector<char>();

			PARAM_PROC_STATUS localStatus = status;
			while(buffer.size()<maxCount&&localStatus!=END_PROCESSING_BLOCK){
				readingBuffer = true;
				localStatus = processParam(param);
			}
			std::vector<char> ret(buffer.begin(), buffer.size()<maxCount? buffer.end(): buffer.begin()+maxCount);
			/*discard the buffer read.*/
			buffer = std::vector<char>(buffer.begin()+ret.size(), buffer.end());
			return ret;
		}
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override{
			resizeBuffer(0);
			param = p;
			return super::preProcessParam(param);
		}
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) override{
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
			PARAM_PROC_STATUS ret;

			///TODO: dirty trick to append data to buffer. Should rewrite.
			std::vector<char> oldBuffer = buffer;
			ret = super::processParam(p); //loads buffer.
			std::vector<char> bufferClone = buffer;
			resizeBuffer(oldBuffer.size()+buffer.size()); //resize the buffer
			std::copy(bufferClone.begin(), bufferClone.end(), &buffer[oldBuffer.size()]); //shift the latest generated data of the buffer to the right
			std::copy(oldBuffer.begin(), oldBuffer.end(), &buffer[0]); //write the old buffer in the shifted position.

			if(readingBuffer){
				readingBuffer = false;
				return ret;
			}
			return DECIDED_BY_OTHER_PARAM;
		}
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override{
			resizeBuffer(0);
			BLOCK_PROC_STATUS ret = super::postProcessParam(p); //loads buffer.
			assert(buffer.size()==0); //The behavior of generating data in genFooter() or postProcessParam() is vauge. Should the buffer generated be appended? or just ignore the previous content of the buffer? Therefore, generating data in genFooter() is disallowed for StreamInputArgument
			return ret;
		}
		virtual std::vector<char> getBuffer() const override{ assert(false); return std::vector<char>(); };

};

class SecureStreamInputArgument:public StreamInputArgument{ ///TODO: unimplemented. find a way to implement the secure feature. Probably by using multiple inheritance. hmm... but how?
	public:
		SecureStreamInputArgument(const char* name):StreamInputArgument(name){};
};


/* This class is replaced by FileNameArgument.
class FileInputArgument:public InputArgument{
	private:
		typedef InputArgument super;
	public:
		FileInputArgument(const char* name):InputArgument(name){}
		void validateParam(std::shared_ptr<const Param_t> p){
			super::validateParam(p);
			//Check whether {[(o)]} is used. If yes, throw an exception.
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
			if(param->pipeInfo&(SRC_DEST&(PIPE_PLAIN|PIPE_LEN_DATA)))
				throw std::invalid_argument("Only reading from file is allowed.");
		}
		PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) = 0;
		//used for getting help message
		virtual const char* getArgCode() override{ return "If"; }
		virtual const char* getArgDescription() override{ return "file input. Only reading from file is allowed."; }
};
*/
