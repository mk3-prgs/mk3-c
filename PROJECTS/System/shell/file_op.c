#include "shell.h"

/// =================================== ff =============================================
extern char cur_path[];

void put_rc (FRESULT rc)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0" "INVALID_PARAMETER\0";
	FRESULT i;

	for(i = 0; i != rc && *str; i++) {
		while (*str++) ;
        }

	if(rc) xprintf("rc=%u FR_%s\n", (UINT)rc, str);
}

const char *mon_name[] =
{
"???",
"янв", "фев", "мар",
"апр", "май", "июн",
"июл", "авг", "сен",
"окт", "ноя", "дек",
"???"
};

void ls(char *s)
{
char *path=NULL;
FILINFO Finfo;
FATFS *f_s;
DIR Dir;
int res;
int p1,s1,s2;
    //
    if(strlen(s)>3) {
        path = &s[3];
        }
    else {
        path = cur_path;
        }
    //
    ///xprintf("PATH=%s\r\n", path);
    //
    res = f_opendir(&Dir, path);
    if(res) {
        put_rc(res);
        return;
        }

    p1 = s1 = s2 = 0;
    for(;;) {
        res = f_readdir(&Dir, &Finfo);
        if((res != FR_OK) || !Finfo.fname[0]) break;

        if(Finfo.fattrib & AM_DIR) {
            s2++;
            }
        else {
            s1++; p1 += Finfo.fsize;
            }

        xprintf("%c%c%c%c%c %02u:%02u %02u %s %u",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-',

                (Finfo.ftime >> 11),     // hh
                (Finfo.ftime >> 5) & 63, // mm

                Finfo.fdate & 31,
                mon_name[(Finfo.fdate >> 5) & 15],
                (Finfo.fdate >> 9) + 1980);

        xprintf(" %9lu  %s\n",
                Finfo.fsize,
                Finfo.fname
                );
        }
    xprintf("\n%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    res = f_getfree(path, (DWORD*)&p1, &f_s);
    if (res == FR_OK)
        xprintf(", %10lu bytes free\n", p1 * f_s->csize * 512);
    else
        put_rc(res);
    //
    res = f_closedir(&Dir);
    if(res) { put_rc(res); return; }
}

void mkdir(char *s)
{
    put_rc(f_mkdir(&s[6]));
}

void rm(char *s)
{
char *pts = &s[3];
    //
    int res = f_unlink(pts);
    put_rc(res);
}

void cd(char *s)
{
char *ptr = &s[3];
char *ps=NULL;
//
    if(strlen(s) >= 3) {
        ptr = &s[3];
        ps = strchr(ptr, ':');
        if(ps) {
            f_chdrive(ptr);
            ptr=ps+1;
            }
        memset(cur_path, 0, 32);
        if(strlen(ptr) > 30) *(ptr+30) = '\0';
        if(*ptr) {
            if(*ptr != '/') xsprintf(cur_path, "/%s", ptr);
            else xsprintf(cur_path, "%s", ptr);
            }
        else xsprintf(cur_path, "/");
        //
        f_chdir(cur_path);
        }
}

void cat(char *s)
{
char *ptr = &s[4];
int res;
UINT cnt;
uint8_t c;
FIL file;
FIL *fp=&file;
    //
    res = f_open(fp, ptr, FA_READ);
    if(res) { put_rc(res); return; }
    //
    while(1) {
        res = f_read(fp, &c, 1, &cnt);
        if(res) { put_rc(res); break; }
        else {
            if(cnt>0) {
                xputc(c);
                }
            else break;
            }
        }
    //
    xprintf("\n");
    f_close(fp);
    fp=NULL;
}

void fdump(char *s)
{
FIL file;
FIL *fp=&file;
int32_t cnt=0;
int res=0;

char *ptr;
int ofs=0;

    uint32_t *Buff = (uint32_t*)pvPortMalloc(512);
    if(Buff) {
        //
        res=f_open(fp, &s[6], FA_READ);
        if(res) {
            put_rc(res);
            goto err;
            }

        ofs = 0;
        for(;;) {
            memset(Buff, 0, 512);
            res = f_read(fp, Buff, 512, (UINT*)&cnt);
            if(res) {
                put_rc(res);
                break;
                }
            if(cnt == 0) break;

            for(ptr=(char*)Buff; cnt > 0; ptr += 16, ofs += 16) {
                int l = 16;
                if(cnt<16) l=cnt;
                put_dump((BYTE*)ptr, ofs, l, DW_CHAR);
                cnt -= 16;
                }
            }
    err:;
        if(fp) { f_close(fp); }
        if(Buff) vPortFree(Buff);
        }
    else xprintf("Мало памяти!\r\n");
}

//
DWORD AccSize;				/* Work register for fs command */
WORD  AccFiles, AccDirs;
FILINFO Finfo;

static FRESULT scan_files(char* path) //		/* Pointer to the path name working buffer */
{
DIR dirs;
FRESULT res;
BYTE i;
    //
	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = strlen(path);
		while(((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
			if (Finfo.fattrib & AM_DIR) {
				AccDirs++;
				*(path+i) = '/';
				strcpy(path+i+1, Finfo.fname);
				res = scan_files(path);
				*(path+i) = '\0';
				if(res != FR_OK) break;
                }
			else {
			/*	xprintf("%s/%s\n", path, fn); */
				AccFiles++;
				AccSize += Finfo.fsize;
                }
            }
        }
	return res;
}


void fs(char *s) // Show volume status
{
static const char *ft[] = {"FAT", "FAT12", "FAT16", "FAT32", "exFAT"};
FATFS fat;
FATFS *fs = &fat;
DWORD dw1;
DWORD dw2;
//
char* path = &s[3];
//
    while (*path == ' ') path++;

    int res = f_getfree(path, (DWORD*)&dw1, &fs);
    if(res) { put_rc(res); return; }
    else {
        xprintf("FAT type = %s\n", ft[fs->fs_type]);
        xprintf("Bytes/Cluster = %lu\n", (DWORD)fs->csize * 512);
        xprintf("Number of FATs = %u\n", fs->n_fats);
        if (fs->fs_type < FS_FAT32) xprintf("Root DIR entries = %u\n", fs->n_rootdir);
        xprintf("Sectors/FAT = %lu\n", fs->fsize);
        xprintf("Number of clusters = %lu\n", (DWORD)fs->n_fatent - 2);
        xprintf("Volume start (lba) = %lu\n", fs->volbase);
        xprintf("FAT start (lba) = %lu\n", fs->fatbase);
        xprintf("FDIR start (lba,clustor) = %lu\n", fs->dirbase);
        xprintf("Data start (lba) = %lu\n\n", fs->database);
#if FF_USE_LABEL
        char *Buff = (char*)pvPortMalloc(4096);
        res = f_getlabel(path, (char*)Buff, (DWORD*)&dw2);
        if (res) { put_rc(res);}
        else {
            xprintf(Buff[0] ? "Volume name is %s\n" : "No volume label\n", (char*)Buff);
            xprintf("Volume S/N is %04X-%04X\n", (DWORD)dw2 >> 16, (DWORD)dw2 & 0xFFFF);
            }
        if(Buff) vPortFree(Buff);
#endif
        AccSize = AccFiles = AccDirs = 0;
        xprintf("...\n");
        res = scan_files(path);
        if(res) {
            put_rc(res);
            return;
            }
        xprintf("%u files, %lu bytes.\n%u folders.\n",
                AccFiles, AccSize, AccDirs);

        uint32_t p1;
        res = f_getfree(path, (DWORD*)&p1, &fs);
        if(res == FR_OK)
            xprintf("%lu bytes free\n", p1 * fs->csize * 512);
        else
            put_rc(res);
        /*
        xprintf("%lu KiB total disk space.\n%lu KiB available.\n",
                (fs->n_fatent - 2), // * (fs->csize / 2), // total
                (DWORD)dw1 // * (fs->csize / 2)          // available
                );
        */
        }
}

void label(char* s)
{
    f_setlabel(&s[6]);
}

static const char mkfs_help[]=
{
"USAGE: mkfs vol fs\r\n"
        "   vol:\r\n"
        "       0 - sd\n"
        "       1 - sf\n"
        "\n"
        "   fs:\n"
        "       FAT    0x01\n"
        "       FAT32  0x02\n"
        "       EXFAT  0x04\n"
        "       ANY    0x07\n"
        "       SFD    0x08\n"
};

void mkfs(char* s)
{
MKFS_PARM f;
MKFS_PARM* opt = &f;
//uint8_t *pbf=NULL;
long t;
BYTE vol=0;
//uint8_t fs=0x01;
char str[8];
char* ptr=&s[5];

    f.fmt = FM_FAT;     // Format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD)
    f.n_fat = 1;        // Number of FATs
    f.align = 0;        // Data area alignment (sector)
    f.n_root = 0;       // Number of root directory entries
    f.au_size = 512;    // Cluster size (byte)
    //
    if(xatoi(&ptr, &t)) {
        vol = (BYTE)t;
        if(xatoi(&ptr, &t)) {
            f.fmt = (BYTE)t;
            }
        //else {
        //    xprintf(mkfs_help);
        //    return;
        //    }
        }
    else {
        xprintf(mkfs_help);
        return;
        }

    xprintf("The volume %d: will be formatted of 0x%02x\n", vol, fs);
    ///xprintf("Are you sure? (Y/n)=");
    ///xgets(str, sizeof str);
    ///xprintf("\n");
    str[0] = 'Y';
    if((str[0] == 'Y')||(str[0] == 'y')) {
        xsprintf(str, "%d:", vol);
        //f.fmt = fs;
        uint8_t *pbf = (uint8_t*)pvPortMalloc(512);
        if(pbf) {
            int res = f_mkfs(str,
                        opt,
                        pbf,
                        512
                        );
            put_rc(res);
            //
            vPortFree(pbf);
            }
        else {
            xprintf("Мало памяти!\n");
            }
        }
    else xprintf("Форматирование отменено!\n");
}

void cp(char *s)
{
int res;
FIL s_file;
FIL d_file;
FIL* fs=&s_file;
FIL* fd=&d_file;
uint8_t *p_bf;
char name[64];
char *src = &name[0];
char *dst = NULL;
uint32_t cnt_s;
uint32_t cnt_d;
int i;
//
    memset(name, 0, 64);
    char c;
    for(i=0; i<63; i++) {
        c = s[i+3];
        if(c) {
            name[i] = c;
            if(c==' ') {
                name[i] = '\0';
                if(dst==NULL) dst = &name[i+1];
                }
            }
        else break;
        }
    //
    xprintf("src:[%s] dst:[%s]\n", src, dst);
    //
    res = f_open(fs, src, FA_OPEN_EXISTING | FA_READ);
    if(res) {
        xprintf("SRC: ");
        put_rc(res);
        return;
        }

    res = f_open(fd, dst, FA_CREATE_ALWAYS | FA_WRITE);
    if(res) {
        xprintf("DST: ");
        put_rc(res);
        f_close(fs);
        return;
        }

    p_bf = (uint8_t*)pvPortMalloc(512);
    if(p_bf != NULL) {
        for (;;) {
            res = f_read(fs, p_bf, 512, (UINT*)&cnt_s);
            if (res || cnt_s == 0) break;   /* error or eof */

            res = f_write(fd, p_bf, cnt_s,  (UINT*)&cnt_d);
            if (res || (cnt_d < cnt_s)) break;   /* error or disk full */
            }
        //
        f_close(fs);
        f_close(fd);
        //
        vPortFree(p_bf);
        }
}

const uint8_t text[]=
{
"Hello!\r\n"
};

void touch(char *s)
{
int res;
FIL file;
FIL* fp=&file;
char name[16];
char *dst = &name[0];
uint32_t cnt_d;
int i;
//
    memset(name, 0, 16);
    char c;
    for(i=0; i<15; i++) {
        c = s[i+6];
        if(c) {
            name[i] = c;
            }
        else break;
        }
    //
    xprintf("dst:[%s]\n", dst);
    //
    res = f_open(fp, dst, FA_CREATE_ALWAYS | FA_WRITE);
    if(res) {
        put_rc(res);
        return;
        }

    res = f_write(fp, text, strlen((char*)text),  (UINT*)&cnt_d);

    f_close(fp);
}

void pwd(char* s)
{
BYTE vol = cur_vol();
    xprintf("%d:%s\n", vol, cur_path);
}


