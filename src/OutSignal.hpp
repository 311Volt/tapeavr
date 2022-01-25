#ifndef TAPEAVR_OUT_SIGNAL_HPP
#define TAPEAVR_OUT_SIGNAL_HPP

#include <array>
#include <string>

#include "AVRMem.hpp"
#include "WAVFile.hpp"

class OutSignal {
public:
	using SerialCmd = std::array<uint8_t, 4>;
	static constexpr SerialCmd CMD_PROGRAMMING_ENABLE {
		0xAC, 0x53, 0x00, 0x00
	};
	static constexpr SerialCmd CMD_CHIP_ERASE {
		0xAC, 0x80, 0x00, 0x00
	};

	class SignalParams {
	public:
		SignalParams(uint8_t clockPulseOffset, uint8_t dutyCycle, unsigned sampleRate, unsigned bitLength)
			: clockPulseOffset(clockPulseOffset), dutyCycle(dutyCycle),
			  sampleRate(sampleRate), bitLength(bitLength) {}
		
		const uint8_t clockPulseOffset;
		const uint8_t dutyCycle;
		const unsigned sampleRate;
		const unsigned bitLength;
	};

	static WAVFile::AudioSignal generateSilence(int millis, const SignalParams& signalParams);
	static WAVFile::AudioSignal generateBit(const SignalParams& signalParams, bool bit);

	class SignalSegments {
	public:
		SignalSegments(const SignalParams& signalParams);

		const SignalParams signalParams;
		const WAVFile::AudioSignal zero, one, idle;
	};

	class Message {
	public:
		Message(uint16_t preDelay, uint16_t postDelay, SerialCmd command);
		WAVFile::AudioSignal asAudioSignal(const SignalParams& signalParams);
	private:
		const uint16_t preDelay, postDelay; //in ms
		const SerialCmd command;
	};

	OutSignal(SignalParams signalParams);

	static constexpr SerialCmd MakeByteLoadCmd(uint8_t idxWord, bool idxByte, uint8_t byte);
	static constexpr SerialCmd MakeWritePageCmd(int pageNum);
	static std::vector<Message> fromAVRMem(AVRMem& avrMem);

	void addMessage(const Message& msg);
	void addMessages(const std::vector<Message>& msgs);

	void saveToWAV(const std::string& filename);
	
	const SignalParams signalParams;
private:

	std::vector<Message> messages;
	std::vector<WAVFile::AudioSignal> bitSignals;
};

#endif //TAPEAVR_OUT_SIGNAL_HPP