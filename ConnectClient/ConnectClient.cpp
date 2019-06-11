#include "pch.h"
#include <iostream>
#include<WS2tcpip.h>
#include<WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

void CHARToWCHAR(WCHAR **des, const char* source) {
	*des = (WCHAR*)malloc((strlen(source) + 1) * 2);
	FillMemory(*des, (strlen(source) + 1) * 2, '\0');
	WCHAR *desTmp = *des;
	int size = strlen(source);
	while (size>0) {
		*desTmp = (WCHAR)*source;
		desTmp++;
		source++;
		size--;
	}
}

int main(int argc,char* argv[])
{
	if (argc == 1) {
		printf("用法说明:\n");
		printf("-------\n-------\n");
		
	}
	if (argc == 2) {
		int nResult = 0;
		SOCKADDR_IN sockaddr_in;
		WSADATA wsadata;
		char inputBuff[512];
		char recvBuff[10240];
		int bytesSent = 0;
		int bytesRecv = 0;
		WCHAR *ip = NULL;
		nResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
		if (nResult != 0) {
			printf("WSAStartup failed! error:%d\n", WSAGetLastError());
			return 1;
		}
		SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("套接字创建失败！error:%d\n", WSAGetLastError());
			return 2;
		}
		CHARToWCHAR(&ip, argv[1]);
		InetPton(AF_INET, ip, &sockaddr_in.sin_addr.S_un.S_addr);
		sockaddr_in.sin_family = AF_INET;
		sockaddr_in.sin_port = htons(8848);
		nResult = connect(clientSocket, (SOCKADDR*)&sockaddr_in, sizeof(sockaddr));
		if (nResult != 0) {
			printf("connect() failed!error:%d\n", WSAGetLastError());
			return 3;
		}
		printf("连接至服务器成功!\n");
		while (true) {
			printf("请输入你要发送的内容!\n");
			ZeroMemory(&inputBuff, 512);
			scanf_s("%[^\n]", inputBuff, 512);
			getchar();
			bytesSent = send(clientSocket, inputBuff, strlen(inputBuff), 0);
			if (bytesSent == SOCKET_ERROR) {
				printf("发送失败!error:%d\n", WSAGetLastError());
				return 4;
			}
			if (strcmp(inputBuff, "\\close") == 0 || strcmp(inputBuff, "\\c") == 0) {
				printf("断开连接!\n");
				break;
			}
			if (strcmp(inputBuff, "kill") == 0) {
				ZeroMemory(&inputBuff, 512);
				printf("请继续输入指令:");
				scanf_s("%[^\n]", inputBuff, 512);
				getchar();
				send(clientSocket, inputBuff, strlen(inputBuff), 0);
			}
			printf("发送成功!\n");
		
			//接收服务端发来的数据
			Sleep(500);
			bytesRecv = recv(clientSocket, recvBuff, 10240, 0);
			if (bytesRecv == SOCKET_ERROR) {
				printf("接收失败!error:%d\n", WSAGetLastError());
			}
			else {
				printf("接收到来自服务端的信息:\n");
				
					printf("%ws\n", recvBuff);
				
			}
			
			
		}
	}
	
}