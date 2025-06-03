.global RandomRDRAND

// TODO: Implement randomness with the HC-128 algorithm for systems that do not support the rdrand instruction
RandomRDRAND:
	rdrand %rax
	jnc Random
	ret
