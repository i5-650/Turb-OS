; https://github.com/limine-bootloader/limine/blob/c90ddf6b20c90375625a70ed87f870580bc5c263/common/lib/mem.asm64
;)

memcpy:
    mov rcx, rdx
    mov rax, rdi
    rep movsb
    ret
GLOBAL memcpy

memset:
    push rdi
    mov rax, rsi
    mov rcx, rdx
    rep stosb
    pop rax
    ret
GLOBAL memset

memmove:
    mov rcx, rdx
    mov rax, rdi

    cmp rdi, rsi
    ja .copy_backwards

    rep movsb
    jmp .done

.copy_backwards:
    lea rdi, [rdi + rcx - 1]
    lea rsi, [rsi + rcx - 1]
    std
    rep movsb
    cld

.done:
    ret
GLOBAL memmove

memcmp:
    mov rcx, rdx
    repe cmpsb
    jecxz .equal

    mov al, byte [rdi - 1]
    sub al, byte [rsi - 1]
    movsx rax, al
    jmp .done

.equal:
    xor eax, eax

.done:
    ret
GLOBAL memcmp