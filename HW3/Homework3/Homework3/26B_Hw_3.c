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

typedef struct record RECORD;
struct record
{
	char id[4];
	char name[20];
	int  qty;
};

long hash(char *key, int size);
FILE *create_hash_file(char *filename);
void insert_record(RECORD *tempRecord, long hashedId, FILE *fp);

int main(int argc, char *argv[])
{
	FILE *fp;
	char line[100];
	RECORD tempRecord;
	long hashedId;

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
		else printf("opened default file %s", DEFAULT_FILE);
	}

	printf("Enter the name of the file to hash to: ");
	fgets(line, sizeof(line), stdin);
	*(line + strlen(line) - 1) = '\0';
	create_hash_file(line);

	while (fgets(line, sizeof(line), fp))
	{
		strcpy(tempRecord.id, strtok(line, ','));
		strcpy(tempRecord.name, strtok(NULL, ':'));
		tempRecord.qty = strtoi(strtok(NULL, '\n'));
		hashedId = hash(tempRecord.id, TABSIZE);
		insert_record(&tempRecord, hashedId, fp);
	}
	fclose(fp);
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
	RECORD hashtable[TABSIZE][BUCKETSIZE];
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