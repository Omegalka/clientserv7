#include <iostream>
#include <conio.h> 
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <thread>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET ConnectSocket;

void ReceiveMessages() {
    while (true) {
        char buffer[1024];
        int recvResult = recv(ConnectSocket, buffer, sizeof(buffer), 0);
        if (recvResult > 0) {
            cout << buffer;
        }
        else if (recvResult == 0) {
            cout << "Server disconnected\n";
            break;
        }
        else {
            cout << "recv failed\n";
            closesocket(ConnectSocket);
            WSACleanup();
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo("localhost", "27015", &hints, &result) != 0) {
        cout << "getaddrinfo failed\n";
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "Error at socket(): " << WSAGetLastError() << endl;
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Unable to connect to server\n";
        WSACleanup();
        return 1;
    }

    cout << "Connected to server\n";

    cout << "Enter your username: ";
    string username;
    cin >> username;
    send(ConnectSocket, username.c_str(), username.size(), 0);

    thread receiveThread(ReceiveMessages);

    while (true) {
        string message;
        getline(cin, message);
        send(ConnectSocket, message.c_str(), message.size(), 0);
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}


