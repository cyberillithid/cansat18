#include "lora.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
//#define DEBUG
#include "radio.h"
#include <unistd.h>
#include <ctime>
#include <chrono>


LoRa::LoRa(const char* devAddr, uint8_t addr) { //"spidev1.0"
	
	mode = SPI_MODE_0;
	//TODO: run RESET ?
	fd = open(devAddr, O_RDWR);
	if (fd < 0)
		pthrow("can't open device");
	spiInit();
	
	//SX::ON
	uint8_t ver = getVersion();
	if (ver == 0x12) {
		// sx1276
		printf("SX1276/7/8/9 detected, starting.\n");
		//_board = SX1276Chip;
	} else {
		printf("Unrecognized transceiver: version %x.\n", ver);
		throw new mexception("Unrecognized transceiver");
	}
	
		//TODO: RxChainCalibration();
	setMaxCurrent();
	setLORA();
	fetchPreambleLength();
	fetchSyncWord();
	////start config
	setMode(); ////lora mode = 1
		// SIFS_cad_number=3; -- ???
	setChannel(0xD84CCC); //// loraChannel = 0xD84CCC; //CH_10_868; 
	setPower(); // setPower('M'); //or 'X'
	_nodeAddr = addr;
	_packNo = 0;
	printf("\nConfiguration successful");
	// cfg finished
	_length = 0;
	
}

void LoRa::transfer(uint8_t* tx, uint8_t sz, uint8_t* rx)
{
	int ret;
	struct spi_ioc_transfer tr;
	memset(&tr, 0, sizeof(struct spi_ioc_transfer));
		tr.tx_buf = (unsigned long)tx;
		tr.rx_buf = (unsigned long)rx;
		tr.len = sz;
		tr.speed_hz = speed;
		tr.delay_usecs = S_delay;
		tr.bits_per_word = bits;
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pthrow("can't send spi message");
#ifdef SPI_DEBUG
	for (ret = 0; ret < sz; ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");
#endif
}

void LoRa::spiInit(){
	int ret;
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pthrow("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pthrow("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pthrow("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pthrow("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pthrow("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pthrow("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}

uint8_t LoRa::readRegister(uint8_t reg){
	uint8_t tx[] = {reg, 0};
	uint8_t rx[] = {0, 0};
	transfer(tx, 2, rx);
	return rx[1];
}

void LoRa::writeRegister(uint8_t reg, uint8_t val) {
	uint8_t tx[] = {(uint8_t)(reg | 0x80), val};
	uint8_t rx[] = {0,0};
	transfer(tx, 2, rx);
}

void LoRa::setMaxCurrent() {
	//0x1B
	uint8_t rate = 0x1B | 0B00100000; //240 mA
	uint8_t st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	writeRegister(REG_OCP, rate);
	writeRegister(REG_OP_MODE, st0);
}

void LoRa::setLORA() {
	uint8_t retry=0;
	uint8_t st0;

	do {
		delay(200);
		writeRegister(REG_OP_MODE, FSK_SLEEP_MODE);    // Sleep mode (mandatory to set LoRa mode)
		writeRegister(REG_OP_MODE, LORA_SLEEP_MODE);    // LoRa sleep mode
		writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
		delay(50+retry*10);
		st0 = readRegister(REG_OP_MODE);
		printf("...\n");

		if ((retry % 2)==0) {
			if (retry==20)
				retry=0;
			else
				retry++;
			}
		/* TODO: RESET
		if (st0!=LORA_STANDBY_MODE) {
			pinMode(SX1272_RST,OUTPUT);
			digitalWrite(SX1272_RST,HIGH);
			delay(100);
			digitalWrite(SX1272_RST,LOW);
		}
		*/

	} while (st0!=LORA_STANDBY_MODE);	// LoRa standby mode

	if( st0 == LORA_STANDBY_MODE )
	{ // LoRa mode
//		_modem = LORA;
//		state = 0;
//if (SX1272_debug_mode > 1)
		printf("## LoRa set with success ##\n");
		printf("\n");
//endif
	}
	else
	{ // FSK mode
		throw new mexception("LORA installation failed");
	}
}

void LoRa::fetchPreambleLength() {
	uint16_t ret;
	uint8_t* pPL = (uint8_t*)(&ret);
	pPL[0] = readRegister(REG_PREAMBLE_LSB_LORA);
	pPL[1] = readRegister(REG_PREAMBLE_MSB_LORA);
	_preambleLength = ret;
}

void LoRa::fetchSyncWord() {
	uint8_t st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);		// Set Standby mode to write in registers
	writeRegister(REG_SYNC_WORD, 0x12);
	delay(100);
	uint8_t config1 = readRegister(REG_SYNC_WORD);
	if (config1==0x12) {
		_syncWord = 0x12;
		printf("## Sync Word 0x12 has been successfully set ##\n");
	}
	else {
		throw new mexception("Sync word setting failure");
	}
writeRegister(REG_OP_MODE,st0);	// Getting back to previous status
delay(100);
}

void LoRa::setMode(){ //using 'mode 1'
	 uint8_t st0 = readRegister(REG_OP_MODE);
	 writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	 //setCR(CR_5);        // CR = 4/5     
	//setBW(BW_125);      // BW = 125 KHz
	uint8_t config1 = 0B01110010; //readRegister(REG_MODEM_CONFIG1);
	//config1 &= 1;
	//config1 |= 0B01110010;
	writeRegister(REG_MODEM_CONFIG1, config1);		// Update config1

	//setSF(SF_12);       // SF = 12
	uint8_t config2 = readRegister(REG_MODEM_CONFIG2);
	config2 = config2 & 0B11001111;	// clears bits 5 & 4 from REG_MODEM_CONFIG2
	config2 = config2 | 0B11000000;	// sets bits 7 & 6 from REG_MODEM_CONFIG2
	
	uint8_t config3=readRegister(REG_MODEM_CONFIG3);
	config3 = config3 | 0B00001000;
	writeRegister(REG_MODEM_CONFIG3,config3);
	
	//setHeaderON();
	// LoRa detection Optimize: 0x03 --> SF7 to SF12
	writeRegister(REG_DETECT_OPTIMIZE, 0x03);

	// LoRa detection threshold: 0x0A --> SF7 to SF12
	writeRegister(REG_DETECTION_THRESHOLD, 0x0A);
	
	writeRegister(REG_MODEM_CONFIG2, config2);		// Update config2
	
	delay(100);
	
	// verify
	config1 = readRegister(REG_MODEM_CONFIG1);
	if( (config1 >> 1) != 0x39 )
		throw new mexception("Error setting mode");
	config2 = readRegister(REG_MODEM_CONFIG2);
	if( (config2 >> 4) != SF_12 )
		throw new mexception("Error setting mode");
	writeRegister(REG_OP_MODE, st0);	// Getting back to previous status
	delay(100);
}

void LoRa::setChannel(uint32_t freq) {
	uint32_t f = freq;
	uint8_t* pF = (uint8_t*)(&f);
	uint8_t st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	writeRegister(REG_FRF_MSB, pF[2]);
	writeRegister(REG_FRF_MID, pF[1]);
	writeRegister(REG_FRF_LSB, pF[0]);
	delay(100);
	pF[2] = readRegister(REG_FRF_MSB);
	pF[1] = readRegister(REG_FRF_MID);
	pF[0] = readRegister(REG_FRF_LSB);
	writeRegister(REG_OP_MODE, st0);
	if (f != freq) 
		throw new mexception("Frequency set failed");
		
}

void LoRa::setPower() {
	uint8_t st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	// 0x0F
	writeRegister(0x4d, 0x84); // ???
	writeRegister(REG_PA_CONFIG, 0x7F);
	if (readRegister(REG_PA_CONFIG) != 0x7F)
		throw new mexception("Power cfg failed");
	writeRegister(REG_OP_MODE, st0);
}

void LoRa::clearFlags() {
	
	uint8_t st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);	// Stdby mode to write in registers
	writeRegister(REG_IRQ_FLAGS, 0xFF);	// LoRa mode flags register
	writeRegister(REG_OP_MODE, st0);
	
}

uint8_t LoRa::getVersion() {
	return readRegister(0x42);
}

bool LoRa::sendPacketTimeout(uint8_t dest, char* payload, uint16_t payloadLen, uint16_t wait) {
	// truncPacket
	// setPacket (dest, payload)
	uint8_t st0 = readRegister(REG_OP_MODE);	// Save the previous status
	clearFlags();
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	
	//setDestination, setPayload
	writeRegister(REG_PAYLOAD_LENGTH_LORA, payloadLen);
	//packet_sent.type |= PKT_TYPE_DATA; // | PKT_FLAG_ACK_REQ;

	writeRegister(REG_FIFO_ADDR_PTR, 0x80); 
	
	writeRegister(REG_FIFO, dest); 		// Writing the destination in FIFO
	// added by C. Pham
	writeRegister(REG_FIFO, PKT_TYPE_DATA); 		// Writing the packet type in FIFO
	writeRegister(REG_FIFO, _nodeAddr);		// Writing the source in FIFO
	writeRegister(REG_FIFO, _packNo++);	// Writing the packet number in FIFO
	for(unsigned int i = 0; i < payloadLen; i++)
	{
		writeRegister(REG_FIFO, payload[i]);  // Writing the payload in FIFO
	}
	
	writeRegister(REG_OP_MODE, st0);
	
	//sendwithtimeout
	clearFlags();
	writeRegister(REG_OP_MODE, LORA_TX_MODE);  
	uint8_t value = readRegister(REG_IRQ_FLAGS);
	auto finWait = std::chrono::steady_clock::now() + std::chrono::milliseconds(wait);
	while (((value & 8) == 0) && (std::chrono::steady_clock::now() < finWait)){
		value = readRegister(REG_IRQ_FLAGS);
		delay(100);
	}
	return ((readRegister(REG_IRQ_FLAGS) & 8) == 8);
}

char* LoRa::receiveAll(uint16_t timeout) {
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	clearFlags();	
	writeRegister(REG_PA_RAMP, 0x08);
	writeRegister(REG_LNA, LNA_MAX_GAIN);
	writeRegister(REG_FIFO_ADDR_PTR, 0x00);  // Setting address pointer in FIFO data buffer
	writeRegister(REG_SYMB_TIMEOUT_LSB,0x05);
	writeRegister(REG_FIFO_RX_BYTE_ADDR, 0x00);
	//state = setPacketLength(MAX_LENGTH);	// With MAX_LENGTH gets all packets with length < MAX_LENGTH
	writeRegister(REG_PAYLOAD_LENGTH_LORA, 252);
		
	writeRegister(REG_OP_MODE, LORA_RX_MODE);  	  // LORA mode - Rx
	
	uint8_t value = readRegister(REG_IRQ_FLAGS);
	auto finWait = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
	while (((value & 64) == 0) && (std::chrono::steady_clock::now() < finWait)) {
		value = readRegister(REG_IRQ_FLAGS);
		delay(10);
	}
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);	
	if ((value & 64) == 64) {
		if ((value & 32) == 32) {
			printf("CRC incorrect\n");
			_length = 0;
			return nullptr;
		}
	} else {
		printf("Packet not received\n");
		_length = 0;
		return nullptr;
	}
	
	writeRegister(REG_FIFO_ADDR_PTR, 0x00); 
	uint8_t dst = readRegister(REG_FIFO);
	uint8_t type = readRegister(REG_FIFO);
	uint8_t src = readRegister(REG_FIFO);
	uint8_t packno = readRegister(REG_FIFO);
	
	_length = readRegister(REG_RX_NB_BYTES); // - 4;
	char* pkg = new char [_length];
	
	for (int i = 0; i < _length; i++) {
		pkg[i] = readRegister(REG_FIFO);
	}
	
	if (loglevel > 2) {
	printf("## Packet received:\n");
		printf("Destination: ");
		printf("%d\n", dst);			 	// Printing destination
		printf("Source: ");
		printf("%d\n", src);			 	// Printing source
		printf("Packet type: ");
		printf("%d\n", type);			// Printing packet number
		printf("Packet number: ");
		printf("%d\n", packno);			// Printing packet number
		printf("Packet length: ");
		printf("%d\n", _length);			// Printing packet length
		if (loglevel > 4){
		printf("Data: ");
		for(unsigned int i = 0; i < _length; i++)
		{
			printf("%c", pkg[i]);		// Printing payload
		}
		printf("\n");
	}
		printf(" ##\n");
		printf("\n");
	}
	return pkg;
}

void LoRa::setLogLevel(int ll) {
	loglevel = ll;
}

uint8_t LoRa::getLength() {
	return _length;
}
/*
char* fetchPacket() {
	uint8_t value = readRegister(REG_PKT_SNR_VALUE);
	int8_t _SNR;
	if( value & 0x80 ) // The SNR sign bit is 1
	{
		// Invert and divide by 4
		value = ( ( ~value + 1 ) & 0xFF ) >> 2;
		_SNR = -value;
	}
	else
	{
		// Divide by 4
		_SNR = ( value & 0xFF ) >> 2;
	}
	state = 0;
	printf("## SNR value is ");
	printf("%d", _SNR);
	printf(" ##\n");
	printf("\n");
}
*/
