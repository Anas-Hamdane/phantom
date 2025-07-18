.text

.globl _start
.globl _exit

_start: # _start is the entry point known to the linker
    xor %rbp, %rbp            # effectively RBP := 0, mark the end of stack frames
    mov (%rsp), %rdi          # get argc from the stack
    lea 8(%rsp), %rsi         # take the address of argv from the stack
    lea 16(%rsp,%rdi,8), %rdx # take the address of envp from the stack
    xor %rax, %rax            # per ABI and compatibility with icc
    call main                 # %rdi, %rsi, %rdx are the three args to main

    # If main returns, exit with its return value
    mov %rax, %rdi            # transfer the return of main to exit code
    call _exit                # call our custom exit function

# Custom exit function - can be called from anywhere in the program
_exit:
    mov $60, %rax             # system call number for sys_exit on x86_64
    syscall                   # invoke system call
    # syscall doesn't return, but just in case:
    hlt                       # halt processor (should never reach here)
