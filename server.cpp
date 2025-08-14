#include <iostream>
#include <winsock2.h>
#include <vector>
#include<string>
using namespace std; 

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 5
#define PORT 8080


void printServer()
{
    vector<string> S = {
        " ##### ",
        "#      ",
        " ##### ",
        "      #",
        " ##### " };

    vector<string> E = {
        "###### ",
        "#      ",
        "#####  ",
        "#      ",
        "###### " };

    vector<string> R = {
        "#####  ",
        "#   #  ",
        "#####  ",
        "#  #   ",
        "#   #  " };

    vector<string> V = {
        "#     #",
        "#     #",
        " #   # ",
        "  # #  ",
        "   #   " };

    // Print each row of all letters side by side
    for (int i = 0; i < 5; ++i)
    {
        cout << S[i] << "  ";
        cout << E[i] << "  ";
        cout << R[i] << "  ";
        cout << V[i] << "  ";
        cout << E[i] << "  ";
        cout << R[i] << endl;
    }
}





int main() {

    printServer(); 

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1; 
    }; 
    //                              Ipv4(TCP) 
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, MAX_CLIENTS);

    fd_set master_set, temp_set;
    FD_ZERO(&master_set);
    FD_SET(serverSocket, &master_set);

    std::vector<SOCKET> clientSockets;

    while (true) {
        temp_set = master_set;
        struct timeval timeout = { 1, 0 };  // 1-second timeout

        int activity = select(0, &temp_set, NULL, NULL, &timeout);

        if (activity == SOCKET_ERROR) {
            std::cerr << "Select failed!\n";
            break;
        }

        if (FD_ISSET(serverSocket, &temp_set)) {
            SOCKET newClient = accept(serverSocket, NULL, NULL);
            clientSockets.push_back(newClient);
            FD_SET(newClient, &master_set);
            std::cout << "New client connected!\n";
        }

        for (auto it = clientSockets.begin(); it != clientSockets.end(); ) {
            SOCKET clientSock = *it;
            if (FD_ISSET(clientSock, &temp_set)) {
                char buffer[1024];
                int recvBytes = recv(clientSock, buffer, sizeof(buffer), 0);
                if (recvBytes <= 0) {
                    closesocket(clientSock);
                    FD_CLR(clientSock, &master_set);
                    it = clientSockets.erase(it);
                    std::cout << "Client disconnected.\n";
                }
                else {
                  
                    if (recvBytes < sizeof(buffer))
                        buffer[recvBytes] = '\0';
                    std::cout << "Received: " << buffer << std::endl;
                    send(clientSock, buffer, recvBytes, 0);
                    ++it; 
                }
            }
            else {
                ++it;
            }
        }
    }

    WSACleanup();
    return 0;
}
