#pragma once
#include <Windows.h>
void SendMessageToMailslot(LPWSTR lpContentTobeSent, DWORD dwBytesTobeWrite);
WCHAR** GetMessageFromMailslot(HANDLE hMailslot);
void FreePointers(LPWSTR **p);
void CHARToWCHAR(WCHAR **des, const char* source)