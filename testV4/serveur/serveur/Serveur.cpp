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


std::vector<std::string> extraire_mots(const std::string& chaine) {
    std::vector<std::string> mots;
    std::string::size_type debut_mot = 0;
    std::string::size_type fin_mot = chaine.find('-');

    while (fin_mot != std::string::npos) {
        std::string mot = chaine.substr(debut_mot, fin_mot - debut_mot);
        if (!mot.empty()) {  // ajouter le mot seulement s'il n'est pas vide
            mots.push_back(mot);
        }
        debut_mot = fin_mot + 1;
        fin_mot = chaine.find('-', debut_mot);
    }

    std::string dernier_mot = chaine.substr(debut_mot);
    if (!dernier_mot.empty()) {  // ajouter le dernier mot seulement s'il n'est pas vide
        mots.push_back(dernier_mot);
    }

    return mots;
}


int __cdecl main(void)
{
    WSADATA wsaData; //initialisation pour utuliser winsock
    int iResult; //resultat de l'initiliasation stock� dedans 

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

    //////////d�finition de la structure hints 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;//famille @ � utiliser(AF_INET => IP version 4 utilis�e)
    hints.ai_socktype = SOCK_STREAM;//type socket � utiliser(SOCK_STREAM => TCP)
    hints.ai_protocol = IPPROTO_TCP;//protocole � utiliser(IPPROTO_TCP =>  TCP)
    hints.ai_flags = AI_PASSIVE;//d�finit indicateurs pour obtention de l'adresse(AI_PASSIVE => utilis�e pour accepter connexions entrantes)

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        cout << "getaddrinfo failed with error: " << iResult;
        WSACleanup(); //lib�ration des ressources Windows Sockets
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
        closesocket(ListenSocket);//fermer le socket d'�coute 
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
        
        
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        string result;
        for (int i = 0; i < iResult; i++)
        {
            result += recvbuf[i];
        }
        cout << result << endl;
        //connection intitulé 
        //reçois le fichier dentete et fais un switch 
        //switch connection
        //switch listeFichier 
        //switch telechargement 
        //deconnexion 
    }

    return 0;

}