#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#define INSTNUMQ  7
#define MAXLENQ   7
#define BUFFSIZE  256

void hexDump (char *desc, void *addr, int len);

uint8_t buff[INSTNUMQ][MAXLENQ]={{0x61, 0x64, 0x64,    0,    0,    0,    0},
			      {0x73, 0x75, 0x62,    0,    0,    0,    0}, 
			      {0x52, 0x31,    0,    0,    0,    0,    0},
			      {0x52, 0x32,    0,    0,    0,    0,    0},
			      {0x6c, 0x6f, 0x61, 0x64,    0,    0,    0},
			      {0x73, 0x74, 0x6f, 0x72, 0x65,    0,    0},
			      {0x4d, 0x5b, 0x00, 0x00, 0x00, 0x00, 0x5d}};
unsigned int bin[INSTNUMQ]={3, 3, 2, 2, 4, 5, 7};
uint8_t outchar[INSTNUMQ] = {0x09, 0x0a, 0x01, 0x02, 0x03, 0x04, 0x05}; //translations

int main(void){

  uint8_t input[BUFFSIZE], output[BUFFSIZE];
  FILE *fp, *fo;
  int i=0, halt=0;
  unsigned int inputindex=0, outputindex=0;
  uint8_t consuming=0x0, space;
  int compresult=0;
  //********Initialize i/o arrays, read input array from input file***************
  for(i=0; i<BUFFSIZE; i++){
    input[i]=0;
    output[i]=0;
  }
  fp = fopen("./source.hex", "r");
  fo = fopen("./machine.hex","w");
  if(fp == NULL) { 
    printf("cannot open input file\n"); exit(1);
  }
  if(fo == NULL) { 
    printf("cannot open output file\n"); exit(1);
  }

  fread((void*) input, sizeof(uint8_t), BUFFSIZE, fp); // place input file into a memory buffer. 

  
  for(i=0; i< INSTNUMQ; i++){
    printf("%d\n", i); 
    compresult = memcmp((void*) buff[i], (void*) (input+inputindex), sizeof(uint8_t)*bin[i]);
    //printf("comporesult = %d \n", compresult);
    if((i == 6) && (input[inputindex]== 0x4d) && (input[inputindex+1] == 0x5b) && (input[inputindex+6] == 0x5d)){
      compresult =0;
    }
						  
    
    if(compresult==0){
      printf("equalty i found: %d\n", i); 
      output[outputindex++] = outchar[i]; //put the machine code into output buffer for this instruction

      if(i==6){
	memcpy((void*) (output+outputindex), (void *) (input+inputindex+2), 4*sizeof(uint8_t)); //copy mem addresses as such.
	outputindex += 4;
      }

      inputindex += bin[i];             //Get ready for reading next part of input buffer.
      while(input[inputindex] == 0x20 || input[inputindex] == 0x0a){
	inputindex++;
      } // filter white spaces and line feeds. 

      i=-1; // do the same again because a pattron has been found.
    }//endif
    
  }//for loop ends here.
  printf("%d\n", i); 
  printf("Input index: %u, output index: %u\n", inputindex, outputindex);
  hexDump("Input:", input, BUFFSIZE);  
  hexDump("Output", output, BUFFSIZE);
  fclose(fp);
  fclose(fo);
} // Main ends here

void hexDump (char *desc, void *addr, int len) {
  int i;
  unsigned char buff[17];
  unsigned char *pc = (unsigned char*)addr;

  // Output description if given.
  if (desc != NULL)
    printf ("%s:\n", desc);

  // Process every byte in the data.
  for (i = 0; i < len; i++) {
    // Multiple of 16 means new line (with line offset).

    if ((i % 16) == 0) {
      // Just don't print ASCII for the zeroth line.
      if (i != 0)
	printf ("  %s\n", buff);

      // Output the offset.
      printf ("  %04x ", i);
    }

    // Now the hex code for the specific character.
    printf (" %02x", pc[i]);

    // And store a printable ASCII character for later.
    if ((pc[i] < 0x20) || (pc[i] > 0x7e))
      buff[i % 16] = '.';
    else
      buff[i % 16] = pc[i];
    buff[(i % 16) + 1] = '\0';
  }

  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0) {
    printf ("   ");
    i++;
  }

  // And print the final ASCII bit.
  printf ("  %s\n", buff);
}


