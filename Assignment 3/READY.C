/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "myHeader.h"

/*------------------------------------------------------------------------
 *  ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
int	ready (pid)
	int	pid;			/* id of process to make ready	*/
{
	register struct	pentry	*pptr;

	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	insert(pid,rdyhead,pptr->pprio);
	qFlag = 1; //needs to restart the rotations
	return(OK);
}
