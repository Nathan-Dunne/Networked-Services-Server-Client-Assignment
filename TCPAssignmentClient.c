/*
Author: Nathan Dunne
Date: 22/11/2018i
Purpose: Setup a socket and communicate on that socket.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"
#include <arpa/inet.h>
#include <string.h>

void recvFromSocket(int socket, char *recv_string);
void sendToSocket(int socket, char *send_string);
void cleanString(char *string_to_clean, ssize_t numBytesToSearch);

char *reverseString(char *word, char *reversed_word, size_t size);

#define MAX_REQUEST_STRING_SIZE 96

// Preset port as directed by assignment.
const char *TCP_PORT = "48031";


int main(int argc, char *argv[])
{
	if (argc > 3 || argc < 2)
	{             // Test for correct number of arguments
		DieWithUserMessage("Parameter(s)",
		                   "<Server Address/Name> [<Server Port/Service> OR <none>]");
	}

	const char *server = argv[1];     // First arg: server address/name
	// Second arg (optional): server port/service. Will default to assignment port 48031.
	const char *service = (argc == 3) ? (argv[2]) : TCP_PORT;

	// Create a TCP connected socket.
	int socket = SetupTCPClientSocket(server, service);

	if (socket < 0)
	{
		DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");
	}

	// Retrieve the local IP and port details of the client socket.
	int len = sizeof(struct sockaddr);
	struct sockaddr_in my_address;
	char client_IP[16];
	getsockname(socket, (struct sockaddr *) &my_address, &len);
	inet_ntop(AF_INET, &my_address.sin_addr, client_IP, sizeof(client_IP));
	unsigned int client_port = ntohs(my_address.sin_port);

	printf("Local ip address: %s\n", client_IP);
	printf("Local port: %u\n", client_port);

	int max_port_digits = 5;
	char client_port_char[max_port_digits];
	sprintf(client_port_char, "%u", client_port);

	// Assemble request string
	char request_string[MAX_REQUEST_STRING_SIZE] = "netsvr type0 ndunne ";
	strcat(request_string, client_IP);
	strcat(request_string, "-");
	strcat(request_string, client_port_char);

	// SEND assembled request string to socket.
	sendToSocket(socket, request_string);
	cleanString(request_string, sizeof(request_string));
	printf("Sent request string to Server: %s\n", request_string);

	// RECV request string response from socket.
	char request_string_response[BUFSIZE];
	recvFromSocket(socket, request_string_response);
	printf("Server Response: %s\n", request_string_response);

	// SEND greeting response to Server.
	char phrase_request[BUFSIZE] = "Greeting received, please send phrase to reverse and return.";
	sendToSocket(socket, phrase_request);
	cleanString(phrase_request, sizeof(phrase_request));
	printf("Sending phrase request to server: %s\n", phrase_request);

	// RECV phrase from server.
	char server_phrase[BUFSIZE];
	recvFromSocket(socket, server_phrase);
	printf("Server Response: %s\n", server_phrase);

	// Reverse phrase.
	char *word = server_phrase;
	size_t rlen = strlen(word);
	char *reversedPhrase = reverseString(word, word, rlen);

	// SEND reversed phrase to server.
	sendToSocket(socket, reversedPhrase);
	cleanString(reversedPhrase, BUFSIZE);
	printf("Sending reverse of phrase to server: %s\n", reversedPhrase);

	// Close sending side of connection.
	shutdown(socket, SHUT_WR);
	printf("Shutdown client sending.\n");

	// RECV outcome from server.
	char server_outcome[BUFSIZE];
	recvFromSocket(socket, server_outcome);
	printf("Server outcome: %s\n", server_outcome);
}

/*
	Send a text string to the socket given the socket ID to send it on.
*/
void sendToSocket(int socket, char *send_string)
{
	int flags = 0;

	// Concatenate an end-of-line marker to the end of the string.
	char *end_of_line_marker = "\r\n";
	strcat(send_string, end_of_line_marker);

	// Get the length of the string and send it off.
	size_t send_string_length = strlen(send_string);
	ssize_t numBytesSent = send(socket, send_string, send_string_length, flags);

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
	Receieve a text string from a socket given the socket ID to receive it from.
*/
void recvFromSocket(int socket, char *recv_string)
{
	int flags = 0;

	// BUFSIZE-1 is to make room for a NULL terminator to be put at the end of the string.
	ssize_t numBytesRcvd = recv(socket, recv_string, BUFSIZE-1, flags);

	if (numBytesRcvd < 0)
	{
		DieWithSystemMessage("recv() failed");
	}
	else if(recv_string[0] == ' ') // If there is whitespace at the start of the line we should raise an error
	{
		DieWithSystemMessage("Syntax ERROR, first character of recv line is whitespace.");
	}
	else if(numBytesRcvd > 0)
	{
		// Before doing anything with the string it needs to be cleaned and setup for use.
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
