#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>

#define ON 1
#define OFF 0


typedef struct posAndTime{
	unsigned long int countTime;
	unsigned long int pos;
}posAndTime;

volatile int hertz = 1;
volatile int current_cursor_position = 999;
volatile unsigned long int countTime = 0;
volatile int flagDontPrint = 0;

unsigned long int totalTimeCounter = 0;
unsigned long int totalPositionChanged = 1;
unsigned char scanc, input_char;
unsigned int scanCode, pressed; 
int origSoundLatch;
char attr = 7;


posAndTime myMaxPosTime;
posAndTime myMinPosTime;
posAndTime currPosAndTime;


void interrupt(*int9Save) (void);  /* Pointer to function */
void interrupt(*int8Save) (void);  /* Pointer to function */

int readLatch();
void updateAttr();
void changeLatch();
void makeSound();
void mySound();
void changeSpeaker(int status);
void updateMaxPos();
void updateMinPos();
void F1Pressed();
void EnterPressed();
void UpArrowPressed();
void LeftArrowPressed();
void RightArrowPressed();
void DownArrowPressed();
void SetCursorPos(int position);
char ReadKeyboardBuffer();
void PrintChar(int pos, char input_ch);
void Initialize();

void interrupt h9(void)
{
	asm {
		in al,60h
		mov scanc, al
	}
	scanCode = (unsigned int) scanc;
	if(scanCode == 42 ||scanCode == 54 || scanCode == 58){ // ignore shifts (left/right) and caps lock presses
		asm {
			in al,61h
			or al,80h
			out 61h,al
			and al,7fh
			out 61h,al
			mov al,20h
			out 20h,al
		}
		return;
	}

	if (scanCode == 28 ||scanCode == 59 || scanCode == 72|| scanCode == 75|| scanCode == 77|| scanCode == 80) 
		{
			flagDontPrint = 1;
			switch (scanCode) {
				case 28: // Enter pressed
				{
					EnterPressed();
					asm{
						in al,61h
						or al,80h
						out 61h,al
						and al,7fh
						out 61h,al
						mov al,20h
						out 20h,al
					}
					exit(1);
					break;
				}
				case 59: // F1 Pressed
				{
					F1Pressed();
					break;
				}
				case 72: // Up Arrow Pressed
				{
					UpArrowPressed();
					break;
				}
				case 75: // Left Arrow Pressed
				{
					LeftArrowPressed();
					break;
				}
				case 77: // Right Arrow Pressed
				{
					RightArrowPressed();
					break;
				}
				case 80: // Down Arrow Pressed
				{
					DownArrowPressed();
					break;
				}
				default:
				{
					break;
				}
			}	
		}
	else{flagDontPrint = 0;}
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
		input_char = ReadKeyboardBuffer();
		if(input_char == 0) //if the char has ascii code we want to print it 
			flagDontPrint = 1;
		if(flagDontPrint == 0) //if the char doesn't have ascii code we don't want to print it 
			PrintChar(current_cursor_position, input_char);
	}
	int9Save();
}
void interrupt h8(void)
{
	int8Save();
	countTime++;
	totalTimeCounter++;
	updateMaxPos();
}
void EnterPressed() // Exit the program
{
	double avg;
	int line,col;
	unsigned long int int8avg;
	hertz = origSoundLatch;
	changeSpeaker(OFF);
	updateMinPos();
	updateMaxPos();
	asm{
		CLI
		PUSH AX
		MOV AL, 036h
		OUT 43h,AL
		MOV AX,0
		OUT 40h,AL
		MOV AL,AH
		OUT 40h,AL
		POP AX
	}
	setvect(8, int8Save);
	setvect(9, int9Save);
	asm{
		PUSH	AX
		MOV  	AX,2
		INT  	10h
		POP		AX
	}
	col = myMaxPosTime.pos%40 + 1;
	line = myMaxPosTime.pos/40 + 1;
	int8avg = totalTimeCounter/totalPositionChanged;
	avg = (double)totalTimeCounter/(double)(totalPositionChanged*1069);
	printf("maxCell = %d:%d = %lu\n",line,col,myMaxPosTime.countTime);//,
	col = myMinPosTime.pos%40 + 1;
	line = myMinPosTime.pos/40 + 1;
	printf("minCell = %d:%d = %lu\n",line,col, myMinPosTime.countTime);
	printf("avgSecCellTime(int8 = %lu) = %.2lf\n",int8avg,avg);
}
void F1Pressed() // Changing the attributes of the cursor
{
	int pos = current_cursor_position*2;
			asm{ 
				PUSH 	AX
				PUSH 	ES
				PUSH 	SI
				PUSH 	BX
				MOV     AX,0B800h // Segment address of memory on color adapter
				MOV     ES,AX // Set up extra segment register
				MOV     SI,0 // Initial offset address into segment
				MOV 	SI,pos	// SI = cursor's position
				MOV 	BX,WORD PTR ES:[SI]	// Moving to BX the the attributes and the ascii code of the cursor's position
				CMP 	BX,256	// Checking if the attributes is more than 256 (or equal)
				JNE		Not256	// Jumping to "Not256" if not
				MOV		BH,-1	// Reseting the attributes of the cursor's position
			}
			Not256:
			asm{
				ADD		BH,1	// Increasing the attributes by 1
				MOV 	WORD PTR ES:[SI], BX // Saving the attributes
				POP 	BX
				POP 	SI
				POP 	ES
				POP 	AX
		}
}

void updateMaxPos() // update the max position accordig to the amount of interrupts
{
	currPosAndTime.countTime = countTime;
	currPosAndTime.pos = current_cursor_position;
	if (myMaxPosTime.countTime < currPosAndTime.countTime){
		myMaxPosTime.countTime = currPosAndTime.countTime;
		myMaxPosTime.pos = currPosAndTime.pos;
	}
}

void updateMinPos(){ // update the min position accordig to the amount of interrupts
		currPosAndTime.countTime = countTime;
		currPosAndTime.pos = current_cursor_position;
		if (myMinPosTime.countTime > currPosAndTime.countTime){
		myMinPosTime.countTime = currPosAndTime.countTime;
		myMinPosTime.pos = currPosAndTime.pos;
	}
}

void UpArrowPressed() // Place the cursor 1 cell upwards
{
	if (current_cursor_position > 39)
	{
		totalPositionChanged++;
		updateMaxPos();
		updateMinPos();
		hertz = countTime;
		mySound();
		SetCursorPos(current_cursor_position-40);
		current_cursor_position -= 40;
		countTime = 1;
	}
}

void RightArrowPressed() // Place the cursor 1 cell to the right
{
	if(current_cursor_position%40 != 39)
	{
		totalPositionChanged++;
		updateMaxPos();
		updateMinPos();
		hertz = countTime;
		mySound();
		SetCursorPos(current_cursor_position+1);
		current_cursor_position += 1;
		countTime = 1;
	}
}

void LeftArrowPressed() // Place the cursor 1 cell to the left
{
	if (current_cursor_position%40 != 0)
	{
		totalPositionChanged++;
		updateMaxPos();
		updateMinPos();
		hertz = countTime;
		mySound();
		SetCursorPos(current_cursor_position-1);
		current_cursor_position -= 1;
		countTime = 1;
	}
}

void DownArrowPressed() // Place the cursor 1 cell downwards
{
	
	if (current_cursor_position < 960)
	{
		totalPositionChanged++;
		updateMaxPos();
		updateMinPos();
		hertz = countTime;
		mySound();
		SetCursorPos(current_cursor_position+40);
		current_cursor_position += 40;
		countTime = 1;
	}
}

void updateAttr(){ // updates the attribute according the current cell
			int pos = current_cursor_position*2;
			asm{ 
				PUSH 	AX
				PUSH 	ES
				PUSH 	SI
				PUSH 	BX
				MOV     AX,0B800h // Segment address of memory on color adapter
				MOV     ES,AX // Set up extra segment register
				MOV     SI,0 // Initial offset address into segment
				MOV 	SI,pos	// SI = cursor's position
				MOV 	BX,WORD PTR ES:[SI+1]	// Moving to BX the the attri
				MOV 	attr,Bl
				POP 	BX
				POP 	SI
				POP 	ES
				POP 	AX
			}
}

void Initialize() // Initializes the graphic screen
{
	origSoundLatch = readLatch();
	myMaxPosTime.countTime = 0;
	myMaxPosTime.pos = 0;
	myMinPosTime.countTime = 32767;
	myMinPosTime.pos = 0;
	currPosAndTime.countTime = 0;
	currPosAndTime.pos = 0;
	asm{
		PUSH 	AX
		MOV		AX,1 // 40 by 25 color image 
		INT		10h // Adapter initialized.Page 0 displayed
		POP		AX
	}
	SetCursorPos(999); // set the cursor position to the bottom right
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

void SetCursorPos(int position) // Move the cursor to the position
{
	asm{
		PUSH 	BX
		PUSH 	DX
		PUSH 	AX
		MOV		BX, position
		MOV		DX, 3D4H // Point to 3D4H - 3D5H port pair
		MOV		AL, 14 // Address of cursor register pos high byte
		MOV		AH, BH // Get desired value of cursor pos high byte
		OUT		DX, AX // Port(3D4h) = 14, Port(3D5h) = Value of BH

		MOV		AL, 15 // Address of cursor register pos low byte
		MOV		AH, BL // Get desired value pf cursor pos low byte
		OUT		DX, AX // Port(3D4h) = 15, Port(3D5h) = Value of BL
		POP 	AX
		POP 	DX
		POP 	BX
	}
}

void PrintChar(int pos, char input_ch) // Prints a char to "pos" at the screen
{
	int print_pos = pos*2;
	updateAttr();
		asm{ 
			PUSH 	AX 
			PUSH 	ES
			PUSH 	SI
			PUSH 	BX
			MOV     AX,0B800h // Segment address of memory on color adapter
			MOV     ES,AX 	// Set up extra segment register
			MOV     SI,0 	// Initial offset address into segment
			MOV 	SI,print_pos // Moving to SI the position on the screen we want to print to
			MOV 	BL,input_ch	// Moving to BL the char we want to print
			MOV		BH,attr		// Attributes
			MOV 	WORD PTR ES:[SI], BX	// Printing
			POP 	BX
			POP 	SI
			POP 	ES
			POP 	AX
		}
}

int readLatch() // save the orignal latch of the speakers
{
	int latch;
	asm	{
		CLI
		MOV AL, 0B6h
		OUT 43H, AL
		IN AL, 40H
		MOV BYTE PTR latch, AL
		IN AL, 40H
		MOV BYTE PTR latch+1, AL
		STI
	}
	return latch;
}

void mySound() // produce a sound according to the hertz var
{
	asm {
		PUSH AX
		MOV AL,0B6h
		OUT 43h,AL
		MOV AX,hertz
		AND AX,0FFh
		OUT 42h,AL
		MOV AX,hertz
		MOV AL,AH
		OUT 42h,AL
		POP AX
	}
}

void changeLatch() // change the latch of int8
{
		asm{
		CLI
		PUSH AX
		MOV AL, 036h
		OUT 43h,AL
		MOV AX,700
		OUT 40h,AL
		MOV AL,AH
		OUT 40h,AL
		POP AX
		STI
	} 
}

void makeSound() // turning on the speakers and produce sound 
{
	asm{ STI };
	changeSpeaker(ON);
	mySound();
}

void changeSpeaker( int status ) // change the speaker status-ON/OFF
{
  int portval = 0;
   asm {
        PUSH AX
        IN AL,61h
        MOV byte ptr portval,AL
        POP AX
       }
    if ( status==ON )
	   portval |= 0x03;
    else
       portval &=~ 0x03;
    asm {
        PUSH AX
        MOV AX,portval
        OUT 61h,AL
        POP AX
      }
}

void main()
{
	Initialize();
	changeLatch();
	int9Save = getvect(9);
	setvect(9, h9);
	int8Save = getvect(8);
	setvect(8, h8);
	while(1) { 
	makeSound();
	}
}
