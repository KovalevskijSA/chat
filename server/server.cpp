#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <list>
#include <Windows.h>
#include <fstream>

#pragma comment (lib, "Ws2_32.lib")

#define MAX_SIZE 1024
#define MAX_PIPE_SIZE 1024

using namespace std;
class ClientInfo {
public:
	string name;
	SOCKET socket;
	thread cthread;
	ClientInfo() {
		name = "";
		socket = INVALID_SOCKET;
	}
	~ClientInfo(){
		closesocket(socket);
		cthread.detach();
	}
};

list<ClientInfo*> clients;

string ip = "127.0.0.1";
string port = "5000";

void client_handler(ClientInfo* client) {
	string message = "";
	char temp_msg[MAX_SIZE] = "";
	list<ClientInfo*>::iterator itr;
	memset(temp_msg, 0, MAX_SIZE);
	int result = recv(client->socket, temp_msg, MAX_SIZE, 0);

	if (result != SOCKET_ERROR) {
		for (itr = clients.begin(); itr != clients.end(); itr++) {
			if ((*itr)->name == temp_msg) {
				message = "-ok";
				send(client->socket, message.c_str(), strlen(message.c_str()), 0);
				delete client;
				return;
			}
		}
		client->name = temp_msg;
		clients.push_back(client);
		message = "ok";
		send(client->socket, message.c_str(), strlen(message.c_str()), 0);
	}
	else {
		delete client;
		return;
	}

	while (true) {
		memset(temp_msg, 0, MAX_SIZE);
		int result = recv(client->socket, temp_msg, MAX_SIZE, 0);
		if (result != SOCKET_ERROR) {
			if (strcmp("", temp_msg) && strcmp("\n", temp_msg))
				message = client->name + ": " + temp_msg;

			cout << message << endl;
			for (itr = clients.begin(); itr != clients.end(); itr++) {
				if ((*itr)->socket != INVALID_SOCKET && (*itr)->socket != client->socket)
					send((*itr)->socket, message.c_str(), strlen(message.c_str()), 0);
			}
		}
		else {
			message = client->name + " disconnected";
			cout << message << endl;
			clients.remove(client);
			delete client;
			break;
		}
	}
}

void ServiceStatus() {
	HANDLE hNamedPipe = CreateNamedPipe(
		"\\\\.\\pipe\\Chat",
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_MESSAGE | PIPE_WAIT,
		1,
		MAX_PIPE_SIZE,
		0,
		0,
		NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE) {
		cout << "Connection with the named pipe failed";
		return;
	}
	while (true) {
		if (!ConnectNamedPipe(hNamedPipe, NULL)) {
			cout << "Connection with the named pipe failed." << endl;
			CloseHandle(hNamedPipe);
			return;
		}
		string names = "";
		for (auto itr = clients.begin(); itr != clients.end(); itr++) {
			names += (*itr)->name + "\n";
		}
		DWORD dwBytesWritten;
		WriteFile(
			hNamedPipe,
			names.c_str(),
			sizeof(char) *(names.length() + 1),
			&dwBytesWritten,
			NULL
		);
		DisconnectNamedPipe(hNamedPipe);
	}
}


void ReadConfigFile(string config) {
	ifstream fin(config);
	string data = "";
	if (fin.is_open()) {
		string line;
		while (getline(fin, line)) {
			data += line;
		}
		fin.close();
		int l = data.find("<port>");
		int r = data.find("</port>");
		port = data.substr(l + 6, r - l - 6);

		l = data.find("<ip>");
		r = data.find("</ip>");
		ip = data.substr(l + 4, r - l - 4);
	}
}


int main() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		cout << "WSAStartup failed:";
		return result;
	}
	struct addrinfo hints;
	struct addrinfo* server = NULL;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	ReadConfigFile("config.xml");
	cout << "ip " << ip << endl;
	cout << "port " << port << endl;
	result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &server);
	if (result != 0) {
		cout << "getaddrinfo failed: ";
		WSACleanup();
		return 1;
	}

	int server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (server_socket == INVALID_SOCKET) {
		cout << "Error at socket: " << WSAGetLastError();
		freeaddrinfo(server);
		WSACleanup();
		return 1;
	}

	result = bind(server_socket, server->ai_addr, (int)server->ai_addrlen);
	if (result == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(server);
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}
	
	cout << "Ok..." << std::endl;
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(server_socket);
		WSACleanup();
		return 1;
	}

	thread status_thread = thread(ServiceStatus);
	string message = "";;
	while (true) {
		SOCKET client_socket = accept(server_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) continue;
		ClientInfo* client = new ClientInfo();
		client->socket = client_socket;
		client->cthread = thread(client_handler, ref(client));
	} 
	status_thread.detach();
	closesocket(server_socket);
	clients.clear();

	WSACleanup();
	system("pause");
	return 0;
}