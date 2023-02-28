// BALJ17058609 - baldo jérôme 

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>


using namespace std;
using namespace filesystem;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int Initialisation(addrinfo& hints, WSADATA& wsaData)
{
    int resultat = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (resultat != 0)
    {
        return resultat;
    }
    ZeroMemory(&hints, sizeof(&hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    return 0;
}
int LiaisonServeur(addrinfo& hints, addrinfo*& result, addrinfo*& ptr, SOCKET& ConnectSocket)
{
    int resultat;
    resultat = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    //iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (resultat != 0) {
        cerr << "getaddrinfo a echoue avec l'erreur : " << resultat;
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cerr << "socket a echoue avec l'erreur : " << WSAGetLastError() << endl;
            WSACleanup();
            return 1;
        }

        // Connect to server.
        resultat = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (resultat == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        cerr << "Impossible de se connecter au serveur" << endl;
        WSACleanup();
        return 1;
    }

    return 0;
}
string TentativeConnex()
{
    string identifiant;
    string motPasse;

    cout << "Se connecter au serveur" << endl;
    cout << "Identifiant : ";
    cin >> identifiant;
    cout << "Mot de passe : ";
    cin >> motPasse;

    string enteteConnex = "connexion\\" + identifiant + "\\" + motPasse + "\\";

    return enteteConnex;
}

int __cdecl main(int argc, char** argv)
{
    /////////////////////////////////////////////
    //INSTANCIATION ET INITIALISATION DES VARIABLES 
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    // Déclaration et initialisation des variables
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints = { 0 };
    //char de reception 
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    //char d'envoi A FAIRE
    int iResult;
    /////////////////////////////////////////////

    /*////////////////////////////////////////////
    INITIALISATION DE WINSOCK + LIAISON SERVEUR*/
    iResult = Initialisation(hints, wsaData);
    if (iResult != 0) { return 1; } 
    iResult = LiaisonServeur(hints, result, ptr, ConnectSocket);
    if (iResult != 0) { return 1; }

    /////////////////////////////////////////////
    //CHOIX DU CHEMIN servira pour mettre les copies 
    string extractChemin;
    cout << "Choississez votre dossier ou mettre les fichiers a telecharger" << endl;
    cout << "Faites copier-coller du chemin du dossier : ";
    cin >> extractChemin;
    string cheminTraite;
    for (char c : extractChemin)
    {
        if (c == '\\')
        {
            cheminTraite += '\\';
        }

        cheminTraite += c;
    }
    cout << endl << endl;
    /////////////////////////////////////////////
    //TENTATIVE CONNEXION AVEC ID + MDP
    bool connex = false;  
    while (!connex)
    {
        string connexion = TentativeConnex();
        iResult = send(ConnectSocket,connexion.c_str(), connexion.size(), 0);
        if (iResult == SOCKET_ERROR)
        {
            cerr << "Erreur lors de l'envoi des donnees : " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult == SOCKET_ERROR)
        {
            cerr << "Erreur lors de la reception pour la reponse de connection : " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        vector<string> reponse;
        string repConnex;
        for (int i = 0; i < iResult; i++)
        {
            if (recvbuf[i] == '\\')
            {
                reponse.push_back(repConnex);
                repConnex = "";
            }
            else
            {
                repConnex += recvbuf[i];
            }
        }
        
        if (reponse[0] == "connectionOk")
        {
            cout << "----- CONNECTION ETABLIT -----" << endl << endl;
            connex = true;
        }
        else
        {
            cout << "----- ID / MDP INCORRECT -----" << endl << endl;
        }
    }
    /////////////////////////////////////////////
    //DIALOGUE POUR LISTE FICHIERS + TELECHARGEMENT FICHIER
    bool deconnex = false;
    while (!deconnex)
    {
        //envoi de l'entete listTel (liste de telechargement pour avoir les fichiers à disposition)
        string demandList = "listTel";
        iResult = send(ConnectSocket, demandList.c_str(), demandList.size(), 0);
        if (iResult == SOCKET_ERROR)
        {
            cerr << "Erreur lors de l'envoi de l'entete pour la liste : " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        //reception de la liste des fichiers pouvant être téléchargés 
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult == SOCKET_ERROR)
        {
            cerr << "Erreur lors de la reception de la liste de fichiers : " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        string listTele;
        for (int i = 0; i < iResult; i++)
        {
            listTele += recvbuf[i];
        }
        vector<string>list;
        string membre = "";
        for (auto it : listTele)
        {
            if (it == '\\')//la liste a été envoyé sous forme de string avec "\\" pour chaque ligne
            {
                list.push_back(membre);//vecteur servira pour l'affichage de chaque ligne 
                membre = "";
            }
            else
            {
                membre += it;
            }
        }
        //affichage des options pour le client 
        cout << "----- CHOIX TELECHARGEMENT OU DECONNECTION -----" << endl;
        int choix;
        int numFichier = 1;
        for (auto it : list)
        {
            cout << numFichier << "- " << it << endl;//affichage ligne par ligne en précisant le rang du 
            numFichier++;
        }
        cout << numFichier << "- Deconnexion" << endl;//incrementation permet d'afficher la deconnection
        cout << "Choisissez un numero " << endl << endl;
        cin >> choix;
        if (choix == numFichier)//si le choix est égal au dernier alors on sait que le client demande la deconnexion 
        {
            //envoi de l'entete de deconnection pour informer le serveur de supprimer ce socket
            string deconnec = "deconnection";
            iResult = send(ConnectSocket, deconnec.c_str(), deconnec.size(), 0);
            if (iResult == SOCKET_ERROR)
            {
                cerr << "Erreur lors de l'envoi de l'entete pour la liste : " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
            //une fois l'envoi fini deconnection + sortie de boucle
            cout << "----- DECONNECTION CLIENT -----" << endl; 
            deconnex = true;
        }
        else // tous les autres sont des demandes de téléchargement de fichiers 
        {
            cout << "----- TELECHARGEMENT FICHIER -----" << endl;
            cout << "Demande de fichier a telecharger - " << list[choix-1] << endl;
            string fichierTelecharg = to_string(choix-1);//tableau de 0 à n
            iResult = send(ConnectSocket, fichierTelecharg.c_str(), fichierTelecharg.size(), 0);//envoi de la position 
            if (iResult == SOCKET_ERROR)
            {
                cerr << "Erreur lors de l'envoi de l'entete pour telecharg fichier : " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
           
            // Lecture du fichier par morceaux et écriture dans le fichier
            string nomFichier = list[choix - 1];
            string cheminFichier = cheminTraite + "\\" + nomFichier;
            //d'abord réception de la taille pour préparer la réception du fichier
            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            if (iResult == SOCKET_ERROR)
            {
                cerr << "Erreur lors de la reception de la liste de fichiers : " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
            string taille;
            for (int i = 0; i < iResult; i++)
            {
                taille += recvbuf[i];
            }
            int tailleFichierRecp = stoi(taille);
            
            //configuration d'un buffer à la taille du fichier qui sera réceptionné
            char* buffer = new char[tailleFichierRecp];
            int bytesRecus = 0;
            while (bytesRecus < tailleFichierRecp)//incremente des bytes jusquà la taille total du fichier 
            {
                //decrementation de la taille totale restant à réceptionner 
                //incrementation de la position sur le buffer. permet de le remplir  
                int result = recv(ConnectSocket, buffer + bytesRecus, tailleFichierRecp - bytesRecus, 0);
                if (result == SOCKET_ERROR)
                {
                    cerr << "Erreur lors de la reception du fichier : " << WSAGetLastError() << endl;
                    closesocket(ConnectSocket);
                    WSACleanup();
                    delete[] buffer;
                    return 1;
                }
                bytesRecus += result;//incrementation 
            }

            //chemin deja selectionne + nomfichier (nom + extension)
            ofstream fichier(cheminFichier, ios::out | ios::binary);

            // Vérification si le fichier a bien été créé
            if (!fichier.is_open())
            {
                cerr << "Erreur lors de la creation du fichier" << endl;
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
            // Écriture des données binaires dans le fichier
            fichier.write(buffer, tailleFichierRecp);
            // Fermeture du fichier
            fichier.close();
            delete[] buffer;//effacer le buffer pour éviter les fuites de mémoire
            cout << "Reussite du telechargement de - " << list[choix - 1] << endl << endl;
        }
    }
    //shutown du send et receive mais pas forcement utile
    iResult = shutdown(ConnectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}





