/*Claudia Bustamante 
Laboratorio 3 - Sistemas Operativos 2014 
Universidad de Santiago*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <ctype.h>
#include <fstream> 
#include <sstream>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "md5.h"
         
using namespace std;
/*Claudia Bustamante */
typedef struct{ //estructura del usuario con su clave
	string user;
	string pass;
}UsuarioPass;

typedef struct { //tupla para la caché del pablo
	string encryptMD5;
	string decryptMD5;
	int frequency;
}tuple;

UsuarioPass usuarioActual;
vector<tuple> tupla;
int TamCache;
string resultadoActual;
bool encontroActual;
pthread_mutex_t barrera = PTHREAD_MUTEX_INITIALIZER; 
/*Claudia Bustamante */
vector<UsuarioPass> lecturaDB(string nombreDB){
	vector<UsuarioPass> DB;
	ifstream file;
	file.open(nombreDB.c_str());
	string word;
	UsuarioPass auxiliar;
	int i=0;
	while(!file.eof()){
		file >> word;
		if(word.compare("INSERT")==0){
			file >> word;
			if(word.compare("INTO")==0){
				getline(file,word);
				char caracter;
				string nombre;
				string contrasena;
				caracter=file.get();
				while(caracter!= ';'){
					caracter=file.get();
					if(caracter== '\''){
						for(int u=0;u<3;u++){
							while(caracter!='\''){
								if(caracter!=',' && caracter!=' '){
									nombre.push_back(caracter);
								}
								caracter=file.get();
							}
							if(caracter=='\'' && u==0){
								caracter=file.get();
								while(caracter!='\''){
									nombre.push_back(caracter);
									caracter=file.get();
								}
							}
							else if(caracter=='\'' && u==1){
								caracter=file.get();
								while(caracter!='\''){
									caracter=file.get();
								}
							}
							else if(caracter=='\'' && u==2){
								caracter=file.get();
								while(caracter!='\'' && u==2){
									contrasena.push_back(caracter);
									caracter=file.get();
								}
							}
							caracter=file.get();
						}
						auxiliar.user=nombre;
						auxiliar.pass=contrasena;
						nombre.clear();
						contrasena.clear();
						DB.push_back(auxiliar);
					}
				}
			}
		}
	}
	return DB;
}
/*Claudia Bustamante */
vector<string> lecturaDiccionario(string NombreDicionario){
	ifstream dicc;
	dicc.open(NombreDicionario.c_str());
	string palabra;
	vector<string> vectorDiccionario;
	while(!dicc.eof()){
		dicc >> palabra;
		vectorDiccionario.push_back(palabra);
	}
	return vectorDiccionario;
}

vector <vector<string> > divisionDiccionario(int numeroHebras, vector<string> diccionario){
	
	vector<vector<string> > minidiccionarios(numeroHebras);
	int indicador;
	for(int i=0;i<diccionario.size();i++){
		indicador=i%numeroHebras;
		minidiccionarios[indicador].push_back(diccionario[i]);
	}
	return minidiccionarios;
}

string BuscarCache(string encryptMD5){
/*Claudia Bustamante */
	for(int i=0;i<tupla.size();i++){
		if(tupla[i].encryptMD5.compare(encryptMD5)==0){
			tupla[i].frequency++;
		return tupla[i].decryptMD5;
		}
	}
	return "1";
}

void insert(string encryptMD5, string decryptMD5){

	if(tupla.size()<TamCache){
		tupla.push_back(tuple());
		tupla[tupla.size()-1].encryptMD5 = encryptMD5;
		tupla[tupla.size()-1].decryptMD5 = decryptMD5;
		tupla[tupla.size()-1].frequency = 1;
		return;
	}
	int lowerFrequencyIndex = 0;
	for(int i=0;i<tupla.size();i++){
		if(tupla[i].frequency < tupla[lowerFrequencyIndex].frequency){
			lowerFrequencyIndex == i;
		}
	}
	tupla[lowerFrequencyIndex].encryptMD5 = encryptMD5;
	tupla[lowerFrequencyIndex].decryptMD5 = decryptMD5;
	tupla[lowerFrequencyIndex].frequency = 1;
	return;
}
/*Claudia Bustamante */
void * funcionHebra(void* parametro){
	vector<string> diccionario = *(vector<string>*)parametro;
	//cout << "hola, soy una hebra y contengo: " << endl; 

	for (int i = 0; i < diccionario.size(); ++i){
		pthread_mutex_lock(&barrera);
		if (encontroActual == true){
			pthread_mutex_unlock(&barrera);
			break;
		}
		pthread_mutex_unlock(&barrera);
/*Claudia Bustamante */
		string codificado = md5(diccionario[i]);
		if(codificado == usuarioActual.pass){
			pthread_mutex_lock(&barrera);
			resultadoActual = diccionario[i];
			cout << "User: " << usuarioActual.user << " | Pass: " << resultadoActual << endl;
			encontroActual = true;
			pthread_mutex_unlock(&barrera);
			break;
		}
	}
}


/*Claudia Bustamante */
int main(int argc, char **argv){

	int NumeroHebras;
	string NombreDicionario;
	char * NombreDatabase = NULL;
	vector<string> vectorDiccionario;
	vector<UsuarioPass> vectorUsers;
	vector<vector<string> > diccionariosDivididos;

	string rVal = ""; //  Nombre Database 
	int rflag=0;
	string cVal = ""; //  Tamaño Caché
	int cflag=0;
	string dVal = ""; //  Diccionario
	int dflag=0;
	string hVal = ""; //  Número Hebras
	int hflag=0;
	int cont=0;
	int c;
	opterr =0;
	while((c = getopt(argc,argv,"r:c:d:h:")) != -1)
		switch(c){
			case 'r':
				rflag = 1;
				cont++;
				NombreDatabase=optarg;
				break;
			case 'c':
				cflag = 1;
				cont++;
				TamCache=atoi(optarg);
				break;
			case 'd':
				dflag = 1;
				cont++;
				NombreDicionario=optarg;
				break;
			case 'h':
				hflag = 1;
				cont++;
				NumeroHebras=atoi(optarg);
				break;
			case '?':
				if (optopt=='r'||optopt == 'c'||optopt == 'd'||optopt == 'h'){
					fprintf (stderr, "Opcion -%c requiere un argumento.\n", optopt);
					if (isprint (optopt))
						fprintf (stderr, "Opcion incorrecta `-%c'.\n", optopt);
		        }
				return 1;  
			default:
				abort();
		}	
		/*Claudia Bustamante */
	if (rflag && cflag && dflag && hflag && cont == 4){
		cout << "Diccionario es: "<<NombreDicionario<<endl;
		cout << "Numero Hebras es: "<<NumeroHebras<<endl;
		cout << "Nombre DB es: "<<NombreDatabase<<endl;
		cout << "Tamaño Cache es: "<<TamCache<<endl;
		
		vectorUsers=lecturaDB(NombreDatabase);
		vectorDiccionario=lecturaDiccionario(NombreDicionario);
		diccionariosDivididos=divisionDiccionario(NumeroHebras, vectorDiccionario);
		
		vector<pthread_t> vectorHebras(NumeroHebras);

		for (int j = 0; j <vectorUsers.size(); j++)
		{
			usuarioActual = vectorUsers[j];
			encontroActual = false;

			string resultadoCache = BuscarCache(usuarioActual.pass);
			if(resultadoCache != "1"){
				cout << "User: " << usuarioActual.user << " | Pass: " << resultadoCache << endl;
			
			}else{
			
				for (int i = 0; i < NumeroHebras; i++){
					pthread_create(&vectorHebras[i], NULL, funcionHebra, (void*)&diccionariosDivididos[i]);
					
				}
				for (int i = 0; i < NumeroHebras; i++)
				{
					pthread_join(vectorHebras[i], NULL);
				}

				if(encontroActual){
					insert(usuarioActual.pass,resultadoActual);

				}

			}
		}
	}
}


