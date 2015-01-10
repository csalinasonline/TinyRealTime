//========================================================
// Define timers for use with TRT
// Bruce Land, Feb 2009
//
//========================================================

#define MAXNBRTIMERS 3

// Three values for TIMERTICK 0.1 mS or 1 mS or 10 mS
// only choose one value
#define TIMERTICK 10 
//#define TIMERTICK 1
//#define TIMERTICK 0.1
#define SECONDS2TIMERTICK(t) ((t)*1000/TIMERTICK)

// bit definitions for the mode byte
#define ENABLEBIT 7 // =1 if enabled
#define RUNBIT 0 	// =1 if running
#define MODEBIT 1 	//periodic=0 oneshot=1
#define PERIODIC 0
#define ONESHOT 1

#define READ(U, N) ((U) >> (N) & 1u)
#define SET(U, N) ((void)((U) |= 1u << (N)))
#define CLR(U, N) ((void)((U) &= ~(1u << (N))))
#define FLIP(U, N) ((void)((U) ^= 1u << (N)))

// define a timer
struct {
		uint16_t period; 	// the timeout value
		uint16_t count;		// the current value
		uint16_t Nperiods;	// number of timeouts which have occured
		uint8_t status;		// running/stopped, enabled/disabled, periodic/oneshot
		uint8_t semaphore;	// what to signal: zero means no signal 
		} timers[MAXNBRTIMERS] ;


//====================================================
//timer 0 overflow ISR
ISR (TIMER0_COMPA_vect) {
	uint8_t i ;   
 	// Handle array of soft timers
  	for (i=0; i<MAXNBRTIMERS; i++){
		// for all enabled timers
		if (READ(timers[i].status,ENABLEBIT)){
			// if they are running
			if (READ(timers[i].status,RUNBIT)) {
				if (timers[i].count > 0) {
					timers[i].count-- ;
				}//if (timers[i].count > 0)
				else { // timer count==0
					timers[i].Nperiods++ ;
					//reset for next cycle
					timers[i].count = timers[i].period-1;
					// signal if a semaphore is defined
					if (timers[i].semaphore>0) trtSignal(timers[i].semaphore);
					// stop the timer if mode==ONESHOT
					if (READ(timers[i].status, MODEBIT)) CLR(timers[i].status,RUNBIT);
				}// else timer count == 0
			}//if (timers[i].status & (1<<RUNBIT))
		}//if (READ(timers[i].status,ENABLEBIT))

	} //for (i=0; i<MAXNBRTIMERS; i++)
} //ISR  


//====================================================
void trtInitTimer(void) {
	uint8_t i ;

	// set up 0.1 mS tick
	if (TIMERTICK==0.1){
		//set up timer 0 for 1 mSec timebase 
  		TIMSK0= (1<<OCIE0A);	//turn on timer 0 cmp match ISR 
  		OCR0A = 24;  		//set the compare 250 time ticks
  		//set prescalar to divide by 64 
  		TCCR0B= 3; //0b00000011;	
  		// turn on clear-on-match
  		TCCR0A= (1<<WGM01) ;
	} //if (TIMERTICK==0.1)

	// set up 1 mS tick
	if (TIMERTICK==1){
		//set up timer 0 for 1 mSec timebase 
  		TIMSK0= (1<<OCIE0A);	//turn on timer 0 cmp match ISR 
  		OCR0A = 249;  		//set the compare 250 time ticks
  		//set prescalar to divide by 64 
  		TCCR0B= 3; //0b00000011;	
  		// turn on clear-on-match
  		TCCR0A= (1<<WGM01) ;
	} //if (TIMERTICK==1)
	
	// set up 10 mS tick
	if (TIMERTICK==10){
		//set up timer 0 for 10 mSec timebase 
  		TIMSK0= (1<<OCIE0A);	//turn on timer 0 cmp match ISR 
  		OCR0A = 155;  		//set the compare 156 time ticks
  		//set prescalar to divide by 1024 
  		TCCR0B= 5; //0b00000101;	
  		// turn on clear-on-match
  		TCCR0A= (1<<WGM01) ;
	} //if (TIMERTICK==10)
	
	//set all timers to disabled 
	for (i=0; i<MAXNBRTIMERS; i++){
		CLR(timers[i].status, ENABLEBIT); 
	} //for (i=0; i<MAXNBRTIMERS; i++)

} //void trtInitTimer

//====================================================
void trtSetTimer(uint8_t timer_number, 
				uint16_t period, 
				uint8_t mode,
				uint8_t sem) {

	// set period
	timers[timer_number-1].period = period ;
	// set count=period-1 (zero based)
	timers[timer_number-1].count = period-1 ;
	// set status to (STOP | mode)
	CLR(timers[timer_number-1].status, RUNBIT) ;
	if(mode==PERIODIC) CLR(timers[timer_number-1].status, MODEBIT) ;
	else SET(timers[timer_number-1].status, MODEBIT) ;
	// set Nperiod to zero
	timers[timer_number-1].Nperiods = 0 ;
	// set semaphore
	timers[timer_number-1].semaphore = sem ;
	// enable timer
	SET(timers[timer_number-1].status, ENABLEBIT);
} //void trtSetTimer

//====================================================
void trtStartTimer(uint8_t timer_number){
	SET(timers[timer_number-1].status, RUNBIT) ;
}

//====================================================
void trtStopTimer(uint8_t timer_number){
	CLR(timers[timer_number-1].status, RUNBIT) ;
}
//====================================================
void trtDisableTimer(uint8_t timer_number){
	CLR(timers[timer_number-1].status, ENABLEBIT) ;
}

//====================================================
uint8_t trtStatusTimer(uint8_t timer_number){
	return timers[timer_number-1].status ;
}

//====================================================
uint16_t trtNumPeriods(uint8_t timer_number){
	return timers[timer_number-1].Nperiods ;
}
//====================================================



