#include <stdio.h>
#include "network.h"

SOCKET server_sock;
WSAPOLLFD pollfds[CONNECTION_MAX] = { 0 };
int pollfds_count = 0;

static void server_init(char* port)
{
  WSA_Startup();

  ADDRINFOA hints = { 0 };
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  PADDRINFOA paddrinfoa = { 0 };

  lookup_addrinfo(NULL, port, hints, &paddrinfoa, FALSE);

  server_sock = socket(paddrinfoa->ai_family, paddrinfoa->ai_socktype, paddrinfoa->ai_protocol);
  
  _bind(server_sock, &paddrinfoa);
  freeaddrinfo(paddrinfoa);

  _listen(server_sock);
}

static void server_shutdown()
{
  closesocket(server_sock);
  WSA_cleanup();
}

static int server_accept()
{
	SOCKADDR_IN sockaddr = { 0 };
	SOCKET incomingSocket = 0;
	socklen_t socklen = sizeof(sockaddr);

	return accept(server_sock, (SOCKADDR*)&sockaddr, &socklen);
}

void removeConnection(int index)
{
	printf("Removing Connection...");

	// copying the last valid pollfds entry into slot of the 
	//		connection being removed 
	int latestIndex = pollfds_count - 1;
	pollfds[index].fd = pollfds[latestIndex].fd;
	pollfds[index].events = pollfds[latestIndex].events;
	pollfds[index].revents = pollfds[latestIndex].revents;

	pollfds_count--;
}

int main(int argc, char* argv[])
{
	server_init("8500");

	pollfds[0].fd = server_sock;
	pollfds[0].events = POLLIN;
	pollfds_count = 1;

	int returnNum = 0;
	char buf[128];
	while ((returnNum = WSAPoll(pollfds, pollfds_count, -1)) != SOCKET_ERROR) // -1 is wait indefinitely
	{
		int pollFound_count = 0;
		for (int i = 0; i < pollfds_count; i++)
		{
			if (pollfds[i].revents & POLLIN)
			{
				pollFound_count++;
				if (pollfds[i].fd == server_sock)
				{
					// connection incoming, accept call and add to list
					int sock = server_accept();

					pollfds[pollfds_count].fd = sock;
					pollfds[pollfds_count].events = POLLIN;
					pollfds_count++;

					printf("A Client Has Connected\n");
				}
				else
				{
					// data incoming from existing connection
					int sock = pollfds[i].fd;
					int numBytes = recv(sock, buf, 128, 0);

					if (numBytes <= 0)
					{
						removeConnection(i);
					}
					else
					{
						printf("Message: %.*s", numBytes, &buf);

						// sending message back to all connected clients
						for (int i = 1; i < pollfds_count; i++)
						{
							send(pollfds[i].fd, buf, numBytes, 0);
						}
					}
				}
				if (pollFound_count >= returnNum) continue;
			}
			else if (pollfds[i].revents & POLLHUP)
			{
				removeConnection(i);
			}
		}

	}

	WSA_error("WSAPoll\0");
}
