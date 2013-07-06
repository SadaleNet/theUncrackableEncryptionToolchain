/*fileName.h
This file holds the definition of FileNameArgument.
FileNameArgument does not open/close the file. It requires derived class to open/close if it needs to.
*/

class FileNameArgument: public FileArgument{
	private:
		typedef FileArgument super;
	public:
		FileNameArgument(const char* name):FileArgument(name){};
		virtual void validateParam(std::shared_ptr<const Param_t> p) override{
			super::validateParam(p);
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);

			if(!(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME))) || //Must have a file on SRC_DEST
				param->pipeInfo&(REDIRECT&(PIPE_PLAIN|PIPE_LEN_DATA))) //Must NOT redirect non-filename.
				throw std::invalid_argument("Only {i}, {ir}, file and {file} are allowed.");

			assert(!(param->pipeInfo&(REDIRECT&PLAIN_FILE)));
		}
		virtual PARAM_PROC_STATUS initParamProcStatus() override{ return REPEAT_PROCESSING_BLOCK; }
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override final{
			std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
			if(param->pipeInfo&(SRC_DEST&PIPE_FILENAME))
				param->value = readFileNameFromStdin();
			redirectHeader(param);
			assert(param->pipeInfo&(SRC_DEST&(PLAIN_FILE|PIPE_FILENAME)));
			assert(!(param->pipeInfo&(REDIRECT&(PIPE_PLAIN|PIPE_LEN_DATA))));
			return status;
		}
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) override = 0;
		virtual BLOCK_PROC_STATUS postProcessParam(std::shared_ptr<const Param_t> p) override final{
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
			redirectFooter(param);
			return END_BLOCK;
		}
		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "Fn"; }
		virtual const char* getArgDescription() override{ return "File name argument. A argument that only accept file, {file}, {i} and {ir} as a parameter."; }
};
