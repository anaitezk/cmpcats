	.text
	.globl _start	
												
_start:

                    li $t0,0
                    la $a0,b0
                    l.s $f1,($a0)

                    j loop

Exit:				li 		$v0, 10
                    syscall				#au revoir...


loop:               la $a0, insert
                    jal print_str

                    jal read_float
                    mov.s $f12,$f0				#moving float to $f12

                    jal print_float
                    jal print_endl

                    c.eq.s $f0,$f1
                    bc1t Exit

                    mov.s $f2,$f0

                    cvt.w.s $f2,$f2

                    mfc1 $t1,$f2        #mfc1, mtc1 apo int se float ===> t1 = float converted to int
                    mtc1 $t1,$f3        #f3 = int converted to float

                    cvt.s.w $f3,$f3
                    c.eq.s $f0,$f3

                    bc1t true


false:              la $a0,no
                    jal print_str
                    jal print_endl

                    j loop
                    


true:               la $a0, yes
                    jal print_str

                    move $a0,$t1 
                    jal print_int
                    jal print_endl

                    j loop


print_str:			li $v0, 4
					syscall
					jr $ra

print_endl:		    la		$a0,endl 			# system call to print
					li		$v0, 4 				# out a newline
					syscall
					jr $ra

print_int:			li $v0, 1
					syscall
					jr $ra

print_float:	    li $v0, 2
					syscall
					jr $ra

read_float:		    li		$v0, 6				
					syscall
					jr $ra
					




.data
endl: 					.asciiz 	"\n"
b0:                     .float      0.0
yes:                    .asciiz     "FOUND-INTEGER =  "
no:                     .asciiz     "NOT-INTEGER"
insert:                 .asciiz     "Insert a float!  "