/* kill.c - kill */
/* 8086 version */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <q.h>
#include <sleep.h>
#include "myheader.h"

/*------------------------------------------------------------------------
 *  kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(pid)
	int	pid;		     /* process to kill		*/
{
	struct	pentry	*pptr;	     /* points to proc. table for pid */
	int	ps;	             /* saved processor status	*/
        int pstate;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	freestk(pptr->pbase, pptr->plen);

	pstate = pptr->pstate;
	pptr->pstate = PRFREE;	

	if (pstate == PRCURR)
	  resched();
        else	
         if (pstate == PRWAIT)
         {
            semaph[pptr->psem].semcnt++;
	    dequeue(pid);
         } /* PRWAIT */
         else
         if (pstate == PRREADY)
	    dequeue(pid);
         else
         if (pstate == PRSLEEP)
         {
           if ( q[pid].qnext < NPROC ) /* qnext is not tail */
                q[q[pid].qnext].qkey += q[pid].qkey;  /* Account for the 
                                                              pid delay */
                dequeue(pid);
               /* The sleep queue may now be empty, or have a new first */
            	if ( slnempty = nonempty(clockq) ) 
                         sltop = &firstkey(clockq);

         } /*  PRSLEEP */
         else; /* default: PRSUSP, PRRECV  */
	/*new*/
	procArr[pid].firstCounter = 0;
	procArr[pid].nextCounter = 0;
	procArr[pid].parent = -1;
	procArr[pid].ppid = -1;
	/*new*/
	restore(ps);
	return(OK);

} /* kill */
