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

    loadSectors 0x8400,5,22

    call check_disk_operation_success

    ;--------------------------------------------TO HERE-----------------------------------------
    isodetectedend:
    

    ; tell AMD this is a 64 bit system
    mov ax,0xEC00
    mov bl,2
    int 15h




    ;protected mode loader section

    
    print16 msg3

    ;checking A20
    call enable_A20

    ;change video mode to vesa

    call changeToVesa


    mov ax, 0x0003  ; modalità VGA text 80x25
    int 0x10

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
alF db "al INCORRECT, not all sectors got read!!", 13, 10, 0

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



vbe_controller_addr equ 0x7000
mode_info_addr equ 0x7200

cfC db "cf = false, Disk Read Success!!", 13, 10, 0
alT db "al Correct, All Sectors Loaded.", 13, 10, 0
msgISOReady db "ISO: All Sectors Loaded by BIOS", 13, 10, 0
msg3 db "Loading Protected x32 Mode......", 13, 10, 0
vesaErr db "VESA UNSUPPORTED!!", 13, 10, 0

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

changeToVesa:

    push es
    mov ax, 0x4F01
    mov cx, 0x118     
    mov di, vbe_controller_addr 
    int 0x10


    mov ax, 0x4F02          ; VBE Set Mode function
    mov bx, 0x4118          ; Mode 0x118 (1024x768, 32-bit colors) + LFB bit (0x4000)
    int 0x10
    pop es
    cmp ax, 0x004F          ; Check if function supported and successful
    jne .vesa_error
    ret

    .vesa_error:
        print16 vesaErr
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

%include "SystemSource/sysboot/protectedUtils.asm"

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
    xor eax, eax
    mov eax, [mode_info_addr + 40]  ; offset 40 = framebuffer address
    printNum32At eax, 0, 20, 0x0F
    ; edi = display cursor
    ;mov byte [edi], 'A'      ; ASCII CHAR
    ;mov byte [edi+1], 0x0F     ; Attributes (white on black)

    print32At debugString,45,7,0x0E

    ;loadAt 40,20
    
    xor eax,eax
    mov ax,[mode_info_addr+16]
    ;printNum32At eax, 0, 20, 0x0F  ; pitch
    xor eax,eax
    mov ax,[mode_info_addr+18]
    ;printNum32At eax, 0, 21, 0x0F  ; width  
    xor eax,eax
    mov ax,[mode_info_addr+20]
    ;printNum32At eax, 0, 22, 0x0F  ; height

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

    ; check CPUID Availability

    call checkCPUIDAv

    ; mov bl,[CPUIDflag]
    ; printNum32At ebx,20,20,0x0F

    ; check long mode Availability
    ; call yes
    call checkLongMode
    
    ; setup the addresses that will contain the paging tables
    call setupPagingTables
   
    ; switch to 32x Long Mode compatibility
    call switchToLongMode


    ;Load the 64-bit GDT
    lgdt [GDT64.Pointer]

    ; Perform the Far Jump to the 64-bit Code Segment
    ; 0x08 is the offset of the Code Descriptor in GDT64
    jmp 0x08:long_mode
  

;=========================================================================
;||                             64 BIT                                  ||
;=========================================================================
[BITS 64]

%include "SystemSource/sysboot/longUtils.asm"

kernel_entry equ 0x8400
long_mode:

    ; Load 64-bit Data Segment into segment registers
    ; 0x10 is the offset of the Data Descriptor in GDT64
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, 0x90000    ; Ricarica lo stack pointer a 64-bit
    and rsp, -16        ; Forza l'allineamento a 16 byte (fondamentale!)

    ; --- 64-BIT TEST ---
    ; Let's write "OK" in Green on Black at the top-right corner
    ; using 64-bit registers (RAX and RDI)
    ;mov rax, 0x0A4B0A4F          ; 0x0A = Green, 'O' = 0x4F, 'K' = 0x4B
    ;mov rdi, 0xB8000 + 156       ; Video memory address for top-right
    ;mov [rdi], rax               ; Write 4 bytes (O, attr, K, attr) at once

    ; From here, the CPU is running in full 64-bit Long Mode.
    ;hlt                          ; Halt the CPU




    ;jmp kernel_entry ; jmp to kernel



jmp $
TIMES 2048 - ($ - $$) db 0


