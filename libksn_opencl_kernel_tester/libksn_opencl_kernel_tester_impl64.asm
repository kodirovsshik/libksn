
memcpy PROTO

.code

?call_kernel@ksn_opencl_kernel_tester@@YA_NP6AXXZAEBUarguments_adapter_t@1@@Z proc
	push rbp
	mov rbp, rsp

	mov r9, rcx ;Save to non-volatile register
	mov r10, rdx
	mov r11, r8

	sub rsp, r8
	
	mov rcx, rsp
	call memcpy
	
	;r8 = function
	;r9 = params data




	mov rsp, rbp
	pop rbp
	ret
?call_kernel@ksn_opencl_kernel_tester@@YA_NP6AXXZAEBUarguments_adapter_t@1@@Z endp

end
