#include "WAVFile.hpp"
#include <cstdio>
#include <fstream>
#include <cmath>

#include "HexFile.hpp"
#include "AVRMem.hpp"
#include "OutSignal.hpp"

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("please provide an input filename\n");
		return 1;
	}
	std::string filename(argv[1]);
	AVRMem avm(8192, 32, 2);
	HexFile prg(filename);
	
	prg.dumpToAVRMemory(avm);
	OutSignal out(OutSignal::SignalParams(15, 40, 44100, 40));
	out.addMessage(OutSignal::Message(300, 1500, OutSignal::CMD_PROGRAMMING_ENABLE));
	out.addMessage(OutSignal::Message(0, 500, OutSignal::CMD_CHIP_ERASE));
	out.addMessages(out.fromAVRMem(avm));
	out.saveToWAV("output.wav");
}