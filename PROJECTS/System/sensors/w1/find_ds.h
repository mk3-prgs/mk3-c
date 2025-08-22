#ifndef FIND_DS_H_INCLUDED
#define FIND_DS_H_INCLUDED

#define FALSE 0
#define TRUE 1

// method declarations
int OWFirst();
int OWNext();
int OWVerify();
void OWTargetSetup(unsigned char family_code);
void OWFamilySkipSetup();

int OWReset();
void OWWriteByte(unsigned char byte_value);
void OWWriteBit(unsigned char bit_value);
unsigned char OWReadBit();

int OWSearch();
unsigned char docrc8(unsigned char value);

extern unsigned char ROM_NO[8];

#endif // FIND_DS_H_INCLUDED
