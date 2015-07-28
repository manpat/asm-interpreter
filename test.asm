	mov [0], 1
	mov [1], 1
	mov A, 1

go:
	mov B, A
	sub B, 1
	add A, 1
	mov [A], [B]
	add B, 1
	add [A], [B]

	mov D, A
	div D, 5
	add IP, D
jmp go
	debug

	mov B, blah
	print B

	xor B, B
	print B

	or B, A
	and B, 111b
	shr B, 1
	or B, A
	print B

	print FLAGS
	cmp B, 5
	print FLAGS

blah: