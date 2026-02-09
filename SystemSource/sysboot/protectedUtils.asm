[BITS 32]
;macro section for protected mode
%macro loadAt 2
    push edx
    mov dl,%1
    mov dh,%2
    call loadAtPos
    pop edx
%endmacro
%macro print32At 4
    push edx 
    push ecx
    
    push esi
    mov dl,%2
    mov dh,%3
    mov cl,%4
    mov esi,%1
    call printf32
    pop esi
    pop ecx
    pop edx
%endmacro
%macro putc32 2
    mov al,%1
    mov cl,%2
    call putcf32
%endmacro
%macro print32 2
    push edx 
    push ecx
    push esi
   
    mov dh,25
    mov cl,%2
    mov esi,%1
    call printf32
    pop esi
    pop ecx
    pop edx
%endmacro

%macro printNum32At 4
    push edx 
    push ecx
    push esi
   
    mov esi,%1
    mov dl,%2
    mov dh,%3
    mov cl,%4
    call printNumf32
    pop esi
    pop ecx
    pop edx
%endmacro
%macro printNum32 2
    push edx 
    push ecx
    push esi                
    mov dh,25
    mov cl,%2
    mov esi, %1
    call printNumf32
    pop esi
    pop ecx
    pop edx
%endmacro
%macro setIDTGate 2
    push eax
    push ebx
    mov ebx,%1
    mov eax,%2
    call setIDTGatef
    pop ebx
    pop eax
%endmacro

;end macro section


debugString db "Protected Mode Working!!", 0
Number dd 0
CPUIDflag db 1
checkCPUIDstr db "CPUID STATUS: ", 0
Av db "AVAILABLE", 0
notAv db "NOT AVAILABLE", 0
msgcheckLongMode db "Long Mode:    ",0



loadAtPos: ;convert dl in column and dh in row(l*h)
    push ebx
    push eax
    push ecx

    mov ebx,0xB8000
    mov eax,80
    mul dh
    add al, dl
    SHL eax,1
    add ebx,eax
    mov edi,ebx

    pop ecx
    pop eax
    pop ebx
ret

putcf32:

    mov byte [edi], al 
    mov byte [edi+1], cl

    add edi,2

ret

yes:
    putc32 '[',0x0F
    putc32 251,0x0F
    putc32 ']',0x0F
ret

printf32:
    push eax

    cmp dh,25
    jge .skipLoad
    loadAt dl,dh
    .skipLoad:

    .printLoop:

        lodsb
        or al,al
        jz .finished

        
       call putcf32

    jmp .printLoop
    .finished:
    pop eax
ret

printNumf32:
        push eax
        push ebx
         
        cmp dh,25
        jge .skipLoad
        loadAt dl,dh
        .skipLoad:
        
        mov eax,esi
        cmp eax, 10
        jge .notMultipleDigits

        ;asdad
        add eax,'0'    ; '0' = 48 ???? vscode syntax bug
        call putcf32
        jmp .finished

        .notMultipleDigits:
        
            mov ebx,10
            xor ch,ch
        .getNum:

            

            xor edx,edx
            div ebx

           

            add dl,48
            push dx
            inc ch

            or eax,eax
            jz .printLoop
            
            
        jmp .getNum
            
        .printLoop:

            or ch,ch
            jz .finished
            
            pop dx
            mov byte [edi], dl 
            mov byte [edi+1], cl

            add edi,2

            dec ch
            
            
        jmp .printLoop

    .finished:

        
        pop ebx
        pop eax
ret



setIDTGatef:
    
    push edi


    ;calculate address in physical ram
    mov edi, ebx
    shl edi,3
    add edi,IDT_ADDRESS                   
    

    mov word [edi], ax              ; Low Offset
    mov word [edi + 2], CODE_SEG    ; Selector
    mov byte [edi + 4], 0           ; Reserved
    mov byte [edi + 5], 10001110b   ; Flags
    shr eax, 16
    mov word [edi + 6], ax          ; Offset alto
    pop edi



ret


msgException db "KERNEL EXCEPTION DETECTED: "
msgExDiv0 db "Division by Zero", 0
msgExGPF db "General Protection Fault", 0
msgExUE db "Unknown Exception", 0

exception_handler:
    push esi

    print32At msgException, 0,24, 0x4F
    
    ; eax contains the exception number
    cmp eax,0
    je .isDiv0
    cmp eax,13
    je .isGPF
    
    
    ; default message
    mov esi,msgExUE
    jmp .printEnd
    
.isDiv0:
    mov esi,msgExDiv0
    jmp .printEnd
.isGPF:
    mov esi,msgExGPF

.printEnd:
    print32 esi,0x4F

    pop esi                         
iretd                   
       

; it may be preferrable to put this in a separate file to be included,
; along with any other EFLAGS bits you may want to use
EFLAGS_ID equ 1 << 21           ; if this bit can be flipped, the CPUID
                                ; instruction is available

; Checks if CPUID is supported by attempting to flip the ID bit (bit 21) in
; the EFLAGS register. If we can flip it, CPUID is available.
; returns eax = 1 if there is cpuid support; 0 otherwise
checkCPUIDAv:
    pushfd
    pop eax

    ; The original value should be saved for comparison and restoration later
    mov ecx, eax
    xor eax, EFLAGS_ID

    ; storing the eflags and then retrieving it again will show whether or not
    ; the bit could successfully be flipped
    push eax                    ; save to eflags
    popfd
    pushfd                      ; restore from eflags
    pop eax

    ; Restore EFLAGS to its original value
    push ecx
    popfd

    ; if the bit in eax was successfully flipped (eax != ecx), CPUID is supported.
    print32At checkCPUIDstr,45,8,0x0F

    xor eax, ecx
    jnz .supported
    .notSupported:
        print32 notAv,0x0F
        mov byte [CPUIDflag],0
    ret
    .supported:
        print32 Av,0x0F

    putc32 '(',0x0F
    mov bl,[CPUIDflag]
    printNum32 ebx,0x0F
    putc32 ')',0x0F

ret

checkLongMode:

    print32At msgcheckLongMode,45,9,0x0F

    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

 
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29      ; Test bit 29
    jz .no_long_mode
    
    mov eax, 1             

    print32 Av,0x0F


    ret

    .no_long_mode:
    xor eax, eax           
    print32 notAv,0x0F

ret






PML4_ADDR equ 0x10000   ; 64 KB
PDPT_ADDR equ 0x11000   ; 68 KB
PD_ADDR   equ 0x12000   ; 72 KB
PT_ADDR   equ 0x13000   ; 76 KB

setupPagingTables:
    push eax
    push ecx
    push edi

    mov edi, PML4_ADDR   
    xor eax, eax
    mov ecx, 4096      
    rep stosd

    mov edi, PML4_ADDR
    mov eax, 0x11003   
    mov [edi], eax

    mov edi, PDPT_ADDR
    mov eax, 0x12003   
    mov [edi], eax

    mov edi, PD_ADDR
    mov eax, 0x13003   
    mov [edi], eax

    mov edi, PT_ADDR
    mov eax, 0x00000003 
    mov ecx, 512

    .loop_pt:
        mov [edi], eax
        add eax, 0x1000
        add edi, 8          
    loop .loop_pt


    pop edi
    pop ecx
    pop eax

ret




switchToLongMode:
    push eax


    ; load PML4 address in CR3 since cpu needs to know its location
    mov eax, PML4_ADDR
    mov cr3, eax

   
    mov eax, cr4
    or eax, 1 << 5          ; enable bit PAE(5th bit)
    mov cr4, eax

    
    
    mov ecx, 0xC0000080     
    rdmsr                   
    or eax, 1 << 8          ; enable the bit 8 (LME)
    wrmsr                  


    mov eax, cr0
    or eax, 1 << 31 | 1 << 0 ; enable paging and protected mode at once Bit 31 (PG) e Bit 0 (PE)
    mov cr0, eax

    ; now you're in "Long Mode Compatibility", but still in 32-bit.
    ; the procesor needs a "Far Jump" to a 64-bit segment.


    pop eax
ret

align 8
GDT64:                           ; Global Descriptor Table for 64-bit Long Mode
    .Null: equ $ - GDT64         ; Null Descriptor (mandatory 8 bytes of zeros)
        dq 0
    .Code: equ $ - GDT64         ; 64-bit Code Segment Descriptor (Selector 0x08)
        dw 0                     ; Limit (ignored in 64-bit mode)
        dw 0                     ; Base (low 16 bits, ignored)
        db 0                     ; Base (mid 8 bits, ignored)
        db 10011010b             ; Access: Present, Ring 0, Executable, Readable
        db 00100000b             ; Flags: L-bit (Long Mode) set, SZ=0
        db 0                     ; Base (high 8 bits, ignored)
    .Data: equ $ - GDT64         ; 64-bit Data Segment Descriptor (Selector 0x10)
        dw 0                     ; Limit (ignored)
        dw 0                     ; Base (ignored)
        db 0                     ; Base (ignored)
        db 10010010b             ; Access: Present, Ring 0, Data, Writable
        db 00000000b             ; Flags: All zero for data
        db 0                     ; Base (ignored)
    .Pointer:
        dw $ - GDT64 - 1         ; Table Size (Limit) - 2 bytes
        dq GDT64                 ; Table Address (64-bit Base) - 8 bytes


