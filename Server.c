#include <stdio.h>
#include "Practical.h"
#include <unistd.h>
#include <string.h>
// Handle new TCP client
void HandleTCPClient(int clntSocket);
// Preset port as directed by assignment.
const char *TCP_PORT = "48031";
char *reverse(char *word, char *reversed_word, size_t size);
void recvFromClient(int clntSocket, char *recv_string);
void tokenizeString(char *string_to_tokenize, char *string_tokens[], char* delimter);

int main(int argc, char *argv[])
{

		if (argc > 2)
		{ // Test for correct number of arguments
				DieWithUserMessage("Parameter(s)", "<Server Port/Service> OR none");
		}

		// Use the preset port if nogcc -o Server Server.c DieWithMessage.c TCPServerUtility.c AddressUtility.c argument was passed in.
		const char *service = (argc == 2) ? (argv[1]) : TCP_PORT;

		// Create socket for incoming connections
		int servSock = SetupTCPServerSocket(service);

		if (servSock < 0)
		{
				DieWithUserMessage("SetupTCPServerSocket() failed", service);
		}

		while(true) // Run forever
		{
				// New connection creates a connected client socket
				int clntSock = AcceptTCPConnection(servSock);

				HandleTCPClient(clntSock); // Process client
		}

		//close(servSock);
}

void HandleTCPClient(int clntSocket)
{
		char client_request_string[BUFSIZE];// = recvFromClient(clntSocket); // client_request_string for echo string
    recvFromClient(clntSocket, client_request_string);

    ssize_t numBytesRcvd = 0;

		printf("Recieved request string from client: %s\n", client_request_string);

/*
    for(int i = 0; i!= sizeof(client_request_string); i++)
    {
        printf("i: %d\n", i);
        if(client_request_string[i] == '\n')
        {
            printf("Removing newline");
            client_request_string[i] = '\0';
        }
    }
*/
    char *delimter = " ";
    char *request_string_tokens[4];

    tokenizeString(client_request_string, request_string_tokens, delimter);

		char *server_name = request_string_tokens[0];
		char *server_type = request_string_tokens[1];
		char *user_name = request_string_tokens[2];
		char *dotted_quad_port_number = request_string_tokens[3];

		char greeting[BUFSIZE] = "hello ";
		strcat(greeting, dotted_quad_port_number);
		strcat(greeting, ", welcome to the ");
		strcat(greeting, server_name);
		strcat(greeting, " server\r\n");

		/*
		   SEND greeting to client.
		 */
		size_t greeting_length = strlen(greeting);
		printf("Sending greeting to client at: %s\n", dotted_quad_port_number);
		ssize_t numBytesSent = send(clntSocket, greeting, greeting_length, 0);
		if (sizeof(greeting) < 0)
		{
				DieWithSystemMessage("send() failed");
		}
		else if (sizeof(greeting) != sizeof(greeting))
		{
				DieWithUserMessage("send()", "sent unexpected number of bytes");
		}
		/*
		       End sending greeting to client.
		 */

		/*
		   RECV greeting response from client.
		 */
		char greetingResponse[BUFSIZE];
		numBytesRcvd = recv(clntSocket, greetingResponse, BUFSIZE-1, 0);

		if (numBytesRcvd < 0)
		{
				DieWithSystemMessage("recv() failed");
		}
		else if(numBytesRcvd > 0)
		{
				greetingResponse[numBytesRcvd] = '\0';
		}

		for(int i = 0; i!= sizeof(greetingResponse); i++)
		{
				if(greetingResponse[i] == '\n')
				{
						greetingResponse[i] = '\0';
				}
		}

		printf("Client greeting response: %s\n", greetingResponse);
		/*
		   End recieving greeting response from client..
		 */

		/*
		   SEND phrase to client.
		 */
		char phrase[BUFSIZE] = "hello\r\n";
		size_t phrase_length = strlen(phrase);
		printf("Sending phrase to client: %s\n", dotted_quad_port_number);
		printf("Phrase to client: %s\n", phrase);
		numBytesSent = send(clntSocket, phrase, phrase_length, 0);

		if (numBytesSent < 0)
		{
				DieWithSystemMessage("send() failed");
		}
		else if (numBytesSent != phrase_length)
		{
				DieWithUserMessage("send()", "sent unexpected number of bytes");
		}
		/*
		   End phrase send to client.
		 */

		// Recieve phrase reversal from client.
		char phraseResponse[BUFSIZE];
		numBytesRcvd = recv(clntSocket, phraseResponse, BUFSIZE-1, 0);
		if (numBytesRcvd < 0)
				DieWithSystemMessage("recv() failed");
		phraseResponse[numBytesRcvd] = '\0';

		for(int i = 0; i!= sizeof(phraseResponse); i++)
		{
				if(phraseResponse[i] == '\n')
				{
						phraseResponse[i] = '\0';
				}
		}

		printf("Client Reversal Response: %s\n", phraseResponse);
		//printf("%s\n",phraseResponse);
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

void recvFromClient(int clntSocket, char *recv_string)
{
		ssize_t numBytesRcvd = recv(clntSocket, recv_string, BUFSIZE-1, 0);

		if (numBytesRcvd < 0)
		{
				DieWithSystemMessage("recv() failed");
		}
		else if(numBytesRcvd > 0)
		{
				recv_string[numBytesRcvd] = '\0';

				for(int i = 0; i!= BUFSIZE; i++)
				{
            printf("i: %d\n", i);
						if(recv_string[i] == '\n')
						{
                printf("Removing newline");
								recv_string[i] = '\0';
						}
				}
		}
}

void tokenizeString(char *string_to_tokenize, char *string_tokens[], char* delimter)
{
  int i = 0;
  char *token = strtok (string_to_tokenize, " ");
  //char *request_string_tokens[4];

  while (token != NULL)
  {
      string_tokens[i++] = token;
      token = strtok (NULL, " ");
  }
}
