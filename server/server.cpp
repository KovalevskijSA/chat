#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <vector>

#pragma comment (lib, "Ws2_32.lib")

#define IP_ADDRESS "127.0.0.1"
#define DEFAULT_PORT "5000"
#define MAX_SIZE 1024


using namespace std;

struct ClientInfo {
	string name;
	int socket;
};

const int MAX_CLIENTS = 200;
/*
int client_handler(ClientInfo* new_client, vector<ClientInfo*>& clients) {
	string message = "";
	char temp_msg[MAX_SIZE] = "";
	memset(temp_msg, 0, MAX_SIZE);
	int result = recv(new_client->socket, temp_msg, MAX_SIZE, 0);
	
	if (result != SOCKET_ERROR) {
		for (auto c : clients) {
			if (c->name == temp_msg) {
				message = "-ok";
				send(new_client->socket, message.c_str(), strlen(message.c_str()), 0);
				closesocket(new_client->socket);
				return 0;
			}
		}
		message = "ok";
		send(new_client->socket, message.c_str(), strlen(message.c_str()), 0);
		new_client->name = temp_msg;

	}
	else return 0;

	while(true) {
		memset(temp_msg, 0, MAX_SIZE);
		int result = recv(new_client->socket, temp_msg, MAX_SIZE, 0);

		if (result != SOCKET_ERROR) {
			if (strcmp("", temp_msg) && strcmp("\n", temp_msg))
				message = new_client->name + ": " + temp_msg;

			cout << message << endl;
			for (auto c : clients) {
				if (c->name != new_client->name)
					send(c->socket, message.c_str(), strlen(message.c_str()), 0);
			}
		}
		else {
			message = new_client->name + " disconnected";
			cout << message << endl;
			closesocket(new_client->socket);
			int cn = -1;
			for (int i = 0; i < clients.size(); i++) {
				if (clients[i]->name != new_client->name)
					send(clients[i]->socket, message.c_str(), strlen(message.c_str()), 0);
				else cn = i;
			}
			//
			clients.erase(clients.begin() + cn);
			return 0;
		}
	}
}*/


int client_handler(vector<ClientInfo>& clients, int n) {
	string message = "";
	char temp_msg[MAX_SIZE] = "";
	memset(temp_msg, 0, MAX_SIZE);
	int result = recv(clients[n].socket, temp_msg, MAX_SIZE, 0);

	if (result != SOCKET_ERROR) {
		for (auto c : clients) {
			if (c.name == temp_msg) {
				message = "-ok";
				send(clients[n].socket, message.c_str(), strlen(message.c_str()), 0);
				closesocket(clients[n].socket);
				return 0;
			}
		}
		message = "ok";
		send(clients[n].socket, message.c_str(), strlen(message.c_str()), 0);
		clients[n].name = temp_msg;
	}
	else return 0;
	ClientInfo new_client = clients[n];

	while (true) {
		memset(temp_msg, 0, MAX_SIZE);
		int result = recv(new_client.socket, temp_msg, MAX_SIZE, 0);

		if (result != SOCKET_ERROR) {
			if (strcmp("", temp_msg) && strcmp("\n", temp_msg))
				message = new_client.name + ": " + temp_msg;

			cout << message << endl;
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i].socket != INVALID_SOCKET && i != n)
					send(clients[i].socket, message.c_str(), strlen(message.c_str()), 0);
			}
		}
		else {
			message = new_client.name + " disconnected";
			cout << message << endl;
			closesocket(new_client.socket);

			closesocket(new_client.socket);
			clients[n].socket = INVALID_SOCKET;
			clients[n].name = "";

			//Broadcast the disconnection message to the other clients
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i].socket != INVALID_SOCKET)
					send(clients[i].socket, message.c_str(), strlen(message.c_str()), 0);
			}
			break;
		}
	}
	return 0;
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

	result = getaddrinfo(static_cast<PCSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &server);
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


	string message = "";
	/*
	vector<ClientInfo*> clients;
	vector<thread> myThreads;
	*/
	vector<ClientInfo> clients(MAX_CLIENTS);
	vector<thread> myThreads(MAX_CLIENTS);
	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].name = "";
		clients[i].socket = INVALID_SOCKET;
	}

	while (true) {
		int client_socket = accept(server_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) continue;
		int i = 0;
		for (; i < MAX_CLIENTS; i++) {
			if (clients[i].socket == INVALID_SOCKET) {
				clients[i].socket = client_socket;
				myThreads[i] = thread(client_handler, ref(clients), i);
				break;
			}
		}
		if (i == MAX_CLIENTS) {
			message = "Server is full";
			send(client_socket, message.c_str(), strlen(message.c_str()), 0);
		}
		/*
		clients.push_back(t);
		myThreads.push_back(thread(client_handler, ref(t), ref(clients)));
		*/
	} 

	closesocket(server_socket);

	/*
	for (int i = 0; i < clients.size(); i++) {
		myThreads[i].detach();
		closesocket(clients[i]->socket);
	}
	*/
	for (int i = 0; i < MAX_CLIENTS; i++) {
		myThreads[i].detach();
		closesocket(clients[i].socket);
	}

	WSACleanup();

	system("pause");
	return 0;
}