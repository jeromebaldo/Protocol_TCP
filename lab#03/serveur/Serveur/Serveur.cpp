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
    int iResult; //resultat de l'initiliasation stock� dedans 

    //initialisation de socker d'�coute et du socket du client 
    /*
    - cr�ation d'1 socket d'�coute
    - cr�ation d'1 socket client (accepte connexion client)
    - pointeur result (type struct addrinfo) (stockage info adresses r�solues)
    - hints (type struct addrinfo) (stockage crit�res pour r�solution noms d'h�tes)
    - iSendResult stocke r�sultat de la fonction send(envoie data � client connect�)
    - recvbuf => stockage data re�ues � partir d'un client(taille = DEFAULT_BUFLEN)
    - recvbuflen => d�finition de la longueur maximale des donn�es � recevoir
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

    //////////d�finition de la structure hints 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;//famille @ � utiliser(AF_INET => IP version 4 utilis�e)
    hints.ai_socktype = SOCK_STREAM;//type socket � utiliser(SOCK_STREAM => TCP)
    hints.ai_protocol = IPPROTO_TCP;//protocole � utiliser(IPPROTO_TCP =>  TCP)
    hints.ai_flags = AI_PASSIVE;//d�finit indicateurs pour obtention de l'adresse(AI_PASSIVE => utilis�e pour accepter connexions entrantes)

    /*
    r�solution d'adresses et ports pour le socket :
        - arguments en entr�e : 
            "NULL" => pas connaissance du nom d'h�te pour r�solution informations d'adresse.
            "DEFAULT_PORT" => constante contenant num�ro de port o� les paquets vont transiter
            "&hints" => pointeur vers structure "hints" (r�solution de noms d'h�tes)
            "&result" => pointeur vers structure "addrinfo" (rempli avec getaddrinfo)
        - si retour de valeur diff�rent de 0 alors �chec
    */
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) 
    {
        cout << "getaddrinfo failed with error: " << iResult;
        WSACleanup(); //lib�ration des ressources Windows Sockets
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
    La fonction bind est utilis�e pour lier une adresse socket � une socket donn�e. 
    - en entr�e =>
        - ListenSocket(socket � lier) 
        - result->ai_addr(structure d'@) 
        - (int)result->ai_addrlen(longueur @)
    - Si probl�me liaison adresse socket au socket, retourne code erreur  
    */
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result); //vider la structure d'adresse 
        closesocket(ListenSocket);//fermer le socket d'�coute 
        WSACleanup(); //vider 
        return 1;
    }
    
    freeaddrinfo(result);//vider l'adresse 
    //////////////////////////////////////////////////
    /////////////////////////////////////////////////
    /*
    op�ration d'�coute sur socket ListenSocket.
    - fonction listen utilis�e pour mettre socket en mode �coute pour connexions entrantes. 
    - Le premier argument => socket � �couter
    - second argument => SOMAXCONN (nombre max de connexions en attente autoris� simultan�ment)
    - fonction listen retourne 0 si OK sinon KO retourne n�gatif. 
    - Si iResult = SOCKET_ERROR => erreur  
    - WSAGetLastError => obtenir code d'erreur associ� 
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
        - accepter une connexion entrante sur un socket d'�coute appel� "ListenSocket"
        - accept attend une connexion entrante sur le socket d'�coute "ListenSocket", 
        - puis renvoie un nouveau socket qui peut �tre utilis� pour communiquer avec le client connect�. 
        - Ce nouveau socket est stock� dans la variable "ClientSocket".
        - "NULL" et "NULL", sont des pointeurs vers des structures de donn�es qui peuvent �tre utilis�es pour obtenir des informations sur le client connect�. 
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
        M�thode pour recevoir les donn�es du socket client:
            - fonction recv => recevoir donn�es envoy�es par client � travers ClientSocket.
            - Les donn�es re�ues sont stock�es dans tampon recvbuf de taille recvbuflen.
            - La boucle s'ex�cute tant que la valeur renvoy�e par recv est sup�rieure � 0:
                - des donn�es ont �t� re�ues avec succ�s.
            - Si recv renvoie valeur > 0 => donn�es re�ues = OK
                - fonction send => renvoyer les donn�es re�ues au client
            - Si recv renvoie 0 => connexion a �t� ferm�e par client.
            - Si recv renvoie valeur n�gative => erreur est survenue pendant r�ception data
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
        effectue une op�ration de "shutdown" sur un socket de client appel� "ClientSocket".
        shutdownest => arr�ter une communication sur socket ClientSocket
        SD_SEND => sp�cifie que la communication d'envoi doit �tre arr�t�e.
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