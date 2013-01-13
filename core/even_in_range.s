/*
// ************************************************************************************************
//	__even_in_range (Pseudo code)
//
//	If __value is not even or out of range, then return 0.
//
//	This code assumes that the two paremeters are passed in R12 and R14
//	and the return is passed back in R12.
// ************************************************************************************************
*/

	.name __even_in_range
	.global __even_in_range
	.text 
__even_in_range:
	bit.b #0, r12				;// Test:	__value is even?
	jnz NotValid				;// If so:	goto NotValid
	cmp.b r12, r14				;// Test:	__value in range?
	jlo Exit					;// If so:	goto Exit
NotValid:
	mov.b #0, r12				;// Set:	r12 = 0
Exit:
	ret							;// Return: leave passed paremeter in r12
