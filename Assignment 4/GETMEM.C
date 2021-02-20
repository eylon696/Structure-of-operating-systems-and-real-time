/* getmem.c - getmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include "myheader.h"
/*------------------------------------------------------------------------
 *  getmem  --  allocate heap storage, returning lowest integer address
 *------------------------------------------------------------------------
 */

char *getmem(nbytes)
word nbytes;
{
	int	ps;
	struct	mblock	*p, *q, *leftover;
	int firstN, nextN;
	char *firstPtr = NULL, *nextPtr = NULL;
	int flagFirst = 0, flagNext = 0;
	disable(ps);
	if ( nbytes==0 ) 
	{
		restore(ps);
		return( NULL );
	}
	nbytes = roundew(nbytes);
	report_mem(nbytes,&firstN,&firstPtr,&nextN,&nextPtr);
	if(firstPtr==NULL && nextPtr==NULL)
	{
		restore(ps);
		return( NULL );
	}
	/*new*/
	//the process was create by "create function" or is a parent
	if((procArr[currpid].parent == -1)||(procArr[currpid].parent == 1))
	{
		/*new*/
		if((firstN <= nextN)){
			flagFirst = 1;
			procArr[currpid].firstCounter++;
		}
		else
		{
			flagNext = 1;
			procArr[currpid].nextCounter++;
		}
		for (q=&memlist, p=q->mnext ;(char *)p != NULL ;q=p, p=p->mnext){
			if(flagFirst && (char *)p == firstPtr)
				break;
			if(flagNext && (char *)p == nextPtr )
				break;
		}
		if (p->mlen == nbytes) 
		{
			q->mnext = p->mnext;
			savePtr=(char *)(p->mnext);//found a block to alocate so we update savePtr to this block for next fit algorithm
			restore(ps);
			return( (char *) p );
		}
		else if(p->mlen > nbytes)
		{
			leftover = (struct mblock *)( (char *)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			savePtr=(char *)leftover;//found a block to alocate so we update savePtr to this block for next fit algorithm
			restore(ps);
			return( (char *) p );
		}
	restore(ps);
	return( NULL );
	}
	//the process is a child
	else if(procArr[currpid].parent==0)
	{
		//if-else: update the memory allocation function according to the counters (first/next) of father
		if(procArr[procArr[currpid].ppid].firstCounter>=procArr[procArr[currpid].ppid].nextCounter){
			flagFirst = 1;
			procArr[currpid].firstCounter = 1;
			procArr[currpid].nextCounter = 0;
		}
		else{
			flagNext = 1;
			procArr[currpid].firstCounter = 0;
			procArr[currpid].nextCounter = 1;
		}
		//loop to get the right block
		for (q=&memlist, p=q->mnext ;(char *)p != NULL ;q=p, p=p->mnext)
		{
			if(flagFirst && (char *)p == firstPtr)
				break;
			if(flagNext && (char *)p == nextPtr )
				break;
		}
		if (p->mlen == nbytes) 
		{
			q->mnext = p->mnext;
			savePtr=(char *)p->mnext;//found a block to alocate so we update savePtr to this block for next fit algorithm
			restore(ps);
			return( (char *) p );
		}
		else if(p->mlen > nbytes)
		{
			leftover = (struct mblock *)( (char *)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			savePtr=(char *)leftover;//found a block to alocate so we update savePtr to this block for next fit algorithm
			restore(ps);
			return( (char *) p );
		}
	}
restore(ps);
return( NULL );
}

