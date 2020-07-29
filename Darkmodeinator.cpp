
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

void Error(char* message);
class Scanner {
public:
	typedef struct _Signature {
		_Signature(const char* pattern, const char* mask, int offset) { szPattern = pattern; szMask = mask; lenght = strlen(szMask); offsetFromEnd = offset; }
		const char* szPattern;
		const char* szMask;
		int lenght;
		int offsetFromEnd;
	}Signature, * PSignature;

	Scanner() {};

	inline char* ScannPattern(char* pBuffer, int bufferSize, Signature& sig) {
		for (size_t i = 0; i < bufferSize - sig.lenght; ++i, ++pBuffer)
		{		
			if (ComparePattern(pBuffer, sig.szPattern, sig.szMask))
				return pBuffer;
		}
		return nullptr;
	}
private:

	inline bool ComparePattern(char* szSource, const char* szPattern, const char* szMask)
	{
		for (; *szMask; ++szSource, ++szPattern, ++szMask)
			if (*szMask == 'x' && *szSource != *szPattern)
				return false;

		return true;
	}


};

class Patcher {
public:
	Patcher() {};
	void Patch(void* pAddr, uint8_t* bytes, int size)
	{
		uint8_t* pointer = reinterpret_cast<uint8_t*>(pAddr);
		for (size_t i = 0; i < size; i++, pointer++, bytes++)
		{
			*pointer = *bytes;
		}
	}
private:
};


std::map<std::string, Scanner::Signature> unityVersions = {
	{"2020.1.0f1", Scanner::Signature("\x40\x53\x48\x83\xEC\x40\x48\xC7\x44\x24\x20\xFE\xFF\xFF\xFF\x48\x8B\xD9\x8B\x00\x00\x00\x00\x00\x65\x48\x8B\x04\x25\x58\x00\x00\x00\xB9\x20\x00\x00\x00\x48\x8B\x14\xD0\x8B\x04\x11\x39\x00\x00\x00\x00\x00\x7F\x25\x80\x3D\x00\x00\x00\x00\x00\x75\x14", "xxxxxxxxxxxxxxxxxxx?????xxxxxxxxxxxxxxxxxxxxxx?????xxxx?????xx", -2)},
	{"2019.4.5f1", Scanner::Signature("\x48\x8B\xC4\x57\x48\x83\xEC\x70\x48\xC7\x40\xA8\xFE\xFF\xFF\xFF\x48\x89\x58\x08\x48\x89\x68\x10\x48\x89\x70\x18\x48\x8B\xF9\x8B\x00\x00\x00\x00\x00\x65\x48\x8B\x04\x25\x58\x00\x00\x00\xB9\x20\x00\x00\x00\x48\x8B\x14\xD0\x8B\x04\x11\x39\x00\x00\x00\x00\x00\x7F\x36\x80\x3D\x00\x00\x00\x00\x00\x75\x15", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?????xxxxxxxxxxxxxxxxxxxxxx?????xxxx?????xx", -2)}
};


int main(int argc, char* argv[], char* envp[])
{
	if (argc < 2)
		Error((char*)"Please Drag Unity.exe on Darkemodeinator.exe");

	if(strstr(argv[1], "Unity.exe") == NULL)
		Error((char*)"Path doesn't contain Unity.exe");

	std::cout << "Please insert your Unity version. For Example: 2020.1.0f1. Or enter a random character an scann for all versions (takes longer)." << std::endl;
	std::string version;
	std::cin >> version;

	//read unity.exe into buffer
	std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);


	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		std::cout << "Exe loaded into memory: " << buffer.size() << " Bytes" << std::endl;
		file.close();
	}
	else { file.close(); Error((char*)"Error loading file into memory"); }


	//find pattern
	bool found = false;
	char* addy = nullptr;
	Scanner::Signature currentSig("0", "0", 0);
	auto it = unityVersions.find(version);
	if (it != unityVersions.end()) {
		std::cout << "Unity version found!" << std::endl;
		std::cout << "Scanning..." << std::endl;
		Scanner s;
		currentSig = (*it).second;
		addy = s.ScannPattern(buffer.data(), buffer.size(), currentSig);
		found = addy == nullptr ? false : true;
	}

	if (found == false)
	{
		std::cout << "Unity version or pattern was NOT found. Trying patterns of other versions..." << std::endl;
		std::cout << "Scanning..." << std::endl;
		Scanner s;
		for (auto [key, val] : unityVersions)
		{
			//just scann all versions

			//if (key == version) //alredy scanned above
			//	continue;

			currentSig = val;
			addy = s.ScannPattern(buffer.data(), buffer.size(), currentSig);
			if (addy != nullptr)
				break;
		}
	}

	if (addy != nullptr)
		std::cout << "Signature found!" << std::endl;
	else
		Error((char*)"Signature not found");
   
		
	std::cout << "Patching..." << std::endl;
	auto pos = static_cast<void*>(addy + (currentSig.lenght + currentSig.offsetFromEnd));

	//Patch at found address
	Patcher patcher;
	uint8_t patch[] = { 0x74 };
	patcher.Patch(pos, patch, 1);

	std::cout << "Writing to file..." << std::endl;
	std::ofstream outFile("PatchedUnity.exe", std::ofstream::binary);
	if (outFile.write(buffer.data(), size))
		std::cout << "Successfully activated DarkMode for Unity." << std::endl << "You can now rename the created PatchedUnity.exe to Unity.exe and replace the original one";
	else
	{
		outFile.close(),
		Error((char*)"Can't create new file. Try run Darkmodeinator with admin rights or move the original Unity.exe in a directory where no admin rights are required. For example to the Desktop");
	}
	
	outFile.close();
	
	std::string stopper;
	std::cin >> stopper;

}

void Error(char* message)
{
	std::cout << message << std::endl;
	std::string stopper;
	std::cin >> stopper;
	exit(0);
}



