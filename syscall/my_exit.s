    .globl my_exit
my_exit:
    movl    $0xe7, %eax
    syscall
