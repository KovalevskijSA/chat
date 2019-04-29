#include <windows.h>
#include <strsafe.h>
#include <tchar.h>
#include <string>
#include <fstream>
#include <iostream>
static const int MAX_BUFFER_LEN = 1024;
using namespace std;
#pragma comment(lib, "advapi32.lib")

SERVICE_STATUS_HANDLE   hServiceStatusHandle;
SERVICE_STATUS          ServiceStatus;
TCHAR pServiceName[MAX_PATH] = "Chat";



void WriteLog(string msg) {
	TCHAR strModuleFile[MAX_BUFFER_LEN];
	DWORD dwSize = GetModuleFileName(NULL, strModuleFile, MAX_BUFFER_LEN);
	string strLogFile(strModuleFile);
	fstream of;
	strLogFile += ".log";
	of.open(strLogFile, ios::app);
	of << msg << endl;
}

void UserFun(DWORD dwArgc, LPSTR* lpszArgv) {
	for (int i = 0; i < 100; i++) {
		WriteLog(to_string(i));
		Sleep(100);
	}
}

void ServiceHandler(DWORD fdwControl)
{
	WriteLog("-->Service ServiceHandle fdwControl: 0x%0X" + fdwControl);
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		break;
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	};

	if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
	{
		WriteLog("-->Service SetServiceStatus failed, error code = %d" + GetLastError());
	}
}

void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	WriteLog("-->Service  ServiceMain start...");

	hServiceStatusHandle = RegisterServiceCtrlHandler(pServiceName, (LPHANDLER_FUNCTION)ServiceHandler);
	if (hServiceStatusHandle == 0)
	{
		WriteLog("-->Service RegisterServiceCtrlHandler failed, error code = %d" + GetLastError());
		return;
	}

	// Initialization complete - report running status
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
	{
		WriteLog("-->Service SetServiceStatus failed, error code = " + GetLastError());
	}

	WriteLog(TEXT("-->Service callback UserServiceMain"));
	UserFun(dwArgc, lpszArgv);

	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;
	if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
	{
		WriteLog("-->Service SetServiceStatus failed, error code = %d" + GetLastError());
	}
}


void main() {
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ pServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		WriteLog("StartServiceCtrlDispatcher");
	}
}
