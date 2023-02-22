#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

using namespace std;

//constantes globales 
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

vector<string> RecupAuthClient(SOCKET ListenSocket, SOCKET& ClientSocket, int& iResult)
{
    vector<string> data;
    vector<string> erreur;
    int data_count;

    iResult = recv(ClientSocket, (char*)&data_count, sizeof(data_count), 0);
    if (iResult == SOCKET_ERROR)
    {
        std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return  erreur;
    }

    data.reserve(data_count);
    for (int i = 0; i < data_count; ++i)
    {
        int str_size = 0;
        std::string str;
        iResult = recv(ClientSocket, (char*)&str_size, sizeof(str_size), 0);
        if (iResult == SOCKET_ERROR)
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
            return erreur;
        }
        str.resize(str_size);
        iResult = recv(ClientSocket, &str[0], str_size, 0);
        if (iResult == SOCKET_ERROR)
        {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
            return erreur;
        }

        data.push_back(str);
    }

    return data;
}

bool VerifUtilisateur(vector<string>authentif)
{
    struct profil {
        string ID;
        string MDP;
    };
    vector<profil> utilisateurs;
    utilisateurs.push_back({ "jerome", "1234" });
    utilisateurs.push_back({ "loic", "1234" });
    utilisateurs.push_back({ "josephine", "1234" });
       
    bool Verif = false;
    for (auto it : utilisateurs)
    {
        if (it.ID == authentif[1] && it.MDP == authentif[2])
        {
            return Verif = true;
        }
    }

    return Verif;
}

int __cdecl main(void)
{
    WSADATA wsaData; //initialisation pour utuliser winsock
    int iResult; //resultat de l'initiliasation stocké dedans 

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL; //
    struct addrinfo hints; //

    int iSendResult; //
    char recvbuf[DEFAULT_BUFLEN]; //
    int recvbuflen = DEFAULT_BUFLEN; //

    //Initialisation de Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //version 2.2 
    if (iResult != 0) 
    {
        cout << "WSAStartup failed with error: " << iResult << endl;
        return 1;
    }

    //////////définition de la structure hints 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;//famille @ à utiliser(AF_INET => IP version 4 utilisée)
    hints.ai_socktype = SOCK_STREAM;//type socket à utiliser(SOCK_STREAM => TCP)
    hints.ai_protocol = IPPROTO_TCP;//protocole à utiliser(IPPROTO_TCP =>  TCP)
    hints.ai_flags = AI_PASSIVE;//définit indicateurs pour obtention de l'adresse(AI_PASSIVE => utilisée pour accepter connexions entrantes)

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) 
    {
        cout << "getaddrinfo failed with error: " << iResult;
        WSACleanup(); //libération des ressources Windows Sockets
        return 1; //fin application avec un retour de 1
    }
    
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) 
    {
        cout << "socket failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) 
    {
        cout << "bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result); //vider la structure d'adresse 
        closesocket(ListenSocket);//fermer le socket d'écoute 
        WSACleanup(); //vider 
        return 1;
    }
    
    freeaddrinfo(result);//vider l'adresse 
    
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) 
    {
        cout << "listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        cout << "accept failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        vector<string> data = RecupAuthClient(ListenSocket, ClientSocket, iResult);
        if (!data.empty() && VerifUtilisateur(data) == true)
        {
            
            cout << "Received data:" << std::endl;
            for (const auto& str : data)
            {
                cout << str << endl;
            }
            const char* repConnex = "connectionOK";
            int taille = 12;

            iSendResult = send(ClientSocket, repConnex, taille, 0);
        }
        else
        {
            cout << "renvoyer un message d'erreur et couper " << endl;
        }
        
  
    }
    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}