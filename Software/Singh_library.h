#ifndef  SINGH_LIBRARY_H_
#define	 SINGH_LIBRARY_H_

// Keyword for no switch pressed:
#define	null	0xFF
// Keywords for different switchpresses:
//				c-76543210
#define S1		0b11100111
#define S2		0b11010111
#define S3		0b10110111
#define S4		0b01110111
#define S5		0b11101011
#define S6		0b11011011
#define S7		0b10111011
#define S8		0b01111011
#define S9		0b11101101
#define S10		0b11011101
#define S11		0b10111101
#define S12		0b01111101
#define S13		0b11101110
#define S14		0b11011110
#define S15		0b10111110
#define S16		0b01111110


// Defining keywords for password:
#define	P1		0x01
#define	P2		0x02
#define P3		0x03
#define	P4		0x04
#define	P5		0x05
#define P6		0x06
#define	P7		0x07
#define	P8		0x08
#define P9		0x09
#define	P10		0x0A
#define	P11		0x0B
#define P12		0x0C
#define	P13		0x0D
#define	P14		0x0E
#define P15		0x0F
#define	P16		0x10


/*--------------------------------------------------------|
Methods used setting up stuff; ports, interrupts etc:
---------------------------------------------------------*/
void IS(void); // Initial Setup. Calls PS() and LCDS().

void PS(void); // Setup for ports

void LCDS(void); // Setup for LCD. PS! CODE IN HERE WRITTEN BY R.A. ALL CREDITS GO TO HIM.




/*--------------------------------------------------------|
Methods for detecting switch presses:					  
---------------------------------------------------------*/
unsigned char DetectSwitchPress();// Container method for detecting switch press. 

unsigned char Detect0(unsigned char, unsigned char);// Method for detecting if a switch in row 0 has been pressed
unsigned char Detect1(unsigned char, unsigned char);// Method for detecting if a switch in row 1 has been pressed
unsigned char Detect2(unsigned char, unsigned char);// Method for detecting if a switch in row 2 has been pressed
unsigned char Detect3(unsigned char, unsigned char);// Method for detecting if a switch in row 3 has been pressed







/*--------------------------------------------------------|
Methods used to store each switch press to its designated variable:
---------------------------------------------------------*/
unsigned char AssignSV();







/*--------------------------------------------------------|
Methods used to initialise and start timers:
---------------------------------------------------------*/
void StartTimer1(void); // Used for 15s countdown during password input on switchpad
void StartTimer3(void); // Used for 60s countdown during lockdown mode of system. Stated mode is triggered by red push button next to LCD
void StartTimer4(void); // Used for PWM - servo motor. Motor spins when access is granted (correct password input)

/*--------------------------------------------------------|
Methods used to stop and clear timers:
---------------------------------------------------------*/
void ResetTimer1(void);
void ResetTimer3(void);
void ResetTimer4(void);

/*--------------------------------------------------------|
Methods used to check the input password from keypad
---------------------------------------------------------*/
void CheckPassword(unsigned char (*p_passw)[], unsigned char (*p_input)[]);


/*--------------------------------------------------------|
Methods used if access is granted:
---------------------------------------------------------*/
void AccessGranted(void);
void SpinMotor(void);



/*--------------------------------------------------------|
Methods used if access is not granted:
---------------------------------------------------------*/
void AccessDenied(void);













/*--------------------------------------------------------|
FUNCTION IMPLEMENTATIONS HERE ON OUT
---------------------------------------------------------*/

void IS(){
	
	
	
	PS(); // General Port Setup for Keypad, Servo motor and Interrupt 4 (for ext. intr.)
	LCDS(); // LCD Setup. PS! THIS METHOD INCLUDES CODE ORIGINALLY WRITTEN BY R.A. ALL CREDIT GOES TO HIM.
	
	sei(); // Enable global interrupts.
	
}

void PS(){
	
	// Rows on the keypad are outputs while columns are inputs (will be used to detect logic level and determine which switch is pressed)
	// Set bits C3 downto C0 as output (rows R3-R0) and C7 downto C0 as input (columns C3-C0)
	DDRC = 0b0001111;
	// Set port C as high; all rows will begin with having high level, the columns need to be pulled up to high level before any switch is pressed.
	PORTC= 0xFF;
	
	
	// PortA is for the LEDs. Their data direction register is set high (output) and the LEDs will initially be off.
	DDRA = 0xFF; // Setting PortA as output
	PORTA = 0x00; // Output signal is initially low (0)
	
	
	// PortH3 - 6 is for PWM for servo motor. This port is connected to OC4A and can be used for PWM signaling. 
	// Once access is granted, servo motor will spin to indicate door opening.
	DDRH = (1 << PORTH3); // Setting PH3 to output
	//PORTB = (0 << PORTB7); // Output signal is initially low (0)
	
	
	// Using INT4 (PE4) as ext. intr. for push button that sets system in lockdown mode
	DDRE = (0 << PORTE4); // Setting PE4 as input
	PORTE = (1 << PORTE4); // Internal pull-up
	
	EIMSK = 0x00; // Clearing external interrupt mask register before setting EICRB
	
	EICRB = (1<<ISC41) | (1<<ISC40); // a rising edge (low to high) will trigger an interrupt
	
	EIMSK = (1<<INT4); // Enabling ext. intr. request for intr. 4
	
}

// PS! CODE IN LCDS() CONCISTS OF SUB-ROUTINES WRITTEN BY R.A. ALL CREDITS TO HIM.
void LCDS(){
	
	// LCD Setup; 2 line mode and 5*8 pixel (small font)
	LCD_Initialise(true, false); // from BorrowedMisc_code.h, see crediting in preamble.
	
	LCD_ShiftDisplay(false /*true = ON false = OFF*/, true /*true = shift right, false = shift left*/);
	LCD_Display_ON_OFF(true /*Display ON*/, false /*Cursor OFF*/, false /*Cursor Position OFF*/);
	
	// Clearing screen:
	LCD_Clear();
	// Setting the cursor:
	LCD_Home();	
	
	// Write to the top row: "Input Password:"
	LCD_WriteString(" Input Password");

}




unsigned char DetectSwitchPress(){
	/* We need two variables; one to hold value for which row we will be giving a high level, 
	 * and one to temporarily hold value for switch press:
	*/

	unsigned char R;
	unsigned char _SV=null;
	
// As long as no switch is pressed, the program will keep waiting for switch press, 
// except if an interrupt flag for timeout is set:	
while (_SV==null)
{	
	
	// Method for detecting switch press in row 0:
	_SV = Detect0(R, _SV);
	
	// if a key has been pressed or the interrupt flag for timeout has been set, return the switch value _SV:
	if (_SV!=null || _intrFlag || _intrFlag_EG){return _SV;}
	
	// Method for detecting switch press in row 1:
	_SV = Detect1(R, _SV);
	
	// if a key has been pressed or the interrupt flag for timeout has been set, return the switch value _SV:
	if (_SV!=null || _intrFlag || _intrFlag_EG){return _SV;}
	
	// Method for detecting switch press in row 2:
	_SV = Detect2(R, _SV);
	
	// if a key has been pressed or the interrupt flag for timeout has been set, return the switch value _SV:
	if (_SV!=null || _intrFlag || _intrFlag_EG){return _SV;}
	
	// Method for detecting switch press in row 3:
	_SV = Detect3(R, _SV);
	
	// if a key has been pressed or the interrupt flag for timeout has been set, return the switch value _SV:
	if (_SV!=null || _intrFlag || _intrFlag_EG){return _SV;}
	
}
		
	
}
unsigned char Detect0(unsigned char R, unsigned char _SV){
	
	// The purpose of this subroutine is to determine whether a switch in row 0 has been pressed.
	
	R = 0x01; // This is the value for switch S1 at row 0
	
	/*Configuring port C with all high levels. 
	* Then only row 0 (bit C3) is sent a low level. 
	* This way the column connected to row 0 through the switch press will also have a low logic level.
	* Based on that we can detect which key is pressed
	*/
	
	PORTC = 0xFF;
	PORTC &= 0b11110111; // row 0 is given low level
	
	/*After giving row 0 logic 0, we start looking at which columns 
	* (these are inputs to MCU) also have a logic 0 present
	*/
	switch(PINC){
		
		case S1:	// This means that row 0 has a low, and column 0 has a low. This means that S1 has been pressed				
			_SV = R;		// The temporary switch value is given value of 0x01 which is the hex value for switch S1.
			
			
			break;
		
		case S2:	 
			_SV = R + 1;	// S2
			break;
			
		case S3:
			_SV = R + 2;	// S3
			break;
					
		case S4:
			_SV = R + 3;	// S4
			break;
		
		default:
			_SV = null; // No key is pressed in this row
			break;
	}
	
	return _SV;
}
unsigned char Detect1(unsigned char R, unsigned char _SV){
	
	// Scanning row 1	(bit C2)
	R = 0x05; // This is the value for switch S5 at row 1
	
	PORTC = 0xFF;
	PORTC &= 0b11111011;
	// Scanning columns
	switch(PINC){
		
		case S5:
		_SV = R;		// S5
		break;
		
		case S6:
		_SV = R + 1;	// S6
		break;
		
		case S7:
		_SV = R + 2;	// S7
		break;
		
		case S8:
		_SV = R + 3;	// S8
		break;
		
		default:
		_SV = null;
		break;
	}
	
	return _SV;
}
unsigned char Detect2(unsigned char R, unsigned char _SV){
	
	R = 0x09; // This is the value for switch S1 at row 2
	
	PORTC = 0xFF;
	PORTC &= 0b11111101; // row 2 is given low level
	
	
	switch(PINC){
		
		case S9:		
			_SV = R;		
			break;
		
		case S10:	 
			_SV = R + 1;	
			break;
			
		case S11:
			_SV = R + 2;	
			break;
					
		case S12:
			_SV = R + 3;	
			break;
		
		default:
			_SV = null; // No key is pressed in this row
			break;
	}
	
	return _SV;
}
unsigned char Detect3(unsigned char R, unsigned char _SV){
	
	
	R = 0x0D; 
	
	PORTC = 0xFF;
	PORTC &= 0b11111110; 
	

	switch(PINC){
		
		case S13:				
			_SV = R;		
			break;
		
		case S14:	 
			_SV = R + 1;	 
			break;
			
		case S15:
			_SV = R + 2;	
			break;
					
		case S16:
			_SV = R + 3;	
			break;
		
		default:
			_SV = null; // No key is pressed in this row
			break;
	}
	
	return _SV;
}

unsigned char AssignSV(){
	
	unsigned char SV = DetectSwitchPress();
	DebounceDelay(); // PS! THIS METHOD BORROWED FROM R.A. ALL CREDITS GO TO HIM.
	
	/* After returning from "DetectSwitchPress()" the program checks whether
	 the interruptflag for timeout or lockdown mode has been set or not. If it has been set, the switch value 
	 that is returned and given to "input[i]" will be equal to null (oxFF).
	*/
	if (_intrFlag || _intrFlag_EG)
	{
		SV=null;
		
		return SV;
	}
	
	return SV;
}





/* Timer 1 is used as timeout timer. 
It counts to 1 sec (CTC mode) and triggers ISR 
I am applying it to 15 sec so that user has enough time to input password before timeout
*/
void StartTimer1(void){
	// TCCRnA= COMnA2 | COMnA0 | COMnB1	| COMnB0 | COMnC1 | COMnC0 | WGMn1 | WGMn0 |
	// In order to have normal port operation, all "comms" are cleared.
	// In order to clear timer on compare match (CTC), last two bits need to be 0
	TCCR1A = 0x00;
	
	// TCCR3B= ICNCn | ICESn |   X	| WGMn3 | WGMn2 | CSn2 | CSn1 | CSn0 |
	// In order to have CTC, WGMn2 needs to be set.
	// In order to have 1024 prescaling, CS bits have to be 101
	//TCCR1B = 0b00001101;	// CTC mode, use 1024 prescaler
	TCCR1B = 0b00001011;	// CTC mode, use 64 prescaler
	
	TCCR1C = 0x00; // not done anything with

	// System clock is 1 MHz, timer "clock" is 1MHz/64= 15625Hz
	// to get 1 seconds in total, we need to count to 15625 in decimal
	// 15625 is 0x3D09 in hex
	OCR1AH = 0x3D; // Each of these store half of the total time
	OCR1AL = 0x09;
	

	TCNT1H = 0x00;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0x00;
	
	// TIMSK1= X | X |   ICIE1	| X | OCIE1C | OCIE1B | OCIE1A | TOIE1 |
	// In order to have an interrupt generated by output compare match on channel A, OCIE1A needs to be set.
	TIMSK1 = 0b00000010;
	
}
void ResetTimer1(void){
	// TCCR1A= COM1A1 | COM1A0 | COM1B1	| COM1B0 | COM1C1 | COM1C0 | WGM11 | WGM10 |
	// In order to have normal port operation, all "comms" are cleared.
	// In order to clear timer on compare match (CTC), last two bits need to be 0
	TCCR1A = 0x00;
	
	// TCCR1B= ICNC1 | ICES1 |   X	| WGM13 | WGM12 | CS12 | CS11 | CS10 |
	// In order to have CTC, WGM12 needs to be set.
	// In order to have 1024 prescaling, CS bits have to be 101
	TCCR1B = 0b00000000;	// CTC mode, use 1024 prescaler
	
	
	TCCR1C = 0x00; // not done anything with

	// System clock is 1 MHz, timer "clock" is 976.5625=15625/16 Hz
	// to get 15 seconds in total, we need to count to 15*976=14648.4375~14648 in decimal
	// 14648 is 0x3938 in hex
	OCR1AH = 0x00; // Each of these store half of the total time
	OCR1AL = 0x00;
	
	OCR1BH = 0x00; 
	OCR1BL = 0x00; 

	TCNT1H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0b00000000;
	
	// TIMSK1= X | X |   ICIE1	| X | OCIE1C | OCIE1B | OCIE1A | TOIE1 |
	// In order to have an interrupt generated by output compare match on channel A, OCIE1A needs to be set.
	TIMSK1 = 0b00000000;
	
	
	
}

/* Timer 3 is used as Lockdown timer.
It counts to 1 sec (CTC mode) and triggers ISR
I am applying it to 60 sec (lockdown time) 
*/
void StartTimer3(void){
	// TCCR3A= COM3A2 | COM3A0 | COM3B1	| COM3B0 | COM3C1 | COM3C0 | WGM31 | WGM30 |
	// In order to have normal port operation, all "comms" are cleared.
	// In order to clear timer on compare match (CTC), last two bits need to be 0
	TCCR3A = 0x00;
	
	// TCCR3B= ICNC3 | ICES3 |   X	| WGM33 | WGM32 | CS32 | CS31 | CS30 |
	// In order to have CTC, WGM32 needs to be set.
	// In order to have 1024 prescaling, CS bits have to be 101
	//TCCR3B = 0b00001101;	// CTC mode, use 1024 prescaler
	TCCR3B = 0b00001011; // CTC mode, use 64 prescaler
	
	
	TCCR3C = 0x00; // not done anything with

	// System clock is 1 MHz, timer "clock" is 1MHz/64= 15625Hz
	// to get 1 seconds in total, we need to count to 15625 in decimal
	// 15625 is 0x3D09 in hex
	OCR3AH = 0x3D; // Each of these store "half" of the total time
	OCR3AL = 0x09;
	

	TCNT3H = 0x00;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT3L = 0x00;
	
	// TIMSK3= X | X |   ICIE3	| X | OCIE3C | OCIE3B | OCIE3A | TOIE3 |
	// In order to have interrupt generated by output compare match on channel A, OCIE3A is set.
	TIMSK3 = 0b00000010;
	
}
void ResetTimer3(void){
	// TCCR1A= COM1A1 | COM1A0 | COM1B1	| COM1B0 | COM1C1 | COM1C0 | WGM11 | WGM10 |
	// In order to have normal port operation, all "comms" are cleared.
	// In order to clear timer on compare match (CTC), last two bits need to be 0
	TCCR3A = 0x00;
	
	// TCCR1B= ICNC1 | ICES1 |   X	| WGM13 | WGM12 | CS12 | CS11 | CS10 |
	// In order to have CTC, WGM12 needs to be set.
	// In order to have 1024 prescaling, CS bits have to be 101
	TCCR3B = 0b00000000;	// CTC mode, use 1024 prescaler
	
	
	TCCR3C = 0x00; // not done anything with

	// System clock is 1 MHz, timer "clock" is 976.5625=15625/16 Hz
	// to get 15 seconds in total, we need to count to 15*976=14648.4375~14648 in decimal
	// 14648 is 0x3938 in hex
	OCR3AH = 0x00; // Each of these store half of the total time
	OCR3AL = 0x00;
	
	TCNT3H = 0b00000000;	// Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT3L = 0b00000000;
	
	// TIMSK1= X | X |   ICIE1	| X | OCIE1C | OCIE1B | OCIE1A | TOIE1 |
	// In order to have an interrupt generated by output compare match on channel A, OCIE1A needs to be set.
	TIMSK3 = 0b00000000;
	
	
	
	
}


/* Timer 4 is used for fast PWM to servo motor.
Global variable DT (dutycycle) determines the dutycycle.
https://www.electronicoscaldas.com/datasheet/MG995_Tower-Pro.pdf
Datasheet for servo motor gives that PWM period has to be 20ms (50 Hz freq)
We will not be using any prescaler.
*/
void StartTimer4(void){
	// TCCR4A= COM4A1 | COM4A0 | COM4B1	| COM4B0 | COM4C1 | COM4C0 | WGM41 | WGM40 |
	// We use channel A, so COM4A1 is set. A0 is 0 (non-inverting mode).We will be using Fast PWM with ICR4 as TOP
	// Therefore WGM is set to 1 1 1 0 (mode 14)
	TCCR4A = 0b10000010;
	
	// TCCR4B= ICNC4 | ICES4 |   X	| WGM43 | WGM42 | CS42 | CS41 | CS40 |
	// In order to have fast PWM, we will be using mode 14,
	// so WGM has to be set 1 1 1 0
	// We will not be using any prescaler (N=1) so CS = 0 0 1
	TCCR4B = 0b00011001;
	
	// ICR4 is calc. by ICR4= (F_CPU/(F_PWM*N)) - 1
	// F_CPU=10^6, F_PWM=50, N=1 --> ICR4 = 19999
	ICR4 = 19999; // Period 20ms
	
	
	
}
void ResetTimer4(void){
	TCCR4A = 0;
	TCCR4B = 0;
	ICR4 = 0; 
}








void CheckPassword(unsigned char (*p_passw)[], unsigned char (*p_input)[]){
	/* The purpose of this sub-routine is to check whether the input password matches the predefined 
	password in "passw" array. The way it checks this is to see if each digit pressed corresponds
	to respective digit in passw array as well, i.e. input0 should match passw0 and so on.
	For each digit that matches, a variable increments, signifying the correct match of input digit and
	password digit. If the number of correct digits is matches with the length of predefined password,
	then access is granted. Otherwise it is not. 
	*/
			
	int digits_correct=0;
	
	for (int i = 0; i < passw_size; i++)
	{
		if((*p_input)[i]==(*p_passw)[i]){
			digits_correct++;
		}
	}
	
	
	if (digits_correct==passw_size) // We grant access (next state is 3)
	{
		// Access granted:
		nextState = 3;
		//AccessGranted();
				
	} else{ // We do not grant access (next state is 4)
		
		// Access not granted:
		nextState = 4;
		//AccessDenied();
					
	}
	
}

void AccessGranted(void){
	//Write to the LCD that Access is granted on top row
	// Clearing screen:
	LCD_Clear();
	// Setting the cursor:
	LCD_Home();
	LCD_WriteString(" Access granted");
	
	
	// Flash blue lights, signaling access granted
	PORTA=0xC0;
	_delay_ms(200);
	PORTA=0x00;
	_delay_ms(200);
	PORTA=0xC0;
	_delay_ms(200);
	PORTA=0x00;
	_delay_ms(200);
	PORTA=0xC0;
	_delay_ms(200);
	PORTA=0x00;
	_delay_ms(200);
	PORTA=0xC0;
	_delay_ms(200);
	PORTA=0x00;
}
void SpinMotor(void){
	
	//Write to the LCD that Access is granted on top row
	// Clearing screen:
	LCD_Clear();
	// Setting the cursor:
	LCD_Home();
	LCD_WriteString("   Door open");
	
	// Starting Timer4 for PWM
	StartTimer4();
	
	// Spin 180 deg
	OCR4A = 950;
	
	// Let it spin for 2 sec
	_delay_ms(1000);
	
	//Spin back to 0
	OCR4A = 0;
	
	// Let it spin for 2 sec
	_delay_ms(1000);
	
	// Stop timer4
	ResetTimer4();
	
}


void AccessDenied(void){
	//Write to the LCD that Access is denied on top row
	// Clearing screen:
	LCD_Clear();
	// Setting the cursor:
	LCD_Home();
	LCD_WriteString(" Access denied");
	DebounceDelay(); // a little delay (same delay as for switch bouncing).PS! CODE BY R.A. ALL CREDITS TO HIM.
	
	// Flash red lights, signaling access denied
	PORTA=0x03;
	_delay_ms(200);
	PORTA=0x00;
	_delay_ms(200);
	PORTA=0x03;
	_delay_ms(200);
	PORTA=0x00;
	_delay_ms(200);
	PORTA=0x03;
	_delay_ms(200);
	PORTA=0x00;
	_delay_ms(200);
	PORTA=0x03;
	_delay_ms(200);
	PORTA=0x00;
	
	
}



#endif

