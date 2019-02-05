/*
*	File: encode.c
*	Name: Charlotte Murphy
*	Assignment: VPI Technology Embedded Software Engineer "Homework"
*	Date: February 4, 2019
*	Desc: Library for otp_enc_d.c and otp_enc.c
*		Based on CS344 assignment, Program4, dated June 9, 2017
*	Usage: For use with encoding client otp_enc.c and encoding server otp_enc_d.c
*/
#include "encode.h"

//Valid characters for plain text, key, and cipher text
char*	LETTERS = " ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 

/*---------------------------------------------------------------------------------*/
/*Client functions*/

// Function: initClientInput
// Initialize structs to store plain text and key. Set values for filename and buffsize.
// Set line, charsEntered, and length to NULL or 0; 
// INPUT p gets filename stored at arg1 and INPUT k gets filename stored at arg2
// Pre: Two INPUT structs declared
// Post: Structs initialized and respective filenames stored
void initClientInput(INPUT* p, char* arg1, INPUT* k, char* arg2) {
	p->fileName = arg1;
	p->line = NULL;
	p->bufsize = 256;
	p->charsEntered = 0;
	p->length = 0;

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
void getLineFromFile(INPUT* input) {
	// open file for reading
	FILE* fp = fopen(input->fileName, "r+");
	if (fp == (NULL)) { fprintf(stderr, "CLIENT: No such file\n"); exit(1); }

	// initialize values and memory for input
	input->charsEntered = 0;
	input->line = (char*)malloc((input->bufsize + 1) * sizeof(char));
	if (input->line == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(input->line, '\0', sizeof(input->line));

	// read line from file, save to input->line 
	// close file
	input->charsEntered = getline(&(input->line), &input->bufsize, fp);
	if (input->charsEntered == (-1)) { fprintf(stderr, "CLIENT: getline() failed\n"); exit(1); }
	fclose(fp);

	// remove trailing newlines
	// input->length gets length of input->line 
	input->line[strcspn(input->line, "\n")] = '\0';
	input->length = strlen(input->line);
}


// Function validateInput
// Ensures server only encodes data from an ENC client
// Ensures only valid characters - spaces or capital letters - are received
// Ensures the key is at least as long as the plain text
// If any condition is fasle, writes error to stderr then exits
// Pre: successful calls to getLineFromFile() for p->fileName and k->fileName
// Post: Client continues
void validateInput(INPUT* p, INPUT* k) {
	//ensure key is at least as long as plain text
	if (p->length > k->length) { fprintf(stderr, "CLIENT: Key is too short\n"); exit(1); }
	
	//check each char of plain text and key for invalid chars
	else {
		// array of 2 input pointers gets p and k
		INPUT* inputArr[2] = { p, k };
		int i , j;
		char c; // char var for comparison

		// for each input
		for (i = 0; i < 2; i++) {
			// check each char input->line for invalid chars - anything that's not a space or a capital letter
			char* tLine = inputArr[i]->line;
			int tLen = inputArr[i]->length;
			for (j = 0; j < tLen; j++) {
				c = tLine[j];
				if ((c < 65 && c != 32) || 90 < c) { //if c is not a space or a capital letter
					fprintf(stderr, "CLIENT: %s contains invalid characters\n", inputArr[i]->fileName); exit(1);
				}
			}
		}
	}
}


// Function: initClientMessage
// Initializes variables for MESSAGE* c to hold a complete message of plain text + key
//	the size of p->line + k->line + extra chars to hold delims, termdelim, and a null char
// Initializes variables for MESSAGE* e to hold a cipher text
//	the size of p->line + extra chars for termdelim and a null char
// PRE: validateInput() returned normally - file input validated successfully
// POST: MESSAGE c and e are initialized and ready to store data
void initClientMessages(MESSAGE* c, MESSAGE* e, INPUT* p, INPUT* k) {
	//allocate & clear message char array
	c->bufsize = p->charsEntered + k->charsEntered + 8;
	c->message = (char*)malloc(c->bufsize * sizeof(char));
	if (c->message == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(c->message, '\0', sizeof(c->message));
	c->length = 0;

	e->bufsize = p->charsEntered + 3;
	e->message = (char*)malloc((e->bufsize) * sizeof(char));
	if (e->message == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(e->message, '\0', sizeof(e->message));
	e->length = 0;
}


// Function: buildCompleteMessage
//  Concatenates identifier + delim + p->line(plain text) + delim + k->line(key) + termDelim
// Pre: succssful call to initClientMessages
// Post: MESSAGE c contains complete input from files concatenated in one line
void buildCompleteMessage(MESSAGE* c, INPUT* p, INPUT* k)
{
	//build message string
	strcpy(c->message, "ENC");
	strcat(c->message, "@@");
	strcat(c->message, p->line);
	strcat(c->message, "@@");
	strcat(c->message, k->line);
	strcat(c->message, "&&");
	c->length = strlen(c->message);
}


// Function validateServerConnection
// Ensures data was sent to correct server type - ENC
// Pre: Data successfully sent to server and server successfully returned encoded text
// Post: If server is ENC, program continues, else, error written to stderr and client exits 
void validateServerConnection(MESSAGE* e) {
	if (strcmp(e->message, "NO&&") == 0) { fprintf(stderr, "CLIENT: Server rejected connection, incorrect client type\n"); exit(1); }
}


// Function printMessage
// Removes trailing term chars and sends string stored in m->message to stdout
// Pre: MESSAGE m is initialized and a string is stored at m->message
// Post: term chars trimmed from m->message
void printMessage(MESSAGE * m) {
	if (m->message == NULL) { fprintf(stderr, "CLIENT: Message is NULL string\n"); exit(1); }

	//remove terminating chars before print and clear output buffer
	m->message[strcspn(m->message, "&")] = '\0';
	m->message[strcspn(m->message, "&")] = '\0';
	printf("%s\n", m->message); fflush(stdout);
}


/*---------------------------------------------------------------------------------*/
/*Server functions*/

// Function: initServer
// Initializes strings identifier, delim, and term delim
// Initializes Struct for incoming message and struct for cipher text
// Sets bufsize and allocates & clears memory for for both structs to 
// store message and cipher text
// Pre: MESSAGE in and CIPHER enc declared
// Post in and enc have memory and variables initialized
void initServer(MESSAGE* in, CIPHER* enc) {
	memset(identifier, '\0', sizeof(identifier));
	strcpy(identifier, "ENC@@");
	memset(delim, '\0', sizeof(delim));
	strcpy(delim, "@");
	memset(endDelim, '\0', sizeof(endDelim));
	strcpy(endDelim, "&&");

	in->bufsize = 170000;
	in->message = (char*)malloc((in->bufsize + 1) * sizeof(char));
	if (in->message == NULL) { fprintf(stderr, "SERVER: Memory not allocated\n"); exit(1); }
	memset(in->message, '\0', sizeof(in->message));
	in->length = 0;

	enc->bufsize = 85000;
	enc->plainText = enc->key = enc->cipherText = NULL;
	enc->pLength = enc->kLength = 0;
}


// Function: validateMessage
// Ensures in->message ends with termDelim - to see if entire message received
// Ensures message receieved from an ENC client
// return 0 for true - per call to strncmp()
// Pre: message received from client
// Post: valid - returns 0 for true
// invalid - sends error to strerr and exits
int validateMessage(MESSAGE* in) {
	int valid = 1;
	in->length = strlen(in->message);

	//check for termdelim
	if(in->message[in->length - 1] != '&') { fprintf(stderr, "SERVER: Did not receive entire message from client\n"); exit(1); }
	
	//check for proper client identifier
	valid = strncmp(in->message, identifier, strlen(identifier));

	return valid;
}


// Function: parseMessage
// stores copy of in->message for tokenizing without compromising original data
// deconstructs message received from client by tokenizing by delim
// ignores first token
// stores second token as enc->plainText
// allocates & clears memory for enc->cipherText
// stores third token as enc->key
// Pre: successful call to validateMessage - server received valid message from client
// Post: enc->plainText and enc->key hold plain text and key received from client
// enc->cipherText points to allocated and cleared memory to eventually store encoded plain text
void parseMessage(MESSAGE* in, CIPHER* enc) {
	char tMsg[170000];
	memset(tMsg, '\0', sizeof(tMsg));
	char *token = NULL;
		
	strcpy(tMsg, in->message);
	//store identifier
	token = strtok(tMsg, "@");
	if (token == NULL) { fprintf(stderr, "SERVER: Identifier strtok failed\n"); exit(1); }
	
	//store plain text
	token = strtok(NULL, "@");
	if (token == NULL) { fprintf(stderr, "Plain text strtok failed\n"); exit(1); }
	enc->pLength = strlen(token);
	enc->plainText = (char*)malloc((enc->pLength + 1) * sizeof(char)); //allocate mem for plain text
	if (enc->plainText == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(enc->plainText, '\0', sizeof(enc->plainText)); //clear plaintext memory
	strcpy(enc->plainText, token); //store plain text

	//allocate & clear memory for cipher text - the same size as plain text + chars for termDelim and NULL
	enc->cipherText = (char*)malloc((enc->pLength + 3) * sizeof(char));
	if (enc->cipherText == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(enc->cipherText, '\0', sizeof(enc->cipherText));

	//store key
	token = strtok(NULL, "@&");
	enc->kLength = strlen(token);
	if (token == NULL) { fprintf(stderr, "Key strtok failed\n"); exit(1); }
	enc->key = (char*)malloc((enc->kLength + 1) * sizeof(char)); //allocate mem for key
	if (enc->key == NULL) { fprintf(stderr, "CLIENT: Memory not allocated\n"); exit(1); }
	memset(enc->key, '\0', sizeof(enc->key)); //clear key memory
	strcpy(enc->key, token); //store key
}


// Function: encodeMessage
// Generates cipher text based on plain text and key received from client
// Each enc->plainText character is compared to LETTERS to determine the pIndex of the 
// corresponding LETTERS char
// Each enc->key character is compared to LETTERS to determine the kIndex of the 
// corresponding LETTERS char
// If the corresponding LETTERS index of either letter is outside the range of [0 .. 26], error and exit
// The index values are added together to determine the index of the coded letter, 
// Ex. coded letter = LETTERS[cIdx] = LETTERS[pIdx 9 + kIdx 5] = LETTERS[14]
// If cIdx > 26, subtract 27 to "wrap around" back to beginning of the LETTERS array
// Ex. coded letter = LETTERS[cIdx] = LETTERS[pIdx 9 + kIdx 25] = LETTERS[34] = LETTERS[34 - 27] = LETTERS[7]
// Each coded letter is stored in enc->cipherText
// Pre: successful call to parseMessage() - data deconstructed and stored in enc->plainText and enc->key
// Post: enc->cipherText gets encoded plain text
void encodeMessage(CIPHER* enc) {
	char* p = enc->plainText;
	char* c = enc->cipherText;
	char* k = enc->key;
	int	pIdx = -5;
	int cIdx = -5;
	int kIdx = -5;
	int len = enc->pLength;
	char find[2]; memset(find, '\0', sizeof(find));
	memset(enc->cipherText, '\0', sizeof(enc->cipherText));

	int i;
	//For each letter in enc->plainText and enc->Key, up to length of enc->plainText...
	for (i = 0; i < len; i++)  	{
		if (p[i] == '\0' || k[i] == '\0') { fprintf(stderr, "SERVER: String error\n"); exit(1); }
		
		//plain text: determine index of LETTERS char that corresponds to letter at p[i]
		find[0] = p[i]; //store plain text letter in string for call to strcspn
		pIdx = (int)strcspn(LETTERS, find); //return index of LETTERS char corresponding to p[i]

		//key: determine index of LETTERS char that corresponds to letter at k[i]
		find[0] = k[i]; //store plain text letter in string for call to strcspn
		kIdx = (int)strcspn(LETTERS, find);
		
		//check to see if each letter was actually located in LETTERS[]
		if(pIdx > 26 || kIdx > 26) { fprintf(stderr, "SERVER: Invalid character\n"); exit(1); }

		//determine resulting code letter index
		cIdx = pIdx + kIdx;	
		if (cIdx > 26) cIdx -= 27; //subtract 27 if cIdx is greater than index of last LETTERS[] char - wraps around to beginning
		
		//store code letter at enc->cipherText[i]
		c[i] = LETTERS[cIdx];
	}
}


// Function: invalidClientType
// If client is not "ENC" types, sends "NO" to client via CIPHER enc->cipherText
// PRE:
// POST:
void invalidClientType(CIPHER* enc) {
	// Allocate memory
	// Copy "NO" to cipherText
	// Add endDelim 
	enc->cipherText = (char*)malloc(7 * sizeof(char));
	memset(enc->cipherText, '\0', sizeof(enc->cipherText));
	if (enc->cipherText == NULL) { fprintf(stderr, "SERVER: Memory not allocated\n"); exit(1); }
	strcpy(enc->cipherText, "NO"); //server reply if client is not ENC
	strcat(enc->cipherText, endDelim);
}


/*---------------------------------------------------------------------------------*/
/*Manage memory functions*/

// Function: cleanupInput
// Frees memory allocated for an INPUT struct
// PRE: INPUT i exists and memory successfully allocated for i->line
// POST: Memory allocated for i->line now free
void cleanupInput(INPUT* i) {
	free(i->line);
}

// Function: cleanupMessage
// Frees memory allocated for a MESSAGE struct
// PRE: MESSAGE m exists and memory successfully allocated for m->message
// POST: Memory allocated for m->message now free
void cleanupMessage(MESSAGE* m) {
	free(m->message);
}

// Function: cleanupCipher
// PRE: CIPHER c exists and memory for c->plainText and c->cipherText successfully allocated
// POST: Memory allocated for c->plainText and c->cipherText now free
void cleanupCipher(CIPHER* c) {
	free(c->plainText);
	free(c->cipherText);
	free(c->key);
}
