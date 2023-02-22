#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

vector<string> AuthenUtil()
{
    cout << "Connexion etablie - veuillez maintenant vous identifier" << endl;
    string ID;
    string MDP;
    string entete = "tentative Connexion";
    vector<string> data;

    cout << "Identifiant : ";
    cin >> ID;
    cout << "Mot de passe : ";
    cin >> MDP;

    data.push_back(entete);
    data.push_back(ID);
    data.push_back(MDP);

    return data;
}

int __cdecl main(int argc, char** argv)
{
    //creation des parametres de sockets 
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    
    

    /*
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }*/
    ////initialisation des parametres 
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //adresse IP du serveur 
    //const char* IP = "192.168.2.13";
    //const char* IP = "172.30.0.33";
    const char* IP = "172.20.224.45";
    
    //liaison avec le serveur 
    // Resolve the server address and port
    iResult = getaddrinfo(IP, DEFAULT_PORT, &hints, &result);
    //iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Unable to connect to server!" << endl;
        WSACleanup();
        return 1;
    }

    bool appliRoule = true;
    bool connexionOK = false;
    while (appliRoule)
    {
        //while (!connexionOK)
        
            vector<string>data = AuthenUtil();
            const int data_count = data.size();

            //envoie de la taille 
            iResult = send(ConnectSocket, (char*)&data_count, sizeof(data_count), 0);
            if (iResult == SOCKET_ERROR)
            {
                cout << "send failed with error: " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }

            for (const std::string& str : data) 
            {
                int str_size = str.size();
                iResult = send(ConnectSocket, (char*)&str_size, sizeof(str_size), 0);
                if (iResult == SOCKET_ERROR) 
                {
                    cout << "send failed with error: " << WSAGetLastError() << endl;
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }

                iResult = send(ConnectSocket, str.c_str(), str_size, 0);
                if (iResult == SOCKET_ERROR) 
                {
                    cout << "send failed with error: " << WSAGetLastError() << endl;
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
            }

            do {

                iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0)
                {
                    for (int i = 0; i < iResult; i++)
                    {
                        cout << recvbuf[i];
                    }
                    cout << endl;
                }
                else if (iResult == 0)
                {
                    
                }
                else
                {
                    cout << "recv failed with error: " << WSAGetLastError() << endl;
                }


            } while (iResult > 0);
    }
    
    // cleanup et fin de l'application
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}