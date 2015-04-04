#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#define BUFFSIZE 256

void hexDump (char *desc, void *addr, int len);


int main(void){
  uint8_t input[BUFFSIZE], output[BUFFSIZE];
  FILE *fp, *fo;
  int i=0, halt=0;
  int inputindex=0, outputindex=0;
  uint8_t consuming=0x0;
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

  fread((void*) input, sizeof(uint8_t), BUFFSIZE, fp);  
  hexDump("hex dupmp:", input, BUFFSIZE);
  //********** Done initilization ************
  
  consuming=input[inputindex];
  while(1){
   
    if(consuming == 0x20) { consuming = input[++inputindex]; continue; }
    if(consuming == 0x0a) { consuming = input[++inputindex]; continue; }
    // *** Consuming 6c
    if(consuming == 0x6c){
      //printf("consumed 0x6c\n");	
      consuming = input[++inputindex];
      if(consuming == 0x6f) consuming = input[++inputindex];
      if(consuming == 0x61) consuming = input[++inputindex];
      if(consuming == 0x64) consuming = input[++inputindex];
      if(consuming == 0x20 || consuming == 0x0a){
    //success
      consuming = input[++inputindex];
      output[outputindex]= 0x03;
      outputindex++;
      continue;
      }else break;
    }
    // *** Consuming 61
    if(consuming == 0x61){
      //printf("consumed 0x61\n");	
      consuming = input[++inputindex];
      if(consuming == 0x64) consuming = input[++inputindex];
      if(consuming == 0x64) consuming = input[++inputindex];
      if(consuming == 0x20 || consuming == 0x0a){
    //success
      consuming = input[++inputindex];
      output[outputindex]= 0x09;
      outputindex++;
      continue;
      }else break;
    }
    //** Consuming 52
    if(consuming == 0x52){
      //printf("consumed 0x52\n");	      
      consuming = input[++inputindex];      
      if(consuming == 0x31){ 
	consuming = input[++inputindex];
	if(consuming == 0x20 || consuming == 0x0a){
	  //success
	  consuming = input[++inputindex];
	  output[outputindex]= 0x01;
	  outputindex++;
	  continue;
	} else break;
      } // end of consuming 52{31}
      if(consuming == 0x32){
	//printf("consumed 0x32\n");	
	consuming = input[++inputindex];
	if(consuming == 0x20 || consuming == 0x0a){
	  //success
      consuming = input[++inputindex];
      output[outputindex]= 0x02;
      outputindex++;
      continue;
      }else break;
      }// end of consuming 52{32}
    } //  end of consuming 52
    
    //*** Consuming 73
    

      if(consuming == 0x73){
	//printf("consumed 0x73\n");	
      consuming = input[++inputindex];
	if(consuming == 0x74){
	  //printf("consumed 0x74\n");	
	  consuming = input[++inputindex];
	  if(consuming == 0x6f) consuming = input[++inputindex];
	  if(consuming == 0x72) consuming = input[++inputindex];
	  if(consuming == 0x65) consuming = input[++inputindex];
	  if(consuming == 0x20 || consuming == 0x0a){
	    //success
	    consuming = input[++inputindex];
	    output[outputindex]= 0x04;
	    outputindex++;
	    continue;
	  }else break;
	}// end of consuming 73{74}
	if(consuming == 0x75){
	  //printf("consumed 0x75\n");	
	  consuming = input[++inputindex];
	  if(consuming == 0x62) consuming = input[++inputindex];
	  if(consuming == 0x20 || consuming == 0x0a){
	    //success
	    consuming = input[++inputindex];
	    output[outputindex]= 0x0a;
	    outputindex++;
	    continue;
	  }else break;
	}
	
      }// end of consuming 73
  
	//********** Consuming 4d
      if(consuming == 0x4d){
	//printf("consumed 0x4d\n");	
	consuming = input[++inputindex];
	if(consuming == 0x5b) {
	  //printf("consumed 0x5b\n");
	  consuming = input[++inputindex]; //consuming 1st byte of address
	  output[outputindex] = 0x05; outputindex++;
	  output[outputindex] = consuming; outputindex++;
	  consuming=input[++inputindex]; //consuming 2nd byte of address.
	  output[outputindex] = consuming; outputindex++;
	  consuming=input[++inputindex]; //consuming 3rd byte of address
	  output[outputindex] = consuming; outputindex++;
	  consuming=input[++inputindex]; //consuming 4th byte of address.
	  output[outputindex] = consuming; outputindex++;
	  consuming=input[++inputindex];
	  
	  if(consuming== 0x5d){
	    consuming=input[++inputindex];
	    //printf("consumed 0x5d\n");
	    continue;
	  }
	}
      }
      if(consuming == 0x03){
	output[outputindex]=0x03; outputindex++;
	printf("Found HALT. Translation is successful.\n");
	break;
      }
      printf("Unexpectly reached here. There might be errors in program. Breaking.\n");
      break;
	
  }// *end of While(1)
  fwrite((void *)output, sizeof(uint8_t), BUFFSIZE, fo);
  hexDump("Output Hex dump:", output, BUFFSIZE);

  fclose(fp);
  fclose(fo);
}


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
