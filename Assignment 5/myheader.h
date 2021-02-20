extern void terminate_on_replace();							
extern int killFlag;
extern void setOldVect(int intNum);
extern void setNewVect(int intNum, int (*newisr)(int));	 
extern int(*saveOrigIsr4)(int);	
extern int(*saveOrigIsr8)(int);		
extern int myIsr4(int mdevno);
extern int myIsr8(int mdevno);	
