.global Random

// TODO: Implement randomness with the HC-128 algorithm for systems that do not support the rdrand instruction
Random:
	rdrand %rax
	jnc Random
	ret
