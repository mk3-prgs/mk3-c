#ifndef MX25L_H_
#define MX25L_H_

#include "spi_df.h"

#ifndef bool
	#define bool unsigned short
#endif
#ifndef byte
	#define byte unsigned char
#endif
#ifndef uint
	#define uint unsigned int
#endif
#ifndef ulong
	#define ulong unsigned long
#endif

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif

byte SPI_Transfer(byte data);
//
typedef enum
{
    W25Q10 = 0x40,
    W25Q20,
    W25Q40,
    W25Q80,
    W25Q16,
    W25Q32,
    W25Q64,
    W25Q128,
    W25Q256,
    W25Q512,
    W25UNK,
} W25QXX_ID_t;

typedef struct
{
    W25QXX_ID_t ID;
    uint8_t UniqID[8];
    uint16_t PageSize;
    uint32_t PageCount;
    uint32_t SectorSize;
    uint32_t SectorCount;
    uint32_t BlockSize;
    uint32_t BlockCount;
    uint32_t CapacityInKiloByte;
    uint8_t StatusRegister1;
    uint8_t StatusRegister2;
    uint8_t StatusRegister3;
    uint8_t Lock;
} w25qxx_t;

uint32_t Get_Identification(w25qxx_t* desc);
byte GetStatus();
void SetWriteEnable(bool enable);
void SetStatus(byte status);

void SectorErase(ulong address);
void BlockErase(ulong address);
void ChipErase();

void SetAddress(ulong address);
void Write(uint32_t address, const uint8_t* data, uint32_t length);
void Read(uint32_t address, uint8_t* data, uint32_t length);
void FastRead(uint32_t address, uint8_t* data, uint32_t length);
void ProtectBlock(bool bp0, bool bp1, bool bp2, bool bp3);

void ReadUniqID(w25qxx_t* desc);
uint8_t ReadStatusRegister(w25qxx_t* desc, uint8_t SelectStatusRegister_1_2_3);

#endif /* MX25L_H_ */
