#include "pch.h"
#include <iostream>
#include<Windows.h>
#include <TlHelp32.h>
#include<Psapi.h>
#include <ShlObj.h>
#include<exception>
#include<PathCch.h>
#include<TlHelp32.h>
#include"../FriendlyVirus/FriendlyVirus.h"

#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"../debug/FriendlyVirus.lib")

;
WCHAR* KillProcessByName(WCHAR* processName) {
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	WCHAR *msg=nullptr;
	DWORD pid = 0;
	BOOL result = 0;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	pe.dwSize = sizeof(PROCESSENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot==INVALID_HANDLE_VALUE)
	{
		WCHAR msgTmp[] = L"CreateToolhelp32SSnapshot() failed!";
		int size = (lstrlen(msgTmp) + 1) * 2;
		msg = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		CopyMemory(msg, msgTmp, size);
		return msg;
	}
	Process32Next(hSnapshot, &pe);
	do {
		if (lstrcmp(pe.szExeFile, processName) == 0) {
			pid = pe.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &pe));
	if (pid==0)
	{
		WCHAR msgTmp[] = L"target program does not exist!";
		int size = (lstrlen(msgTmp) + 1) * 2;
		msg = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		CopyMemory(msg, msgTmp, size);
		return msg;
	}
	WCHAR cmdline[46] = L"taskkill /F /pid ";
	WCHAR wpid[14];
	//将DWORD类型转换成wchar_t型字符串
	_itow_s(pid, wpid, 10);
	lstrcat(cmdline, wpid);
	result = CreateProcess(NULL,cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	if (result) {
		WCHAR msgTmp[] = L"target process terminate successful!";
		int size = (lstrlen(msgTmp) + 1) * 2;
		msg = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		CopyMemory(msg, msgTmp, size);
		return msg;
	}
	else {
		WCHAR msgTmp[] = L"target process terminate failed!";
		int size = (lstrlen(msgTmp) + 1) * 2;
		msg = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
		CopyMemory(msg, msgTmp, size);
		return msg;
	}
}


void PrintMessage(WCHAR **src) {
	WCHAR **tmp = NULL;
	tmp = src;
	while (*tmp) {
		printf("%ws\n", *tmp);
		tmp++;
	}
}

//公共函数，向邮槽发送信息
VOID SendMyMessage(WCHAR**(*p)()) {
	WCHAR **content = NULL;
	WCHAR **content_ = NULL;
	content = p();
	content_ = content;
	while (*content) {
		SendMessageToMailslot(*content, (lstrlen(*content) + 1) * sizeof(WCHAR));
		content++;
	}
	FreePointers(&content_);
	
}

//overload the function SendMyMessage
void SendMyMessage(WCHAR* (p)(WCHAR *param),WCHAR* prgName) {
	WCHAR *msg = NULL;
	msg = p(prgName);
	SendMessageToMailslot(msg, (lstrlen(msg) + 1) * sizeof(WCHAR));
	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, msg);
}


//获取主机中所有正在运行的进程信息
WCHAR** GetAllRunningProcess() {
	HANDLE hSnapshot;
	WCHAR** pe=NULL;
	WCHAR ** petmp = NULL;
	WCHAR *exeName = NULL;
	PROCESSENTRY32 peCur;
	DWORD dwProcessIds[1024];
	DWORD dwBytesOfAllProcess = 0;
	EnumProcesses(dwProcessIds, sizeof(dwProcessIds), &dwBytesOfAllProcess);
	pe = (WCHAR**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (dwBytesOfAllProcess/sizeof(DWORD)+1)*sizeof(WCHAR*));
	petmp = pe;
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	peCur.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &peCur)) {
		printf("Process32First() failed!errpr:%d\n", GetLastError());
		return nullptr;
	}
	do {
		exeName = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(peCur.szExeFile));
		CopyMemory(exeName, peCur.szExeFile, (lstrlen(peCur.szExeFile)+1)*sizeof(WCHAR));
		*petmp = exeName;
		petmp++;
	} while (Process32Next(hSnapshot, &peCur));
	return pe;
};

//获取回收站的内容
VOID GetDustBinFolderInformation() {
	LPITEMIDLIST lptmdl;
	LPENUMIDLIST lpenum;
	LPITEMIDLIST lptmdlCurrent;
	IShellFolder *lsfdBase=NULL;
	IShellFolder *lsfdObj=NULL;
	WCHAR *content = NULL;
	DWORD dwFetched = 0;
	SHGetDesktopFolder(&lsfdBase);
	SHGetFolderLocation(nullptr, CSIDL_BITBUCKET, nullptr, 0,&lptmdl);
	lsfdBase->BindToObject(lptmdl, NULL, IID_IShellFolder, (void**)&lsfdObj);
	lsfdObj->EnumObjects(NULL, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN,&lpenum);
	STRRET strret;
	ZeroMemory(&strret, sizeof(STRRET));
	while (true){
		if (lpenum->Next(1, &lptmdlCurrent, &dwFetched) == S_FALSE) {
			printf("目标文件夹内为空！\n");
			break;
		}
		lsfdObj->GetDisplayNameOf(lptmdlCurrent, SHGDN_NORMAL, &strret);
		printf("%ws\t%s\n", strret.pOleStr,strret.cStr);
	}
}

//获取桌面上的所有文件
WCHAR** GetFilesInDesktop() {
	LPITEMIDLIST lpitlst;
	LPITEMIDLIST lpitlstCurrent;
	IShellFolder *isllfd;
	LPENUMIDLIST lpEnum;
	DWORD dwFetched;
	SHGetFolderLocation(NULL, CSIDL_DESKTOP, NULL, 0, &lpitlst);
	SHGetDesktopFolder(&isllfd);
	STRRET strret;
	WCHAR **content = NULL;
	ZeroMemory(&strret, sizeof(strret));
	isllfd->EnumObjects(nullptr, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN, &lpEnum);
	content = (WCHAR**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR*)*100);
	ZeroMemory(content, sizeof(WCHAR*) * 100);
	WCHAR** tmp = content;
	while (true)
	{
		if (lpEnum->Next(1, &lpitlstCurrent, &dwFetched) == S_FALSE) {
			break;
		}
		isllfd->GetDisplayNameOf(lpitlstCurrent, SHGDN_NORMAL, &strret);
		WCHAR *name = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (lstrlen(strret.pOleStr) + 1) * sizeof(WCHAR));
		lstrcpy(name, strret.pOleStr);
		*tmp = name;
		(tmp)++;
		
	}
	return content;
}

int main(int argc,char *argv[])
{
	if (argc==3)
	{
		if (strcmp(argv[1], "kill") == 0) {
			WCHAR *exeName = nullptr;
			CHARToWCHAR(&exeName, argv[2]);
			SendMyMessage(KillProcessByName,exeName);
		}
		else if (strcmp(argv[1], "") == 0) {

		}
	}
	else if (argc == 2) {
		if (strcmp(argv[1], "desktopfile") == 0) {
			SendMyMessage(GetFilesInDesktop);
		}
		else if (strcmp(argv[1], "processes") == 0) {
			SendMyMessage(GetAllRunningProcess);
		}
	}
	else if (argc == 1) {
		printf("本程序的功能有:\n");
		printf("========\n========\n"); 
	}
	return 0;
}