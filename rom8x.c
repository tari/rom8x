/*
	ROM File Creator
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
//#include <assert.h>

/*
	Valid Calculator Models:
		-83PBE
		-83PSE
		-84PBE
		-84PSE

	The first dump refers to the 84P(B|S)E USB code page and the second dump refers to the boot page

	The default file names are in the format:
		D8?P?E?.8xv
	where the first ? is 3 or 4, the second ? is B or S, and the third ? is 1 or 2

	... and I apologize in advance for my bloated code :|
*/

//#define NDEBUG

#define FILE_NAME_LENGTH 128	//128 bytes maximum?
#define CALCTYPE_MODELS 5		//the number of supported calc types
#define CALCTYPE_LENGTH 5		//the maximum length for calcType (not including NULL)
#define TRUE 1
#define FALSE 0
#define PAGE_SIZE 0x4000		//size of 1 page

#define NONFLASH 1
#define FLASH 2

const char usage[] = "USAGE: rom8x calcType [-1 fileName [-2 fileName]] [-u fileName]\n"
	"where:\n"
	"      calcType:  calculator model (83PBE|83PSE|84PBE|84PSE)\n"
	"   -1 fileName:  file name of first dump\n"
	"   -2 fileName:  file name of second dump\n"
	"   -u fileName:  OS upgrade file option\n";

const char calcTypeList[][CALCTYPE_LENGTH+1] = {		//a list of supported calcTypes
	"83PBE",		//0
	"83PSE",		//1
	"84PBE",		//2
	"84PSE",		//3
	"84CSE"			//4
}

const char calcTypeNames[][25] = {		//a list of names (TI-style)
	"TI-83 Plus",						//0
	"TI-83 Plus Silver Edition",		//1
	"TI-84 Plus",						//2
	"TI-84 Plus Silver Edition",		//3
	"TI-84 Plus Color Silver Edition"	//4
};

//Lookup arrays
const int NumPagesList[] = { 0x1F + 1, 0x7F + 1, 0x3F + 1, 0x7F + 1, 0xFF + 1 };
const char ModelNumber[] = "33444";
const char ModelType[] = "BSBSC";

int validModel(char [], char [], int*);		//calcType, argv[1], calcModel
int validHeader(FILE*, int, int);			//input file, type (see comments below)
int makeBlankRom(/*FILE**/, int);			//ROM file, number of pages
int writePage(/*FILE**/, FILE*, int);		//ROM file, dump file, page #
int write8xu(/*FILE**/, FILE*, int);		//ROM file, OS file, calcModel
int GetOSVersion(FILE*, int*, int*);		//OS file, v_major, v_minor
int toInt(char);							//converts numeric character to integer;
void ExitHandler(void);
void (*ExitHandlerPtr)(void);

//The following were only referenced in main()
// -- if anywhere else, could cause global/local issues (mainly parameters)
//moved here because of ExitHandler(void)
int romCreated;				//flag if the ROM file was created
char fileNameRom[FILE_NAME_LENGTH];
FILE *romFile;				//output

int main(int argc, char* argv[])
{
	FILE *fileDump1, *fileDump2, *file8xu;	//input file pointers
	char fileNameDump1[FILE_NAME_LENGTH], fileNameDump2[FILE_NAME_LENGTH], fileName8xu[FILE_NAME_LENGTH];
	char calcType[CALCTYPE_LENGTH+1];		//5 characters and NULL
	int calcModel;			//number of calculator (like _GetHardwareVersion)
	char calcModelNumber;	//either '3' or '4'
	char calcModelType;		//either 'B' or 'S'
	int numPages;			//number of pages for the calc model
	int fD1, fD2, f8xu;		//flags if the command line arguments were present and valid
	int v_major, v_minor;

	int count;
	char temp[5];			//for GetOSVersion stuff

	//if too few arguments or arguments (excluding calcType) are not an even count
	//since every switch precedes a file name
	if (argc <= 1 || ((argc - 2) % 2 != 0))	//subtraction is redundant
	{
		fprintf(stderr,usage);
		exit(EXIT_FAILURE);
	}

	//now loop through command-line arguments
	//printf("Parsing arguments...\n");

	if (!validModel(calcType, argv[1], &calcModel))		//if model was not detected
	{
		fprintf(stderr,"%s is not a supported model.\n",calcType);
		exit(EXIT_FAILURE);
	}

	//check if file names available or if 8xu file specified
	fD1 = fD2 = f8xu = FALSE;
	count = 2;
	while (count < argc)
	//for (count = 2; count < argc; count += 2)	//pay attention to even # of arguments
	{
		if (argv[count][0] == '-')
		{
			if ((strlen(argv[count]) == 2) && (count + 1 < argc))
			{
				switch (argv[count][1])
				{
					case '1':
						fD1 = TRUE;
						count++;			//increment to next argument
						strcpy(fileNameDump1,argv[count]);
						break;
					case '2':
						fD2 = TRUE;
						count++;
						strcpy(fileNameDump2,argv[count]);
						break;
					case 'u':
						f8xu = TRUE;
						count++;
						strcpy(fileName8xu,argv[count]);
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

	//fD1, fD2, and f8xu determine if there was a command-line switch available.
	//	next, they will determine if the files were successfully opened, but not now.
	//if fD1 or fD2 were not specified, set the default file name
	//(maybe) also check to see if the correct # of files were specified

// 	switch (calcModel)
// 	{
// 		case 0:					//83PBE
// 			calcModelNumber = '3';
// 			calcModelType = 'B';
// 			break;
// 		case 1:					//83PSE
// 			calcModelNumber = '3';
// 			calcModelType = 'S';
// 			break;
// 		case 2:					//84PBE
// 			calcModelNumber = '4';
// 			calcModelType = 'B';
// 			break;
// 		case 3:					//84PSE
// 			calcModelNumber = '4';
// 			calcModelType = 'S';
// 			break;
// 		default:
// 			fprintf(stderr,"*** Fatal error!  You should never get this message! O_O");
// 			exit(EXIT_FAILURE);
// 	}

// 	//Why doesn't the following compile? O_o
// 	char ModelNumber[5] = "3344";
// 	char ModelType[] = "BSBS";
	calcModelNumber = ModelNumber[calcModel];
	calcModelType = ModelType[calcModel];

	numPages = NumPagesList[calcModel];

	if (!fD1)		//set default file name for #1
	{
		strcpy(fileNameDump1,"D8");
		//strcat(fileNameDump1,calcModelNumber);
		fileNameDump1[2] = calcModelNumber;
		fileNameDump1[3] = '\0';		//NULL
		strcat(fileNameDump1,"P");
		//strcat(fileNameDump1,calcModelType);
		fileNameDump1[4] = calcModelType;
		fileNameDump1[5] = '\0';
		strcat(fileNameDump1,"E1.8xv");
	}
	if (!fD2)		//set default file name for #2
	{
		strcpy(fileNameDump2,"D8");
		//strcat(fileNameDump2,calcModelNumber);
		fileNameDump2[2] = calcModelNumber;
		fileNameDump2[3] = '\0';		//NULL
		strcat(fileNameDump2,"P");
		//strcat(fileNameDump2,calcModelType);
		fileNameDump2[4] = calcModelType;
		fileNameDump2[5] = '\0';		//NULL
		strcat(fileNameDump2,"E2.8xv");
	}

	fprintf(stderr,"Calculator model: %s\nDump 1: %s\n",calcTypeNames,fileNameDump1);
	if (calcModel >= 2 && calcModel <= 4)
		fprintf(stderr,"Dump 2: %s\n",fileNameDump2);
	if (f8xu)
		fprintf(stderr,"Upgrade file: %s\n",fileName8xu);
	fprintf(stderr,"\n");

	//Now open files (and check if the right files exist for the right calculator)
	fileDump1 = fopen(fileNameDump1,"rb");	//open in reading and binary
	if (!fileDump1)
	{
		perror(fileNameDump1);
		exit(EXIT_FAILURE);
	}
	if (!validHeader(fileDump1, NONFLASH, calcModel))
	{
		fprintf(stderr,"%s: invalid dump file.\n",fileNameDump1);
		exit(EXIT_FAILURE);
	}
	if (calcModel >= 2 && calcModel <= 4)	//if 84PBE, PSE or CSE
	{
		fileDump2 = fopen(fileNameDump2,"rb");
		if (!fileDump2)
		{
			perror(fileNameDump2);
			exit(EXIT_FAILURE);
		}
		if (!validHeader(fileDump2, NONFLASH, calcModel))
		{
			fprintf(stderr,"%s: invalid dump file.\n",fileNameDump2);
			exit(EXIT_FAILURE);
		}
	}
	if (f8xu)		//OS upgrade file
	{
		file8xu = fopen(fileName8xu,"rb");
		if (!file8xu)
		{
			perror(fileName8xu);
			exit(EXIT_FAILURE);
		}
		if (!validHeader(file8xu, FLASH, calcModel))
		{
			fprintf(stderr,"%s: invalid OS file.\n",fileName8xu);
			exit(EXIT_FAILURE);
		}
	}
	//and now the ROM file itself, but first initialize the name
	strcpy(fileNameRom,calcType);
	if (f8xu)
	{
		GetOSVersion(file8xu, &v_major, &v_minor);
		//printf("%i\n",v_minor);
		strcat(fileNameRom,"_v");
		temp[0] = v_major + '0';
		temp[1] = ((v_minor - (v_minor % 10))/10) + '0';
		temp[2] = (v_minor % 10) + '0';
		temp[3] = '\0';
		strcat(fileNameRom,temp);
	}
	strcat(fileNameRom,".rom");
	romFile = fopen(fileNameRom,"w+b");	//I suppose r+ could work to one's advantage, but it doesn't compile correctly...
	if (!romFile)
	{
		perror(fileNameRom);
		exit(EXIT_FAILURE);
	}

	romCreated = FALSE;
	ExitHandlerPtr = ExitHandler;
	atexit(ExitHandlerPtr);			//set the "error handler"

	//Let's make the ROM file!
	if (!makeBlankRom(/*romFile,*/ numPages))
	{
		fprintf(stderr,"%s: unable to make blank ROM file.\n",fileNameRom);
		exit(EXIT_FAILURE);
	}

	//Page [137F]F
	if (!writePage(/*romFile,*/ fileDump1, numPages - 1))
	{
		//perror(fileNameDump2);
		fprintf(stderr,"%s: unable to write %s to ROM file.\n",fileNameRom, fileNameDump1);
		exit(EXIT_FAILURE);
	}
	//Page [26E]F
	if (calcModel >= 2 && calcModel <= 4)
	{
		if (!writePage(/*romFile,*/ fileDump2, numPages - 1 - 0x10))
		{
			fprintf(stderr,"%s: unable to write %s to ROM file.\n",fileNameRom, fileNameDump2);
			exit(EXIT_FAILURE);
		}
	}
	if (f8xu)
	{
		if (!write8xu(/*romFile,*/ file8xu, calcModel))
		{
			fprintf(stderr,"%s: unable to write %s to ROM file.\n",fileNameRom, fileName8xu);
			exit(EXIT_FAILURE);
		}
		//validate the OS
		fseek(romFile,0x0056l,SEEK_SET);
		fwrite("\x5A\xA5",1,2,romFile);
		fseek(romFile,(((long)(numPages - 2)) * 0x4000) + 0x1FE0, SEEK_SET);
		fwrite("\0",1,1,romFile);
	}

	romCreated = TRUE;		//Success!
	//WE ARE DONE! Close all files

	//the following code is supposed to be in a function that is hooked and called whenever it exit()s, but....
	fclose(fileDump1);
	if (fD2)
		fclose(fileDump2);
	if (f8xu)
		fclose(file8xu);
	//fclose(romFile);

// 	if (!romCreated)
// 	{
// 		remove(fileNameRom);
// 		fprintf(stderr,"\n%s was not created.\n",fileNameRom);
// 		return EXIT_FAILURE;
// 	}
// 	//else
// 	fprintf(stderr,"\n%s was successfully created.\n",fileNameRom);
	return (romCreated ? EXIT_SUCCESS : EXIT_FAILURE);
}

int validModel(char calcType[], char argv1[], int* calcModel)		//check to see if calculator is a valid model
{
	int valid;
	int count;		//local var
	size_t n;		//for strncpy()

	//see if calculator model is valid
	n = strlen(argv1);		//get size of calcType argument
	//printf("calcType length: %u\n",n);
	if (n > CALCTYPE_LENGTH)
		n = CALCTYPE_LENGTH;	//if too big, set to max size

	//assert(n <= CALCTYPE_LENGTH);
	strncpy(calcType, argv1, n);	//copy n bytes to calcType
	calcType[n] = '\0';			//append NULL (remember zero-based indexed!)

	valid = FALSE;	//temp = if calcType is a valid model
	for (count = 0; count < CALCTYPE_MODELS; count++)
	{
		//printf("%s\n",calcTypeList[count]);
		if (strcmp(calcType, calcTypeList[count]) == 0)
		{
			valid = TRUE;			//valid model detected
			*calcModel = count;	//assume calcTypeList follows _GetHardwareVersion
		}
	}
	return valid;
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
	//int valid = TRUE;
	char buffer[0x4D];		//input header size
	char tempName[9];		//a temporary string
	int bufferSize;

	//int v_major, v_minor;	//local variables!
	int certID;

	if (fseek(inFile, 0, SEEK_SET))
		return FALSE;		//BAD FILE!

	bufferSize = (type == NONFLASH)? 0x37 + 0x11 : 0x4D;		//whee.. magic numbers :D
	fread(buffer, 1, bufferSize, inFile);

	for (count = 0; count < 8; count++)
		tempName[count] = buffer[count];
	tempName[count] = '\0';

	if (type == NONFLASH)
	{
		if (strcmp(tempName, "**TI83F*") != 0)
		{
			//printf("Not valid signature\n");
			return FALSE;
		}
		//Lengths == 0x4005? / first byte of header == 0x0D?
		if ((buffer[0x37] != 0x0D) || /*(buffer[0x39] != 0x05) || (buffer[0x3A] != 0x40) || */(buffer[0x39] != buffer[0x46]) || (buffer[0x3A] != buffer[0x47]))
		{
			//printf("%X %X%X != %X%X\n",buffer[0x37], buffer[0x3A], buffer[0x39], buffer[0x47], buffer[0x46]);
			return FALSE;
		}
	}
	else if (type == FLASH)
	{
		if (strcmp(tempName, "**TIFL**") != 0)		//valid signature?
			return FALSE;
		for (count = 0x11; count < 0x11 + 8; count++)
			tempName[count-0x11] = buffer[count];
		tempName[count-0x11] = '\0';
		if (strcmp(tempName, "basecode") != 0)		//OS file?
			return FALSE;
		if ((buffer[0x30] != 0x73) || (buffer[0x31] != 0x23))	//83+ and OS file?
			return FALSE;
		//Now check if 84+ OS specified for 84+ ROM and vice versa
		fseek(inFile, 0x68, SEEK_SET);		//another magic number! :D
		certID = toInt((char)fgetc(inFile));				//fgetc() returns an int...
		if ((certID == 0x0A) && (calcModel < 2))	//if 84+ OS for 83+ ROM
			return FALSE;
		if ((certID == 0x04) && (calcModel > 1))	//if 83+ OS for 84+ ROM
			return FALSE;
	}

	//return valid
	return (!fseek(inFile, 0, SEEK_SET));
}

int makeBlankRom(/*FILE *romFile,*/ int numPages)
{
	//writes a blank ROM file
	//assume fseek() at beginning

	unsigned char blankPage[PAGE_SIZE];
	int count;

	for (count = 0; count < PAGE_SIZE; count++)
		blankPage[count] = 0xFF;				//blank

	for (count = 0; count < numPages; count++)
	{
		if (fwrite(blankPage, 1, PAGE_SIZE, romFile) != PAGE_SIZE)
			return FALSE;
	}
	return TRUE;
}

int writePage(/*FILE *romFile,*/ FILE *dumpFile, int pageNum)
{
	//writes the specified page to romFile
	//automatically fseeks()

	long position;
	int count;
	int character;		//for fputc() and fgetc(), since fwrite() isn't nice >:(

	position = ((long)pageNum * 0x4000);
	if (fseek(romFile, position, SEEK_SET) != 0)
		return FALSE;

	position = 0x48 + 2;		//more magic numbers :D
	if (fseek(dumpFile, position, SEEK_SET) != 0)
		return FALSE;

// 	printf("about to fwrite\n");
// 	//if (fwrite(dumpFile, 1, PAGE_SIZE, romFile) != PAGE_SIZE)
// 	//	return FALSE;
// 	position = fwrite(dumpFile, 1, PAGE_SIZE, romFile);
// 	if (position != PAGE_SIZE)
// 	{
// 		printf("%u\n",position);
// 		return FALSE;
// 	}

	for (count = 0; (count < PAGE_SIZE) && !feof(romFile) && !feof(dumpFile); count++)
	{
		fputc(fgetc(dumpFile), romFile);
	}
	if (feof(romFile) || feof(dumpFile))
		return FALSE;

	return TRUE;
}

int write8xu(/*FILE *romFile,*/ FILE *file8xu, int calcModel)
{
	//integrates the OS into the romFile
	//most of the following code is from rompatch by Benjamin Moody
	
	char c;
	int nbytes, rectype, dl, dh, check, a;
	int pagenum=0;
	long position;
	int j;
	int bp=0;
	int semode=0;						//be sure to init to 0
	//char ibuf[0x4d];
	int parts_remaining/* = 1*/;
	
	fseek(romFile, 0, SEEK_SET);		//reset romFile to beginning ('reset' previous sessions)
	//the header is ignored since it'll get overwritten anyway
	fseek(file8xu, 0x4E, SEEK_SET);		//magic numbers ;)
	
	//if (ibuf[0x30] == 0x23)		//if OS file
		parts_remaining = 2;
	
	if (calcModel == 1 || calcModel == 3)		//if Silver Edition
		semode = 0x60;						//OR mask
	else if (calcModel == 2)				//if 84PBE
		semode = 0x20;
	else if (calcModel == 4)
		semode = 0xe0;
// 	else
// 		semode = 0;

	c=fgetc(file8xu);

	while (!feof(file8xu) && !ferror(file8xu) && parts_remaining)
	{
		if (!feof(file8xu) && !ferror(file8xu))
		{
			if (c != ':')
			{
				//fprintf(stderr,"%s: hex file %s not in valid Intel format",
				//argv[0],argv[i]);
				return FALSE;
			}

			fscanf(file8xu,"%2X%2X%2X%2X",&nbytes,&dh,&dl,&rectype);		//read 'header' data
			check=nbytes+dh+dl+rectype;			//add to checksum

			if (rectype == 0)
			{
				position = (pagenum * 0x4000l) + (((dh*0x100)+dl) & 0x3fff);
				if (fseek(romFile, position, SEEK_SET) != 0)
				{
					perror("fseeking on ROM file");
					return FALSE;
				}
			}

			for (j = 0; j < nbytes; j++)
			{
				fscanf(file8xu,"%2X",&a);		//read a byte of the data
				check+=a;						//add to checksum
				if (rectype == 0)
					fputc(a,romFile);			//if just data record, write it to the ROM file
				else if (rectype == 2)			//else if TI-specific record
				{
					if (semode)
					{
						if ((a > 0x0F) && (a < 0x20))
							pagenum = a | semode;	//add the OR mask
						else
							pagenum = a;
					}
// 					else if (bp<0)
// 						pagenum=(-bp)-a;
					else
						pagenum=/*bp+*/a;
				}
			}

			if (rectype == 1)		//if we reached an 'end' record
				parts_remaining--;

			fscanf(file8xu,"%2X",&a);	//checksum byte
			//printf("Calc and Checksum: %i %i\n",check,a);
			check+=a;					//not sure how this works, but it does :D

			if (check & 0xFF)		//if bad checksum
			{
// 				fprintf(stderr,
// 				"%s: invalid checksum %X (at %02X%02X, type %02X) in %s\n",
// 				argv[0],check,dh,dl,rectype,argv[i]);
				return FALSE;
			}
		}
		
		//get rid of whitespace characters
		do
			c=fgetc(file8xu);
		while ((c == '\n') || (c == '\r') || (c == '\t') || (c == '\f') || (c == ' '));
	}

	return TRUE;
}

int GetOSVersion(FILE *file8xu, int *v_major, int *v_minor)
{
	//assuming the OS version is at specific offset (may cause future problems!)
	//return *file8xu fseek()ed to beginning
	//Assume ASCII charset systems
	char byte1, byte2;

	fseek(file8xu, 0x6D, SEEK_SET);		//more magic numbers :D
	byte1 = fgetc(file8xu);
	byte2 = fgetc(file8xu);
	*v_major = (toInt(byte1) * 16) + toInt(byte2);	//remember hex

	fseek(file8xu, 0x73, SEEK_SET);		//...
	byte1 = fgetc(file8xu);
	byte2 = fgetc(file8xu);
	//printf("%c%c %i\n",byte1, byte2,toInt(byte2));
	*v_minor = (toInt(byte1) * 16) + toInt(byte2);	//remember hex

	return (!fseek(file8xu, 0, SEEK_SET));		//set to beginning
}

int toInt(char hex)
{
	if ((hex >= '0') && (hex <= '9'))
		return ((int)hex - '0');
	else if ((toupper(hex) >= 'A') && (toupper(hex) <= 'F'))		//upper case
		return ((int)hex - 'A' + 10);
	else
		return 0;		//0 by default
}

void ExitHandler(void)
{
	//We quit from somewhere -- display the message and delete romFile
	if (!romCreated)
	{
		fclose(romFile);
		remove(fileNameRom);
		fprintf(stderr,"%s was not created.\n",fileNameRom);
	}
	else
		fprintf(stderr,"%s was successfully created.\n",fileNameRom);

	//since the FILEs are closed at termination, it is not NEEDED to close them here
}