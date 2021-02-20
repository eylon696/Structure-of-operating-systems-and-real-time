#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include "myheader.h"

//update pointers to next and first blocks and sizes 
 SYSCALL report_mem(int nbytes,int *firstN, char** firstPtr,int  *nextN, char** nextPtr)
{
	int ps;
	struct  mblock  *p, *q;
	disable(ps);
	if (nbytes==0) 
	{
		restore(ps);
		return(NULL);
	}
	//loop to find first fit block
	for (q=&memlist, p=q->mnext ;(char *)p != NULL ;q=p, p=p->mnext)
	{
		//kprintf("%d,%u\n",currpid,size);
		if(p->mlen >= nbytes)
		{
			*firstN = p->mlen;
			*firstPtr = (char*)p;
			break;
		}
	}
	//loop to find next fit block
	for (q=(struct  mblock*)savePtr, p=q->mnext ;((char *)p) != savePtr ;q=p, p=p->mnext)
	{	
		if((char *)p==NULL)
		{
			q=&memlist;
			p=q->mnext;
		}
		if(p->mlen >= nbytes)
		{
			*nextN = p->mlen;
			*nextPtr = (char*)p;
			break;
		}
	}
restore(ps);
return(OK); 
} /* for */