/* ATTENTION: THIS CODE INCLUDES SAMPLE CODE NOT AUTHORED BY ME. 
 * ALL CREDITS FOR THAT CODE TO ORIGINAL AUTHOR RICHARD ANTHONY (R.A.)
 * I HAVE ONLY USED AND MODIFIED CODE WRITTEN BY HIM TO BETTER SUIT MY PROJECT.
 * THE BORROWED CODE CAN BE FOUND IN "BorrowedMisc_code.h".
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000L
#include <util/delay.h>
#include <stdbool.h>

// Defining our password size: (PS! Remember to set password of correct length in "passw" array as well)
#define	passw_size	4
// Defining lockdown time in seconds
#define lockdown_time 60
// Defining timeout time in seconds
#define timeout_time 15

// Variable to hold state value.
volatile int nextState = 0;

// Variable that gets true if timer interrupt occured
volatile bool _intrFlag = false;

// Variable that gets true if push button interrupt occured (i.e. lockdown mode for 1 min).
volatile bool _intrFlag_EG = false;

// Variable to be indirectly used in conjunction with TIMER1, to display timer on LCD
volatile int _timer1 = timeout_time; // The value is 15 seconds so users have enough time to input password.

// Variable to be indirectly used in conjunction with TIMER3, to display timer on LCD
volatile int _timer3 = lockdown_time; // The value is 60 seconds for lockdown mode.


#include "BorrowedMisc_code.h" // ATTENTION: ALL CODE IN THIS HEADER FILE IS ORIGINALLY WRITTEN BY R. ANTHONY. ALL CREDITS GO TO HIM.

// This library is where I've stored the declaration of my methods and keywords:
#include "Singh_library.h"


// Array to hold our predefined password. P1 means switch 1 (upper leftmost switch on keypad), P2 means switch 2 and so on.
 unsigned char passw[passw_size] = {P1, P1, P1, P1};
	
// Array to hold our input (switch presses, e.g. S1, S2 and so forth)
unsigned char input[passw_size];


int main(void)
{	
	// Initial (general) Setup. Consists of sub-setups.
	IS();

	// Filling the input array with null=0xFF values
	for (int i = 0; i < passw_size; i++){input[i]=null;}
	
	// Declaring a pointer to our input array. This pointer can be used to pass in functions instead of whole array.
	unsigned char (*p_input)[passw_size] = &input;
	
	// Declaring a pointer to our password array. This pointer can be used to pass in functions instead of whole array.
	unsigned char  (*p_passw)[passw_size] = &passw;
    
	
    while (1) 
    {
		// Implementing state machine:		
		switch (nextState)
		{
			case 0: /* STATE 0: THIS STATE SHOWS "INPUT PASSWORD" ON LCD 
			AND REGISTERS THE KEY PRESSES TO ARRAY "input". */
			
			/* In order to write to the screen, we first clear the screen, 
			then set the cursor to left, upper corner and write our sentence*/
			
			// Clearing screen:
			LCD_Clear();
			// Setting the cursor:
			LCD_Home();
			
			// Write to the top row: "Input Password:"
			LCD_WriteString(" Input Password");
			LCD_SetCursorPosition( 6 /*0 - 40 */,  1 /*0 for top row, 1 for bottom row*/);
			LCD_WriteString("Time: 15s");
			
			// Resetting _timer1 and _timer3:
			_timer1 = timeout_time;
			_timer3 = lockdown_time;
			
			/* In order to register the key presses to an array, we start with a for loop. Inside the for-loop, before a switch press is parsed into our "input" array,
			 we check whether there has been any interrupts signaling that 15 seconds has passed yet. If there are no flags, then we proceed with registering switch press 0-3 
			 to their respective places in array "input".
			 To see how AssignSV works, see its implementation in the header file "Singh_KeyPad.h".
			 
			 After that, program checks whether the statevalue is the same, i.e. if a decision to change states has been made or not. If the statevalue is the same (nextState = 0)
			 stars get printed to LCD for each digit of the password stored in "input" array. If not, then it proceeds to the next if-statement. That is, if for-loop variable
			 'i' and the statevalue 'nextState' is equal to 0 (i.e. first switch press and we are to stay in state 0), then a timer starts. Before it is started, it is reset, just to
			 be sure that nothing is faulty. This is timer 1, a 16-bit timer that needs no overflow counting in order to count to 1 or 15 seconds. With timer 0, overflow counting and remainder
			 'ticks' are necessary. These things mess up the math since they incl. decimal points. Therefore I used timer 1. The way I have chosen to apply this timer is to make it count 1 second,
			 display that second on the LCD (as countdown) and issue a timeout after 15 seconds. See .h file "Singh_KeyPad.h" for the timer's declaration and implementation. A interrupt service 
			 routine has been made for the timer. The ISR is called whenever timer counts to 1 (in CTC mode). See the bottom of this file for ISRs.
			*/
				for (int i = 0; i < passw_size; i++)
					{
						
						if (!_intrFlag && !_intrFlag_EG){
							input[i]=AssignSV(); // Program waits (while-loop) in AssignSV() until a switch press has been detected. 
							// This function returns that switch press to input[i].		
							
							
							if (nextState==0 && !_intrFlag_EG){
								LCD_SetCursorPosition( i+1 /*0 - 40 */,  1 /*0 for top row, 1 for bottom row*/); // PS! CODE BORROWED FROM R.A. ALL CREDITS TO HIM.
								LCD_WriteString("*");
								}
							
							if (i==0 && nextState==0){

									ResetTimer1();
									
									StartTimer1(); // ISR for TIMER1 is responsible for updating LCD with timer countdown.
									
								}							
						} 						
					}
				/* The only way to get out of for-loop is if the 15second interrupt happens, or 4 digits were pressed (doesn't matter if they're correct or not)
				After the for-loop, the timeout timer is reset. After that, the program checks if the interruptflag for the 15s timer and/or for the push button (lockdown mode)
				is set or not. If not set, i.e. input timelimit was not reached or push button not pushed, the program is to proceed to next state (checking the password). 
				The following nested if-statements checks whether any interrupt flags have been set. "_intrFlag" is an interrupt flag for 
				*/
					
				ResetTimer1();
			
				if (_intrFlag || _intrFlag_EG)
				{
					if(_intrFlag && !_intrFlag_EG){	// If the timeout flag is set but not the push button flag, we deny access by going to state 4 next, clear the flag				
						// Deny access:
						nextState = 4;
						
						// Clear interrupt flag
						_intrFlag = false;
				
					} else if(!_intrFlag && _intrFlag_EG){ // If the push button flag is set but not the timeout flag, we go into lockdown(State2) and clear the flag.
							// Go into Lockdown mode for 1 minute:
							nextState = 2;
						
							// Clear interrupt flag
							_intrFlag_EG = false;
						
						} else if (_intrFlag && _intrFlag_EG){ // In the odd case that timeout flag and push button flag is set, we go into lockdown as previous if-statement shows
								// Go into Lockdown mode for 1 minute:
								nextState = 2;
								
								// Clear interrupt flag
								_intrFlag_EG = false;
								// Clear interrupt flag
								_intrFlag = false;
							}
					
				}	else{ // if no interrupts have been triggered at all, we go on to check the input password in state 1
					
					// Go to state 1 to check input password
					nextState = 1;
					
					}			
				break; /* END OF STATE 0 */
			
			
			
			case 1: /* The purpose of this state is to check the input on keypad against predefined password in array "passw".
			After password is checked, system goes to state 3 or 4, depending if access was given or not.
			*/
				CheckPassword((*p_passw), (*p_input));
			
				break; /* END OF STATE 1 */
			
			
			
			
			
			case 2: // The purpose of this state is to set the system in lockdown mode for 1 minute and then restart program (state 0)------------------------------
			
				/* ISR for TIMER3 gets called for every 1 sec after button is pressed. Push button is connected to INT4 (PE4) and once button is pressed,
				the interrupt service routine for INT4 gets called. The ISR for INT4 starts timer3 if global variable _timer3=60. ISR for timer 3 takes
				care of displaying timer on LCD. Keep in mind that the whole period we are in lockdown mode, we are actually in this state (state 2).
				The only thing that is happening in this state is ISR for timer3 is called every sec, and we are executing the code below. The code below
				checks whether global variable _timer3 is equal to 0, since it decrements with 1 for every ISR TIMER3 call. 
				That is why _timer3 variable is initialised at 60 before main function.
				If _timer3 is equal to 0, meaning lockdown is done, we restart program (state 0) and reset _timer3 to 60.
				*/
			
				// if the counter has reached 60 to 0
				if (_timer3 == 0)
				{
					// Start program again, that is state 0
					nextState = 0;
				
					// Set timer3 to 60 again
					_timer3=lockdown_time;
				}
			
				break;
			
			case 3: // STATE 3: ACCESS GRANTED------------------------------------------------------------------------------------------

				AccessGranted();
			
				// Checking which state to go to:
				if (_intrFlag_EG) // If push button flag is set
				{
					// Go to lockdown state
					nextState = 2;
				
					// Clear interrupt flag
					_intrFlag_EG = false;
				
					break;
				
					} 
				
				// "Open door" by showing that servo motor is turning.	
				SpinMotor();	
					
				// Go to state 0
				nextState = 0;
					
				break;		
			
			case 4: // STATE 4: ACCESS DENIED-------------------------------------------------------------------------------------------

				AccessDenied();
				
				// Checking which state to go to:
				if (_intrFlag_EG) // If push button flag is set
				{
						// Go to lockdown state
						nextState = 2;
					
						// Clear interrupt flag
						_intrFlag_EG = false;
					
						break;
									
					}
				
				
				// Go to state 0
				nextState = 0;
				
				break;
				
			
			
			case 5: // Reserved for future use
			
			
			// Start program again, that is state 0
			nextState = 0;
			break;
			
			default:
			
			// Start program again, that is state 0
			nextState = 0;
			break;
		}
    }
}



// Interrupt Service Routine for timer1, channel A (Timeout timer)
ISR(TIMER1_COMPA_vect){
	/* The purpose of this interrupt service routine is to hold count of time, 
	display countdown on LCD and trigger timeout if 15 secs has been counted
	*/
	
	// A second has passed because ISR has now been called;
	// Decrement the "timer value":
	_timer1--;
	
	// Setting the cursor for writing remaining time
	LCD_SetCursorPosition( 6 /*0 - 40 */,  1 /*0 for top row, 1 for bottom row*/); // PS! CODE BY R.A. ALL CREDITS GO TO HIM.
	
	// A switch case for the timer. Displays countdown on LCD.
	// When timer is 0, timeout happens and timer is reset.
	
	switch (_timer1)
	{
		case 14:
		
		LCD_WriteString("Time: 14s");
		PORTA = 0x0e;
		
		break;
		
		case 13:
		
		LCD_WriteString("Time: 13s");
		PORTA = 0x0d;
		
		break;
		
		case 12:
		
		LCD_WriteString("Time: 12s");
		PORTA = 0x0c;
		
		break;
		
		case 11:
		
		LCD_WriteString("Time: 11s");
		PORTA = 0x0b;
		
		break;
		
		case 10:
		
		LCD_WriteString("Time: 10s");
		PORTA = 0x0a;
		
		break;
		
		case 9:
		
		LCD_WriteString("Time: 9s ");
		PORTA = 0x09;
		
		break;
		
		case 8:
		
		LCD_WriteString("Time: 8s ");
		PORTA = 0x08;
		
		break;
		
		case 7:
		
		LCD_WriteString("Time: 7s ");
		PORTA = 0x07;
		
		break;
		
		case 6:
		
		LCD_WriteString("Time: 6s ");
		PORTA = 0x06;
		
		break;
		
		case 5:
		
		LCD_WriteString("Time: 5s ");
		PORTA = 0x05;
		
		break;
		
		case 4:
		
		LCD_WriteString("Time: 4s ");
		PORTA = 0x04;
		
		break;
		
		case 3:
		
		LCD_WriteString("Time: 3s ");
		PORTA = 0x03;
		
		break;
		
		case 2:
		
		LCD_WriteString("Time: 2s ");
		PORTA = 0x02;
		
		break;
		
		case 1:
		
		LCD_WriteString("Time: 1s ");
		PORTA = 0x01;
		
		break;
		
		case 0:
		
		// Set timeout flag since timeout has occured
		_intrFlag=true;
		
		// Clearing screen:
		LCD_Clear();
		// Setting the cursor:
		LCD_Home();
		
		// Write to the top row: "Input Password:"
		LCD_WriteString("    Timeout!");
		PORTA=0x3C;
		_delay_ms(1000); // hold screen for 1 second
		PORTA=0x00;
		
		// Stop Timer1
		ResetTimer1();
		
		// Reset _timer1 global variable
		_timer1 = timeout_time;
		
		break;
		
		default:
		// Clearing screen:
		LCD_Clear();
		// Setting the cursor:
		LCD_Home();
		
		// Write to the top row: "Input Password:"
		LCD_WriteString("FAULT. SEE ISR.");
		
		
		
		break;


	}


}

// Interrupt Service Routine for timer3, channel A (Lockdown mode timer)
ISR(TIMER3_COMPA_vect) 
{
	/* The purpose of this interrupt service routine is to hold count of lockdown time, 
	display countdown on LCD and restart program after countdown is done
	*/
	
	// A second has passed because ISR has now been called;
	// Decrement the "timer value":
	_timer3--;
	
	// Setting the cursor for writing remaining time
	LCD_SetCursorPosition( 6 /*0 - 40 */,  1 /*0 for top row, 1 for bottom row*/); // PS! CODE BY R.A. ALL CREDITS GO TO HIM.
	
	// A switch case for the timer. Displays countdown on LCD.
	// When timer is 0, timeout happens and timer is reset.
	
	switch (_timer3)
	{
		case 60:
		
		LCD_WriteString("Time: 60s");
		
		
		break;
		
		case 59:
		
		LCD_WriteString("Time: 59s");
		
		
		break;
		
		case 58:
		
		LCD_WriteString("Time: 58s");
		
		
		break;
		
		case 57:
		
		LCD_WriteString("Time: 57s");
		
		
		break;
		
		case 56:
		
		LCD_WriteString("Time: 56s");
		
		
		break;
		
		case 55:
		
		LCD_WriteString("Time: 55s");
		
		
		break;
		
		case 54:
		
		LCD_WriteString("Time: 54s");
		
		
		break;
		
		case 53:
		
		LCD_WriteString("Time: 53s");
		
		
		break;
		
		case 52:
		
		LCD_WriteString("Time: 52s");
		
		
		break;
		
		case 51:
		
		LCD_WriteString("Time: 51s");
		
		
		break;
		
		case 50:
		
		LCD_WriteString("Time: 50s");
		
		
		break;
		
		case 49:
		
		LCD_WriteString("Time: 49s");
		
		
		break;
		
		case 48:
		
		LCD_WriteString("Time: 48s");
		
		
		break;
		
		case 47:
		
		LCD_WriteString("Time: 47s");
		
		
		break;
		
		case 46:
		
		LCD_WriteString("Time: 46s");
		
		
		break;
		
		case 45:
		
		LCD_WriteString("Time: 45s");
		
		
		break;
		
		case 44:
		
		LCD_WriteString("Time: 44s");
		
		
		break;
		
		case 43:
		
		LCD_WriteString("Time: 43s");
		
		
		break;
		
		case 42:
		
		LCD_WriteString("Time: 42s");
		
		
		break;
		
		case 41:
		
		LCD_WriteString("Time: 41s");
		
		
		break;
		
		case 40:
		
		LCD_WriteString("Time: 40s");
		
		
		break;
		
		case 39:
		
		LCD_WriteString("Time: 39s ");
		
		
		break;
		
		case 38:
		
		LCD_WriteString("Time: 38s ");
		
		
		break;
		
		case 37:
		
		LCD_WriteString("Time: 37s ");
		
		
		break;
		
		case 36:
		
		LCD_WriteString("Time: 36s ");
		
		
		break;
		
		case 35:
		
		LCD_WriteString("Time: 35s ");
		
		
		break;
		
		case 34:
		
		LCD_WriteString("Time: 34s ");
		
		
		break;
		
		case 33:
		
		LCD_WriteString("Time: 33s ");
		
		
		break;
		
		case 32:
		
		LCD_WriteString("Time: 32s ");
		
		
		break;
		
		case 31:
		
		LCD_WriteString("Time: 31s ");
	
		
		break;
		
		case 30:
		
		LCD_WriteString("Time: 30s");
		
		
		break;
		
		case 29:
		
		LCD_WriteString("Time: 29s");
		
		
		break;
		
		case 28:
		
		LCD_WriteString("Time: 28s");
		
		
		break;
		
		case 27:
		
		LCD_WriteString("Time: 27s");
		
		
		break;
		
		case 26:
		
		LCD_WriteString("Time: 26s");
		
		
		break;
		
		case 25:
		
		LCD_WriteString("Time: 25s");
		
		
		break;
		
		case 24:
		
		LCD_WriteString("Time: 24s");
		
		
		break;
		
		case 23:
		
		LCD_WriteString("Time: 23s");
		
		
		break;
		
		case 22:
		
		LCD_WriteString("Time: 22s");
		
		
		break;
			
		case 21:
		
		LCD_WriteString("Time: 21s");
		
		
		break;
		
		case 20:
		
		LCD_WriteString("Time: 20s");
		
		
		break;
		
		case 19:
		
		LCD_WriteString("Time: 19s");
		
		
		break;
		
		case 18:
		
		LCD_WriteString("Time: 18s");
		
		
		break;
		
		case 17:
		
		LCD_WriteString("Time: 17s");
		
		
		break;
		
		case 16:
		
		LCD_WriteString("Time: 16s");
		
		
		break;
		
		case 15:
		
		LCD_WriteString("Time: 15s");
		
		
		break;
		
		case 14:
		
		LCD_WriteString("Time: 14s");
		
		
		break;
		
		case 13:
		
		LCD_WriteString("Time: 13s");
		
		
		break;
		
		case 12:
		
		LCD_WriteString("Time: 12s");
		
		
		break;
		
		case 11:
		
		LCD_WriteString("Time: 11s");
		
		
		break;
		
		case 10:
		
		LCD_WriteString("Time: 10s");
		
		
		break;
		
		case 9:
		
		LCD_WriteString("Time: 9s ");
		
		
		break;
		
		case 8:
		
		LCD_WriteString("Time: 8s ");
		
		
		break;
		
		case 7:
		
		LCD_WriteString("Time: 7s ");
		
		
		break;
		
		case 6:
		
		LCD_WriteString("Time: 6s ");
		
		
		break;
		
		case 5:
		
		LCD_WriteString("Time: 5s ");
		
		
		break;
		
		case 4:
		
		LCD_WriteString("Time: 4s ");
		
		
		break;
		
		case 3:
		
		LCD_WriteString("Time: 3s ");
		
		
		break;
		
		case 2:
		
		LCD_WriteString("Time: 2s ");
		
		
		break;
		
		case 1:
		
		LCD_WriteString("Time: 1s ");
		
		
		break;
		
		case 0:
		
		// Turning off all LEDs that were set because of push button press (ISR for INT4)
		PORTA = 0;
		
		// Resetting TIMER3
		ResetTimer3();
		
		
		break;
		
		default:
		// Clearing screen:
		LCD_Clear();
		// Setting the cursor:
		LCD_Home();
		
		// Write to the top row: "Input Password:"
		LCD_WriteString("FAULT. SEE ISR3.");	
		
		
		break;
	}
}

// Interrupt Service Routine for push button (INT4), that sets system in lockdown mode for 1 minute
ISR(INT4_vect){

	if (_timer1<timeout_time) // if the user pushes push-button when typing password
	{
		// Reset timeout timer
		// Turn of Timer1 (this is valid for the situation when user starts
		// pressing switches to input passcode and then pushes red push-button for lockdown)
		ResetTimer1();
	}
	
	if(_timer3==lockdown_time){		
		
		// Light up orange leds
		PORTA=0x3C;
		
		// Start Timer3 to be used for lockdown mode
		StartTimer3();
		
		//Write to the LCD that system is in lockdown mode
		// Clearing screen:
		LCD_Clear();
		// Setting the cursor:
		LCD_Home();
		LCD_WriteString("  LOCKDOWN MODE");
		// Setting the cursor for writing remaining time
		LCD_SetCursorPosition( 6 /*0 - 40 */,  1 /*0 for top row, 1 for bottom row*/); // PS! CODE BY R.A. ALL CREDITS GO TO HIM.
		LCD_WriteString("Time: 60s");
		
		// Push button has now been set so we set the flag for push button
		_intrFlag_EG = 1;
	}
}
