/**** An assembler for Hack Assembly language. 
      Reads an text file contain hack assembly and writes a text file containing hack machine code.
      Author: Qazi Hamid.
      
      Use:  assembler  input.asm output.hack 
       *****/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#define BUFF_SIZE 128

enum cmdtype{A_COMMAND, C_COMMAND, L_COMMAND};
int iscomment(char *line, size_t len);
enum cmdtype getcmdtype(char *, size_t len);
int getsymbol(char *line, char *symbol, size_t len);
int parseC_COMMAND(char *line, size_t len, char * destmn, char *compmn, char *jumpmn);
int16_t getsymaddress(char* symbol, size_t len);
int getaddressbits(int16_t address, char* bits);
int getcommandbits(char *destmn, char *compmn, char *jumpmn, char *cmdbits);

/*********** Below is a basic key values store that is used to lookup the machine code for a symbols**************/

typedef struct { char *key; char* val; } t_symstruct;


static t_symstruct lookuptable[] = {
  { "0",       "0101010"}, 
  { "1",       "0111111"}, 
  { "-1",      "0111010"},
  { "D",       "0001100"},
  {"A",        "0110000"},
  {"!D",       "0001101"},
  {"!A",       "0110001"},
  {"-D",       "0001111"},
  {"-A",       "0110011"},
  {"D+1",      "0011111"},
  {"A+1",      "0110111"},
  {"D-1",      "0001110"},
  {"A-1",      "0110010"},
  {"D+A",      "0000010"},
  {"D-A",      "0010011"},
  {"A-D",      "0000111"},
  {"D&A",      "0000000"},
  {"D|A",      "0010101"},
  {"M",        "1110000"},
  {"!M",       "1110001"},
  {"-M",       "1110011"},
  {"M+1",      "1110111"},
  {"M-1",      "1110010"},
  {"D+M",      "1000010"},
  {"D-M",      "1010011"},
  {"M-D",      "1000111"},
  {"D&M",      "1000000"},
  {"D|M",      "1010101"},
  {"",         "000"},
  {"JGT",      "001"},
  {"JEQ",      "010"},
  {"JGE",      "011"},
  {"JLT",      "100"},
  {"JNE",      "101"},
  {"JLE",      "110"},
  {"JMP",      "111"}
};


#define NKEYS (sizeof(lookuptable)/sizeof(t_symstruct))

char* valuefromkey(char *key)
{
  int i; 
  t_symstruct *sym = lookuptable;

  for (i=0; i < NKEYS; i++) {
    if (strcmp(sym->key, key) == 0)
      return sym->val;
    sym++;
  }
  return NULL;
}


/********** End of keyvalue store**************/


int main(int argc, char **argv){
  
  FILE *infile, *outfile;
  char *line; char currentcommand[BUFF_SIZE]={0}; char symbol[BUFF_SIZE]={0};
  char destmn[5]={10}; char compmn[10]={0}; char jumpmn[10];
  int16_t address=0; char addressbits[17]={0}; char cmdbits[17]={0};
  size_t linelen=0, currentline=0;
  ssize_t charsinline;
  enum cmdtype type = A_COMMAND;

  line = (char*) malloc(128*sizeof(char));
  if(argc!=3){
    printf("Please specify one input file. and one output file\n");
    return(1024);
  }
  if((infile=fopen(argv[1],"r"))==NULL){
    printf("Could not open the input file %s\n", argv[1]);
     exit(1025);
  }
  if((outfile=fopen(argv[2],"w"))==NULL){
    printf("Could not open the output file %s\n", argv[2]);
    return(1025);
  }
  while((charsinline=getline(&line, &linelen, infile))!= -1){
    currentline++;    
    //printf("Retrieved line of length %zu :\n", charsinline);
    //printf("Line Read: %s", line);
    if(iscomment(line, linelen)){
      //printf("COMMENT OR EMPTY LINE: LINE IGNORED\n");
      continue; //skip comments
      //printf("LINE IS NOT COMMENT: SENT TO NEXT STAGE\n");
    }
    
    //strncpy(currentcommand, line, linelen); //Copied command to current command. 

    type = getcmdtype(line, linelen);
    //printf("Type of command is %d\n", type);
    if(type==A_COMMAND || type==L_COMMAND){
      if(getsymbol(line, symbol, linelen)==0){
	printf("ERROR: Line: %lu: Syntax Error\n", currentline);
	exit(111);  // get the symbol in command.
      }else{
	if(type == A_COMMAND){
	  address = getsymaddress(symbol, strlen(symbol));
	  //printf("Register A set to %u \n", address);
	  getaddressbits(address, addressbits);
	  printf("%s\n", addressbits);
	  fprintf(outfile, "%s\n", addressbits);
	}else{
	  printf("New Symbol [%s] has been defined \n", symbol);
	}
	//printf("Line: %lu contains a symbol %s", currentline, symbol);
      }
    }
    if(type==C_COMMAND){
      parseC_COMMAND(line, linelen, destmn, compmn, jumpmn);
      //printf("Destmn=%s : Compmn=%s : jumpmn=%s \n", destmn, compmn, jumpmn);
      getcommandbits(destmn,compmn,jumpmn,cmdbits);
      printf("%s \n", cmdbits);
      fprintf(outfile, "%s\n", cmdbits);
    }
  }
  fclose(infile);
  fclose(outfile);
  return(0);
}

int iscomment(char *line, size_t len){
  int i=0;
  while((line[i]==' ') && (i<len)) i++; //skip preceding white space.
  if((line[i]=='\r') || line[i]=='\n')
    return(1);
  if((line[i] == '/') && (line[i+1] == '/'))
     return(1);
     return(0);
}

enum cmdtype getcmdtype(char *line, size_t len){
  int i=0;
  while((line[i]==' ') && (i<len)) i++; //skip preceding white space.
  if( line[i]=='@') return(A_COMMAND);
  if( line[i]=='(') return(L_COMMAND);
  return(C_COMMAND);
}

int getsymbol(char *line, char *symbol, size_t len){
  //captuer symbol from the line and place it in the symbol location.
  int i=0, j=0;
  while((line[i]==' ') && (i<len)) {
    i++;  //skip preceding white space. 
  }

  i++; // skip the first character '@' or '('.
  while((i<len) && ((line[i] !='\r') && (line[i] != '\n') && (line[i]!=')'))){
    symbol[j]=line[i];
    i++; j++;
  }
  
  symbol[j]='\0';  // terminate string with null byte to make it compatible for other routines
  return(1);
}
int parseC_COMMAND(char *line, size_t len, char * destmn, char *compmn, char *jumpmn){
  int i=0, j=0, jumpcomp=0;
  while((line[i]==' ') && (i<len)) {
    i++;  //skip preceding white space.                                                                                                                                                                                                                                                
  }   
  while((i<len) && (line[i] != '=') && (line[i] != ' ') && (line[i] != ';')){
    destmn[j]=line[i];
    compmn[j]=line[i];
    i++; j++;
  }
  if(line[i] == ';'){
    destmn[0]='\0'; 
    jumpcomp=1;
  }
  destmn[j]='\0';
  compmn[j]='\0';
  i++; // skip '=' after dest or ';' after comp
  j=0;
  while((line[i]==' ') && (i<len)) {
    i++;  //skip preceding white space.                                                                                                                                                                                                                                                
  }   
  if(jumpcomp == 0){
    while((i<len) && (line[i] != ';') && (line[i] != ' ') && (line[i] != '\r') && (line[i] != '\n')){
      compmn[j]=line[i];
      i++; j++;
    }
    compmn[j]='\0';
    jumpmn[0]='\0';
    return(1);
    //i++;  //after compmn skip the semicolon;
  }
  j=0;
  while((line[i]==' ') && (i<len)) {
    i++;  //skip preceding white space.
    if((line[i]=='\r')||( line[i]='\n')){
      jumpmn[0]='\0';
      return(1);
    }
  }   
  while((i<len) && ((line[i] != '\r') && (line[i] != '\n') && (line[i] != ' '))){
    jumpmn[j]=line[i];
    i++; j++;
  }
  jumpmn[j]='\0';
  j=0;
  return(1);
}
int16_t getsymaddress(char* symbol, size_t len){
  long tempaddress;
  int16_t address = -1;
  if(isdigit(symbol[0])){
    tempaddress = strtol(symbol, NULL, 10);
  address = tempaddress;
  }
  return address;
}

int getaddressbits(int16_t address, char* bits){
  // address must be 16 bit integer and bits must have pre allocated memory of 16 chars.
  int i=0, bitcounter=15;

  for(i=0; i<16; i++){
    bits[bitcounter] = (address & 1) + 48;
    //printf("char: %d", bits[bitcounter]);
    bitcounter--;
    address >>= 1;
  }
  bits[16] = '\0';  // terminate as C string.
  return(1);
}

int getcommandbits(char *destmn, char *compmn, char *jumpmn, char *cmdbits){
  int i=0;
  char *compbits;
  char *jumpbits;

  cmdbits[0]='1';   // its a command so MSB must be set.
  cmdbits[1]=cmdbits[2]='1';  // these bits are unused. set to 1 by convention.

  //getcompbits() and copycompbits;
  compbits = valuefromkey(compmn);
  if(strlen(compbits) != 7){
    printf("expected comp bit %d, but got %lu", 7, strlen(compbits));  
    exit(102);
  }
  for(i=0; i<7; i++){
    cmdbits[i+3] = compbits[i];
  }
  //getdestbits();
  
  i=0; //important initialization of i before while();
  cmdbits[10]=cmdbits[11]=cmdbits[12]='0';// if dest is null, explicitly write '\0' in these bits because buffers are reused and can contain garbage.
  
  while(destmn[i] != '\0'){
    if(destmn[i] == 'A') cmdbits[10]='1';    
    if(destmn[i] == 'D') cmdbits[11]='1';
    if(destmn[i] == 'M') cmdbits[12]='1';
    i++;
    }
    //getjumpbits();
  jumpbits = valuefromkey(jumpmn);
  
  for(i=0; i<3; i++)
    cmdbits[i+13] = jumpbits[i];
  cmdbits[16]='\0';
  return(1); //success
  
}
