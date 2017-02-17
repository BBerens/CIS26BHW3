/**
CIS 26B - Advanced C Programming
Homework #3: 
 Hashing to a (binary) file and using advanced string manipulation functions.
 
 This program allows additions to, deletions from, or displays of database records in a  
 hardware store database.

 NAME: Brett Berens	
 IDE: Microsoft Visual Studio 2015
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define DEFAULT_FILE "input.txt"
#define HASHFILE "hashtable.bin"
#define TABSIZE 40
#define BUCKETSIZE 3
#define DIGITS "1234567890"
#define NAME_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\040()"
#define FLUSH while(c =getchar()!='\n' && c!= EOF)


typedef struct record RECORD;
struct record
{
	char id[5];
	char name[20];
	int  qty;
};

long hash(char *key, int size);
FILE *create_hash_file(char *filename);
void insert_record(RECORD *tempRecord, long hashedId, FILE *fp);
int search_record(char *id, long hash, FILE *fp);
void manualInput(FILE *fp);
void errorReporter(int error);
void formatName(char* str);
FILE *openInputFile(char *fileName);
void batchInput(FILE *inputFile, FILE *hashFile);
void deleteRecord(FILE *hashFile);
void displayRecord(FILE *hashFile);

int main(int argc, char *argv[])
{
	FILE *inputFile = NULL;
	FILE *hashFile;
	char line[100];
	long hashedId;
	int yesNo;
	int c;

	hashFile = create_hash_file(HASHFILE);

	if (argc > 1)
	{
		strcpy(line ,argv[1]);
		if ((inputFile = openInputFile(argv[1])) == NULL)
			inputFile = openInputFile(DEFAULT_FILE);
	}
	else
		inputFile = openInputFile(DEFAULT_FILE);
	
	batchInput(inputFile, hashFile);
	
	do{
		printf("Would you like to add records from another file? Y/N: ");
		if (yesNo = getchar() == 'y' || yesNo == 'Y')
		{
			FLUSH;
			printf("Enter the file to open: ");
			fgets(line, sizeof(line), stdin);
			*(line + strlen(line) - 1) = '\0';
			if (inputFile = openInputFile(line))
				batchInput(inputFile, hashFile);
		}
		FLUSH;
		} while ((yesNo == 'y' || yesNo == 'Y'));
	
	do{
		printf("Would you like to manually add records? Y/N: ");
		if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
		{
			FLUSH;
			manualInput(hashFile);
		}
		FLUSH;
	} while ((yesNo == 'y' || yesNo == 'Y'));
	
	do{
		printf("Would you like to delete records? Y/N: ");
		if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
		{
			FLUSH;
			deleteRecord(hashFile);
		}
	} while ((yesNo == 'y' || yesNo == 'Y'));


	do {
		printf("Would you like to display records? Y/N: ");
		if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
		{
			FLUSH;
			printf("Enter the id to search for: ");
			fgets(line, sizeof(line), stdin);
			*(line + strlen(line) - 1) = '\0';
			hashedId = hash(line, TABSIZE);

			if (search_record(line, hashedId, hashFile))
				displayRecord(hashFile);
		}
	} while ((yesNo == 'y' || yesNo == 'Y'));




	fclose(hashFile);
	system("pause");

	return 0;
}

/*******************************************************
Hash formula is the same as used in Chapter 3
*/

long hash(char *key, int size)
{
	long address = 0;
	for (; *key != '\0'; key++)
	{
		address += *key * *key * *key;
	}
	return address % size;
}

FILE *create_hash_file(char *filename)
{
	FILE *fp;
	RECORD hashtable[TABSIZE][BUCKETSIZE] = { "", "", 0 };
	if ((fp = fopen(filename, "w+b")) == NULL)
	{
		printf("Could not open file %s. Aborting!\n", filename);
		exit(1);
	}
	if (fwrite(&hashtable[0][0], sizeof(RECORD), TABSIZE * BUCKETSIZE, fp) < TABSIZE)
	{
		printf("Hash table could not be created. Abort!\n");
		exit(2);
	}
	rewind(fp);
	return fp;
}

void insert_record(RECORD *tempRecord, long hashedId, FILE *fp)
{
	RECORD detect;
	int i;
	if (fseek(fp, hashedId * BUCKETSIZE * sizeof(RECORD), SEEK_SET) != 0)
	{
		printf("Fatal seek error! Abort!\n");
		exit(4);
	}

	for (i = 0; i < BUCKETSIZE; i++)
	{
		fread(&detect, sizeof(RECORD), 1, fp);
		if (*detect.id == '\0')
		{
			fseek(fp, -1 * (long)sizeof(RECORD), SEEK_CUR);
			fwrite(tempRecord, sizeof(RECORD), 1, fp);
			return;
		}
	}
	printf("Hash table overflow! Abort!\n");
	exit(5);
}

int search_record(char *id, long hash, FILE *fp)
{
	RECORD detect;
	int i;

	if (fseek(fp, hash * BUCKETSIZE * sizeof(RECORD), SEEK_SET) != 0)
	{
		printf("Fatal seek error! Abort!\n");
		exit(4);
	}
	for (i = 0; i < BUCKETSIZE; i++)
	{
		fread(&detect, sizeof(RECORD), 1, fp);
		if (strcmp(detect.id, id) == 0)
		{
			fseek(fp, -1 * (long)sizeof(RECORD), SEEK_CUR);
			return 1;
		}
	}
	printf("Unable to find %s in the hash table.\n", id);
	return 0;
}

void manualInput(FILE *fp)
{
	char line[100];
	long hashedId;
	char *tempID, *tempName, *tempQty;
	RECORD tempRecord;
	long errorFlags;


	printf("Enter the record to be added to the database in the format:\n"
			"\tID, NAME: QUANTITY\n"
			"\tExample: 1238,WELDING TORCH:18\n"
			"Enter 'QUIT' to stop record entry: ");
	while (errorFlags = 0, strcmp(fgets(line, sizeof(line), stdin), "QUIT\n"))
	{
		if ((tempID = strtok(line, ",")) == 0)					errorFlags += 1;	// if there is no ',', set error bit 0
		else
		{
			tempName = tempID + strlen(tempID) + 1;
			tempID = strtok(tempID, "\040\t");							// removes leading and trailing whitespace
			if (strspn(tempID, DIGITS) != strlen(tempID))			errorFlags += 2;	// if there are non digit chars, set error bit 1
			if (strlen(tempID) > 4)									errorFlags += 4;	// if there are more than 4 digits, set error bit 2

			if ((tempName = strtok(tempName, ":")) == 0)				errorFlags += 8;	// if there is no ':', set error bit 3
			else
			{
				if(tempName) formatName(tempName);						// will receive exception if we pass null to formatName
				if (strlen(tempName) > 20)								errorFlags += 16;	// if name length is more than 20 chars, set error bit 4
				if (strspn(tempName, NAME_CHARS) != strlen(tempName))	errorFlags += 32;	// if invalid name chars are used, set error bit 5

				tempQty = strtok(NULL, "\040\t\n");					// removes trailing and leading whitespace
				if (strspn(tempQty, DIGITS) != strlen(tempQty))			errorFlags += 64;	// if there are non-digit chars, set error bit 6
				if ((int)strtol(tempQty, NULL, 10) > 2000)				errorFlags += 128;	// if number is greater than 2000, set error bit 7
			}
		}
		if (errorFlags)
			errorReporter(errorFlags);			// if any error bit is set, report to user and do not try to store parsed value in tempRecord
		else
		{
			strcpy(tempRecord.id, tempID);
			strcpy(tempRecord.name, tempName);
			tempRecord.qty = (int)strtol(tempQty, NULL, 10);
			hashedId = hash(tempRecord.id, TABSIZE);
			insert_record(&tempRecord, hashedId, fp);
			printf("\tAdded the record: %s,%s:%d\n\n", tempRecord.id, tempRecord.name, tempRecord.qty);
		}
		printf("Enter the next record to be added to the database in the format : \n\tID, NAME: QUANTITY\n\tExample : 1238, WELDING TORCH: 18\nEnter 'QUIT' to stop record entry :");
	}
	return;
}

void errorReporter(int error)
{
	printf("ERROR: The record could not be added for the following reasons:\n");
	if (error & 1) printf("Missing ',' delimiter between Id and Name. Unable to parse the entry.\n");
	if (error & 2) printf("Id contains a non-numerical character.\n");
	if (error & 4) printf("Id is out of range. Id's must be in the range 0-9999.\n");
	if (error & 8) printf("Missing ':' delimiter between Name and Quantity. Unable to parse the entry.\n");
	if (error & 16) printf("Name is too long. Name must be a maximum of 20 chars.\n");
	if (error & 32) printf("Name contains invalid characters. Valid names can consist only of letters, spaces and ().\n");
	if (error & 64) printf("Quantity contains a non-numerical character.\n");
	if (error & 128) printf("Quantity is too large. Quantity must be in the range 0-2000.\n");
	printf("\n\n");
	return;
}

void formatName(char* str) {
	char *cur, *walk;
	cur = walk = str;
	while (*walk)
	{
		if (!isspace(*walk) || (cur != str && !isspace(*(walk-1))))
			*cur++ = toupper(*walk);
		walk++;
	}
	if (*(cur - 1) == ' ') *(cur - 1) = '\0';
	else *cur = '\0';
	return;
}

FILE *openInputFile(char *fileName)
{
	FILE *inputFile;
	if ((inputFile = fopen(fileName, "r")) == NULL)
	{
		printf("Could not open %s.\n", fileName);
		if (strcmp(fileName, DEFAULT_FILE))
			return inputFile;				// NULL
		else
			exit(1);						// couldn't open default. Exit.
	}
	fseek(inputFile, 0, SEEK_END);
	if (ftell(inputFile) == 0)
	{
		printf("File %s is empty. Aborting!\n", DEFAULT_FILE);
		exit(1);
	}
	fseek(inputFile, 0, SEEK_SET);
	return inputFile;
}

void batchInput(FILE *inputFile, FILE *hashFile)
{
	char line[100];
	RECORD tempRecord;
	long hashedId;
	while (fgets(line, sizeof(line), inputFile))
	{
		strcpy(tempRecord.id, strtok(line, ","));
		strcpy(tempRecord.name, strtok(NULL, ":"));
		tempRecord.qty = (int)strtol(strtok(NULL, "\n"), NULL, 10);
		hashedId = hash(tempRecord.id, TABSIZE);
		insert_record(&tempRecord, hashedId, hashFile);
	}
	fclose(inputFile);
}

void deleteRecord(FILE *hashFile)
{
	long hashedId;
	char line[100];
	int yesNo;
	RECORD tempRecord = { "","",0 };
	int c;

	printf("Enter the id of the record you would like to delete or enter QUIT to stop: ");
	while (strcmp(fgets(line, sizeof(line), stdin), "QUIT\n"))
	{
		*(line + strlen(line) - 1) = '\0';
		hashedId = hash(line, TABSIZE);
		if (search_record(line, hashedId, hashFile))
		{
			printf("The following record was found:\n");
			displayRecord(hashFile);
			printf("Do you want to delete it? Y/N: ");
			if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
			{
				fwrite(&tempRecord, sizeof(RECORD), 1, hashFile);
				printf("Deleted!\n\n");
			}
			FLUSH;
		}
		printf("Enter the id of the record you would like to delete or enter QUIT to stop: ");
	}
}

void displayRecord(FILE *hashFile)
{
	RECORD tempRecord;

	fread(&tempRecord, sizeof(RECORD), 1, hashFile);
	fseek(hashFile, -1 * (long)sizeof(RECORD), SEEK_CUR);
	printf("%s,%s:%d\n", tempRecord.id, tempRecord.name, tempRecord.qty);
}