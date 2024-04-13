#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct ClientInfo {
    SOCKET socket;
    string username;
};

vector<ClientInfo> clients;

void BroadcastMessage(const string& message, SOCKET sender) {
    for (const auto& client : clients) {
        if (client.socket != sender) {
            send(client.socket, message.c_str(), message.size(), 0);
        }
    }
}

void AddClient(SOCKET clientSocket) {
    ClientInfo newClient;
    newClient.socket = clientSocket;
    clients.push_back(newClient);
}

void RemoveClient(SOCKET clientSocket) {
    auto it = remove_if(clients.begin(), clients.end(), [clientSocket](const ClientInfo& client) {
        return client.socket == clientSocket;
        });
    clients.erase(it, clients.end());
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "27015", &hints, &result) != 0) {
        cout << "getaddrinfo failed\n";
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        cout << "bind failed\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen failed\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Waiting for client connection...\n";

    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            cout << "accept failed\n";
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        char usernameBuffer[1024];
        recv(ClientSocket, usernameBuffer, 1024, 0);
        string username = usernameBuffer;

        AddClient(ClientSocket);
        BroadcastMessage(username + " has joined the chat\n", ClientSocket);
        cout << username << " has joined the chat\n";

        for (const auto& client : clients) {
            if (client.socket != ClientSocket) {
                send(ClientSocket, client.username.c_str(), client.username.size(), 0);
            }
        }

        while (true) {
            char buffer[1024];
            int recvResult = recv(ClientSocket, buffer, sizeof(buffer), 0);
            if (recvResult > 0) {
                string message = username + ": " + buffer;
                cout << message;
                BroadcastMessage(message, ClientSocket);
            }
            else if (recvResult == 0) {
                cout << username << " disconnected\n";
                BroadcastMessage(username + " disconnected\n", ClientSocket);
                RemoveClient(ClientSocket);
                closesocket(ClientSocket);
                break;
            }
            else {
                cout << "recv failed\n";
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
    }

    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}



