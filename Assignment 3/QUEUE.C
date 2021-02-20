/* queue.c - dequeue, enqueue */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include "myHeader.h"

/*------------------------------------------------------------------------
 *  enqueue  --  insert an item at the tail of a list
 *------------------------------------------------------------------------
 */
int enqueue(item, tail)
int item;				/* item to enqueue on a list	*/
int tail;				/* index in q of list tail	*/
{
	struct	qent	*tptr;		/* points to tail entry		*/
	struct	qent	*mptr;		/* points to item entry		*/

	tptr = &q[tail];
	mptr = &q[item];
	mptr->qnext = tail;
	mptr->qprev = tptr->qprev;
	q[tptr->qprev].qnext = item;
	tptr->qprev = item;
	return(item);
}


/*------------------------------------------------------------------------
 *  dequeue  --  remove an item from a list and return it
 *------------------------------------------------------------------------
 */
int dequeue(item)
int item;
{
	struct	qent	*mptr;		/* pointer to q entry for item	*/

	mptr = &q[item];
	if(ifItemRdy(item)){
		qFlag = 1; //needs to restart the rotations
	}
	q[mptr->qprev].qnext = mptr->qnext;
	q[mptr->qnext].qprev = mptr->qprev;
	return(item);
}
//checks if the id is in the ready queue
int ifItemRdy(int id)
{
	int next;
	next = q[rdyhead].qnext;
		while(next != rdytail)
		{
			if(next == id)
				return 1;
			next = q[next].qnext;
		}
	return 0;
}
