/*
rom8x: a TI-8x ROM File Creator

Copyright (C) 2013 Peter Marheine
Copyright (C) 2005 Andree Chea

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void fclose_nn(FILE *f) {
    if (f)
        fclose(f);
}

//#include <assert.h>

/*
    Valid Calculator Models:
        -83PBE
        -83PSE
        -84PBE
        -84PSE
        -84CSE

    The first dump is the boot page on all calculators. The second dump (only
    on 84+ series calculators) is the USB code page.

    The default file names are in the format:
        D8[34][PC][BS]E[12].8xv
*/

//#define NDEBUG

#define FILE_NAME_LENGTH 128     //128 bytes maximum?
typedef enum {
    false = 0,
    true
} bool;

// Size of ROM page (16k)
#define PAGE_SIZE 0x4000
#define NONFLASH 1
#define FLASH 2

const char usage[] = "USAGE: rom8x calcType [-1 fileName [-2 fileName]] -u fileName\n"
    "where:\n"
    "      calcType:  calculator model (83PBE|83PSE|84PBE|84PSE|84CSE)\n"
    "   -1 fileName:  file name of first dump\n"
    "   -2 fileName:  file name of second dump\n"
    "   -u fileName:  OS upgrade file option\n";

static const struct calc {
    // calcType parameter
    const char shortName[6];
    // Friendly name
    const char *const longName;
    // Number of ROM pages
    const unsigned numPages;
    // Has USB (needs two pages dumped)
    const bool hasUSB;
} calcTypes[5] = {
    { "83PBE", "TI-83 Plus", 0x20, false },
    { "83PSE", "Ti-83 Plus Silver Edition", 0x80, false },
    { "84PBE", "TI-84 Plus", 0x40, true },
    { "84PSE", "TI-84 Plus Silver Edition", 0x80, true },
    { "84CSE", "TI-84 Plus Color Silver Edition", 0x100, true }
};

const struct calc *determineTarget(const char *opt, int *calcModel);
int validHeader(FILE*, int, int);            //input file, type (see comments below)
int makeBlankRom(FILE*, int);                //ROM file, number of pages
int writePage(FILE*, FILE*, int);            //ROM file, dump file, page #
int write8xu(FILE*, FILE*, int);             //ROM file, OS file, calcModel
int GetOSVersion(FILE*, int*, int*);         //OS file, v_major, v_minor
void ExitHandler(void);

//The following were only referenced in main()
// -- if anywhere else, could cause global/local issues (mainly parameters)
//moved here because of ExitHandler(void)
int romCreated;                //flag if the ROM file was created
char fileNameRom[FILE_NAME_LENGTH];
// Output file
FILE *romFile;
// Input files
FILE *fileDump1 = NULL;
FILE *fileDump2 = NULL;
FILE *file8xu = NULL;

int main(int argc, char* argv[])
{
    const struct calc *targetCalc = NULL;

    char *fileNameDump1 = NULL;
    char *fileNameDump2 = NULL;
    const char *fileName8xu = NULL;
    int calcModel;            //number of calculator (like _GetHardwareVersion)
    int fD1, fD2, f8xu;        //flags if the command line arguments were present and valid
    int v_major, v_minor;

    int count;

    //if too few arguments or arguments (excluding calcType) are not an even count
    //since every switch precedes a file name
    if (argc <= 1 || ((argc - 2) % 2 != 0))    //subtraction is redundant
    {
        fprintf(stderr,usage);
        exit(EXIT_FAILURE);
    }

    //now loop through command-line arguments
    //printf("Parsing arguments...\n");

    targetCalc = determineTarget(argv[1], &calcModel);
    if (targetCalc == NULL) {
        fprintf(stderr, "%s is not a supported calculator.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    //check if file names available or if 8xu file specified
    fD1 = fD2 = f8xu = false;
    count = 2;
    while (count < argc)
    {
        if (argv[count][0] == '-')
        {
            if ((strlen(argv[count]) == 2) && (count + 1 < argc))
            {
                switch (argv[count][1])
                {
                    case '1':
                        fD1 = true;
                        count++;            //increment to next argument
                        fileNameDump1 = argv[count];
                        break;
                    case '2':
                        fD2 = true;
                        count++;
                        fileNameDump2 = argv[count];
                        break;
                    case 'u':
                        f8xu = true;
                        count++;
                        fileName8xu = argv[count];
                        break;
                    default:
                        fprintf(stderr,"%s: invalid switch.\n",argv[count]);
                        break;
                }
            }
            else
            {
                fprintf(stderr,"%s: not enough parameters.\n",argv[count]);
            }
        }
        else
        {
            fprintf(stderr,"%s: unknown option.\n",argv[count]);
        }
        count++;
    }

    if (!f8xu) {
        fprintf(stderr, "Upgrade file (-u *.8xu) must be specified.\n");
        fprintf(stderr, usage);
        exit(EXIT_FAILURE);    
    }

    //fD1, fD2, and f8xu determine if there was a command-line switch available.
    //    next, they will determine if the files were successfully opened, but not now.
    //if fD1 or fD2 were not specified, set the default file name
    //(maybe) also check to see if the correct # of files were specified

    size_t n_default = strlen(targetCalc->shortName) + 5 + 1;
    if (!fD1)        //set default file name for #1
    {
        fileNameDump1 = malloc(n_default);
        snprintf(fileNameDump1, n_default, "D%s1.8xv", targetCalc->shortName);
    }
    if (!fD2)        //set default file name for #2
    {
        fileNameDump2 = malloc(n_default);
        snprintf(fileNameDump2, n_default, "D%s2.8xv", targetCalc->shortName);
    }

    fputs("== PARAMETERS ==", stderr);
    fprintf(stderr,"Calculator model: %s\n"
                   "Dump 1: %s\n",
                   targetCalc->longName, fileNameDump1);
    if (targetCalc->hasUSB)
        fprintf(stderr,"Dump 2: %s\n",fileNameDump2);
    fprintf(stderr,"Upgrade file: %s\n",fileName8xu);
    fprintf(stderr,"\n");

    //Now open files (and check if the right files exist for the right calculator)
    fileDump1 = fopen(fileNameDump1,"rb");    //open in reading and binary
    if (!fileDump1) {
        perror(fileNameDump1);
        exit(EXIT_FAILURE);
    } else if (!validHeader(fileDump1, NONFLASH, calcModel)) {
        fprintf(stderr,"%s: invalid dump file.\n",fileNameDump1);
        exit(EXIT_FAILURE);
    }

    if (targetCalc->hasUSB)
    {
        fileDump2 = fopen(fileNameDump2,"rb");
        if (!fileDump2) {
            perror(fileNameDump2);
            exit(EXIT_FAILURE);
        } else if (!validHeader(fileDump2, NONFLASH, calcModel)) {
            fprintf(stderr,"%s: invalid dump file.\n",fileNameDump2);
            exit(EXIT_FAILURE);
        }
    }

    file8xu = fopen(fileName8xu,"rb");
    if (!file8xu) {
        perror(fileName8xu);
        exit(EXIT_FAILURE);
    } else if (!validHeader(file8xu, FLASH, calcModel)) {
        fprintf(stderr,"%s: invalid OS file.\n",fileName8xu);
        exit(EXIT_FAILURE);
    }

    //and now the ROM file itself, but first initialize the name
    GetOSVersion(file8xu, &v_major, &v_minor);
    snprintf(fileNameRom, sizeof(fileNameRom), "%s_v%u%u.rom", targetCalc->shortName, v_major, v_minor);

    romFile = fopen(fileNameRom,"w+b");
    if (!romFile)
    {
        perror(fileNameRom);
        exit(EXIT_FAILURE);
    }

    romCreated = false;
    //Let's make the ROM file!
    if (!makeBlankRom(romFile, targetCalc->numPages))
    {
        fprintf(stderr,"%s: unable to make blank ROM file.\n",fileNameRom);
        exit(EXIT_FAILURE);
    }

    //Page [137F]F
    if (!writePage(romFile, fileDump1, targetCalc->numPages - 1))
    {
        //perror(fileNameDump2);
        fprintf(stderr,"%s: unable to write %s to ROM file.\n",fileNameRom, fileNameDump1);
        exit(EXIT_FAILURE);
    }
    //Page [26E]F
    if (targetCalc->hasUSB)
    {
        if (!writePage(romFile, fileDump2, targetCalc->numPages - 0x11))
        {
            fprintf(stderr,"%s: unable to write %s to ROM file.\n",fileNameRom, fileNameDump2);
            exit(EXIT_FAILURE);
        }
    }
    // Contents of OS file
    if (!write8xu(romFile, file8xu, calcModel))
    {
        fprintf(stderr,"%s: unable to write %s to ROM file.\n",fileNameRom, fileName8xu);
        exit(EXIT_FAILURE);
    }
    // Mark OS as valid
    fseek(romFile, 0x56, SEEK_SET);
    fputc(0x5A, romFile);
    fputc(0xA5, romFile);

    fseek(romFile,((targetCalc->numPages - 2) * PAGE_SIZE) + 0x1FE0, SEEK_SET);
    fputc(0, romFile);

    romCreated = true;        //Success!
    printf("Created %s successfully\n", fileNameRom);

    return (romCreated ? EXIT_SUCCESS : EXIT_FAILURE);
}

const struct calc *determineTarget(const char *opt, int *calcModel) {
    const unsigned NCALCS = sizeof(calcTypes) / sizeof(calcTypes[0]);
    unsigned i;

    for (i = 0; i < NCALCS; i++) {
        if (strcmp(opt, calcTypes[i].shortName) == 0) {
            *calcModel = i;
            return &calcTypes[i];
        }
    }
    return NULL;
}

int validHeader(FILE *inFile, int type, int calcModel)
{
    //calcModel only needed for FLASH
    //type:
    //  NONFLASH for Dump file (.8xp/8xv)
    //  FLASH for OS file (.8xu)
    //returns:
    //  true or false if validHeader
    //  inFile pointer is set to beginning of file

    int count;
    //int valid = true;
    char buffer[0x4D];        //input header size
    char tempName[9];        //a temporary string
    int bufferSize;

    //int v_major, v_minor;    //local variables!
    int certID;

    if (fseek(inFile, 0, SEEK_SET))
        return false;        //BAD FILE!

    bufferSize = (type == NONFLASH)? 0x37 + 0x11 : 0x4D;        //whee.. magic numbers :D
    fread(buffer, 1, bufferSize, inFile);

    for (count = 0; count < 8; count++)
        tempName[count] = buffer[count];
    tempName[count] = '\0';

    if (type == NONFLASH)
    {
        if (strcmp(tempName, "**TI83F*") != 0)
        {
            //printf("Not valid signature\n");
            return false;
        }
        //Lengths == 0x4005? / first byte of header == 0x0D?
        if ((buffer[0x37] != 0x0D) || /*(buffer[0x39] != 0x05) || (buffer[0x3A] != 0x40) || */(buffer[0x39] != buffer[0x46]) || (buffer[0x3A] != buffer[0x47]))
        {
            //printf("%X %X%X != %X%X\n",buffer[0x37], buffer[0x3A], buffer[0x39], buffer[0x47], buffer[0x46]);
            return false;
        }
    }
    else if (type == FLASH)
    {
        if (strcmp(tempName, "**TIFL**") != 0)        //valid signature?
            return false;
        for (count = 0x11; count < 0x11 + 8; count++)
            tempName[count-0x11] = buffer[count];
        tempName[count-0x11] = '\0';
        if (strcmp(tempName, "basecode") != 0)        //OS file?
            return false;
        if ((buffer[0x30] != 0x73) || (buffer[0x31] != 0x23))    //83+ and OS file?
            return false;

        //Now check if 84+ OS specified for 84+ ROM and vice versa
        fseek(inFile, 0x68, SEEK_SET);        //another magic number! :D
        fscanf(inFile, "%1x", &certID);
        if ((certID == 0x0A) && (calcModel < 2))    //if 84+ OS for 83+ ROM
            return false;
        if ((certID == 0x04) && (calcModel > 1))    //if 83+ OS for 84+ ROM
            return false;
    }

    //return valid
    return (!fseek(inFile, 0, SEEK_SET));
}

int makeBlankRom(FILE *romFile, int numPages)
{
    //writes a blank ROM file
    //assume fseek() at beginning

    unsigned char blankPage[PAGE_SIZE];
    int count;

    for (count = 0; count < PAGE_SIZE; count++)
        blankPage[count] = 0xFF;                //blank

    for (count = 0; count < numPages; count++)
    {
        if (fwrite(blankPage, 1, PAGE_SIZE, romFile) != PAGE_SIZE)
            return false;
    }
    return true;
}

int writePage(FILE *romFile, FILE *dumpFile, int pageNum)
{
    //writes the specified page to romFile
    //automatically fseeks()
    void *buf = NULL;
    int out = false;

    if (fseek(romFile, pageNum * PAGE_SIZE, SEEK_SET) != 0)
        return false;

    // Skip 8xv header in dump file
    if (fseek(dumpFile, 0x48 + 2, SEEK_SET) != 0)
        return false;

    buf = malloc(PAGE_SIZE);
    if (fread(buf, PAGE_SIZE, 1, dumpFile) != 1)
        goto out;
    if (fwrite(buf, PAGE_SIZE, 1, romFile) != 1)
        goto out;
    out = true;

out:
    free(buf);
    return out;
}

int write8xu(FILE *romFile, FILE *file8xu, int calcModel)
{
    //integrates the OS into the romFile
    //most of the following code is from rompatch by Benjamin Moody
    
    char c;
    int nbytes, rectype, dl, dh, check, a;
    int pagenum=0;
    long position;
    int j;
    //int bp=0;
    int semode=0;                        //be sure to init to 0
    //char ibuf[0x4d];
    int parts_remaining/* = 1*/;
    
    fseek(romFile, 0, SEEK_SET);        //reset romFile to beginning ('reset' previous sessions)
    //the header is ignored since it'll get overwritten anyway
    fseek(file8xu, 0x4E, SEEK_SET);        //magic numbers ;)
    
    //if (ibuf[0x30] == 0x23)        //if OS file
        parts_remaining = 2;
    
    if (calcModel == 1 || calcModel == 3)        //if Silver Edition
        semode = 0x60;                        //OR mask
    else if (calcModel == 2)                //if 84PBE
        semode = 0x20;
    else if (calcModel == 4)
        semode = 0xe0;
//     else
//         semode = 0;

    c=fgetc(file8xu);

    while (!feof(file8xu) && !ferror(file8xu) && parts_remaining)
    {
        if (!feof(file8xu) && !ferror(file8xu))
        {
            if (c != ':')
            {
                //fprintf(stderr,"%s: hex file %s not in valid Intel format",
                //argv[0],argv[i]);
                return false;
            }

            fscanf(file8xu,"%2X%2X%2X%2X",&nbytes,&dh,&dl,&rectype);        //read 'header' data
            check=nbytes+dh+dl+rectype;            //add to checksum

            if (rectype == 0)
            {
                position = (pagenum * 0x4000l) + (((dh*0x100)+dl) & 0x3fff);
                if (fseek(romFile, position, SEEK_SET) != 0)
                {
                    perror("fseeking on ROM file");
                    return false;
                }
            }

            for (j = 0; j < nbytes; j++)
            {
                fscanf(file8xu,"%2X",&a);        //read a byte of the data
                check+=a;                        //add to checksum
                if (rectype == 0)
                    fputc(a,romFile);            //if just data record, write it to the ROM file
                else if (rectype == 2)            //else if TI-specific record
                {
                    if (semode)
                    {
                        if ((a > 0x0F) && (a < 0x20))
                            pagenum = a | semode;    //add the OR mask
                        else
                            pagenum = a;
                    }
//                     else if (bp<0)
//                         pagenum=(-bp)-a;
                    else
                        pagenum=/*bp+*/a;
                }
            }

            if (rectype == 1)        //if we reached an 'end' record
                parts_remaining--;

            fscanf(file8xu,"%2X",&a);    //checksum byte
            //printf("Calc and Checksum: %i %i\n",check,a);
            check+=a;                    //not sure how this works, but it does :D

            if (check & 0xFF)        //if bad checksum
            {
//                 fprintf(stderr,
//                 "%s: invalid checksum %X (at %02X%02X, type %02X) in %s\n",
//                 argv[0],check,dh,dl,rectype,argv[i]);
                return false;
            }
        }
        
        //get rid of whitespace characters
        do
            c=fgetc(file8xu);
        while ((c == '\n') || (c == '\r') || (c == '\t') || (c == '\f') || (c == ' '));
    }

    return true;
}

int GetOSVersion(FILE *file8xu, int *v_major, int *v_minor)
{
    //assuming the OS version is at specific offset (may cause future problems!)
    //return *file8xu fseek()ed to beginning

    // Magic version number offset
    fseek(file8xu, 0x6D, SEEK_SET);
    fscanf(file8xu, "%2x", v_major);

    // ...
    fseek(file8xu, 0x73, SEEK_SET);
    fscanf(file8xu, "%2x", v_minor);

    return (!fseek(file8xu, 0, SEEK_SET));        //set to beginning
}

void ExitHandler(void)
{
    // Close all files
    fclose_nn(fileDump1);
    fclose_nn(fileDump2);
    fclose_nn(file8xu);
    fclose_nn(romFile);

    if (!romCreated) {
        fclose(romFile);
        remove(fileNameRom);
        fprintf(stderr,"%s was not created.\n",fileNameRom);
    } else
        fprintf(stderr,"%s was successfully created.\n",fileNameRom);
}
