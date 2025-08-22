#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"
//
#include "MX25L.h"

#define COMMAND_WREN  0x06
#define COMMAND_WRDI  0x04
#define COMMAND_RDSR  0x05
#define COMMAND_WRSR  0x01
#define COMMAND_READ  0x03
#define COMMAND_FREAD 0x0B
#define COMMAND_WRITE 0x02
#define COMMAND_RDID  0x9F
#define COMMAND_RUID  0x4B

#define COMMAND_SE    0x20
#define COMMAND_BE    0x52
#define COMMAND_CE    0x60

#define STATUS_WIP    0b00000001
#define STATUS_WEL    0b00000010
#define STATUS_BP0    0b00000100
#define STATUS_BP1    0b00001000
#define STATUS_BP2    0b00010000
#define STATUS_BP3    0b00100000
#define STATUS_RES    0b01000000
#define STATUS_SWRD   0b10000000

#define DUMMY         0xA5
#define DUMMY_BYTE    0xA5

#define SLAVESELECT (CS_DF=0)
#define SLAVEDESELECT (CS_DF=1)

byte SPI_Transfer(byte data)
{
    return(spi_df_RW(data));
}

uint32_t Get_Identification(w25qxx_t* desc)
{
uint32_t id=0;
//
	SLAVESELECT;
	SPI_Transfer(COMMAND_RDID);
	id = SPI_Transfer(DUMMY);
	id <<= 8; id |= SPI_Transfer(DUMMY);
	id <<= 8; id |= SPI_Transfer(DUMMY);
	SLAVEDESELECT;
	//
	desc->Lock = 1;
	//
	switch(id & 0x000000FF) {
        case 0x20: // 	w25q512
            desc->ID = W25Q512;
            desc->BlockCount = 1024;
            break;
        case 0x19: // 	w25q256
            desc->ID = W25Q256;
            desc->BlockCount = 512;
            break;
        case 0x18: // 	w25q128
            desc->ID = W25Q128;
            desc->BlockCount = 256;
            break;
        case 0x17: //	w25q64
            desc->ID = W25Q64;
            desc->BlockCount = 128;
            break;
        case 0x16: //	w25q32
            desc->ID = W25Q32;
            desc->BlockCount = 64;
            break;
        case 0x15: //	w25q16
            desc->ID = W25Q16;
            desc->BlockCount = 32;
            break;
        case 0x14: //	w25q80
            desc->ID = W25Q80;
            desc->BlockCount = 16;
            break;
        case 0x13: //	w25q40
            desc->ID = W25Q40;
            desc->BlockCount = 8;
            break;
        case 0x12: //	w25q20
            desc->ID = W25Q20;
            desc->BlockCount = 4;
            break;
        case 0x11: //	w25q10
            desc->ID = W25Q10;
            desc->BlockCount = 2;
            break;
        default:
            desc->ID = W25UNK;
            desc->BlockCount = 0;
            break;
        }
    //
    desc->SectorCount = desc->BlockCount * 16;
	desc->PageSize = 256;
	desc->SectorSize = 0x1000;
	desc->PageCount = (desc->SectorCount * desc->SectorSize) / desc->PageSize;
	desc->BlockSize = desc->SectorSize * 16;
	desc->CapacityInKiloByte = (desc->SectorCount * desc->SectorSize) / 1024;
	//
    desc->Lock = 0;
	return(id);
}

byte GetStatus()
{
	byte status;
	SLAVESELECT;
	SPI_Transfer(COMMAND_RDSR);
	status = SPI_Transfer(DUMMY);
	SLAVEDESELECT;
	return status;
}

void SetWriteEnable(bool enable)
{
	SLAVESELECT;
	SPI_Transfer(enable ? COMMAND_WREN : COMMAND_WRDI);
	SLAVEDESELECT;
}

void SetStatus(byte status)
{
	SetWriteEnable(true);
	SLAVESELECT;
	SPI_Transfer(COMMAND_WRSR);
	SPI_Transfer(status);
	SLAVEDESELECT;
}

void Erase(byte command, ulong address)
{
    command=command;
    address=address;
}

void SectorErase(ulong address)
{
	SetWriteEnable(true);
	SLAVESELECT;
	SPI_Transfer(COMMAND_SE);
	SetAddress(address);
	SLAVEDESELECT;
	while(GetStatus() & STATUS_WIP) vTaskDelay(1);
}

void BlockErase(ulong address)
{
	SetWriteEnable(true);
	SLAVESELECT;
	SPI_Transfer(COMMAND_BE);
	SetAddress(address);
	SLAVEDESELECT;
	while(GetStatus() & STATUS_WIP);
}

void ChipErase()
{
	SetWriteEnable(true);
	SLAVESELECT;
	SPI_Transfer(COMMAND_CE);
	SLAVEDESELECT;
	while(GetStatus() & STATUS_WIP);
}

void SetAddress(ulong address)
{
	SPI_Transfer( (byte) (address >> 16) );
	SPI_Transfer( (byte) (address >> 8) );
	SPI_Transfer( (byte) address );
}

void Write(uint32_t address, const uint8_t* data, uint32_t length)
{
	SetWriteEnable(true);
	SLAVESELECT;
	SPI_Transfer(COMMAND_WRITE);
	SetAddress(address);
	for(uint i = 0;i < length;i ++)
	{
		SPI_Transfer(*(data + i));
	}
	SLAVEDESELECT;
	while(GetStatus() & STATUS_WIP) vTaskDelay(1);
}

void Read(uint32_t address, uint8_t* data, uint32_t length)
{
	SLAVESELECT;
	SPI_Transfer(COMMAND_READ);
	SetAddress(address);
	for(uint i = 0;i < length;i ++)
	{
		*(data + i) = SPI_Transfer(DUMMY);
	}
	SLAVEDESELECT;
	while(GetStatus() & STATUS_WIP) vTaskDelay(1);
}

void FastRead(uint32_t address, uint8_t* data, uint32_t length)
{
	SLAVESELECT;
	SPI_Transfer(COMMAND_FREAD);
	SetAddress(address);
	SPI_Transfer(DUMMY);
	for(uint i = 0;i < length;i ++)
	{
		*(data + i) = SPI_Transfer(DUMMY);
	}
	SLAVEDESELECT;
	while(GetStatus() & STATUS_WIP) vTaskDelay(1);
}

void ProtectBlock(bool bp0, bool bp1, bool bp2, bool bp3)
{
	byte status = GetStatus();
	if(bp0) status |= STATUS_BP0;
	if(bp1) status |= STATUS_BP1;
	if(bp2) status |= STATUS_BP2;
	if(bp3) status |= STATUS_BP3;
	SetStatus(status);

}

void ReadUniqID(w25qxx_t* desc)
{
int i;
    desc->Lock = 1;
	SLAVESELECT;
	SPI_Transfer(COMMAND_RUID);
	for(i = 0; i < 4; i++)
		SPI_Transfer(DUMMY);
	for(i = 0; i < 8; i++)
		desc->UniqID[i] = SPI_Transfer(DUMMY);
	SLAVEDESELECT;
    desc->Lock = 0;
}

uint8_t ReadStatusRegister(w25qxx_t* desc, uint8_t SelectStatusRegister_1_2_3)
{
	uint8_t status = 0;

	if (SelectStatusRegister_1_2_3 == 1)
	{
		SPI_Transfer(0x05);
		status = SPI_Transfer(DUMMY);;
		desc->StatusRegister1 = status;
	}
	else if (SelectStatusRegister_1_2_3 == 2)
	{
		SPI_Transfer(0x35);
		status = SPI_Transfer(DUMMY);;
		desc->StatusRegister1 = status;
	}
	else
	{
		SPI_Transfer(0x15);
		status = SPI_Transfer(DUMMY);;
		desc->StatusRegister1 = status;
	}

	return status;
}
