void copyFile(const boost::filesystem::path& from, const boost::filesystem::path& to){
	std::string command;
	#ifdef __WIN32__
		std::string fromStr = from.string(), toStr = to.string();
		std::replace(fromStr.begin(), fromStr.end(), '/', '\\');
		std::replace(toStr.begin(), toStr.end(), '/', '\\');
		//convert the strings to  .c_str() to prevent '\0' from being written in the middle of the string. I suspect that it's a bug of MinGW :S
		command = std::string(std::string("copy /b /y ")+fromStr.c_str()+" "+toStr.c_str());
	#elif defined(__unix__)||defined(__APPLE__)
		command = std::string("cp -f ")+from.string()+" "+to.string();
	#endif
	std::system(command.c_str());
}
void copyDirectory(const boost::filesystem::path& from, const boost::filesystem::path& to){
	std::string command;
	#ifdef __WIN32__
		std::string fromStr = from.string(), toStr = to.string();
		std::replace(fromStr.begin(), fromStr.end(), '/', '\\');
		std::replace(toStr.begin(), toStr.end(), '/', '\\');
		std::system((std::string("mkdir ")+to.string()).c_str());
		command = std::string("xcopy /e /y /i ")+fromStr.c_str()+" "+toStr.c_str();
	#elif defined(__unix__)||defined(__APPLE__)
		std::system((std::string("mkdir ")+to.string()).c_str());
		command = std::string("cp -Rf ")+from.string()+"/* "+to.string();
	#endif
	std::system(command.c_str());
}

std::string praseCommand(std::string command){
	struct ReplaceStruct{ uni::string original, replace; };
	std::vector<ReplaceStruct> replaceMap = {
		#ifdef __WIN32__
			{"./dummy", "dummy.exe"},
			{"./keyGen", "keyGen.exe"},
			{"./nuke", "nuke.exe"},
			{"./crypter", "crypter.exe"},
			{"./injector", "injector.exe"},
			{"./redate", "redate.exe"},
			/*escape special characters: http://technet.microsoft.com/en-us/library/cc723564.aspx
				& | ( ) < > ^ are the reserved characters.
			*/
			{"^", "^^"},
			{"(", "^("},
			{")", "^)"},
			{"<", "^<"},
			{">", "^>"},
			{"/", "\\"}, //for file name.
			{"\\\\\\", "///"}, //for REGEX_MASTER
			{"\\\\", "//"}, //for REGEX_SLAVE
			{" ^> ", " > "},

		#elif defined(__unix__)||defined(__APPLE__)
			{"\\", "\\\\"},
			{"(", "\\("},
			{")", "\\)"},
			{"<", "\\<"},
			{">", "\\>"},
			{" \\> ", " > "},
		#endif
	};

	for(auto& i:replaceMap){
		for(size_t j=command.size(); j-->0; ){
			if(command.find(i.original, j)==j)
				command.replace(j, i.original.size(), i.replace);
		}
	}
	return command;
}
