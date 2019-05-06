#include <Windows.h>
#include <vector>
#include <iostream>
using namespace std;

#define MAX_PIPE_SIZE 512

int main() {
	HANDLE hNamedPipe;
	DWORD dwBytesRead;
	HANDLE hPipe = CreateFileA(
		"\\\\.\\pipe\\ChatService",
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		cout << "Connection failed" << endl;
		cin.get();
		return 0;
	}
	vector<char> buf(MAX_PIPE_SIZE + 1);
	if (!ReadFile(hPipe, buf.data(), MAX_PIPE_SIZE, &dwBytesRead, NULL)) {
		cout << "Data reading from the named pipe failed" << endl;
		cin.get();
	}
	buf[MAX_PIPE_SIZE] = '\0';
	cout << buf.data() << endl;
	cin.get();
	return 0;
}