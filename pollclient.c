#include <stdio.h>
#include "network.h"
#include <processthreadsapi.h>

SOCKET client_sock;

// nodeName == hostName/ipv4, serviceName == port/serviceType (e.g., http or 80)
static void client_connect(char* nodeName, char* serviceName)
{
	printf("Connecting to %s:%s...\n", nodeName, serviceName);

	ADDRINFOA hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	PADDRINFOA paddrinfoa = { 0 };

	lookup_addrinfo(nodeName, serviceName, hints, &paddrinfoa, FALSE);

	client_sock = socket(paddrinfoa->ai_family, paddrinfoa->ai_socktype, paddrinfoa->ai_protocol);

	_connect(client_sock, paddrinfoa);

	freeaddrinfo(paddrinfoa);

	printf("Connection Successful!\n");
}

static void client_init()
{
	WSA_Startup();
}

static int client_send_bytes(char* bytes, size_t size)
{
	int bytesSent = send(client_sock, bytes, size, 0);
	//printf("bytesSent: %d\n", bytesSent);

	return bytesSent;
}

static int client_recv(char* buf, size_t bufSize)
{
	int ret = recv(client_sock, buf, bufSize, 0);
	if (ret == SOCKET_ERROR)
	{
		printf("Server has most likely disconnected.\n");
		exit(1);
	}
	return ret;
}

static void client_shutdown()
{
	shutdown(client_sock, SD_BOTH);
	closesocket(client_sock);

	WSA_cleanup();
}

DWORD WINAPI receive(LPVOID lpparams)
{
  char buf[128] = { 0 };

  int messageSize = -1;
	do
  {
    messageSize = client_recv(buf, 128);
		printf("Message: %.*s", messageSize, &buf);
	} while (messageSize > 0);

	return messageSize;
}

static void enableVTCodes()
{
	HANDLE stdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (stdOutputHandle == NULL)
	{
		fprintf(stderr, "Failed to get stdOutputHandle, error: %d\n", GetLastError());
		exit(1);
	}

	DWORD lpModeOut;
	if (!GetConsoleMode(stdOutputHandle, &lpModeOut))
	{
		fprintf(stderr, "Failed to get console mode, error: %d\n", GetLastError());
		exit(1);
	}

	lpModeOut |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	lpModeOut &= ~ENABLE_QUICK_EDIT_MODE; // not sure if this is needed...

	if (!SetConsoleMode(stdOutputHandle, lpModeOut))
	{
		fprintf(stderr, "Failed to set output console mode, error: %d\n", GetLastError());
		exit(1);
	}
}

int main(int argc, char* argv[])
{
	enableVTCodes();

	client_init();
	client_connect("localhost", "8500");

	// create and run thread for receiving messages
	CreateThread(NULL, 0, receive, NULL, 0, NULL);

	// send message loop
	char message[MESSAGE_MAX];

	printf("Type a message and press Enter to send (max characters: %d).\n", MESSAGE_MAX);
	printf("To exit, send the message 'exit'.\n");
	while (1)
	{
		fgets(message, MESSAGE_MAX, stdin);
		printf("\033[1F");		// go up to the previous line (where the message was input)
		printf("\033[2K\r");	// erase the current line and carriage return to the beginning

		if (message != NULL)
		{
			if (memcmp(&message, "exit", 4) == 0) break;

			client_send_bytes(message, strlen(message));
		}
	}

	client_shutdown();
}
