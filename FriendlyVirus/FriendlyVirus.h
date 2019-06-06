#pragma once
#include <Windows.h>
void SendMessageToMailslot(LPWSTR lpContentTobeSent, DWORD dwBytesTobeWrite);
WCHAR** GetMessageFromMailslot(HANDLE hMailslot);
void FreePointers(WCHAR ** p);