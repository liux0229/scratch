	.file	"signal.cpp"
	.section	.text.unlikely,"ax",@progbits
	.type	std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*) [clone .part.1], @function
std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*) [clone .part.1]:
.LFB1473:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	movq	(%rdi), %rax
	addq	-24(%rax), %rdi
	movl	32(%rdi), %esi
	orl	$1, %esi
	call	std::basic_ios<char, std::char_traits<char> >::clear(std::_Ios_Iostate)
	popq	%rdx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1473:
	.size	std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*) [clone .part.1], .-std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*) [clone .part.1]
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"diff = "
.LC1:
	.string	"i = "
	.text
	.p2align 4,,15
	.globl	handler(int)
	.type	handler(int), @function
handler(int):
.LFB1308:
	.cfi_startproc
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movl	$7, %edx
	movl	$.LC0, %esi
	movl	%edi, %r12d
	movl	std::cout, %edi
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
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movq	%rbx, %rsi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<long>(long)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
	movq	pa(%rip), %rax
	movl	std::cout, %edi
	movq	32(%rax), %rsi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<void const*>(void const*)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
	movq	pa(%rip), %rax
	movl	$4, %edx
	movl	$.LC1, %esi
	movl	std::cout, %edi
	movq	8(%rax), %rbx
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movl	std::cout, %edi
	movq	%rbx, %rsi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<long>(long)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
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
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L6
	.p2align 4,,10
	.p2align 3
.L8:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L9
	.p2align 4,,10
	.p2align 3
.L10:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L11
.L7:
	call	std::__throw_bad_cast()
	.cfi_endproc
.LFE1308:
	.size	handler(int), .-handler(int)
	.p2align 4,,15
	.globl	installSignal()
	.type	installSignal(), @function
installSignal():
.LFB1309:
	.cfi_startproc
	subq	$168, %rsp
	.cfi_def_cfa_offset 176
	xorl	%eax, %eax
	movl	$19, %ecx
	movq	%rsp, %rdi
	rep stosq
	leaq	8(%rsp), %rdi
	movq	handler(int), (%rsp)
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
.LFE1309:
	.size	installSignal(), .-installSignal()
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
	.globl	f()
	.type	f(), @function
f():
.LFB1310:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	movl	std::cout, %edi
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
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<void const*>(void const*)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
	leaq	32(%rsp), %rdi
	xorl	%eax, %eax
	movl	$19, %ecx
	rep stosq
	leaq	40(%rsp), %rdi
	movq	handler(int), 32(%rsp)
	call	sigemptyset
	leaq	32(%rsp), %rsi
	xorl	%edx, %edx
	movl	$2, %edi
	call	sigaction
	leaq	32(%rsp), %rsi
	xorl	%edx, %edx
	movl	$15, %edi
	call	sigaction
	movl	std::cout, %edi
#APP
# 51 "signal.cpp" 1
	call L1; L1: addq %rsi, %rsi
# 0 "" 2
#NO_APP
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<void const*>(void const*)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
	movq	16(%rsp), %rbx
	movl	$2, %edx
	movl	$.LC2, %esi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movq	%rbx, %rsi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<void const*>(void const*)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
	leaq	192(%rsp), %rsi
	xorl	%edi, %edi
	movl	$1024, %edx
	call	read
	movl	$15, %edx
	movq	%rax, %r12
	movl	$.LC3, %esi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movq	%r12, %rsi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<long>(long)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
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
	movq	16(%rsp), %rbx
	movl	$2, %edx
	movl	$.LC2, %esi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movq	%rbx, %rsi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<void const*>(void const*)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
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
	call	std::ctype<char>::_M_widen_init() const
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
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movq	%rbx, %rsi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<long>(long)
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
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
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
	movl	std::cout, %edi
	movsd	%xmm0, 8(%rsp)
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movq	%rbp, %rsi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<unsigned long>(unsigned long)
	movl	$8, %edx
	movl	$.LC8, %esi
	movq	%rax, %rdi
	movq	%rax, %rbx
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	movsd	8(%rsp), %xmm0
	movq	%rbx, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<double>(double)
	movl	$2, %edx
	movl	$.LC9, %esi
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
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
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L46
.L26:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L27
.L33:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L34
.L31:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L32
.L29:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
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
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
	testq	%rbx, %rbx
	je	.L77
	movq	%rbx, %rdi
	call	strlen
	movq	%rbx, %rsi
	movq	%rax, %rdx
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, long)
.L37:
	movq	std::cout(%rip), %rax
	movq	-24(%rax), %rax
	movq	std::cout+240(%rax), %rbx
	testq	%rbx, %rbx
	je	.L28
	cmpb	$0, 56(%rbx)
	je	.L38
	movzbl	67(%rbx), %eax
.L39:
	movsbl	%al, %esi
	movl	std::cout, %edi
	call	std::basic_ostream<char, std::char_traits<char> >::put(char)
	movq	%rax, %rdi
	call	std::basic_ostream<char, std::char_traits<char> >::flush()
	jmp	.L35
.L38:
	movq	%rbx, %rdi
	call	std::ctype<char>::_M_widen_init() const
	movq	(%rbx), %rax
	movl	$10, %esi
	movq	%rbx, %rdi
	call	*48(%rax)
	jmp	.L39
.L77:
	movq	std::cout(%rip), %rax
	movq	-24(%rax), %rdi
	addq	std::cout, %rdi
	movl	32(%rdi), %esi
	orl	$1, %esi
	call	std::basic_ios<char, std::char_traits<char> >::clear(std::_Ios_Iostate)
	jmp	.L37
.L28:
	call	std::__throw_bad_cast()
.L76:
	.p2align 4,,8
	call	__stack_chk_fail
	.cfi_endproc
.LFE1310:
	.size	f(), .-f()
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB1311:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	call	f()
	xorl	%eax, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE1311:
	.size	main, .-main
	.p2align 4,,15
	.type	_GLOBAL__sub_I_pa, @function
_GLOBAL__sub_I_pa:
.LFB1471:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	std::__ioinit, %edi
	call	std::ios_base::Init::Init()
	movl	$__dso_handle, %edx
	movl	std::__ioinit, %esi
	movl	std::ios_base::Init::~Init(), %edi
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	__cxa_atexit
	.cfi_endproc
.LFE1471:
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
	.local	std::__ioinit
	.comm	std::__ioinit,1,1
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC6:
	.long	0
	.long	1079574528
	.hidden	__dso_handle
	.ident	"GCC: (Ubuntu/Linaro 4.8.1-10ubuntu9) 4.8.1"
	.section	.note.GNU-stack,"",@progbits
