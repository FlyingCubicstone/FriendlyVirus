#include "pch.h"
#include <iostream>
#include <Windows.h>

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;
BOOL CreateConnectServer(LPSTARTUPINFO lpStrtinfo, LPPROCESS_INFORMATION lpProcessinfo) {
	ZeroMemory(lpStrtinfo, sizeof(STARTUPINFO));
	ZeroMemory(lpProcessinfo, sizeof(PROCESS_INFORMATION));
	WCHAR cmdline[] = L"ConnectServer.exe";
	BOOL result = 0;
	result = CreateProcess(NULL,
		cmdline, NULL, NULL, FALSE, 0, NULL, NULL, lpStrtinfo, lpProcessinfo);
	return result;
}

DWORD ServiceInialization() {
	return 0;
};

VOID WINAPI HandleProc(DWORD dwOpcode) {
	DWORD status;
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_PAUSE:
		serviceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_STOP:
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		serviceStatus.dwCheckPoint = 0;
		serviceStatus.dwWaitHint = 0;
		if (SetServiceStatus(serviceStatusHandle, &serviceStatus))
		{
			status = GetLastError();
			printf("[SPLSRV_SERVICE] SetServiceStatus Error %ld\n", status);

		}
		printf("[SPLSRV_SERVICE] Leaving SplSrvService\n");
		return;
	case SERVICE_CONTROL_INTERROGATE:
		MessageBeep(MB_OK);
		break;
	default:
		printf(".[SPLSRV_SERVICE] Unrecognized opcode %ld\n", dwOpcode);
		break;
	}
	if (!SetServiceStatus(serviceStatusHandle, &serviceStatus))
	{
		status = GetLastError();
		printf("[SPLSRV_SERVICE] SetServiceStatus error %ld\n", status);
	}
	return;
};


VOID WINAPI TeInetMain(DWORD dwArgc,LPTSTR* lpszArgv) {
	serviceStatus.dwCheckPoint=0;
	serviceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE;
	serviceStatus.dwCurrentState=SERVICE_START_PENDING;
	serviceStatus.dwServiceSpecificExitCode=0;
	serviceStatus.dwServiceType=SERVICE_WIN32;
	serviceStatus.dwWaitHint=0;
	serviceStatus.dwWin32ExitCode=0;
	if (ServiceInialization() != 0) {
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		serviceStatus.dwCheckPoint = 0;
		serviceStatus.dwWaitHint = 0;
		return;
	}
	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	serviceStatusHandle = RegisterServiceCtrlHandler(L"TeInet",(LPHANDLER_FUNCTION)HandleProc);
	if (!SetServiceStatus(serviceStatusHandle, &serviceStatus)) {
		return;
	}
	//创建tcp服务端进程
	STARTUPINFO strtinfo;
	PROCESS_INFORMATION processInformation;
	BOOL result;
	result = CreateConnectServer(&strtinfo, &processInformation);
	printf("%d\n", result);
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
};

int main()
{
	WCHAR serviceName[] = L"TeInet";
	SERVICE_TABLE_ENTRY DispatchTable[] = {
		{serviceName,(LPSERVICE_MAIN_FUNCTION)TeInetMain},
		{NULL,NULL}
	};
	StartServiceCtrlDispatcher(DispatchTable);
}