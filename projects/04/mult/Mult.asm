// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.

(START)
	@R2
	M=0			
	@R1
	D=M
	@R3
	M=D //copy the value of R1 to aoid changing it
(AGAIN)
	@R3
	D=M
	@END
	D;JEQ
	@R3
	M=M-1
	@R0
	D=M
	@R2
	M=D+M
	@AGAIN
	0;JMP
(END)
	@END
	0;JMP