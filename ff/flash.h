#ifndef FLASH_H
#define FLASH_H

#define FMC_PAGE_SIZE          ((uint16_t)0x800U)
#define FMC_START_ADDR    ((uint32_t)0x08020000U)
#define FMC_END_ADDR      ((uint32_t)0x08080000U)

void fmc_erase_page(int page);
void fmc_program(int page, uint32_t* pbf, int numb);
void fmc_read(int page, uint32_t* pbf, int numb);
int fmc_compare(int page, uint32_t* pbf, int numb);
int fmc_test_ff(int page);

#endif
