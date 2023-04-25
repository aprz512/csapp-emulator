	.file	"stack.c"
	.text
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%d"
	.text
	.globl	main
	.type	main, @function
main:
.LFB23:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	a(%rip), %eax			a
	leal	(%rax,%rax,2), %edx		3a
	leal	(%rdx,%rax,4), %edx		7a
	leal	(%rdx,%rax,8), %edx		15a
	leaq	.LC0(%rip), %rsi
	movl	$1, %edi
	movl	$0, %eax

	call	__printf_chk@PLT
	movl	$0, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE23:
	.size	main, .-main
	.comm	a,4,4
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
