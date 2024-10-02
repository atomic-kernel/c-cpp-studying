    .globl my_write
my_write:
    movl    $1, %eax
    syscall
    retq
