global enmascarar_asm

section .data
   
    blanco db 255,255,255,255,255,255,255,255
    
section .text
enmascarar_asm:

    push ebp ; enter
    mov ebp, esp  ; enter

    mov ebx,0 ;inicio la variable en 0 (registro para desplazarce)
    mov edx,[ebp+24] ;cantidad de bytes por imagen
    movq mm0,qword[blanco] ;paso a mm0 el blanco para usarlo con simd

loopeo:

cmp edx,0 ;cuando es igual a 0 termina el programa
je terminar

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
