    .code64
	.global empty_handler_w, pit_handler_w
	.extern pit_handler
        
empty_handler_w:
	iretq

pit_handler_w:
	push %rax
	push %rcx
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11

	call pit_handler
	
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rcx
	pop %rax
	
	iretq
