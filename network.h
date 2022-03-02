#ifndef NETWORK_H

#define NETWORK_H

// link: Ws2_32.lib
#include <winsock2.h>
#include <ws2tcpip.h>

//#define SHOW_SUCCESSFUL_STEPS
#define CONNECTION_MAX 5
#define MESSAGE_MAX 128

static void WSA_Startup()
{
  WSADATA wsaData;

  if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
  {
    fprintf(stderr, "WSAStartup failed.\n");
    exit(1);
  }
#ifdef SHOW_SUCCESSFUL_STEPS
  else
  {
    printf("WSAStartup successful.\n");
  }
#endif
}

static void WSA_cleanup()
{
  if (WSACleanup() != 0)
  {
    fprintf(stderr, "WSACleanup failed.\n");
    exit(1);
  }
#ifdef SHOW_SUCCESSFUL_STEPS
  else
  {
    printf("WSACleanup successful.\n");
  }
#endif
}

static void WSA_error(char* name)
{
	int errCode = WSAGetLastError();
	fprintf(stderr, "%s failed with error code %d.\n", name, errCode);
	exit(1);
}

static void lookup_addrinfo(char* nodeName, char* serviceName, ADDRINFOA hints, PADDRINFOA* paddrinfoa, boolean printResults)
{
  DWORD dwRetval = getaddrinfo(nodeName, serviceName, &hints, paddrinfoa);
  if (dwRetval != 0)
  {
    fprintf(stderr, "getaddrinfo failed with error %d\n", dwRetval);
    exit(1);
  }
#ifdef SHOW_SUCCESSFUL_STEPS
  else
  {
    printf("getaddrinfo call successful\n");
  }
#endif
  if (printResults)
  {
    printf("Printing addrinfo list...\n");
    for (PADDRINFOA info = *paddrinfoa; info != NULL; info = info->ai_next)
    {
      char ippres[46];
      USHORT port = -1;

      if (info->ai_family == AF_INET)
      {
        SOCKADDR_IN* ipv4 = (SOCKADDR_IN*)info->ai_addr;
        IN_ADDR inaddr = ipv4->sin_addr;
        inet_ntop(AF_INET, &inaddr, (PSTR)&ippres, 46);
        port = ipv4->sin_port;
        port = ntohs(port);
      }
      else if (info->ai_family == AF_INET6)
      {
        SOCKADDR_IN6* ipv6 = (SOCKADDR_IN6*)info->ai_addr;
        IN6_ADDR inaddr6 = ipv6->sin6_addr;
        inet_ntop(AF_INET6, &inaddr6, (PSTR)&ippres, 46);
        port = ipv6->sin6_port;
        port = ntohs(port);
      }

      printf("%s:%hu\n", ippres, port);
      ippres[0] = '\0';
    }
  }
}

static void _bind(SOCKET sock, PADDRINFOA* paddrinfoa)
{
  if (bind(sock, (*paddrinfoa)->ai_addr, (*paddrinfoa)->ai_addrlen) != 0)
  {
    WSA_error("Socket Bind");
  }
#ifdef SHOW_SUCCESSFUL_STEPS
  else
  {
    printf("Socket Bind was successful: %d\n", sock);
  }
#endif
}

static void _listen(SOCKET sock)
{
  if (listen(sock, CONNECTION_MAX) != 0)
  {
    WSA_error("Listen");
  }
#ifdef SHOW_SUCCESSFUL_STEPS
  else
  {
    printf("Listen successful.\n");
  }
#endif
}

static void _connect(SOCKET sock, PADDRINFOA paddrinfoa)
{
  if (connect(sock, paddrinfoa->ai_addr, paddrinfoa->ai_addrlen) != 0)
  {
    WSA_error("Connection");
  }
#ifdef SHOW_SUCCESSFUL_STEPS
  else
  {
    printf("Connection successful! \n");
    printf("SOCK: %d\n", sock);
  }
#endif
}

#endif // NETWORK_H
