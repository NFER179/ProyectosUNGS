#!/bin/bash

#$1     #01folder
#$2     #02folder
#$3     #maskFolder
#$4     #toFolder

#processFolder="bmpFiles" #Carpeta en la que se dejaran los archivos que se pasen a .bmp
fMask="enmascarados"
fSummary="reportes"
#mkdir $processFolder
bmpFiles="bmps"

#Pregunto si exista la carpeta donde se van a almacenar todas las imagenes.
#if [ -d $processFolder ]; then
if [ -d "$2" ]; then
    #Caso que existe consulto que las quiere eliminar y volver a crear (limpiar).
#    echo "Ya existe la carpeta ${processFolder}"
    echo "Ya existe la carpeta $2"
    echo "ADVERTENCIA: Si no reemplaza la carpeta podra sufrir perdida de información."
    echo "¿Desea reemplazarla? [Y/n]"
    read answer

    if [ "${answer}" = "Y" ] || [ "${answer}" = "y" ] ; then
    #En caso de no quere dejar los archivos viejos elimino la carpeta y la vuelvo a crear.
#        rm -R $processFolder
#        mkdir $processFolder
#        mkdir "$processFolder/$fMask"
        rm -R $2
        mkdir $2
        #
        mkdir "$2/$bmpFiles"
        #
        mkdir "$2/$fMask"
        mkdir "$2/$fSummary"
    else
        #En el caso de no quere "limpiar" la carpeta consulto si la carpeta enmascarados esta creada.
        #    if [ -d "${processFolder}/${fMask}" ]; then
        if [ -d "$2/${fMask}" ]; then
            #Caso que exista se consulta si se la quiere mantener, advirtiendo que se pueden pisar archivo.
#           echo "Ya existe la carpeta \"${processFolder}/${fMask}/\"."
            echo "Ya existe la carpeta \"$2/${fMask}/\"."
            echo "¿Desea reemplazlar? [Y/n]"
            read answer2
            if [ "${answer2}" = "Y" ]; then
#               rm -R "$processFolder/${fMask}"
                rm -R "$2/${fMask}"
                mkdir "$2/${fMask}"
            fi
        else
            #Caso que no exista la carpeta se crea
            mkdir "$2/${fMask}"
        fi

#
        if [ ! -d "$2/${bmpFiles}" ]; then
            mkdir "$2/${bmpFiles}"
        fi
#

        if [ ! -d "$2/${fSummary}" ]; then
            mkdir "$2/${fSummary}"
        fi
    fi
else
    #Caso en que no exista la carpeta se creara la carpeta.
#    mkdir "${processFolder}/enmascarados/"
    mkdir "$2"
#
    mkdir "$2/${bmpFiles}"
#
    mkdir "$2/${fMask}/"
    mkdir "$2/${fSummary}"
fi

#Primero paso los archivos bmp existentes
#if [ -n $(find . -maxdepth 1 -name '.bmp' -type f -print quit) ];then
#    echo "tiene"
#fi
for f in $1/*.bmp; do
    cp $f "$2/${bmpFiles}/$(basename $f)"
#    echo "file: $f $2/$(basename $f)"
done

for f in $1/*.jpg; do
    cp $f "$2/${bmpFiles}/$(basename $f)"
#    echo "for2: $f to $2/$(basename $f)"
done

#Ahora convierto todo a bmp
for f in $2/${bmpFiles}/*.jpg; do
    gm convert $f "${f%.*}.bmp"
    rm $f
done

nasm -f elf32 2_SASM_function_v02.s
gcc -m32 -o executable 2_SASM_function_v02.o 2_main_v02.c 2_functions.c -lpthread
./executable $2