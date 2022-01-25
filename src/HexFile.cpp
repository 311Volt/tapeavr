#include "HexFile.hpp"

#include <fstream>

unsigned HexDigit(char d)
{
	if(d >= '0' && d <= '9') {
		return d - '0';
	} else if(d >= 'a' && d <= 'f') {
		return 10 + d - 'a';
	} else if(d >= 'A' && d <= 'F') {
		return 10 + d - 'A';
	}

	throw std::out_of_range("hex digit expected, got " + std::string({d}));
	return 0;
}

unsigned HexFile::Record::findStartIndex(const std::string& ln)
{
	int ret = 0;
	for(ret=0; ret<ln.size(); ret++) {
		if(ln[ret] == START_CODE) {
			ret++;
			break;
		}
	}
	if(ret == ln.size()) {
		throw std::out_of_range("could not find start code");
	}
	return ret;
}

void HexFile::Record::readField(const std::string& ln, int& idx, int size, uint8_t* output)
{
	if(idx >= (int)ln.size()-1) {
		throw std::out_of_range("unexpected EOL while reading field");
	}
	for(int i=0; i<size; i++) {
		*output++ = (HexDigit(ln[idx])<<4) + (HexDigit(ln[idx+1]));
		idx += 2;
	}
}

uint32_t HexFile::Record::readFieldAsInteger(const std::string& ln, int& idx, int size)
{
	if(size < 1 || size > 4) {
		throw std::invalid_argument("invalid size for Record::readFieldAsInteger");
	}
	uint32_t ret = 0;
	uint8_t bytes[4] = {0,0,0,0};
	readField(ln, idx, size, bytes);
	for(int i=0; i<size; i++) {
		ret = (ret<<8) + bytes[i];
	}
	return ret;
}

const uint8_t* HexFile::Record::dataPtr()
{
	return data.data();
}

HexFile::Record::Record(const std::string& line)
{
	enum {
		F_INIT=0,
		F_BYTECOUNT=1,
		F_ADDR=2,
		F_RECTYPE=3,
		F_DAT=4,
		F_CHKSUM=5,
		F_END=6
	};
	static std::vector<std::string> field_names{
		"init",
		"byte count",
		"address",
		"record type",
		"data",
		"checksum",
		"end"
	};
	int state = F_INIT;
	try {
		int idx = findStartIndex(line);
		
		state = F_BYTECOUNT;
		byteCount = readFieldAsInteger(line, idx, 1);
		
		state = F_ADDR;
		address = readFieldAsInteger(line, idx, 2);
		
		state = F_RECTYPE;
		recordType = readFieldAsInteger(line, idx, 1);

		if(recordType > 1) {
			throw std::out_of_range("wrong record type (sorry, only I8HEX is implemented as of now)");
		}

		state = F_DAT;
		data.resize(byteCount);
		readField(line, idx, byteCount, data.data());

		state = F_CHKSUM;
		checksum = readFieldAsInteger(line, idx, 1);

		state = F_END;
		//printf("bc=%d adr=%d rt=%d cs=%d\n", (int)byteCount, (int)address, (int)recordType, (int)checksum);
	} catch(std::out_of_range& e) {
		throw std::runtime_error("error while parsing " + field_names[state] + "field in line " + line + ": " + e.what());
	}
}

HexFile::HexFile(const std::string& filename)
{
	std::ifstream inFile(filename);
	std::string line;

	while(std::getline(inFile, line)) {
		if(line.empty()) {
			continue;
		}
		records.push_back(Record(line));
	}
	//printf("%d records read\n", int(records.size()));
}

void HexFile::dumpToAVRMemory(AVRMem& avrMem)
{
	for(auto& record: records) {
		if(record.recordType == Record::REC_TYPE_DATA) {
			//printf("writing %d byte record to %d\n", (int)record.byteCount, (int)record.address);
			avrMem.write(record.address, record.dataPtr(), record.byteCount);
		}
	}
}