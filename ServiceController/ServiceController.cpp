#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

string GetServiceBinaryPath()
{
	HMODULE Module = GetModuleHandleW(NULL);
	CHAR Path[MAX_PATH];
	GetModuleFileName(Module, Path, MAX_PATH);
	string path_str(Path);
	size_t last_backslash = path_str.find_last_of(L'\\');
	path_str.resize(last_backslash + 1);
	path_str = '\"' + path_str + "ChatService.exe\"";
	return path_str;
}

int main() {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (hSCManager == NULL && GetLastError() == ERROR_ACCESS_DENIED)
	{
		cout << "Access denied. Administrator rights are required";
		CloseServiceHandle(hSCManager);
		cin.get();
		return ERROR_ACCESS_DENIED;
	}
	SC_HANDLE hOldService = OpenService(
		hSCManager,
		"ChatService",
		SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
	if (hOldService != NULL)
	{
		SERVICE_STATUS Status;
		QueryServiceStatus(hOldService, &Status);
		if (Status.dwCurrentState != SERVICE_STOPPED)
		{
			ControlService(hOldService, SERVICE_CONTROL_STOP, &Status);
		}
		DeleteService(hOldService);
		CloseServiceHandle(hOldService);
		CloseServiceHandle(hSCManager);
		cout << "Service deleted successfully";
		cin.get();
		return 0;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		"ChatService",
		"ChatService",
		SERVICE_START,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		GetServiceBinaryPath().c_str(),
		NULL,
		NULL,
		NULL,
		NULL,
		"");
	if (hService != 0) {
		StartService(hService, 0, NULL);
		cout << "Service installed successfully";
		CloseServiceHandle(hService);
	}
	CloseServiceHandle(hSCManager);
	cin.get();
	return 0;
}
