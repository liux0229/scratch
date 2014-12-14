	.file	"signal.cpp"
	.section	.text.unlikely,"ax",@progbits
	.type	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc.part.1, @function
_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc.part.1:
.LFB1474:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	(%rdi), %rax
	addq	-24(%rax), %rdi
	movl	32(%rdi), %esi
	orl	$1, %esi
	call	_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate
	popq	%rdx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1474:
	.size	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc.part.1, .-_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc.part.1
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"diff = "
.LC1:
	.string	"i = "
	.text
	.p2align 4,,15
	.globl	_Z7handleri
	.type	_Z7handleri, @function
_Z7handleri:
.LFB1309:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movl	$7, %edx
	movl	$.LC0, %esi
	movl	%edi, %r12d
	movl	$_ZSt4cout, %edi
	pushq	%rbp
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	subq	$16, %rsp
	.cfi_def_cfa_offset 48
	movq	pa(%rip), %rbx
	leaq	8(%rsp), %rax
	subq	%rax, %rbx
	sarq	$3, %rbx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%rbx, %rsi
	movl	$_ZSt4cout, %edi
	call	_ZNSo9_M_insertIlEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L7
	cmpb	$0, 56(%rbx)
	je	.L5
	movzbl	67(%rbx), %eax
.L6:
	movsbl	%al, %esi
	movq	%rbp, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	movq	pa(%rip), %rax
	movl	$_ZSt4cout, %edi
	movq	32(%rax), %rsi
	call	_ZNSo9_M_insertIPKvEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L7
	cmpb	$0, 56(%rbx)
	je	.L8
	movzbl	67(%rbx), %eax
.L9:
	movsbl	%al, %esi
	movq	%rbp, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	movq	pa(%rip), %rax
	movl	$4, %edx
	movl	$.LC1, %esi
	movl	$_ZSt4cout, %edi
	movq	8(%rax), %rbx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movl	$_ZSt4cout, %edi
	movq	%rbx, %rsi
	call	_ZNSo9_M_insertIlEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L7
	cmpb	$0, 56(%rbx)
	je	.L10
	movzbl	67(%rbx), %eax
.L11:
	movq	%rbp, %rdi
	movsbl	%al, %esi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	movq	pa(%rip), %rdx
	movq	8(%rdx), %rax
	movq	%rax, observed(%rip)
	xorl	%eax, %eax
	cmpl	$2, %r12d
	setne	%al
	addq	$1, %rax
	movq	%rax, (%rdx)
	addq	$16, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 32
	popq	%rbx
	.cfi_def_cfa_offset 24
	popq	%rbp
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L5:
	.cfi_restore_state
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L6
	.p2align 4,,10
	.p2align 3
.L8:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L9
	.p2align 4,,10
	.p2align 3
.L10:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L11
.L7:
	call	_ZSt16__throw_bad_castv
	.cfi_endproc
.LFE1309:
	.size	_Z7handleri, .-_Z7handleri
	.p2align 4,,15
	.globl	_Z13installSignalv
	.type	_Z13installSignalv, @function
_Z13installSignalv:
.LFB1310:
	.cfi_startproc
	subq	$168, %rsp
	.cfi_def_cfa_offset 176
	xorl	%eax, %eax
	movl	$19, %ecx
	movq	%rsp, %rdi
	rep stosq
	leaq	8(%rsp), %rdi
	movq	$_Z7handleri, (%rsp)
	call	sigemptyset
	movq	%rsp, %rsi
	xorl	%edx, %edx
	movl	$2, %edi
	call	sigaction
	movq	%rsp, %rsi
	xorl	%edx, %edx
	movl	$15, %edi
	call	sigaction
	addq	$168, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1310:
	.size	_Z13installSignalv, .-_Z13installSignalv
	.section	.rodata.str1.1
.LC2:
	.string	"a="
.LC3:
	.string	"read returned: "
.LC4:
	.string	"error: "
.LC5:
	.string	"After signal: i="
.LC7:
	.string	"total: "
.LC8:
	.string	" equal: "
.LC9:
	.string	"%\n"
	.text
	.p2align 4,,15
	.globl	_Z1fv
	.type	_Z1fv, @function
_Z1fv:
.LFB1311:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	movl	$_ZSt4cout, %edi
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	subq	$1240, %rsp
	.cfi_def_cfa_offset 1280
	movq	%fs:40, %rax
	movq	%rax, 1224(%rsp)
	xorl	%eax, %eax
	leaq	16(%rsp), %rax
	movq	$0, 16(%rsp)
	movq	$0, 24(%rsp)
	movq	%rax, %rsi
	movq	%rax, pa(%rip)
	call	_ZNSo9_M_insertIPKvEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L26
	movzbl	67(%rbx), %eax
.L27:
	movsbl	%al, %esi
	movq	%rbp, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	leaq	32(%rsp), %rdi
	xorl	%eax, %eax
	movl	$19, %ecx
	rep stosq
	leaq	40(%rsp), %rdi
	movq	$_Z7handleri, 32(%rsp)
	call	sigemptyset
	leaq	32(%rsp), %rsi
	xorl	%edx, %edx
	movl	$2, %edi
	call	sigaction
	leaq	32(%rsp), %rsi
	xorl	%edx, %edx
	movl	$15, %edi
	call	sigaction
	movl	$_ZSt4cout, %edi
#APP
# 51 "signal.cpp" 1
	call L1; L1: addq %rsi, %rsi
# 0 "" 2
#NO_APP
	call	_ZNSo9_M_insertIPKvEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L29
	movzbl	67(%rbx), %eax
.L30:
	movsbl	%al, %esi
	movq	%rbp, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	movl	$2, %edx
	movl	$.LC2, %esi
	movl	$_ZSt4cout, %edi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	leaq	16(%rsp), %rsi
	movl	$_ZSt4cout, %edi
	call	_ZNSo9_M_insertIPKvEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L31
	movzbl	67(%rbx), %eax
.L32:
	movsbl	%al, %esi
	movq	%rbp, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	leaq	192(%rsp), %rsi
	xorl	%edi, %edi
	movl	$1024, %edx
	call	read
	movl	$15, %edx
	movq	%rax, %r12
	movl	$.LC3, %esi
	movl	$_ZSt4cout, %edi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%r12, %rsi
	movl	$_ZSt4cout, %edi
	call	_ZNSo9_M_insertIlEERSoT_
	movq	%rax, %rbp
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%rbp,%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L33
	movzbl	67(%rbx), %eax
.L34:
	movsbl	%al, %esi
	movq	%rbp, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	testq	%r12, %r12
	js	.L74
.L35:
	xorl	%r12d, %r12d
	xorl	%ebp, %ebp
	.p2align 4,,10
	.p2align 3
.L41:
	movq	24(%rsp), %rax
	addq	$1, %rax
	movq	%rax, 24(%rsp)
	movq	16(%rsp), %rax
	testq	%rax, %rax
	je	.L41
	movl	$2, %edx
	movl	$.LC2, %esi
	movl	$_ZSt4cout, %edi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	leaq	16(%rsp), %rsi
	movl	$_ZSt4cout, %edi
	call	_ZNSo9_M_insertIPKvEERSoT_
	movq	%rax, %r13
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%r13,%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L42
	movzbl	67(%rbx), %eax
.L43:
	movsbl	%al, %esi
	movq	%r13, %rdi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	movq	16(%rsp), %rax
	cmpq	$1, %rax
	je	.L75
	movq	16(%rsp), %rax
	cmpq	$2, %rax
	je	.L48
.L47:
	movq	$0, 16(%rsp)
	jmp	.L41
	.p2align 4,,10
	.p2align 3
.L42:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L43
	.p2align 4,,10
	.p2align 3
.L75:
	movq	24(%rsp), %rbx
	movl	$16, %edx
	movl	$.LC5, %esi
	movl	$_ZSt4cout, %edi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%rbx, %rsi
	movl	$_ZSt4cout, %edi
	call	_ZNSo9_M_insertIlEERSoT_
	movq	%rax, %r13
	movq	(%rax), %rax
	movq	-24(%rax), %rax
	movq	240(%r13,%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L45
	movzbl	67(%rbx), %eax
.L46:
	movsbl	%al, %esi
	movq	%r13, %rdi
	addq	$1, %rbp
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	movq	24(%rsp), %rdx
	movq	observed(%rip), %rax
	cmpq	%rax, %rdx
	sete	%al
	movzbl	%al, %eax
	addq	%rax, %r12
	jmp	.L47
.L48:
	testq	%r12, %r12
	js	.L50
	cvtsi2sdq	%r12, %xmm0
.L51:
	testq	%rbp, %rbp
	mulsd	.LC6(%rip), %xmm0
	js	.L52
	cvtsi2sdq	%rbp, %xmm1
.L53:
	divsd	%xmm1, %xmm0
	movl	$7, %edx
	movl	$.LC7, %esi
	movl	$_ZSt4cout, %edi
	movsd	%xmm0, 8(%rsp)
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	%rbp, %rsi
	movl	$_ZSt4cout, %edi
	call	_ZNSo9_M_insertImEERSoT_
	movl	$8, %edx
	movl	$.LC8, %esi
	movq	%rax, %rdi
	movq	%rax, %rbx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movsd	8(%rsp), %xmm0
	movq	%rbx, %rdi
	call	_ZNSo9_M_insertIdEERSoT_
	movl	$2, %edx
	movl	$.LC9, %esi
	movq	%rax, %rdi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	movq	1224(%rsp), %rax
	xorq	%fs:40, %rax
	jne	.L76
	addq	$1240, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
.L45:
	.cfi_restore_state
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L46
.L26:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L27
.L33:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L34
.L31:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L32
.L29:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L30
.L52:
	movq	%rbp, %rax
	movq	%rbp, %rdx
	shrq	%rax
	andl	$1, %edx
	orq	%rdx, %rax
	cvtsi2sdq	%rax, %xmm1
	addsd	%xmm1, %xmm1
	jmp	.L53
.L50:
	movq	%r12, %rax
	andl	$1, %r12d
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L51
.L74:
	call	__errno_location
	movl	(%rax), %edi
	call	strerror
	movl	$7, %edx
	movq	%rax, %rbx
	movl	$.LC4, %esi
	movl	$_ZSt4cout, %edi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
	testq	%rbx, %rbx
	je	.L77
	movq	%rbx, %rdi
	call	strlen
	movq	%rbx, %rsi
	movq	%rax, %rdx
	movl	$_ZSt4cout, %edi
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l
.L37:
	movq	_ZSt4cout(%rip), %rax
	movq	-24(%rax), %rax
	movq	_ZSt4cout+240(%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L38
	movzbl	67(%rbx), %eax
.L39:
	movsbl	%al, %esi
	movl	$_ZSt4cout, %edi
	call	_ZNSo3putEc
	movq	%rax, %rdi
	call	_ZNSo5flushEv
	jmp	.L35
.L38:
	movq	%rbx, %rdi
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L39
.L77:
	movq	_ZSt4cout(%rip), %rax
	movq	-24(%rax), %rdi
	addq	$_ZSt4cout, %rdi
	movl	32(%rdi), %esi
	orl	$1, %esi
	call	_ZNSt9basic_iosIcSt11char_traitsIcEE5clearESt12_Ios_Iostate
	jmp	.L37
.L28:
	call	_ZSt16__throw_bad_castv
.L76:
	.p2align 4,,8
	call	__stack_chk_fail
	.cfi_endproc
.LFE1311:
	.size	_Z1fv, .-_Z1fv
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB1312:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	call	_Z1fv
	xorl	%eax, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1312:
	.size	main, .-main
	.p2align 4,,15
	.type	_GLOBAL__sub_I_pa, @function
_GLOBAL__sub_I_pa:
.LFB1472:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$_ZStL8__ioinit, %edi
	call	_ZNSt8ios_base4InitC1Ev
	movl	$__dso_handle, %edx
	movl	$_ZStL8__ioinit, %esi
	movl	$_ZNSt8ios_base4InitD1Ev, %edi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	__cxa_atexit
	.cfi_endproc
.LFE1472:
	.size	_GLOBAL__sub_I_pa, .-_GLOBAL__sub_I_pa
	.section	.init_array,"aw"
	.align 8
	.quad	_GLOBAL__sub_I_pa
	.globl	observed
	.bss
	.align 8
	.type	observed, @object
	.size	observed, 8
observed:
	.zero	8
	.globl	pa
	.align 16
	.type	pa, @object
	.size	pa, 8
pa:
	.zero	8
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC6:
	.long	0
	.long	1079574528
	.hidden	__dso_handle
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
