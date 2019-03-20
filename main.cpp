#include "lodepng.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

unsigned width, height;
unsigned average = 0;
int const disparity = 260;
const int windowSize = 9;
double correlation = 0;
double biggestCorrelation = 0;


unsigned char reduceim0[504][735];
unsigned char reduceim1[504][735];

// MAPAS DE DISPARIDAD //
double IzqDer[504][735];
double DerIzq[504][735];

// MAPA FINAL//
double FinalMap[504][735];

// MAPA FINAL EN UN VECTOR
vector<unsigned char>FinalVector(361816);

void reduceTheMatrix(vector<unsigned char> imagen, unsigned char reducematrix[504][735]);
void ZNCC(unsigned char im0[504][735], unsigned char im1[504][735], double DisMap[504][735]);
double operations(int j, int i, int vector0[windowSize*windowSize], unsigned char im1[504][735], int average0, int desTipica0);
void MapToZero(double DispMap[504][735]);
void CalculateLastMap(double firstMap[504][735], double secondMap[504][735], double lastMap[504][735]);
void SustituirCeros(double lastMap[504][735]);
void MapToVector(double[504][735], vector<unsigned char>& vector);
void encodeOneStep(const char* filename, vector<unsigned char>& image, unsigned width, unsigned height);

int main(int argc, char *argv[]){
/*
	vector<unsigned char>cristinita(40);
	double amaia[5][8];
	double marco = 2.5;
	double natalia = 0;
	for (int alvaro = 0; alvaro < 5; alvaro++){
		for (int ixaka = 0; ixaka < 8; ixaka++){
			amaia[alvaro][ixaka] = marco + natalia;
			cout << amaia[alvaro][ixaka] << " ";
			natalia++;
		}
		cout << "\n";
	}
	int raquel = 0;
	for (int paula = 0; paula < 4; paula++){
		for (int rosa = 0; rosa < 4; rosa++){
			cristinita[raquel] = amaia[paula][rosa];
			raquel++;
		}
	}
	encodeOneStep(".\\images\\out.png", cristinita, 5, 8);
*/
	////////////////////////////////// GETING THE PIXELS FROM THE FIRST IMAGE ///////////////////////////////////////////////
	const char* filename0 = argc > 1 ? argv[1] : "im0.png";
	//load and decode
	std::vector<unsigned char> image0;
	unsigned error0 = lodepng::decode(image0, width, height, filename0);
	//if there's an error, display it
	if (error0) std::cout << "decoder error " << error0 << ": " << lodepng_error_text(error0) << std::endl;
	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...


	///////////////////////////////// GETING THE PIXELS FROM THE SECOND IMAGE ///////////////////////////////////////////////
	const char* filename1 = argc > 1 ? argv[1] : "im1.png";
	//load and decode
	std::vector<unsigned char> image1;
	unsigned error1 = lodepng::decode(image1, width, height, filename1);
	//if there's an error, display it
	if (error1) std::cout << "decoder error " << error1 << ": " << lodepng_error_text(error1) << std::endl;
	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...


	reduceTheMatrix(image0, reduceim0);
	reduceTheMatrix(image1, reduceim1);
	//MapToZero(IzqDer);
	//MapToZero(DerIzq);
	ZNCC(reduceim0, reduceim1, IzqDer);
	ZNCC(reduceim1, reduceim0, DerIzq);
	CalculateLastMap(IzqDer, DerIzq, FinalMap);
	SustituirCeros(FinalMap);
	MapToVector(FinalMap, FinalVector);
	//IMPRIMIR FOTOGRAFIA	
	encodeOneStep(".\\images\\out.png", FinalVector, width, height);
	
}

void reduceTheMatrix(vector<unsigned char> imagen, unsigned char reducematrix[504][735]){

	width = 735; height = 504;
	int R = 0, G = 1, B = 2;
	for (int i = 0; i < height; i++){
		for (int x = 0; x < width; x++){
			reducematrix[i][x] = imagen[R] * 0.2126 + imagen[G] * 0.7152 + imagen[B] * 0.0722;
			R = R + 16;
			G = G + 16;
			B = B + 16;
		}

	}
}

void ZNCC(unsigned char im0[504][735], unsigned char im1[504][735], double DisMap[504][735]){
	int count0 = 0, count1 = 0;  //Contador para la suma de los datos de las ventanas
	int vector0[windowSize * windowSize]; //Guarda los datos de la ventana de la primera imagen
	int vector1[windowSize * windowSize]; //Guarda los datos de la ventana de la segunda imagen
	unsigned average0, average1; //Calcular medias
	double desTipica0 = 0, desTipica1 = 0; //Calcular desviaciones t�picas
	double covarianza = 0; // Calcular covarianza

	for (int i = windowSize / 2; i < height - windowSize + 1/ 2; i++){  //RECORRER IMAGEN
		for (int j = windowSize / 2; j < width - windowSize + 1 / 2; j++){

			int m = 0;
			count0 = 0;
			average = 0;
			for (int y = i - windowSize / 2; y < i + 1 + windowSize / 2; y++) {// RECORRER VENTANA IMAGEN 0
				for (int x = j - windowSize / 2; x < j + 1 + windowSize / 2; x++) {
				
					count0 = count0 + im0[y][x]; //Sumatorio datos ventana imagen 0
					vector0[m] = im0[y][x]; //Guardar datos
					m++;
				}
			}
			average0 = count0 / (pow(windowSize,2)); //Media ventana imagen0
			int aux0 = 0;
			desTipica0 = 0;
			for (m = 0; m < sizeof(vector0) / sizeof(*vector0); m++){//RECORRER OTRA VEZ VENTANA IMAGEN 0, esta vez usamos el vector
				aux0 = vector0[m] - average0;
				desTipica0 = desTipica0 + (pow(aux0, 2));
			}
			desTipica0 = sqrt(desTipica0 / (pow(windowSize, 2)));

			DisMap[i][j] = operations(i, j, vector0, im1, average0, desTipica0);							
	//		cout << DisMap[i][j] << " ";
			}
	}

}

double operations(int hei, int wid, int vector0[windowSize*windowSize], unsigned char im1[504][735], int average0, int desTipica0){

	int vector1[windowSize * windowSize];
	int h = wid;
	int g = hei;
	int countDisparity = 0;
	biggestCorrelation = 0;
	while (g < height - windowSize + 1/ 2 && countDisparity < disparity){
		while (h < width - windowSize + 1/ 2 && countDisparity < disparity){
			int count1 = 0;
			int n = 0;
			for (int win_y = g - windowSize / 2; win_y < g + 1 + windowSize / 2; win_y++){ //CONTADOR DE LOS DATOS VENTANA IMAGEN 1
				for (int win_x = h - windowSize / 2; win_x < h + 1 + windowSize / 2; win_x++){
					count1 = count1 + im1[win_y][win_x];
					vector1[n] = im1[win_y][win_x]; //CREAR VECTOR DE DATOS VENTANA IMAGEN 1
					n++;
				}
			}
			int average1 = 0;
			average1 = count1 / (pow(windowSize, 2));
			int aux1 = 0;
			double desTipica1 = 0;
			for (n = 0; n < sizeof(vector1) / sizeof(*vector1); n++){//RECORRER OTRA VEZ VENTANA IMAGEN 1, esta vez usamos el vector
				aux1 = vector1[n] - average1;
				desTipica1 = desTipica1 + (pow(aux1, 2));
			}
			desTipica1 = sqrt(desTipica1 / (pow(windowSize, 2)));

			double auxcovarianza = 0;
			double covarianza = 0;
			for (n = 0; n < sizeof(vector1) / sizeof(*vector1); n++) { //CALCULAR COVARIANZA CON LOS DATOS GUARDADOS EN LOS VECTORES
				auxcovarianza = (vector0[n] - average0) * (vector1[n] - average1);
				covarianza = covarianza + auxcovarianza;
			}
			covarianza = covarianza / (pow(windowSize, 2));

			correlation = covarianza / (desTipica0 * desTipica1); // CORRELACI�N
			// WE ARE GOING TO COMPARING ONE WINDOW IN THE IMG0 WITH 260 WINDOWS IN THE IMG1 AND WE TAKE THE BIGGEST ONE 
			// WHICH IT IS THE BIGGEST DISPARITY
			
			if (correlation > biggestCorrelation){
				biggestCorrelation = correlation;				
			}
			countDisparity++;
			h++;
		}
		g++;
	}
	return biggestCorrelation;
}

/*void MapToZero(double DispMap[735][504]) {
	for (int i = 0; i < 735; i++) {
		for (int j = 0; j < 504; j++) {
			DispMap[i][j] = 0;
		}
	}
}
*/

void CalculateLastMap(double firstMap[504][735], double secondMap[504][735], double lastMap[504][735]){
	
	int resta = 0;
	for (int i = 4; i < 504 - 4 + 1; i++) {
		for (int j = 4; j < 735 - 4  + 1; j++) {
			resta = firstMap[i][j] - secondMap[i][j];
			resta = abs(resta);
			if (resta > 8){
				lastMap[i][j] = 0;
			}
			else{
				lastMap[i][j] = resta;
			}
		}
	}
}

void SustituirCeros(double lastMap[504][735]){
	int aux = 0;
	for (int i = windowSize / 2; i < height + 1 - windowSize / 2; i++){
		for (int j = windowSize / 2; j < width + 1 - windowSize / 2; j++){
			if (lastMap[i][j] == 0){
				aux = lastMap[i][j + 1] + lastMap[i][j - 1] + lastMap[i + 1][j] + lastMap[i - 1][j] + lastMap[i - 1][j - 1] + lastMap[i - 1][j + 1] + lastMap[i + 1][j - 1] + lastMap[i + 1][j + 1];
				aux = aux / 8;
				lastMap[i][j] = aux;
				//cout << lastMap[j][i] << " ";

			}
		}
	}
}

void MapToVector(double Map[504][735], vector<unsigned char>& vector){
	int n = 0;
	for (int i = windowSize / 2; i < height + 1 - windowSize / 2; i++){
		for (int j = windowSize / 2; j < width + 1 - windowSize / 2; j++){
			vector[n] = Map[i][j];
			n++;
		}
	}
}

void encodeOneStep(const char* filename, vector<unsigned char>& image, unsigned width, unsigned height) {
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}