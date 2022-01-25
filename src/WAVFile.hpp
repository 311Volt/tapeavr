#ifndef LIBSNDFILE_WAVFILE_HPP
#define LIBSNDFILE_WAVFILE_HPP

#include <sndfile.h>
#include <string>
#include <vector>

class WAVFile {
public:
	using AudioSignal = std::vector<std::vector<float>>;

	WAVFile(const std::string& filename, int numCh, int depth, int smpRate);
	WAVFile(const std::string& filename);
	~WAVFile();

	int getNumChannels();
	int getSampleRate();

	AudioSignal get(int smpBegin, int smpLength);
	void append(const AudioSignal& audio);

private:

	static constexpr int MODE_WRITE = 0;
	static constexpr int MODE_READ = 1;
	
	int mode;
	SF_INFO inf;
	SNDFILE* sf;
};

#endif