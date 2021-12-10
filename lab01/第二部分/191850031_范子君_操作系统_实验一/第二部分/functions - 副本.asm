asciiToNum:
    push edx
    mov dl, 48
    sub al, dl
    pop edx
    ret


clearAh:
    push edx
    mov edx, 0
    mov dl,al
    mov eax, edx
    pop edx ; !! eax = al = carry
    ret
    
sinput:
    push eax
    push ebx
    push ecx
    push edx
    
    mov eax, 3
    mov ebx, 0
    mov ecx, buff
    mov edx, 100
    int 80h
    
    pop eax
    pop ebx
    pop ecx
    pop edx
    ret
    

printLF:
    push eax 
    push ebx
    push ecx
    push edx
    
    mov eax, 10
    push eax
    mov ecx, esp
    pop eax
    mov eax, 4
    mov ebx, 1
    mov edx, 1
    int 80h
    
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

    


slen:
    push    ebx
    mov     ebx, eax
 
nextchar:
    cmp     byte [eax], 0
    jz      finished
    inc     eax
    jmp     nextchar
 
finished:
    sub     eax, ebx
    pop     ebx
    ret
 

sprint:
    push    edx
    push    ecx
    push    ebx
    push    eax
    call    slen
 
    mov     edx, eax
    pop     eax
 
    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h
 
    pop     ebx
    pop     ecx
    pop     edx
    ret
 
 

sprintLF:
    call    sprint
 
    push    eax
    mov     eax, 0AH
    push    eax
    mov     eax, esp
    call    sprint
    pop     eax
    pop     eax
    ret
 
 

quit:
    mov     ebx, 0
    mov     eax, 1
    int     80h
    ret


