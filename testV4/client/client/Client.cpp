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

void connection(SOCKET ConnectSocket, int iResult)
{
    string connexion = "connexion-ident-mdp-";

    const char* connection = connexion.c_str();
    iResult = send(ConnectSocket, connection, connexion.size(), 0);
    // On utilise connexion.size() pour envoyer la taille de la chaîne,
    // au lieu de sizeof(connection) qui renvoie la taille du pointeur.

    if (iResult == SOCKET_ERROR) {
        cout << "Erreur lors de l'envoi des données : " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    //faire la réponse 
    //puis renvoyer un booléen 


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

    const char* sendbuf = "tentative de connection";

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
    const char* IP = "172.30.0.216";
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
    connection(ConnectSocket, iResult);
    
    bool connection = false;
    while (!connection)
    {
        //connexion 
        
        

        //SINON CELA SERA DES VECTEURS DE UNSIGNED CHAR pour les fichiers

        ///faire une methode permettant de rajouter les séparateurs 
        //faire une méthode permettant d'enlever les séparateurs 
        // 
        //envoyer un vecteur de string
        // envoyer vecteur de binaire 
        // switch connection Ok 
        // switch connection KO
        // switch demande 
        //switch deconnexion 
    }
    string listFichier = "listFichier-";
    string telechargement = "telechg-numFichier";

    //reponse du serveur 
    string connexionKO = "connexionKo-";
    string listFichierOk = "listFichierOk-";
    string telecharOk = "telechargOk-titre-extension-";
    string telecharKo = "telechgKo-motif-";

    return 0;
}