#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <time.h>
#include "myHeader.h"



unsigned long int count1 = 0, count2 = 0, count3 = 0, count4 = 0;
xmain()
{
    int Inc(), Pr();
    resume( create(Inc, INITSTK, INITPRIO , "proc 1", 1, &count1) );
    resume( create(Inc, INITSTK, INITPRIO-5, "proc 2", 1, &count2) );
    resume( create(Inc, INITSTK, INITPRIO - 5, "proc 3", 1, &count3) );
    resume( create(Inc, INITSTK, INITPRIO - 10, "proc 4", 1, &count4) );
    resume( create(Pr, INITSTK, INITPRIO + 1, "proc 5", 0) );
}
Inc(int ptr)
{
  unsigned long int *ptr1;

  ptr1 = (unsigned long int *)ptr;
  while (1)
     (*ptr1)++;

}  /* Inc */

Pr()
{
  char str[80];

  while(1)
  {
    sleep(3);
    sprintf(str, "count1 = %lu, count2 = %lu, count3 = %lu,"
      " count4 = %lu\n",
                       count1, count2, count3, count4);
    printf(str);
   } /* while */

} /* Pr */
  


