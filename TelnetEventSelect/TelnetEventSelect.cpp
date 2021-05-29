#include <stdio.h>
#include <WinSock2.h>
#define MAX_CLIENT 1024
WSAEVENT g_events[MAX_CLIENT];
SOCKET g_sockets[MAX_CLIENT];
int g_count = 0;

void main()
{
	WSADATA DATA;
	WSAStartup(MAKEWORD(2, 2), &DATA);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;
	saddr.sin_port = htons(8888);
	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);
	g_sockets[g_count] = s;
	g_events[g_count] = WSACreateEvent();
	WSAEventSelect(g_sockets[g_count], g_events[g_count], FD_ACCEPT);
	g_count += 1;
	while (0 == 0)
	{
		int index = WSAWaitForMultipleEvents(g_count, g_events, FALSE, INFINITE, FALSE);
		index = index - WSA_WAIT_EVENT_0; //Smallest index of the opened event
		for (int i = index; i < g_count; i++)
		{
			WSANETWORKEVENTS networkEvent;
			WSAEnumNetworkEvents(g_sockets[i], g_events[i], &networkEvent);
			if (networkEvent.lNetworkEvents & FD_ACCEPT) //Accept - g_sockets[i]
			{
				if (networkEvent.iErrorCode[FD_ACCEPT_BIT] == 0) //No error
				{
					SOCKADDR_IN caddr;
					int clen = sizeof(caddr);
					SOCKET c = accept(s, (sockaddr*)&caddr, &clen);
					char* success = (char*)"Please send command to execute:\n";
					send(c, success, strlen(success), 0);
					HANDLE e = WSACreateEvent();
					g_sockets[g_count] = c;
					g_events[g_count] = e;
					WSAEventSelect(g_sockets[g_count], g_events[g_count], FD_READ | FD_CLOSE);
					g_count += 1;
					
				}
			}
			if (networkEvent.lNetworkEvents & FD_READ)
			{
				if (networkEvent.iErrorCode[FD_READ_BIT] == 0)
				{
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					recv(g_sockets[i], buffer, sizeof(buffer), 0);
					while (buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n')
					{
						buffer[strlen(buffer) - 1] = 0;
					}

					sprintf(buffer + strlen(buffer), "%s", "> F:\\Code\\NetworkProgramming\\MyTelnetData\\telnet.txt");
					system(buffer);

					FILE* f = fopen("F:\\Code\\NetworkProgramming\\MyTelnetData\\telnet.txt", "rb");
					fseek(f, 0, SEEK_END); // Dua con tro ve cuoi file
					int flen = ftell(f); // Lay vi tri con tro
					fseek(f, 0, SEEK_SET); // Dua con tro ve lai dau file
					char* fdata = (char*)calloc(flen, 1);
					fread(fdata, 1, flen, f);
					fclose(f);
					send(g_sockets[i], fdata, flen, 0);
					free(fdata);
					char* success = (char*)"-------------------------------------------\nPlease send command to execute:\n";
					send(g_sockets[i], success, strlen(success), 0);
				}
			}
			if (networkEvent.lNetworkEvents & FD_CLOSE)
			{
				printf("A client has disconnected\n");
			}
		}
	}
}