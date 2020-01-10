////////////////////////////////////////////////////////////////////////////
//// LCD420_S3.C ////
//// Driver for common 4x20 LCD modules ////
//// ////
//// lcd_init() Must be called before any other function. ////
//// ////
//// lcd_putc(c) Will display c on the next position of the LCD. ////
//// The following have special meaning: ////
//// \f Clear display ////
//// \n Go to start of second line ////
//// \b Move back one position ////
//// \c Cursor on ////
//// \d Blink character ////
//// \1 Custom character DegC ////
//// ////
//// lcd_gotoxy(x,y) Set write position on LCD (upper left is 1,1) ////
//// ////
//// lcd_getc(x,y) Returns character at position x,y on LCD ////
//// ////
////////////////////////////////////////////////////////////////////////////
//// (C) Copyright 1996,1997 Custom Computer Services ////
//// This source code may only be used by licensed users of the CCS C ////
//// compiler. This source code may only be distributed to other ////
//// licensed users of the CCS C compiler. No other use, reproduction ////
//// or distribution is permitted without written permission. ////
//// Derivative programs created using this software in object code ////
//// form are not restricted in any way. ////
////////////////////////////////////////////////////////////////////////////

// These pins are for the PIC development board:
// B1 enable
// B2 rs
// B3 rw
// B4 D4
// B5 D5
// E1 D6
// E0 D7

#if defined(__PCM__) 
#define LCD_RS PIN_B2

#elif defined(__PCH__)
#define LCD_RS PIN_E0
#endif

#define LCD_E PIN_B3			// B3
//#define LCD_RS PIN_B2			// B2 Nop
//#define LCD_RW PIN_B1			// B1 Nop
#define LCD_DB4 PIN_B4
#define LCD_DB5 PIN_B5
#define LCD_DB6 PIN_B6
#define LCD_DB7 PIN_B7

// Clear and home
#define LCD_CLR_DISP 0x01 // Clear screen, home cursor, unshift display
#define LCD_RETURN_HOME 0x02 // Home cursor, unshift display

// Set entry mode: display shift on/off, dec/inc cursor move direction
#define LCD_ENTRY_DEC 0x04 // Display shift off, dec cursor move dir
#define LCD_ENTRY_DEC_SHIFT 0x05 // Display shift on, dec cursor move dir
#define LCD_ENTRY_INC 0x06 // Display shift off, inc cursor move dir
#define LCD_ENTRY_INC_SHIFT 0x07 // Display shift on, inc cursor move dir

// Display on/off, cursor on/off, blinking char at cursor position
#define LCD_DISP_OFF 0x08 // Display off
#define LCD_DISP_ON 0x0C // Display on, cursor off
#define LCD_DISP_ON_BLINK 0x0D // Display on, cursor off, blink char
#define LCD_DISP_ON_CURSOR 0x0E // Display on, cursor on
#define LCD_DISP_ON_CURSOR_BLINK 0x0F // Display on, cursor on, blink char

// Move cursor/shift display
#define LCD_MOVE_CURSOR_LEFT 0x10 // Move cursor left (decrement)
#define LCD_MOVE_CURSOR_RIGHT 0x14 // Move cursor right (increment)
#define LCD_MOVE_DISP_LEFT 0x18 // Shift display left
#define LCD_MOVE_DISP_RIGHT 0x1C // Shift display right

// Function set: set interface data length and number of display lines
#define LCD_FUNCTION_4BIT_1LINE 0x20 // 4-bit interface, single line, 5x7 dots
#define LCD_FUNCTION_4BIT_2LINES 0x28 // 4-bit interface, dual line, 5x7 dots
#define LCD_FUNCTION_8BIT_1LINE 0x30 // 8-bit interface, single line, 5x7 dots
#define LCD_FUNCTION_8BIT_2LINES 0x38 // 8-bit interface, dual line, 5x7 dots

#define LCD_CGRAM_BASE_ADDR 0x40 // Set the CGRAM address
#define LCD_DDRAM_BASE_ADDR 0x80 // Set the DDRAM address

// Address positions
#define LCD_LINE_1 0x80 // Position of start of line 1
#define LCD_LINE_2 0xC0 // Position of start of line 2
#define LCD_LINE_3 0x94 // Position of start of line 3
#define LCD_LINE_4 0xD4 // Position of start of line 4

#define LCD_DEGREE_CHAR 0x00 // Ascii 00

#define lcd_type 2 // 0=5x7, 1=5x10, 2=2 lines

BYTE const LCD_INIT_STRING[4] = {
LCD_FUNCTION_4BIT_2LINES | (lcd_type << 2), // Set mode: 4-bit, 2 lines, 5x7 dots
LCD_DISP_ON,
LCD_CLR_DISP,
LCD_ENTRY_INC };

BYTE const LCD_CUSTOM_CHARS[] = {
0x1C,0x14,0x1C,0x00,0x00,0x00,0x00,0x00, // DegC
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // Not used
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // Not used
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // Not used
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // Not used
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // Not used
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // Not used
0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F }; // Not used

BYTE lcd_line;
/*
BYTE lcd_read_nibble() {
BYTE retval;

#bit retval_0 = retval.0
#bit retval_1 = retval.1
#bit retval_2 = retval.2
#bit retval_3 = retval.3
retval = 0;
output_high(LCD_E);
delay_cycles(1);
retval_0 = input(LCD_DB4);
retval_1 = input(LCD_DB5);
retval_2 = input(LCD_DB6);
retval_3 = input(LCD_DB7);
output_low(LCD_E);
return(retval);
}
/*
BYTE lcd_read_byte() {
BYTE low,high;

output_high(LCD_RW);
delay_cycles(1);
high = lcd_read_nibble();
low = lcd_read_nibble();
return((high << 4) | low);
}
*/
void lcd_send_nibble( BYTE n ) {
	output_bit(LCD_DB4, !!(n & 1));
	output_bit(LCD_DB5, !!(n & 2));
	output_bit(LCD_DB6, !!(n & 4));
	output_bit(LCD_DB7, !!(n & 8));
	delay_cycles(1);
	output_high(LCD_E);
	delay_cycles(2);
	output_low(LCD_E);
}

void lcd_send_byte( BYTE address, BYTE n ) {
	output_low(LCD_RS);
//	while (bit_test(lcd_read_byte(),7)); // wait until busy flag is low
	if (address)
		output_high(LCD_RS);
	else
		output_low(LCD_RS);
//	delay_cycles(1);
//	output_low(LCD_RW);
//	delay_cycles(1);
	output_low(LCD_E);
	lcd_send_nibble(n >> 4);
	lcd_send_nibble(n & 0xf);
	delay_us(39);	
}

void lcd_init() {
BYTE i;
	for (i=1;i<=3;++i) {
		lcd_send_byte(0,0x20);
		delay_ms(2);
	}
	for (i=0;i<=sizeof(LCD_INIT_STRING)-1;++i){
		lcd_send_byte(0, LCD_INIT_STRING[i]);
		delay_ms(2);
		}
}

void lcd_init_custom_chars() {
BYTE i;

	lcd_send_byte(0,LCD_CGRAM_BASE_ADDR);
	for (i=0;i<64;i++) {
		lcd_send_byte(1,LCD_CUSTOM_CHARS[i]);
		delay_ms(2);
	}
}
void lcd_gotoxy( BYTE x, BYTE y ) {
BYTE address;

	switch(y) {
		case 1 : address=LCD_LINE_1; break;
		case 2 : address=LCD_LINE_2; break;
		case 3 : address=LCD_LINE_3; break;
		case 4 : address=LCD_LINE_4; break;
		
	}
	address+=x-1;
	lcd_send_byte(0,0x80 | address);
}

void lcd_putc( char c ) {
	switch(c) {
		case '\f' : lcd_send_byte(0,LCD_CLR_DISP);
			lcd_line=1;
			delay_ms(2); break;
		case '\n' : lcd_gotoxy(1,++lcd_line); break;
		case '\b' : lcd_send_byte(0,LCD_MOVE_CURSOR_LEFT); break;
		case '\1' : lcd_send_byte(1,LCD_DEGREE_CHAR); break;				// caractere degree
		default : lcd_send_byte(1,c); break;
	}
}

void cursor( byte c ) {
	switch(c) {
		case 0 : lcd_send_byte(0,LCD_DISP_ON);break;						// curseur OFF
		case 1 : lcd_send_byte(0,LCD_DISP_ON_CURSOR); break;				// curseur ON
		case 2 : lcd_send_byte(0,LCD_DISP_ON_BLINK); break;					// cursour OFF blink
		case 3 : lcd_send_byte(0,LCD_DISP_ON_CURSOR_BLINK); break;			// cursour ON blink
	}
}

/*
char lcd_getc( BYTE x, BYTE y ) {
char value;

	lcd_gotoxy(x,y);
	while (bit_test(lcd_read_byte(),7)); // wait until busy flag is low
	output_high(LCD_RS);
	value = lcd_read_byte();
	output_low(LCD_RS);
	return(value);
}*/