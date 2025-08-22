#include "gd32f10x.h"
#include "xprintf.h"
#include "flash.h"

void hex_dump(void);

void fmc_erase_page(int page)
{
    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* erase the flash pages */
    fmc_page_erase(FMC_START_ADDR + (FMC_PAGE_SIZE * page));
    //
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    /* lock the main FMC after the erase operation */
    fmc_lock();
}

/*!
    \brief      program fmc word by word from FMC_WRITE_START_ADDR to FMC_WRITE_END_ADDR
    \param[in]  none
    \param[out] none
    \retval     none
*/
void fmc_program(int page, uint32_t* pbf, int numb)
{
int i;
    /* unlock the flash program/erase controller */
    fmc_unlock();

    uint32_t address = FMC_START_ADDR + (FMC_PAGE_SIZE * page);

    /* program flash */
    for(i=0; i<numb; i++) {
        fmc_word_program(address, *pbf);
        pbf++;
        address += 4;
        //
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        }

    /* lock the main FMC after the program operation */
    fmc_lock();
}

void fmc_read(int page, uint32_t* pbf, int numb)
{
int i;
uint32_t *ptrd = (uint32_t *)(FMC_START_ADDR + (FMC_PAGE_SIZE * page));

    /* check flash whether has been programmed */
    for(i = 0; i < numb; i++) {
        *pbf++ = *ptrd++;
        }
}

int fmc_test_ff(int page)
{
uint32_t i;
uint32_t *ptrd = (uint32_t *)(FMC_START_ADDR + (FMC_PAGE_SIZE * page));
uint32_t data;
uint32_t numb = FMC_PAGE_SIZE;
int res=0;
    /* check flash whether has been programmed */
    for(i = 0; i < numb; i++) {
        data = *ptrd++;
        if(data != 0xffffffff) {
            res = -1;
            break;
            }
        }
    return(res);
}
