/*
 * BorrowedMisc_code.h
 
 
 * Original Author: Richard Anthony
 * ------------------------------------------------------------------------------------------------------------
 * ATTENTION: ALL CODE IN THIS FILE IS BORROWED FROM ORIGINAL AUTHOR RICHARD ANTHONY. ALL CREDIT GOES TO HIM.| 
 * I'VE ONLY USED HIS MATERIAL TO BETTER SUIT MY CONFIG (PORTS ETC).
 *------------------------------------------------------------------------------------------------------------
 */ 

//
#include <stdbool.h>
#include <string.h>
#define LCD_DisplayWidth_CHARS 16



#ifndef BORROWEDMISC_CODE_H_
#define BORROWEDMISC_CODE_H_

void DebounceDelay()	// This delay is needed because after pressing a key, the mechanical switch mechanism tends
// to bounce, possibly leading to many key presses being falsely detected. The debounce delay
// makes the program wait long enough for the bouncing to stop before reading the keypad again.
{
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 255; j++);
	}
}







//----------------------------------------------------------------------------------------------------
// The following methods are from Richard Anthony's "LCD_LibraryFunctions2560.h". All credit goes to him. 
// I've just changed and/or used the implementation of some methods to suit the ports and configurations I have.
//----------------------------------------------------------------------------------------------------


//Function declarations
// *** 'Private' Functions accessed by other member functions - do not call these direct from application code ***
void LCD_Write_CommandOrData(bool bCommand /*true = Command, false = Data*/, unsigned char DataOrCommand_Value);
void LCD_Wait();
void LCD_Enable();
void LCD_Disable();
// *** END of 'Private' Functions accessed by other member functions - do not call these direct from application code ***

// *** USER functions
void LCD_Initialise(bool bTwoLine/*false = 1 line mode, true =  2 line mode*/, bool bLargeFont/*false = 5*8pixels, true = 5*11 pixels*/);
void LCD_Display_ON_OFF(bool bDisplayON /*true = ON, false = OFF*/, bool bCursorON, bool bCursorPositionON); // Turn the LCD display ON / OFF
void LCD_Clear();
void LCD_Home();
void LCD_WriteChar(unsigned char cValue);
void LCD_ShiftDisplay(bool bShiftDisplayON /*true = On false = OFF*/, bool bDirectionRight /*true = shift right, false = shift left*/);
void LCD_SetCursorPosition(unsigned char iColumnPosition /*0 - 40 */, unsigned char iRowPosition /*0 for top row, 1 for bottom row*/);
void LCD_WriteString(char Text[]);
// *** END of USER functions

// Function implementations
// *** 'Private' Functions accessed by other member functions - do not call these direct from application code ***
void LCD_Write_CommandOrData(bool bCommand /*true = Command, false = Data*/, unsigned char DataOrCommand_Value)
{
	LCD_Wait();	// Wait if LCD device is busy

	// The access sequence is as follows:
	// 1. Set command lines as necessary:
	if(true == bCommand)
	{	// Register Select LOW, and R/W direction LOW
		PORTG &= 0b11111100;	// Clear Read(H)/Write(L) (PortG bit1),	Clear Register Select for command mode (PortG bit0)
	}
	else /*Data*/
	{	// Register Select HIGH, and R/W direction LOW
		PORTG |= 0b00000001;	// Set Register Select HIGH for data mode (PortG bit0)
		PORTG &= 0b11111101;	// Clear Read(H)/Write(L) (PortG bit1)
	}
	// 2. Set Enable High
	// 3. Write data or command value to PORTL
	// 4. Set Enable Low
	LCD_Enable();
	DDRL = 0xFF;					// Configure PortL direction for Output
	PORTL = DataOrCommand_Value;	// Write combined command value to port L
	LCD_Disable();
}

void LCD_Wait()		// Check if the LCD device is busy, if so wait
{					// Busy flag is mapped to data bit 6, so read as port A bit 6
	PORTG &= 0b11111110;		// Clear Register Select for command mode (PortG bit0)
	PORTG |= 0b00000010;		// Set Read(H)/Write(L) (PortG bit1)
	DDRL = 0x00;				// Configure PortL direction for Input (so busy flag can be read)
	
	unsigned char PINA_value = 0;
	while(PINA_value & 0b10000000);	// Wait here until busy flag is cleared
	{
		LCD_Enable();
		PINA_value = PINL;
		LCD_Disable();
	}
}

void LCD_Enable()
{
	PORTG |= 0b00000100;	// Set LCD Enable (PortG bit2)
}

void LCD_Disable()
{
	PORTG &= 0b11111011;	// Clear LCD Enable (PortG bit2)
}
// *** END of 'Private' Functions accessed by other member functions - do not call these direct from application code ***

// *** USER functions
void LCD_Initialise(bool bTwoLine/*false = 1 line mode, true =  2 line mode*/, bool bLargeFont/*false = 5*8pixels, true = 5*11 pixels*/)
{	// Note, in 2-line mode must use 5*8 pixels font
	
	// Set Port L and Port G for output
	DDRG = 0xFF;		// Configure PortG direction for Output
	PORTG = 0x00;		// Clear port G
	DDRL = 0xFF;		// Configure PortL direction for Output
	PORTL = 0x00;		// Clear port L

	unsigned char Command_value = 0b00110000;	// bit 5 'Function Set' command, bit 4 High sets 8-bit interface mode
	if(true == bTwoLine)
	{
		Command_value |= 0b00001000;	// bit 3 high = 2-line mode (low = 1 line mode)
	}
	else
	{	// One-line mode
		if(true == bLargeFont)
		{	// Large font (nested because can only use large font in one-line mode)
			Command_value |= 0b00000100;	// bit 2 high = large font mode 5*11 pixels (low = small font 5*8pixels)
		}
	}

	LCD_Write_CommandOrData(true /*true = Command, false = Data*/, Command_value);
}

void LCD_Display_ON_OFF(bool bDisplayON /*true = ON, false = OFF*/, bool bCursorON, bool bCursorPositionON) // Turn the LCD display ON / OFF
{
	if(true == bDisplayON)
	{
		if(true == bCursorON)
		{
			if(true == bCursorPositionON)
			{
				// 'Display ON/OFF' function command, Display ON, Cursor ON, Cursor POSITION ON
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001111);
			}
			else
			{
				// 'Display ON/OFF' function command, Display ON, Cursor ON, Cursor POSITION OFF
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001110);
			}
		}
		else /*Cursor OFF*/
		{
			if(true == bCursorPositionON)
			{
				// 'Display ON/OFF' function command, Display ON, Cursor OFF, Cursor POSITION ON
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001101);
			}
			else
			{
				// 'Display ON/OFF' function command, Display ON, Cursor OFF, Cursor POSITION OFF
				LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001100);
			}
		}
	}
	else
	{
		// 'Display ON/OFF' function command, Display OFF, Cursor OFF, Cursor POSITION OFF
		LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00001000);
	}
}

void LCD_Clear()			// Clear the LCD display
{
	LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000001);
	_delay_ms(2);	// Enforce delay for specific LCD operations to complete
}

void LCD_Home()			// Set the cursor to the 'home' position
{
	LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000010);
	_delay_ms(2);	// Enforce delay for specific LCD operations to complete
}

void LCD_WriteChar(unsigned char cValue)
{	// Write character in cValue to the display at current cursor position (position is incremented after write)
	LCD_Write_CommandOrData(false /*true = Command, false = Data*/, cValue);
}

void LCD_ShiftDisplay(bool bShiftDisplayON /*true = On false = OFF*/, bool bDirectionRight /*true = shift right, false = shift left*/)
{
	if(true == bShiftDisplayON)
	{
		if(true == bDirectionRight)
		{
			LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000101);
		}
		else /*shift display left*/
		{
			LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000111);
		}
	}
	else /*ShiftDisplay is OFF*/
	{
		LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00000100);
	}
}

void LCD_SetCursorPosition(unsigned char iColumnPosition /*0 - 40 */, unsigned char iRowPosition /*0 for top row, 1 for bottom row*/)
{
	// Cursor position is achieved by repeatedly shifting from the home position.
	// In two-line mode, the beginning of the second line is the 41st position (from home position)
	unsigned char iTargetPosition = (40 * iRowPosition) + iColumnPosition;
	LCD_Home();
	for(unsigned char iPos = 0; iPos < iTargetPosition; iPos++)
	{
		LCD_Write_CommandOrData(true /*true = Command, false = Data*/, 0b00010100); // Shift cursor left one place
	}
}

void LCD_WriteString(char Text[])
{
	for(unsigned char iIndex = 0; iIndex < strlen(Text); iIndex++)
	{
		LCD_WriteChar(Text[iIndex]);
	}
}
// *** END of USER functions



#endif /* BORROWEDMISC_CODE_H_ */