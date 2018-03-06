#include "system.h"
#include "screen.h"

.global set_color
.global clear_screen
.global get_color
.global set_cursor
.global put_char
.global puts
.globl divide
.globl mistery

.data
	chr_attr: .byte 0
	cursor_row: .byte 0
	cursor_col: .byte 0
	text: .byte 'G', 'd', 'k', 'k', 'n', 'w', 'V', 'n', 'q', 'k', 'c', 0

.text

set_color:
  la $t0, 0($a0)
	sll $t0, $a1, 4
	andi $t1, $a0, 0x0f
	or $t0, $t0, $t1
	lui $t1, %hi(chr_attr)
	sb $t0, %lo(chr_attr)($t1)
	jr $ra

get_cursor:
	lui $t1, %hi(cursor_row)
	lb $t0, %lo(cursor_row)($t1)
	sb $t0, 0($a0)
	lui $t1, %hi(cursor_col)
	lb $t0, %lo(cursor_col)($t1)
	sb $t0, 0($a1)

get_color:
	lui $t1, %hi(chr_attr)
	lb $t0, %lo(chr_attr)($t1)
	andi $t1, $t0, 0x0f
	sb $t1, 0($a0)
	srl $t1, $t0, 4
	sb $t1, 0($a1)
	jr $ra

set_cursor:
	li $t0, (MAX_ROWS - 1)
	sltu $t0, $t0, $a0
	bnez $t0, set_cursor_end
	li $t0, (MAX_COLS - 1)
	sltu $t0, $t0, $a1
	bnez $t0, set_cursor_end

	lui $t0, %hi(cursor_row)
	sb $a0, %lo(cursor_row)($t0)
	lui $t0, %hi(cursor_col)
	sb $a1, %lo(cursor_col)($t0)

set_cursor_end:
	jr $ra

put_char:
	li $t0, 13
	bne $t0, $a0, not_eol
	lui $t1, %hi(cursor_row)
	lbu $t1, %lo(cursor_row)($t1)
	slti $t2, $t1, (MAX_ROWS-1)
	beqz $t2, not_less
	addiu $t1, $t1, 1
	lui $t2, %hi(cursor_row)
	sb $t1, %lo(cursor_row)($t2)
not_less:
	lui $t2, %hi(cursor_col)
	sb $zero, %lo(cursor_col)($t2)
	j put_char_end
not_eol:
	lui $t0, %hi(cursor_row)
	lbu $t1, %lo(cursor_row)($t0)
	sll $t2, $t1, 6
	sll $t3, $t1, 4
	addu $t2, $t2, $t3
	lui $t0, %hi(cursor_col)
	lb $t1, %lo(cursor_col)($t0)
	addu $t2, $t2, $t1
	sll $t2, $t2, 1
	lui $t0, %hi(chr_attr)
	lb $t1, %lo(chr_attr)($t0)
	sll $t1, $t1, 8
	or $t1, $a0, $t1
	li $t0, VGA_START_ADDR
	addu $t2, $t2, $t0
	sh $t1, 0($t2)

	lui $t0, %hi(cursor_col)
	lb $t1, %lo(cursor_col)($t0)

	sltiu $t2, $t1, (MAX_COLS - 1)
	beqz $t2, not_within_col
	addiu $t1, $t1, 1
	sb $t1, %lo(cursor_col)($t0)
	j put_char_end

not_within_col:
	sb $zero, %lo(cursor_col)($t0)
	lui $t0, %hi(cursor_row)
	lbu $t1, %lo(cursor_row)($t0)

	addiu $t1, $t1, 1
	sb $t1, %lo(cursor_row)($t0)

put_char_end:
	jr $ra

mistery:
  li $t0, 0xb800
    la $t1, text
m_loop:
    lb $t3, 0($t1)
    beqz $t3, m_end
    li $t2, 0xd0
    sll $t2, $t2, 8

    or $t2, $t2, $t3
    sh $t2, 0($t0)
    addiu $t0, $t0, 160
    addiu $t1, $t1, 1
    j m_loop

m_end:
    jr $ra

clear_screen:
	li $t0, VGA_START_ADDR
	li $t1, VGA_END_ADDR

	lui $t2, %hi(chr_attr)
	lbu $t2, %lo(chr_attr)($t2)
	sll $t2, $t2, 8
	ori $t2, $t2, 0x20

.clear_screen_loop:
	sltu $t3, $t0, $t1
	beqz $t3, .clear_screen_end
	sh $t2, 0($t0)
	addiu $t0, $t0, 2
	j .clear_screen_loop

.clear_screen_end:
	jr $ra

puts:
	move $t4, $a0
	lb $a0, 0($t4)
	beqz $a0, puts_end
	addiu $sp, $sp, -4
	sw $ra, 0($sp)

not_eow:
	jal put_char
	addiu $t4, $t4, 1
	lb $a0, 0($t4)
	bnez $a0, not_eow
	lw $ra, 0($sp)
	addiu $sp, $sp, 4

puts_end:
	jr $ra

divide:
	move $t1, $zero
divide_loop:
	sltu $t0, $a0, $a1
	bnez $t0, divide_end
	subu $a0, $a0, $a1
	addiu $t1, $t1, 1
	j divide_loop

divide_end:
	sw $t1, 0($a2)
	sw $a0, 0($a3)
	jr $ra
