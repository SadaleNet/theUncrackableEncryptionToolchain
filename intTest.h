#include <cstdlib>
#include <cstdio>

std::string exec(std::string command, bool expectedZeroRet=true){
	command = praseCommand(command);
std::cerr<<"executing: "<<command<<std::endl;
	if((std::system(command.c_str())==0)^expectedZeroRet){
		std::cerr<<std::endl<<std::endl;
		std::cerr.flush();
		throw std::runtime_error(std::string("exec(): ")+(expectedZeroRet?"Program teminated unexpectedly.":"Program did not detected the syntax error. ")+" The command was: \n"+command);
	}
	return command;
}

const size_t CHUNK_SIZE = 1024*1024;
std::vector<char> getContent(std::string file){
	#ifdef __WIN32__
		std::replace(file.begin(), file.end(), '/', '\\');
	#endif
	std::vector<char> data;
	std::ifstream stream(file, std::ios_base::in|std::ios_base::binary);
	assert(stream);
	while(!stream.eof()){
		size_t oldSize = data.size();
		data.resize(data.size()+CHUNK_SIZE);
		stream.read(&data[oldSize], CHUNK_SIZE);
		data.resize(oldSize+stream.gcount());
	}
	stream.close();
	return data;
}

bool contentSame(std::string a, std::string b){
	#ifdef __WIN32__
		std::replace(a.begin(), a.end(), '/', '\\');
		std::replace(b.begin(), b.end(), '/', '\\');
	#endif
	if(boost::filesystem::file_size(a)!=boost::filesystem::file_size(b))
		return false;

	std::ifstream aStrm(a, std::ios_base::in|std::ios_base::binary);
	assert(aStrm);
	std::vector<char> aBuf;
	while(!aStrm.eof()){
		size_t oldSize = aBuf.size();
		aBuf.resize(aBuf.size()+CHUNK_SIZE);
		aStrm.read(&aBuf[oldSize], CHUNK_SIZE);
		aBuf.resize(oldSize+aStrm.gcount());
		aStrm.peek();
	}

	std::ifstream bStrm(a, std::ios_base::in|std::ios_base::binary);
	assert(bStrm);
	std::vector<char> bBuf;
	while(!bStrm.eof()){
		size_t oldSize = bBuf.size();
		bBuf.resize(bBuf.size()+CHUNK_SIZE);
		bStrm.read(&bBuf[oldSize], CHUNK_SIZE);
		bBuf.resize(oldSize+bStrm.gcount());
		bStrm.peek();
	}

	if(aBuf!=bBuf){
		aStrm.close();
		return false;
	}
/**TODO: SCREW YOU, MINGW! Y U NO ALLOW ME TO OPEN TWO FSTREAMS SIMULTANEOUDLY, AND DOES NOT HINT ANY DEBUG MESSAGE? THE FOLLOWING CODE IS WORKING ELSEWHERE, BUT NOT ON MINGW!*/
/*	std::ifstream aStrm(a, std::ios_base::in|std::ios_base::binary);
	std::ifstream bStrm(b, std::ios_base::in|std::ios_base::binary);
	assert(aStrm&&bStrm);
	while(!aStrm.eof()&&!bStrm.eof()){
		char aBuf[CHUNK_SIZE], bBuf[CHUNK_SIZE];
		aStrm.read(aBuf, CHUNK_SIZE);
		bStrm.read(bBuf, CHUNK_SIZE);
		if(!std::equal(&aBuf[0], &aBuf[CHUNK_SIZE], &bBuf[0])){
			aStrm.close();
			bStrm.close();
			return false;
		}
	}
	aStrm.close();
	bStrm.close();*/
	return true;
}

bool dummyTest(){
	std::cerr<<"echo test\t";

	exec("echo test | ./dummy -f (i) ./sandbox/dummy/#");
	char str[] = "test";
	if(!std::equal(&str[0], &str[sizeof(str)-1], &getContent("./sandbox/dummy/#")[0]))
		return false;

	std::cerr<<"pipe test\t";
	exec("./keyGen 5M -f ./sandbox/dummy/0"); //write some random data to ./sandbox/dummy/0

	exec("./dummy -f ./sandbox/dummy/0 ./sandbox/dummy/1");
	exec("./dummy ./sandbox/dummy/0 [o] | ./dummy -f [ir) ./sandbox/dummy/2 > ./sandbox/dummy/3");
	exec("./dummy ./sandbox/dummy/0 [o] | ./dummy [ir] [o] | ./dummy -f [i] ./sandbox/dummy/4 [ir) ./sandbox/dummy/5 > ./sandbox/dummy/6");
	exec("./dummy ./sandbox/dummy/0 [o] | ./dummy [ir] [o] | ./dummy -f [i] ./sandbox/dummy/7 [ir) ./sandbox/dummy/8 > ./sandbox/dummy/9");
	for(char i='1'; i<='9'; i++){
		std::cerr<<i<<'\t';
		if(!contentSame("./sandbox/dummy/0", std::string("./sandbox/dummy/")+i))
			return false;
	}

	std::cerr<<"advance tests\t";
	exec("./dummy -f ./sandbox/dummy/0 (o) | ./dummy -f (i) ./sandbox/dummy/A");
	exec("./dummy -f ./sandbox/dummy/0 (o) > ./sandbox/dummy/B");
	exec("./dummy -f ./sandbox/dummy/0 [o] | ./dummy -f [i] ./sandbox/dummy/C");
	//exec("./dummy -f ./sandbox/dummy/0 {o} | ./dummy -f {i} ./sandbox/dummy/D");

	exec("./dummy -f ./sandbox/dummy/0 (o) | ./dummy -f (ir) ./sandbox/dummy/E > ./sandbox/dummy/F");
	exec("./dummy -f ./sandbox/dummy/0 [o] | ./dummy -f [ir] ./sandbox/dummy/G | ./dummy -f [i] ./sandbox/dummy/H");
	//exec("./dummy -f ./sandbox/dummy/0 {o} | ./dummy -f {ir} ./sandbox/dummy/I | ./dummy -f {i} ./sandbox/dummy/J");

	exec("./dummy -f ./sandbox/dummy/0 (./sandbox/dummy/K) > ./sandbox/dummy/L");
	exec("./dummy -f ./sandbox/dummy/0 [./sandbox/dummy/M] | ./dummy -f [i] ./sandbox/dummy/N");
	exec("./dummy -f ./sandbox/dummy/0 {./sandbox/dummy/O} | ./dummy -f {i} ./sandbox/dummy/P");

	exec("./dummy -f (./sandbox/dummy/0) ./sandbox/dummy/Q > ./sandbox/dummy/R");
	exec("./dummy -f [./sandbox/dummy/0] ./sandbox/dummy/S | ./dummy -f [i] ./sandbox/dummy/T");
	exec("./dummy -f {./sandbox/dummy/0} ./sandbox/dummy/U | ./dummy -f {i} ./sandbox/dummy/V");

	exec("./dummy -f [./sandbox/dummy/0] [./sandbox/dummy/W] | ./dummy -f [i] ./sandbox/dummy/X [i] ./sandbox/dummy/Y");
	exec("./dummy -f {./sandbox/dummy/0} ./sandbox/dummy/Z | ./dummy {r] | ./dummy [r) | ./dummy (r] | ./dummy -f [i] ./sandbox/dummy/@");

	exec("./dummy -f {./sandbox/dummy/0} {./sandbox/dummy/D} | ./dummy {r] {r] | ./dummy -f [ir] [./sandbox/dummy/I] | ./dummy -f [i] ./sandbox/dummy/J [r) > ./sandbox/dummy/[");

	///TODO: test the param {i} amd {ir)]} for OutputArgument.

	for(char i='@'; i<='['; i++){
		std::cerr<<i<<'\t';
		if(!contentSame("./sandbox/dummy/0", std::string("./sandbox/dummy/")+i))
			return false;
	}

	std::cerr<<"Invalid tests";
	/*redir argument not at the beginning of the block.*/
	exec("./dummy -f ./sandbox/dummy/0 (r) ./sandbox/dummy/@", false);
	/*redir information that is not available*/
	exec("./dummy -f (r}", false);
	exec("./dummy -f [r}", false);
	exec("./dummy -f (ir} ./sandbox/dummy/@", false);
	exec("./dummy -f [ir} ./sandbox/dummy/@", false);
	/*use o in InputArgument*/
	exec("./dummy -f (o) ./sandbox/dummy/@", false);
	exec("./dummy -f [o] ./sandbox/dummy/@", false);
	exec("./dummy -f {o} ./sandbox/dummy/@", false);
	/*use i in OutputArgument(except {i})*/
	exec("./dummy -f ./sandbox/dummy/0 (i)", false);
	exec("./dummy -f ./sandbox/dummy/0 [i]", false);
	exec("./dummy -f ./sandbox/dummy/0 (ir)", false);
	exec("./dummy -f ./sandbox/dummy/0 [ir]", false);
	exec("./dummy -f ./sandbox/dummy/0 [ir}", false);
	std::cerr<<"*******************************************************************************"<<std::endl;
	std::cerr<<"*Don't worry. It's normal to see the error messages as these are invalid tests*"<<std::endl;
	std::cerr<<"*******************************************************************************"<<std::endl;

	return true;
}

bool regexTest(){
	std::cerr<<"regex warm up"<<std::endl;

	copyDirectory("./data", "./sandbox/regex");
	exec(R"(./dummy -f ./sandbox/regex///CIMG(\d*).JPG ./sandbox/regex2//The_\1_foto.JPG)");
	if(!contentSame("./sandbox/regex/CIMG2576.JPG", "./sandbox/regex2/The_2576_foto.JPG")||
		!contentSame("./sandbox/regex/CIMG2627.JPG", "./sandbox/regex2/The_2627_foto.JPG")||
		!contentSame("./sandbox/regex/CIMG2640.JPG", "./sandbox/regex2/The_2640_foto.JPG")||
		!contentSame("./sandbox/regex/CIMG2649.JPG", "./sandbox/regex2/The_2649_foto.JPG")||
		!contentSame("./sandbox/regex/CIMG2897.JPG", "./sandbox/regex2/The_2897_foto.JPG"))
			return false;

	std::cerr<<"filename regex pipe test"<<std::endl;
	exec(R"(./dummy -f ./sandbox/regex///CIMG(\d*).JPG <{./sandbox/regex2//The_\1_foto.JPG}> | ./keyGen -f 1M <{i}>)");
	if(boost::filesystem::file_size("./sandbox/regex2/The_2576_foto.JPG")!=1024*1024||
		boost::filesystem::file_size("./sandbox/regex2/The_2627_foto.JPG")!=1024*1024||
		boost::filesystem::file_size("./sandbox/regex2/The_2640_foto.JPG")!=1024*1024||
		boost::filesystem::file_size("./sandbox/regex2/The_2649_foto.JPG")!=1024*1024||
		boost::filesystem::file_size("./sandbox/regex2/The_2897_foto.JPG")!=1024*1024)
			return false;

	exec(R"(./dummy -f <[./sandbox/regex///CIMG(\d*).JPG]> ./sandbox/regex2//The_\1_foto.JPG | ./crypter -f <[i]> ./sandbox/regex2//The_\1_foto.JPG ./sandbox/regex//out\1)");

exec(R"(./crypter -fg ./sandbox/regex//key\1 ./sandbox/regex///CIMG(\d*).JPG ./sandbox/regex//out\1)");
exec(R"(./crypter -fnd ./sandbox/regex//key\1 ./sandbox/regex///out(\d*) ./sandbox/regex//DIMG\1.JPG)");

	if(!contentSame("./sandbox/regex/CIMG2576.JPG","./sandbox/regex/DIMG2576.JPG")||
		!contentSame("./sandbox/regex/CIMG2627.JPG","./sandbox/regex/DIMG2627.JPG")||
		!contentSame("./sandbox/regex/CIMG2640.JPG","./sandbox/regex/DIMG2640.JPG")||
		!contentSame("./sandbox/regex/CIMG2649.JPG","./sandbox/regex/DIMG2649.JPG")||
		!contentSame("./sandbox/regex/CIMG2897.JPG","./sandbox/regex/DIMG2897.JPG"))
			return false;

/** TEST CASES THAT REQUIRES CHECKING MANUALLY.*/
/*Short encryption*/
exec(R"(./dummy -f <[./sandbox/regex///test(\d*)]> ./sandbox/regex/tmp | ./crypter -f <[i]> ./sandbox/regex//test\1 ./sandbox/regex//outShort\1)");
exec(R"(./dummy -f <[./sandbox/regex///test(\d*)]> ./sandbox/regex/tmp | ./crypter -fd <[i]> ./sandbox/regex//outShort\1 ./sandbox/regex//testOut\1)");
/*long encryption*/
exec(R"(./dummy -f <[./sandbox/regex///CIMG(\d*).JPG]> ./sandbox/regex/tmp | ./crypter -fg ./sandbox/regex//keyLong\1 <[i]> ./sandbox/regex//outLong\1)");
exec(R"(./crypter -fd ./sandbox/regex//keyLong\1 ./sandbox/regex///outLong(\d*) ./sandbox/regex//DIMG\1.JPG)");
/*long encryption with filename pipe*/
exec(R"(./dummy -f <{./sandbox/regex///CIMG(\d*).JPG}> ./sandbox/regex/tmp | ./crypter -fng ./sandbox/regex//keyFileName\1 <{i}> ./sandbox/regex//outFileName\1)");
exec(R"(./crypter -fnd ./sandbox/regex//keyFileName\1 ./sandbox/regex///outFileName(\d*) ./sandbox/regex//EIMG\1.JPG)");

/*long encryption, filename pipe, with a non-generative key.*/
exec(R"(./dummy -f <{./sandbox/regex///DIMG(\d*).JPG}> ./sandbox/regex/tmp | ./crypter -f <{i}> ./sandbox/regex//EIMG\1.JPG ./sandbox/regex//outFileNameNonGen\1)");
exec(R"(./crypter -fnd ./sandbox/regex///DIMG(\d*).JPG ./sandbox/regex//outFileNameNonGen\1 ./sandbox/regex//FIMG\1.JPG)");
	std::cerr<<std::endl<<
R"(*****************************************
*****Please check the files manually*****
*****	  in ./sandbox/regex/	  *****
*****The auto check is unimplemented*****
*****************************************)"<<std::endl;

	std::cerr<<"invalid test"<<std::endl;
	exec(R"(./dummy -f <[./sandbox/regex///CIMG(\d*).JPG]> ./sandbox/regex2//The_\1_foto.JPG | ./crypter -fg <[i]> ./sandbox/regex2//The_\1_foto.JPG ./sandbox/regex//out\1)", false);
	std::cerr<<"data regex pipe test"<<std::endl;

///TODO: untested: multiple regex pipe in the same block.

	return true;
}

bool keyGenTest(){
	std::cerr<<"generating keys. It will take a while...";
	exec("./keyGen 50 -f ./sandbox/keyGen/50");
	exec("./keyGen 1234567 -f ./sandbox/keyGen/1234567");
	exec("./keyGen 1k -f ./sandbox/keyGen/1KiB");
	exec("./keyGen 1kb -f ./sandbox/keyGen/1kB");
	exec("./keyGen 50000k -f ./sandbox/keyGen/50000KiB");
	exec("./keyGen 50000kb -f ./sandbox/keyGen/50000kB");
	exec("./keyGen 5m -f ./sandbox/keyGen/5MiB");
	exec("./keyGen 5mb -f ./sandbox/keyGen/5MB");
	/*Too slow, not testing.*/
	//exec(R"(./keyGen 1g -f ./sandbox/keyGen/1GiB)");
	//exec(R"(./keyGen 1gb -f ./sandbox/keyGen/1GB)");
	std::map<std::string, size_t> nameSizeMap = {
		{ "./sandbox/keyGen/50",		50 },
		{ "./sandbox/keyGen/1234567",	1234567 },
		{ "./sandbox/keyGen/1KiB",		1024 },
		{ "./sandbox/keyGen/1kB",		1000 },
		{ "./sandbox/keyGen/50000KiB",	50000*1024 },
		{ "./sandbox/keyGen/50000kB",	50000*1000 },
		{ "./sandbox/keyGen/5MiB",		5*1024*1024 },
		{ "./sandbox/keyGen/5MB",		5*1000*1000 },
	};
	for(auto& i:nameSizeMap){
		std::cerr<<i.first.substr(i.first.rfind("/")+1)<<'\t';
		if(boost::filesystem::file_size(i.first)!=i.second)
			return false;
	}
	return true;
}

bool nukeTest(){
	/*DECLARATION: Nothing makes me hate Japan. Japan is alright. I am just being a little bit bored.
	The content inside this function is not meant to offense. It is meant to be a joke.*/
	std::cerr<<"preparing files..."<<std::endl;
	exec("./keyGen 50K ./sandbox/nuke/Hiroshima ");
	exec("./keyGen 8M ./sandbox/nuke/Nagasaki ");
	/*US: I am going to drop two nukes, Japan. You better to surrender--NOW!*/
	exec("./nuke -f -n=20 ./sandbox/nuke/Hiroshima"); //nuke 20 times
	exec("./nuke -f ./sandbox/nuke/Nagasaki"); //nuke default(i.e. 3) times.
	/*See whether Japan have surrendered.*/
	if(boost::filesystem::exists("./sandbox/nuke/Hiroshima")
		|| boost::filesystem::exists("./sandbox/nuke/Nagasaki"))
			return false;
	return true;
}

bool crypterTest(){
	exec("./keyGen -f 8MB ./sandbox/crypter/key");
	exec("./keyGen -f 2MB ./sandbox/crypter/data");
	exec("./crypter -f ./sandbox/crypter/key ./sandbox/crypter/data ./sandbox/crypter/output0");
	exec("./crypter -fd ./sandbox/crypter/key ./sandbox/crypter/output0 ./sandbox/crypter/data0");
	exec("./crypter -af ./sandbox/crypter/key ./sandbox/crypter/data ./sandbox/crypter/output1");
	exec("./crypter -afd ./sandbox/crypter/key ./sandbox/crypter/output1 ./sandbox/crypter/data1");
	exec("echo 'password' | ./crypter -af (i) ./sandbox/crypter/data ./sandbox/crypter/output2");
	exec("echo 'password' | ./crypter -afd (i) ./sandbox/crypter/output2 ./sandbox/crypter/data2");
	exec("./crypter -fg ./sandbox/crypter/generatedKey ./sandbox/crypter/data ./sandbox/crypter/output3");
	exec("./crypter -fd ./sandbox/crypter/generatedKey ./sandbox/crypter/output3 ./sandbox/crypter/data3");
	exec("./crypter -afg ./sandbox/crypter/generatedAesKey ./sandbox/crypter/data ./sandbox/crypter/output4");
	exec("./crypter -afd ./sandbox/crypter/generatedAesKey ./sandbox/crypter/output4 ./sandbox/crypter/data4");
	exec("./crypter -o=12345 -f ./sandbox/crypter/key ./sandbox/crypter/data ./sandbox/crypter/output5");
	exec("./crypter -o=12345 -fd ./sandbox/crypter/key ./sandbox/crypter/output5 ./sandbox/crypter/data5");
	exec("./crypter -o=12345 -fg ./sandbox/crypter/generatedKey2 ./sandbox/crypter/data ./sandbox/crypter/output6");
	exec("./crypter -o=12345 -fd ./sandbox/crypter/generatedKey2 ./sandbox/crypter/output6 ./sandbox/crypter/data6");
	exec("./crypter -o=12345 -afg ./sandbox/crypter/generatedAesKey2 ./sandbox/crypter/data ./sandbox/crypter/output7");
	exec("./crypter -o=12345 -afd ./sandbox/crypter/generatedAesKey2 ./sandbox/crypter/output7 ./sandbox/crypter/data7");
	for(char i='0'; i<='7';i++){
		std::cerr<<i<<'\t';
		contentSame("./sandbox/crypter/data", std::string("./sandbox/crypter/data")+i);
	}
	std::cerr<<std::endl<<"crypt and nuke test...";

	copyFile("./sandbox/crypter/data", "./sandbox/crypter/dataToBeNuked");
	exec("./crypter -fng ./sandbox/crypter/key ./sandbox/crypter/dataToBeNuked ./sandbox/crypter/outputToBeNuked");
	if(boost::filesystem::exists("./sandbox/crypter/dataToBeNuked"))
		return false;
	exec("./crypter -fnd ./sandbox/crypter/key ./sandbox/crypter/outputToBeNuked ./sandbox/crypter/data8");
	if(!contentSame("./sandbox/crypter/data", std::string("./sandbox/crypter/data8")))
		return false;
	return true;
}

bool injectorTest(){
	/*generate and copy files.*/
	exec("./keyGen -f 3MB ./sandbox/injector/srcOriginal");
	exec("./keyGen -f 2MB ./sandbox/injector/destOriginal");

	copyFile("./sandbox/injector/srcOriginal", "./sandbox/injector/src");
	copyFile("./sandbox/injector/destOriginal", "./sandbox/injector/dest");

	std::cerr<<"basic test";
	exec("./injector -f ./sandbox/injector/src ./sandbox/injector/dest");
	if(boost::filesystem::file_size("./sandbox/injector/dest") != boost::filesystem::file_size("./sandbox/injector/destOriginal")+boost::filesystem::file_size("./sandbox/injector/src")+sizeof(uint64_t))
		return false;
	exec("./injector -fe ./sandbox/injector/dest ./sandbox/injector/src");
	std::cerr<<".";
	if(!contentSame("./sandbox/injector/src", "./sandbox/injector/srcOriginal"))
			return false;


	std::cerr<<"multiple injection+discard mode test";
	copyFile("./sandbox/injector/destOriginal", "./sandbox/injector/dest");
	exec("./injector -f ./sandbox/injector/src ./sandbox/injector/dest");
	exec("./injector -f ./sandbox/injector/src ./sandbox/injector/dest");
	exec("./injector -f ./sandbox/injector/src ./sandbox/injector/dest");
	exec("./injector -f ./sandbox/injector/src ./sandbox/injector/dest");
	exec("./injector -fed ./sandbox/injector/dest ./sandbox/injector/src0");
	exec("./injector -fed ./sandbox/injector/dest ./sandbox/injector/src1");
	exec("./injector -fed ./sandbox/injector/dest ./sandbox/injector/src2");
	exec("./injector -fed ./sandbox/injector/dest ./sandbox/injector/src3");
	if(!contentSame("./sandbox/injector/src", "./sandbox/injector/srcOriginal")
		|| !contentSame("./sandbox/injector/dest", "./sandbox/injector/destOriginal"))
			return false;
	for(char i='0'; i<='3'; i++){
		std::cerr<<".";
		if(!contentSame("./sandbox/injector/srcOriginal", std::string("./sandbox/injector/src")+i))
			return false;
	}

	std::cerr<<"inject and nuke test";
	exec("./injector -fn ./sandbox/injector/src ./sandbox/injector/dest");
	exec("./injector -fen ./sandbox/injector/dest ./sandbox/injector/src");
	if(!contentSame("./sandbox/injector/src", "./sandbox/injector/srcOriginal"))
		return false;

	return true;
}

bool redateTest(){
	exec("./keyGen -f 1MB ./sandbox/redate/amc");
	exec("./keyGen -f 2MB ./sandbox/redate/am");
	exec("./keyGen -f 3MB ./sandbox/redate/ac");
	exec("./keyGen -f 4MB ./sandbox/redate/mc");
	exec("./keyGen -f 5MB ./sandbox/redate/a");
	exec("./keyGen -f 6MB ./sandbox/redate/m");
	exec("./keyGen -f 7MB ./sandbox/redate/c");

	#ifdef __WIN32__
	std::string program = "./redate ";
	#elif defined(__unix__)||defined(__APPLE__)
	std::string program = "sudo ./redate ";
	#endif

	exec(program+"-amc=201212211111.11.123456789 ./sandbox/redate/amc");
	exec(program+"-am=201212211111.11.123456789 ./sandbox/redate/am");
	exec(program+"-ac=201212211111.11.123456789 ./sandbox/redate/ac");
	exec(program+"-mc=201212211111.11.123456789 ./sandbox/redate/mc");
	exec(program+"-a=201212211111.11.123456789 ./sandbox/redate/a");
	exec(program+"-m=201212211111.11.123456789 ./sandbox/redate/m");
	exec(program+"-c=201012211111.11.123456789 ./sandbox/redate/c");

	std::cerr<<std::endl<<
R"(*****************************************
*****Please check the time manually *****
*****	  in ./sandbox/redate	  *****
*****The xtime of the file should be*****
***** 21/12/2012 11:11:11.123456789 *****
***** , where  x  is the  file name *****
*****************************************)"<<std::endl;

	return true;
}

bool integrationTests(){
	#ifdef __WIN32__
	system("rmdir /S /Q sandbox");
	#elif defined(__unix__)||defined(__APPLE__)
	system("rm -Rf ./sandbox");
	#endif
	assertWithMsg(dummyTest);
	assertWithMsg(regexTest);
	assertWithMsg(keyGenTest);
	assertWithMsg(nukeTest);
	assertWithMsg(crypterTest);
	assertWithMsg(injectorTest);
	assertWithMsg(redateTest);

	return true;
}
