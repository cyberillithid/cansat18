#ifndef radio_h
#define radio_h
//! REGISTERS //
#define        REG_OP_MODE        				0x01
#define        REG_OCP    						0x0B

#define        REG_FRF_MSB    					0x06
#define        REG_FRF_MID    					0x07
#define        REG_FRF_LSB    					0x08
#define        REG_PA_CONFIG    				0x09
#define        REG_PAYLOAD_LENGTH_LORA		 	0x22
#define        REG_FIFO_ADDR_PTR  				0x0D
#define        REG_FIFO        					0x00
#define        REG_IRQ_FLAGS	    			0x12
#define        REG_PA_RAMP    					0x0A
#define        REG_LNA    						0x0C
#define LNA_MAX_GAIN                0x23
#define        REG_SYMB_TIMEOUT_LSB  			0x1F
#define        REG_FIFO_RX_BYTE_ADDR 			0x25
#define        REG_RX_NB_BYTES	    			0x13
#define        REG_MODEM_CONFIG1	 		 	0x1D
#define        REG_MODEM_CONFIG2	  			0x1E
#define        REG_MODEM_CONFIG3	  			0x26
#define        REG_DETECTION_THRESHOLD          0x37
#define        REG_DETECT_OPTIMIZE              0x31
#define        REG_PREAMBLE_MSB_LORA  			0x20
#define        REG_PREAMBLE_LSB_LORA  			0x21
#define        REG_SYNC_WORD                    0x39

#define        REG_VERSION	  					0x42

const uint32_t CH_10_868 = 0xD84CCC; // channel 10, central freq = 865.20MHz

//LORA MODES:
const uint8_t LORA_SLEEP_MODE = 0x80;
const uint8_t LORA_STANDBY_MODE = 0x81;
const uint8_t LORA_TX_MODE = 0x83;
const uint8_t LORA_RX_SINGLE_MODE = 0x86;
const uint8_t LORA_RX_CONT_MODE = 0x85;
const uint8_t LORA_RX_MODE = 0x85;

const uint8_t FSK_SLEEP_MODE = 0x00;

//LORA BANDWIDTH:
const uint8_t BW_125 = 0x07;

//LORA CODING RATE:
const uint8_t CR_5 = 0x01;

//LORA SPREADING FACTOR:
const uint8_t SF_12 = 0x0C;


#define PKT_TYPE_DATA   0x10

#endif
