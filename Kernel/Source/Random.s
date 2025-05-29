.global Random

Random:
	rdrand %rax
	jnc Random
	ret
