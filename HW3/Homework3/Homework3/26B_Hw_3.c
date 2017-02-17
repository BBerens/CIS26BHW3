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

#define DEFAULT_FILE "hardware_db.txt"
#define TABSIZE 40
#define BUCKETSIZE 3
#define DIGITS "1234567890"

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

int main(int argc, char *argv[])
{
	FILE *fp;
	FILE *hashFile;
	char line[100];
	RECORD tempRecord;
	long hashedId;
	char temp[100];

	if (argc > 1)
	{
		if ((fp = fopen(argv[1], "r+b")) == NULL)
		{
			printf("Could not open %s.\n", argv[1]);
			if ((fp = fopen(DEFAULT_FILE, "r+b")) == NULL)
			{
				printf("Could not open default file %s.\n", DEFAULT_FILE);
				exit(1);
			}
		}
	}
	else
	{
		if ((fp = fopen(DEFAULT_FILE, "r+b")) == NULL)
		{
			printf("Could not open default file %s.\n", DEFAULT_FILE);
			exit(1);
		}
	}

	printf("Enter the name of the file to hash to: ");
	fgets(line, sizeof(line), stdin);
	*(line + strlen(line) - 1) = '\0';
	hashFile = create_hash_file(line);

	while (fgets(line, sizeof(line), fp))
	{
		strcpy(tempRecord.id, strtok(line, ","));
		strcpy(tempRecord.name, strtok(NULL, ":"));
		tempRecord.qty = (int)strtol(strtok(NULL, "\n"), NULL, 10);
		hashedId = hash(tempRecord.id, TABSIZE);
		insert_record(&tempRecord, hashedId, hashFile);
	}
	fclose(fp);
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
			fseek(fp, -1 * sizeof(RECORD), SEEK_CUR);
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
			printf("You found %s at hash bucket %ld:\nID: %s\nNAME: %s\nQTY: %d", id, hash, detect.id, detect.name, detect.qty);
			return;
		}
	}
	printf("Unable to find %s in the hash table.\n", id);
	return
}

void addUserInput(FILE *fp)
{
	char line[100];
	int error;
	char tempName[100];

	printf("Enter the record to be added to the database in the format:\n ID,NAME:QUANTITY\n Example: 1238,WELDING TORCH:18\n Enter 'QUIT' to stop record entry: ");
	while (error = 0, strcmp(fgets(line, sizeof(line), stdin), "QUIT\n"))
	{
		strcpy(tempName, strtok(line, ","));
		if (strlen(line) != 4) error = 2;
		if (strspn(line, DIGITS) != 4) error = 3;


	}
}