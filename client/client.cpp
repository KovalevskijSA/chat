#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define MAX_SIZE 1024 


int client_handler(int msocket) {
	char receivedMessage[MAX_SIZE];
	while(true) {
		memset(receivedMessage, 0, MAX_SIZE);
		if (msocket != 0) {
			int iResult = recv(msocket, receivedMessage, MAX_SIZE, 0);
			if (iResult != SOCKET_ERROR)
				cout << receivedMessage << endl;
			else 
				break;
		}
	}

	if (WSAGetLastError() == WSAECONNRESET)
		cout << "The server has disconnected" << endl;

	system("pause");
	return 0;
}

int main() {
	int msocket;
	string name;
	char receivedMessage[MAX_SIZE];
	string ip;
	string port;

	WSAData wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		cout << "WSAStartup failed:";
		return result;
	}
	struct addrinfo* res = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	string message;

	cout << "Starting Client...\n";
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	cout << "Name: ";
	cin >> name;
	cout << "IP: ";
	cin >> ip;
	cout << "Port: ";
	cin >> port;
	/*
	ip = "127.0.0.1";
	port = "5000";
	*/

	result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
	if (result != 0) {
		cout << "getaddrinfo failed ";
		WSACleanup();
		system("pause");
		return 1;
	}

	msocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (msocket == INVALID_SOCKET) {
		cout << "Can't connect to the server, winsock error: " << WSAGetLastError() << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	result = connect(msocket, res->ai_addr, (int)res->ai_addrlen);
	if (result == SOCKET_ERROR) {
		cout << "Unable to connect to server!";
		closesocket(msocket);
		WSACleanup();
		system("pause");
		return 1;
	}
	freeaddrinfo(res);

	send(msocket, name.c_str(), strlen(name.c_str()), 0);
	memset(receivedMessage, 0, MAX_SIZE);
	recv(msocket, receivedMessage, MAX_SIZE, 0);
	message = receivedMessage;
	if (message == "ok") {
		cout << "Connection success" << endl;
		thread my_thread(client_handler, msocket);
		while (true) {
			cin >> message;
			printf("\r%80c\r", ' ');
			cout << "\r" << name << ": " << message;
			result = send(msocket, message.c_str(), strlen(message.c_str()), 0);
			if (result <= 0) {
				cout << "send() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
		my_thread.detach();
	}
	else if (message == "-ok")
		cout << "Can't connect to the server, name " + name + " already in use" << endl;
	else cout << message;
	closesocket(msocket);
	WSACleanup();
	system("pause");
	return 0;
}