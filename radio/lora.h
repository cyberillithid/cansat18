#include <stdint.h>

#define RCVR_ADDR 6
#define SNDR_ADDR 1


//FREQUENCY CHANNELS:
const uint32_t CH_10_868 = 0xD84CCC; // channel 10, central freq = 865.20MHz
const uint32_t CH_11_868 = 0xD86000; // channel 11, central freq = 865.50MHz
const uint32_t CH_12_868 = 0xD87333; // channel 12, central freq = 865.80MHz
const uint32_t CH_13_868 = 0xD88666; // channel 13, central freq = 866.10MHz
const uint32_t CH_14_868 = 0xD89999; // channel 14, central freq = 866.40MHz
const uint32_t CH_15_868 = 0xD8ACCC; // channel 15, central freq = 866.70MHz
const uint32_t CH_16_868 = 0xD8C000; // channel 16, central freq = 867.00MHz
const uint32_t CH_17_868 = 0xD90000; // channel 17, central freq = 868.00MHz

// added by C. Pham
const uint32_t CH_18_868 = 0xD90666; // 868.1MHz for LoRaWAN test


const uint32_t CH_00_433 = 0x6C8000; //chanel 0 ,central freq = 433.00MHZ
const uint32_t CH_01_433 = 0x6C8C3B; //chanel 1 ,central freq = 434.19MHZ
const uint32_t CH_02_433 = 0x6C9907; //chanel 2 ,central freq = 434.39MHZ
const uint32_t CH_03_433 = 0x6CA5D4; //chanel 3 ,central freq = 434.59MHZ
const uint32_t CH_04_433 = 0x6CB2A1; //chanel 4 ,central freq = 434.79MHZ
const uint32_t CH_05_433 = 0x6D7F6E; //chanel 5 ,central freq = 437.99MHZ

const uint32_t CH_433_80 = 0x6C7345; // freq = idx * 61.035 / 1M
const uint32_t CH_866_80 = 0xD8B357;


class LoRa {
	//SPIDevice [implement later?]
private:

	//SPI funcs
	int fd;
	uint8_t mode;
	uint8_t bits = 8;
	uint32_t speed = 500000;
	uint16_t S_delay = 0;
	
	void transfer(uint8_t* tx, uint8_t sz, uint8_t* rx);
	void spiInit();
	uint8_t readRegister(uint8_t reg);
	void writeRegister(uint8_t reg, uint8_t val);
	
	int loglevel = 5;
	void setMaxCurrent();
	void setLORA();
	uint16_t _preambleLength;
	void fetchPreambleLength();
	uint8_t _syncWord;
	void fetchSyncWord();
	void setMode();
	void setChannel(uint32_t freq);
	void setPower();
	void clearFlags();
	uint8_t _nodeAddr, _packNo;
	uint8_t _length;
public:
	LoRa(const char* devAddr, uint8_t addr, uint32_t ch = CH_10_868);
	uint8_t getVersion();
	uint8_t getLength();
	
	uint8_t _dst, _type, _src, _packno;
	
	void setLogLevel(int ll);
	bool sendPacketTimeout(uint8_t dest, char* payload, uint16_t payloadLen, uint16_t wait);
	char* receiveAll(uint16_t timeout);
};
