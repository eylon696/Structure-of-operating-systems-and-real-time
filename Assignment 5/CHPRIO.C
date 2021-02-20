/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "myheader.h"

/*------------------------------------------------------------------------
 *  chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(pid,newprio)
	int	pid;
	int	newprio;		/* newprio > 0			*/
{
	int	oldprio;
	struct	pentry	*pptr;
        int	ps;

	disable(ps);
	if (isbadpid(pid) ||(newprio<= 0 && killFlag == 0) || (newprio< -1 && killFlag  == 1) || (pptr = &proctab[pid])->pstate == PRFREE) 
	{
		restore(ps);
		return(SYSERR);
	}
	oldprio = pptr->pprio;
	pptr->pprio = newprio;
	restore(ps);
	return(oldprio);
}










/*	------------------Original function for beckup-------------------
SYSCALL chprio(pid, newprio)
int	pid;
int	newprio;	
{
	int	oldprio;
	struct	pentry	*pptr;
	int	ps;

	disable(ps);
	if (isbadpid(pid) || newprio <= 0 ||
		(pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	oldprio = pptr->pprio;
	pptr->pprio = newprio;
	restore(ps);
	return(oldprio);
}
*/