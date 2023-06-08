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

fact:
lw $v1, 0($t3)
sw $v1, -4($fp)
li $t0, 1
sw $t0, -8($fp)
lw $t1, -4($fp)
lw $t2, -8($fp)
beq $t1, $t2, label0
j label1
label0:
lw $t0, -4($fp)
move $v0, $t0
jr $ra
j label2
label1:
li $t0, 1
sw $t0, -12($fp)
lw $t1, -4($fp)
lw $t2, -12($fp)
sub $t0, $t1, $t2
sw $t0, -16($fp)
addi $sp, $sp, -4
lw $a0, -16($fp)
sw $a0, 0($sp)
move $t3, $sp
addi $sp, $sp, -8
sw $fp, 4($sp)
sw $ra, 0($sp)
move $fp, $sp
addi $sp, $sp, -24
jal fact
move $t0, $v0
move $sp, $fp
lw $fp, 4($sp)
lw $ra, 0($sp)
addi $sp, $sp, -8
addi $sp, $sp, 4
sw $t0, -20($fp)
lw $t1, -4($fp)
lw $t2, -20($fp)
mul $t0, $t1, $t2
sw $t0, -24($fp)
lw $t0, -24($fp)
move $v0, $t0
jr $ra
label2:
main:
move $t3, $sp
addi $sp, $sp, -8
sw $fp, 4($sp)
sw $ra, 0($sp)
move $fp, $sp
addi $sp, $sp, -28
move $t3, $sp
addi $sp, $sp, -8
sw $fp, 4($sp)
sw $ra, 0($sp)
move $fp, $sp
addi $sp, $sp, -64
jal read
move $sp, $fp
lw $fp, 4($sp)
lw $ra, 0($sp)
addi $sp, $sp, -8
addi $sp, $sp, 0
move $t0, $v0
sw $t0, -4($fp)
lw $t1, -4($fp)
move $t0, $t1
sw $t0, -8($fp)
li $t0, 1
sw $t0, -12($fp)
lw $t1, -8($fp)
lw $t2, -12($fp)
bgt $t1, $t2, label3
j label4
label3:
addi $sp, $sp, -4
lw $a0, -8($fp)
sw $a0, 0($sp)
move $t3, $sp
addi $sp, $sp, -8
sw $fp, 4($sp)
sw $ra, 0($sp)
move $fp, $sp
addi $sp, $sp, -24
jal fact
move $t0, $v0
move $sp, $fp
lw $fp, 4($sp)
lw $ra, 0($sp)
addi $sp, $sp, -8
addi $sp, $sp, 4
sw $t0, -16($fp)
lw $t1, -16($fp)
move $t0, $t1
sw $t0, -20($fp)
j label5
label4:
li $t0, 1
sw $t0, -24($fp)
lw $t1, -24($fp)
move $t0, $t1
sw $t0, -20($fp)
label5:
addi $sp, $sp, -4
lw $a0, -20($fp)
sw $a0, 0($sp)
move $t3, $sp
addi $sp, $sp, -8
sw $fp, 4($sp)
sw $ra, 0($sp)
move $fp, $sp
addi $sp, $sp, -64
lw $a0, 0($t3)
jal write
move $sp, $fp
lw $fp, 4($sp)
lw $ra, 0($sp)
addi $sp, $sp, -8
addi $sp, $sp, 4
li $t0, 0
sw $t0, -28($fp)
lw $t0, -28($fp)
move $v0, $t0
jr $ra
