#include "shell.h"

///#include "ssd1306.h"

void uid(void);

void sys_mount(void);
void init_console(void);

static int sh_quit=0;
char cur_path[32];

extern uint32_t SystemCoreClock;
xTaskHandle xHandle=NULL;

char*  m_args[16];
int    m_argc;
char** m_argv;

/*---------------------------------------------------------------------------*/
static void shell_quit(char *str)
{
    sh_quit=1;
}
/*---------------------------------------------------------------------------*/

const ptentry parsetab[] =
{
#ifdef CONFIG_SYS
    #include "sys_conf.h"
#endif

#ifdef CONFIG_FILE_OP
    #include "file_op_conf.h"
#endif

#ifdef CONFIG_DISK_OP
    #include "disk_op_conf.h"
#endif

#ifdef CONFIG_OTHER
    #include "other_conf.h"
#endif

#ifdef CONFIG_SD_CARD
    #include "sd_card_conf.h"
#endif

#ifdef CONFIG_ADC
    #include "adc_conf.h"
#endif

#ifdef CONFIG_NET
    #include "net_conf.h"
#endif

#ifdef CONFIG_DIO
    #include "d_io_conf.h"
#endif

#ifdef CONFIG_IO_TASK
    #include "io_task_conf.h"
#endif

#ifdef CONFIG_SENSORS
    #include "sens_conf.h"
#endif

#ifdef CONFIG_DAC
    #include "dac_op_conf.h"
#endif

#ifdef CONFIG_GLCD
    #include "glcd_op_conf.h"
#endif
    //
    {"exit", shell_quit, "Выход"},

    {NULL, NULL}
};

static int parse(char *str)
{
ptentry *p;
int i;
char cmd[16];
    //
    p = (ptentry*)parsetab;

    memset(cmd, 0, 16);
    for(i=0; i<15; i++) {
        if((str[i] == ' ')||(str[i] == '\0')) break;
        cmd[i] = str[i];
        }
    int l_cmd = strlen(cmd);
    int l_pt=0;
    //
    for(; p->commandstr != NULL; p++) {
        l_pt = strlen(p->commandstr);
        if(((strncmp(p->commandstr, cmd, l_cmd)) == 0)&&(l_cmd == l_pt)) {
            break;
            }
        }
    //
    if(p->commandstr == NULL) return(1);
    //
    if(p->task == 0) {
        //xprintf("str: %s\n", str);
        p->pfunc(str);
        }
    else {
        xTaskCreate( (void*)p->pfunc, (const char *)p->commandstr,  2*configMINIMAL_STACK_SIZE, NULL, 4, &xHandle);
        }
    //
    return(0);
}

///extern uint8_t s_SSD1306_Buffer[SSD1306_BUFFER_SIZE];
void mh_z14a(void* p);

static char cmd[64];
static char arg[64];

void shell(void* dummy)
{
(void) dummy;
int i;
    //
    init_console();
    //
	//====================================================================
    //
    vTaskDelay(100);
    //
    sys_mount();
    //
    memset(cur_path, 0, 32);
    strncpy(cur_path, "/", 32);
    f_chdrive("0:");
    //
	xputs("STM32F103 shel.\n");
	xprintf("\n");
	xputs(FF_USE_LFN ? "LFN Enabled" : "LFN Disabled");
	xprintf(", Code page: %u\n", FF_CODE_PAGE);
	//
	xprintf("\n");
	//
	uid();
	//
	xprintf("SystemCoreClock: %d\n", SystemCoreClock);
    //
    strncpy(cmd, "date", 64);
    date(cmd);
    //
    dac("dac 1500");
    /*
    glcd_init("");
    LCD_Clear();
    set_font(&font_M);
    g_marker(0,0);
    LCD_PutString("Привет Мир!\n");
    */
    /*
    PWM3_Init();
    */
    ///PWM_Init();
    ///adc_init("");
    sol_init("");
    sol("sol 10");
    //
    //
    //ssd1306_Init();
    //ssd1306_Fill(Black);
    //memset(s_SSD1306_Buffer, 0x55, sizeof(s_SSD1306_Buffer));
    //ssd1306_UpdateScreen();
    //
    xTaskCreate(mh_z14a, "mh_z14a",  configMINIMAL_STACK_SIZE*4, NULL, 1, (xTaskHandle*)NULL);
    //
    for(;;) {
        xprintf("%d:%s > ", cur_vol(), cur_path);
        memset(cmd, 0, 64);
        xgets(cmd, 63);
        //
        memset(arg, 0, 64);
        strncpy(arg, cmd, 64);
        //
        m_argv = m_args;
        //
        for(i=0; i<16; i++) m_argv[i]=NULL;
        //
        char* pt = arg;
        i=0;
        for(;;) {
            if(*pt == 0) break;
            else if(i==0) { m_argv[i] = pt; i++; pt++; }
            else if(*pt == ' ') {
                    *pt = '\0';
                    pt++;
                    if(*pt) {
                        m_argv[i] = pt;
                        if(i<15) i++;
                        else break;
                        }
                    else break;
                    }
            else pt++;
            }
        m_argc = i;
        if(m_argc > 8) m_argc=8;

        /*
        xprintf("cmd: %s\n", cmd);
        xprintf("\n");
        for(i=0; i<m_argc; i++) {
            xprintf("%d %s\n", i, m_argv[i]);
            }
        */

        xprintf("\n");

        if(cmd[0] != '\0') {
            if(parse(cmd) == 0) {}
            //
            else if(load(m_argv[0]) == 0) {
                xTaskHandle th=NULL;
                uint32_t *ps= (void*)0x2000c000;
                void (*p)(void* arg);
                p = (void*)*(ps);
                //
                //xprintf("start address: 0x%08x\n", p);
                //
                uint32_t* pa = (void*)0x2000c004;
                *pa++ = m_argc;

                for(i=0; i<m_argc; i++) {
                    *pa++ = (uint32_t)m_argv[i];
                    }
                //
                pa = (void*)(0x2000c000 + (10*4));
                if(*pa == 0x12345678) {
                    //
                    xTaskCreate( p, m_argv[0], configMINIMAL_STACK_SIZE*8, NULL, 2, &th);
                    //
                    if(!(*m_argv[m_argc-1] == '&')) {
                        do {
                            th = xTaskGetHandle(m_argv[0]);
                            vTaskDelay(100);
                            } while(th);
                        }
                    }
                else {
                    xprintf("%s this is not tasklet!\n", m_argv[0]);
                    }
                //
                xprintf("\n");
                }
            //
            else {
                xprintf("Uncnown command: %s.\n", cmd);
                }
            }
        //
        if(sh_quit) break;
        }
    vTaskDelete(NULL);
}

