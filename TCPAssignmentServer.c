/*
Author: Nathan Dunne
Date: 22/11/2018i
Purpose: Setup a server socket and communicate on that socket.
*/

#include <stdio.h>
#include "Practical.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <arpa/inet.h>

void handleTCPClient(int client_socket);
void recvFromClient(int client_socket, char *recv_string);
void sendToClient(int client_socket, char *send_string);
void tokenizeString(char *string_to_tokenize, char *string_tokens[], char* delimter);
void cleanString(char *string_to_clean, ssize_t numBytesToSearch);

char *reverseString(char *word, char *reversed_word, size_t size);
unsigned int generateRandomNumber();

// Preset port as directed by assignment.
const char *TCP_PORT = "48031";

int main(int argc, char *argv[])
{
	// Test for correct number of arguments if any.
	if (argc > 2)
	{                     
		DieWithUserMessage("Parameter(s)", "<Server Port/Service> OR <none>");
	}

	// Use the assignment preset port if no argument was passed in.
	const char *service = (argc == 2) ? (argv[1]) : TCP_PORT;

	// Create socket for incoming connections
	int servSock = SetupTCPServerSocket(service);

	if (servSock < 0)
	{
		DieWithUserMessage("SetupTCPServerSocket() failed", service);
	}

	while(true)    // Run forever
	{
		// New connection creates a connected client socket
		int client_socket = AcceptTCPConnection(servSock);

		// Handle the client communications.
		handleTCPClient(client_socket);         
	}
}

void handleTCPClient(int client_socket)
{
	// Retrieve connected socket information.
	int len = sizeof(struct sockaddr);
	struct sockaddr_in client_address;
	char client_IP[16];

	getpeername(client_socket, (struct sockaddr *) &client_address, &len);
	inet_ntop(AF_INET, &client_address.sin_addr, client_IP, sizeof(client_IP));
	unsigned int client_port = ntohs(client_address.sin_port);

	// Assemble endpoint identifier information into a string.
	int max_port_digits = 5;
	char client_port_char[max_port_digits];
	sprintf(client_port_char, "%u", client_port);

	char real_client_endpoint_indentifier[21] = "";
	strcat(real_client_endpoint_indentifier, client_IP);
	strcat(real_client_endpoint_indentifier, "-");
	strcat(real_client_endpoint_indentifier, client_port_char);

	// RECV request string from client.
	char client_request_string[BUFSIZE];
	recvFromClient(client_socket, client_request_string);
	printf("Recieved request string from client: %s\n", client_request_string);

	// Tokenize request string.
	char *delimter = " ";
	char *request_string_tokens[4];
	tokenizeString(client_request_string, request_string_tokens, delimter);

	// Retrieve individual tokens.
	char *server_name = request_string_tokens[0];
	char *server_type = request_string_tokens[1];
	char *user_name = request_string_tokens[2];
	char *received_client_endpoint_identifier = request_string_tokens[3];

	
	int isClientCorrect = strcmp(real_client_endpoint_indentifier,
	                             received_client_endpoint_identifier);
	// Compare the endpoint identifier to that of what the client sent in the request string.
	if(isClientCorrect != 0)
	{
		DieWithSystemMessage("Connection ERROR, improper endpoint identifier.");
	}
	else
	{
		// Assemble client greeting with tokens.
		char greeting[BUFSIZE] = "hello ";
		strcat(greeting, real_client_endpoint_indentifier);
		strcat(greeting, ", welcome to the ");
		strcat(greeting, server_name);
		strcat(greeting, " server.");

		// SEND assembled greeting to client.
		sendToClient(client_socket, greeting);
		printf("Sent greeting to client at: %s\n", real_client_endpoint_indentifier);

		// RECV greeting response from client.
		char greeting_response[BUFSIZE];
		recvFromClient(client_socket, greeting_response);
		printf("Client greeting response: %s\n", greeting_response);

		// SEND phrase to client.
		char phrase[BUFSIZE] = "Networks assignment complete.";
		sendToClient(client_socket, phrase);
		cleanString(phrase, sizeof(phrase));
		printf("Sent phrase: '%s' to client: %s\n", phrase, real_client_endpoint_indentifier);

		// RECV phrase reversal from client.
		char phrase_response[BUFSIZE];
		recvFromClient(client_socket, phrase_response);
		printf("Client reversal response: %s\n", phrase_response);

		char *word = phrase;
		size_t rlen = strlen(word);
		char *reversedPhrase = reverseString(word, word, rlen);

		// If the phrase reversed and phrase response aren't the same.
		if(strcmp(reversedPhrase, phrase_response) != 0)
		{
			char error_reversal[BUFSIZE] = "ERROR_invalid_phrase_reversal";
			sendToClient(client_socket, error_reversal);
			cleanString(phrase, sizeof(error_reversal));
			printf("Sent ERROR to client: %s\n", error_reversal);

			close(client_socket);
		}
		else // Send OK and cookie.
		{
			// Generate cookie from random number.
			unsigned int cookie = generateRandomNumber();
			char cookie_string[BUFSIZE];
			size_t cookie_string_length = strlen(cookie_string);
			snprintf(cookie_string, BUFSIZE, "%u", cookie);

			char outcome_string[BUFSIZE] = "OK ";
			strcat(outcome_string, cookie_string);

			// SEND cookie to client.
			sendToClient(client_socket, outcome_string);
			cleanString(outcome_string, BUFSIZE);
			printf("Sent cookie: '%s' to client: %s\n", outcome_string, real_client_endpoint_indentifier);
		}
	}
}

/*
	Send a text string to the client given the socket to send it on.
*/
void sendToClient(int client_socket, char *send_string)
{
	// Concatenate an end-of-line marker to the end of the string.
	char *end_of_line_marker = "\r\n"; 
	strcat(send_string, end_of_line_marker);

	int flags = 0;

	// Get the length of the string and send it off.
	size_t send_string_length = strlen(send_string);
	ssize_t numBytesSent = send(client_socket, send_string, send_string_length, flags);

	if (numBytesSent < 0)
	{
		DieWithSystemMessage("send() failed");
	}
	else if(send_string[0] == ' ') // If there is whitespace at the start of the line we should raise an error
	{
		DieWithSystemMessage("Syntax ERROR, first character of send line is whitespace.");
	}
	else if (numBytesSent != send_string_length)
	{
		DieWithUserMessage("send()", "sent unexpected number of bytes");
	}
}

/*
	Receieve a text string from the client given the socket to receive it from.
*/
void recvFromClient(int client_socket, char *recv_string)
{
	int flags = 0;

	// BUFSIZE-1 is to make room for a NULL terminator to be put at the end of the string.
	ssize_t numBytesRcvd = recv(client_socket, recv_string, BUFSIZE-1, flags);

	if (numBytesRcvd < 0 )
	{
		DieWithSystemMessage("recv() failed");
	}
	else if(recv_string[0] == ' ') // If there is whitespace at the start of the line we should raise an error
	{
		DieWithSystemMessage("Syntax ERROR, first character of recv line is whitespace.");
	}
	else if(numBytesRcvd > 0)
	{	// Before doing anything with the string it needs to be cleaned and setup for use.
		cleanString(recv_string, numBytesRcvd); 
	}
}
/*
	Add a NULL termiantor to the end of a string and clean it of the newline and carriage return
	escape characters.
*/
void cleanString(char *string_to_clean, ssize_t numBytesToSearch)
{
	string_to_clean[numBytesToSearch] = '\0';

	for(int i = 0; i!= numBytesToSearch; i++)
	{
		if(string_to_clean[i] == '\n')
		{
			string_to_clean[i] = '\0';
		}

		if(string_to_clean[i] == '\r')
		{
			string_to_clean[i] = '\0';
		}
	}
}

/*
	Split the given string into tokens by a given delimter and put them into the given 2D array for extraction.
*/
void tokenizeString(char *string_to_tokenize, char *string_tokens[], char* delimter)
{
	int i = 0;
	char *token = strtok (string_to_tokenize, delimter);

	while (token != NULL)
	{
		string_tokens[i++] = token;
		token = strtok (NULL, delimter);
	}
}

char *reverseString(char *word, char *reversed_word, size_t size)
{
	size_t index = 0;

	// Put a NULL terminator at the end of the reversed string containter before use.
	reversed_word[size] = '\0'; 

	// For each index in the word, starting at the end index.
	while (size-- > index) 
	{
		const char temp = word[index]; // Store a temp char equal to the value at the index
		reversed_word[index++] = word[size]; // Put the end character of the word at the incrementing index
		reversed_word[size] = temp; // Put the temp char at the decrementing size index.
	}

	return reversed_word;
}

unsigned int generateRandomNumber()
{
	// Use Linux's urandom file to generate a random source
	int i, randomSrc = open("/dev/urandom", O_RDONLY);
	unsigned int random;

	int iterations = 1000;

	for(i=0; i<iterations; i++)
	{
		// Generate the random int from the source.
		read(randomSrc, &random, sizeof(unsigned int));
	}

	close(randomSrc);

	return random;
}
