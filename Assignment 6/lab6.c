#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>


void interrupt(*int9Save) (void);  /* Pointer to function */
void interrupt(*int8Save) (void);  /* Pointer to function */

int countTime = 0;
int i =0;
int m = 0;
char input_char[20];
char temp;
unsigned char scanc;
int copyTime[20];
unsigned int pressed,scanCode;
volatile int flagEnter = 0;

void create_password(char passwd[], int times[]);
int test_password(char passwd[], int times[]);
char ReadKeyboardBuffer();


void interrupt h9(){
	asm {
		in al,60h
		mov scanc, al
	}
	scanCode = (unsigned int) scanc;
	if (scanCode == 28) // Enter pressed
		{
			
			asm{
				in al,61h
				or al,80h
				out 61h,al
				and al,7fh
				out 61h,al
				mov al,20h
				out 20h,al
				}
				flagEnter = 1;
				return;
		}
	asm{
		PUSH AX
		MOV AH,1
		INT 16h
		PUSHF
		POP AX
		AND AX,64  //zero flag
		MOV pressed,AX
		POP AX
	}
	if( pressed == 0)//check if button pressed (not released)
	{
		temp = ReadKeyboardBuffer();
		if(temp != 0){ //if the char has ascii code we want to print it 
			input_char[i]=temp;
			copyTime[i]=countTime;
			countTime=0;
			printf("*");
			i++;
		}
	}
	int9Save();
}

void interrupt h8(){
	int8Save();
	countTime++;
}


char ReadKeyboardBuffer() // Read a char from the buffer and return the char
{
	char c2;
	asm{
		PUSH 	AX
		MOV		AH,0
		INT 	16h
		MOV 	c2,AL
		POP 	AX
	}
	return c2;
}


void create_password(char passwd[], int times[]){
	char temp;
	int j;
	printf("Enter password:\n");
	int9Save = getvect(9);
	setvect(9, h9);
	int8Save = getvect(8);
	setvect(8, h8);
	while(flagEnter != 1){
}
	m=i;
	for(j=0;j<m;j++){
		passwd[j]=input_char[j];
		times[j]=copyTime[j];
	}
	printf("\n");
		/*for(j=0;j<m;j++){
		printf("%c ",passwd[j]);
		printf("%d\n",times[j]);
	}*/
	i=0;
}
int test_password(char passwd[], int times[]){
	char temp;
	int j;
	float calc1,calc2;
	flagEnter = 0;
	printf("Enter password:\n");
	while(flagEnter != 1){}		
	if(m!=i)
		return 0;
	for(j=0;j<m;j++){
	//	printf("%c ",input_char[j]);
	//	printf("%d\n",copyTime[j]);
		if(passwd[j]!= input_char[j])
			return 0;
		calc1 = times[j]-18.2/5;
		calc2 = times[j]+18.2/5;
		if(calc1 > copyTime[j] || copyTime[j] > calc2)
			return 0;
	}
	return 1;
}

int main()
{
 char password[20];
 int times[20],flag,j;

create_password(password, times);

flag = test_password(password, times);
 if (flag == 1)
   printf("\npassword succeeded!\n");
 else
   printf("\npassword failed!\n");
 setvect(8, int8Save);
 setvect(9, int9Save);

 return 0;
} // main
