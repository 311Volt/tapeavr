#include "AVRMem.hpp"

AVRMem::AVRMem(unsigned dataSize, unsigned pageWords, unsigned wordSize)
	: dataSize(dataSize), pageWords(pageWords), wordSize(wordSize),
	  numPages(dataSize / (pageWords * wordSize)), pageBytes(pageWords*wordSize)
{
	if(dataSize % pageBytes) {
		throw std::invalid_argument("error: memory size does not divide page size");
	}

	data = std::vector<uint8_t>(dataSize, 0xFF);
	pageUsage = std::vector<bool>(dataSize, false);
}

bool AVRMem::write(unsigned addr, const uint8_t* dat, unsigned size)
{
	if(size == 0) {
		return true;
	}
	if(addr+size-1 > dataSize) {
		return false;
	}
	unsigned pageBegin = getAddrPage(addr);
	unsigned pageEnd = getAddrPage(addr+size-1);

	for(unsigned page=pageBegin; page<=pageEnd; page++) {
		pageUsage[page] = true;
	}
	std::copy(dat, dat+size, data.begin()+addr);
	return true;
}

void AVRMem::read(unsigned addr, uint8_t* out, unsigned size)
{
	if(addr+size > dataSize) {
		throw std::out_of_range("AVRMem::read() out of range");
	}
	//printf("addr=%d size=%d addr+size=%d, dataSize=%d",addr,size,addr+size,dataSize);
	for(int i=addr; i<addr+size; i++) {
		*out++ = data[i];
	}
	//std::copy(data.begin()+addr, data.end()+addr+size, out);
}

std::vector<uint8_t> AVRMem::read(unsigned addr, unsigned size)
{
	std::vector<uint8_t> ret(size);
	read(addr, ret.data(), size);
	return ret;
}

void AVRMem::readPage(unsigned pageNum, uint8_t* out)
{
	read(getPageAddr(pageNum), out, pageBytes);
}

std::vector<uint8_t> AVRMem::readPage(unsigned pageNum)
{
	return read(getPageAddr(pageNum), pageBytes);
}

unsigned AVRMem::getAddrPage(unsigned addr)
{
	return addr / pageBytes;
}

unsigned AVRMem::getPageAddr(unsigned pageNum)
{
	return pageNum * pageBytes;
}

bool AVRMem::isPageUsed(unsigned pageNum)
{
	return pageUsage.at(pageNum);
}