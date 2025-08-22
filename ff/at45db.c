/*************************************************************************************
*
*  Драйвер Atmel DataFlash
*
*  Совместимо с Strict ANSI C, MISRA-C 2004
*
*  by:      BSVi
*  version: 1.00
*
*************************************************************************************/

#include "at45db.h"
#include "xprintf.h"

//------------------------------------------------------------------------------------
//  Определение типа dataflash
//------------------------------------------------------------------------------------
//#ifdef DF_AUTODETECT_FEATURES

df_Info_t df_Info;

uint8_t crc_tx[16];
uint8_t crc_rx[16];

const df_Info_t df_table[] =
{
   { "AT45DB011 1M", 512,  264,  9,  (0x3<<2) }, // 1M  AT45DB011
   { "AT45DB021 2M", 1024, 264,  9,  (0x5<<2) }, // 2M  AT45DB021
   { "AT45DB041 4M", 2048, 264,  9,  (0x7<<2) }, // 4M  AT45DB041
   { "AT45DB081 8M", 4096, 264,  9,  (0x9<<2) }, // 8M  AT45DB081
   { "AT45DB161 16M", 4096, 528,  10, (0xB<<2) }, // 16M AT45DB161
   { "AT45DB321 32M", 8192, 528,  10, (0xD<<2) }, // 32M AT45DB321
   { "AT45DB642 64M", 8192, 1056, 11, (0xF<<2) }, // 64M AT45DB642
};

//------------------------------------------------------------------------------------
//  Коды комманд для внутреннего использования
//------------------------------------------------------------------------------------
#define FlashPageRead			0x52	// Main memory page read
#define ContArrayRead			0x68	// Continuous Array Read (Note : Only A/B/D-parts supported)
#define StatusReg			0x57	// Status register
#define Buf1Read			0x54	// Buffer 1 read
#define Buf2Read			0x56	// Buffer 2 read
#define Buf1Write			0x84	// Buffer 1 write
#define Buf2Write			0x87	// Buffer 2 write

#define DF_STATUS_READY 0x80

//------------------------------------------------------------------------------------
//  Проверяет, готова ли память
//------------------------------------------------------------------------------------
uint8_t df_isReady()
{
   return df_GetStatus() &  DF_STATUS_READY;
}


//------------------------------------------------------------------------------------
//  Читает байт состояния Dataflash. О содержимом этого байта можно узнать в
//  разделе STATUS REGISTER READ.
//------------------------------------------------------------------------------------
uint8_t df_GetStatus()
{
   uint8_t status;

   CS_DF=0;
   status = spi_df_RW(StatusReg);
   status = spi_df_RW(0x00);
   CS_DF=1;

   return status;
}

//------------------------------------------------------------------------------------
//  Обнаруживает тип dataflash
//------------------------------------------------------------------------------------
uint8_t chip_id;

uint8_t df_DetectParams(void)
{
    uint8_t i;
    chip_id = df_GetStatus() & 0x3C;

    for(i=0; i<sizeof(df_table); i++) {
        if(df_table[i].chip_id == chip_id) {
            df_Info = df_table[i];
            return(0);
            }
        }
    return(1);
}

//------------------------------------------------------------------------------------
//  Читает один байт из SRAM-буффера dataflash
//	Параметры: BufferNo -> Номер буффера (1 или 2)
//                  Addr  -> Адрес в буффере
//------------------------------------------------------------------------------------
uint8_t df_GetChar(uint8_t BufferNo, uint16_t Addr )
{
   uint8_t data;

   CS_DF=0;

   if ( BufferNo == 1 )	{ spi_df_RW(Buf1Read); }       // буффер 1
   else	if ( BufferNo == 2 ) { spi_df_RW(Buf2Read); }  // буффер 2
   else { CS_DF=1; return 0; }                      // ошиблись с номером буффера

   spi_df_RW(0x00);              // цикл чтения описан в даташите
   spi_df_RW((uint8_t)(Addr>>8));
   spi_df_RW((uint8_t)(Addr));
   spi_df_RW(0x00);
   data = spi_df_RW(0x00);

   CS_DF=1;

   return data;
}



//------------------------------------------------------------------------------------
//  Читает блок данных из SRAM-буффера dataflash
//	Параметры: BufferNo   -> Номер буффера (1 или 2)
//                  Addr       -> Адрес в буффере с которого начинать чтение
//                  Count      -> Количество байт, которое необходимо прочитать
//  		   *BufferPtr -> Адрес буффера в который заносить данные
//------------------------------------------------------------------------------------
void df_Read( uint8_t BufferNo, uint16_t Addr, uint16_t Count, uint8_t *BufferPtr )
{
    uint16_t i;
    int n=0;
    //
    CS_DF=0;

   if ( BufferNo == 1 )	{ spi_df_RW(Buf1Read); }       // буффер 1
   else	if ( BufferNo == 2 ) { spi_df_RW(Buf2Read); }  // буффер 2
   else { CS_DF=1; return; }                        // ошиблись с номером буффера

   spi_df_RW(0x00);
   spi_df_RW((unsigned char)(Addr>>8));
   spi_df_RW((unsigned char)(Addr));
   spi_df_RW(0x00);


    ///SPI_CalculateCRC(SPI1, ENABLE);
    spi_crc_on(SPI0);
    n=0;
    for(i=0; i<Count; i++) {
        *(BufferPtr) = spi_df_RW(0x00);
        BufferPtr++;
        //
        if(((i%32) == 0)&&(i != 0)) {
            ///crc_rx[n] = SPI_GetCRC(SPI1, SPI_CRC_Rx);
            crc_rx[n] = spi_crc_get(SPI0, 0x00);
            n++;
            n&=0x0f;
            }
        }
    ///crc_rx[n] = SPI_GetCRC(SPI1, SPI_CRC_Rx);
    crc_rx[n] = spi_crc_get(SPI0, 0x00);
    ///SPI_CalculateCRC(SPI1, DISABLE);
    spi_crc_off(SPI0);

    CS_DF=1;
}


//------------------------------------------------------------------------------------
//  Записывает байт в SRAM-буффер dataflash
//	Параметры: BufferNo   -> Номер буффера (1 или 2)
//                  Addr       -> Адрес в буффере с которого начинать чтение
//                  Data       -> Байт который нужно записать
//------------------------------------------------------------------------------------
void df_WriteByte( uint8_t BufferNo, uint16_t Addr, uint8_t Data )
{
   CS_DF=0;

   if ( BufferNo == 1 )	{ spi_df_RW(Buf1Write); }       // буффер 1
   else	if ( BufferNo == 2 ) { spi_df_RW(Buf2Write); }  // буффер 2
   else { CS_DF=1; return; }                         // ошиблись с номером буффера

   spi_df_RW(0x00);
   spi_df_RW((unsigned char)(Addr>>8));
   spi_df_RW((unsigned char)(Addr));
   spi_df_RW(Data);

   CS_DF=1;
}


//------------------------------------------------------------------------------------
//  Записывает блок данных в SRAM-буффер dataflash
//	Параметры: BufferNo   -> Номер буффера (1 или 2)
//                  Addr       -> Адрес в буффере с которого начинать чтение
//                  Count      -> Количество байт, которое необходимо записать
//  		   *BufferPtr -> Буффер, данные из которого нужно записать
//------------------------------------------------------------------------------------
void df_Write(uint8_t BufferNo, uint16_t Addr, uint16_t Count, uint8_t *BufferPtr)
{
    uint16_t i;
    int n=0;
    //
    CS_DF=0;

   if ( BufferNo == 1 )	{ spi_df_RW(Buf1Write); }       // буффер 1
   else	if ( BufferNo == 2 ) { spi_df_RW(Buf2Write); }  // буффер 2
   else { CS_DF=1; return; }                         // ошиблись с номером буффера

   spi_df_RW(0x00);
   spi_df_RW((unsigned char)(Addr>>8));
   spi_df_RW((unsigned char)(Addr));

    ///SPI_CalculateCRC(SPI1, ENABLE);
    spi_crc_on(SPI0);
    n=0;
    for(i=0; i<Count; i++) {
        //*(BufferPtr) = spi_df_RW(0x00);
        spi_df_RW(*(BufferPtr));
        BufferPtr++;
        //
        if(((i%32) == 0)&&(i != 0)) {
            ///crc_rx[n] = SPI_GetCRC(SPI1, SPI_CRC_Rx);
            crc_rx[n] = spi_crc_get(SPI0, 0x00);
            n++;
            n&=0x0f;
            }
        }
    //crc_rx[n] = SPI_GetCRC(SPI1, SPI_CRC_Rx);
    crc_rx[n] = spi_crc_get(SPI0, 0x00);
    //SPI_CalculateCRC(SPI1, DISABLE);
    spi_crc_off(SPI0);

    CS_DF=1;
}


//------------------------------------------------------------------------------------
//  Манипуляции со страницами
//	Параметры: PageСmd  -> Команда
//                  PageAdr  -> Номер страницы
//------------------------------------------------------------------------------------
void df_PageFunc( uint8_t PageCmd, uint16_t PageAdr )
{
   CS_DF=0;

   spi_df_RW(PageCmd);
   spi_df_RW((uint8_t)(PageAdr >> (16 - df_Info.page_bit)));
   spi_df_RW((uint8_t)(PageAdr << (df_Info.page_bit - 8)));
   spi_df_RW(0x00);

   CS_DF=1;

   while(!( df_GetStatus() & DF_STATUS_READY )); // Ожидаем завершения операции
}

//------------------------------------------------------------------------------------
//  Прочитать блок данных непосредственно из Flash - памяти
//	Параметры: PageAdr    -> Номер страницы, с которой начинать чтение
//                  Addr       -> Адрес в странице, с которого начинать чтение
//                  Count      -> Количество байт, которое необходимо прочитать
//  		   *BufferPtr -> Адрес буффера в который заносить данные
//------------------------------------------------------------------------------------
void df_FlashRead( uint16_t PageAdr, uint16_t Addr, uint16_t Count, uint8_t *BufferPtr )
{
    uint16_t i;
   CS_DF=0;

   spi_df_RW(ContArrayRead);
   spi_df_RW((unsigned char)(PageAdr >> (16 - df_Info.page_bit)));
   spi_df_RW((unsigned char)((PageAdr << (df_Info.page_bit - 8))+ (Addr>>8)));
   spi_df_RW((unsigned char)(Addr));
   spi_df_RW(0x00);
   spi_df_RW(0x00);
   spi_df_RW(0x00);
   spi_df_RW(0x00);

   for( i=0; i < Count; i++ )
   {
      *(BufferPtr) = spi_df_RW(0x00);
      BufferPtr++;
   }

   CS_DF=1;
}

static int is_inited=0;

uint8_t df_Init(void)
{
uint8_t res;

    if(!is_inited) {
        spi_df_Init();
        is_inited=1;
        }
    //
    res = df_DetectParams();
    //if(is_inited == 1) {
        //xprintf("df_Init: %s %u pages\n", df_Info.name, df_Info.pages);
        is_inited=2;
    //    }
    return(res);
}
