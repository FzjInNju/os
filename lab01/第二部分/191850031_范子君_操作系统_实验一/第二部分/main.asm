%include 'functions.asm'

SECTION .bss
    buff resb 100   ; 64 ascii, 64 byte
    
SECTION .data
    Result  times 400  db 0
    numA times 100 db 0
    numB times 100 db 0
    lenA times 4 db 0
    lenB times 4 db 0
    A times 100 db 0
    B times 100 db 0
    msg db 'Please enter x and y: ',0, 0Ah 

SECTION .text
global _start
_start:
    mov eax, msg
    call sprintLF
    call sinput
    
;----------------------------------
; int getOffset()
; ret edi(numA offset) and esi(numB offset)

    mov esi,0   ; !! cannot set in getOffset
    mov ebx, buff
getOffset:
    cmp byte[ebx + esi], 32 ; ascii(' ') = 32
    je .getedi
    cmp byte[ebx + esi] , 10 ; ascii('enter') = 10  
    je .finishGet
.continueGetOff:
    inc esi
    jmp getOffset
.getedi:
    mov edi, esi  
    dec edi
    jmp .continueGetOff
.finishGet:
    dec esi
   
   
   
;-----------------------------
;set numA  numB
setNumAandNumB:
    mov ecx, 0
setNumALoop:
    mov al, [buff + ecx]
    mov [numA + ecx], al
    cmp ecx,edi
    jg finishSetNumA
    inc ecx
    jmp setNumALoop
finishSetNumA:
    mov al,0
    mov [numA + ecx], al
    
    mov edx, ecx
    inc edx
    mov ecx, 0
setNumBLoop:
    mov al, [buff + edx]
    mov [numB + ecx], al
    cmp edx, esi
    jg finishSetNumB
    inc ecx
    inc edx
    jmp setNumBLoop
finishSetNumB:
    mov al, 0
    mov [numB + ecx], al
    
    
;------------------------------
; edi = numA offset  , len A = edi + 1
; esi = numB offset  , lenB = esi + 1
    sub esi, edi
    sub esi, 2
    push esi
    push edi
    inc esi
    inc edi
    mov [lenA], edi
    mov [lenB],esi
    pop edi
    pop esi
    
baseCal:
   
    mov eax, 100
    push eax
calculateSum:
    mov eax, 0    ;tempRes and carry
    mov edx, 0
    mov ecx, 0
.startCal:
    add al, byte[numA+edi]
    call asciiToNum
    add al, byte[numB+esi]
    call asciiToNum
    
    ;ax / cl = al(carry) .....ah(bitRes)
    mov cl, 10
    div cl 
    mov edx, 0
    mov dl, ah 
    push edx    ; !! push bitRes !! cannot push bl
    call clearAh ; !! eax = al = carry !! clear ah

    cmp esi, 0
    je .leaveNumA
    cmp edi, 0
    je .leaveNumB
    dec edi   
    dec esi
    
    jmp .startCal
    
.leaveNumA: ; the offset of numA is edi
    cmp edi,0
    je .checkCarry

    dec edi
    add al,[numA+edi]
    call asciiToNum      ; !!! dont forget ascii to num
    
    ;ax / cl = al(carry) .....ah(bitRes)
    mov cl, 10
    div cl 
    mov edx, 0
    mov dl, ah 
    push edx    ; !! push bitRes !! cannot push bl
    call clearAh ; !! eax = al = carry !! clear ah
    
    jmp .leaveNumA

.leaveNumB:
    cmp esi,0
    je .checkCarry

    dec esi
    add al,[numB+esi]
    call asciiToNum      ; !!! dont forget ascii to num
    
    ;ax / cl = al(carry) .....ah(bitRes)
    mov cl, 10
    div cl 
    mov edx, 0
    mov dl, ah 
    push edx    ; !! push bitRes !! cannot push bl
    call clearAh ; !! eax = al = carry !! clear ah
    
    jmp .leaveNumB

    
.checkCarry:
    cmp al,0
    je printSum
    push edx
    mov edx, 0
    mov dl, al
    mov eax, edx
    pop edx
    push eax
    
    
printSum:
    pop eax
    cmp eax,100
    je .finishPrintSum

    
    add eax, 48    ; the ascii to print
    push eax
    mov ecx, esp    ;get the address of eax
    pop eax
    mov eax, 4
    mov ebx, 1
    mov edx, 1
    int 80h
    jmp printSum
.finishPrintSum:
    
    call printLF
    
;----------------------------------
     mov al,48
     mov ecx,[lenA] ;ecx???A ?????????
      mov ebx,0   ; ebx???numA??????
 reverseStoreA: 
     sub [numA+ebx],al  ;numA???ebx??? - 48
      mov dl,[numA+ebx] ; dl = numA???ebx??? - 48
      dec ecx
     mov [A+ecx],dl  ; ??????
     inc ecx
    inc ebx ;numA ?????????+1
     dec ecx ; ecx??????
    cmp ecx, 0 ;???A????????????0??????
    jne  reverseStoreA

   
     mov al,48
     mov ecx,[lenB] ;ecx???A ?????????
      mov ebx,0   ; ebx???numA??????
 reverseStoreB: 
     sub [numB+ebx],al  ;numA???ebx??? - 48
      mov dl,[numB+ebx] ; dl = numA???ebx??? - 48
      dec ecx
     mov [B+ecx],dl  ; ??????
     inc ecx
    inc ebx ;numA ?????????+1
     dec ecx ; ecx??????
    cmp ecx, 0 ;???A????????????0??????
    jne  reverseStoreB

   
    
xor eax,eax  ;eax = 0
xor ebx,ebx  ;ebx= 0
mov edx,0   ;edx = 0
mulBit:	
    mov edx,0   ;ecx = 0
fixNumBBit: 
    mov al,[A+ecx]  ;al = A???ecx???
    mov bl,[B+edx]  ; bl = B[edx]
    imul eax,ebx   ;  eax = A???ecx???* B[edx]    
    mov edi,ecx   ;  
    add edi,edx    ;  edi = ecx + edx
    add [Result+edi * 4],eax   ; Result[4 * (ecx+edx)] += eax
    inc  edx  ;  A index + 1
    cmp edx,[lenB]   ; ???A index????????????
    jne  fixNumBBit
    inc ecx   ; B index + 1
    cmp ecx,[lenA]   ; ???B index ??? len ??????
    jne mulBit  
  

    mov ecx, 100   ; result????????? ecx
    mov ebx,10   ;???????????? 10 ebx
    mov edi,0    ;     
sumCarry:  
    ;edi  =  edi * 4
    push edi   ; ??????edi
    mov eax, 4   ; eax = 4
    mul edi     ; eax = edi * 4, ??????????????????????????????
    mov edi, eax   ; edi = edi * 4
           
    cmp [Result+edi],ebx   ; ????????????10??????
    jl finishSumCarry   ; ???10?????? ???????????????

    mov eax,[Result+edi]  ; eax???????????????
    xor edx,edx     ; edx = 0
    div ebx   ;  eax = carry    edx = result 
    add [Result+edi + 4],eax  ;???carry???????????????
    mov [Result+edi],edx  ; ???edx?????????????????????

    
    
finishSumCarry:
    pop edi
    inc edi    ; edi??????
    loop sumCarry  ;ecx??????0????????????
  

;----------------------------------------------
   mov ecx,100
clearZero:
    dec ecx   ; ecx* 4???result???index???ecx??? 99??????
    mov eax, 4    ; eax?????????4
    push ecx
    mul ecx   ; eax=  4 * ecx
    pop ecx
    mov edx,eax ; edx ??? 4 * ecx,
    cmp byte[Result+edx],0   ; ??? Result[index*4] ??? 0 ???
    Je clearZero   ; ???????????? ???????????????
    
   
;------------------------------------------
    inc ecx
printMul:
    cmp ecx,0
    je finishPrintMul
    dec ecx
    mov eax,[Result + ecx * 4]
    add eax, 48
    push ebx
    push edx
    push ecx
    push eax
    mov ecx, esp
    mov eax, 4
    mov ebx, 1
    mov edx, 1
    int 80h
    pop eax
    pop ecx
    pop edx
    pop ebx
    jmp printMul
finishPrintMul:
   call printLF
   call quit
    


    

