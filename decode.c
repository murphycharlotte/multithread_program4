/*
*	File: decode.c
*	Name: Charlotte Murphy
*	Course: CS 344-400
*	Assignment: Program 4
*	Due: June 9, 2017
*	Desc: Library for otp_enc_d.c and otp_enc.c
*	Usage: For use with encoding client otp_dec.c and encoding server otp_dec_d.c
*/

#include "decode.h"

//Valid characters for plain text, key, and cipher text

char*	LETTERS = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";


// Function: initClientInput
// Initialize structs to store cipher text and key. Set values for filename and buffsize.
// Set line, charsEntered, and length to NULL or 0; 
// INPUT ci gets filename stored at arg1 and INPUT k gets filename stored at arg2
// Pre: Two INPUT structs declared
// Post: Structs initialized and respective filenames stored

void initClientInput(INPUT* ci, char* arg1, INPUT* k, char* arg2)
{
	ci->fileName = arg1;
	ci->line = NULL;
	ci->bufsize = 256;
	ci->charsEntered = 0;
	ci->length = 0;

	k->fileName = arg2;
	k->line = NULL;
	k->bufsize = 256;
	k->charsEntered = 0;
	k->length = 0;
}


// Function: getLineFromFile
// Reads one line of text from file named in struct paramenter
// Stores text in the struct's line variable
// Pre: call initClientInput()
// Post: Struct Param has a string stored in it's line var and the appropriate 
// values for length and charsEntered 

void getLineFromFile(INPUT* input)
{
	FILE* fp = fopen(input->fileName, "r+");
	if (fp == (NULL)) { fprintf(stderr, "CLIENT: No such file"); exit(1); }

	input->bufsize = 256;
	input->charsEntered = 0;
	input->line = (char*)malloc((input->bufsize + 1) * sizeof(char));
	if (input->line == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }

	memset(input->line, '\0', sizeof(input->line));
	input->charsEntered = getline(&(input->line), &input->bufsize, fp);
	if (input->charsEntered == (-1)) { fprintf(stderr, "CLIENT: getline() failed\n"); exit(1); }
	fclose(fp);

	//remove trailing newlines
	input->line[strcspn(input->line, "\n")] = '\0';
	input->length = strlen(input->line);

}


// Function validateInput
// Ensures server only encodes data from an DEC client
// Ensures only valid characters - spaces or capital letters - are received
// Ensures the key is longer than the plain text
// If any condition is fasle, writes error to stderr then exits
// Pre: successful calls to getLineFromFile() for ci->fileName and k->fileName
// Post: Client continues

void validateFileInput(INPUT* ci, INPUT* k)
{
	int valid = 1;
	//check length
	if (ci->length > k->length) { valid = 0; fprintf(stderr, "CLIENT: Key too short\n"); exit(1); }
	//check for invalid chars
	else
	{
		INPUT* inputArr[2] = { ci, k };
		int i , j;
		char c;
		for (i = 0; i < 2; i++)
		{
			char* tLine = inputArr[i]->line;
			int tLen = inputArr[i]->length;
			for (j = 0; j < tLen; j++)
			{
				c = tLine[j];
				if ((c < 65 && c != 32) || 90 < c) //if c is not a space or a capital letter
				{
					valid = 0; fprintf(stderr, "CLIENT: %s contains invalid characters\n", inputArr[i]->fileName); exit(1);
				}
			}
		}
	}
}


// Function: initClientMessage
// Initializes variables for MESSAGE* c to hold a complete message of cipher text + key
//	the size of ci->line + k->line + extra chars to hold delims, termdelim, and a null char
// Initializes variables for MESSAGE* dec to hold plain text
//	the size of ci->line + extra chars for termdelim and a null char
// PRE: validateInput() returned normally - file input validated successfully
// POST: MESSAGE c and dec are initialized and ready to store data

void initClientMessages(MESSAGE* c, MESSAGE* dec, INPUT* ci, INPUT* k)
{
	//allocate & clear message char array
	c->bufsize = ci->charsEntered + k->charsEntered + 8;
	c->message = (char*)malloc(c->bufsize * sizeof(char));
	if (c->message == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(c->message, '\0', sizeof(c->message));
	c->length = 0;

	dec->bufsize = c->bufsize;
	dec->message = (char*)malloc((dec->bufsize) * sizeof(char));
	if (dec->message == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(dec->message, '\0', sizeof(dec->message));
	dec->length = 0;
}


// Function: buildCompleteMessage
//  Concatenates identifier + delim + ci->line(plain text) + delim + k->line(key) + termDelim
// Pre: succssful call to initClientMessages
// Post: MESSAGE c contains complete input from files concatenated in one line

void buildCompleteMessage(MESSAGE* c, INPUT* ci, INPUT* k)
{
	//build message string
	strcpy(c->message, "DEC");
	strcat(c->message, "@@");
	strcat(c->message, ci->line);
	strcat(c->message, "@@");
	strcat(c->message, k->line);
	strcat(c->message, "&&");
	c->length = strlen(c->message);
}


// Function validateServerConnection
// Ensures data was sent to correct server type - DEC
// Pre: Data successfully sent to server and server successfully returned encoded text
// Post: If server is DEC, program continues, else, error written to stderr and client exits 

void validateServerConnection(MESSAGE* d)
{
	if (strcmp(d->message, "NO&&") == 0) { fprintf(stderr, "CLIENT: Server rejected connection\n"); exit(1); }
}

void printMessage(MESSAGE * m)
{
	//remove terminating chars before printing
	m->message[strcspn(m->message, "&")] = '\0';
	m->message[strcspn(m->message, "&")] = '\0';
	printf("%s\n", m->message);
}

/*---------------------------------------------------------------------------------*/
/*Server functions*/


// Function: initServer
// Initializes strings identifier, delim, and term delim
// Initializes Struct for incoming message and struct for plain text
// Sets bufsize and allocates & clears memory for for both structs to 
// store message and plain text
// Pre: MESSAGE in and CIPHER dec declared
// Post in and dec have memory and variables initialized

void initServer(MESSAGE* in, CIPHER* dec)
{
	memset(identifier, '\0', sizeof(identifier));
	strcpy(identifier, "DEC@@");
	memset(delim, '\0', sizeof(delim));
	strcpy(delim, "@");
	memset(endDelim, '\0', sizeof(endDelim));
	strcpy(endDelim, "&&");

	in->bufsize = 170000;
	in->message = (char*)malloc((in->bufsize + 1) * sizeof(char));
	if (in->message == NULL) { fprintf(stderr, "SERVER: Memory not allocated\n"); exit(1); }
	memset(in->message, '\0', sizeof(in->message));
	in->length = 0;

	dec->bufsize = 85000;
	dec->plainText = dec->key = dec->cipherText = NULL;
	dec->cLength = dec->kLength = 0;
}


// Function: validateMessage
// Ensures in->message ends with termDelim - to see if entire message received
// Ensures message receieved from an DEC client
// return 0 for true - per call to strncmp()
// Pre: message received from client
// Post: valid - returns 0 for true
// invalid - sends error to strerr and exits

int validateMessage(MESSAGE* in)
{
	int valid = 1;
	in->length = strlen(in->message);

	//check for termdelim
	if(in->message[in->length - 1] != '&') { fprintf(stderr, "SERVER: Did not receive entire message from client\n"); exit(1); }

	valid = strncmp(in->message, identifier, strlen(identifier));

	return valid;
}


// Function: parseMessage
// stores copy of in->message for tokenizing without compromising original data
// deconstructs message received from client by tokenizing by delim
// ignores first token
// stores second token as dec->cipherText
// allocates & clears memory for dec->cipherText
// stores third token as dec->key
// Pre: successful call to validateMessage - server received valid message from client
// Post: dec->cipherText and dec->key hold plain text and key received from client
// dec->plainText points to allocated and cleared memory to eventually store decoded cipher text

void parseMessage(MESSAGE* in, CIPHER* dec)
{
	char tMsg[170000];
	memset(tMsg, '\0', sizeof(tMsg));
	char *token = NULL;
		
	strcpy(tMsg, in->message);
	//store identifier
	token = strtok(tMsg, "@");
	if (token == NULL) { fprintf(stderr, "SERVER: Identifier strtok failed\n"); exit(1); }
	
	//store cipher text
	token = strtok(NULL, "@");
	if (token == NULL) { fprintf(stderr, "Plain text strtok failed\n"); exit(1); }
	dec->cLength = strlen(token);
	dec->cipherText = (char*)malloc((dec->cLength + 1) * sizeof(char));
	if (dec->cipherText == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(dec->cipherText, '\0', sizeof(dec->cipherText));
	strcpy(dec->cipherText, token);

	//allocate & clear memory for plain text
	dec->plainText = (char*)malloc((dec->cLength + 3) * sizeof(char));
	if (dec->plainText == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(dec->plainText, '\0', sizeof(dec->plainText));

	//store to key
	token = strtok(NULL, "@&");
	dec->kLength = strlen(token);
	if (token == NULL) { fprintf(stderr, "Key strtok failed\n"); exit(1); }
	dec->key = (char*)malloc((dec->kLength + 1) * sizeof(char));
	if (dec->key == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(dec->key, '\0', sizeof(dec->key));
	strcpy(dec->key, token);
}


// Function: decodeMessage
// Generates plain text based on cipher text and key received from client
// Each dec->cipherText character is compared to LETTERS to determine the cIndex of the 
// corresponding LETTERS char
// Each dec->key character is compared to LETTERS to determine the kIndex of the 
// corresponding LETTERS char
// If the corresponding LETTERS index of either letter is outside the range of [0 .. 26], error and exit
// The index values are subtracted to determine the index of the decoded letter, 
// Ex. decoded letter = LETTERS[pIdx] = LETTERS[cIdx 9 - kIdx 5] = LETTERS[4]
// If pIdx < 0, add 27 to "wrap around" back to beginning of the LETTERS array
// Ex. coded letter = LETTERS[pIdx] = LETTERS[cIdx 9 - kIdx 25] = LETTERS[-14] = LETTERS[-14 + 27] = LETTERS[13]
// Each coded letter is stored in dec->plainText
// Pre: successful call to parseMessage() - data deconstructed and stored in dec->cipherText and dec->key
// Post: dec->plainText gets decoded cipher text

void decodeMessage(CIPHER* dec)
{
	char* p = dec->plainText;
	char* c = dec->cipherText;
	char* k = dec->key;
	int	pIdx = -5;
	int cIdx = -5;
	int kIdx = -5;
	int len = dec->cLength;
	char find[2]; memset(find, '\0', sizeof(find));
	memset(dec->plainText, '\0', sizeof(dec->plainText));

	int i;
	//For each letter in dec->cipherText and dec->Key, up to length of dec->cipherText...
	for (i = 0; i < len; i++)
	{
		if (c[i] == '\0' || k[i] == '\0') { fprintf(stderr, "SERVER: String error\n"); exit(1); }

		//cipher text: determine index of LETTERS[] char that corresponds to letter at c[i]
		find[0] = c[i]; //store cipher text letter in string for call to strcspn
		cIdx = (int)strcspn(LETTERS, find); //return index of LETTERS char corresponding to c[i]

		//key: determine index of LETTERS char that corresponds to letter at k[i]
		find[0] = k[i]; //store plain text letter in string for call to strcspn
		kIdx = (int)strcspn(LETTERS, find);

		//check to see if each letter was actually located in LETTERS[]
		if (cIdx > 26 || kIdx > 26) { fprintf(stderr, "SERVER: Invalid character\n"); exit(1); }

		//determine resulting plain text letter index
		pIdx = cIdx - kIdx;
		if (pIdx < 0) pIdx += 27;

		//store plainText letter at dec->plainText[i]
		p[i] = LETTERS[pIdx];
	}
}