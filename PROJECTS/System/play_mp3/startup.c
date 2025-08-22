#include <FreeRTOS.h>
#include "task.h"

#include <stdint.h>
#include <string.h>

void main(int argc, char* argv[]);

typedef void (*pFunc)(void*);
//
extern uint32_t _idata;
extern uint32_t _bdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
//
int argc;
char** argv;
//
void startup(void* s)
{
    s = s;
//
uint32_t *pulSrc;
uint32_t *pulDest;
    //
    for(pulDest = &_sbss; pulDest < &_ebss; ) {
        //xprintf("%d 0x%08x\n", n, pulDest);
        *(pulDest++) = 0;
        }
    // Copy the data segment initializers from flash to SRAM.
    //
    pulSrc  = (uint32_t *)(&_idata);
    pulDest = (uint32_t *)(&_bdata);
    //
    for(;pulDest < &_edata; ) {
        //
        *(pulDest++) = *(pulSrc++);
        }
//
    uint32_t *pa = (uint32_t*)0x08040004; // !!!
    argc = *pa++;
    argv = (void*)pa;
    //
    main(argc, argv);
    //
    vTaskDelete(NULL);
}

const pFunc __Vectors[] __attribute__((section("vectors"))) = {
        (pFunc)&startup,
        (void*)0x00000000, // argc
        (void*)0x00000000,  //
        (void*)0x00000000,  //
        (void*)0x00000000,  //
        (void*)0x00000000,  // arfv[0 ... 7]
        (void*)0x00000000,  //
        (void*)0x00000000,  //
        (void*)0x00000000,  //
        (void*)0x00000000,  //
        (void*)0x12345678, // key for .bin file
};

