	.file	"html-linker.c"
	.intel_syntax noprefix
	.section .rdata,"dr"
LC0:
	.ascii "\33[38;2;128;0;0mERROR: %s\12\0"
	.text
	.globl	_error
	.def	_error;	.scl	2;	.type	32;	.endef
_error:
LFB17:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	sub	esp, 24
	mov	eax, DWORD PTR [ebp+8]
	mov	DWORD PTR [esp+8], eax
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC0
	mov	eax, DWORD PTR __imp___iob
	add	eax, 64
	mov	DWORD PTR [esp], eax
	call	_fprintf
	mov	DWORD PTR [esp], 1
	call	_exit
	.cfi_endproc
LFE17:
	.section .rdata,"dr"
LC1:
	.ascii "Arguments parsed: \0"
LC2:
	.ascii "\11Input file: %s\12\0"
LC3:
	.ascii "\11Output file: %s\12\0"
	.text
	.globl	_print_args
	.def	_print_args;	.scl	2;	.type	32;	.endef
_print_args:
LFB18:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	sub	esp, 24
	mov	DWORD PTR [esp], OFFSET FLAT:LC1
	call	_puts
	mov	eax, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [eax]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC2
	call	_printf
	mov	eax, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [eax+4]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC3
	call	_printf
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE18:
	.section .rdata,"dr"
LC4:
	.ascii "Too few arguments.\0"
LC5:
	.ascii "i: %d\12\0"
LC6:
	.ascii "arg: %s\12\0"
LC7:
	.ascii "args[i]: %s\12\0"
LC8:
	.ascii "-o\0"
	.text
	.globl	_parse_args
	.def	_parse_args;	.scl	2;	.type	32;	.endef
_parse_args:
LFB19:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	sub	esp, 56
	cmp	DWORD PTR [ebp+12], 3
	jg	L4
	mov	DWORD PTR [esp], OFFSET FLAT:LC4
	call	_error
L4:
	mov	DWORD PTR [ebp-12], 1
	jmp	L5
L8:
	mov	eax, DWORD PTR [ebp-12]
	lea	edx, [0+eax*4]
	mov	eax, DWORD PTR [ebp+16]
	add	eax, edx
	mov	eax, DWORD PTR [eax]
	mov	DWORD PTR [ebp-16], eax
	mov	eax, DWORD PTR [ebp-12]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC5
	call	_printf
	mov	eax, DWORD PTR [ebp-16]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC6
	call	_printf
	mov	eax, DWORD PTR [ebp-12]
	lea	edx, [0+eax*4]
	mov	eax, DWORD PTR [ebp+16]
	add	eax, edx
	mov	eax, DWORD PTR [eax]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC7
	call	_printf
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC8
	mov	eax, DWORD PTR [ebp-16]
	mov	DWORD PTR [esp], eax
	call	_strcmp
	test	eax, eax
	jne	L6
	add	DWORD PTR [ebp-12], 1
	mov	eax, DWORD PTR [ebp-12]
	lea	edx, [0+eax*4]
	mov	eax, DWORD PTR [ebp+16]
	add	eax, edx
	mov	eax, DWORD PTR [eax]
	mov	DWORD PTR [ebp-24], eax
	jmp	L7
L6:
	mov	eax, DWORD PTR [ebp-16]
	mov	DWORD PTR [ebp-28], eax
L7:
	add	DWORD PTR [ebp-12], 1
L5:
	mov	eax, DWORD PTR [ebp-12]
	cmp	eax, DWORD PTR [ebp+12]
	jl	L8
	lea	eax, [ebp-28]
	mov	DWORD PTR [esp], eax
	call	_print_args
	mov	eax, DWORD PTR [ebp+8]
	mov	edx, DWORD PTR [ebp-28]
	mov	DWORD PTR [eax], edx
	mov	edx, DWORD PTR [ebp-24]
	mov	DWORD PTR [eax+4], edx
	mov	edx, DWORD PTR [ebp-20]
	mov	DWORD PTR [eax+8], edx
	mov	eax, DWORD PTR [ebp+8]
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE19:
	.globl	_inline_scripts_in_html
	.def	_inline_scripts_in_html;	.scl	2;	.type	32;	.endef
_inline_scripts_in_html:
LFB20:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	sub	esp, 16
	mov	eax, DWORD PTR [ebp+12]
	mov	DWORD PTR [ebp-4], eax
	nop
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE20:
	.def	___main;	.scl	2;	.type	32;	.endef
	.section .rdata,"dr"
LC9:
	.ascii "r\0"
	.align 4
LC10:
	.ascii "Memory error: calloc() returned NULL\0"
LC11:
	.ascii "w\0"
LC12:
	.ascii "Writing %d bytes to %s\12\0"
LC13:
	.ascii "Hello, World!\12%s\12\0"
	.align 4
LC14:
	.ascii "Successfully wrote %d bytes to %s\12\0"
	.text
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
LFB21:
	.cfi_startproc
	push	ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	mov	ebp, esp
	.cfi_def_cfa_register 5
	push	ebx
	and	esp, -16
	sub	esp, 64
	.cfi_offset 3, -12
	call	___main
	lea	eax, [esp+28]
	mov	edx, DWORD PTR [ebp+12]
	mov	DWORD PTR [esp+8], edx
	mov	edx, DWORD PTR [ebp+8]
	mov	DWORD PTR [esp+4], edx
	mov	DWORD PTR [esp], eax
	call	_parse_args
	mov	eax, DWORD PTR [esp+28]
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC9
	mov	DWORD PTR [esp], eax
	call	_fopen
	mov	DWORD PTR [esp+60], eax
	mov	DWORD PTR [esp+8], 2
	mov	DWORD PTR [esp+4], 0
	mov	eax, DWORD PTR [esp+60]
	mov	DWORD PTR [esp], eax
	call	_fseek
	mov	eax, DWORD PTR [esp+60]
	mov	DWORD PTR [esp], eax
	call	_ftell
	mov	DWORD PTR [esp+56], eax
	mov	DWORD PTR [esp+8], 0
	mov	DWORD PTR [esp+4], 0
	mov	eax, DWORD PTR [esp+60]
	mov	DWORD PTR [esp], eax
	call	_fseek
	mov	eax, DWORD PTR [esp+56]
	mov	DWORD PTR [esp+4], 1
	mov	DWORD PTR [esp], eax
	call	_calloc
	mov	DWORD PTR [esp+52], eax
	cmp	DWORD PTR [esp+52], 0
	jne	L12
	mov	DWORD PTR [esp], OFFSET FLAT:LC10
	call	_error
L12:
	mov	eax, DWORD PTR [esp+56]
	mov	edx, DWORD PTR [esp+60]
	mov	DWORD PTR [esp+12], edx
	mov	DWORD PTR [esp+8], eax
	mov	DWORD PTR [esp+4], 1
	mov	eax, DWORD PTR [esp+52]
	mov	DWORD PTR [esp], eax
	call	_fread
	mov	eax, DWORD PTR [esp+60]
	mov	DWORD PTR [esp], eax
	call	_fclose
	mov	eax, DWORD PTR [esp+52]
	mov	DWORD PTR [esp+4], eax
	mov	eax, DWORD PTR [esp+48]
	mov	DWORD PTR [esp], eax
	call	_strcpy
	mov	eax, DWORD PTR [esp+28]
	mov	DWORD PTR [esp+4], eax
	mov	eax, DWORD PTR [esp+32]
	mov	DWORD PTR [esp+8], eax
	mov	eax, DWORD PTR [esp+36]
	mov	DWORD PTR [esp+12], eax
	mov	eax, DWORD PTR [esp+48]
	mov	DWORD PTR [esp], eax
	call	_inline_scripts_in_html
	mov	DWORD PTR [esp+44], eax
	mov	eax, DWORD PTR [esp+32]
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC11
	mov	DWORD PTR [esp], eax
	call	_fopen
	mov	DWORD PTR [esp+40], eax
	mov	DWORD PTR [esp+8], 0
	mov	DWORD PTR [esp+4], 0
	mov	eax, DWORD PTR [esp+40]
	mov	DWORD PTR [esp], eax
	call	_fseek
	mov	eax, DWORD PTR [esp+32]
	mov	DWORD PTR [esp+8], eax
	mov	eax, DWORD PTR [esp+56]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC12
	call	_printf
	mov	eax, DWORD PTR [esp+44]
	mov	DWORD PTR [esp+8], eax
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC13
	mov	eax, DWORD PTR [esp+40]
	mov	DWORD PTR [esp], eax
	call	_fprintf
	mov	ebx, DWORD PTR [esp+32]
	mov	eax, DWORD PTR [esp+40]
	mov	DWORD PTR [esp], eax
	call	_ftell
	mov	DWORD PTR [esp+8], ebx
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], OFFSET FLAT:LC14
	call	_printf
	mov	eax, DWORD PTR [esp+40]
	mov	DWORD PTR [esp], eax
	call	_fclose
	mov	eax, 0
	mov	ebx, DWORD PTR [ebp-4]
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
LFE21:
	.ident	"GCC: (MinGW.org GCC-6.3.0-1) 6.3.0"
	.def	_fprintf;	.scl	2;	.type	32;	.endef
	.def	_exit;	.scl	2;	.type	32;	.endef
	.def	_puts;	.scl	2;	.type	32;	.endef
	.def	_printf;	.scl	2;	.type	32;	.endef
	.def	_strcmp;	.scl	2;	.type	32;	.endef
	.def	_fopen;	.scl	2;	.type	32;	.endef
	.def	_fseek;	.scl	2;	.type	32;	.endef
	.def	_ftell;	.scl	2;	.type	32;	.endef
	.def	_calloc;	.scl	2;	.type	32;	.endef
	.def	_fread;	.scl	2;	.type	32;	.endef
	.def	_fclose;	.scl	2;	.type	32;	.endef
	.def	_strcpy;	.scl	2;	.type	32;	.endef
