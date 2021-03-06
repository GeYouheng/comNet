// ConsoleApplication1.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <winsock.h>
#include <string>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <cstdio>
#include <process.h>
#include <windows.h>
#define SERVER_PORT	5865
#define MY_MSG WM_USER+100
#define TIME_MSG WM_USER+110


using namespace std;

struct message
{
	string str;
	int type;
	int strlen;
	int port;
	char ipaddr[20];
	int id;
	char list[20][20];
	message() {
		str = "";
		type = 0;
		strlen = 0;
		port = 0;

	}
};

bool linkState;
DWORD cThread;

class client {
public:
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	SOCKET sClient; //连接套接字
	struct sockaddr_in saServer;//地址信息
	

	BOOL fSuccess = TRUE;

	
	struct message mssg;
	char *ptr = (char *)&mssg;


	client() {
		wVersionRequested = MAKEWORD(2, 2);//希望使用的WinSock DLL的版本
		ret = WSAStartup(wVersionRequested, &wsaData);
		if (ret != 0)
		{
			printf("WSAStartup() failed!\n");
			return;
		}

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			WSACleanup();
			printf("Invalid Winsock version!\n");
			return;
		}

		sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sClient == INVALID_SOCKET)
		{
			WSACleanup();
			printf("socket() failed!\n");
			return;
		}
	}

	int connec(string addr) {
		char* taddr=(char*)addr.data();
		saServer.sin_family = AF_INET;//地址家族
		saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
		int h=inet_pton(AF_INET,taddr, &saServer.sin_addr.S_un.S_addr);
		ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
		if (ret == SOCKET_ERROR)
		{
			printf("connect() failed!\n");
			closesocket(sClient);//关闭套接字
			WSACleanup();
			return 0;
		}
		return 1;
	}

	

	int sen(struct message &mg) {
		ret = send(sClient, (char *)&mg, sizeof(mg), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("send() failed!\n");
			return 1;
		}
		else
		{
			printf("message has been sent!\n");
			return 0;
		}
	}

	int quit() {

	}

	int receive() {

	}

	

	char ServerAddr[15];
};

client cli;
struct message mesg;



unsigned int __stdcall recMess(LPVOID pm)
{
	mesg.type = 1;
	PostThreadMessage(cThread, MY_MSG, (WPARAM)(&mesg), 0);
	while(0) {
		int nLeft = sizeof(mesg);
		char * ptr = (char *)&mesg;
		while (nLeft > 0) {
			
			cli.ret = recv(cli.sClient, ptr, nLeft, 0);
			if (cli.ret == SOCKET_ERROR)
			{
				printf("recv() failed!\n");
				break;
			}

			if (cli.ret == 0) //客户端已经关闭连接
			{
				printf("client has closed the connection!\n");
				break;
			}
			nLeft -= cli.ret;
			ptr += cli.ret;
		}
		if (!nLeft) {
			PostThreadMessage(cThread, MY_MSG, (WPARAM)(&mesg), 0);
		}
	}
	return 0;
}

int main()
{
	cli =  client();
	linkState = false;
	
	string cmd;
	string ipaddr;
	MSG msg;
	struct message mg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	HANDLE hThread;
	bool flag = true;
	printf("Please enter the commond\n");
	cThread = GetCurrentThreadId();
	while (flag) {
		cin >> cmd;

		unsigned nThreadID;
		switch (cmd.c_str()[0]) {
		case 'l':
			printf("Please enter the ip address\n");
			cin >> ipaddr;
			cout << ipaddr;
			if (cli.connec(ipaddr) == 1)linkState = true;// else break;
			hThread = (HANDLE)_beginthreadex(NULL, 0, recMess, NULL, 0, &nThreadID);
			break;//link
		case 'c':
			if (linkState) {
				closesocket(cli.sClient);
				linkState = false;
				_endthreadex(nThreadID);
			}
			break;//close
		case 't':
			/*if (!linkState) {
				printf("not link");
				break;
			}*/
			
			mg.type = 1;
			mg.strlen = 0;
			//cli.sen(mg);
				GetMessage(&msg, 0, 0, 0);

				if (msg.message == MY_MSG) {
					struct message * pInfo = (struct message *)msg.wParam;
					printf("recv %s/n", pInfo->str);
					//delete pInfo;
					break;
				}
			

			break;//time
		case 'n':
			if (!linkState) {
				printf("not link");
				break;
			}
			
			mg.type = 2;
			mg.strlen = 0;
			cli.sen(mg);
			for (;;) {
				GetMessage(&msg, 0, 0, 0);

				if (msg.message == MY_MSG) {
					struct message * pInfo = (struct message *)msg.wParam;
					printf("recv %s/n", pInfo->str);
					delete pInfo;
					break;
				}
			}
			break;//name
		case 'L':break;//Linked list
		case 's':break;//send msg
		case 'q':flag = false; break;//quit
		}
	}
    return 0;
}

