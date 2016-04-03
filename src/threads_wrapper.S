    .code64
	.global switch_threads, function_wrapper
	.extern unlock_with_interrupts, thread_exit

switch_threads:
	pushfq
	pushq %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movq %rsp, (%rdi)
	movq %rsi, %rsp

	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
	popfq

	ret

function_wrapper:
	popq %rdi
	callq unlock_with_interrupts
	popq %rdi
	popq %rax
	callq *%rax
	movq %rax, %rdi
	callq thread_exit
	