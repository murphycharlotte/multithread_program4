/*
*	File: decode.h
*	Name: Charlotte Murphy
*	Course: CS 344-400
*	Assignment: Program 4
*	Due: June 9, 2017
*	Desc: Header file
*	Usage: For use with decode.c
*/
#pragma once
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>

#ifndef DECODE
#define DENCODE

char		identifier[6], delim[3], endDelim[3];

typedef struct input {
	char*	fileName;
	char*	line;
	size_t	bufsize;
	size_t	charsEntered;
	int		length;
} INPUT;

typedef struct  message {
	size_t	bufsize;
	char*	message;
	int		length;
} MESSAGE;

typedef struct  cipher {
	size_t	bufsize;
	char*	cipherText;
	int		cLength;
	char*	key;
	int		kLength;
	char*	plainText;
} CIPHER;

void		initClientInput(INPUT* ci, char* arg1, INPUT* k, char* arg2);
void		initClientMessages(MESSAGE* c, MESSAGE* d, INPUT* ci, INPUT* k);
void		getLineFromFile(INPUT* data);
void		validateFileInput(INPUT* ci, INPUT* k);
void		buildCompleteMessage(MESSAGE* c, INPUT* cInput, INPUT* kInput);
void		validateServerConnection(MESSAGE* d);
void		printMessage(MESSAGE* m);


void		initServer(MESSAGE* in, CIPHER* dec);
int			validateMessage(MESSAGE* in);
void		parseMessage(MESSAGE* in, CIPHER* dec);
void		decodeMessage(CIPHER* dec);

#endif // !DECODE
