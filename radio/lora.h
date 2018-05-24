#include <stdint.h>

#define RCVR_ADDR 6
#define SNDR_ADDR 1

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
	LoRa(const char* devAddr, uint8_t addr);
	uint8_t getVersion();
	uint8_t getLength();
	
	uint8_t _dst, _type, _src, _packno;
	
	void setLogLevel(int ll);
	bool sendPacketTimeout(uint8_t dest, char* payload, uint16_t payloadLen, uint16_t wait);
	char* receiveAll(uint16_t timeout);
};
