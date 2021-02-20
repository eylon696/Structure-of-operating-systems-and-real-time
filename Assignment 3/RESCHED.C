/* resched.c - resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <time.h>
#include <q.h>
#include <stdio.h>
#include "myHeader.h"



/*------------------------------------------------------------------------
 *  resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRCURR.
 *------------------------------------------------------------------------
 */
 
volatile int hour,min,sec;


int stack[NPROC]; 
int top = -1;  
int numOfPrio=0; 

//returns the number of diffrent priorities (num of secondary rotation)
int getDifPrio()
{
	int counter = 1;
	int next;
	int temp,temp1;
	next = q[rdyhead].qnext;
	temp = proctab[next].pprio;
		while(next != rdytail)
		{
			next = q[next].qnext;
			if (next!= rdytail)
			{
			temp1 = proctab[next].pprio;
			if(temp != temp1 && next > 3)
				{
				counter++;
				temp = temp1;
				}
			}
		}
	return counter;
} 

int stackIsEmpty() {
   if(top == -1)
      return 1;
   return 0;
}

int stackIsFull() {

   if(top == NPROC)
      return 1;
   return 0;
}

int pop() {
   int data;
	
   if(!stackIsEmpty()) {
      data = stack[top];
      top = top - 1;   
      return data;
   } else {
      printf("Could not retrieve data, Stack is empty.\n");
   }
   return -1;
}

void push(int data) {

   if(!stackIsFull()) {
      top = top + 1;   
      stack[top] = data;
   } else {
      printf("Could not insert data, Stack is full.\n");
   }
}
//empting the stack into the ready queue 
void stackToReadyQ(){
	int proccessID;
	int next;
	while(!stackIsEmpty()){
		proccessID=pop();
		insert(proccessID,rdyhead,proctab[proccessID].pprio);
	}
}
//Checks if adjacent pids have the same priotity 
int checkSamePrio(int pid,int prio){
	int prevPrio;
	int nextPrio;
	if(q[pid].qprev!=rdyhead)
		prevPrio=proctab[q[pid].qprev].pprio;
	if(q[pid].qnext!=rdytail)
		nextPrio=proctab[q[pid].qnext].pprio;
	return (prevPrio==prio || nextPrio==prio);
}

 
 
 int  convert_to_binary(int x)
{
 int i;
 int temp, scale, result;
 

  temp =0;
  scale = 1;
  for(i=0; i < 4; i++)
   {
     temp = temp + (x % 2)*scale;
     scale *= 2;
     x = x >> 1;
   } // for

  result = temp;
  temp = 0;

  scale = 1;
  for(i=0; i < 4; i++)
   {
     temp = temp + (x % 2)*scale;
     scale *= 2;
     x = x >> 1;
   } // for

  temp *= 10;
  result = temp + result;
  return result;

} // convert_to_binary
 
//read the clock from RTC 
void readclk()
{
  int i;
  hour=min=sec=0;
  
  asm {
   PUSH AX
   MOV AL,0
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR sec,AL
;
   MOV AL,2
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR min,AL
;
   MOV AL,4
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR hour,AL
;
   POP AX
  } // asm

  sec = convert_to_binary(sec);
  min = convert_to_binary(min);
  hour = convert_to_binary(hour);


} // readclk
 
 
int	resched()
{
	int localTime = 0;
	int node;
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */
	int k,numberOfCycles=0;
	optr = &proctab[currpid];
	readclk();
	localTime = hour*3600 + min*60 + sec;
/*------------------------------------------------------------------------
 *original resched begins 
 *------------------------------------------------------------------------
 */
	if(localTime<(18*3600) || localTime>(22*3600))
	{
	if ( optr->pstate == PRCURR ) 
         {
		/* no switch needed if current prio. higher than next	*/
		/* or if rescheduling is disabled ( pcxflag == 0 )	*/
		if ( sys_pcxget() == 0 || lastkey(rdytail) < optr->pprio
                 || ( (lastkey(rdytail) == optr->pprio) && (preempt > 0) ) )
			return;
		/* force context switch */
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
		} /* if */ 
        else if ( sys_pcxget() == 0 ) 
            {
		kprintf("pid=%d state=%d name=%s",
			currpid,optr->pstate,optr->pname);
		panic("Reschedule impossible in this state");
	    } /* else if */

	
	/* remove highest priority process at end of ready list */
	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
	preempt = QUANTUM;		/* reset preemption counter	*/
	ctxsw(&optr->pregs,&nptr->pregs);
	/* The OLD process returns here when resumed. */
	return;
	}
/*------------------------------------------------------------------------
 *original resched ends and new one begins 
 *------------------------------------------------------------------------
 */
	else
	{
	optr = &proctab[currpid];
	if(qFlag || numberOfCycles == 0){
		qFlag = 0;
		numberOfCycles = getDifPrio();
	}
	if ( optr->pstate == PRCURR ) 
         {
		/* no switch needed if current prio. higher than next	*/
		/* or if rescheduling is disabled ( pcxflag == 0 )	*/
		if ((sys_pcxget() == 0) || (preempt > 0))	
		{
			return;
		}
		/* force context switch */
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
		
		if(currpid>3 ){	
			if(checkSamePrio(currpid,proctab[currpid].pprio)==0)
				cycleCounter++;
			dequeue(currpid);
			push(currpid);	
			numberOfCycles--;
			if(cycleCounter==currentCycle)
			{
				cycleCounter=0;
				stackToReadyQ();
				currentCycle++;
			}			
		}	
		if(numberOfCycles==0)
		{
			stackToReadyQ();
			currentCycle=1;
			cycleCounter=0;
		}	
	} /* if */ 
	else if ( sys_pcxget() == 0 ) 
	{
		kprintf("pid=%d state=%d name=%s",currpid,optr->pstate,optr->pname);
		panic("Reschedule impossible in this state");
	} /* else if */
	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
	if(nptr->pprio%25 == 0 && nptr->pprio!= 0 )
		preempt = nptr->pprio/25;
	else
		preempt = nptr->pprio/25 + 1;
	ctxsw(&optr->pregs,&nptr->pregs);
	/* The OLD process returns here when resumed. */
	return;
	}
}


