.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
li $v0, 4
la $a0, _prompt
syscall
li $v0, 5
syscall
jr $ra

write:
li $v0, 1
syscall
li $v0, 4
la $a0, _ret
syscall
move $v0, $0
jr $ra

main:
move $t3, $sp
addi $sp, $sp, -8
sw $fp, 4($sp)
sw $ra, 0($sp)
move $fp, $sp
addi $sp, $sp, -36
li $t0, 0
sw $t0, -4($fp)
lw $t1, -4($fp)
move $t0, $t1
sw $t0, -8($fp)
li $t0, 1
sw $t0, -12($fp)
lw $t1, -8($fp)
lw $t2, -12($fp)
add $t0, $t1, $t2
sw $t0, -16($fp)
lw $t1, -16($fp)
move $t0, $t1
sw $t0, -8($fp)
li $t0, 2
sw $t0, -20($fp)
lw $t1, -8($fp)
lw $t2, -20($fp)
blt $t1, $t2, label0
j label1
label0:li $t0, 1
sw $t0, -24($fp)
lw $t1, -8($fp)
lw $t2, -24($fp)
add $t0, $t1, $t2
sw $t0, -28($fp)
lw $t1, -28($fp)
move $t0, $t1
sw $t0, -8($fp)
j label2
label1:li $t0, 1
sw $t0, -32($fp)
lw $t1, -8($fp)
lw $t2, -32($fp)
sub $t0, $t1, $t2
sw $t0, -36($fp)
lw $t1, -36($fp)
move $t0, $t1
sw $t0, -8($fp)
label2:lw $t0, -8($fp)
move $v0, $t0
jr $ra
