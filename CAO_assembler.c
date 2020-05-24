#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MAX_SYMBOL_NAME_LENGTH 30
#define MAX_FILE_NAME_LENGTH 30


//arithmetic
#define NOP 0x00
#define STP 0x08
#define RSF 0x10
#define ADD 0x20 
#define ADDI 0x28
#define SUB 0x30 
#define SUBI 0x38
#define NOT 0x40
#define XOR 0x50
#define XORI 0x58
#define ORR 0x60
#define ORRI 0x68
#define AND 0x70
#define ANDI 0x78

//flow control
#define JMP 0x80 //1000
#define JMPB 0x88
#define JMP_C 0x01
#define JMP_N 0x02
#define JMP_Z 0x04
#define JZ JMP+JMP_Z
#define JN JMP+JMP_N
#define JC JMP+JMP_C
#define JZN JMP + JMP_Z + JMP_N
#define JZC JMP + JMP_Z + JMP_C
#define JNC JMP + JMP_N + JMP_C
#define JZNC JMP + JMP_Z + JMP_N + JMP_C

//data movement (load store)
#define LDA 0xA0 //1010
#define MOVA 0xA4
#define LDAB 0xA8 
#define LDB 0xB0 //1011
#define MOVB 0xB4
#define LDBB 0xB8 
#define LDC 0xC0 //1100
#define MOVC 0xC4
#define LDCB 0xC8
#define STC 0xD0 //1101
#define STCB 0xD8

typedef struct symbol_table_entry{
	char name[MAX_SYMBOL_NAME_LENGTH];
	uint16_t address;
} ste;


int st_max_num_entries=128;
int st_num_entries=0;
ste* st;

void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

ste* find_symbol(char* symbol)
{
	for(int i =0 ; i< st_num_entries ; i++)
	{
		if(strcmp(symbol, st[i].name)==0)
		{
			return &st[i];
		}
	}
	return 0;
}

int add_symbol(char* name, int address)
{
	if(st_num_entries==st_max_num_entries)
	{
		st_max_num_entries*=2;
		st=realloc(st, st_max_num_entries);
	}

	strcpy(st[st_num_entries].name,name);
	st[st_num_entries].address=address;
	st_num_entries++;
	return 0;
}

void display_symbol_table()
{
	for(int i = 0 ; i< st_num_entries ; i ++)
	{
		printf("symbol: %s, address: %d\n", st[i].name, st[i].address);
	}
}

int is_symbol(char* string)
{
	return (string[(strlen(string)-1)] == ':');
}


uint32_t to_num(char* string)
{
	//printf("%c\n", string[0]);
	//printf("%c\n", string[1]);
	//printf("%s\n", string);
	//printf("isdigit: %d\n", isdigit(string[0]));
	if(string==0)
		return 0;
	remove_spaces(string);
	if(string[1]=='x')
	{
		//printf("hex detected\n");
		return (int)strtol(string+2, NULL, 16);
	}
	if(string[1]=='b')
	{
		//printf("binary detected\n");
		return (int)strtol(string+2, NULL, 2);
	}
	else if(string[0]==39) //if single quote
	{
		return (uint32_t)string[1];
	}
	else if(isdigit(string[0]))
	{
		//printf("decimal detected\n");
		return atoi(string);
	}
	else if(string[0]=='-')
	{
		return ((-1*atoi(string+1))&0xFFFF);
	}
	else if(isalpha(string[0]))
	{
		uint32_t val;
		char symbol_name[MAX_SYMBOL_NAME_LENGTH];
		int i=0;
		for(i =0 ; i< strlen(string); i++)
		{
			if(string[i]=='+' || string[i]=='-')
				break;
		}
		strncpy(symbol_name,string,i);
		symbol_name[i]=0;
		ste* symbol=find_symbol(symbol_name);
		if(symbol==0)
		{
			printf("can't find symbol %s\n", symbol_name );
			return 0;
		}
		val = symbol->address;
		if(i<strlen(string))
		{
			if(string[i]=='+')
				val+=atoi(string+1);
			else if(string[i]=='-')
				val-=atoi(string+1);
		}
		return val;
	}
	if(string[0]=='.'&&string[1]=='H')
	{
		char input[MAX_SYMBOL_NAME_LENGTH];
		strncpy(input,string+6,strlen(string)-7);
		input[(strlen(string)-7)]=0;
		return (to_num(input)>>8)&0xFF;

	}
	else if(string[0]=='.'&&string[1]=='L')
	{
		char input[MAX_SYMBOL_NAME_LENGTH];
		strncpy(input,string+5,strlen(string)-6);
		input[(strlen(string)-6)]=0;
		return to_num(input)&0xFF;
	}

}

void instruction_to_binary(FILE* binary_file, uint8_t instruction, int instruction_length, uint16_t* current_address)
{
	switch(instruction_length)
	{
		int status;
		char* string;
		uint32_t num;
		case 3: 	
        	string = strtok(NULL, "\n/");
        	//printf("input: %s\n",string );        		
        	fseek(binary_file, *current_address, SEEK_SET);
        	num = to_num(string);
        	if( (num&0x10000) != 0 )
        	{
        		status = fputc(instruction+1, binary_file);
        	}
        	else
        	{
       			status = fputc(instruction, binary_file);
        	}
       		*current_address+=1;
       		fseek(binary_file, *current_address, SEEK_SET);
       		status = fputc( (uint8_t)(num>>8) , binary_file);
       		*current_address+=1;
       		fseek(binary_file, *current_address, SEEK_SET);
       		status = fputc( (uint8_t)num, binary_file);
       		*current_address+=1;
			break;
		case 2:
        	string = strtok(NULL, "\n/");
        	fseek(binary_file, *current_address, SEEK_SET);
        	num = to_num(string);
        	status = fputc(instruction, binary_file);
        	*current_address+=1;
        	fseek(binary_file, *current_address, SEEK_SET);
       		status = fputc( (uint8_t)num , binary_file);
       		*current_address+=1;
			break;
		case 1:
			fseek(binary_file, *current_address, SEEK_SET);
       		status = fputc(instruction, binary_file);
       		*current_address+=1; 
			break;
	}
	
}


int main(int argc, char *argv[]){
	 /* File pointer to hold reference to our file */
	FILE * assembly_file;
	FILE * binary_file;
	char binary_file_name[MAX_FILE_NAME_LENGTH];
	int specify_output=0;

	//find the input CAO assembly file name
	int i = 0;
	while(i < argc)
	{
		char* extension = argv[i] + strlen(argv[i]) - 1 - 3; //should points to '.'
		if(strcmp(extension, ".cao" )==0 || strcmp(extension, ".CAO" )==0)
			break;
		i++;
	}
	if(argc==i)
	{
		printf("no input CAO assembly file detected (notice: file extension must be .cao or .CAO)\n");
		return 0;
	}
	printf("input file is %s\n", argv[i]);
	
	//change the output extension to .bin
	strcpy(binary_file_name, argv[i]);
	binary_file_name[strlen(argv[i])-1]='n';
	binary_file_name[strlen(argv[i])-2]='i';
	binary_file_name[strlen(argv[i])-3]='b';
	
	//first create the symbol table
	assembly_file = fopen(argv[i], "r");
	char line[256];
	uint16_t current_address;
	st = (ste*) malloc(sizeof(ste)*st_max_num_entries);

	char *token;
    while (fgets(line, sizeof(line), assembly_file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        printf("%s", line); 
        token = strtok(line, " /\n");
        if(token==NULL)
        	continue;

        if(is_symbol(token))
        {
        	char symbol_name[MAX_SYMBOL_NAME_LENGTH];
        	strncpy(symbol_name,token,strlen(token)-1); //-1 to get rid of the ':' sign
        	symbol_name[strlen(token)-1]=0; //null terminate it
        	if(find_symbol(symbol_name))
        	{
        		printf("symbol name %s is already defined previously\n", token);
        	}
        	else{
        		add_symbol(symbol_name,current_address);
        	}
        }

        if(strcmp(token, "LDA")==0 && strlen(token) == strlen("LDA"))
        	current_address+=3;
        else if (strcmp(token, "LDB")==0 && strlen(token) == strlen("LDB"))
        	current_address+=3;
        else if (strcmp(token, "LDC")==0 && strlen(token) == strlen("LDC"))
        	current_address+=3;
        else if(strcmp(token, "MOVA")==0 && strlen(token) == strlen("MOVA"))
        	current_address+=2;
        else if (strcmp(token, "MOVB")==0 && strlen(token) == strlen("MOVB"))
        	current_address+=2;
        else if (strcmp(token, "MOVC")==0 && strlen(token) == strlen("MOVC"))
        	current_address+=2;
        else if (strcmp(token, "STC")==0 && strlen(token) == strlen("STC"))
        	current_address+=3;
        else if (strcmp(token, "JMP")==0 && strlen(token) == strlen("JMP"))
        	current_address+=3;
        else if (strcmp(token, "ADDI")==0 && strlen(token) == strlen("ADD"))
        	current_address+=2;
        else if (strcmp(token, "SUBI")==0 && strlen(token) == strlen("SUB"))
        	current_address+=2;
        else if (strcmp(token, "ANDI")==0 && strlen(token) == strlen("AND"))
        	current_address+=2;
        else if (strcmp(token, "ORRI")==0 && strlen(token) == strlen("ORR"))
        	current_address+=2;
        else if (strcmp(token, "XORI")==0 && strlen(token) == strlen("XOR"))
        	current_address+=2;
        else if (strcmp(token, "ADD")==0 && strlen(token) == strlen("ADD"))
        	current_address+=1;
        else if (strcmp(token, "SUB")==0 && strlen(token) == strlen("SUB"))
        	current_address+=1;
        else if (strcmp(token, "RSF")==0 && strlen(token) == strlen("RSF"))
        	current_address+=1;
        else if (strcmp(token, "AND")==0 && strlen(token) == strlen("AND"))
        	current_address+=1;
        else if (strcmp(token, "ORR")==0 && strlen(token) == strlen("ORR"))
        	current_address+=1;
        else if (strcmp(token, "NOT")==0 && strlen(token) == strlen("NOT"))
        	current_address+=1;
        else if (strcmp(token, "XOR")==0 && strlen(token) == strlen("XOR"))
        	current_address+=1;
        else if (strcmp(token, "NOP")==0 && strlen(token) == strlen("NOP"))
        	current_address+=1;
       	else if (strcmp(token, "STP")==0 && strlen(token) == strlen("STP"))
        	current_address+=1;
        else if (strcmp(token, ".BYTE")==0 && strlen(token) == strlen(".BYTE"))
        	current_address+=1;
        else if (strcmp(token, ".STRING")==0 && strlen(token) == strlen(".STRING"))
        {
        	token = strtok(NULL, " /\n");
        	current_address+=(strlen(token)-2);
        }
    }
	display_symbol_table();

    //then create the binary file
    current_address=0;
    fseek(assembly_file, 0, SEEK_SET);//reset file position
    binary_file = fopen(binary_file_name, "w");
    while (fgets(line, sizeof(line), assembly_file)) {
    	token = strtok(line, " /\n");
        if(token==NULL)
        	continue;
        if(strcmp(token, "LDA")==0 && strlen(token) == strlen("LDA"))
        {
        	/*
        	int status;
        	char* address_string = strtok(NULL, "\n/");
        	//printf("%s\n",address_string );        		
        	fseek(binary_file, current_address, SEEK_SET);
        	uint32_t address = to_num(address_string);
        	if( (address&0x10000) != 0 )
        	{
        		status = fputc(LDA+1, binary_file);
        	}
        	else
        	{
       			status = fputc(LDA, binary_file);
        	}
       		current_address++;
       		fseek(binary_file, current_address, SEEK_SET);
       		status = fputc( (uint8_t)(address>>8) , binary_file);
       		current_address++;
       		fseek(binary_file, current_address, SEEK_SET);
       		status = fputc( (uint8_t)address, binary_file);
       		current_address++;
       		*/
       		instruction_to_binary(binary_file,LDA,3,&current_address);

        }
        else if (strcmp(token, "LDB")==0 && strlen(token) == strlen("LDB"))
        	instruction_to_binary(binary_file,LDB,3,&current_address);
        else if (strcmp(token, "LDC")==0 && strlen(token) == strlen("LDC"))
        	instruction_to_binary(binary_file,LDC,3,&current_address);
        else if (strcmp(token, "STC")==0 && strlen(token) == strlen("STC"))
        	instruction_to_binary(binary_file,STC,3,&current_address);
        else if (strcmp(token, "JMP")==0 && strlen(token) == strlen("JMP"))
        	instruction_to_binary(binary_file,JMP,3,&current_address);

        else if(strcmp(token, "MOVA")==0 && strlen(token) == strlen("MOVA"))
        	instruction_to_binary(binary_file,MOVA,2,&current_address);
        else if (strcmp(token, "MOVB")==0 && strlen(token) == strlen("MOVB"))
        	instruction_to_binary(binary_file,MOVB,2,&current_address);
        else if (strcmp(token, "MOVC")==0 && strlen(token) == strlen("MOVC"))
        	instruction_to_binary(binary_file,MOVC,2,&current_address);

        else if (strcmp(token, "ADDI")==0 && strlen(token) == strlen("ADD"))
        	instruction_to_binary(binary_file,ADDI,2,&current_address);
        else if (strcmp(token, "SUBI")==0 && strlen(token) == strlen("SUB"))
        	instruction_to_binary(binary_file,SUBI,2,&current_address);
        else if (strcmp(token, "ANDI")==0 && strlen(token) == strlen("AND"))
        	instruction_to_binary(binary_file,ANDI,2,&current_address);
        else if (strcmp(token, "ORRI")==0 && strlen(token) == strlen("ORR"))
        	instruction_to_binary(binary_file,ORRI,2,&current_address);
        else if (strcmp(token, "XORI")==0 && strlen(token) == strlen("XOR"))
        	instruction_to_binary(binary_file,XORI,2,&current_address);

        else if (strcmp(token, "ADD")==0 && strlen(token) == strlen("ADD"))
        {
       		/*
       		fseek(binary_file, current_address, SEEK_SET);
       		int status = fputc(ADD, binary_file);
       		current_address++;
       		*/
       		instruction_to_binary(binary_file,ADD,1,&current_address);
        }
        else if (strcmp(token, "SUB")==0 && strlen(token) == strlen("SUB"))
			instruction_to_binary(binary_file,ADD,1,&current_address);
        else if (strcmp(token, "RSF")==0 && strlen(token) == strlen("RSF"))
			instruction_to_binary(binary_file,SUB,1,&current_address);
        else if (strcmp(token, "AND")==0 && strlen(token) == strlen("AND"))
			instruction_to_binary(binary_file,AND,1,&current_address);
        else if (strcmp(token, "ORR")==0 && strlen(token) == strlen("ORR"))
			instruction_to_binary(binary_file,ORR,1,&current_address);
        else if (strcmp(token, "NOT")==0 && strlen(token) == strlen("NOT"))
			instruction_to_binary(binary_file,NOT,1,&current_address);
        else if (strcmp(token, "XOR")==0 && strlen(token) == strlen("XOR"))
			instruction_to_binary(binary_file,XOR,1,&current_address);
        else if (strcmp(token, "NOP")==0 && strlen(token) == strlen("NOP"))
			instruction_to_binary(binary_file,NOP,1,&current_address);
       	else if (strcmp(token, "STP")==0 && strlen(token) == strlen("STP"))
			instruction_to_binary(binary_file,STP,1,&current_address);
        else if (strcmp(token, ".BEGIN")==0 && strlen(token) == strlen(".BEGIN"))
		{	
			int status;
        	char* address_string = strtok(NULL, "\n/");
        	uint32_t address = to_num(address_string);
        	current_address=address;
		}
		else if (strcmp(token, ".BYTE")==0 && strlen(token) == strlen(".BYTE"))
		{	
			int status;
        	char* string = strtok(NULL, "\n/");
        	uint32_t num = to_num(string);
        	fseek(binary_file, current_address, SEEK_SET);
       		status = fputc((uint8_t)num, binary_file);
       		current_address++;
		}
		else if (strcmp(token, ".STRING")==0 && strlen(token) == strlen(".STRING"))
		{	
			int status;
        	char* string = strtok(NULL, "\n/");
        	
        	for(int i=0; i < (strlen(string)-2); i++ ){ //subtract two because of the double quote
        		fseek(binary_file, current_address, SEEK_SET);
       			status = fputc( string+1+i, binary_file); //+1 to skip the double quote
       			current_address++;
       		}
		}
    }
    fclose(binary_file);
    fclose(assembly_file);
    
}


