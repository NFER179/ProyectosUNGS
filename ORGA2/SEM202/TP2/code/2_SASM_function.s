global enmascarar_asm

section .data
   
    blanco db 255,255,255,255,255,255,255,255
    colorBlanco dd 255                            ;la uso igual que la tira pero para instrucciones de uno en uno
    colorNegro dd 0                               ;color negro para negar la mascara (sin usar simd)
    acumulador dd 0                               ;acumulador para cuando aplico mascara sin instrucciones SIMD                          
    desplazamiento dd 0                           ;desplazamiento para cuando aplico mascara sin instrucciones SIMD  
    aux dd 0
    
section .text
enmascarar_asm:

    push ebp ; enter
    mov ebp, esp  ; enter

    mov ebx,0 ;inicio la variable en 0 (registro para desplazarce)
    mov edx,[ebp+24] ;cantidad de bytes por imagen
    movq mm0,qword[blanco] ;paso a mm0 el blanco para usarlo con simd

loopeo:

      cmp edx,8 ;cuando es igual a 0 termina el programa
      jl menorA8

      mov eax,[ebp+16] 
      movq mm1,[eax+ebx] ;guardo mascara en mm1

      mov eax,[ebp+8]
      movq mm2,[eax+ebx] ;guardo img1 en mm2

      mov eax,[ebp+12]
      movq mm3,[eax+ebx] ;guardo img2 en mm3

      pand mm3,mm1  ;saco los pixeles de la img2 donde la mask es negra
      pxor mm1,mm0  ;hago el contrario de la mascara con el mm0(blanco)
      pand mm2, mm1 ;saco los pixeles de la img1 donde la mask es blanca
      por mm2, mm3 ;combino las dos imagenes cambiadas y la guardo en mm2(img1)
      
      mov eax,[ebp+8] ;cargo la img1 para guardar cambios
      movq qword[eax+ebx], mm2 ;guardo lo que tengo en mm1

      add ebx,8 ;sumo para el desplazamiento
      sub edx,8 ;resto cantidad de bytes

      jmp loopeo ;salto a loopeo

menorA8:
    ;valido que edx no sea 0 - pasaria solo cuando la cant de bytes sea un multiplo de 8
    cmp edx,0    
    je fin

    ;A partir de acÃ¡ la lÃ³gica es la misma pero en vez de aplicar la mascara en 8 bits,se hace de uno en uno 
    ;(se harÃ¡ a lo sumo 7 veces y solo en casos que la cant de bits no sea mÃºltiplo de 8)
    
    ;guardo el acumulador que desciende
    mov dword[acumulador],edx
    ;guardo el desplazamiento | notar que viene del lugar donde quedo en el ciclo anterior
    mov dword[desplazamiento],ebx

segundoLoop:

    ;guardo para aplicar ese color en mascara
    mov edx, [colorBlanco]

    ;las operaciones se ejecutaran de forma tal que se puedan usar todos los registros sin que se pisen entre si

    mov eax, [desplazamiento] ;guardo en eax el desplazamiento de cada img

    mov ebx,[ebp+12] ;img2
    mov ecx,[ebx+eax] ;guardo contenido de img2 en ecx | me desplazo con eax

    mov ebx,[ebp+16];mascara
    mov edx,[ebx+eax];guardo contenido de mascara en edx | me desplazo con eax

    and ecx,edx ; borro de la img 2 los canales de pixeles donde la mascara es negra 
    xor edx,[colorNegro]; creo la negacion de la mascara | aca tengo el color negro que en mmx tenia en mm3

    mov dword[aux],edx ;guardo en variable aux el bit que obtuve de la operacion anterior para poder usar edx
    
    mov edx,[desplazamiento] ;guardo en edx el desplazamiento de cada img

    mov ebx,[ebp+8] ;img1
    mov eax,[ebx+edx];guardo contenido de mascara en eax | me desplazo con edx
     
    and eax,[aux] ;borro de la img 1 los canales de pixeles donde la imagen es blanca
    or eax,ecx ;sumamos las dos imagenes alteradas y guardo el resultado en la imagen 1

    mov ecx,[ebp+8] ; vuelvo a cargar img1 en ecx para guardar cambios
    mov [ecx+edx], eax ;guardo lo obtenido en eax (resultados parciales) en la img1 (con su desplazamiento en edx) 

    SUB dword[acumulador],1 ;bajo el acumulador que controla el ciclo
    add dword[desplazamiento],1      ;subo el desplazamiento

    ;repito el ciclo mientras el acumulador no sea 0
    cmp dword[acumulador],0
    JNE menorA8Ciclo

terminar:
    mov ebp,esp 
    pop ebp 

    ret

    ;Get parameters
    ;Tomo los valores desde C
    ;            +----------------------+
    ;esp, ebp -> |         ...          |
    ;            +----------------------+
    ;            |         hola         | +4
    ;            +----------------------+
    ;            | Img01 Buffer pointer | +8
    ;            +----------------------+
    ;            | Img02 Buffer pointer | +12
    ;            +----------------------+
    ;            |  Mask Buffer pointer | +16
    ;            +----------------------+
    ;            |  Out buffer pointer  | +20
    ;            +----------------------+
    ;            |       cant           | +24
    ;            +----------------------+
    ;            |        .....          | +28
    ;            +----------------------+
    ;            |       .......        | +32
    ;            +----------------------+
