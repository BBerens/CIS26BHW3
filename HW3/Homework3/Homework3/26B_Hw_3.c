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

#define DEFAULT_FILE "input.txt"				// default file name to be used if none is provided
#define HASHFILE "hashtable.bin"				// name of bin file to write hashtable to
#define TABSIZE 40								// number of buckets
#define BUCKETSIZE 3							// number of records per bucket
#define DIGITS "1234567890"						// valid digits
#define NAME_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\040()"		// valid chars for name attribute
#define FLUSH while(c =getchar()!='\n' && c!= EOF)							// flushes input buffer


typedef struct record RECORD;
struct record
{
	char id[5];
	char name[20];
	int  qty;
};

long hash(char *key, int size);
FILE *create_hash_file(char *filename);
int insert_record(RECORD *tempRecord, long hashedId, FILE *fp);
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

	if (argc > 1)					// if a command parameter is passed
	{
		if ((inputFile = openInputFile(argv[1])) == NULL)	// if the user inputted file couldn't be opened, open the default file
			inputFile = openInputFile(DEFAULT_FILE);
	}
	else
		inputFile = openInputFile(DEFAULT_FILE);	// if no command paramter is passed, opent the default file
	
	batchInput(inputFile, hashFile);		// perform a batch input of txt file to hashfile
	

	printf("Would you like to add records from another file? Y/N: ");
	if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
	{
		FLUSH;										
		printf("Enter the file to open: ");
		fgets(line, sizeof(line), stdin);
		*(line + strlen(line) - 1) = '\0';			// replace '\n' with '\0'
		if (inputFile = openInputFile(line))		// if the user inputted file can be opened
		{
			batchInput(inputFile, hashFile);		// perform a batch input of txt file to hashfile
			printf("Records from %s added.\n\n", line);
		}
	}
	else FLUSH;											
	
	printf("Would you like to manually add records? Y/N: ");
	if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
	{
		FLUSH;
		manualInput(hashFile);
	}
	else FLUSH;
	
	printf("Would you like to delete a record? Y/N: ");
	if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
	{
		FLUSH;
		deleteRecord(hashFile);
	}
	else FLUSH;

	do 
	{
		printf("Would you like to display a record? Y/N: ");
		if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
		{
			FLUSH;
			printf("Enter the id to search for: ");
			fgets(line, sizeof(line), stdin);
			*(line + strlen(line) - 1) = '\0';
			hashedId = hash(line, TABSIZE);						// hash the id

			if (search_record(line, hashedId, hashFile))					// check if the id is present in the hashtable 
				displayRecord(hashFile);									// if so, display it
			else
				printf("Unable to find %s in the hash table.\n", line);		// if not, prompt user
		}
		else FLUSH;
	} while (yesNo == 'y' || yesNo == 'Y');

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

/*******************************************************
Writes an empty hash table to specified filename.
Uses TABSIZE and BUCKETSIZE determine size of 
Returns pointer to file
*/

FILE *create_hash_file(char *filename)
{
	FILE *fp;
	RECORD hashtable[TABSIZE][BUCKETSIZE] = { "", "", 0 };		// empty record to initialize the hashtable with zeros
	if ((fp = fopen(filename, "w+b")) == NULL)		// if the specified filename cannot be opened, print error and exit.
	{
		printf("Could not open file %s. Aborting!\n", filename);
		exit(1);
	}
	if (fwrite(&hashtable[0][0], sizeof(RECORD), TABSIZE * BUCKETSIZE, fp) < TABSIZE) // write TABSIZE * BUCKETSIZE records to file
	{
		printf("Hash table could not be created. Abort!\n");	// if unable to write hashtable, print error and exit
		exit(2);
	}
	rewind(fp);				// return fp to beginning of the file.
	return fp;
}

/*******************************************************
Inserts tempRecord to hashtable in file fp using hashedId
returns 1 if succesful and 0 if unsuccesful
*/

int insert_record(RECORD *tempRecord, long hashedId, FILE *fp)
{
	RECORD detect;
	int i;

	if (fseek(fp, hashedId * BUCKETSIZE * sizeof(RECORD), SEEK_SET) != 0)	// move fp to correct bucket indicated by hashedId
	{
		printf("Fatal seek error! Abort!\n");			// if unable to find bucket, print error and exit
		exit(4);
	}

	for (i = 0; i < BUCKETSIZE; i++)					// for each record in the bucket
	{
		fread(&detect, sizeof(RECORD), 1, fp);			// read the record into detect
		if (*detect.id == '\0')							// if the id == 0 it is empty and free to be used.
		{
			fseek(fp, -1 * (long)sizeof(RECORD), SEEK_CUR);		// move fp back one record
			fwrite(tempRecord, sizeof(RECORD), 1, fp);			// write tempRecord to file
			return 1;		// successful
		}
	}
	printf("Hash table overflow! Record %s,%s:%d could not be added!\n\n", tempRecord->id, tempRecord->name, tempRecord->qty);
	return 0;		// unsuccesful, let user know the record wasn't added due to hash table overflow.
}

/*******************************************************
Searches hashtable for Id using hash to go direct to correct bucket
Returns 1 if record is found or 0 if not
*/

int search_record(char *id, long hash, FILE *fp)
{
	RECORD detect;
	int i;

	if (fseek(fp, hash * BUCKETSIZE * sizeof(RECORD), SEEK_SET) != 0)	// move to correct bucket using hash
	{
		printf("Fatal seek error! Abort!\n");	// if bucket cannot be found print error and exit
		exit(4);
	}
	for (i = 0; i < BUCKETSIZE; i++)		// for each record in the bucket
	{
		fread(&detect, sizeof(RECORD), 1, fp);		// read the record into detect
		if (strcmp(detect.id, id) == 0)		// if the id of the record is the same as the id passed as an argument
		{
			fseek(fp, -1 * (long)sizeof(RECORD), SEEK_CUR);		// move back one record
			return 1;		// report the record with the id has been found
		}
	}
	return 0;		// unable to find record with that id
}

/*******************************************************
Prompts user to manually enter a record to be added to the hashtable
Performs multiple formatting checks on user input and sets errorFlags if there are any issues.
If any errors, report all to the user
Checks if Id is a duplicate and if not adds to the hashtable
*/

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
			"Enter 'QUIT' to stop record entry: ");	// prompt user
	while (errorFlags = 0, strcmp(fgets(line, sizeof(line), stdin), "QUIT\n"))	// clear errorFlags and check if user wants to quit
	{
		if ((tempID = strtok(line, ",")) == 0)					errorFlags += 1;	// if there is no ',', set error bit 0
		else
		{
			tempName = tempID + strlen(tempID) + 1;
			tempID = strtok(tempID, "\040\t");											// removes leading and trailing whitespace
			if (strspn(tempID, DIGITS) != strlen(tempID))				errorFlags += 2;	// if there are non digit chars, set error bit 1
			if (strlen(tempID) != 4)									errorFlags += 4;	// if there are not 4 digits, set error bit 2

			if ((tempName = strtok(tempName, ":")) == 0)				errorFlags += 8;	// if there is no ':', set error bit 3
			else
			{
				if(tempName) formatName(tempName);						// will receive exception if we pass null to formatName
																		// removes redundant whitespace and puts chars in uppercase
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
		{		// now that we know there are no format errors in the user input,
			strcpy(tempRecord.id, tempID);								// copy id into tempRecord
			strcpy(tempRecord.name, tempName);							// copy name into tempRecord
			tempRecord.qty = (int)strtol(tempQty, NULL, 10);			// copy qty into tempRecord
			hashedId = hash(tempRecord.id, TABSIZE);					// hash the id
			if (search_record(tempRecord.id, hashedId, fp))				// check if the id is a duplicate
				printf("Duplicate ID. Cannot add %s,%s:%d to database.\n\n", tempRecord.id, tempRecord.name, tempRecord.qty);
			else if(insert_record(&tempRecord, hashedId, fp))			// insert the record into the hashtable
				printf("\tAdded the record: %s,%s:%d\n\n", tempRecord.id, tempRecord.name, tempRecord.qty);
		}
		printf("Enter the next record to be added to the database in the format : \n\tID, NAME: QUANTITY\n\tExample : 1238, WELDING TORCH: 18\nEnter 'QUIT' to stop record entry :");
	}
	return;
}

/*******************************************************
Accepts a long and checks if each bit is set. Prints appropriate error message for each error
*/
void errorReporter(long error)
{
	printf("ERROR: The record could not be added for the following reasons:\n");
	// uses bitwise and with a mask to check each bit.
	if (error & 1) printf("Missing ',' delimiter between Id and Name. Unable to parse the entry.\n");
	if (error & 2) printf("Id contains a non-numerical character.\n");
	if (error & 4) printf("Id is out of range. Id's must be a 4-digit number.\n");
	if (error & 8) printf("Missing ':' delimiter between Name and Quantity. Unable to parse the entry.\n");
	if (error & 16) printf("Name is too long. Name must be a maximum of 20 chars.\n");
	if (error & 32) printf("Name contains invalid characters. Valid names can consist only of letters, spaces and ().\n");
	if (error & 64) printf("Quantity contains a non-numerical character.\n");
	if (error & 128) printf("Quantity is too large. Quantity must be in the range 0-2000.\n");
	printf("\n\n");
	return;
}

/*******************************************************
Removes leading and trailing whitespace, redundant whitespace, and changes alpha chars to capitals
*/
void formatName(char* str) {
	char *cur, *walk;
	cur = walk = str;
	if (str)		// checks if null pointer is passed to function. If so, just return
	{
		while (*walk)		// walk to the end of the string
		{
			if (!isspace(*walk) || (cur != str && !isspace(*(walk - 1))))	// if *walk isn't a space or if the previous char isn't a space
				*cur++ = toupper(*walk);	// capitalize the char and write to *cur
			walk++;	
		}
		if (*(cur - 1) == ' ')
			*(cur - 1) = '\0';	// if the last char is a space, change it to '\0'
		else 
			*cur = '\0';	
	}
	return;
}

/*******************************************************
Open a text file with filename passed as an argument
Return a file pointer to the file if opened
Return null if unable to open
If the filename passed is the default input file it cannot be opened, will exit program
*/
FILE *openInputFile(char *fileName)
{
	FILE *inputFile;
	if ((inputFile = fopen(fileName, "r")) == NULL)	//if the file cannot be opened
	{
		printf("Could not open %s.\n", fileName);	// tell user the file couldn't be opened
		if (strcmp(fileName, DEFAULT_FILE))		// if filename is NOT default input file
			return NULL;					// return Null to indicate failed to open
		else
			exit(1);						// couldn't open default. Exit.
	}
	fseek(inputFile, 0, SEEK_END);			// go to the end of the file
	if (ftell(inputFile) == 0)				// if the file doesn't contain any data (empty
	{
		if (strcmp(fileName, DEFAULT_FILE))		// if filename is NOT default input file
		{
			printf("File %s is empty. No data read in!\n", fileName);	// print error and exit
			return NULL;			// indicates failed to open properly
		}
		else
		{														// if the default file is empty
			printf("File %s is empty. Aborting!\n", DEFAULT_FILE);	// print error and exit
			exit(1);
		}
	}
	fseek(inputFile, 0, SEEK_SET);			// go to start of file
	return inputFile;		// successful open
}

/*******************************************************
Perform batch insertion of records from a text file to hash file

*/
void batchInput(FILE *inputFile, FILE *hashFile)
{
	char line[100];
	RECORD tempRecord;
	long hashedId;
	while (fgets(line, sizeof(line), inputFile))	// read in a line from input file
	{
		strcpy(tempRecord.id, strtok(line, ","));					// copy id into tempRecord
		strcpy(tempRecord.name, strtok(NULL, ":"));					// copy name into tempRecord
		tempRecord.qty = (int)strtol(strtok(NULL, "\n"), NULL, 10);	// copy qty into tempRecord
		hashedId = hash(tempRecord.id, TABSIZE);					// hash the id
		if (search_record(tempRecord.id, hashedId, hashFile))		// check if it is a duplicate id
			printf("Duplicate ID. Cannot add %s,%s:%d to database.\n\n", tempRecord.id, tempRecord.name, tempRecord.qty);
		else 
			insert_record(&tempRecord, hashedId, hashFile);			// insert the record into the hashtable
	}
	fclose(inputFile);
}

/*******************************************************
Deletes a record from the hashtable
*/
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
		hashedId = hash(line, TABSIZE);					// hash the userinput
		if (search_record(line, hashedId, hashFile))	// search for the record and if found,
		{
			printf("The following record was found:\n");
			displayRecord(hashFile);					// show the user the record and ask if they want to delete it
			printf("Do you want to delete it? Y/N: ");
			if ((yesNo = getchar()) == 'y' || yesNo == 'Y')
			{
				fwrite(&tempRecord, sizeof(RECORD), 1, hashFile);	// write an empty record over the selected record, effectively deleting it
				printf("Deleted!\n\n");
			}
			FLUSH;
		}
		else
			printf("Unable to find %s in the hash table.\n", line);		// tell user id couldn't be found in the hashtable
		printf("Enter the id of the record you would like to delete or enter QUIT to stop: ");
	}
}

/*******************************************************
Displays the data in a record pointed to by hashFile
*/
void displayRecord(FILE *hashFile)
{
	RECORD tempRecord;
	
	fread(&tempRecord, sizeof(RECORD), 1, hashFile);				// read in the record to tempRecord
	fseek(hashFile, -1 * (long)sizeof(RECORD), SEEK_CUR);			// move back one record size
	printf("%s,%s:%d\n", tempRecord.id, tempRecord.name, tempRecord.qty);	// print the id, name, and qty of the record
}