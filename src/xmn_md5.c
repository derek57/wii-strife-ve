//#include <pspdebug.h>
#include <stdio.h>
#include "md5.h"
#include "doomdef.h"

//#define printf pspDebugScreenPrintf

extern char		target[MAXPATH];

char calculated_md5_string[33];
char known_md5_string_strife_share_1_0_iwad[33] = "de2c8dcad7cca206292294bdab524292";
char known_md5_string_strife_share_1_1_iwad[33] = "bb545b9c4eca0ff92c14d466b3294023";
char known_md5_string_strife_reg_1_0_iwad[33] = "8f2d3a6a289f5d2f2f9c1eec02b47299";
char known_md5_string_strife_reg_1_2_iwad[33] = "2fed2031a5b03892106e0f117f17901f";
char known_md5_string_voices_iwad[33] = "082234d6a3f7086424856478b5aa9e95";

int MD5_Check(char *final)		// FOR PSP: THIS FUNCTION DEFINITELY WORKS, BUT IT WAS NEVER USED - MAYBE FUTURE
{
    int i;
    int bytes;
//    char *filename = target;
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char data[1024];

    FILE *inFile = fopen (final, "rb");

    MD5_CTX mdContext;

//    if (inFile == NULL)
//    {
//        printf("%s can't be opened.\n", filename);

//        return 0;
//    }

    MD5_Init(&mdContext);

    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);

    MD5_Final(c, &mdContext);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
	sprintf(&calculated_md5_string[i * 2], "%02x", (unsigned int)c[i]);
/*
    if(strncmp(calculated_md5_string, known_md5_string_hexen_1_1_iwad, 32) == 0)
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\nMD5 MATCH!\n");
    else
	printf("\nMD5 FAIL!\n");

    printf("%s\n", known_md5_string_hexen_1_1_iwad);

    printf("%s\n", calculated_md5_string);

    printf("%s\n", final);
*/
    fclose(inFile);

    return 0;
}

