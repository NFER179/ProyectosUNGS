extern printf
global enmascarar_sasm

section .data

    tira_blanco: db 255,255,255,255,255,255,255,255
    array_index: dd 0; index del array.
    array_len: dd 0; Largo del array.

section .bss

section .text
enmascarar_sasm:

    push ebp
    mov ebp,esp

    ;Tomo los valores desde C
    ;            +----------------------+
    ;esp, ebp -> |         ...          |
    ;            +----------------------+
    ;            |   enmascarar_sasm    | +4
    ;            +----------------------+
    ;            | Img01 Buffer pointer | +8
    ;            +----------------------+
    ;            | Img02 Buffer pointer | +12
    ;            +----------------------+
    ;            |  Mask Buffer pointer | +16
    ;            +----------------------+
    ;            |  Out buffer pointer  | +20
    ;            +----------------------+
    ;            |       Cantidad       | +24
    ;            +----------------------+

    xor edx, edx                ; limpiamos la variable edx.

    mov ecx, [ ebp + 24 ]       ; Total de bytes.
    mov [ array_len ], ecx

    movq mm4, qword[ tira_blanco ]

loop:
    cmp edx, ecx
    jg greater_than_array_size

    ; Cargamos cargamos en conteido de las imagenes en posición ebx * 8
    mov eax, [ ebp + 8 ]        ; Buffer img01
    movq mm0, [ eax + edx * 8 ]

    mov eax, [ ebp + 12 ]       ; Buffer img02
    movq mm1, [ eax + edx * 8 ]

    mov eax, [ ebp + 16 ]       ; Buffer mask
    movq mm2, [ eax + edx * 8 ]

    mov eax, [ ebp + 20 ]       ; Buffer out
    movq mm3, [ eax + edx * 8 ]

    pand mm1, mm2
    pxor mm2, mm4
    pand mm0, mm2
    por mm0, mm1

    mov eax, [ ebp + 20 ]
    movq qword[ eax + edx * 8 ], mm0

    inc edx
    jmp loop

greater_than_array_size:
    cmp edx, ecx
    je finish

;    mov dword[ acumulador ]
;    mov dword[ desplazamiento ]



finish:

    mov esp, ebp
    pop ebp

    xor eax, eax
    ret