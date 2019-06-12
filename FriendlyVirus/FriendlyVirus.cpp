// FriendlyVirus.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <iostream>
//向邮槽发送信息
void SendMessageToMailslot(LPWSTR lpContentTobeSent, DWORD dwBytesTobeWrite) {
	HANDLE hFile;
	bool writeRet;
	DWORD dwBytesWritten = 0;
	hFile = CreateFile(L"\\\\.\\mailslot\\lzq",
		GENERIC_ALL, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("进程[%d]打开邮槽失败!error:%d\n",GetCurrentProcessId(),GetLastError());
		return;
	}
	writeRet = WriteFile(hFile, lpContentTobeSent, dwBytesTobeWrite, &dwBytesWritten, NULL);
	if (!writeRet) {
		printf("进程[%d]写入信息到邮槽失败!%d\n",GetCurrentProcessId(),GetLastError());
	}
	CloseHandle(hFile);
};

//获取邮槽中的信息
WCHAR** GetMessageFromMailslot(HANDLE hMailslot) {
	DWORD dwMaxMessageSize = 0;
	DWORD dwNextSize;
	DWORD dwMessageCount;
	DWORD dwReadTimeWaitOut = 1000;
	BOOL getMsgRet = 0;
	WCHAR **readContent = NULL;
	WCHAR **readContentTmp = NULL;
	DWORD dwBytesRead = 0;
	getMsgRet = GetMailslotInfo(hMailslot, &dwMaxMessageSize, &dwNextSize, &dwMessageCount, &dwReadTimeWaitOut);
	if (!getMsgRet) {
		printf("获取邮槽信息失败!error:%d\n", GetLastError());
		return NULL;
	}
	readContent = (WCHAR**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR*)*(dwMessageCount+1));
	readContentTmp = readContent;
	while (dwMessageCount != 0) {
		WCHAR *lpReadBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNextSize);
		if (!ReadFile(hMailslot, lpReadBuffer, dwNextSize, &dwBytesRead, NULL)) {
			printf("ReadFile() failed!error:%d\n", GetLastError());
			return NULL;
		}
		*readContentTmp = lpReadBuffer;
		readContentTmp++;
		GetMailslotInfo(hMailslot, &dwMaxMessageSize, &dwNextSize, &dwMessageCount, &dwReadTimeWaitOut);
	}
	return readContent;
}

void FreePointers(LPWSTR **p) {
	LPWSTR **pTmp = p;
	while (*pTmp) {
		HeapFree(GetProcessHeap(), 0, *pTmp);
		(*pTmp)++;
	}
	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, *p);
}

void CHARToWCHAR(WCHAR **des, const char* source) {
	*des = (WCHAR*)malloc((strlen(source) + 1) * 2);
	FillMemory(*des, (strlen(source) + 1) * 2, '\0');
	WCHAR *desTmp = *des;
	int size = strlen(source);
	while (size > 0) {
		*desTmp = (WCHAR)*source;
		desTmp++;
		source++;
		size--;
	}
}