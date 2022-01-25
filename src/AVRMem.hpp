#ifndef TAPEAVR_AVR_MEM_HPP
#define TAPEAVR_AVR_MEM_HPP

#include <vector>
#include <stdexcept>

class AVRMem {
public:
	AVRMem(unsigned dataSize, unsigned pageWords, unsigned wordSize);

	bool write(unsigned addr, const uint8_t* dat, unsigned size);
	void read(unsigned addr, uint8_t* out, unsigned size);
	std::vector<uint8_t> read(unsigned addr, unsigned size);

	void readPage(unsigned pageNum, uint8_t* out);
	std::vector<uint8_t> readPage(unsigned pageNum);

	unsigned getAddrPage(unsigned addr);
	unsigned getPageAddr(unsigned pageNum);

	bool isPageUsed(unsigned pageNum);

	const unsigned dataSize;
	const unsigned numPages;
	const unsigned pageWords;
	const unsigned pageBytes;
	const unsigned wordSize;
private:
	std::vector<uint8_t> data;
	std::vector<bool> pageUsage;
};

#endif //TAPEAVR_AVR_MEM_HPP