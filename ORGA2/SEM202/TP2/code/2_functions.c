#include <stdio.h>
#include <stdbool.h> //Objeto para trabajar con booleans.
#include <dirent.h>//Para el manejo de directorios.
#include <string.h>//Para la comparacion de strings
#include "2_structs.h" //Archivo donde se encuentran todas las structs.

extern void hola( char *img01Buffer, char *img02Buffer, char *maskbuffer, char *outPutFile, int columns, int row, int padding );
extern void enmascarar_sasm( char *buff_01, char *buff_02, char *buff_mask, char *buff_out, int cantidad );

// char bmpFolderName[] = "bmps";
char mascara[] = "mascara";

// Print HEADER.
void printHeader( char *prefix, HEADER header ){
    printf( "%sTamaño del segmento: %d\n", prefix, sizeof( HEADER ) );
    printf( "%stype: %hu,\n", prefix, header.type01 );
    printf( "%ssize: %d,\n", prefix, header.size );
    printf( "%sreserved: %hu,\n", prefix, header.reserved01 );
    printf( "%sreserved: %hu,\n", prefix, header.reserved02 );
    printf( "%sdataOffset: %d,\n", prefix, header.dataOffset );
}

// Print HEADERINFO.
void printHeaderInfo( char *prefix, HEADERINFO headerInfo){
    printf( "%sSize: %d\n", prefix, headerInfo.size );
    printf( "%sWidth: %d\n", prefix, headerInfo.width );
    printf( "%sHeight: %d\n", prefix, headerInfo.height );
    printf( "%sPlanes: %hu\n", prefix, headerInfo.planes );
    printf( "%sBitPerPixel: %hu\n", prefix, headerInfo.bitPerPixel );
    printf( "%sRest: 24 chars\n", prefix );
}

// Escribir datos en un csv.
void write_in_csv( char *filename, char *line ){
    FILE *f;

    char fName[ strlen( filename ) + 5 ];

    strcpy( fName, filename );

    f = fopen( fName, "a" );

    fprintf( f, "%s", line );
    fprintf( f, "\n" );

    fclose( f );
}

// Escribir en archivo de log.
void writeLog( char *filename, char *line ){
    FILE *f;

    char fName[ strlen( filename ) + 1 ];
    strcpy( fName, filename );

    f = fopen( fName, "a" );

    fprintf( f, "%s", line );

    fclose( f );
}

// Filtra por los archivos que se agreguen.
bool isImgFile( struct dirent *file ) {

    if ( strcmp( file -> d_name, "." ) == 0 ) {
        return false;
    }

    if ( strcmp( file -> d_name, ".." ) == 0 ) {
        return false;
    }

    return true;
}

// Deprecated
// // Cantidad de archivos a comparar 
// int countFileToCompareInFolder( char *folder ) {

//     int folderCount = 0;

//     DIR* dir;
//     struct dirent* strctDir;
//     folder = strcat( folder, "/" );
//     folder = strcat( folder, bmpFolderName );

//     dir = opendir( folder );

//     while( ( strctDir = readdir( dir ) ) ) {
//         if ( isImgFile( strctDir ) ) {
//             folderCount++;
//         }
//     }

//     closedir( dir );

//     return folderCount;
// }

// Gereramos un array con todas las posibles conbinaciones de archivos posibles.
int getFilesToMerge( char *dir ) {

    // Tengo que contar la cantidad de archivos combinables.
    int cantidadMerges = 0;
    DIR* folder01;
    DIR* folder02;
    DIR* folder03;
    struct dirent* file01;
    struct dirent* file02;
    struct dirent* file03;
    
    char directory[ strlen( dir ) ];

    strcpy( directory, dir );

    folder01 = opendir( directory );

    HEADER header01, header02, maskHeader;
    HEADERINFO headerInfo01, headerInfo02, maskHeaderinfo;

    while ( ( file01 = readdir( folder01 ) ) != NULL ){
        if ( isImgFile( file01 ) && ! strstr( file01->d_name, mascara ) ) {
            //printf( "File: %s\n", file01 -> d_name );
            char fName01[ strlen( directory ) + 1 + strlen( file01 -> d_name ) + 1 ];
            strcpy( fName01, directory );
            strcat( fName01, "/" );
            strcat( fName01, file01 -> d_name );

            FILE * f01 = fopen( fName01, "rb" );

            fseek( f01, 0x00, SEEK_SET );
            fread( &header01, sizeof( HEADER ), 1, f01 );

            //printf( "File: %s\n", file01 -> d_name );
            //printHeader( "\t", header01 );
            
            fread( &headerInfo01, sizeof( HEADER ), 1, f01 );
            //printHeaderInfo( "\t\t", headerInfo01 );

            folder02 = opendir( directory );
            while ( ( file02 = readdir( folder02 ) ) != NULL ) {
                // Comparo que no sea la misma imagen y que no sea una mascara.
                if ( isImgFile( file02 ) 
                    && ! strstr ( file02 -> d_name, mascara )
                    && strcmp( file01 -> d_name, file02 -> d_name ) != 0 ) {
                    //printf("\tFile: %s\n", file02 -> d_name );

                    char img02Name[ strlen( directory ) + 1 + strlen( file02 -> d_name ) + 1 ];
                    strcpy( img02Name, directory );
                    strcat( img02Name, "/" );
                    strcat( img02Name, file02 -> d_name );

                    FILE* fImg02 = fopen( img02Name, "rb" );

                    fseek( fImg02, 0x00, SEEK_SET );
                    fread( &header02, sizeof( HEADER ), 1, fImg02 );
                    fread( &headerInfo02, sizeof( HEADER ), 1, fImg02 );
                    
                    // Imagenes del mismos tamaño
                    if( headerInfo01.width == headerInfo02.width
                            && headerInfo01.height == headerInfo02.height ){
                        
                        folder03 = opendir( directory );
                        while ( ( file03 = readdir ( folder03 ) ) != NULL ) {
                            // Solo si es mascara.
                            if( strstr( file03 -> d_name, mascara ) ){
                                
                                char maskName[ strlen( directory ) + 1 + strlen( file03 -> d_name ) + 1 ];
                                strcpy( maskName, directory );
                                strcat( maskName, "/" );
                                strcat( maskName, file03 -> d_name );

                                FILE* maskFile = fopen( maskName, "rb" );

                                fseek( maskFile, 0x00, SEEK_SET );
                                fread( &maskHeader, sizeof( HEADER ), 1, maskFile );
                                fread( &maskHeaderinfo, sizeof( HEADERINFO ), 1, maskFile );

                                if ( headerInfo02.width == maskHeaderinfo.width
                                        && headerInfo02.height == maskHeaderinfo.height ) {
                                    

                                    // printf( "Combinación de %s & %s, con la mascara:%s\n",file01 -> d_name, file02 -> d_name, file03 -> d_name );

                                    cantidadMerges++;
                                }
                                fclose( maskFile );
                            }
                        }
                        closedir( folder03 );
                    }
                    fclose( fImg02 );
                }
            }
            fclose( f01 );
            closedir( folder02 );
        }
    }
    closedir( folder01 );

    // printf( "Cantidad de combinaciones: %d\n", cantidadMerges );

    return cantidadMerges;
}

// Cargamos el array de estructuras con la información de que archivos hay que enmascarar.
void getFilesMerge( FILE_MERGE *file_merge, char *pathIn, char *pathOut ) {
    // Tengo que contar la cantidad de archivos combinables.
    char charImgId[9];
    int intImgId = 1;
    int fileMerge_pointer = 0;
    DIR* folder01;
    DIR* folder02;
    DIR* folder03;
    struct dirent* file01;
    struct dirent* file02;
    struct dirent* file03;
    
    char directory[ strlen( pathIn ) ];

    strcpy( directory, pathIn );

    folder01 = opendir( directory );

    HEADER header01, header02, maskHeader;
    HEADERINFO headerInfo01, headerInfo02, maskHeaderinfo;

    while ( ( file01 = readdir( folder01 ) ) != NULL ){
        if ( isImgFile( file01 ) && ! strstr( file01->d_name, mascara ) ) {
            // printf( "File: %s\n", file01 -> d_name );
            char fName01[ strlen( directory ) + 1 + strlen( file01 -> d_name ) + 1 ];
            strcpy( fName01, directory );
            strcat( fName01, "/" );
            strcat( fName01, file01 -> d_name );

            FILE * f01 = fopen( fName01, "rb" );

            fseek( f01, 0x00, SEEK_SET );
            fread( &header01, sizeof( HEADER ), 1, f01 );

            // printf( "File: %s\n", file01 -> d_name );
            // printHeader( "\t", header01 );
            
            fread( &headerInfo01, sizeof( HEADER ), 1, f01 );
            // printHeaderInfo( "\t\t", headerInfo01 );

            folder02 = opendir( directory );
            while ( ( file02 = readdir( folder02 ) ) != NULL ) {
                if ( isImgFile( file02 ) // Comparo que no sea la misma imagen y que no sea una mascara.
                    && ! strstr ( file02 -> d_name, mascara )
                    && strcmp( file01 -> d_name, file02 -> d_name ) != 0 ) {
                    // printf("\tFile: %s\n", file02 -> d_name );

                    char img02Name[ strlen( directory ) + 1 + strlen( file02 -> d_name ) + 1 ];
                    strcpy( img02Name, directory );
                    strcat( img02Name, "/" );
                    strcat( img02Name, file02 -> d_name );

                    FILE* fImg02 = fopen( img02Name, "rb" );

                    fseek( fImg02, 0x00, SEEK_SET );
                    fread( &header02, sizeof( HEADER ), 1, fImg02 );
                    fread( &headerInfo02, sizeof( HEADER ), 1, fImg02 );
                    
                    // Imagenes del mismos tamaño
                    if( headerInfo01.width == headerInfo02.width
                            && headerInfo01.height == headerInfo02.height ){
                        
                        folder03 = opendir( directory );
                        while ( ( file03 = readdir ( folder03 ) ) != NULL ) {
                            // Solo si es mascara.
                            if( strstr( file03 -> d_name, mascara ) ){
                                
                                char maskName[ strlen( directory ) + 1 + strlen( file03 -> d_name ) + 1 ];
                                strcpy( maskName, directory );
                                strcat( maskName, "/" );
                                strcat( maskName, file03 -> d_name );

                                FILE* maskFile = fopen( maskName, "rb" );

                                fseek( maskFile, 0x00, SEEK_SET );
                                fread( &maskHeader, sizeof( HEADER ), 1, maskFile );
                                fread( &maskHeaderinfo, sizeof( HEADERINFO ), 1, maskFile );

                                if ( headerInfo02.width == maskHeaderinfo.width
                                        && headerInfo02.height == maskHeaderinfo.height ) {

                                    sprintf( charImgId, "img_%04d", intImgId++ );
                                    strcpy( file_merge[ fileMerge_pointer ].id, charImgId );
                                    strcpy( file_merge[ fileMerge_pointer ].pathIn, directory );
                                    strcpy( file_merge[ fileMerge_pointer ].pathOut, pathOut );
                                    strcpy( file_merge[ fileMerge_pointer ].image_name_01, file01 -> d_name );
                                    strcpy( file_merge[ fileMerge_pointer ].image_name_02, file02 -> d_name );
                                    strcpy( file_merge[ fileMerge_pointer ].mask_name, file03 -> d_name );
                                    fileMerge_pointer++;
                                }
                                fclose( maskFile );
                            }
                        }
                        closedir( folder03 );
                    }
                    fclose( fImg02 );
                }
            }
            fclose( f01 );
            closedir( folder02 );
        }
    }
    closedir( folder01 );
}

// Deprecated
// // Verificamos si el pixel es blanco o negro
// bool isBlackPixel( PIXELCOLOR pColor ){

//     if ( pColor.r <= 0xf0 || pColor.g <= 0xf0 || pColor.b <= 0xf0 ){
//         return true;
//     }

//     return false;
// }
// // Char Version
// bool isBlackPixel02(unsigned char r, unsigned char g, unsigned char b ) {

//     if( r <= 0xf0 || g <= 0xf0 || b <= 0xf0 ){
//         return true;
//     }

//     return false;
// }

// Deprecated version
// // Pasamos los punteros de los pixeles para cambiar sus valores.
// void copyPixelColor( PIXELCOLOR fromPixelColor, PIXELCOLOR *toPixelColor ) {

//     toPixelColor->r = fromPixelColor.r;
//     toPixelColor->g = fromPixelColor.g;
//     toPixelColor->b = fromPixelColor.b;

// }

// Metotodo para comparar de a 3 bytes que son los valores de los colores.
// Deprecated
// void mainPthread( char *img01Buffer, char *img02Buffer, char *maskBuffer, char *outBuffer, int columnPixels, int rowPixels, int maskPadding ) {

//     int i, j, byte_base;

//     for( i = 0; i < rowPixels; i++ ) {
//         for( j = 0; j < columnPixels; j++ ) {
//             byte_base = ( ( columnPixels * 3 + maskPadding ) * i ) + ( j * 3 );
//             //printf( "%d - j: %d, col:%d\n", byte_base, j, columnPixels );
//             if( isBlackPixel02( maskBuffer[ byte_base + 0 ], maskBuffer[ byte_base + 1 ], maskBuffer[ byte_base + 2 ] ) ) {
//                 outBuffer[ byte_base + 0 ] = img01Buffer[ byte_base + 0 ];
//                 outBuffer[ byte_base + 1 ] = img01Buffer[ byte_base + 1 ];
//                 outBuffer[ byte_base + 2 ] = img01Buffer[ byte_base + 2 ];
//             } else {
//                 outBuffer[ byte_base + 0 ] = img02Buffer[ byte_base + 0 ];
//                 outBuffer[ byte_base + 1 ] = img02Buffer[ byte_base + 1 ];
//                 outBuffer[ byte_base + 2 ] = img02Buffer[ byte_base + 2 ];
//             }
//         }
//     }
// }
// Version_02
// Ahora preguntamos byte por byte en vez de cada tres bytes, ya que la mascara en sus tres bytes de van a ser iguales ( Esto pasa porque forma pixeles negros y blancos unicamente. )
void mainPthread( char *img01Buffer, char *img02Buffer, unsigned char *maskBuffer, char *outBuffer, int cantidad ) {

    int i, j, byte_base;

    for( i = 0; i < cantidad; i++ ) {
        // Negro
        if( maskBuffer[ i ] <= 0xf0 ) {
            outBuffer[ i ] = img01Buffer[ i ];
        } else {
            outBuffer[ i ] = img02Buffer[ i ];
        }
    }
}


// // Paso los archivos de las imagenes para ir comparandolos pixel por pixel.
// // Deprecated Version
// void generateOutImg( char *image_01, char *image_02, char *mask, char *outImage ) {

//     int maskPadding, rowPixels, columnPixels, row, column;
//     HEADER headerImg01, headerImg02, headerMask, headerOut;
//     HEADERINFO hinfoImg01, hinfoImg02, hinfoMask, hinfoOut;
//     PIXELCOLOR pColorImg01, pColorImg02, pColorMask, pColorOut;
//     PADDING paddingImg01, paddingImg02, paddingMask;

//     // Leo los archivos que me pasan en byte y comienzo a escribir el archivo out tambien en bytes.
//     FILE* fImg01 = fopen( image_01, "rb" );
//     FILE* fImg02 = fopen( image_02, "rb" );
//     FILE* fMask = fopen( mask, "rb" );
//     FILE* fOut = fopen( outImage, "wb" );

//     // Me posiciono al comienzo de todos los archivos.
//     fseek( fImg01, 0x00, SEEK_SET );
//     fseek( fImg02, 0x00, SEEK_SET );
//     fseek( fMask, 0x00, SEEK_SET );
//     fseek( fOut, 0x00, SEEK_SET );

//     // Leemos la cabecera de los archivos pero escribimos la cabecera del archivo out.
//     fread( &headerImg01 , sizeof(HEADER), 1, fImg01 );
//     fread( &headerImg02 , sizeof(HEADER), 1, fImg02 );
//     fread( &headerMask , sizeof(HEADER), 1, fMask );
//     fwrite( &headerImg01 , sizeof(HEADER), 1, fOut );

//     // Leemos las información del archivo y escribimos la información del archivo out.
//     fread( &hinfoImg01 , sizeof(HEADERINFO), 1, fImg01 );
//     fread( &hinfoImg02 , sizeof(HEADERINFO), 1, fImg02 );
//     fread( &hinfoMask , sizeof(HEADERINFO), 1, fMask );
//     fwrite( &hinfoImg01, sizeof(HEADERINFO), 1, fOut );

//     // Si el ancho no es divisible por 4 entonces le tenemos que agregar bits hasta que sea divisible.
//     maskPadding = ( ( int ) hinfoImg01.width ) % 4;
//     if ( maskPadding != 0 ) {
//         maskPadding = 4 - maskPadding;
//     }

//     rowPixels = ( int ) hinfoImg01.height;
// 	columnPixels = ( int ) hinfoImg01.width;

//     // Recorremos cada unas de las filas que contiene la imagen.
//     for ( row = 0; row < rowPixels; row++ ) {
//         // Recorremos cada una de las columnas.
//         for ( column = 0; column < columnPixels; column++ ) {
//             // Leemos cada uno de los pixeles de las imanges y de la mascara para avanzar simultaneamente.
//             fread( &pColorImg01 ,sizeof(PIXELCOLOR), 1, fImg01 );
//             fread( &pColorImg02 ,sizeof(PIXELCOLOR), 1, fImg02 );
//             fread( &pColorMask ,sizeof(PIXELCOLOR), 1, fMask );

//             // Preguntamos si el pixel de la mascara es negro o blanco para saber que pixel escribimos sobre la imagen de salida.
//             if ( isBlackPixel( pColorMask ) ) {
//                 pColorOut.r = pColorImg01.r;
//                 pColorOut.g = pColorImg01.g;
//                 pColorOut.b = pColorImg01.b;
//             }
//             else {
//                 pColorOut.r = pColorImg02.r;
//                 pColorOut.g = pColorImg02.g;
//                 pColorOut.b = pColorImg02.b;
//             }

//             // Escribimos el color del byte en el archivo out.
//             fwrite( &pColorOut, sizeof(PIXELCOLOR), 1, fOut );
            
//         }

//         // Si el padding da diferente de 0 entonces lo leemos y escribimos en el archivos de salida.
//         if ( maskPadding != 0 ) {
//             fread( &paddingImg01, sizeof(PADDING), maskPadding, fImg01 );
//             fread( &paddingImg02, sizeof(PADDING), maskPadding, fImg02 );
//             fread( &paddingMask, sizeof(PADDING), maskPadding, fMask );
//             fwrite( &paddingImg01, sizeof(PADDING), maskPadding, fOut );
//         }
//     }

//     fclose( fImg01 );
//     fclose( fImg02 );
//     fclose( fMask );
//     fclose( fOut );
// }
// // Version_02
// // La ruta en la que trabajar y los nombres de los archivos los cuales hay que enmascarar.
// // void generateOutImg( char *image_01, char *image_02, char *mask, char *outImage ) {
// // void generateOutImgFromStruct( char *pathIn, char *pathOut, FILE_MERGE *filesToMerge ) {
void generateOutImgFromStruct( FILE_MERGE *filesToMerge, int executionType ) {

    int intPathin = strlen( filesToMerge->pathIn );
    int intPathOut = strlen( filesToMerge->pathOut );

    char img01FullPath[ intPathin + strlen( filesToMerge->image_name_01 ) + 1 ];
    char img02FullPath[ intPathin + strlen( filesToMerge->image_name_02 ) + 1 ];
    char maskFullPath[ intPathin + strlen( filesToMerge->mask_name ) + 1 ];
    char outFullPath[ intPathOut + strlen( filesToMerge->id ) + 6 ];
    char type[2];
    int maskPadding, rowPixels, columnPixels, row, column;
    HEADER headerImg01, headerImg02, headerMask, headerOut;
    HEADERINFO hinfoImg01, hinfoImg02, hinfoMask, hinfoOut;
    // PIXELCOLOR pColorImg01, pColorImg02, pColorMask, pColorOut;
    // PADDING paddingImg01, paddingImg02, paddingMask;

    // Armo las rutas absolutas para las imagenes.
    strcpy( img01FullPath, filesToMerge->pathIn );
    strcat( img01FullPath, filesToMerge->image_name_01);

    strcpy( img02FullPath, filesToMerge->pathIn );
    strcat( img02FullPath, filesToMerge->image_name_02);    
    
    strcpy( maskFullPath, filesToMerge->pathIn );
    strcat( maskFullPath, filesToMerge->mask_name);
    
    sprintf( type, "%d", executionType );
    strcpy( outFullPath, filesToMerge->pathOut );
    strcat( outFullPath, type );
    strcat( outFullPath, filesToMerge->id );
    strcat( outFullPath, ".bmp" );

    // Leo los archivos que me pasan en byte y comienzo a escribir el archivo out tambien en bytes.
    FILE* fImg01 = fopen( img01FullPath, "rb" );
    FILE* fImg02 = fopen( img02FullPath, "rb" );
    FILE* fMask = fopen( maskFullPath, "rb" );
    FILE* fOut = fopen( outFullPath, "wb" );

    // Me posiciono al comienzo de todos los archivos.
    fseek( fImg01, 0x00, SEEK_SET );
    fseek( fImg02, 0x00, SEEK_SET );
    fseek( fMask, 0x00, SEEK_SET );
    fseek( fOut, 0x00, SEEK_SET );

    // Leemos la cabecera de los archivos pero escribimos la cabecera del archivo out.
    fread( &headerImg01 , sizeof(HEADER), 1, fImg01 );
    fread( &headerImg02 , sizeof(HEADER), 1, fImg02 );
    fread( &headerMask , sizeof(HEADER), 1, fMask );
    fwrite( &headerImg01 , sizeof(HEADER), 1, fOut );

    // Leemos las información del archivo y escribimos la información del archivo out.
    fread( &hinfoImg01 , sizeof(HEADERINFO), 1, fImg01 );
    fread( &hinfoImg02 , sizeof(HEADERINFO), 1, fImg02 );
    fread( &hinfoMask , sizeof(HEADERINFO), 1, fMask );
    fwrite( &hinfoImg01, sizeof(HEADERINFO), 1, fOut );

    // Si el ancho no es divisible por 4 entonces le tenemos que agregar bits hasta que sea divisible.
    maskPadding = ( ( int ) hinfoImg01.width ) % 4;
    if ( maskPadding != 0 ) {
        maskPadding = 4 - maskPadding;
    }

    rowPixels = ( int ) hinfoImg01.height;
	columnPixels = ( int ) hinfoImg01.width;

    int matrizImgSize = rowPixels * ( columnPixels * 3 + maskPadding );

    // printf( "%d\n", matrizImgSize );

    char img01Buffer[ matrizImgSize ];
    char img02Buffer[ matrizImgSize ];
    char maskBuffer[ matrizImgSize ];
    char outBuffer[ matrizImgSize ];

    fread( &img01Buffer, sizeof( img01Buffer ), 1, fImg01 );
    fread( &img02Buffer, sizeof( img02Buffer ), 1, fImg02 );
    fread( &maskBuffer, sizeof( maskBuffer ), 1, fMask );

    // strcpy( img01Buffer, "B" );
    
    int i_size = matrizImgSize / 8;

    switch( executionType ) {
        case 1:
            // printf( "Ejecución de %s desde hilo principal.\n", filesToMerge->id );
            mainPthread( img01Buffer, img02Buffer, maskBuffer, outBuffer, matrizImgSize );
            break;
        case 2:
            // printf( "Ejecución de %s con varios hilos.\n", filesToMerge->id );
            break;
        case 3:
            // carga los tiempos de los hilos aca dentro
            mainPthread( img01Buffer, img02Buffer, maskBuffer, outBuffer, matrizImgSize );
            break;
        case 4:
            // Hilos con tiempo dentro de la funcion.
            break;
        case 5:
            // Nasm
            // printf( "Ejecución de %s desde ASM.\n", filesToMerge->id );
            // hola( img01Buffer, img02Buffer, maskBuffer, outBuffer, columnPixels, rowPixels, maskPadding );
            enmascarar_sasm( img01Buffer, img02Buffer, maskBuffer, outBuffer, i_size );
            break;
        default:
            printf( "Ejecucion fuera de parametros [%s].\n", filesToMerge->id );
            break;
    }

    fwrite( &outBuffer, sizeof( outBuffer ), 1, fOut );

    fclose( fImg01 );
    fclose( fImg02 );
    fclose( fMask );
    fclose( fOut );

}