#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <io.h>
#include "myheader.h"


int(*saveOrigIsr4)(int);
int(*saveOrigIsr8)(int);


//change prio to -1 and set new isr8
 void terminate_on_replace()
 {
	 int ps;
	 disable(ps);
	 killFlag = 1;
	 chprio(getpid(), -1);
	 setNewVect(8, myIsr8);
	 restore(ps);
 }


//turn on terminate_on_replace
 INTPROC myIsr4(int mdevno)//check mdevno
 {
	 kprintf("Process %d overflow\n", getpid());
	 terminate_on_replace();
	 return 0;
 }

//restore the original isr8 if needed
 INTPROC myIsr8(int mdevno)
 {
	 int ps;
	 saveOrigIsr8(mdevno);
	 disable(ps);
	 if (q[q[(rdyhead)].qnext].qkey == -1 && killFlag == 1)
	 {
		 kill(firstid(rdyhead));
		 killFlag = 0;
		 setOldVect(8);
	 }
	 restore(ps);
	 return 0;
 }


//set new isr 4 or 8
void setNewVect(int intNum, int(*newisr)(int))
{
	int i;
	
	for (i = 0; i < 32; i++)
		if (sys_imp[i].ivec == intNum)
		{
			if(intNum == 4)
				saveOrigIsr4 = sys_imp[i].newisr;
			else if(intNum == 8)
				saveOrigIsr8 = sys_imp[i].newisr;
			sys_imp[i].newisr = newisr;
			return;
		} 
} 


//restore original isr
void setOldVect(int intNum)
{
	int i;

	for (i = 0; i < 32; i++)
		if (sys_imp[i].ivec == intNum)
		{
			if(intNum == 4)
				sys_imp[i].newisr = saveOrigIsr4;
			else if(intNum == 8)
				sys_imp[i].newisr = saveOrigIsr8;
			return;
		}
}


void tested_sum(long int *dest, long int x, long int y)
{
	*dest = x + y;
	asm { into};

} //tested_sum

unsigned long int count1 = 0, count2 = 0, count3 = 0;
xmain()
{
	int Inc(),Inc2(), Pr();

	resume( create(Inc2, INITSTK, INITPRIO, "proc 1", 1, &count1) );
	resume( create(Inc, INITSTK, INITPRIO, "proc 2", 1, &count2) );
	resume( create(Inc, INITSTK, INITPRIO, "proc 3", 1, &count3) );

	resume( create(Pr, INITSTK, INITPRIO + 1, "proc 4", 0) );
}

Inc(int ptr)
{
	unsigned long int *ptr1;

	ptr1 = (unsigned long int *)ptr;
	while (1)
	{
		(*ptr1)++;
	}
}  /* Inc */

Inc2(int ptr)
{
	unsigned long int *ptr1;

	ptr1 = (unsigned long int *)ptr;
	while (1)
	{
	//         address 0   1     
	tested_sum(ptr1, *ptr1, 1);
	}
}  /* Inc */


Pr()
{
	char str[80];

	while(1)
	{
		sleep(3);
		sprintf(str, "count1 = %lu, count2 = %lu, count3 = %lu\n",count1, count2, count3);
		printf(str);
	} /* while */
} /* Pr */
