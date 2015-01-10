//=============================================================
// Mutex facility for TRT
// builds on top of exisiting semaphores
// by adding:
//  -- binary value, LOCK, UNLOCK
//  -- mutex can only be UNLOCKED by the task which LOCKED it
//  -- task which locks a mutex can optionally be set to have a early deadline
//     (to avoid contention for the mutex)
//
// Bruce Land. March 2009, Cornell University
//==============================================================

#define UNLOCKED 1
#define LOCKED 0
#define NONE 0

// define a mutex
struct {
		uint8_t owner; 	// the task which owns the mutex
		uint8_t state;
		} mutex[MAXNBRSEMAPHORES] ;

// trtInitMutex
// set all owners to zero (no owner)
void trtInitMutex(void) {
	uint8_t i ;
	//set all timers to disabled 
	for (i=0; i<MAXNBRSEMAPHORES; i++){
		mutex[i].owner = NONE ; 
		mutex[i].state = UNLOCKED ;
	} //for (i=0; i<MAXNBRSEMAPHORES; i++)
} //void trtInitMutex(void)

//==============================================================
// trtCreatMutex
// All mutex are created UNLOCKED with no owner
void trtCreateMutex(uint8_t mutex_number) {
	trtCreateSemaphore(mutex_number, UNLOCKED) ;
	mutex[mutex_number].state = UNLOCKED ;
	mutex[mutex_number].owner = NONE ;
} //void trtCreateMutex(uint8_t mutex_number)

//==============================================================
// trtLockMutex
// LOCK and record owner 
// If already LOCKED, force a context switch
void trtLockMutex(uint8_t mutex_number){
	cli();
	// if not locked
	if (mutex[mutex_number].state == UNLOCKED){
		mutex[mutex_number].state = LOCKED ;
		mutex[mutex_number].owner = kernel.running ;
	}//if (tmutex[mutex_number].state = UNLOCKED)
	else if (mutex[mutex_number].owner != kernel.running){
	 	trtWait(mutex_number) ;
	}
	sei();
} //void trtLockMutex(uint8_t mutex_number)

//==============================================================
// trtUnlockMutex
// if owner matches, UNLOCK
void trtUnlockMutex(uint8_t mutex_number){
	uint8_t *s;
	cli();
	s = &kernel.semaphores[mutex_number-1];
	if (kernel.running==mutex[mutex_number].owner){
		if (*s>0) *s=0; //enforce binary sem
		trtSignal(mutex_number);
		mutex[mutex_number].owner = NONE ;
		mutex[mutex_number].state = UNLOCKED ;
	}//if (kernel.running==mutex[mutex_number].owner)
	sei();
} //void trtUnlockMutex(uint8_t mutex_number)

//==============================================================
// trtQueryMutex
// returns state LOCK/UNLOCK
uint8_t trtQueryMutex(uint8_t mutex_number){
	return mutex[mutex_number].state ;
} //void trtQueryMutex(uint8_t mutex_number)

//==============================================================

// trtOwnerMutex
// returns 1 of current task is the owner
uint8_t trtOwnerMutex(uint8_t mutex_number){
	return (mutex[mutex_number].owner==kernel.running) ;
} //void trtQueryMutex(uint8_t mutex_number)

//==============================================================

