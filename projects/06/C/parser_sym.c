/**** An assembler for Hack Assembly language. 
      Reads an text file contain hack assembly and writes a text file containing hack machine code.
      Author: Qazi Hamid.
      
      Use:  assembler  input.asm output.hack 
       *****/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 128
#define MAX_SYMBOLS 2000
#define MAX_SYM_LEN 50

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
      return(sym->val);
    sym++;
  }
  return(NULL);
}


/********** End of keyvalue store**************/

/***************** Symbol Table ***/
typedef struct S_Sym_struct{
  char name[MAX_SYM_LEN]; // Symbol names must not be more than 50 chars long
  int16_t value;
}symbolstruct;
int symboltable_end=23; // There are 23 pre-defined symbols in hack assembly.
symbolstruct symboltable[MAX_SYMBOLS]={
  {"R0", 0},
  {"R1", 1},
  {"R2", 2},
  {"R3", 3},
  {"R4", 4},
  {"R5", 5},
  {"R6", 6},
  {"R7", 7},
  {"R8", 8},
  {"R9", 9},
  {"R10", 10},
  {"R11", 11},
  {"R12", 12},
  {"R13", 13},
  {"R14", 14},
  {"R15", 15},
  {"SP", 0},
  {"LCL", 1},
  {"ARG", 2},
  {"THIS", 3},
  {"THAT", 4},
  {"SCREEN", 16384},
  {"KBD", 24576},
    
  };  // Number of symbols must not exceed 500

int16_t contains(char *t_sym){
  //check if the table contains this sybbol. if return its value.
  // if table does not contain symbols, return -1.
  int i;
  for(i=0; i<symboltable_end; i++){
    if(strcmp(symboltable[i].name, t_sym) == 0)
      return(symboltable[i].value);
  }
  return(-1);
}

int isnumericsymbol(char *symbol, size_t len){
  int i;
  for(i=0; i<len; i++)
    if(!isdigit(symbol[i]))
      return(0);
  return(1);
}



/*************** Main ***************/
int main(int argc, char **argv){
  
  FILE *infile, *outfile; char outfilename[50];
  char *line;  char symbol[BUFF_SIZE]={0};  //char currentcommand[BUFF_SIZE]={0};
  char destmn[5]={10}; char compmn[10]={0}; char jumpmn[10];
  int16_t address=0, next_RAM_add=16; // RAM addresses after 16 are allocated.
  char addressbits[17]={0}; char cmdbits[17]={0};
  size_t linelen=0, currentline=0;
  ssize_t charsinline;
  enum cmdtype type = A_COMMAND;


  if(argc !=2 && argc !=3){
    printf("Usage ERROR: Arguments must be 1 or 2: \n Proper Use example:  assembler infile.asm outfile.hack\n Exampe: assembler infile.asm\n");
    exit(1);
  }
  // printf("ARG 1= %s    ARG 2= %s  ARG 3= %s \n", argv[0], argv[1], argv[2]);
  if(argc==2){
    //printf("Please specify one input file. and one output file\n");
    //return(1024);
    int i=0;
    while((i<40) && (argv[1][i]=='.')){
      outfilename[i]= argv[1][i];
      i++;
    }
    while((i<40)&&(argv[1][i] != '.') && (argv[1][i] != ' ')){
      outfilename[i] = argv[1][i];  // copy first part of file name
      i++;
    }
    outfilename[i++] = '.'; outfilename[i++]='h'; outfilename[i++]='a'; outfilename[i++]='c'; outfilename[i++]='k'; outfilename[i]='\0';
    fprintf(stderr, "STDERR: No output file has been specified. Wrting output in %s\n", outfilename);
  }
  if(argc == 3)
    strcpy(outfilename, argv[2]);

  if((infile=fopen(argv[1],"r"))==NULL){
    printf("Could not open the input file %s\n", argv[1]);
     exit(1025);
  }
  if((outfile=fopen(outfilename,"w"))==NULL){
    printf("Could not open the output file %s\n", argv[2]);
    return(1025);
  }
  line = (char*) malloc(128*sizeof(char));
  /******************* PASS 1******************************/
  while((charsinline=getline(&line, &linelen, infile))!= -1){
    currentline++;    
    if(iscomment(line, linelen))
      continue; //skip comments and empty line
    type = getcmdtype(line, linelen);
    if(type==A_COMMAND || type==C_COMMAND){
      address++;  // This command will take one ROM address so increment address
      continue; // skip A and C commands and look for symbols
    }else{ // its an L_COMMAND
      getsymbol(line, symbol, linelen);
      if(strlen(symbol)>=MAX_SYM_LEN){
	printf("ERROR: Symbol name [%s] is too long. Max length must be less than 31 Chars", symbol);
	exit(113);
      }
      //insert symbol into symbol table
      if(contains(symbol) != -1){
	printf("ERROR: Line %zu: Symbol [%s] has multiple definitions.\n", currentline, symbol);
	exit(114);
       }
      symboltable[symboltable_end].value = address;
      strcpy(symboltable[symboltable_end].name, symbol);
      symboltable_end++;
      
      //printf("New Symbol [%s] defined with value %du\n", symbol, address);
      if(symboltable_end >= MAX_SYMBOLS){
	printf("Error: Too many symbols defined in the program: \n");
	exit(115);
      }
    }
  }  

  /******************** PASS 2 ****************************/
  //getting ready for pass2
  currentline =0;
  address=0;
  fseek(infile, 0, SEEK_SET);

  //Ready for pass2 now
  while((charsinline=getline(&line, &linelen, infile))!= -1){
    currentline++;    
    if(iscomment(line, linelen)){
      continue; //skip comments
    }
    type = getcmdtype(line, linelen);
    //printf("Type of command is %d\n", type);
    if(type==A_COMMAND){
      if(getsymbol(line, symbol, linelen)==0){
	printf("ERROR: Line: %zu: Syntax Error\n", currentline);
	exit(111);  // get the symbol in command.
      }
      if(strlen(symbol) >= MAX_SYM_LEN){
	printf("ERROR: Line %zu: Symbol name [%s] is too long", currentline, symbol);
	exit(109);
      }
      if(isnumericsymbol(symbol, strlen(symbol)))
	address = getsymaddress(symbol, strlen(symbol));
      else
	address=contains(symbol);

      if(address == -1){
	// A variable has been found. Assign it next available RAM address and put it into the symbol table.
	//remember to check symbol table bounds.
	strcpy(symboltable[symboltable_end].name, symbol);
	symboltable[symboltable_end].value= next_RAM_add;
	address=next_RAM_add;
	next_RAM_add++;
	symboltable_end++;
	if(symboltable_end >= MAX_SYMBOLS){
	  printf("ERROR: line %zu: Too many symbols encountered. Not enough memory was allocated", currentline);
	  exit(112);
	}
	//printf("ERROR: Line %d: Unrecognized Symbol [%s]", currentline, symbol);
	// exit(112);
      }

      //printf("Register A set to %u \n", address);
      getaddressbits(address, addressbits);
      printf("%s\n", addressbits);
      fprintf(outfile, "%s\n", addressbits);

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
  free(line);
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
  while((i<len) && ((line[i] !='\r') && (line[i] != '\n') && (line[i]!=')') &&(line[i] !=' '))){
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
  if(!isnumericsymbol(symbol, len)){
    printf("ERROR: Expected a number but found %s\n", symbol);
    exit(118);
  }
  //if(isdigit(symbol[0])){
    tempaddress = strtol(symbol, NULL, 10);
    address = tempaddress;
    //}
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
  if(compbits == NULL){
    printf("ERROR: Unrecognized mnemonic %s\n",compmn);
    exit(119);
  }
  if(strlen(compbits) != 7){
    printf("expected comp bit %d, but got %zu", 7, strlen(compbits));  
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
  if(jumpbits == NULL){
    printf("ERROR: Unrecognized mnemonic %s\n", jumpmn);
    exit(119);
  }
  
  for(i=0; i<3; i++)
    cmdbits[i+13] = jumpbits[i];
  cmdbits[16]='\0';
  return(1); //success
  
}
