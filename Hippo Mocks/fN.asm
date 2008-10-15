.586
.MODEL FLAT, SYSCALL 
.CODE

EXTERN ?base_func@base_mock@@QAEP81@AEXXZH@Z:PROC

?f0@base_mock@@QAEXXZ PROC
	push 0
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f0@base_mock@@QAEXXZ ENDP

?f1@base_mock@@QAEXXZ PROC
	push 1
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f1@base_mock@@QAEXXZ ENDP

?f2@base_mock@@QAEXXZ PROC
	push 2
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f2@base_mock@@QAEXXZ ENDP

?f3@base_mock@@QAEXXZ PROC
	push 3
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f3@base_mock@@QAEXXZ ENDP

?f4@base_mock@@QAEXXZ PROC
	push 4
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f4@base_mock@@QAEXXZ ENDP

?f5@base_mock@@QAEXXZ PROC
	push 5
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f5@base_mock@@QAEXXZ ENDP

?f6@base_mock@@QAEXXZ PROC
	push 6
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f6@base_mock@@QAEXXZ ENDP

?f7@base_mock@@QAEXXZ PROC
	push 7
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f7@base_mock@@QAEXXZ ENDP

?f8@base_mock@@QAEXXZ PROC
	push 8
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f8@base_mock@@QAEXXZ ENDP

?f9@base_mock@@QAEXXZ PROC
	push 9
	call ?base_func@base_mock@@QAEP81@AEXXZH@Z
	jmp eax
?f9@base_mock@@QAEXXZ ENDP

END

