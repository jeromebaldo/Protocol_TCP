
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

string RecupAdresseIp();
vector<string> AuthenUtil();

int __cdecl main(int argc, char** argv)
{
	////////////////////////////////////////
	//PARTIE INSTANCIATION ET INITIALISATION
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	//initialisation du socket du client 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
		return 1;
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	////////////////////////////////////////////////////////////
	

	//mettre tableau string quand cela marche pour remettre après
	//mettre variable du chemin la faire valider quand c'est bon
	
	////////////////////////////////////////////////////////////////
	//1er ETAPE : se connecter - tant que pas de connection retenter
	///////////////////////////////////////////////////////////////
	string identifiant;
	string motDePasse;
	bool connection = false;
	bool deconnection = false;
	while (!connection)
	{
		//initialisation de la liaison

		//const char* IP = "172.20.224.45";
		const char* IP = "172.30.0.216";
		//liaison avec le serveur 
		// Resolve the server address and port
		iResult = getaddrinfo(IP, DEFAULT_PORT, &hints, &result);
		//iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
		if (iResult != 0) 
		{
			cout << "getaddrinfo failed with error: " << iResult << endl;
			WSACleanup();
			return 1;
		}

		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) 
			{
				cout << "socket failed with error: " << WSAGetLastError() << endl;
				WSACleanup();
				return 1;
			}
			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) 
			{
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);
		//gestion erreur serveur 
		if (ConnectSocket == INVALID_SOCKET) 
		{
			cout << "Impossible de se connecter au serveur!" << endl;
			WSACleanup();
			return 1;
		}

		// la connection est établie on peut à l'identification de l'utilisateur 
		cout << "CONNECTION SERVEUR ETABLIE" << endl;
		//puis envoi de l'ID + mot de passe
		vector<string>authenf = AuthenUtil();
		//transformer 
		//calibrer les outils de binarisation pour le projet des deux côtés
		// penser mettre les codes d'erreurs 
		//faire un envoi du libellé puis envoyer la chaine
		
		//faire l'envoi 
		//si réception ok passer à la boucle suivante
		//remplir id et mot de passe 
	}
	//2e etape : choisir l'adresse pour copier les fichiers (boucle while)
	//validation du chemin 
	//3e etape : choisir un fichier à copier ou soit quitter l'application (boucle while)
	while (!deconnection)
	{
		//garder la liaison et faire les demandes 
		deconnection = true;
	}

	//4e etape : shutdown et close socket + fermer application client 
	return 0;
}

string RecupAdresseIp()
{
	string chemin;

	cout << "---- SELECTION DU DOSSIER ----" << endl;
	cout << "Veuillez donner le chemin complet du dossier de reception" << endl;
	//mettre path //mettre librairie 
	cin >> chemin;
	return chemin;
}

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