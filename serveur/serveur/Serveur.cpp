
//BALJ17058609 - baldo jérôme 

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <thread>
using namespace std;
using namespace filesystem;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

//constantes globales 
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

//mise en place du serveur pour initialisation + ecoutBind + ecoutBindErreur
int Initialisation(addrinfo& hints, addrinfo*& result, WSADATA& wsaData)
{
    //Initialisation de Winsock
    int resultat = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (resultat != 0)
    {
        cerr << "WSAStartup failed with error: " << resultat << endl;
        return resultat;
    }

    //////////dEfinition de la structure hints 
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    resultat = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (resultat != 0)
    {
        cerr << "getaddrinfo failed with error: " << resultat;
        WSACleanup();
        return resultat;
    }

    return resultat;
}
void EcoutBind(SOCKET& ListenSocket, addrinfo*& result, int& iResult)
{
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        return;
    }
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        return;
    }
}
bool EcoutBindErreur(SOCKET& ListenSocket, addrinfo*& result, int& iResult)
{
    if (ListenSocket == INVALID_SOCKET)
    {
        cerr << "socket failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    if (iResult == SOCKET_ERROR)
    {
        cerr << "bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result); //vider la structure d'adresse 
        closesocket(ListenSocket);//fermer le socket d'�coute 
        WSACleanup(); //vider 
        return false;
    }

    return true;
}

//authentification du client 
bool VerifClient(vector<string> enteteInfo)
{
    struct auth {
        string identifiant;
        string mdp;
    };
    vector<auth> authClient;
    authClient.push_back({ "jerome", "1234" });
    authClient.push_back({ "louis", "1234" });
    authClient.push_back({ "elodie", "1234" });
    authClient.push_back({ "loic", "1234" });
    authClient.push_back({ "armand", "1234" });
    authClient.push_back({ "eric", "1234" });
    bool idOk = false;
    for (auto it : authClient)
    {
        if (it.identifiant == enteteInfo[1] && it.mdp == enteteInfo[2])
        {
            idOk = true;
            break;
        }
    }

    return idOk;
}

//fonction arbofichier + recuplist + binarisation pour la gestion des fichiers
struct ArboFichiers
{
    string nomFichier;
    string pathFichier;
    string extension;
};
vector<ArboFichiers> RecupList(path pathToDir)
{
    vector<ArboFichiers> listFichier;
    string chemin = pathToDir.string();//obtention chemin du répertoire 
    if (exists(pathToDir) && is_directory(pathToDir))//si répertoire existe  
    {
        for (const auto& entry : directory_iterator(pathToDir))//traite chaque entrée dans répertoire 
        {
            if (entry.is_regular_file())//si entrée est un fichier régulier extraction
            {
                string nomFichier = entry.path().filename().string();//incrementation avec structure arbo pour chaque fichier 
                string extension = entry.path().extension().string();
                string cheminFichier = chemin + '\\' + nomFichier;
                ArboFichiers elementFichier = { nomFichier,cheminFichier,extension };
                listFichier.push_back(elementFichier);
            }
        }
        
        
        
    }
    return listFichier;
}
vector<unsigned char> Binarisation(string cheminFichier, string nomFichier)
{
    ifstream fileIn(cheminFichier, ios::binary);
    //ouverture en mode binaire 
    if (!fileIn.is_open())
    {
        cerr << "Erreur: impossible d'ouvrir le fichier " << nomFichier << endl;
    }

    vector<unsigned char>fichierBytes((istreambuf_iterator<char>(fileIn)), istreambuf_iterator<char>());
    //iterateur permettant de parcourir le fichier à lire et de remplir le vecteur 
    fileIn.close();

    return fichierBytes;
}

//methode qui isole le client 
int ActiviClient(SOCKET& ClientSocket, int& iResult, char recvbuf[DEFAULT_BUFLEN], int& recvbuflen, int& iSendResult, string cheminTraite)
{
    string identifUtilisateur = "";
    /////PARTIE CONNEXION DU CLIENT
    bool connexion = false;
    while (!connexion)
    {
        vector<string> entet;
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult == SOCKET_ERROR)
        {
            cerr << "Erreur lors de la reception de l'entete de connexion : " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        string membre;
        for (int i = 0; i < iResult; i++)
        {
            if (recvbuf[i] == '\\')
            {
                entet.push_back(membre);
                membre = "";
            }
            else
            {
                membre += recvbuf[i];
            }
        }
        if (entet[0] == "connexion")
        {
            bool auth = VerifClient(entet);
            string identUser;
            if (auth)
            {
                //identification + authentification Ok reponse au client 
                cout << "Connexion etablit - " << entet[1] << endl;
                identUser = "connectionOk\\";
                iSendResult = send(ClientSocket, identUser.c_str(),identUser.size(), 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    cerr << "envoie echoue avec erreur : " << WSAGetLastError() << endl;
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
                connexion = true;
                identifUtilisateur = entet[1];
            }
            else
            {
                // identification et authentification pas bonne alors envoie entete au client pour info 
                cout << "Refus connexion - " << entet[1] << endl;
                identUser = "connectionKo\\";
                iSendResult = send(ClientSocket, identUser.c_str(), identUser.size(), 0);
                if (iSendResult == SOCKET_ERROR)
                {
                    cerr << "envoie echoue avec erreur : " << WSAGetLastError() << endl;
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
            }
        }
    }

    ////// PARTIE LISTE + TELECHARGEMENT 
    bool deconnex = false;
    while (!deconnex)
    {
        vector<string> entet;
        string entete = "";
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult == SOCKET_ERROR)
        {
            cerr << "Erreur lors de la réception de l'entête de dialogue : " << WSAGetLastError() << endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        for (int i = 0; i < iResult; i++)
        {
            if (recvbuf[i] == '\\')
            {
                entet.push_back(entete);
                entete = "";
            }
            else
            {
                entete += recvbuf[i];
            }
        }

        // GESTION DES ENTETES 
        path pathToDir(cheminTraite);
        //path pathToDir("C:\\Users\\baldo\\Desktop\\lab#03_RESEAU8V6\\serveur\\fichiers_stockes");
        if (entete == "listTel")
        {
            cout << "Demande liste de fichiers a telecharger - " << identifUtilisateur << endl;
            vector<ArboFichiers> listFichier = RecupList(pathToDir);
            string list;
            for (auto it : listFichier)
            {
                list += it.nomFichier + "\\";
            }
            iSendResult = send(ClientSocket, list.c_str(), list.size(), 0);
            if (iSendResult == SOCKET_ERROR)
            {
                cerr << "Envoie echoue avec erreur : " << WSAGetLastError() << endl;
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

        }
        else if (entete == "deconnection")
        {
            cout << "Deconnection du client - " << identifUtilisateur << endl;
            deconnex = true;//sortie de  boucle 
        }
        else
        {
            vector<ArboFichiers> listFichier = RecupList(pathToDir);
            int num = stoi(entete);
            cout << "Telechargement du fichier - " << listFichier[num].nomFichier << " - " << identifUtilisateur << endl;
            //recuperation de la version binaire du fichier à envoyer 
            vector<unsigned char> fichierBinarise = Binarisation(listFichier[num].pathFichier, listFichier[num].nomFichier);
            //1er etape : preparer le client en envoyant la taille du fichier à réceptionner
            //on stringifie le fichier binaire pour le faire passer en une fois
            string stringifBinaire(fichierBinarise.begin(), fichierBinarise.end());
            //on calcule la taille de ce string 
            string tailleFichier = to_string(stringifBinaire.size());
            iSendResult = send(ClientSocket, tailleFichier.c_str(), tailleFichier.size(), 0);
            if (iSendResult == SOCKET_ERROR)
            {
                cerr << "envoie echoue avec erreur : " << WSAGetLastError() << endl;
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            //2e etape envoi du  fichier stringifié au client 
            iSendResult = send(ClientSocket, stringifBinaire.c_str(), stringifBinaire.size(), 0);
            if (iSendResult == SOCKET_ERROR)
            {
                cerr << "envoie echoue avec erreur : " << WSAGetLastError() << endl;
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        }
    }
    iResult = shutdown(ClientSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ClientSocket);
    return 0;
}

int __cdecl main(void)
{
    // INSTANCIATION DES VARIABLES
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    //////////////////////////////////
    //INITIALISATION DU CHEMIN DE RECUP DES FICHIERS 
     //CHOIX DU CHEMIN servira pour mettre les copies 
    string extractChemin;
    cout << "Choississez le dossier qui servira de base pour les clients" << endl;
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


    //////////////////////////////////
    // INITIALISATION SOCKET + CREATION SOCKET ECOUTE + BIND DESSUS
    iResult = Initialisation(hints, result, wsaData);
    if (iResult != 0) { return 1; }
    EcoutBind(ListenSocket, result, iResult);
    //gestion erreur
    if (EcoutBindErreur(ListenSocket, result, iResult))
    {
        freeaddrinfo(result);
    }
    else
    {
        return 1;
    }
    
    ////////////////////////////////////////
    // 
    //instanciation du socket d'écoute
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        cerr << "listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // LE SERVEUR EST EN ECOUTE PERMANENTE
    //GestionData gestionData;
    bool serveur = false;
    while (!serveur)
    {
        // acceptation d'un client
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            cerr << "accept failed with error: " << WSAGetLastError() << endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        
        ActiviClient(ClientSocket, iResult, recvbuf, recvbuflen, iSendResult, cheminTraite);
        
        
    }

    return 0;
}
