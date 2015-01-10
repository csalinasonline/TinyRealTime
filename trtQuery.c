//---------------------------------------------------------------
// TRT utilities
// For the Mega644 by Bruce Land Feb 2010
// -- Read out kernel state
// -- Read out semaphore state
// -- Read out mutex state
//
// Assumes that the UART has been initialized and that a terminal
// is attached!
//
// All routines may STOP the scheduler for a few hundred millseconds
// and may freeze all ISRs and timers depending on second parameter:
// which takes values FREEZE_TIMERS or RUN_TIMERS
//---------------------------------------------------------------

//struct task {
 // uint8_t spl;       // Stack pointer lo8
 // uint8_t sph;       // Stack pointer hi8
 // uint32_t release;
 // uint32_t deadline;
 // uint8_t state; 

#define FREEZE_TIMER 1
#define RUN_TIMER 0

//============================================
uint8_t trtTaskState(char tsk) 
{
	struct task *t;
	t = &kernel.tasks[tsk]; // struct field syntax t->state 
	return t->state ;
}

//============================================
uint16_t trtTaskStack(char tsk) 
{
	struct task *t;
	t = &kernel.tasks[tsk]; // struct field syntax t->state 
	return ((t->sph)<<8)+(t->spl) ;
}

//============================================
uint16_t trtTaskStackBottom(char tsk) 
{
	struct task *t;
	t = &kernel.tasks[tsk]; // struct field syntax t->state 
	return t->stack_bottom ;
}

//============================================
uint32_t trtTaskRelease(char tsk) 
{
	struct task *t;
	t = &kernel.tasks[tsk]; // struct field syntax t->state 
	return t->release ;
}

//============================================
uint32_t trtTaskDeadline(char tsk) 
{
	struct task *t;
	t = &kernel.tasks[tsk]; // struct field syntax t->state 
	return t->deadline ;
}

//============================================
uint16_t trtTaskStackFreeMin(char tsk)
{
	struct task *t;
	uint16_t stk;
	uint8_t *i ;
	t = &kernel.tasks[tsk]; // struct field syntax t->state 
	stk = ((t->sph)<<8)+(t->spl) ;
	// stack is init to 0xaa by trtCreateTask
	for (i=t->stack_bottom; i<stk; i++)
	{
		if (*i != 0xaa) break;
	}
	return i - t->stack_bottom ;
}

//============================================
uint8_t trtSemValue(uint8_t sem)
{
	uint8_t *s;
	s = &kernel.semaphores[sem-1];
	return *s ;
}

#ifdef UNLOCKED
//============================================
uint8_t trtMutexOwner(uint8_t mut)
{
	return mutex[mut].owner ;
}

//============================================
uint8_t trtMutexState(uint8_t mut)
{
	return mutex[mut].state ;
}

#endif

//============================================
void trtTaskDump(char tsk, char freeze_timers)
{
	char timer0_tccr0b, timer1_tccr1b, timer2_tccr2b ;
	char i ;
	if (freeze_timers) 
	{
		cli(); // disable interrupts
		// and stop timers
		timer0_tccr0b = TCCR0B ;
		TCCR0B = 0 ;
		timer1_tccr1b = TCCR1B ;
		TCCR0B = 0 ;
		timer2_tccr2b = TCCR2B ;
		TCCR0B = 0 ;
		sei();
	}
	fprintf(stdout, "# state     relT   deadT stkfree stkfreeMin\n\r") ;
	
	if (tsk == 0) 
	{
		//loop thru all tasks
		for (i=1; i <= kernel.nbrOfTasks; i++) 
		{
	      	fprintf(stdout, "%1d %4d %8ld %8ld %5d %5d\n\r", 
				i, 
				trtTaskState(i), 
				trtTaskRelease(i) - trtCurrentTime(), 
				trtTaskDeadline(i) - trtCurrentTime(),
				trtTaskStack(i) - trtTaskStackBottom(i),
				trtTaskStackFreeMin(i) ) ;
	    }
	}
	else 
	{
	    fprintf(stdout, "%1d %4d %8ld %8ld %5d\n\r", 
				tsk, 
				trtTaskState(tsk), 
				trtTaskRelease(tsk) - trtCurrentTime(), 
				trtTaskDeadline(tsk) - trtCurrentTime(),
				trtTaskStack(tsk) - trtTaskStackBottom(tsk),
				trtTaskStackFreeMin(tsk) ) ;
	}
	
	if (freeze_timers) 
	{
		// restore timer state
		TCCR0B = timer0_tccr0b ;
		TCCR1B = timer1_tccr1b ;
		TCCR2B = timer2_tccr2b ;
	}
	
}

//============================================
void trtSemDump(char sem, char freeze_timers)
{ 
	char timer0_tccr0b, timer1_tccr1b, timer2_tccr2b ;
	char i ; 
	if (freeze_timers) 
	{
		cli(); // disable interrupts
		// and stop timers
		timer0_tccr0b = TCCR0B ;
		TCCR0B = 0 ;
		timer1_tccr1b = TCCR1B ;
		TCCR0B = 0 ;
		timer2_tccr2b = TCCR2B ;
		TCCR0B = 0 ;
		sei();
	}

	fprintf(stdout, "# value\n\r") ;

	//loop thru all semaphores
	if (sem == 0)
	{
		for (i=1; i <= MAXNBRSEMAPHORES; i++) 
		{
	      	fprintf(stdout, "%1d %4d\n\r", i, trtSemValue(i) ) ;
	    }
	}
	else
	{
	     fprintf(stdout, "%1d %4d\n\r", sem, trtSemValue(sem)) ;
	}

	if (freeze_timers) 
	{
		// restore timer state
		TCCR0B = timer0_tccr0b ;
		TCCR1B = timer1_tccr1b ;
		TCCR2B = timer2_tccr2b ;
	}
}

#ifdef UNLOCKED
//============================================
void trtMutexDump(char mut, char freeze_timers)
{
	char timer0_tccr0b, timer1_tccr1b, timer2_tccr2b ;
	char i ;
	
	if (freeze_timers) 
	{
		cli(); // disable interrupts
		// and stop timers
		timer0_tccr0b = TCCR0B ;
		TCCR0B = 0 ;
		timer1_tccr1b = TCCR1B ;
		TCCR0B = 0 ;
		timer2_tccr2b = TCCR2B ;
		TCCR0B = 0 ;
		sei();
	}

	fprintf(stdout, "# owner UNLOCK\n\r") ;
	
	if (mut == 0)
	{
	//loop thru all mutexs
		for (i=1; i <= MAXNBRSEMAPHORES; i++) 
		{
	      	fprintf(stdout, "%1d %4d %5d\n\r", 
				i, trtMutexOwner(i), trtMutexState(i)) ;
		}
	}
	else
	{
		fprintf(stdout, "%1d %4d %5d\n\r", 
				mut, trtMutexOwner(mut), trtMutexState(mut)) ;
	}
	
	if (freeze_timers) 
	{
		// restore timer state
		TCCR0B = timer0_tccr0b ;
		TCCR1B = timer1_tccr1b ;
		TCCR2B = timer2_tccr2b ;
	}
	
}

//============================================
#endif
