#include "lodepng.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

unsigned width, height;
unsigned average = 0;
int const disparity = 260;
int windowSize = 9;
double correlation = 0;
unsigned biggestCorrelation = 0;


unsigned char reduceim0[735][504];
unsigned char reduceim1[735][504];


void reduceTheMatrix(vector<unsigned char> imagen, unsigned char reducematrix[735][504]);
void project(unsigned char im0[735][504], unsigned char im1[735][504]);
unsigned lodepng_encode_file(const char* filename, const unsigned char* image, unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth);

int main(int argc, char *argv[]){
	
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
	project(reduceim0, reduceim1);
}


void reduceTheMatrix(vector<unsigned char> imagen, unsigned char reducematrix[735][504]){
	
	width = 735; height = 504;
	int R = 0, G = 1, B = 2;
	int a = 0, b;
	for (int i = 0; i < height ; i++){
		b = 0;
		for (int x = 0; x < width; x++){
			reducematrix[a][b] = imagen[R] * 0.2126 + imagen[G] * 0.7152 + imagen[B] * 0.0722;
			b++;
			R = R + 16;
			G = G + 16;
			B = B + 16;
		}
		a++;
	}
}


void project(unsigned char im0[735][504], unsigned char im1[735][504]){
	int count0 = 0, count1 = 0;  //Contador para la suma de los datos de las ventanas
	int vector0[pow(windowSize)] = null; //Guarda los datos de la ventana de la primera imagen
	int vector1[pow(windowSize)] = null; //Guarda los datos de la ventana de la segunda imagen
	unsigned average0, average1; //Calcular medias
	double desTipica0 = 0, desTipica1 = 0; //Calcular desviaciones típicas
	double covarianza = 0; // Calcular covarianza

	for (unsigned i = windowSize / 2; i < height - windowSize / 2; i++){  //RECORRER IMAGEN
		for (int j = windowSize / 2; j < width - windowSize / 2; j++){
			
			int i = 0;
			for (int win_y = j - windowSize / 2; win_y < windowSize; win_y++) {// RECORRER VENTANA IMAGEN 0
				for (int win_x = i - windowSize / 2; win_x < windowSize; win_x++) {
					
						count0 = count0 + im0[win_y][win_x]; //Sumatorio datos ventana imagen 0
						vector0[i] = im0[win_y][win_x]; //Guardar datos
						i++;
				}
			}
			average0 = count0 / pow(windowSize); //Media ventana imagen0
			
			for(i = 0; i < vector0.size; i++){//RECORRER OTRA VEZ VENTANA IMAGEN 0, esta vez usamos el vector
				desTipica0 = desTipica0 + pow(vector0[i]-average0);
			}
			desTipica0 = sqrt(desTipica0 / pow(windowSize));

			for (int d = 0; d < disparity; d++){ //RECORRER VENTANAS IMAGEN 1

				int j = 0;
				for (int win_y = j - windowSize / 2; win_y < windowSize; win_y++){ //CONTADOR DE LOS DATOS VENTANA IMAGEN 1
					for (int win_x = i - windowSize / 2; win_x < windowSize; win_x++){
						count1 = count1 + im1[win_y][win_x];
						vector[j] = im1[win_y][win_x]; //CREAR VECTOR DE DATOS VENTANA IMAGEN 1
						j++;
					}
				}
				average1 = count1 / pow(windowSize);
				
				for(J = 0; J < vector0.size; J++){//RECORRER OTRA VEZ VENTANA IMAGEN 1, esta vez usamos el vector
					desTipica1 = desTipica1 + pow(vector1[J]-average0);
				}
				desTipica1 = sqrt(desTipica1 / pow(windowSize));

				for (j = 0; j < vector0.size; j++) { //CALCULAR COVARIANZA CON LOS DATOS GUARDADOS EN LOS VECTORES
					covarianza = covarianza + ((vector0[j] - average0) * (vector1[j] - average1));
				}
				covarianza = covarianza / pow(windowSize);

				correlation = covarianza / (desTipica0 * desTipica1); // CORRELACIÓN
				
				// WE ARE GOING TO COMPARING ONE WINDOW IN THE IMG0 WITH 260 WINDOWS IN THE IMG1 AND WE TAKE THE BIGGEST ONE 
				// WHICH IT IS THE BIGGEST DISPARITY
				if (correlation > biggestCorrelation){ 
					biggestCorrelation = correlation;
					int newPixelY = i; //WE SAVE THE COORDINATES OF THE PIXEL WITH THE BIGGEST CORRELATION
					int newPixelX = j;

				}
			}
			
		}
	}
	
}

	


	

