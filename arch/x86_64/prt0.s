# stolen from: https://en.wikipedia.org/wiki/Crt0 with some changes

.text

.globl _start
.globl exit

_start:
    xor %rbp, %rbp
    mov (%rsp), %rdi
    lea 8(%rsp), %rsi
    lea 16(%rsp,%rdi,8), %rdx
    xor %rax, %rax
    call main

    mov %rax, %rdi
    call exit

exit:
    mov $60, %rax
    syscall
    hlt
