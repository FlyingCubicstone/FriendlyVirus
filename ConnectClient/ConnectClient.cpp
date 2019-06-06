#include "pch.h"
#include <iostream>
#include<WS2tcpip.h>
#include<WinSock2.h>

#pragma comment(lib,"ws2_32.lib")

int main()
{
	int nResult = 0;
	SOCKADDR_IN sockaddr_in;
	WSADATA wsadata;
	char inputBuff[512];
	int bytesSent = 0;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (nResult!=0) {
		printf("WSAStartup failed! error:%d\n", WSAGetLastError());
		return 1;
	}
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket==INVALID_SOCKET)
	{
		printf("套接字创建失败！error:%d\n",WSAGetLastError());
		return 2;
	}
	InetPton(AF_INET, L"127.0.0.1", &sockaddr_in.sin_addr.S_un.S_addr);
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(8848);
	nResult = connect(clientSocket, (SOCKADDR*)&sockaddr_in, sizeof(sockaddr));
	if (nResult!=0) {
		printf("connect() failed!error:%d\n", WSAGetLastError());
		return 3;
	}
	printf("连接至服务器成功!\n");
	while (true) {
		printf("请输入你要发送的内容!\n");
		ZeroMemory(&inputBuff, 512);
		scanf_s("%s", inputBuff, 512);
		bytesSent = send(clientSocket, inputBuff, strlen(inputBuff), 0);
		if (bytesSent == SOCKET_ERROR) {
			printf("发送失败!error:%d\n",WSAGetLastError());
			return 4;
		}
		if (strcmp(inputBuff, "\\close") == 0|| strcmp(inputBuff, "\\c")==0) {
			printf("断开连接!\n");
			break;
		}
		printf("发送成功！\n");
	}
}