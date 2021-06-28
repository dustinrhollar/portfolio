.code

save_fiber_ctx proc
    ; Save return address and stack ptr
    
    ; Note(Dustin): Not sure if this is correct...
    ; RSP *should* be the pointer to the top of stack
    ; but the contents of the stack is a ptr to another
    ; memory location, so treat it as a double pointer
    mov r8, rsp
    mov r8, qword ptr [r8]
    mov [rcx + 8*0], r8 ; rip

    lea r8, qword ptr [rsp + 8]
    mov qword ptr [rcx + 8*1], r8 ; rsp

    ; Save register values
    mov qword ptr [rcx + 8*2], rbx
    mov qword ptr [rcx + 8*3], rbp
    mov qword ptr [rcx + 8*4], rdi
    mov qword ptr [rcx + 8*5], rsi
    mov qword ptr [rcx + 8*6], r12
    mov qword ptr [rcx + 8*7], r13
    mov qword ptr [rcx + 8*8], r14
    mov qword ptr [rcx + 8*9], r15

    ; Save mmx family of registers
    movups xmmword ptr [rcx + 8*10 + 16*0], xmm6
    movups xmmword ptr [rcx + 8*10 + 16*1], xmm7
    movups xmmword ptr [rcx + 8*10 + 16*2], xmm8
    movups xmmword ptr [rcx + 8*10 + 16*3], xmm9
    movups xmmword ptr [rcx + 8*10 + 16*4], xmm10
    movups xmmword ptr [rcx + 8*10 + 16*5], xmm11
    movups xmmword ptr [rcx + 8*10 + 16*6], xmm12
    movups xmmword ptr [rcx + 8*10 + 16*7], xmm13
    movups xmmword ptr [rcx + 8*10 + 16*8], xmm14
    movups xmmword ptr [rcx + 8*10 + 16*9], xmm15

    ; Return
    xor eax, eax
    ret
save_fiber_ctx endp

restore_fiber_ctx proc
    mov r8,  qword ptr [rcx + 8*0] ; rip, return the address set with ctx
    mov rsp, qword ptr [rcx + 8*1] ; rsp, load stack pointer

    mov rbx, qword ptr [rcx + 8*2]
    mov rbp, qword ptr [rcx + 8*3]
    mov rdi, qword ptr [rcx + 8*4]
    mov rsi, qword ptr [rcx + 8*5]

    mov r12, qword ptr [rcx + 8*6]
    mov r13, qword ptr [rcx + 8*7]
    mov r14, qword ptr [rcx + 8*8]
    mov r15, qword ptr [rcx + 8*9]

    movups xmm6,  xmmword ptr [rcx + 8*10 + 16*0]
    movups xmm7,  xmmword ptr [rcx + 8*10 + 16*1]
    movups xmm8,  xmmword ptr [rcx + 8*10 + 16*2]
    movups xmm9,  xmmword ptr [rcx + 8*10 + 16*3]
    movups xmm10, xmmword ptr [rcx + 8*10 + 16*4]
    movups xmm11, xmmword ptr [rcx + 8*10 + 16*5]
    movups xmm12, xmmword ptr [rcx + 8*10 + 16*6]
    movups xmm13, xmmword ptr [rcx + 8*10 + 16*7]
    movups xmm14, xmmword ptr [rcx + 8*10 + 16*8]
    movups xmm15, xmmword ptr [rcx + 8*10 + 16*9]
    
    ; push the function to jmp to
    push r8

    ; Return.
    xor eax, eax
    ret
restore_fiber_ctx endp

; Callback procedure that is invoked after a fiber finishes executing with
; swap fiber context
return_to_fiber_ctx proc
    pop r10

    mov r8,  qword ptr [r10 + 8*0] ; rip, return the address set with ctx
    mov rsp, qword ptr [r10 + 8*1] ; rsp, load stack pointer

    mov rbx, qword ptr [r10 + 8*2]
    mov rbp, qword ptr [r10 + 8*3]
    mov rdi, qword ptr [r10 + 8*4]
    mov rsi, qword ptr [r10 + 8*5]

    mov r12, qword ptr [r10 + 8*6]
    mov r13, qword ptr [r10 + 8*7]
    mov r14, qword ptr [r10 + 8*8]
    mov r15, qword ptr [r10 + 8*9]

    movups xmm6,  xmmword ptr [r10 + 8*10 + 16*0]
    movups xmm7,  xmmword ptr [r10 + 8*10 + 16*1]
    movups xmm8,  xmmword ptr [r10 + 8*10 + 16*2]
    movups xmm9,  xmmword ptr [r10 + 8*10 + 16*3]
    movups xmm10, xmmword ptr [r10 + 8*10 + 16*4]
    movups xmm11, xmmword ptr [r10 + 8*10 + 16*5]
    movups xmm12, xmmword ptr [r10 + 8*10 + 16*6]
    movups xmm13, xmmword ptr [r10 + 8*10 + 16*7]
    movups xmm14, xmmword ptr [r10 + 8*10 + 16*8]
    movups xmm15, xmmword ptr [r10 + 8*10 + 16*9]
    
    push r8

    ; Return.
    xor eax, eax
    ret

return_to_fiber_ctx endp

; Linux
; rdi 1st arg: context to save
; rsi 2nd arg: context to restore
;
; Win32
; rcx 1st arg: context to save
; rdx 2nd arg: context to restore
swap_fiber_ctx proc
    ; Save return address and stack ptr
    mov r8, rsp
    mov r8, qword ptr [r8]
    mov qword ptr [rcx + 8*0], r8 ; rip

    lea r8, qword ptr [rsp + 8]
    mov qword ptr [rcx + 8*1], r8 ; rsp

    ; Save register values
    mov qword ptr [rcx + 8*2], rbx
    mov qword ptr [rcx + 8*3], rbp
    mov qword ptr [rcx + 8*4], rdi
    mov qword ptr [rcx + 8*5], rsi
    mov qword ptr [rcx + 8*6], r12
    mov qword ptr [rcx + 8*7], r13
    mov qword ptr [rcx + 8*8], r14
    mov qword ptr [rcx + 8*9], r15
    movups xmmword ptr [rcx + 8*10 + 16*0], xmm6
    movups xmmword ptr [rcx + 8*10 + 16*1], xmm7
    movups xmmword ptr [rcx + 8*10 + 16*2], xmm8
    movups xmmword ptr [rcx + 8*10 + 16*3], xmm9
    movups xmmword ptr [rcx + 8*10 + 16*4], xmm10
    movups xmmword ptr [rcx + 8*10 + 16*5], xmm11
    movups xmmword ptr [rcx + 8*10 + 16*6], xmm12
    movups xmmword ptr [rcx + 8*10 + 16*7], xmm13
    movups xmmword ptr [rcx + 8*10 + 16*8], xmm14
    movups xmmword ptr [rcx + 8*10 + 16*9], xmm15

    ; Restore the other context
    mov r8,  qword ptr [rdx + 8*0] ; rip, return the address set with ctx
    mov rsp, qword ptr [rdx + 8*1] ; rsp, load stack pointer

    mov rbx, qword ptr [rdx + 8*2]
    mov rbp, qword ptr [rdx + 8*3]
    mov rdi, qword ptr [rdx + 8*4]
    mov rsi, qword ptr [rdx + 8*5]

    mov r12, qword ptr [rdx + 8*6]
    mov r13, qword ptr [rdx + 8*7]
    mov r14, qword ptr [rdx + 8*8]
    mov r15, qword ptr [rdx + 8*9]

    movups xmm6,  xmmword ptr [rdx + 8*10 + 16*0]
    movups xmm7,  xmmword ptr [rdx + 8*10 + 16*1]
    movups xmm8,  xmmword ptr [rdx + 8*10 + 16*2]
    movups xmm9,  xmmword ptr [rdx + 8*10 + 16*3]
    movups xmm10, xmmword ptr [rdx + 8*10 + 16*4]
    movups xmm11, xmmword ptr [rdx + 8*10 + 16*5]
    movups xmm12, xmmword ptr [rdx + 8*10 + 16*6]
    movups xmm13, xmmword ptr [rdx + 8*10 + 16*7]
    movups xmm14, xmmword ptr [rdx + 8*10 + 16*8]
    movups xmm15, xmmword ptr [rdx + 8*10 + 16*9]

    push r8

    ; Return.
    xor eax, eax
    ret
swap_fiber_ctx endp

end