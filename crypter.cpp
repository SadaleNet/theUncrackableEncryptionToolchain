#include <botan/botan.h>

#include "lib/lib.h"

const char* helpHeader = "This program encrypts file with a key. The file is first encrypted.";
const char* helpFooter = "Pipe note: only the data read in src is redirected to pipe";

bool keyGenMode = false, decryptMode = false, aesMode = false;
size_t readOffset = 0;

/*
template<class T>
void printCheckSum(std::vector<T> a){
	uint64_t sum = 0;
	for(auto& i:a)
		sum += static_cast<unsigned int>(i);
	std::cerr<<"checksum="<<sum<<" size="<<a.size()<<std::endl;
}

template<class T>
void printCheckSum(Botan::SecureVector<T> a){
	uint64_t sum = 0;
	for(auto& i:a)
		sum += static_cast<unsigned int>(i);
	std::cerr<<"checksum="<<sum<<" size="<<a.size()<<std::endl;
}
*/

bool decryptionCompleted = false;
class KeyArgument:public SecureStreamInputArgument{
	private:
		typedef SecureStreamInputArgument super;
	protected:
		/*The file is nuked if nukeMode==true*/
		virtual bool nukable() override{ return false; }
	public:
		KeyArgument(const char* name):SecureStreamInputArgument(name){}
		virtual std::shared_ptr<Param_t> loadParam(const uni::string& str, std::shared_ptr<Param_t> p=nullptr) override{
			p = super::loadParam(str, p);
			if(keyGenMode){
				std::shared_ptr<FileParam_t> param = std::static_pointer_cast<FileParam_t>(p);
				param->pipeInfo |= GENERATE;
			}
			return p;
		}
		virtual std::vector<char> genData() override{
			assert(keyGenMode);
			return SecureRandomDevice<>(CHUNK_SIZE).genVector();
		}
		virtual std::vector<char> genFooter() override{
			assert(keyGenMode);
			/*At most generate 30% extra size.*/
			uint64_t extraSize = dataSizeWritten*SecureRandomDevice<uint16_t>(1).gen()[0]/65535/2;
			if(extraSize==0) return std::vector<char>();
			return SecureRandomDevice<>(SecureRandomDevice<uint32_t>(1).gen()[0]%extraSize).genVector();
		}
		virtual void validateParam(std::shared_ptr<const Param_t> p) override{
			super::validateParam(p);
			std::shared_ptr<const FileParam_t> param = std::static_pointer_cast<const FileParam_t>(p);
			if(keyGenMode&&decryptMode)
				throw std::invalid_argument("key generation mode cannot be used with decryption mode.");
		}
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override{
			decryptionCompleted = false;
			return super::preProcessParam(p);
		}
		virtual PARAM_PROC_STATUS processParam(std::shared_ptr<const Param_t> p) override{
			PARAM_PROC_STATUS ret = super::processParam(p);
			if(decryptionCompleted)
				return END_PROCESSING_BLOCK;
			return aesMode?DECIDED_BY_OTHER_PARAM:ret;
		}
};

uint64_t outputSize;
class EncryptArgument:public OutputArgument{
	private:
		typedef OutputArgument super;
		Botan::Pipe cryptoPipe;
		const static size_t SALT_SIZE = 256/8;
		const static size_t IV_SIZE = 16;
		const static size_t AES_HEADER_SIZE = 16;
	protected:
		virtual bool predictableDataLen() override{ return true; }
		virtual uint64_t getDataLen() override{
			return outputSize;
		}
	public:
		EncryptArgument(const char* name):OutputArgument(name){}
		virtual std::vector<char> genHeader() override{
			/*discard a amount of buffer.*/
			if(aesMode){
				SecureStreamInputArgument* KEY = static_cast<SecureStreamInputArgument*>(inputArgPtr[0]);
				//std::cout<<"WOOF! "<<KEY->buffer.size()<<std::endl;
				KEY->status = END_PROCESSING_BLOCK; //The keyBuffer will no longer be used in AES mode.
				std::vector<char> keyBuffer = KEY->readBuffer(128);
				Botan::PBKDF* pbkdf = Botan::get_pbkdf("PBKDF2(SHA-256)");
				if(!decryptMode){
					std::vector<char> outBuffer; //used to write the salt and IV.
					/*generat the key, salt and IV.*/
					Botan::AutoSeeded_RNG rng;
					std::string password(keyBuffer.begin(), keyBuffer.size()>128? keyBuffer.begin()+128: keyBuffer.end());
					Botan::SecureVector<unsigned char> salt = rng.random_vec(SALT_SIZE);

					Botan::OctetString aes256Key = pbkdf->derive_key(SALT_SIZE, password,
												   &salt[0], salt.size(),
												   31415); //oh! I <3 pie! :D
					Botan::InitializationVector iv(rng, IV_SIZE);
					Botan::Keyed_Filter* aesFilter = get_cipher("AES-256/CBC", aes256Key, iv, Botan::ENCRYPTION);
					/*write the salt and IV to outBuffer*/
					std::copy_n(salt.begin(), SALT_SIZE, std::back_inserter(outBuffer));
					std::copy_n(iv.begin(), IV_SIZE, std::back_inserter(outBuffer));
//printCheckSum(keyBuffer);
					cryptoPipe.reset();
					cryptoPipe.append(aesFilter);
					return outBuffer;
				}else{
					SecureStreamInputArgument* DATA = static_cast<SecureStreamInputArgument*>(inputArgPtr[1]);
					std::vector<char> dataBuffer = DATA->readBuffer(SALT_SIZE+IV_SIZE);
					if(dataBuffer.size()<SALT_SIZE+IV_SIZE)
						throw std::runtime_error("Invalid AES encrypted file.");
					std::vector<unsigned char> dataBufferTemp(dataBuffer.begin(), dataBuffer.begin()+SALT_SIZE+IV_SIZE);
					std::vector<unsigned char> salt( dataBufferTemp.begin(), dataBufferTemp.begin()+SALT_SIZE );
					Botan::InitializationVector iv(&dataBufferTemp.begin()[SALT_SIZE], IV_SIZE);
					std::string password(keyBuffer.begin(), keyBuffer.size()>128? keyBuffer.begin()+128:keyBuffer.end());
					Botan::OctetString aes256Key = pbkdf->derive_key(SALT_SIZE, password,
							   &salt[0], salt.size(),
							   31415); //oh! I <3 pie! :D
//printCheckSum(keyBuffer);
					Botan::Keyed_Filter* aesFilter = get_cipher("AES-256/CBC", aes256Key, iv, Botan::DECRYPTION);
					/*discard the salt and IV in the dataBuffer.*/
					cryptoPipe.reset();
					cryptoPipe.append(aesFilter);
				}
			}
			return std::vector<char>();
		}
		virtual std::vector<char> genData() override{
			SecureStreamInputArgument* KEY = static_cast<SecureStreamInputArgument*>(inputArgPtr[0]);
			SecureStreamInputArgument* DATA = static_cast<SecureStreamInputArgument*>(inputArgPtr[1]);
			/*read the data buffer,*/
			std::vector<char> dataBuffer = DATA->readBuffer(CHUNK_SIZE+((aesMode&&decryptMode)?AES_HEADER_SIZE:0));
			if(/*DATA->status==END_PROCESSING_BLOCK*/dataBuffer.size()==0){
				outputSize = dataSizeWritten;
				if(decryptMode)
					decryptionCompleted = true;
				return std::vector<char>();
			}
			std::vector<char> ret;
			if(aesMode){
				cryptoPipe.process_msg(reinterpret_cast<unsigned char*>(&dataBuffer[0]), dataBuffer.size());
				Botan::SecureVector<unsigned char> outBuffer = cryptoPipe.read_all(Botan::Pipe::LAST_MESSAGE);

				return std::vector<char>(&outBuffer[0], &outBuffer[0]+outBuffer.size());
			}else{ //one time pad mode
				/*initialize buffers*/
				std::vector<char> keyBuffer = KEY->readBuffer(dataBuffer.size());
				std::vector<char> outBuffer(dataBuffer.size());
				if(keyBuffer.size()<dataBuffer.size())
					throw std::runtime_error("The key is too small for en-/decrypting the entire file.");

				/*encrypt the file*/
				for(size_t i=0;i<dataBuffer.size();i++)
					outBuffer[i] = !decryptMode? dataBuffer[i]+keyBuffer[i]: dataBuffer[i]-keyBuffer[i];

				return outBuffer;
			}
		}
		virtual PARAM_PROC_STATUS preProcessParam(std::shared_ptr<Param_t> p) override{
			outputSize = std::numeric_limits<uint64_t>::max();
			PARAM_PROC_STATUS ret = super::preProcessParam(p);
			SecureStreamInputArgument* KEY = static_cast<SecureStreamInputArgument*>(inputArgPtr[0]);
			KEY->readBuffer(readOffset); //discard a amount of buffer.
			return ret;
		}
};

std::vector<std::shared_ptr<Argument>> arguments = {std::make_shared<KeyArgument>("key"),
													std::make_shared<SecureStreamInputArgument>("file"),
													std::make_shared<EncryptArgument>("en-/decryptedFile")};
std::vector<std::shared_ptr<Argument>> specialArguments = {std::make_shared<RecirectArgument>("redir"),
										std::make_shared<OptionArgument>("opt")};
uint32_t enabledOptions = SHOW_HELP|FORCE_OVERWRITE|NUKE_INPUT_FILE;

void init(){
Botan::LibraryInitializer botanInitializer;
	appendOption('a', aesMode, "encrypt the file with AES instead of one time pad. At most 128 bytes in the key is passed into PBKDF2(SHA-256) to generate a hashkey. The hashkey is then used to encrypt the file by using AES-256/CBC algorithm.");
	appendOption('d', decryptMode, "decrypt the file with the key provided.");
	appendOption('g', keyGenMode, "enable key generation mode.");
	appendOption('n', nukeMode, "nuke mode. nuke the source file after the operation is finished.");
	appendOption( uni::string(R"(^o=\d+)"), [](const uni::string paramStr){
	readOffset = strToSizeT(paramStr.substr(paramStr.find('=')+1));
	}, "Set the read offset of the key, in bytes.[default=0]");
}
void preProcess(){}
