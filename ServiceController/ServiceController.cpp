#include "pch.h"
#include <iostream>
#include<Shlwapi.h>
#include<Windows.h>
#pragma comment(lib,"shlwapi.lib")

SC_HANDLE* GetSCManagerHandle() {
	SC_HANDLE scManager;
	SC_HANDLE* scManagerHeap = (SC_HANDLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SC_HANDLE));
	scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CopyMemory(scManagerHeap, scManager, sizeof(scManager));
	return scManagerHeap;
};


void GetServiceStatus(LPSERVICE_STATUS lpServiceStatus) {
	SC_HANDLE* scManager;
	SC_HANDLE scService;
	BOOL bQueryServiceStatusResult = 0;
	scManager = GetSCManagerHandle();
	scService = OpenService(*scManager, L"TeInet", SERVICE_ALL_ACCESS);
	if (!scService) {
		printf("GetServiceStatus() 调用OpenService() 失败!error:%d\n", GetLastError());
		return;
	}
	bQueryServiceStatusResult = QueryServiceStatus(scService, lpServiceStatus);
	if (!bQueryServiceStatusResult) {
		printf("GetServiceStatus() 调用QueryServiceStatus()失败!error:%d\n", GetLastError());
		return;
	}
	CloseHandle(*scManager);
	HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, scManager);
};

VOID MakeServiceStop() {

}

VOID CreateAndStartAService() {
	SC_HANDLE scManager;
	SC_HANDLE scService;
	bool bSrvsSttResult = 0;
	WCHAR path[MAX_PATH];
	DWORD getGCDResult = 0;
	scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (scManager == INVALID_HANDLE_VALUE) {
		printf("OpenSCManager() failed!error:%d\n", GetLastError());
	}
	
	ZeroMemory(path, MAX_PATH);
	getGCDResult = GetCurrentDirectory(MAX_PATH, path);
	if (getGCDResult == 0) {
		printf("获取当前进程所在目录失败!\n");
		return;
	}
	WCHAR pathDes[MAX_PATH];
	PathCombine(pathDes, path, L"FriendlyVirusService.exe");
	scService = CreateService(scManager, L"TeInet", L"TeInet", SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, pathDes,
		NULL, NULL, NULL, NULL, NULL);
	if (!scService) {
		printf("CreateService() failed!error:%d\n", GetLastError());
		return;
	}
	printf("CreateService() success!\n");
	bSrvsSttResult = StartService(scService, 0, NULL);
	if (bSrvsSttResult) {
	}
	else {
		printf("服务启动失败!error:%d\n", GetLastError());
	}
	CloseHandle(scManager);
}

int main(int argc, char * argv[])
{	
	SERVICE_STATUS serviceStatus;
	GetServiceStatus(&serviceStatus);
	CreateAndStartAService();
	return 0;
}
