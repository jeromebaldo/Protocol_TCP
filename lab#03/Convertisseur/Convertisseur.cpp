#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace filesystem;

struct ArboFichiers 
{
    string nomFichier;
    string pathFichier;
    string extension;
};

int ChoixFichier()
{
    cout << "---- CHOIX DU FICHIERS ----" << endl;
    cout << "Choississez votre fichie a copier" << endl;
    cout << "Entre 1 et X numeros de fichiers" << endl;
    
    int num;
    cin >> num;
    
    return num-1;
}
void AffichageListFichier(vector<ArboFichiers>listFichier)
{
    
    cout << "---- LISTE DES FICHIERS PRESENTS SUR SERVEUR ----" << endl;
    int rangFichier = 1;
    for (auto it : listFichier)
    {
        cout << rangFichier << "- " << it.nomFichier << endl;
        rangFichier++;
    }
}
void CopieFichier(string nomFichier, string extension, path pathToCopie, vector<unsigned char> fichierBinaire)
{
    string cheminCopie = pathToCopie.string() + "\\" + nomFichier + extension;
    ofstream outfile;
    outfile.open(cheminCopie, ios::out | ios::binary);
    if (outfile) {
        outfile.write(reinterpret_cast<const char*>(fichierBinaire.data()), fichierBinaire.size());
        outfile.close();
    }
    else {
        cerr << "Failed to create file." << endl;
    }
}
vector<unsigned char> Binarisation(string cheminFichier,string nomFichier)
{
    ifstream fileIn(cheminFichier, ios::binary);

    if (!fileIn.is_open())
    {
        cerr << "Erreur: impossible d'ouvrir le fichier " << nomFichier << endl;
    }
    
    cout << "ouverture est bonne" << endl;
    vector<unsigned char>fichierBytes((istreambuf_iterator<char>(fileIn)), istreambuf_iterator<char>());
    
    fileIn.close();
        
    return fichierBytes;
}
vector<ArboFichiers> RecupList(path pathToDir)
{
    vector<ArboFichiers> listFichier;
    string chemin = pathToDir.string();
    if (exists(pathToDir) && is_directory(pathToDir))
    {
        for (const auto& entry : directory_iterator(pathToDir))
        {
            if (entry.is_regular_file())
            {
                string nomFichier = entry.path().filename().string();
                string extension = entry.path().extension().string();
                string cheminFichier = chemin + '\\' + nomFichier;
                ArboFichiers elementFichier = {nomFichier,cheminFichier,extension};
                listFichier.push_back(elementFichier);
            }
        }
    }

    return listFichier;
}
string TransformChemin(string cheminChoisis)
{
    string cheminTraite; 

    for (char c : cheminChoisis)
    {
        if (c == '\\')
        {
            cheminTraite += '\\';
        }

        cheminTraite += c;
    }
    
    return cheminTraite;
}

int main() 
{
    /*
    string cheminTest;
    cout << "donnez  le chemin des fichiers  à copier" << endl;
    cin >> cheminTest;
    string cheminTransforme = TransformChemin(cheminTest);
    cout << "le chemin est : " << cheminTransforme << endl;
    */
    //////RECUPERATION DE LA LISTE DE FICHIERS 
    path pathToDir("C:\\Users\\baldo\\Desktop\\reseaux\\lab#03\\Convertisseur\\x64\\Debug\\fichier_stockes");
    vector<ArboFichiers>listFichier = RecupList(pathToDir);
    //AFFICHAGE ET CHOIX DU FICHIER A COPIER
    AffichageListFichier(listFichier);
    int rang = ChoixFichier();
    /////OPERATION DE COPIE////////
    path pathToCopie("C:\\Users\\baldo\\Desktop\\reseaux\\lab#03\\Convertisseur\\x64\\Debug\\Fichiers_copiés");
    vector<unsigned char> fichierBinarise = Binarisation(listFichier[rang].pathFichier, listFichier[rang].nomFichier);
    CopieFichier(listFichier[rang].nomFichier, listFichier[rang].extension, pathToCopie, fichierBinarise);
    
    return 0;
}



/*
int main()
{



    return 0;
}

/*
void write_bits(std::ofstream& output_file, const char* bits, int size) {
    output_file.write(bits, size);
}

void read_bits(std::ifstream& input_file, char* bits, int size) {
    input_file.read(bits, size);
}

int main() {
    const char* message = "Hello, World";
    int size = strlen(message);
    char* bits = new char[size];
    memcpy(bits, message, size);

    std::ofstream output_file("bits.bin", std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return 1;
    }
    write_bits(output_file, bits, size);
    output_file.close();

    std::ifstream input_file("bits.bin", std::ios::binary);
    if (!input_file.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        return 1;
    }
    read_bits(input_file, bits, size);
    input_file.close();

    std::cout << bits << std::endl;

    delete[] bits;

    return 0;
}
*/