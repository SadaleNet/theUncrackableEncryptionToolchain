/*piper/arg/redirect.h
This file holds the definition and the implementation of the RecirectArgument class.
RecirectArgument is triggered by {[(r)]}
RecirectArgument does pretty much the same thing as InputArgument, except that:
	a) It's a special argument(i.e. triggered argument)
	b) It have a different validation code.
*/

class RecirectArgument:public InputArgument{
	private:
		/*define a super keyword*/
		typedef InputArgument super;
	public:
		RecirectArgument(const char* name):InputArgument(name){}
		virtual bool triggerArgument(const uni::string& str) override{ return regexMatch(str, R"([\{\[\(]r[\)\]\}])"); } ///TODO: does not support regex pipe redirection.
		virtual void validateParam(std::shared_ptr<const Param_t> p) override;
		virtual void advanceArgPtr(size_t& argPtr) override{
			if(argPtr!=0)
				throw std::invalid_argument("redirect argument cannot be used in the middle or at the end of the block.");
			argPtr += arguments.size();
		}
		/*used for getting help message*/
		virtual const char* getArgCode() override{ return "redir"; }
		virtual const char* getArgDescription() override{ return "redirect. redirect the data from stdin to stdout. Triggered by {[(r)]}"; }
};

void RecirectArgument::validateParam(std::shared_ptr<const Param_t> p){
	FileArgument::validateParam(p);
	std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
	if(!(param->pipeInfo&(SRC_DEST&PIPE_FILENAME)) && (param->pipeInfo&(SRC_DEST&PIPE_FILENAME))) /*([o}*/
		throw std::invalid_argument("(r} or [r} is not allowed.");
	assert(!(param->pipeInfo&PLAIN_FILE));
}
