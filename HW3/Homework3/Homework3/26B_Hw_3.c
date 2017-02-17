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

#define DEFAULT_FILE "input.txt"
#define TABSIZE 40
#define BUCKETSIZE 3
#define DIGITS "1234567890"
#define NAME_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\040()"
#define HASHFILE "hashtable.bin"
#define NUM_ERRORS 10

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
void search_record(char *id, long hash, FILE *fp);
void addUserInput(FILE *fp);
void errorReporter(int error);
void formatName(char* str);

int main(int argc, char *argv[])
{
	FILE *inputFile;
	FILE *hashFile;
	char line[100];
	RECORD tempRecord;
	long hashedId;

	if (argc > 1)
	{
		if ((inputFile = fopen(argv[1], "r+b")) == NULL)
		{
			printf("Could not open %s.\n", argv[1]);
			if ((inputFile = fopen(DEFAULT_FILE, "r+b")) == NULL)
			{
				printf("Could not open default file %s.\n", DEFAULT_FILE);
				exit(1);
			}
		}
	}
	else
	{
		if ((inputFile = fopen(DEFAULT_FILE, "r+b")) == NULL)
		{
			printf("Could not open default file %s.\n", DEFAULT_FILE);
			exit(1);
		}
	}
	hashFile = create_hash_file(HASHFILE);

	while (fgets(line, sizeof(line), inputFile))
	{
		strcpy(tempRecord.id, strtok(line, ","));
		strcpy(tempRecord.name, strtok(NULL, ":"));
		tempRecord.qty = (int)strtol(strtok(NULL, "\n"), NULL, 10);
		hashedId = hash(tempRecord.id, TABSIZE);
		insert_record(&tempRecord, hashedId, hashFile);
	}
	fclose(inputFile);
	printf("Enter the id to search for: ");
	fgets(line, sizeof(line), stdin);
	*(line + strlen(line) - 1) = '\0';
	hashedId = hash(line, TABSIZE);
	search_record(line, hashedId, hashFile);
	
	addUserInput(hashFile);

	printf("Enter the id to search for: ");
	fgets(line, sizeof(line), stdin);
	*(line + strlen(line) - 1) = '\0';
	hashedId = hash(line, TABSIZE);
	search_record(line, hashedId, hashFile);

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

void search_record(char *id, long hash, FILE *fp)
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
			printf("You found %s at hash bucket %ld:\nID: %s\nNAME: %s\nQTY: %d\n", id, hash, detect.id, detect.name, detect.qty);
			return;
		}
	}
	printf("Unable to find %s in the hash table.\n", id);
	return;
}

void addUserInput(FILE *fp)
{
	char line[100];
	long hashedId;
	char *tempID, *tempName, *tempQty, *tokPos;
	RECORD tempRecord;
	long errorFlags;


	printf("Enter the record to be added to the database in the format:\nID,NAME:QUANTITY\nExample: 1238,WELDING TORCH:18\nEnter 'QUIT' to stop record entry: ");
	while (errorFlags = 0, strcmp(fgets(line, sizeof(line), stdin), "QUIT\n"))
	{
		if ((tempID = strtok(line, ",")) == 0) errorFlags += 1;
		tempName = line + strlen(tempID) + 1;						// hold address of next char after token
		tempID = strtok(tempID, "\040\t");							// removes leading and trailing whitespace
		if (strspn(tempID, DIGITS) != strlen(tempID)) errorFlags += 2;
		if (strlen(tempID) > 4) errorFlags += 4;
			
		tempName = strtok(tempName, ":"); 
		formatName(tempName);
		if (strlen(tempName) > 20) errorFlags += 16;
		if (strspn(tempName, NAME_CHARS) != strlen(tempName)) errorFlags += 32;
		
		strcpy(tempQty, strtok(NULL, "\040\t\n"));					// removes whitespace
		if (strspn(tempQty, DIGITS) != strlen(tempQty)) errorFlags += 64;

		if (errorFlags)
			errorReporter(errorFlags);
		else
		{
			strcpy(tempRecord.id, tempID);
			strcpy(tempRecord.name, tempName);
			tempRecord.qty = (int)strtol(tempQty, NULL, 10);
			hashedId = hash(tempRecord.id, TABSIZE);
			insert_record(&tempRecord, hashedId, fp);
		}
		printf("Added record.\nEnter the record to be added to the database in the format : \nID, NAME:QUANTITY\nExample : 1238, WELDING TORCH : 18\nEnter 'QUIT' to stop record entry :");
	}
}

void errorReporter(int error)
{
	if (error & 1) printf("Missing ',' delimiter between Id and Name. Unable to parse the entry.\n");
	if (error & 2) printf("Id is too long. Id's must be in the range 0-9999.\n");
	if (error & 4) printf("Id contains a non-numerical character.\n");
	if (error & 8) printf("Missing ':' delimiter between Name and Quantity. Unable to parse the entry.\n");
	if (error & 16) printf("Name is too long. Name must be a maximum of 20 chars.\n");
	if (error & 32) printf("Name contains invalid characters. Valid names can consist only of letters, spaces and ().\n");
	if (error & 64) printf("Quantity contains a non-numerical character.\n");

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