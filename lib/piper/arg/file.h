/*piper/arg/file.h
This file holds the definition and the implementation of the FileArgument class.
FileArgument is a class that load a param as a file, std::cin or std::cout.
For example, FileArgument interpret (i) as using std::cin as a source of data, and reading it as a file.
FileArgument also supports loading files with regex.
*/

///TODO: document the definition of i, o, r, file, (), [] and {}.

///TODO:regex support. load the filenames to param->value iteratively. Make use of preProcessParam() and postProcessParam().
///TODO: think about it: Should this system allow inputFile == outputFile? (Currently, this is allowed.)

namespace{
	class RegexFileIterator: public boost::filesystem::recursive_directory_iterator{
		private:
			typedef boost::filesystem::recursive_directory_iterator super;
			//regexBase: e.g. /usr/share/bin///\w+ , then /usr/share/bin would be the base
			//regexExpr: e.g. /usr/share/bin///\w+ , then \w+ would be the expr
			uni::string regexBase, regexExpr;
			bool pipeReceiveMode;
			uni::string fileName; //for pipeReceiveMode
			bool endOfIteration; //always falst for REGEX_SLAVE and SRC_DEST&PIPE_REGEX
		public:
			RegexFileIterator(){
				this->pipeReceiveMode = true;
				this->endOfIteration = false;
			}
			RegexFileIterator(const uni::string& regexBase, const uni::string& regexExpr):super(regexBase){
				this->pipeReceiveMode = false;
				this->endOfIteration = false;
				this->regexBase = boost::filesystem::path(regexBase).string<std::basic_string<uni::char_t>>(); //convert the base into native format.
				this->regexExpr = regexExpr;
				if(!regexMatch(getFileNameWithoutBase(), regexExpr.c_str()))
					(*this)++;
			}
			RegexFileIterator& operator++(){
				assert(!pipeReceiveMode);
				uni::string fileNameWOBase;
				do{
					super::operator++();
					if(static_cast<super>(*this)==super()){
						endOfIteration = true;
						return *this;
					}
					fileNameWOBase = getFileNameWithoutBase();
				}while( !regexMatch(fileNameWOBase, regexExpr.c_str()) );
				return *this;
			}
			RegexFileIterator& operator++(int){ return this->operator++(); }
			RegexFileIterator& increment(boost::system::error_code&){
				throw std::runtime_error("Sorry. Unimplemented RegexFileIterator::increment().");
			}
			uni::string getFileName(){
				assert(!eoi());
				if(pipeReceiveMode)
					return fileName;
				return (super::operator*()).path().string<std::basic_string<uni::char_t>>();
			}
			uni::string getFileName(const uni::string& replaceBase, const uni::string& replaceExpr){
				assert(!eoi());
				uni::string fileNameWOBase = getFileNameWithoutBase();
				uni::string replacedFileName = std::regex_replace(fileNameWOBase, uni::regex(regexExpr), replaceExpr);
/*std::cerr<<"regex base="<<regexBase<<" regex expr="<<regexExpr<<std::endl;
std::cerr<<"replace base="<<replaceBase<<" replace expr="<<replaceExpr<<std::endl;
std::cerr<<"fileNameWOBase="<<fileNameWOBase<<" replacedFileName="<<replacedFileName<<std::endl;
std::cerr<<"getFileName()="<<getFileName()<<std::endl;*/ ///TODO: remove this debug message
				#ifdef __WIN32__
				return replaceBase+"\\"+replacedFileName;
				#elif defined(__unix__)||defined(__APPLE__)
				return replaceBase+"/"+replacedFileName;
				#endif
			}
			uni::string getFileNameWithoutBase(){
				assert(!eoi());
				if(pipeReceiveMode)
					return fileName;
				return getFileName().substr(regexBase.size()+1); //+1 to remove the slash
			}
			void setRegexExpr(const uni::string& regexExpr){
				assert(pipeReceiveMode);
				this->regexExpr = regexExpr;
			}
			void setRegexFileName(const uni::string& fileName){
				assert(pipeReceiveMode);
				this->fileName = fileName;
			}
			uni::string getRegexExpr(){ return regexExpr; }
			bool eoi(){	return endOfIteration; } //check whether the iteration is ended for REGEX_MASTER.
	};
	RegexFileIterator regexFileIterator;

	/*	PipeInfo is a set of enum that is used to represent the source/destination of data, as well as the redirection of the data.
		masks:
			SRC_DEST: the mask of source/destination to be used with source/encoding.
			REDIRECT: the mask of redirection to be used with source/encoding.
		source/encoding:
			PLAIN_FILE: the data is read from a file.
			PIPE_PLAIN: the data is read from std::cin/written to std::cout.
			PIPE_LEN_DATA: the data is read from std::cin/written to std::cout with [(uint32_t len,char* data), (uint32_t len,char* data), ...] encoding. The data ends with the len zero.
			PIPE_FILENAME: the filename is read from std::cin/written to std::cout with (uint32_t len,char* data) encoding. For InputArgument, the filename read will be opened to process.
			NO_PIPE: equals to SRC_DEST&PLAIN_FILE. Read from/write to filename in the param.

		FORMAT SPECS:
			PLAIN_FILE		just read from file. No formating.
			PIPE_PLAIN		char* data;
			PIPE_LEN_DATA	uint32_t magicNumber, [uint32_t dataLen>0, char* data]... uint32_t dataLen=0;
			PIPE_FILENAME	uint32_t magicNumber, uint32_t fileNameLen, uni::char_t* fileName;
				Please notice that PIPE_FILENAME is piped after the operation of the param is completed. This is to ensure that the file is processed by programs one by one. This is because the behavior of file race(two program open the same faile at the same time) is undefined.
			PIPE_REGEX		Wraps the data above with the following format:
				uint32_t magicNumber, uint32_t regexMasterExprLen, char* regexMasterExpr,
				[uint32_t fileNameWOBaseLengthFromRegexMaster>0, uni::char_t* fileName, (PIPE DATA) ]...
				uint32_t fileNameWOBaseLengthFromRegexMaster=0

		For !(pipeInfo&GENERATE): [usually InputArgument]
						SRC_DEST										REDIRECT
		PLAIN_FILE		read data from the filename in param			undefined
		PIPE_PLAIN		read data from pipe								redirect data to std::cout
		PIPE_LEN_DATA	read <len,data> from pipe 						redirect <len,data> to std::cout
		PIPE_FILENAME	read filename from pipe, then read that file	redirect the filename to std::cout

		For (pipeInfo&GENERATE): [e.g. OutputArgument]
						SRC_DEST											REDIRECT
		PLAIN_FILE		write processed data to filename in param			undefined
		PIPE_PLAIN		undefined											redirect processed data to std::cout
		PIPE_LEN_DATA	undefined 											redirect processed <len,data> to std::cout
		PIPE_FILENAME	read filename from pipe, then write to the file		redirect the filename to std::cout


		Here are the rules and restrictions of using PipeInfo:

		For i, o, file, both bracket have to be the same.
		For r, both bracket can be different.
		i sets pipeInfo = SRC_DEST&bracket
		o sets pipeInfo = REDIRECT&bracket
		file sets pipeInfo = SRC_DEST&PLAIN_FILE | REDIRECT&bracket
		r sets pipeInfo = SRC_DEST&openingbracket | REDIRECT&closingbracket

		For InputArgument, pipeInfo&SRC_DEST is the source of data and pipeInfo&REDIRECT is the place that the data going to be redirected.
		For OutputArgument, both pipeInfo&SRC_DEST and pipeInfo&REDIRECT are the place that the data going to be redirected, or be outputted. Technically, SRC_DEST&PLAIN_FILE | REDIRECT&PIPE_LEN_DATA is identical to SRC_DEST&PIPE_LEN_DATA | REDIRECT&PLAIN_FILE. However, only the former one is allowed to facilitate checking whether there are more than one PIPE_PLAIN in both std::cout and std::cin. so SRC_DEST&PIPE_WHATEVER never write data to std::cout and REDIRECT&PIPE_WHATEVER always write data to std::cout for all Argument.

		If i is used with r, the generated pipeInfo would be identical to the one without i. For example, both (r) and (ir) set pipeInfo = SRC_DEST&PIPE_PLAIN | REDIRECT&PIPE_PLAIN However, (r) is handlled by RedirectArgument, while (ir) is handled by InputArgument.

		restrictions:
		Both i can be used with r, and vice versa.
		only one SRC_DEST&PIPE_PLAIN and one REDIRECT&PIPE_LEN_DATA is allowed
		PIPE_FILENAME is only available for file param and i.
		For InputArgument: (pipeInfo&SRC_DEST) and must not contains o.
		For OutputArgument: (pipeInfo&(SRC_DEST&(PIPE_PLAIN|PIPE_LEN_DATA))) is not allowed
							(pipeInfo&(REDIRECT&PLAIN_FILE)) is not allowed.
							must not contains i
							[(or)]} is not allowed because the the filename is unknown.

		For RedirectArgument: (pipeInfo&SRC_DEST) && (pipeInfo&REDIRECT)

		examples:
		expr	pipeInfo
		(i)		SRC_DEST&PIPE_PLAIN
		{i}		SRC_DEST&PIPE_FILENAME
		[o]		REDIRECT&PIPE_LEN_DATA
		{o}		SRC_DEST&PIPE_FILENAME , please notice that it's SRC_DEST is used instead of REDIRECT.
		[o}		EXCEPTION THROWN because [(o)] does not allow different brackets
		[r)		SRC_DEST&PIPE_LEN_DATA | REDIRECT&PIPE_PLAIN
		[ir]	SRC_DEST&PIPE_LEN_DATA | REDIRECT&PIPE_LEN_DATA , handlled by InputArgument
		[r]		SRC_DEST&PIPE_LEN_DATA | REDIRECT&PIPE_LEN_DATA , handlled by RedirectArgument
		[or]	REDIRECT&PIPE_LEN_DATA , handlled
			by OutputArgument, which will THROW AN EXCEPTION because of SRC_DEST&PIPE_LEN_DATA is undefined.
		[io]	SRC_DEST&PIPE_PLAIN | REDIRECT&PIPE_LEN_DATA, when it is eiter handlled by
			IuptArgument or OutputArgument, this will THROW exception because o/i is not allowed.
		[ior]	same as [io]
		(ir]	SRC_DEST&PIPE_PLAIN | REDIRECT&PIPE_LEN_DATA
		(or]	SRC_DEST&PIPE_PLAIN | REDIRECT&PIPE_LEN_DATA
		[file)	EXCEPTION THROWN because {[(file)]} does not allow different brackets
		[file]	SRC_DEST&PLAIN_FILE | REDIRECT&PIPE_LEN_DATA
	*/
	enum PipeInfo{
		SRC_DEST = 0x00FF,
		REDIRECT = 0xFF00,

		PLAIN_FILE = 0x0101, //read from / write to file
		PIPE_PLAIN = 0x0202, /*() pipe*/
		PIPE_LEN_DATA = 0x0404, /*[] pipe*/
		PIPE_FILENAME = 0x0808, /*{} pipe*/
		PIPE_REGEX = 0x1010, /*<> pipe*/
		NOT_FILE = PIPE_PLAIN|PIPE_LEN_DATA|PIPE_FILENAME, //This is used instead of NOT_FILE because NOT_FILE returns an int, which will be filled with extra 1's

		GENERATE = 0x10000000,

		NO_PIPE = SRC_DEST&PLAIN_FILE
	};
	/*The enum that describe the regex information of the param.
		NO_REGEX: no regex is involved in the param.
		REGEX_MASTER: Act as a regex "generator". This is triggered by the expression dir///expr. It will iterate all files recursively inside the dir. Then, match the expr with the filenames iterated and store the matched part of the filename.
		REGEX_SLAVE: Act as a regex "receiver". This is triggered by the expression dir2//expr2. expr2 is replaced by the filenames stored in REGEX_MASTER is iteratively.
	*/
	enum RegexInfo{NO_REGEX, REGEX_MASTER, REGEX_SLAVE};

	/*A derived class from Param_t for storing information for FileArgument.*/
	struct FileParam_t:public Param_t{
		uint32_t pipeInfo; //A field that store a masked enum PipeInfo, as described above.
		/*for regex*/
		RegexInfo regexInfo; //A field that store enum RegexInfo, as described above.
		//regexBase: e.g. /usr/share/bin///\w+ , then /usr/share/bin would be the base
		//regexExpr: e.g. /usr/share/bin///\w+ , then \w+ would be the expr
		uni::string regexBase, regexExpr; //for regex slave. For regex master,
	};

	size_t CHUNK_SIZE = 1024*1024;
}

class PipeStream: public std::stringstream{
	private:
		typedef std::stringstream super;
		bool secure;
	protected:
		bool blocked;
	public:
		PipeStream(){}
		PipeStream(bool blocked, bool secure=false, std::ios_base::openmode openMode = std::ios_base::in|std::ios_base::out|std::ios_base::binary ):std::stringstream(openMode){
			this->blocked = blocked;
			this->secure = secure;
		}
		virtual void init(bool blocked, bool secure=false){ //damn it! GCC haven't implement std::stringstream::swap(), making it impossible to use move assignment operator.
			this->blocked = blocked;
			this->secure = secure;
			super::seekg(0);
			super::seekp(0);
			super::str("");
		}
		~PipeStream(){
			std::cout.flush();
			if(secure){
				super::seekg(0, std::ios_base::end);
				std::streamsize size = super::tellg();
				for(size_t i=0;i<3;i++){
					super::seekp(0);
					std::streamsize unwipedSize = size;
					while(unwipedSize!=0){
						std::streamsize toBeWipedSize = unwipedSize>static_cast<std::streamsize>(CHUNK_SIZE)?CHUNK_SIZE:unwipedSize;
						super::write(SecureRandomDevice<char>(toBeWipedSize).gen(), toBeWipedSize);
						unwipedSize -= toBeWipedSize;
					}
				}
				super::flush();
			}
		}
		void setSecure(bool secure){ this->secure = secure; }
};

class PipeIStream: public PipeStream{
	private:
		typedef PipeStream super;
		bool loaded;
		/*read from std::cin, write to buffer stream.*/
		void readInner(char* buffer, size_t maxSize){
			std::cin.read(buffer, maxSize);
			super::write(buffer, std::cin.gcount());
		}
		void readMagicNumber(uint32_t magicNumber){
			uint32_t dummy;
			readInner(reinterpret_cast<char*>(&dummy), sizeof(uint32_t));
			if(dummy!=magicNumber)
				throw std::runtime_error("Wrong encoding format from pipe. Invalid magic number.");
		}
		template <typename len_type=uint32_t, typename data_type=char>
		len_type readLenData(){
			len_type dataLen;
			readInner(reinterpret_cast<char*>(&dataLen), sizeof(len_type));
			if(dataLen==0)
				return dataLen;
			std::unique_ptr<char> buffer(new char[dataLen]);
			readInner(buffer.get(), dataLen*sizeof(data_type));
			if(static_cast<len_type>(std::cin.gcount())!=dataLen*sizeof(data_type))
				throw std::runtime_error("Wrong encoding format from pipe. Reach EOF unexpectedly.");
			return dataLen;
		}
		void loadSingle(uint32_t pipeReadInfo){
			if(pipeReadInfo&PIPE_PLAIN){
				std::unique_ptr<char> buffer(new char[CHUNK_SIZE]);
				while(!std::cin.eof())
					readInner(buffer.get(), CHUNK_SIZE);
			}else if(pipeReadInfo&PIPE_LEN_DATA){
				readMagicNumber(PIPE_LEN_DATA);
				while(readLenData()>0){}
			}else if(pipeReadInfo&PIPE_FILENAME){
				readMagicNumber(PIPE_FILENAME);
				readLenData<uint32_t,uni::string::value_type>();
			}
		}
	public:
		PipeIStream(){}
		PipeIStream( bool blocked, bool secure=false, std::ios_base::openmode openMode = std::ios_base::in|std::ios_base::out|std::ios_base::binary ):super(blocked, secure, openMode){
			this->init(blocked, secure);
		}
		virtual void init(bool blocked, bool secure=false) override{
			super::init(blocked, secure);
			loaded = false;
		}
		/*If blocked, reads from std::cin to stream buffer.*/
		PipeStream& load(std::shared_ptr<const FileParam_t> param){
			assert(!loaded);
			loaded = true;
			if(!blocked)
				return *this;

			uint32_t pipeReadInfo = param->pipeInfo&SRC_DEST;
			if(pipeReadInfo&PIPE_REGEX){
				readMagicNumber(PIPE_REGEX);
				readLenData<uint32_t,uni::string::value_type>(); //reads regex master expression.
				while(readLenData<uint32_t,uni::string::value_type>()>0) //reads a file name from regex master.
					loadSingle(pipeReadInfo);
			}else if(pipeReadInfo&(PIPE_PLAIN|PIPE_LEN_DATA|PIPE_FILENAME)){
				loadSingle(pipeReadInfo);
			}
			super::flush();
			return *this;
		}
		/*If blocked, reads from stream buffer, writes to the destinition. Else, read from std::cin*/
		std::istream& read(char_type* s, std::streamsize count){
			if(!blocked){
				//return std::cin.read(s, count);
				std::istream& ret = std::cin.read(s, count);
//std::cerr<<"read: "<<super::gcount()<<"/"<<count<<" data="<<*reinterpret_cast<uint32_t*>(s)<<std::endl;
				return ret;
			}
			//return super::read(s, count);
			std::istream& ret = super::read(s, count);
//std::cerr<<"read: "<<super::gcount()<<"/"<<count<<" data="<<*reinterpret_cast<uint32_t*>(s)<<std::endl;
			return ret;
		}
		std::streamsize gcount() const{
			if(!blocked)
				return std::cin.gcount();
			return super::gcount();
		}
		int_type peek(){
			if(!blocked)
				return std::cin.peek();
			return super::peek();
		}
		bool eof() const{
			if(!blocked)
				return std::cin.eof();
			return super::eof();
		}
};

class PipeOStream: public PipeStream{
	private:
		typedef PipeStream super;
		bool written;
	public:
		PipeOStream(){}
		PipeOStream( bool blocked, bool secure=false, std::ios_base::openmode openMode = std::ios_base::in|std::ios_base::out|std::ios_base::binary ):super(blocked, secure, openMode){ }
		/*If blocked, read from source, write to stream buffer. Else, write to std::cout*/
		std::ostream& store(const char_type* s, std::streamsize count){
if(count==0) return std::cout;
			if(!blocked){
				std::ostream& ret = std::cout.write(s, count);
				std::cout.flush();
//std::cerr<<"write: "<<count<<" data="<<*reinterpret_cast<const uint32_t*>(s)<<std::endl;
				return ret;
			}
			std::ostream& ret = super::write(s, count);
			super::flush();
			return ret;
		}
		virtual void init(bool blocked, bool secure=false) override{
			super::init(blocked, secure);
			this->written = false;
		}
		/*If blocked, read from stream buffer, write to std::cout*/
		PipeStream& write(){
			assert(!written);
			written = true;
			if(!blocked)
				return *this;
			super::peek();
			std::unique_ptr<char> buffer(new char[CHUNK_SIZE]);
			while(!super::eof()){
				super::read(buffer.get(), CHUNK_SIZE);
				std::cout.write(buffer.get(), super::gcount());
				if(super::gcount()==0)
					break;
			}
			std::cout.flush();
			return *this;
		}
};

class FileArgument: public Argument{
	private:
		/*Define a super keyword to access members in super class.*/
		typedef Argument super;
	protected:
		PipeIStream pipeIStream;
		PipeOStream pipeOStream;
		const static size_t CHUNK_SIZE = 1024*1024;
		/*write/read magic number from/to std::cin/std::cout*/
		void writePipeMagicNumber(uint32_t magicNumber);
		void checkPipeMagicNumber(uint32_t expectedMagicNumber);
		/*read file name from std::cin*/
		uni::string readFileNameFromStdin();
		/*If necessary, writes the data length of the data to std::cout for redirection.*/
		void redirectHeader(std::shared_ptr<const FileParam_t> param);
		/*writes the data(PIPE_PLAIN or PIPE_LEN_DATA) to std::cout for redirection.*/
		void redirectData(std::shared_ptr<const FileParam_t> param, const char* buffer, uint32_t bufferSize);
		/*writes the footer(filename) to std::cout for redirection.*/
		void redirectFooter(std::shared_ptr<const FileParam_t> param);
	public:
		/*A non-default constructor that takes a name. It calls the constructor of Argument and pass the name to it.*/
		FileArgument(const char* name):Argument(name){}
		/*calls loadParam() in Argument. Then create a FileParam_t with the following field loaded:
			param->pipeInfo, param->regexInfo, param->regexBase, param->regexExpr
			If the param is {[(file)]}, the bracket is removed from param->value so that param->value contains the filename.
			This function throws an exception if there is a syntax error.*/
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> p) override;
		/*This function throws an exception if there is something like (regex) because regex cannot be enclosed by ().
			This functino is also inherited to perform validation for derived class.*/
		virtual void validateParam(std::shared_ptr<const Param_t> p) override;
		/*This function  throw exceptions in block base in the following situations:
			multiple regex master
			reading from std::cin with multiple ()
			writing to std::cout with multiple ()
			[undocumented]there are some more cases that it throws exception.
			*/
		virtual void loadAndValidateBlock(std::vector<std::shared_ptr<Param_t>> block, size_t blockOffset) override;

		void loadRegexFileName(std::shared_ptr<Param_t> p);
		virtual PARAM_PROC_STATUS initParamProcStatus() = 0;
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override = 0;
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) override = 0;
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override = 0;

		virtual BLOCK_PROC_STATUS preProcessBlock(std::vector<std::shared_ptr<Param_t>> block, size_t blockOffset) override;
		virtual BLOCK_PROC_STATUS postProcessBlock(std::vector<std::shared_ptr<Param_t>> block, size_t blockOffset) override;
		PARAM_PROC_STATUS repeatRegex(std::shared_ptr<const Param_t> p, PARAM_PROC_STATUS stat);

		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "file"; }
		virtual const char* getArgDescription() override{ return "undocumented"; }
};

std::shared_ptr<Param_t> FileArgument::loadParam(const uni::string& str, std::shared_ptr<Param_t> p=nullptr){
	if(p==nullptr) p = std::make_shared<FileParam_t>();
	super::loadParam(str, p);
	std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);

	///TODO: load PIPE_REGEX and remove <> here.

	param->pipeInfo = 0x0; //Please notice that 0x0 != NO_PIPE. NO_PIPE menas SRC_DEST&PLAIN_FILE.
	uint32_t regexPipe = 0x0;
	if(regexMatch(param->value, R"(<[\{\[\(].*[\)\]\}]>)")){
		param->value = param->value.substr(1,param->value.size()-2);
		regexPipe = PIPE_REGEX;
	}

	/*load the pipeInfo*/
	//check whether the param is enclosed with {[( and )]} . If yes, trigger pipeInfo
	const std::map<char, uint32_t> bracketToPipe = {
		{'(', PIPE_PLAIN|regexPipe},		{')', PIPE_PLAIN|regexPipe},
		{'[', PIPE_LEN_DATA|regexPipe},		{']', PIPE_LEN_DATA|regexPipe},
		{'{', PIPE_FILENAME|regexPipe},		{'}', PIPE_FILENAME|regexPipe},
	};
	if(regexMatch(param->value, R"([\{\[\(][ior]{1,2}[\)\]\}])")){ //if i,o or/and r are enclosed by brackets
		if(param->value.find("i")!=uni::string::npos){
			param->pipeInfo |= SRC_DEST&bracketToPipe.at(param->value.front());
		}
		if(param->value.find("o")!=uni::string::npos){
			if(bracketToPipe.at(param->value.back())&PIPE_FILENAME) /*{o}*/
				throw std::invalid_argument("{o} is not allowed.");
			else /*[(o)]*/
				param->pipeInfo |= REDIRECT&bracketToPipe.at(param->value.back());
		}
		if(param->value.find("r")!=uni::string::npos){
			if(!(param->pipeInfo&SRC_DEST))
				param->pipeInfo |= SRC_DEST&bracketToPipe.at(param->value.front());
			if(param->pipeInfo&REDIRECT)
				throw std::invalid_argument("{[(or)]} is not allowed.");
			param->pipeInfo |= REDIRECT&bracketToPipe.at(param->value.back());
		}else{
			if(bracketToPipe.at(param->value.front())!=bracketToPipe.at(param->value.back()))
				throw std::invalid_argument("Brackets enclosing i or o must be the same.");
		}
	}else if(regexMatch(param->value, R"([\{\[\(].*[\)\]\}])")){ //If a filename is enclosed by the brackets.
			if(bracketToPipe.at(param->value.front())!=bracketToPipe.at(param->value.back()))
				throw std::invalid_argument("Brackets enclosing filename must be the same.");
			param->pipeInfo = (SRC_DEST&PLAIN_FILE)|(REDIRECT&bracketToPipe.at(param->value.front()));
			param->value = param->value.substr(1, param->value.size()-2 ); //remove the bracket to ease reading filename.
	}else param->pipeInfo = NO_PIPE; //NO_PIPE = SRC_DEST&PLAIN_FILE
	std::function<bool(const uni::string, RegexInfo)>
		loadRegexInfo = [&param](const uni::string str, RegexInfo setRegexInfo){
			size_t matchPos;
			if((matchPos=param->value.find(str))!=uni::string::npos){
				param->regexInfo = setRegexInfo;
				param->regexBase = param->value.substr(0, matchPos);
				param->regexExpr = param->value.substr(matchPos+str.size());
				if(setRegexInfo==REGEX_MASTER)
					regexFileIterator = std::move(RegexFileIterator(param->regexBase, param->regexExpr));
				return true;
			}
			return false;
		};

	if(param->pipeInfo&(SRC_DEST&PIPE_REGEX))
		regexFileIterator = std::move(RegexFileIterator());

	/*Now load the regex part.*/
	param->regexInfo = NO_REGEX;
	if(!loadRegexInfo("///", REGEX_MASTER)){
		loadRegexInfo("//", REGEX_SLAVE);
	}

	return param;
}

void FileArgument::validateParam(std::shared_ptr<const Param_t> p){
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	if(param->regexInfo!=NO_REGEX && (param->pipeInfo&PIPE_PLAIN))
		throw std::invalid_argument("() is not allowed to be used with regex.");

	///TODO: This disallows <[o]>, which is supposed to be allowed.
	if(param->regexInfo==NO_REGEX && (param->pipeInfo&(REDIRECT&PIPE_REGEX)))
		throw std::invalid_argument("<> redirection is not allowed for non-regex param.");
	if(!(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME)))&&(param->pipeInfo&(REDIRECT&PIPE_FILENAME)))
		throw std::invalid_argument("invalid use of *} : file name is not available for redirection.");
}

std::vector<std::shared_ptr<Param_t>> block;

namespace{
size_t latestBlockLoadAndValidated = 0;
bool preProcessBlockCalledInThisIteration = false;
size_t postProcessBlockCalledInThisIteration = false;
bool regexBlock = false;
bool regexFromPipe = false;
/*these variables are ignored if !regexBlock*/
bool firstIterationOfThisRegexBlock = true;
bool lastIterationOfThisRegexBlock = true;
}

void FileArgument::loadAndValidateBlock(std::vector<std::shared_ptr<Param_t>> block, size_t blockOffset){
	firstIterationOfThisRegexBlock = true;
	lastIterationOfThisRegexBlock = false;
	preProcessBlockCalledInThisIteration = false;
	postProcessBlockCalledInThisIteration = false;
	//check whether the block have been load and processed by this Argument. If so, skip loading and validating.
	if(blockOffset<=latestBlockLoadAndValidated)
		return;
	latestBlockLoadAndValidated = blockOffset;
	::block = block;
	/*This functions accept a function as an parameter. It counts the number of FileParam_t in
	the block that match a specific criteria.*/
	std::function<int(std::function<bool(std::shared_ptr<const FileParam_t>)>)>
		count = [&block](std::function<bool(std::shared_ptr<const FileParam_t>)> comparator){
			return std::count_if(block.begin(), block.end(),
				[&comparator](std::shared_ptr<const Param_t> p){
					std::shared_ptr<const FileParam_t> param = std::dynamic_pointer_cast<const FileParam_t>(p); //do NOT change it to static_cast because it is not guaranteed that p is a FileParam_t
					return param!=nullptr && comparator(param);
				});
		};
	/*validate the block*/
	//throw an exception if the number of regex master > 1 in a block
	if( count( [](std::shared_ptr<const FileParam_t> param){ return param->regexInfo==REGEX_MASTER; } ) > 1 )
		throw std::invalid_argument("Only one regex master is allowed in a block.");
	if( count( [](std::shared_ptr<const FileParam_t> param){ return param->regexInfo==REGEX_MASTER; } ) >= 1 && count( [](std::shared_ptr<const FileParam_t> param){ return param->pipeInfo&(SRC_DEST&PIPE_REGEX); } ) >= 1 )
		throw std::invalid_argument("regex master cannot be used with <i>");
	if( count( [](std::shared_ptr<const FileParam_t> param){ return param->regexInfo==REGEX_MASTER||param->pipeInfo&(SRC_DEST&PIPE_REGEX); } ) == 0 && count( [](std::shared_ptr<const FileParam_t> param){ return param->regexInfo==REGEX_SLAVE; } ) >= 1 )
		throw std::invalid_argument("regex slave must come with a regex master or a <i>");

	//throw an exception if there are more than one PIPE_PLAIN in std::cin in a block
	if( count( [](std::shared_ptr<const FileParam_t> param){ return (param->pipeInfo&(SRC_DEST&PIPE_PLAIN)); } ) > 1 ) ///FIXME: this only checks the number of () of a block. It does not check for the entire command.
		throw std::invalid_argument("Only one param reading from std::cin using () is allowed in entire command");
	//throw an exception if there are more than one PIPE_PLAIN in std::cout in a block
	if( count( [](std::shared_ptr<const FileParam_t> param){ return (param->pipeInfo&(REDIRECT&PIPE_PLAIN)); } ) > 1 )
		throw std::invalid_argument("Only one param writing to std::cout using () is allowed in a block.");

}

BLOCK_PROC_STATUS FileArgument::preProcessBlock(std::vector<std::shared_ptr<Param_t>> block, size_t){
	if(preProcessBlockCalledInThisIteration)
		return REPEAT_BLOCK;
	preProcessBlockCalledInThisIteration = true;
	if(firstIterationOfThisRegexBlock)
		regexBlock = false;
	/*initialize pipeIStream and pipeOStream*/
	if(firstIterationOfThisRegexBlock){
		/*I: The last one is not blocked.
		O: the first one is not blocked.*/
		/*initialize pipeIStream*/
		bool blocked = false;
		for(std::vector<std::shared_ptr<Param_t>>::reverse_iterator it=block.rbegin(); it!=block.rend(); it++){
			std::shared_ptr<FileParam_t> param = std::dynamic_pointer_cast<FileParam_t>(*it);
			if(param==nullptr)
				continue;
			std::shared_ptr<FileArgument> arg = std::static_pointer_cast<FileArgument>(param->arg);

			arg->pipeIStream.init(param->pipeInfo&(SRC_DEST&PIPE_REGEX)? true: blocked);
			if(param->pipeInfo&(SRC_DEST&(PIPE_PLAIN|PIPE_LEN_DATA|PIPE_FILENAME)))
				blocked = true;
		}
		/*initialize pipeOStream and load pipeIStream.*/
		blocked = false;
		for(auto& p:block){
			std::shared_ptr<FileParam_t> param = std::dynamic_pointer_cast<FileParam_t>(p);
			if(param==nullptr)
				continue;
			std::shared_ptr<FileArgument> arg = std::static_pointer_cast<FileArgument>(param->arg);

			arg->pipeIStream.load(param); //do NOT move it up because the iteration above is a reverse iterator
			arg->pipeOStream.init(blocked);
			if(param->pipeInfo&(REDIRECT&(PIPE_PLAIN|PIPE_LEN_DATA|PIPE_FILENAME)))
				blocked = true;
		}
	}
	std::vector<std::shared_ptr<Param_t>> oldBlock = block;
	/*sort block so that REGEX_MASTER and <i> are loaded first. Please notice that this function have made a copy of block. Sorting here won't affact the global order of block.*/
	std::sort(block.begin(), block.end(), [](const std::shared_ptr<Param_t>& a, const std::shared_ptr<Param_t>& b){
		std::shared_ptr<FileParam_t> paramA = std::dynamic_pointer_cast<FileParam_t>(a);
		std::shared_ptr<FileParam_t> paramB = std::dynamic_pointer_cast<FileParam_t>(b);
		if(paramA==nullptr&&paramB==nullptr){
			return false;
		}else if(paramA!=nullptr&&paramB==nullptr){
			return true;
		}else{ //(paramA!=nullptr&&paramB!=nullptr)
			if(paramA->regexInfo==REGEX_MASTER||paramA->pipeInfo&(SRC_DEST&PIPE_REGEX))
				return true;
		}
		return false;
	});

	BLOCK_PROC_STATUS ret = REPEAT_BLOCK;

	if(regexFileIterator.eoi()){
		uint32_t zero = 0;
		for(auto& p:block){
			std::shared_ptr<FileParam_t> param = std::dynamic_pointer_cast<FileParam_t>(p);
			if(param==nullptr)
				continue;
			std::shared_ptr<FileArgument> arg = std::static_pointer_cast<FileArgument>(param->arg);
			if(param->pipeInfo&(REDIRECT&PIPE_REGEX))
				arg->pipeOStream.store(reinterpret_cast<char*>(&zero), sizeof(uint32_t));
		}
		ret = END_BLOCK;
	}else{
		/*load param->value from regex.*/
		uni::string oldRegexMasterFileNameWOBase;
		uni::string oldRegexExpr;
		for(auto& p:block){
			std::shared_ptr<FileParam_t> param = std::dynamic_pointer_cast<FileParam_t>(p);
			if(param==nullptr)
				continue;
			std::shared_ptr<FileArgument> arg = std::static_pointer_cast<FileArgument>(param->arg);
			if(param->regexInfo==REGEX_MASTER){
				param->value = regexFileIterator.getFileName();
std::cerr<<"read from:"<<param->value<<std::endl;
				regexBlock = true;
				regexFromPipe = false;
			}else if(param->regexInfo==REGEX_SLAVE){
				param->value = regexFileIterator.getFileName(param->regexBase, param->regexExpr);
std::cerr<<"writing to:"<<param->value<<std::endl;
			}
			if(param->pipeInfo&(SRC_DEST&PIPE_REGEX)){
				if(firstIterationOfThisRegexBlock){
					/*do NOT use checkMagicNumber() because checkMagicNumber() assumes that it is called by the param Arument. However, in this function, the magicNumber check performs in block-base. Only the first Argument in the block can process this block. and first Argument!=the parent Argument of param.
					Simply said, arg->pipeIStream is used instead of this->pipeIStream here. However, this->pipeIStream is used in checkMagicNumber(), making checkMagicNumber() unsuitable.*/
					decltype(param->pipeInfo) magicNumber;
					arg->pipeIStream.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));
					if(magicNumber!=PIPE_REGEX)
						throw std::invalid_argument("Wrong encoding data. Check your syntax.");
					regexBlock = true;
					regexFromPipe = true;
					size_t regexExprLen;
					arg->pipeIStream.read(reinterpret_cast<char*>(&regexExprLen), sizeof(uint32_t));
					uni::string regexExpr(regexExprLen, '\0');
					arg->pipeIStream.read(reinterpret_cast<char*>(&regexExpr[0]), regexExprLen*sizeof(uni::string::value_type));

					regexFileIterator.setRegexExpr(regexExpr);

					if(!oldRegexExpr.empty()&&oldRegexExpr!=regexExpr)
						throw std::invalid_argument("The source of <i>s in this block are not the same.");

					oldRegexExpr = regexExpr;
				}
				size_t regexMasterFileNameWOBaseLen;
				arg->pipeIStream.read(reinterpret_cast<char*>(&regexMasterFileNameWOBaseLen), sizeof(uint32_t));
				if(regexMasterFileNameWOBaseLen==0){
					ret = END_BLOCK;
				}else{
					uni::string regexMasterFileNameWOBase(regexMasterFileNameWOBaseLen, '\0');
					arg->pipeIStream.read(reinterpret_cast<char*>(&regexMasterFileNameWOBase[0]), regexMasterFileNameWOBaseLen*sizeof(uni::string::value_type));

					regexFileIterator.setRegexFileName(regexMasterFileNameWOBase);

					if(!oldRegexMasterFileNameWOBase.empty()&&
						oldRegexMasterFileNameWOBase!=regexMasterFileNameWOBase)
						throw std::invalid_argument("The source of <i>s in this block are not the same.");

					oldRegexMasterFileNameWOBase = regexMasterFileNameWOBase;

				}
			}else if(param->pipeInfo&(REDIRECT&PIPE_REGEX)){
				if(firstIterationOfThisRegexBlock){
					/*Do NOT use writeMagicNumber() because it writes to this->pipeOStream instead of arg->pipeOStream*/
					decltype(param->pipeInfo) magicNumber = PIPE_REGEX;
					arg->pipeOStream.store(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber)); //magic number.
					uni::string regexExpr = regexFileIterator.getRegexExpr();
					size_t regexExprLen = regexExpr.size();
					arg->pipeOStream.store(reinterpret_cast<char*>(&regexExprLen), sizeof(uint32_t));
					arg->pipeOStream.store(reinterpret_cast<char*>(&regexExpr[0]), regexExprLen*sizeof(uni::string::value_type));
				}

				uni::string regexMasterFileNameWOBase = regexFileIterator.getFileNameWithoutBase();
				size_t regexMasterFileNameWOBaseLen = regexMasterFileNameWOBase.size();
				arg->pipeOStream.store(reinterpret_cast<char*>(&regexMasterFileNameWOBaseLen), sizeof(uint32_t));
				arg->pipeOStream.store(reinterpret_cast<char*>(&regexMasterFileNameWOBase[0]), regexMasterFileNameWOBaseLen*sizeof(uni::string::value_type));

			}
		}
	}
	postProcessBlockCalledInThisIteration = false;
	if(ret==END_BLOCK)
		lastIterationOfThisRegexBlock = true;
	return ret;
}

BLOCK_PROC_STATUS FileArgument::postProcessBlock(std::vector<std::shared_ptr<Param_t>> block, size_t){
	if(postProcessBlockCalledInThisIteration)
		return END_BLOCK;
	postProcessBlockCalledInThisIteration = true;

	std::function<void()> writeOut = [this, &block](){
		for(auto& p:block){
			std::shared_ptr<FileParam_t> param = std::dynamic_pointer_cast<FileParam_t>(p);
			if(param==nullptr)
				continue;
			std::shared_ptr<FileArgument> arg = std::static_pointer_cast<FileArgument>(param->arg);
			arg->pipeOStream.write();
		}
	};

	firstIterationOfThisRegexBlock = false;
	preProcessBlockCalledInThisIteration = false;

	if(!regexBlock||lastIterationOfThisRegexBlock){
		writeOut();
		firstIterationOfThisRegexBlock = true;
		return END_BLOCK;
	}

	if(!regexFromPipe)
		regexFileIterator++;

	return REPEAT_BLOCK;
}

void FileArgument::writePipeMagicNumber(uint32_t magicNumber){
	pipeOStream.store(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber)); //magic number.
}

void FileArgument::checkPipeMagicNumber(uint32_t expectedMagicNumber){
	decltype(expectedMagicNumber) magicNumber;
	pipeIStream.read(reinterpret_cast<char*>(&magicNumber), sizeof(magicNumber));
	if(magicNumber!=expectedMagicNumber)
		throw std::invalid_argument("Wrong encoding data. Invalid magic number. Check you syntax.");
}

uni::string FileArgument::readFileNameFromStdin(){
	checkPipeMagicNumber(PIPE_FILENAME);
	uni::string fileName;
	uint32_t fileNameLen;
	pipeIStream.read(reinterpret_cast<char*>(&fileNameLen), sizeof(uint32_t));
	fileName.resize(fileNameLen);
	pipeIStream.read(reinterpret_cast<char*>(&fileName[0]), fileNameLen*sizeof(uni::string::value_type));
	if(!boost::filesystem::exists(fileName))
		throw std::invalid_argument(std::string("The file name '")+fileName+"' received from {} does not exist.");
	return fileName;
}

void FileArgument::redirectHeader(std::shared_ptr<const FileParam_t> param){
	assert(!(param->pipeInfo&(REDIRECT&PLAIN_FILE))); //REDIRECT is not designed for writing to file. If you are writing data to a file, use SRC_DEST instead.
	if(param->pipeInfo&(REDIRECT&PIPE_LEN_DATA)){ //If param is a [] redirect
		writePipeMagicNumber(PIPE_LEN_DATA);
	}
}

void FileArgument::redirectData(std::shared_ptr<const FileParam_t> param, const char* buffer, uint32_t bufferSize){
	if(param->pipeInfo&(REDIRECT&PIPE_PLAIN)){ //If param is a ()
		pipeOStream.store(buffer, bufferSize);
	}else if(param->pipeInfo&(REDIRECT&PIPE_LEN_DATA)){ //If param is a []
		if(bufferSize==0) return; //Do NOT write a data chunk begins with zero unless it is the end of the data section.
		pipeOStream.store(reinterpret_cast<char*>(&bufferSize), sizeof(uint32_t));
		pipeOStream.store(buffer, bufferSize);
	}
}

void FileArgument::redirectFooter(std::shared_ptr<const FileParam_t> param){
	if(param->pipeInfo&(REDIRECT&PIPE_LEN_DATA)){ //If param is a []
		uint32_t zero = 0;
		pipeOStream.store(reinterpret_cast<char*>(&zero), sizeof(uint32_t));
	}else if(param->pipeInfo&(REDIRECT&PIPE_FILENAME)){ //write the filename gere instead of writing it in header to ensure that the file is processed
		assert(param->pipeInfo&(SRC_DEST&PLAIN_FILE));
		writePipeMagicNumber(PIPE_FILENAME);
		uint64_t fileNameLen = param->value.size();
		pipeOStream.store(reinterpret_cast<char*>(&fileNameLen), sizeof(uint32_t));
		pipeOStream.store(reinterpret_cast<const char*>(param->value.c_str()), fileNameLen*sizeof(uni::string::value_type));
	}
}
