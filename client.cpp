#include <winsock2.h>
#include <windows.h> // DWORD and multiThreading with OS ... 
#include <ws2tcpip.h>
#include <iostream>
#include <string>
//#include <vector>
using namespace std;

#pragma comment(lib, "ws2_32.lib")





//void printClient()
//{
//    vector<string> C = {
//        " ###### ",
//        " #      ",
//        " #      ",
//        " #      ",
//        " ###### " };
//    vector<string> L = {
//        " #      ",
//        " #      ",
//        " #      ",
//        " #      ",
//        " ###### " };
//    vector<string> I = {
//        " ##### ",
//        "   #   ",
//        "   #   ",
//        "   #   ",
//        " ##### " };
//    vector<string> E = {
//        "###### ",
//        "#      ",
//        "#####  ",
//        "#      ",
//        "###### " };
//
//    vector<string> N = {
//        "#     #",
//        "##    #",
//        "# #   #",
//        "#  #  #",
//        "#   ###" };
//
//    vector<string> T = {
//        "#######",
//        "   #   ",
//        "   #   ",
//        "   #   ",
//        "   #   " };
//
//    for (int i = 0; i < 6; i++)
//    {
//        std::cout << C[i] << " ";
//        std::cout << L[i] << " ";
//        std::cout << I[i] << " ";
//        std::cout << E[i] << " ";
//        std::cout << N[i] << " ";
//        std::cout << T[i] << "  " << std::endl;
//    }
//}


SOCKET clientSocket = INVALID_SOCKET;
bool running = true;
CRITICAL_SECTION cs; // Very important here... as we have multiple client which can talk to server so revise your
//OS class

DWORD WINAPI SendThread(LPVOID lpParam);
DWORD WINAPI RecvThread(LPVOID lpParam);

int main() {
   /* printClient(); */
    WSADATA wsaData;
    SOCKADDR_IN serverAddr;

    // winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // creating a socket  ipv4   TCP 
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

 
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server. Type messages to send (type 'exit' to quit)." << std::endl;

    // inside critical section to handle only one thread to send and recive data at a time
    InitializeCriticalSection(&cs);

    // creating threads
    HANDLE threads[2];
    threads[0] = CreateThread(NULL, 0, SendThread, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, RecvThread, NULL, 0, NULL);

    if (threads[0] == NULL || threads[1] == NULL) {
        std::cerr << "Thread creation failed: " << GetLastError() << std::endl;
        running = false;
    }

    // waiting for other threads to finish
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    CloseHandle(threads[0]);
    CloseHandle(threads[1]);
    closesocket(clientSocket);
    DeleteCriticalSection(&cs);
    WSACleanup();

    return 0;
}

DWORD WINAPI SendThread(LPVOID lpParam) {
    std::string message;

    while (running) {
        std::getline(std::cin, message);

        
        if (message == "exit") {
            EnterCriticalSection(&cs);
            running = false;
            LeaveCriticalSection(&cs);
            break;
        }

        // send message to server
        int sendResult = send(clientSocket, message.c_str(), message.length(), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            EnterCriticalSection(&cs);
            running = false;
            LeaveCriticalSection(&cs);
            break;
        }
    }
    return 0;
}

DWORD WINAPI RecvThread(LPVOID lpParam) {
    fd_set readSet;
    timeval timeout = { 0, 100000 };  // 100ms timeout

    while (true) {
        // exit condition
        EnterCriticalSection(&cs);
        if (!running) {
            LeaveCriticalSection(&cs);
            break;
        }
        LeaveCriticalSection(&cs);

        // initialize socket set
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);

        //  waiting for socket activity
        int selectResult = select(0, &readSet, NULL, NULL, &timeout);
        if (selectResult == SOCKET_ERROR) {
            std::cerr << "Select error: " << WSAGetLastError() << std::endl;
            break;
        }

        // incoming data
        if (selectResult > 0 && FD_ISSET(clientSocket, &readSet)) {
            char buffer[4096];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::cout << "Received: " << buffer << std::endl;
            }
            else if (bytesReceived == 0) {
                std::cout << "Server disconnected" << std::endl;
                EnterCriticalSection(&cs);
                running = false;
                LeaveCriticalSection(&cs);
                break;
            }
            else {
                std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
                EnterCriticalSection(&cs);
                running = false;
                LeaveCriticalSection(&cs);
                break;
            }
        }
    }
    return 0;
}


// so you need to have the knowldege about NETWORking and then mutliThreading with OS module and data.. like buffer and all
// socket api that's all but in post i will explain 
