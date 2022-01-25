#include "WAVFile.hpp"
#include <stdexcept>

WAVFile::WAVFile(const std::string& filename, int numCh, int depth, int smpRate)
{
	mode = MODE_WRITE;

	inf.frames = 0;
	inf.samplerate = smpRate;
	inf.channels = numCh;
	inf.format = SF_FORMAT_WAV;

	switch(depth) {
		case 8: inf.format |= SF_FORMAT_PCM_U8; break;
		case 16: inf.format |= SF_FORMAT_PCM_16; break;
		case 24: inf.format |= SF_FORMAT_PCM_24; break;
		case 32: inf.format |= SF_FORMAT_FLOAT; break;
		default: throw std::runtime_error("unsupported bit depth");
	}

	sf = sf_open(filename.c_str(), SFM_WRITE, &inf);
	if(!sf) {
		throw std::runtime_error("Error while opening " + filename + ": " + sf_strerror(sf));
	}
}
WAVFile::WAVFile(const std::string& filename)
{
	mode = MODE_READ;

	sf = sf_open(filename.c_str(), SFM_READ, &inf);
	if(!sf) {
		throw std::runtime_error("Error while opening " + filename + ": " + sf_strerror(sf));
	}
}
WAVFile::~WAVFile()
{
	if(sf) {
		sf_close(sf);
	}
}

int WAVFile::getNumChannels()
{
	return inf.channels;
}
int WAVFile::getSampleRate()
{
	return inf.samplerate;
}

WAVFile::AudioSignal WAVFile::get(int smpBegin, int smpLength)
{
	std::vector<float> frames(getNumChannels()*smpLength);
	AudioSignal output(2, std::vector<float>(smpLength));

	sf_readf_float(sf, frames.data(), smpLength);
	for(int i=0; i<getNumChannels(); i++) {
		for(int s=0; s<smpLength; s++) {
			output[i][s] = frames[s*getNumChannels() + i];
		}
	}
	return output;
}

void WAVFile::append(const WAVFile::AudioSignal& audio)
{
	int smpLength = audio.at(0).size();
	std::vector<float> frames(getNumChannels()*smpLength);

	for(auto& ch: audio) {
		if(ch.size() != audio[0].size()) {
			throw std::runtime_error("channel sizes do not match");
		}
	}
	for(int i=0; i<getNumChannels(); i++) {
		for(int s=0; s<smpLength; s++) {
			frames[s*getNumChannels() + i] = audio[i][s];
		}
	}
	
	sf_writef_float(sf, frames.data(), smpLength);
}
