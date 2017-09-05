//Compilar como gcc correlacion.c -o corr
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct WAV_HEADER{
	//The RIFF chunk descriptor
	unsigned char ChunkID[4];//4 bytes
	unsigned int ChunkSize;//4 bytes
	unsigned char Format[4];//4 bytes
	//The fmt sub chunk
	unsigned char SubchunklID[4];//4 bytes
	unsigned int SubchunklSize;//4 bytes
	unsigned short int AudioFormat;//2 bytes
	unsigned short int NumChannels;//2 bytes
	unsigned int SampleRate;//4 bytes
	unsigned int ByteRate;//4 bytes
	unsigned short int BlockAlign;//2 bytes
	unsigned short int BitsPerSample;//2 bytes
	//The data sub chunk
	unsigned char Subchunk2ID[4];//4 bytes
	unsigned int Subchunk2Size;//4 bytes
} wav_hdr;

#define FACTOR_ESCALA 32767.0

double energia(short* x, int n){
	double E = 0, tmp;
	int i;
	for(i=0; i<n; i++){
		tmp = x[i];
		tmp*=tmp;
		E += tmp;
	}
	return E;
}

double correlacion(short* x, short* y, int n){
	double C = 0, den;
	int j;
	for(j=0; j<n; j++){ //Se multiplican las senales
		C += x[j]*(double)y[j];				
	}
	den = sqrt(energia(x, n) * energia(y, n)); //Se normalizan
	//En caso de que la energia de ambas sea 0
	if(den != 0.0) 
		C /= den;
	else
		C = 0.0;
	return C;
}

int main(int argc, char* argv[]){
	if(argc != 5){
		printf("Usar como %s <archivo_s_desconocido> <archivo_s_conocida> <salida> <B|NB>\nB: Muestra en donde se encontro la senal con 1\nNB: Muestra el valor de la correlacion\n", argv[0]);
		return 0;
	}
	FILE* fSenalDesconocida = fopen(argv[1], "rb");
	FILE* fSenalConocida = fopen(argv[2], "rb");
	int i, j;
	wav_hdr wh1, wh2;
	size_t bytesRead = fread(&wh1, 1, sizeof(wh1), fSenalConocida);
	bytesRead = fread(&wh2, 1, sizeof(wh2), fSenalDesconocida);
	if(wh1.AudioFormat==1 && wh1.BitsPerSample == 16 && wh1.NumChannels == 1 && wh1.AudioFormat==wh2.AudioFormat && wh1.BitsPerSample == wh2.BitsPerSample && wh2.NumChannels==wh1.NumChannels){
		FILE* fSalida = fopen(argv[3], "wb");
		fwrite(&wh2, 1, sizeof(wh2), fSalida);
		int numSamplesX = wh1.Subchunk2Size/2;
		int numSamplesY = wh2.Subchunk2Size/2;
		printf("|x| = %d |y| = %d\n", numSamplesX, numSamplesY);

		short*x = (short*) malloc(sizeof(short)*numSamplesX);
		short* y = (short*) malloc(sizeof(short)*numSamplesX);
		short tmp;
		double C, den, umbral;
		for(i=0; i<numSamplesX; i++){ //Se leen igual numero de muestras de X y de Y
			fread(&x[i], 1, 2, fSenalConocida);
			fread(&y[i], 1, 2, fSenalDesconocida);
		}
		umbral = (0.7)*1;
		printf("Umbral = %f\n", umbral);
		for(i=0; i<numSamplesY ; i++){ //Se comienza a calcular la correlacion de corrido o running correlation
			C = correlacion(x, y, numSamplesX);
			//printf("%f\t", C);
			//Si se quiere escribir el valor de la correlacion
			if(argv[4][0] == 'N'){
				C*=FACTOR_ESCALA;
				tmp = (short) C;
			}
			//Si se quiere escribir la decisión tomada basandose en un umbral de si hay senal o no
			else{
				tmp = 0;
				if(C > umbral)
					tmp = 32767;
			}
			fwrite(&tmp, 1, 2, fSalida);
			for(j=0; j<numSamplesX - 1; j++){//Se realiza el corrimiento de las y's
				y[j] = y[j+1];
			}
			if(numSamplesY - i >= numSamplesX){//Si aun quedan muestras por leer de y se leen
				fread(&y[numSamplesX-1], 1, 2, fSenalDesconocida);
			}
			else //Sino se les pone en 0's
				y[numSamplesX-1] = 0;
		}
		fclose(fSalida);
		if(argv[4][0] == 'N'){
			printf("Correlacion lista en %s\n", argv[3]);
		}
		else{
			printf("Decision basada en la correlacion lista en %s\n", argv[3]);
		}
	}
	fclose(fSenalDesconocida);
	fclose(fSenalConocida);
	return 0;
}
