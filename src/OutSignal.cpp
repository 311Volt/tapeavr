#include "OutSignal.hpp"

OutSignal::SignalSegments::SignalSegments(const SignalParams& signalParams)
	: signalParams(signalParams),
	  zero(generateBit(signalParams, 0)),
	  one(generateBit(signalParams, 1)),
	  idle(generateSilence(signalParams.bitLength, signalParams))
{

}

WAVFile::AudioSignal OutSignal::generateSilence(int millis, const SignalParams& signalParams)
{
	int samples = (millis * signalParams.sampleRate) / 1000;
	return {
		std::vector<float>(samples, 0.0),
		std::vector<float>(samples, 0.0)
	};
}

WAVFile::AudioSignal OutSignal::generateBit(const SignalParams& signalParams, bool bit)
{
	int len = signalParams.bitLength;
	int durHigh = len * signalParams.dutyCycle / 255;
	std::vector<float> dat(len);
	std::vector<float> clk(len);

	int smpClockOffset = len*uint32_t(signalParams.clockPulseOffset) / 255;

	for(int i=0; i<len; i++) {
		float x = float(i) / float(len-1);
		float xn = (x - 0.5) * 2.0;
		int x1 = (i+smpClockOffset) % len;

		dat[i] = (bit ? 1.0 : 0.0) / (1.0 + xn*xn*100.0);
		clk[i] = (x1 > len/2 && x1 < len/2+durHigh) ? 1.0 : 0.0;
	}

	return WAVFile::AudioSignal {dat, clk};
}


OutSignal::Message::Message(uint16_t preDelay, uint16_t postDelay, SerialCmd command)
	: preDelay(preDelay), postDelay(postDelay), command(command)
{

}

void AppendAudioSignal(WAVFile::AudioSignal& dst, const WAVFile::AudioSignal& src)
{
	for(int ch=0; ch<dst.size(); ch++) {
		dst[ch].insert(dst[ch].end(), src[ch].begin(), src[ch].end());
	}
}

WAVFile::AudioSignal OutSignal::Message::asAudioSignal(const SignalParams& signalParams)
{
	SignalSegments segs(signalParams);
	WAVFile::AudioSignal ret(2);

	AppendAudioSignal(ret, generateSilence(preDelay, signalParams));

	for(auto& byte: command) {
		for(int b=0; b<8; b++) {
			bool bit = byte & (0x80 >> b);
			AppendAudioSignal(ret, bit ? segs.one : segs.zero);
		}
	}

	AppendAudioSignal(ret, generateSilence(postDelay, signalParams));
	return ret;
}


OutSignal::OutSignal(SignalParams signalParams)
	: signalParams(signalParams)
{

}

constexpr OutSignal::SerialCmd OutSignal::MakeByteLoadCmd(uint8_t idxWord, bool idxByte, uint8_t byte)
{
	SerialCmd ret {0x40, 0x00, 0x00, byte};
	ret[2] = idxWord;
	if(idxByte)
		ret[0] |= 0x08;
	return ret;
}

constexpr OutSignal::SerialCmd OutSignal::MakeWritePageCmd(int pageNum)
{
	SerialCmd ret {0x4C, 0x00, 0x00, 0x00};
	ret[1] |= pageNum >> 3;
	ret[2] |= (pageNum & 0x07) << 5;
	return ret;
}

std::vector<OutSignal::Message> OutSignal::fromAVRMem(AVRMem& avrMem)
{
	if(avrMem.pageWords != 32 || avrMem.wordSize != 2) {
		throw std::invalid_argument("unsupported AVR memory layout");
	}
	std::vector<OutSignal::Message> ret;
	for(int p=0; p<avrMem.numPages; p++) {
		if(avrMem.isPageUsed(p)) {
			auto page = avrMem.readPage(p);
			for(int b=0; b<page.size(); b++) {
				ret.push_back(Message(0, 0, MakeByteLoadCmd(b/2, b&1, page.at(b))));
			}
			ret.push_back(Message(0, 10, MakeWritePageCmd(p)));
		}
	}
	return ret;
}

void OutSignal::addMessage(const OutSignal::Message& msg)
{
	messages.push_back(msg);
}

void OutSignal::addMessages(const std::vector<OutSignal::Message>& msgs)
{
	for(const auto& msg: msgs) {
		addMessage(msg);
	}
}

void OutSignal::saveToWAV(const std::string& filename)
{
	WAVFile output(filename, 2, 8, signalParams.sampleRate);
	for(auto& msg: messages) {
		output.append(msg.asAudioSignal(signalParams));
	}
}