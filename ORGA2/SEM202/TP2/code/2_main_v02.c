#include <stdio.h>
#include <dirent.h>//Para el manejo de directorios.
#include <string.h>//Para la comparacion de strings
#include <time.h>//Tiempo en clocks
#include <pthread.h>//Hilos
#include "2_structs.h" // Archivo con todas las structuras.

//void write_in_csv( char *filename, struct CSV_LINE csvLine );
void write_in_csv( char *filename, char *line );
void writeLog( char *filename, char *line );
//int countFileToCompareInFolder(char *folder);
int getFilesToMerge( char *dir );
// void getFilesMerge( FILE_MERGE *file_merge, char *dir);
void getFilesMerge( FILE_MERGE *file_merge, char *pathIn, char *pathOut );
void generateOutImg( char *image_01, char *image_02, char *mask, char *outImage );
// void generateOutImgFromStruct( char *pathIn, char *pathOut, FILE_MERGE *filesToMerge );
void generateOutImgFromStruct( FILE_MERGE *filesToMerge, int exeType );

//internal functions
void *pthreadFunction( void *filemerge ) {

    FILE_MERGE *merge = (FILE_MERGE*)filemerge;
    //printf( "Files to merge:\n" );
    //printf( "\t%s\n\t%s\n", merge->image_name_01, merge->image_name_02 );
    //printf( "Con la mascara: %s\n", merge->mask_name );

    clock_t start_t, end_t;
    start_t = clock();
    generateOutImgFromStruct( merge, 3 );
    end_t = clock();
    merge->cPthreadClockOutFunction = (double) (end_t - start_t) / CLOCKS_PER_SEC;
}

// argc: Identifica la cantidad de parametros que se pasan.
// argv: Es un array que contiene los parametros que se pasan desde la ejecucion, siendo el argv[0] el "./{objetoC}"
int main(int argc, char** argv){

    int i, cantidadmerge;
    double dPthread_total_time = 0;//Variable utilizada como sumalizador de los tiempos parciales de los hilos.
    char num[10];
    char logLine[100];
    char bmpsPath[] = "/bmps/";
    char reportPath[] = "/reportes/";
    char csvSingleImgFileName[] = "tiempoPorCadaImagen.csv";
    char csvAllImgFileName[] = "tiempoPorProcesoCompleto.csv";
    char logFileName[] = "reporte.log";
    char pathOutImg[] = "/enmascarados/";
    char total_clock[10];
    char csvLine[30];
    clock_t allFiles_t, start_t, end_t, total_t;

    char value01[ strlen( argv[ 1 ] ) + 1 ];
    strcpy( value01, argv[ 1 ] );

    // Path de entrada de archivos y path de salida de archivos.
    char pathIn[ strlen( value01 ) + strlen( bmpsPath ) + 1 ];
    char pathOut[ strlen( value01 ) + strlen( pathOutImg ) + 1 ];

    strcpy( pathIn, value01 );
    strcat( pathIn, bmpsPath );

    strcpy( pathOut, value01 );
    strcat( pathOut, pathOutImg );

    // csv files
    // // Time to process all files.
    int csvPathSize = strlen( value01 ) + strlen( reportPath );
    char csvAllFiles[ csvPathSize + strlen( csvAllImgFileName ) + 1 ];
    strcpy( csvAllFiles, value01 );
    strcat( csvAllFiles, reportPath );
    strcat( csvAllFiles, csvAllImgFileName );

    write_in_csv( csvAllFiles, "METODO, TIEMPO(CLOCK)" );

    // // Time to process each file.
    char csvEachFile[ csvPathSize + strlen( csvSingleImgFileName ) + 1 ];
    strcpy( csvEachFile, value01 );
    strcat( csvEachFile, reportPath );
    strcat( csvEachFile, csvSingleImgFileName );

    write_in_csv( csvEachFile, "ARCHIVO, TIEMPO 1 HILO(CLOCK), TIEMPO VARIOS HILOS(CLOCK), TIEMPO SASM(CLOCK)" );


    // log file
    char logName[ strlen( value01 ) + strlen( reportPath ) + strlen( logFileName ) + 1 ];//El puntero tiene que tener el tamaño necesario para la copia del string.
    strcpy( logName, value01 );
    strcat( logName, reportPath );
    strcat( logName, logFileName );

    // Write in log.
    strcpy( logLine, "Comienza el log\n" );
    writeLog( logName, logLine );

    // Mapeo archivos en array de struct para despues recorrer el struct y procesar cada archivos.
    cantidadmerge = getFilesToMerge( pathIn );
    FILE_MERGE fileMerge[ cantidadmerge ];
    // CSV_EACH_FILE strCsvEachFile[ cantidadmerge ];

    getFilesMerge( fileMerge, pathIn, pathOut );

    // -> Procesamineto en main.
    for( i = 0; i < cantidadmerge; i++ ) {

        //  start_t = clock();
        //  sprintf( num, "%ld", start_t );
        //  writeLog( logName, num );
        //  writeLog( logName, "\n" );

        // Escribimos en el log lo que se va a combinar.
        writeLog( logName, "Se combina imagen " );
        writeLog( logName, fileMerge[i].image_name_01 );
        writeLog( logName, " con la imagen " );
        writeLog( logName, fileMerge[i].image_name_02 );
        writeLog( logName, ", en base a la mascara: " );
        writeLog( logName, fileMerge[i].mask_name );
        writeLog( logName, "\n" );

        start_t = clock();
        // Ejecutamos funcion para generar imagen.
        generateOutImgFromStruct( &fileMerge[i], 1 );
        end_t = clock();

        total_t = end_t - start_t;
        allFiles_t = allFiles_t + total_t;

        fileMerge[ i ].sPthreadClockOutFunction = (double) (total_t) / CLOCKS_PER_SEC;

        // Escribimos en el log el tiempo que tardo en procesar este archivo.
        sprintf( num, "%f", (double) total_t / CLOCKS_PER_SEC );
        writeLog( logName, "Tiempo de clock: " );
        writeLog( logName, num );
        writeLog( logName, "\n" );

    }

    // Impresion del total que tardo en procesar todas las imagenes.
    sprintf( total_clock, "%f", (double) allFiles_t / CLOCKS_PER_SEC );
    strcpy( csvLine, "Main," );
    strcat( csvLine, total_clock );
    write_in_csv( csvAllFiles, csvLine );
    allFiles_t = 0;
    // <- Procesamineto en main.

    // -> Procesamiento con hilos.
    int pthreadIndex;
    int j;
    int pthreadCant = 5;
    pthread_t pthread_id[ pthreadCant ];
    // Recorremos el total de enmascarados que tenemos que hacer en grupos igual a la cantidad de hilos definidos.
    for ( i = 0; i < cantidadmerge; i = i + pthreadCant ) {
        pthreadIndex = 0;
        // Creamos grupos de hilos igual al grupo de emagenes que tomamos para enmascarar.
        while ( pthreadIndex < pthreadCant && i + pthreadIndex < cantidadmerge ) {
            int mergeindex = i + pthreadIndex;
            pthread_create( &pthread_id[ pthreadIndex ], NULL,pthreadFunction , &fileMerge[ mergeindex ] );
            pthreadIndex++;
        }
        // Esperamos que terminen todos los hilos para que si necesitamos seguir enmascarando imagenes tengamos los hilos disponibles.
        for ( j = 0; j < pthreadIndex ; j++ ) {
            pthread_join( pthread_id[ j ], NULL );
        }
    }

    // Sumamos cuanto tardo con cada archivo para sacar el total 
    for ( i = 0; i < cantidadmerge; i++ ) {
        dPthread_total_time = dPthread_total_time + fileMerge[ i ].cPthreadClockOutFunction;
    }

    // Impresion del total que tardo en procesar todas las imagenes con hilos.
    sprintf( total_clock, "%f", dPthread_total_time );
    strcpy( csvLine, "Hilos Out," ) ;
    strcat( csvLine, total_clock );
    write_in_csv( csvAllFiles, csvLine );
    allFiles_t = 0;
    // <- Procesamiento con hilos.

    // -> Procesamiento en NASM
    // Recorremos el array de struct para pasar los parametros a la funcion de asm.
    for ( i = 0; i < cantidadmerge; i++ ) {
        start_t = clock();
        generateOutImgFromStruct( &fileMerge[ i ], 5 );
        end_t = clock();

        total_t = end_t - start_t;
        allFiles_t = allFiles_t + total_t;

        fileMerge[ i ].asmClock = (double) ( total_t ) / CLOCKS_PER_SEC ;
    }

    // Impresion del total que tardo en procesar todas las imagenes con sasm
    sprintf( total_clock, "%f", ( double ) ( allFiles_t ) / CLOCKS_PER_SEC );
    strcpy( csvLine, "SASM," );
    strcat( csvLine, total_clock );
    write_in_csv( csvAllFiles, csvLine );
    allFiles_t = 0;
    // <- Procesamiento en NASM

    // Last print of merges structs(to log and csv times)
    for( i = 0; i < cantidadmerge; i++ ) {
        printf( "ImageId: %s\n", fileMerge[ i ].id );
        printf( "Tardo en procesarlo:\n\tmain: %f,\n\tpthread: %f,\n\tasm: %f\n", fileMerge[ i ].sPthreadClockOutFunction, fileMerge[ i ].cPthreadClockOutFunction, fileMerge[ i ].asmClock );
    }

    return 0;
}