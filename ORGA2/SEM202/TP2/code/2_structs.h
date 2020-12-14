#include <time.h>//Tiempo en clocks

// Deprecated
// // Estructura para cargar datos en el csv.
// struct CSV_LINE {
//     char line[140];
// };

// Deprecated
// // Estructura para comparar tamaños de diferentes archivos.
// typedef struct {
//     char* name;
//     int weight;
//     int height;
// } FILE_INFORMATION ;

// Estructura que contiene toda la información para comenzar con el enmascarado de las imagenes y calcular sus tiempos.
typedef struct {
    char id[10];
    char pathIn[100];
    char pathOut[100];
    char image_name_01[50];
    char image_name_02[50];
    char mask_name[50];
    double sPthreadClockOutFunction;
    double sPthreadClockInFunctino;
    double cPthreadClockOutFunction;
    double cPthreadClockInFunction;
    double asmClockOutFunction;
    double asmClockInFunction;
} FILE_MERGE ;

// Estructura para leer la informacion del header y saber si es un archivo bmp.
typedef struct {
	unsigned short type01;          //2 char
	unsigned int size;              //4 char
	unsigned short reserved01;      //2 char
    unsigned short reserved02;      //2 char
	unsigned int dataOffset;        //4 char
} __attribute((packed)) HEADER ;

// Estructura para leer las caracteristicas del archivo bmp.
typedef struct {
	unsigned int size;              //4 char
	int width, height;              //1 char, 1 char
	unsigned short planes;          //2 char
	unsigned short bitPerPixel;     //2 char
	unsigned char rest[24];         //24 char
} __attribute((packed)) HEADERINFO ;

// Deprecated
// // Estructura para leer de a tres bytes que son los que forman el color del pixel.
// typedef struct {
//     unsigned char r;// red
//     unsigned char g;// green
//     unsigned char b;// blue
// } __attribute((packed)) PIXELCOLOR ;

// Deprecated
// // Estructura utilizada para saber cuanto bytes hay que agregar al largo para que los bytes sean divisibles por 4.
// typedef struct {
//     unsigned char paddingByte;
// } __attribute((packed)) PADDING ;

// Deprecated ( Es reemplazada por la estructura que contiene toda la información del enmascarado de archivos. )
// // Estructura que cargaba los tiempos de ejecucion para cada archivo.
// typedef struct {
//     char filename[30];
//     long withOutThread;
//     long withThread;
//     long sasm;
// } CSV_EACH_FILE ;