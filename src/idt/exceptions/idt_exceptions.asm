[BITS 64]

; Import the C++ function from handler.cpp
extern exception_handler

; ==============================================================================
; MACRO 1: For exceptions that DO NOT HAVE an Error Code (e.g., Divide by zero)
; ==============================================================================
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0          ; Push a dummy Error Code (0) to keep the stack uniform
    push %1         ; Push the exception number (e.g., 0, 1, 2...)
    jmp isr_common_stub
%endmacro

; ==============================================================================
; MACRO 2: For exceptions that HAVE an Error Code (e.g., Page Fault)
; ==============================================================================
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    ; No dummy 0 here, the CPU ALREADY pushed the real Error Code!
    push %1         ; Push only the exception number
    jmp isr_common_stub
%endmacro

; ==============================================================================
; CREATION OF THE 32 EXCEPTIONS (Using the macros defined above)
; ==============================================================================
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8   ; Double Fault (HAS error code)
ISR_NOERRCODE 9
ISR_ERRCODE   10  ; Bad TSS (HAS error code)
ISR_ERRCODE   11  ; Segment Not Present (HAS error code)
ISR_ERRCODE   12  ; Stack Fault (HAS error code)
ISR_ERRCODE   13  ; General Protection Fault (HAS error code)
ISR_ERRCODE   14  ; Page Fault (HAS error code)
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17  ; Alignment Check (HAS error code)
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21  ; Control Protection (HAS error code)
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_ERRCODE   29  ; VMM Communication (HAS error code)
ISR_ERRCODE   30  ; Security Exception (HAS error code)
ISR_NOERRCODE 31

; ==============================================================================
; THE COMMON STUB
; Saves registers, calls the C++ handler, and restores everything
; ==============================================================================
isr_common_stub:
    ; 1. Save all 64-bit general-purpose registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; 2. Prepare arguments for C++ (System V AMD64 ABI)
    ; C++ expects: exception_handler(int_number, error_code)
    ; In 64-bit functions, the first argument goes into RDI, the second into RSI
    
    ; We pushed 15 8-byte registers (120 bytes total).
    ; So the interrupt number is located at RSP + 120
    mov rdi, [rsp + 120] 
    
    ; The Error Code is right below it, at RSP + 128
    mov rsi, [rsp + 128] 

    ; 3. Call our C++ "Brain"
    call exception_handler

    ; 4. Restore registers (in the exact reverse order they were saved!)
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; 5. Clean up the stack by removing the 2 numbers we pushed (8 bytes + 8 bytes = 16 bytes)
    add rsp, 16

    ; 6. Return to the interrupted code (iretq is mandatory in 64-bit!)
    iretq