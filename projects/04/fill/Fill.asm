// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, the
// program clears the screen, i.e. writes "white" in every pixel.

// Put your code here.

(READKBD)
	@KBD
	D=M //read keyboard into D register
	@BLACKEN
	D;JNE
	@WHITEN
	0;JMP
(BLACKEN)
	@0
	D=A-1
	@R15
	M=D // set all target pixels black
	@SWEEP
	0;JMP
(WHITEN)
	@0
	D=A
	@R15
	M=D    //set all target pixels white
	@SWEEP
	0;JMP
(SWEEP)
	@8192
	D=A
	@remaining
	M=D // Stored number of remaing pixeles
(REPEAT)
	@remaining
	D=M  //load number of remaining pixels in register D
	@READKBD
	D;JEQ // if remaining pixels are zero jump to start
	@remaining
	M=M-1 // decrement the number of remaining pixels
	@remaining
	D=M
	@SCREEN
	D=D+A  // D now contains address of pixel to target next
	@R14
	M=D   // address of next pixel saved.
	@R15
	D=M   // Read value to paint pixel
	@R14
	A=M   //address read into address register. Everything read.
	M=D   // put the pixel value at target.
	@REPEAT
	0;JMP
	

	
	