#ifndef _FILE_OP_DEFINED
#define _FILE_OP_DEFINED

void put_rc(FRESULT rc);
void ls(char *s);
void mkdir(char *s);
void rm(char *s);
void cd(char *s);
void cat(char *s);
void fdump(char *s);
void fs(char *s);
void mkfs(char* s);
void cp(char *s);
void touch(char *s);
void pwd(char *s);
void label(char* s);

#endif

