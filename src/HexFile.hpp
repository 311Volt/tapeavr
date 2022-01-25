#ifndef TAPEAVR_HEX_FILE
#define TAPEAVR_HEX_FILE

#include <cstdint>
#include <vector>
#include <string>

#include "AVRMem.hpp"

class HexFile {
public:
	static constexpr char START_CODE = ':';

	class Record {
	public:
		static constexpr uint8_t REC_TYPE_DATA = 0;
		static constexpr uint8_t REC_TYPE_EOF = 1;
		
		Record(const std::string& line);

		uint8_t byteCount;
		uint16_t address;
		uint8_t recordType;
		std::vector<uint8_t> data;
		uint8_t checksum;

		const uint8_t* dataPtr();
		bool validate();
	private:
		int strIndex;
		unsigned findStartIndex(const std::string& ln);
		void readField(const std::string& ln, int& idx, int size, uint8_t* output);
		uint32_t readFieldAsInteger(const std::string& ln, int& idx, int size);
	};

	HexFile(const std::string& filename);

	void dumpToAVRMemory(AVRMem& avrMem);
private:
	std::vector<Record> records;
};

#endif //TAPEAVR_INTEL_HEX