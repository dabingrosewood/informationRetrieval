/*
 * Improved calculation of CRC-64 values for protein sequences
 * By Adam Lu 刘亚壮  - 2016-02-26
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* If you want to try the old CRC-64 function, currently employed in
   SWISSPROT/TrEMBL then uncomment the next line */
/* #define OLDCRC */

#ifdef OLDCRC
  #define POLY64REV	0xd800000000000000ULL
  #define INITIALCRC	0x0000000000000000ULL
#else
  #define POLY64REV     0x95AC9329AC4BC9B5ULL
  #define INITIALCRC    0xFFFFFFFFFFFFFFFFULL
#endif



void crc64(char *seq, char *res)
{
    int i, j, low, high;
    unsigned long long crc = INITIALCRC, part;
    static int init = 0;
    static unsigned long long CRCTable[256];
    
    if (!init)
    {
	init = 1;
	for (i = 0; i < 256; i++)
	{
	    part = i;
	    for (j = 0; j < 8; j++)
	    {
		if (part & 1)
		    part = (part >> 1) ^ POLY64REV;
		else
		    part >>= 1;
	    }
	    CRCTable[i] = part;
	}
    }
    
    while (*seq)
	crc = CRCTable[(crc ^ *seq++) & 0xff] ^ (crc >> 8);

    /* 
     The output is done in two parts to avoid problems with 
     architecture-dependent word order
     */
    low = crc & 0xffffffff;
    high = (crc >> 32) & 0xffffffff;
    sprintf (res, "%08X%08X", high, low);

    return;
}
