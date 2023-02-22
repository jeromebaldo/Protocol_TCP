#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

using namespace std;

//constantes globales 
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(void)
{
    WSADATA wsaData; //initialisation pour utuliser winsock
    int iResult; //resultat de l'initiliasation stocké dedans 

    //initialisation de socker d'écoute et du socket du client 
    /*
    - création d'1 socket d'écoute
    - création d'1 socket client (accepte connexion client)
    - pointeur result (type struct addrinfo) (stockage info adresses résolues)
    - hints (type struct addrinfo) (stockage critères pour résolution noms d'hôtes)
    - iSendResult stocke résultat de la fonction send(envoie data à client connecté)
    - recvbuf => stockage data reçues à partir d'un client(taille = DEFAULT_BUFLEN)
    - recvbuflen => définition de la longueur maximale des données à recevoir
    */
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

    /*
    résolution d'adresses et ports pour le socket :
        - arguments en entrée : 
            "NULL" => pas connaissance du nom d'hôte pour résolution informations d'adresse.
            "DEFAULT_PORT" => constante contenant numéro de port où les paquets vont transiter
            "&hints" => pointeur vers structure "hints" (résolution de noms d'hôtes)
            "&result" => pointeur vers structure "addrinfo" (rempli avec getaddrinfo)
        - si retour de valeur différent de 0 alors échec
    */
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) 
    {
        cout << "getaddrinfo failed with error: " << iResult;
        WSACleanup(); //libération des ressources Windows Sockets
        return 1; //fin application avec un retour de 1
    }
    //////////////////////////////////////////////////
    /////////////////////////////////////////////////

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) 
    {
        cout << "socket failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    //////////////////////////////////////////////////
    /////////////////////////////////////////////////
    
    /*
    La fonction bind est utilisée pour lier une adresse socket à une socket donnée. 
    - en entrée =>
        - ListenSocket(socket à lier) 
        - result->ai_addr(structure d'@) 
        - (int)result->ai_addrlen(longueur @)
    - Si problème liaison adresse socket au socket, retourne code erreur  
    */
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result); //vider la structure d'adresse 
        closesocket(ListenSocket);//fermer le socket d'écoute 
        WSACleanup(); //vider 
        return 1;
    }
    
    freeaddrinfo(result);//vider l'adresse 
    //////////////////////////////////////////////////
    /////////////////////////////////////////////////
    /*
    opération d'écoute sur socket ListenSocket.
    - fonction listen utilisée pour mettre socket en mode écoute pour connexions entrantes. 
    - Le premier argument => socket à écouter
    - second argument => SOMAXCONN (nombre max de connexions en attente autorisé simultanément)
    - fonction listen retourne 0 si OK sinon KO retourne négatif. 
    - Si iResult = SOCKET_ERROR => erreur  
    - WSAGetLastError => obtenir code d'erreur associé 
    - La fonction retourne 1 pour indiquer qu'une erreur s'est produite.
    */
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) 
    {
        cout << "listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //////////////////////////////////////////////////
    /////////////////////////////////////////////////


    /*
    Acceptation de la connexion du socket client:
        - accepter une connexion entrante sur un socket d'écoute appelé "ListenSocket"
        - accept attend une connexion entrante sur le socket d'écoute "ListenSocket", 
        - puis renvoie un nouveau socket qui peut être utilisé pour communiquer avec le client connecté. 
        - Ce nouveau socket est stocké dans la variable "ClientSocket".
        - "NULL" et "NULL", sont des pointeurs vers des structures de données qui peuvent être utilisées pour obtenir des informations sur le client connecté. 
        - dans accept il n'y a pas besoin 
    */
    while (true)
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            cout << "accept failed with error: " << WSAGetLastError() << endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        //permet de fermer le socket 
        //closesocket(ListenSocket);

        /*
        Méthode pour recevoir les données du socket client:
            - fonction recv => recevoir données envoyées par client à travers ClientSocket.
            - Les données reçues sont stockées dans tampon recvbuf de taille recvbuflen.
            - La boucle s'exécute tant que la valeur renvoyée par recv est supérieure à 0:
                - des données ont été reçues avec succès.
            - Si recv renvoie valeur > 0 => données reçues = OK
                - fonction send => renvoyer les données reçues au client
            - Si recv renvoie 0 => connexion a été fermée par client.
            - Si recv renvoie valeur négative => erreur est survenue pendant réception data
        */
        do
        {
            
            
            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            cout << recvbuf[0] << endl;

            if (iResult > 0)
            {
                
                for (int i = 0; i < iResult; i++)
                {
                    cout << recvbuf[i];
                }
                cout << endl;


                const char* reponse = "connection etablit";
                cout << reponse << endl;
                iSendResult = send(ClientSocket, reponse, iResult, 0);
                //iSendResult = send(ClientSocket, recvbuf, iResult, 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    cout << "send failed with error: " << WSAGetLastError() << endl;
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
                

            }
            else if (iResult == 0)
            {
                cout << "Connection closing..." << endl;
            }
            else
            {
                cout << "recv failed with error: " << WSAGetLastError() << endl;
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

        } while (iResult > 0);

        /*
        effectue une opération de "shutdown" sur un socket de client appelé "ClientSocket".
        shutdownest => arrêter une communication sur socket ClientSocket
        SD_SEND => spécifie que la communication d'envoi doit être arrêtée.
        */
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
            cout << "shutdown failed with error: " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    }
    

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}