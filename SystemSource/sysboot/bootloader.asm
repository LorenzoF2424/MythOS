[ORG 0x7c00]
[BITS 16]
;macro section



%macro print16 1
    push si
    mov si, %1      
    call printf16      
    pop si
%endmacro
%macro loadSectors 3
    push bx
    mov bx, %1
    mov cl,%2
    mov byte [nSectors],%3
    call loadSectorsf
    pop bx
%endmacro



;end macro section
start:
jmp	0x0000:bootmain

    nSectors db 0
    diskNum db 0
    times 8-($-$$) db 0

     ;	Boot Information Table for ISO Compatibility(dd is used because there is no text and data sections)
    bi_PrimaryVolumeDescriptor  dd  0    ; LBA of the Primary Volume Descriptor
    bi_BootFileLocation         dd  0    ; LBA of the Boot File
    bi_BootFileLength           dd  0    ; Length of the boot file in bytes
    bi_Checksum                 dd  0    ; 32 bit checksum
    bi_Reserved                 times 40 db 0   ; Reserved 'for future standardization'


bootmain:
    
    ;variables initialization
    xor bx, bx
    xor ax, ax
    mov ds, ax
    mov es, ax      
    mov ss, ax
    mov sp, 0x7c00
    mov [diskNum],dl


    
    print16 msg

    cmp dl,0xA0
    jae .isoprint

    print16 msgHD

    jmp .notisoprint
    .isoprint:

        print16 msgISO

    .notisoprint:
    ;expanding available memory space on the disk

    print16 msg2
    ; check if iso for different memory expansion

    cmp byte [diskNum],0xA0
    jae isodetected
    ;-----------------------------------IF ISO SKIP FROM HERE-----------------------------------

    ;load 3 more sectors if hard drive to reach 2kb
    
    loadSectors 0x7E00,2,3
    
    call check_disk_operation_success

    ;also load a lot more sectors for the kernel and everything else

    loadSectors 0x8400,5,3

    call check_disk_operation_success

    ;--------------------------------------------TO HERE-----------------------------------------
    isodetectedend:
    

    

    ;protected mode loader section

    
    print16 msg3

    ;checking A20
    call enable_A20

    ;loading descriptor
    cli
    lgdt [GDT_Descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;start 32bit protected mode
    jmp CODE_SEG:protected_mode

; END FIRST SECTOR REAL CODE
; CONTINUE ON LINE ∼250








printf16:
    push ax
    push bx
    
    .printLoop:
        
        lodsb
        or al,al
        jz .finished

        mov bh,0
        mov ah,14
        int 10h

    jmp .printLoop
    .finished:
      
    pop bx
    pop ax
ret

loadSectorsf:

    mov ax, 0
    mov es , ax
    mov ah,2
    
    mov al,[nSectors]
    mov ch,0
    ;mov cl,starting sector
    mov dh,0
    mov dl,[diskNum]
    ; mov bx,0xwhatever shoud be here
    int 13h

ret


check_disk_operation_success:
    jc .disk_error
    jmp .noDiskErr
    .disk_error:

        print16 cfE

    jmp .if1end
    .noDiskErr:

        print16 cfC

    .if1end:
    
    
    cmp byte [nSectors],al
    je .true2
    jmp .false2
    .true2:

        print16 alT

    jmp .if2end
    .false2:

        print16 alF

    .if2end:

ret


msg db "Bootloader found!!! Mode: ", 0
msgHD db "HD", 13, 10, 0
msgISO db "ISO", 13, 10, 0
msg2 db "Loading other sectors for more memory:", 13, 10, 0
cfE db "cf = true, Disk Read FAILURE!!", 13, 10, 0
alF db "al INCORRECT, one or more sectors didn't get read!!", 13, 10, 0

align 4
dap_packet:
    db 0x10          ; Size (16 bytes)
    db 0             ; Reserved
    dw 16            ; Number of sectors
    dw 0x1000        ; Offset
    dw 0x0000        ; Segment
    dd 1             ; LBA low 32 bits
    dd 0             ; LBA high 32 bits


TIMES 510 - ($ - $$) db 0	;Fill the rest of sector with 0
DW 0xAA55	

;=========================================================================
;||                           SECTOR 2-4                                ||
;=========================================================================

cfC db "cf = false, Disk Read Success!!", 13, 10, 0
alT db "al Correct, All Sectors Loaded.", 13, 10, 0
msgISOReady db "ISO: All Sectors Loaded by BIOS", 13, 10, 0
msg3 db "Loading Protected x32 Mode......", 13, 10, 0

isodetected:

    print16 msgISOReady

jmp isodetectedend



enable_A20:
    pusha
    in al, 0x92
    or al, 2
    out 0x92, al
    popa
ret

GDT_Start:
    null_descriptor:
        dq 0
        
    code_descriptor:
        dw 0xffff
        dw 0
        db 0
        db 10011010b
        db 11001111b
        db 0
    data_descriptor:
        dw 0xffff
        dw 0
        db 0
        db 10010010b
        db 11001111b
        db 0

        
    
 
GDT_End:
GDT_Descriptor:
    dw GDT_End - GDT_Start - 1 ; size
    dd GDT_Start               ; start

CODE_SEG equ code_descriptor - GDT_Start
DATA_SEG equ data_descriptor - GDT_Start ; equ sets constants

IDT_ADDRESS equ 0x1000   ; free address to put the table
IDT_Descriptor:
    dw (256 * 8) - 1     ; size (256 slots)
    dd IDT_ADDRESS       ; physical address

;=========================================================================
;||                             32 BIT                                  ||
;=========================================================================
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
checkLongMode db "Long Mode:    ",0





kernel_entry equ 0x8400




; protected mode code starts here
protected_mode:

    mov ax, DATA_SEG     ; 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x90000
    mov esp, ebp  ; Stack safe
    
    ; edi = display cursor
    ;mov byte [edi], 'A'      ; ASCII CHAR
    ;mov byte [edi+1], 0x0F     ; Attributes (white on black)

    print32At debugString,45,7,0x0E

    ;loadAt 40,20
    

    mov edi, IDT_ADDRESS
    xor eax, eax
    mov ecx, 512 ; 2048 byte / 4
    rep stosd    ; clean memory at IDT address

    ; HANDLER CREATION

    ; Division by Zero (0)
    setIDTGate 0, exception_handler
    
    ; General Protection Fault (13)
    setIDTGate 13, exception_handler

    ; Load the IDT
    lidt [IDT_Descriptor]

   
    ; Test IDT: Divison by Zero
    ;xor edx, edx
    ;div edx        ; /0

    ;check CPUID Availability

    call checkCPUIDAv

    ;mov bl,[CPUIDflag]
    ;printNum32At ebx,20,20,0x0F

    ;check long mode Availability
    ;call yes
    call check_long_mode
    
    push eax
    push ecx
    push edi
    ;call setupPagingTables
    pop edi
    pop ecx
    pop eax

    ;jmp kernel_entry ; jmp to kernel
  

  ;call load_Long_Mode
JMP $






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

PML4_ADDR equ 0x10000   ; 64 KB
PDPT_ADDR equ 0x11000   ; 68 KB
PD_ADDR   equ 0x12000   ; 72 KB
PT_ADDR   equ 0x13000   ; 76 KB

setupPagingTables:
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

ret


check_long_mode:

    print32At checkLongMode,45,9,0x0F

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


switch_long_mode:

    mov eax, cr0
    and eax,0x7FFFFFFF
    mov cr0,eax
    mov eax, cr4
    or eax,0x620
    mov cr4,eax
    mov eax, dword[ebp+0x4]
    mov cr3, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax,0x101
    wrmsr
    mov eax,cr0
    or eax,0xE00000E
    mov cr0,eax
    ;jmp 0x08:long_mode
ret



TIMES 2048 - ($ - $$) db 0


