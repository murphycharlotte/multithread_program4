/*
*	File: keygen.c
*	Author: Charlotte Murphy
*	Course: CS 344-400 
*	Assignment: Program 4
*	Due: June 9, 2017
*	Desc: Writes a cipher key of n random characters, including a space or any 
*		capitals letter, terminated by a new line char to a file
*	Usage: For use with encoding/decoding client programs otp_enc.c and otp_dec.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
ASCII dec values:
	 space: 32
	A .. Z: 65 .. 90
*/

int main(int argc, char *argv[]) {

	if (argc <= 1) {
		printf("No arguments");
		exit(1);
	}  

	srand(time(NULL));

	//get number of characters to generate
	int keyLength = atoi(argv[1]);  

	char* key;
	//allocate and clear memory for key
	key = (char*)malloc(keyLength + 1);
	memset(key, '\0', sizeof(key));

	int i;
	//populate key one character at a time
	for (i = 0; i < keyLength; i++) {
		//random number from 0-26 (27 poss. chars) + 65 for offset of first ascii code for cap letters
		int asciiVal = rand() % 27 + 65;
		if (asciiVal < 91) { //asciiVal's 0 - 25 translate to 65-90, ascii codes for cap letters
			key[i] = asciiVal; //direct int to char assignment
		}
		else { //not a capital letter. assign ' ' char
			key[i] = ' '; //
		}
	}

	//send line + newline char to stdout
	printf("%s\n", key);

	free(key);
	return 0;
}

/*	Sources
*		https://stackoverflow.com/questions/2279379/how-to-convert-integer-to-char-in-c
*/