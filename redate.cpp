#include "lib/lib.h"
#include <chrono>
#include <thread>

const char* helpHeader = "This program changes the timestamps of files to current time. This program is similar to touch in unix. The difference is that this program change the timestamps by temporary changing the system time. Please close all other applications before launching this program. Otherwise, files created/modified/accessed with other applications during the operation will have wrong timestamps.";
const char* helpFooter = "In most cases: In linux, mtime<=ctime, atime<=ctime, and mtime<=atime, where <= means is earlier than. In Windows mtime<=atime . Ensure that the timestamp that you provide satisfies the requirements above. This is necessary to prevent others from knowing that you have modified the timestamps of the files. Please notice that this program does NOT perform these checks automatically. ";

///TODO: copied from output.h::NukeArgument::postProcessParam(). Should find a way to merge them.
uni::string randomRename(const uni::string& oldName){
	boost::filesystem::path path, oldPath;
	do{
		path = oldName;
		oldPath = path;
		size_t fileNameLen = path.filename().string<std::basic_string<uni::char_t>>().size()+(genRand<size_t>()%8);
		/*generate a new filename.*/
		std::vector<uni::char_t> newFileName = SecureRandomDevice<uni::char_t>(fileNameLen+1).genVector();
		RandomFilter(R"(\w)").filter(newFileName);
		newFileName[fileNameLen] = '\0';
		/*change the file name in the path of the file.*/
		path.remove_filename();
		path /= newFileName;
	}while(boost::filesystem::exists(path));
	/*rename the file*/
	boost::filesystem::rename(oldPath.string<std::basic_string<uni::char_t>>(), path.string<std::basic_string<uni::char_t>>());
	return path.string<std::basic_string<uni::char_t>>();
}

#ifdef __WIN32__
#include <windows.h>
#elif defined(__unix__)||defined(__APPLE__)
#include <sys/time.h>
#endif

struct Time;
extern Time getCurrentTime();
#include <ctime>
struct Time{
	std::tm tm;
	int msec;
	int usec;
	int nsec;
	Time(){
		*this = getCurrentTime();
		tm.tm_isdst = -1; //No daylight saving time info is available. Let's put the burden on std::mktime()
	}
	#ifdef __WIN32__
		Time(SYSTEMTIME other){
			tm.tm_year = other.wYear-1900;
			tm.tm_mon = other.wMonth-1;
			tm.tm_mday = other.wDay;
			tm.tm_hour = other.wHour;
			tm.tm_min = other.wMinute;
			tm.tm_sec = other.wSecond;
			msec = other.wMilliseconds;
		}
		operator SYSTEMTIME(){
			SYSTEMTIME st;
			st.wYear = tm.tm_year+1900;
			st.wMonth = tm.tm_mon+1;
			st.wDay = tm.tm_mday;
			st.wHour = tm.tm_hour;
			st.wMinute = tm.tm_min;
			st.wSecond = tm.tm_sec;
			st.wMilliseconds = msec;
			return st;
		}
	#elif defined(__unix__)||defined(__APPLE__)
		Time(timespec other){
			tm = *std::localtime(&other.tv_sec); ///or std::gmtime()?
			msec = other.tv_nsec/1000/1000;
			other.tv_nsec -= msec*1000*1000;
			usec = other.tv_nsec/1000;
			other.tv_nsec -= usec*1000;
			nsec = other.tv_nsec;
		}
		operator timespec(){ return {std::mktime(&tm), msec*1000*1000+usec*1000+nsec}; }
		//operator timespec&(){ return *this; } //NOT working. causes infinitely recursion. :'(
	#endif
};
Time cTime;
Time mTime = cTime; //so that the  cTime, mTime and
Time aTime = cTime; //aTime will be the same.

#ifdef __WIN32__
#include <WinBase.h>
	//read this: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724936%28v=vs.85%29.aspx
	Time getCurrentTime(){
		SYSTEMTIME st;
		GetLocalTime(&st); //This function returns void.
		Time time = st;
		/*It seems that Windows does not provide millisecond precision of time. Let's generate one.*/
		time.usec = genRand<int>()%1000;
		time.nsec = genRand<int>()%1000;
		return time;
	}
	void setTime(Time time){
		SYSTEMTIME st = time;
		if(!SetLocalTime(&st))
			throw std::runtime_error("error on setting system time. Do you have permission to do that?");
		//std::this_thread::sleep_for(std::chrono::nanoseconds(time.usec*1000+time.nsec));
	}
#elif defined(__unix__)||defined(__APPLE__)
	//read this: www.linuxquestions.org/questions/programming-9/c-code-to-change-date-time-on-linux-707384/
	Time getCurrentTime(){
		timespec ts;
		if(clock_gettime(CLOCK_REALTIME, &ts))
			throw std::runtime_error("error on getting system time.");
		return ts;
	}
	void setTime(Time time){ //pass be reference won't work :'(
		timespec ts = time;
		if(clock_settime(CLOCK_REALTIME, &ts))
			throw std::runtime_error("error on setting system time. Are you root?");
	}
#endif

void parseTime(Time& time, const uni::string paramStr){
	uni::match_results result;
	/*Format:[YY]MMDDhhmm[.ss[.mmm[uuu]]]
	regex: (?[amc]{0,3}).*=(\d\d\d\d)?(\d\d){4}([.](\d\d)([.](\d\d\d){1,3})?)?$
	The regex below is an expanded regex for converting the subexpressions*/
	std::regex_match(paramStr, result,
		uni::regex(uni::string(R"(.*=(\d\d\d\d)?(\d\d)(\d\d)(\d\d)(\d\d)[.]?(\d\d)?[.]?(\d\d\d)?(\d\d\d)?(\d\d\d)?$)")));
	assert(!result.empty());
	std::function<void(const uni::string&,int&,int,int)> parse =
		[](const uni::string& str, int& field, int min, int max){
			if(str.empty()) return;
			field = strToSizeT(str);
			if(field<min||field>max)
				throw std::out_of_range("invalid timestamp.");
		};
	/*[YY]MMDDhhmm[[.ss].mmmuuu]*/
	/*The range of data can be found here: http://en.cppreference.com/w/cpp/chrono/c/tm */
	parse(result[1].str(),	time.tm.tm_year,	1900,	std::numeric_limits<int>::max());
	if(result[1].length()) time.tm.tm_year -= 1900;
	parse(result[2].str(),	time.tm.tm_mon,		1,		12);
	time.tm.tm_mon--;
	parse(result[3].str(),	time.tm.tm_mday,	1,		31);
	parse(result[4].str(),	time.tm.tm_hour,	0,		23);
	parse(result[5].str(),	time.tm.tm_min,		0,		59);
	parse(result[6].str(),	time.tm.tm_sec,		0,		59); //even tho tm allows the value 60 as a leap second, it's EXTREMELY rare. Hence, we are setting the upper limit to 59.
	/*These fields have nothing to do with std::tm*/
	parse(result[7].str(),	time.msec,			0,		999);
	parse(result[8].str(),	time.usec,			0,		999);
	parse(result[9].str(),	time.nsec,			0,		999);
}

void touchATime(uni::string paramStr){
	setTime(aTime);
	std::fstream touch(paramStr, std::ios_base::in);
	if(!touch) throw std::runtime_error("Error on changing the atime of the file.");
	touch.get(); //change the atime by reading something.
	touch.close();
}

extern void touchCTime(uni::string paramStr);
void touchMTime(uni::string paramStr){
	setTime(mTime);
	if(boost::filesystem::is_directory(paramStr)){
		/*uni::string dummyFileName = paramStr+"dummy file";
		std::fstream touch(dummyFileName, std::ios_base::out|std::ios_base::app);
		touch.close();
		argumentProcesser<NukeArgument>(dummyFileName);*/
		std::cerr<<"Sorry, unimplemented changing mtime for directories. The modification on atime and ctime will still work."<<std::endl; ///TODO: The implementation above is not working well. Let's reimplement it later.
		return;
	}else if(boost::filesystem::is_symlink(paramStr)){
		std::cerr<<"Sorry, unimplemented changing mtime for symbolic links. The modification on atime and ctime will still work."<<std::endl; ///TODO: unimplemented
		return;
	}
	std::fstream touch(paramStr, std::ios_base::out|std::ios_base::binary|std::ios_base::app);
	if(!touch) throw std::runtime_error("Error on changing the mtime of the file.");
	touch.put(genRand<char>()); //change the mtime by writing something, and then discard it by resizing the file.
	touch.close();
	boost::filesystem::resize_file(paramStr, boost::filesystem::file_size(paramStr)-1);
}
Time oldTime;
void touchCTime(uni::string paramStr){
	setTime(cTime);
	#ifdef __WIN32__
		//It seems making a copy of the file is required: support.microsoft.com/kb/299648
		///TODO: NOT working. It's strange. By copying the file to destination other than the original file name, the ctime is change. However, if the file is renamed to TMP and TMP is copied to the original filename, the ctime remains unchanged. Oh, why?
		uni::string newName = randomRename(paramStr);
		copyFile(newName, paramStr);
		argumentProcesser<NukeArgument>(newName);
	#elif defined(__unix__)||defined(__APPLE__)
		/*rename the file and change back its name to modify its ctime.*/
		uni::string newName = randomRename(paramStr);
		boost::filesystem::rename(newName, paramStr);
	#endif
}

class RedateArgument: public FileNameArgument{
	protected:
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> param) override{
			/*store the original time*/
			oldTime = getCurrentTime();
			try{
				//param->value contains the filename. Now, change the time with param->value.
				#ifdef __WIN32__
					//touchCTime(param->value);
					touchMTime(param->value);
					touchATime(param->value);
				#elif defined(__unix__)||defined(__APPLE__)
					touchATime(param->value);
					touchMTime(param->value);
					touchCTime(param->value);
				#endif
			}catch(std::exception& e){
				std::cerr<<e.what()<<std::endl;
			}
			/*change the time back*/
			setTime(oldTime);
			return END_PROCESSING_BLOCK;
		}
	public:
		RedateArgument(const char* name):FileNameArgument(name){}
};

std::vector<std::shared_ptr<Argument>> arguments = {std::make_shared<RedateArgument>("file")};
std::vector<std::shared_ptr<Argument>> specialArguments = {std::make_shared<RecirectArgument>("redir"),
										std::make_shared<OptionArgument>("opt")};
uint32_t enabledOptions = SHOW_HELP;

void init(){
	///TODO: NOT working on Windows.
	appendOption( uni::string(R"(^[mc]*a[mc]*=(\d\d\d\d)?(\d\d){4}([.](\d\d)([.](\d\d\d){1,3})?)?$)"), [](const uni::string paramStr){ parseTime(aTime, paramStr);
	}, "Change the timestamp of access time of the file to specified time to instead of using current time. Format: [YYYY]MMDDhhmm[.ss[.mmm[uuu[nnn]]]] If the optionals are not provided, it will be substituted by the current system time.");
	appendOption( uni::string(R"(^[ac]*m[ac]*=(\d\d\d\d)?(\d\d){4}([.](\d\d)([.](\d\d\d){1,3})?)?$)"), [](const uni::string paramStr){ parseTime(mTime, paramStr);
	}, "Change the timestamp of modify time of the file to specified time to instead of using current time. Format: ditto");
	#ifdef __WIN32__
	appendOption( uni::string(R"(^[mc]*c[mc]*=(\d\d\d\d)?(\d\d){4}([.](\d\d)([.](\d\d\d){1,3})?)?$)"), [](const uni::string paramStr){ std::cerr<<"Sorry, this option is unimplemented for Windows."<<std::endl;
	}, "Unimplemetned for Windows.");
	#elif defined(__unix__)||defined(__APPLE__)
	appendOption( uni::string(R"(^[am]*c[am]*=(\d\d\d\d)?(\d\d){4}([.](\d\d)([.](\d\d\d){1,3})?)?$)"), [](const uni::string paramStr){ parseTime(cTime, paramStr);
	}, "Change the timestamp of changed time of the file of the specified time instead of using current time(Unix only). Format: ditto");
	#endif
}
void preProcess(){}
