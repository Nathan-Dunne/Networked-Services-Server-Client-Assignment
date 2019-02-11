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

#define MAX_REQUEST_STRING_SIZE 96

char *reverse(char *word, char *reversed_word, size_t size);

int main(int argc, char *argv[])
{

		if (argc < 3 || argc > 4) // Test for correct number of arguments
				DieWithUserMessage("Parameter(s)",
				                   "<Server Address/Name> <Echo Word> [<Server Port/Service>]");

		char *server = argv[1]; // First arg: server address/name

		//const int MAX_REQUEST_STRING_SIZE = 96;
		char request_string[MAX_REQUEST_STRING_SIZE] = "netsvr type0 ndunne "; // Second arg: string to echo
		// Third arg (optional): server port/service
		char *service = (argc == 4) ? argv[3] : "echo";

		// Create a connected TCP socket
		int sock = SetupTCPClientSocket(server, service);
		if (sock < 0)
		{
				DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");
		}

		int len = sizeof(struct sockaddr);
		struct sockaddr_in my_address;

		char client_IP[16];

		getsockname(sock, (struct sockaddr *) &my_address, &len);
		inet_ntop(AF_INET, &my_address.sin_addr, client_IP, sizeof(client_IP));
		unsigned int myPort = ntohs(my_address.sin_port);

		printf("Local ip address: %s\n", client_IP);
		printf("Local port: %u\n", myPort);

		long long n;
		int count = 0;

		while(n!=0)
		{
				n/= 10;
				++count;
		}

		char myCharPort[count];

		sprintf(myCharPort, "%u", myPort);

		strcat(request_string, client_IP);
		strcat(request_string, "-");
		strcat(request_string, myCharPort);
		strcat(request_string, "\n");
		size_t request_string_length = strlen(request_string); // Determine input length

		// Send the request string to the server
		printf("Sending request string to Server:%s", request_string);
		ssize_t numBytes = send(sock, request_string, request_string_length, 0);
		if (numBytes < 0)
				DieWithSystemMessage("send() failed");
		else if (numBytes != request_string_length)
				DieWithUserMessage("send()", "sent unexpected number of bytes");

		// Receive the  string back from the server
		//unsigned int totalBytesRcvd = 0; // Count of total bytes received

		int bytesReceived = 0;

		char request_string_response[BUFSIZE];

		if ((numBytes = recv(sock, request_string_response, BUFSIZE - 1, 0)) <= 0)
		{
				DieWithSystemMessage("recv() failed or connection closed prematurely");
		}
		else
		{
				request_string_response[numBytes] = '\0'; // Terminate string.
				printf("Server Response: %s\n", request_string_response);
		}

		// Send greeting response to Server.
		char *greeting_reponse = "Greeting recieved, please send the phrase.\r\n";
		printf("Sending phrase request to server.\n");
		size_t greeting_reponse_length = strlen(greeting_reponse);
		numBytes = send(sock, greeting_reponse, greeting_reponse_length, 0);
		if (numBytes < 0)
				DieWithSystemMessage("send() failed");
		else if (numBytes != greeting_reponse_length)
				DieWithUserMessage("send()", "sent unexpected number of bytes");

		// Recieve phrase from server.
		char server_phrase[BUFSIZE];
		if ((numBytes = recv(sock, server_phrase, BUFSIZE - 1, 0)) <= 0)
		{
				DieWithSystemMessage("recv() failed or connection closed prematurely");
		}
		else
		{
				server_phrase[numBytes] = '\0'; // Terminate string.
        printf("Recieved server phrase: %s\n", server_phrase);
        size_t phrase_length = strlen(server_phrase);
		}

    for(int i = 0; i!= sizeof(server_phrase); i++)
    {
        if(server_phrase[i] == '\n' || server_phrase[i] == '\r')
        {
            server_phrase[i] = '\0';
        }
    }

    printf("Server phrase after r and n removal: %s\n", server_phrase);

    char *word = server_phrase;
    size_t rlen = strlen(word);
    char *reversedPhrase = reverse(word, word, rlen);

    printf("Reversed Phrase: %s\n", reversedPhrase);

    //printf("Length of: %d\n", rlen);

    strcat(reversedPhrase, "\r\n");

    printf("Sending phrase reversal to server.\n");
    size_t reversed_phrase_length = strlen(reversedPhrase);
    numBytes = send(sock, reversedPhrase, reversed_phrase_length, 0);
    if (numBytes < 0)
        DieWithSystemMessage("send() failed");
    else if (numBytes != reversed_phrase_length)
        DieWithUserMessage("send()", "sent unexpected number of bytes");

		printf("Shutdown Client.\n");
		shutdown(sock, SHUT_WR);
		//exit(0);
}

char *reverse(char *word, char *reversed_word, size_t size)
{
        size_t index = 0;

        reversed_word[size] = '\0'; // 5.

        while (size-- > index) {
                const char temp = word[index];
                reversed_word[index++] = word[size];
                reversed_word[size]    = temp;
        }

        return reversed_word;
}
