#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dos.h>

void interrupt (*isr8)(void);
void interrupt (*int8Save)(void);
void interrupt (*isr9)(void);
void endSlowProg();
void slowPrg();

unsigned char scan;
float speed = 1;
int int8counter = 0;
int leftShiftKey, rightShiftKey, countTime = 0, countTimeFiveSec = 0;

void main() {
	unsigned long int i, j;
	long long int	counter = 0;
	time_t t1, t2;
	i = j = 0;
	slowPrg();
	while (1) {
		(void)time(&t1);
		j = 0;
		while (j < 10) {
			i++;
			counter++;
			if ((i % 10000) == 0) {
				printf("counter = %lld\n", counter);
				j++;
			}
		}// while
		(void)time(&t2);
		printf("\nTotal  Iteration  Time -  = %d seconds\n", (int)(t2 - t1));
	}
}

//ends function to activate the slow system protocol
void endSlowProg(){
	isr8();
	printf("finish");
	setvect(8, isr8);
	setvect(9, isr9);
	asm{
		mov ah, 4ch
		int 21h
	}
}

//my ISR for INT 8 - timer interrupt
void interrupt myISR8(){
	int i;
	countTimeFiveSec++;
	if(rightShiftKey && leftShiftKey)
	{
		countTime++;
		if(countTime >= 3 * 18)//both shifts pressed for 3 secs
			endSlowProg();	
	}
	else
		countTime = 0;

	if(countTimeFiveSec >= 5 * 18){//print system speed every 5 secs
		countTimeFiveSec = 0;
		printf("5 sec : speed : 1 / %d\n",(int)(1/speed));
	}
	int8counter++;
	//slow system
	isr8();
	asm{
            STI
        }
	while(int8counter % (int)(1/speed) != 0);
}
//my ISR for INT 9 - keyboard interrupt
void interrupt myISR9()
{
  	asm  {
		in al,60h          //put scan code of preesed key in al
		mov scan, al       //mov scan code to scan
		cmp al,42          //compare to scan code of left shift click
		jne notLeftShift
		mov leftShiftKey,1 //if left shift is pressed
		jmp advance
	}
	notLeftShift: //check right shift press
	asm{
		cmp al, 54 //compare to scan code of right shift click
		jne notRightShift
		mov rightShiftKey, 1//if right shift is pressed
		jmp advance
	}
	notRightShift: //check left shift released
	asm{
		cmp al,170 //compare to scan code of left shift released
		jne notReleasedRightShift
		mov leftShiftKey,0 //if left shift is released
		jmp advance
	}
	notReleasedRightShift: //check right shift released
	asm{
		cmp al,182 //compare to scan code of right shift released
		jne notReleasedLeftShift
		mov rightShiftKey,0 //if left shift is released
		jmp advance
	}
	notReleasedLeftShift: //check arrow up press 
		asm{
		cmp  al,72 //compare to scan code of arrow up 
		je checkArrowUp
		jmp notArrowUp
	}
	checkArrowUp:
	asm{
		test 	al,80h
		jnz     advance //arrow up released
	}
	speedUp:
	if(speed < 1){
		speed = speed * 2;
		int8counter = 0;
		printf("speed : 1 / %d\n",(int)(1/speed));
	}
	asm{
		jmp advance
	}
	notArrowUp:// check arrow down press
	asm{
		cmp  al,80 //compare to scan code of arrow down 
		je checkArrowDown
		jmp advance
	}
	checkArrowDown:
	asm {
		test 	al,80h
		jnz     advance //arrow down released
	}
	speedDown:
	if(speed > 0.0625){
		speed = speed / 2;
		int8counter = 0;
		printf("speed : 1 / %d\n",(int)(1/speed));
	}

	advance: 
	asm{
		in al,61h
		or al,80h
		out 61h,al
		and al,7Fh
		out 61h,al
		mov al,20h //end int9
		out 20h,al
	}
}
//function to activate the slow system protocol
void slowPrg()
{
	isr8 = getvect(8);
	isr9 = getvect(9);
	setvect(8, myISR8);
	setvect(9, myISR9);
}
