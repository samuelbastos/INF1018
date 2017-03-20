/* Samuel Bastos de Souza Junior   1311416 3wb */
/* Isabella Corbo Ramalho de Mello 1310390 3wb */


#include <stdio.h>
#include <stdlib.h>

typedef int (*funcp) ();

typedef union num
{	
	int i;
	unsigned char b[4];

} Union;

int write_number (unsigned char* memoryBlock, int index_to_change, Union number_to_write)
{
	int i;

	for(i=0, index_to_change; i<4; i++, index_to_change++)
		memoryBlock[index_to_change] = number_to_write.b[i];

	return index_to_change;
}

//Inicia codigo (inclui subl no esp)
int inicializaCod (unsigned char *memoryBlock){

	memoryBlock[0] = 0x55;	// inicialização de ebp e movl %esp, %ebp										
	memoryBlock[1] = 0x89; 
	memoryBlock[2] = 0xe5;
						
	memoryBlock[3] = 0x83;	// variaveis locais (10)
	memoryBlock[4] = 0xec;
	memoryBlock[5] = 0x28;	

	return 6;
}

//Finaliza código (inclui add no esp)
int finalizaCod (unsigned char *memoryBlock, int contaByte){
	
	memoryBlock[contaByte++] = 0x83;
	memoryBlock[contaByte++] = 0xc4;
	memoryBlock[contaByte++] = 0x28;
	memoryBlock[contaByte++] = 0x89;	
	memoryBlock[contaByte++] = 0xec;		
	memoryBlock[contaByte++] = 0x5d;									
	memoryBlock[contaByte++] = 0xc3;

	return contaByte;
}

int leAtt (FILE* arq_fonte, unsigned char* memoryBlock, int contaByte, int c, int* ordem_var_local){

	int ordem;
	char c1, c2, op;
	Union o, o1, o2;
	int i;
	
	fscanf(arq_fonte, "%d = %c%d %c %c%d", &o.i, &c1, &o1.i, &op , &c2, &o2.i);

	switch(c1){ //ecx

		case '$':
		{
			memoryBlock[contaByte++] = 0xb9;

			contaByte = write_number(memoryBlock, contaByte , o1);
			break;
		}

		case 'p':
		{	//ECX = parametro - x(%ebp), %ecx
			memoryBlock[contaByte++] = 0x8b; //puxa parametro
			memoryBlock[contaByte++] = 0x4d;

			if(o1.i == 0)
			memoryBlock[contaByte++] = 0x08; //8(%ebp)

			else if (o1.i == 1)
			memoryBlock[contaByte++] = 0x0c; //12(%ebp)

			else if (o1.i == 2)
			memoryBlock[contaByte++] = 0x10; //16(%ebp)

			else if (o1.i == 3)
			memoryBlock[contaByte++] = 0x14; //...

			else if (o1.i == 4)
			memoryBlock[contaByte++] = 0x18; //...
			

			break;
		}

		case 'v':
		{
			for(i=0; i < 10 ; i++)
			{
				if( ordem_var_local[i] == o1.i){
				ordem = i;
				break;
				}
				if( ordem_var_local[i] < 0){
				ordem = i;
				break;
				}	
			}
			
			memoryBlock[contaByte++] = 0x8b; //relativo a operacao
			memoryBlock[contaByte++] = 0x4d; //relativo a registrador
			
			memoryBlock[contaByte++] = (0xfc - (ordem)*4 ); //relativo a -x(%ebp)		
		
			break;
		}

		default:
			fprintf(stderr, "Simbolo invalido\n");
	}

	switch(c2){ //edx 


		case '$':
		{
			memoryBlock[contaByte++] = 0xba;

			contaByte = write_number(memoryBlock, contaByte , o2);
	
			break;
		}

		case 'p':
		{
			memoryBlock[contaByte++] = 0x8b;
			memoryBlock[contaByte++] = 0x55;
			memoryBlock[contaByte++] = (0x08 + (o2.i)*4 );
			
			break;
		}

		case 'v':
		{
			for(i=0; i < 10 ; i++)
			{
				if( ordem_var_local[i] == o2.i){
				ordem = i;
				break;
				}
				if( ordem_var_local[i] < 0){
				ordem = i;
				break;
				}	
			}
			
			memoryBlock[contaByte++] = 0x8b;
			memoryBlock[contaByte++] = 0x55;
			memoryBlock[contaByte++] = (0xfc - (ordem)*4 );
			break;
		}

		default:
			fprintf(stderr, "erro - caracter invalido\n");
	}

	switch(op) //resultado em %ecx
	{	
		
		case '+':
		{
			memoryBlock[contaByte++] = 0x01;
			memoryBlock[contaByte++] = 0xd1;
			break;
			
				
		}
		
		case '-':
		{	
			memoryBlock[contaByte++] = 0x29;
			memoryBlock[contaByte++] = 0xd1;
			break;
		}
		
		case '*':
		{
			memoryBlock[contaByte++] = 0x0f;
			memoryBlock[contaByte++] = 0xaf;
			memoryBlock[contaByte++] = 0xca;
			break;
		}

		default:
			fprintf(stderr, "erro - caracter invalido \n");
	}

	switch(c){ 

		case 'p':
		{	//mov %ecx, x(%ebp) - valor pra pilha
			memoryBlock[contaByte++] = 0x89;
			memoryBlock[contaByte++] = 0x4d;
			memoryBlock[contaByte++] = (0x08 + (o.i)*4 ); //-> indica apenas se p0,p1,p2 .....

			break;
		}

		case 'v':
		{
			
			for(i=0; i < 10 ; i++)
			{	
				if( ordem_var_local[i] == o.i){
				ordem = i;
				break;}
				if( ordem_var_local[i] < 0){
				ordem = i;
				break;}
			}
			
			// movl %ecx, -x(%ebp)  - valor pra pilha
			memoryBlock[contaByte++] = 0x89;
			memoryBlock[contaByte++] = 0x4d;
			memoryBlock[contaByte++] = 0xfc - 4*(ordem); //-> indica apenas se é v0,v1,v2 .....
			ordem_var_local[ordem] = o.i;

						
			if(o.i == 0) // se for v0, além de salvar na pilha, vai jogar para %eax
			{
				memoryBlock[contaByte++] = 0x89;
				memoryBlock[contaByte++] = 0xc8;
			}	

			break;
		}
	}

	return contaByte;
}

int leRet (FILE* arq_fonte, unsigned char* memoryBlock, int contaByte){
	
	fscanf( arq_fonte, "et");

	return finalizaCod(memoryBlock, contaByte);
}

int adicionaCmp (unsigned char* memoryBlock, int contaByte){

	memoryBlock[contaByte++] = 0x83;
	memoryBlock[contaByte++] = 0xf9;
	memoryBlock[contaByte++] = 0x00;
	
	return contaByte;
}

int leWhile(FILE* arq_fonte, unsigned char* memoryBlock, int contaByte, int* ordem_var_local){
	
	int ordem;
	char c1;
	Union o;
	int i;

	fscanf( arq_fonte, "hile %c%d", &c1, &o.i);

	switch(c1){ //ecx

		case 'p':
		{	//ECX = parametro - x(%ebp), %ecx
			memoryBlock[contaByte++] = 0x8b; //puxa parametro
			memoryBlock[contaByte++] = 0x4d;

			if(o.i == 0)
			memoryBlock[contaByte++] = 0x08; //8(%ebp)

			else if (o.i == 1)
			memoryBlock[contaByte++] = 0x0c; //12(%ebp)

			else if (o.i == 2)
			memoryBlock[contaByte++] = 0x10; //16(%ebp)

			else if (o.i == 3)
			memoryBlock[contaByte++] = 0x14; //...

			else if (o.i == 4)
			memoryBlock[contaByte++] = 0x18; //...
			
			break;
		}

		case 'v':
		{
			
			for(i=0; i < 10 ; i++)
			{
				if( ordem_var_local[i] == o.i){
				ordem = i;
				break;
				}
				if( ordem_var_local[i] < 0){
				ordem = i;
				break;
				}	
			}
			
			memoryBlock[contaByte++] = 0x8b; //relativo a operacao
			memoryBlock[contaByte++] = 0x4d; //relativo a registrador
			
			memoryBlock[contaByte++] = (0xfc - (ordem)*4 ); //relativo a -x(%ebp) - 0xfc = -4(%ebp)
					
			break;
		}

		default:
			fprintf(stderr, "erro - caracter invalido\n");
	}
	
	contaByte = adicionaCmp(memoryBlock, contaByte);
	
	memoryBlock[contaByte++] = 0xff; // byte que vai ser alterado futuramente (jz)	
	memoryBlock[contaByte++] = 0xff; // byte que vai ser alterado futuramente
	
	return contaByte;
}

int leEnd(FILE* arq_fonte, unsigned char* memoryBlock, int contaByte, int* armazenaWhile){
	
	int i;
	int valor;
	int armazenaIndice;

	//procuro o último indíce preenchido com valor diferente de -1
	for(i=0; armazenaWhile[i] > 0; i++){

		valor = armazenaWhile[i]; //achado a quantidade de bytes correspondente ao while em questão
		armazenaIndice = i;		  //pegar índice do while sendo terminado	
	}

	armazenaWhile[armazenaIndice] = -1; //atualizo, pois o while já recebeu end e foi lido com sucesso !

	char jz, jmp;
	int bytedifjz = contaByte - valor + 2; // obtenho em um byte a representacao do inteiro
	jz = (char) bytedifjz;
	
	int bytedifjmp = contaByte - valor;
	jmp = (char) bytedifjmp;


	fscanf( arq_fonte, "nd");

	
	memoryBlock[valor - 2] =  0x74;		 //primeiro byte do jz
	memoryBlock[valor - 1] =  0x00 + jz; 
	
	memoryBlock[contaByte++] = 0xeb;
	memoryBlock[contaByte++] = 0xfc - jmp;
	

	return contaByte;
}


funcp geracod(FILE* arq_fonte){
	
	
	int c = 0;
	int contaByte = 0; //acumulador de bytes
	
	int ordem_var_local[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; // guardar ordem das variaveis locais

	int armazenaWhile[25] =  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	int i = 0;

	unsigned char* memoryBlock = (unsigned char*) malloc(2000); //bloco aonde vai ser armazenado codigo de maquina
	contaByte = inicializaCod(memoryBlock); // Comeca com 6
	
	while ((c = fgetc(arq_fonte)) != EOF){  

		switch(c)
		{
			case 'v': case 'p':
			{
				contaByte = leAtt(arq_fonte, memoryBlock, contaByte, c, ordem_var_local); //Lê atribuições da linguagem
				break;
			}	
			case 'r':
			{	
				contaByte = leRet(arq_fonte, memoryBlock, contaByte); // Lê o final da função
				break;
			}
			
			case 'w':
			{	
				contaByte = leWhile(arq_fonte, memoryBlock, contaByte, ordem_var_local); // Lê o while
				armazenaWhile[i] = contaByte;
				i++;
				break;
			}
			case 'e': // o primeiro end vai ser correspondente, sempre ao último while escrito (Lê o final do while)
			{
				contaByte = leEnd(arq_fonte, memoryBlock, contaByte, armazenaWhile);
				break;
			}
				
		}

		fscanf(arq_fonte, " ");
	}

	return (funcp) memoryBlock;
}

void liberacod (void *p){

free(p);

}
