//-----------------------------------------------------
//--------- Program DSM501A Dust Sensor Module---------
//-----------------------------------------------------
//-- Projet: air quality measurement with DSM501A    --
//-- Date:   28-10-2019                              --
//-- Progr:  Main.c \DSM501A_QAir					 --
//-- Auteur: SEMI1009 - UMONS      					 --
//-----------------------------------------------------
//-- Air quality measurement program with DSM501A    --
//-- module. The air quality is indicated on an LCD  --
//-- screen and with three colored LEDs.             --
//--                                                 --
//--                                                 --
//--                                                 --
//--                  								 --
//----------------------------------------------------- 
// Interfacing PIC18F4550 with DSM501A CCS C code
 
// DSM501A module and Fan connections with PIC18f4550 
/*						+------------
	NC		---> CTRL1 -|1
	NC      ---> Vout2 -|2
	+5V  	---> VCC   -|3  DSM501A
	Pin B2  ---> Vout1 -|4
	GND     ---> GND   -|5
						+------------
						
				        +------------
	+5V		---> VCC   -|
	Pin A0	---> Cmd   -|	Fan
	GND		---> GND   -|
				        +------------
*/

#include <18F4550.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,VREGEN
#use delay(clock=24M)
#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=RS232)

#define led PIN_A4
#define ledR PIN_D0
#define ledO PIN_D3
#define ledG PIN_D1
#define ledtgl	output_toggle(led)
#define led_R(x) output_bit(ledR,x)
#define led_O(x) output_bit(ledO,x)
#define led_G(x) output_bit(ledG,x)

#define ventilo PIN_A0
#define Fan(x) output_bit(ventilo,x)

#define butt PIN_E2
#define button input(butt)

#define _1ms 	65535 - 5950		// Timer for 1ms 
#define OFF 0
#define ON 1
//-------------------------E/S------------------------
#include <math.h>
#include "..\include\LCD420_S3.c"
#include "DSM501A.h"
//------------------------xxx---------------------

int1 flgB,flag_d = false;
int16 compteur = 0;
int16 val_st,val_fin;
long long mes_tot = 0; 
//------------------------xxx---------------------
#INT_TIMER1								// Set timer for 1 ms
void  TIMER1_isr(void) 
{
	set_timer1(_1ms);
	compteur++;							// 1 mS counter
}
//------------------------xxx---------------------
#INT_EXT2
void  EXT_isr(void) 
{
	switch(flag_d){

		case 0:									// if falling edge
			Val_st = compteur;					// read value compteur
			flag_d = true;						// prepar flag for other edge
			ext_int_edge(2, L_TO_H );			// int on rising edge
		break;
		case 1:									// if rising edge
			Val_fin = compteur;					// read value compteur
			flag_d = false;						// prepar flag for other edge
			mes_tot = mes_tot + (val_fin - val_st); // accumulator for number of 1ms
			ext_int_edge(2, H_TO_L );			// int on falling edge
		break;
	}
}
//------------------------xxx---------------------
void out_leds(int1 green,int1 orang,int1 red){		// select LED

	led_G(green);
	led_O(orang);
	led_R(red);
}


//------------------------xxx---------------------
void wait_60s() 
{
  // wait 60s for DSM501 to warm up
  lcd_putc("\fWait 60s Warmup!\n  Sec");
  for (int i = 1; i <= 60; i++)
  {
    delay_ms(1000); // 1s
    lcd_gotoxy(1,2);
    printf(lcd_putc,"%u",i);
  }
  lcd_gotoxy(1,2);
  lcd_putc("Ready!    ");
  delay_ms(1000);	
}
//------------------------xxx---------------------
void initialisation() 
{
	out_leds(0,0,0);
	flgB = false;
	setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);		// 10,9 ms overflow
	set_timer1(_1ms);							// setting Timer 1 for 1ms overflow
	Disable_interrupts(INT_TIMER1);				// disable timer 1
	ext_int_edge(2, H_TO_L );					// setting edge start
	enable_interrupts(INT_EXT2);				// active extern INT
	enable_interrupts(GLOBAL);					// active all INT to the cpu
	lcd_init();									// int LCD
   	lcd_putc("\fMes DSM501A \n");				// Welcome message
	lcd_putc("(C)UMONS - 2019");
	cursor(0);									// curseur OFF
	delay_ms(1000);								// Wait 1 sec
	wait_60s();									// Wait for preheating
	Fan(ON);
	enable_interrupts(INT_TIMER1);				// active timer 1
}

//------------------------xxx---------------------
void main(void)
{
	initialisation();
	lcd_putc("\f");									// Clear LCD
	while(true) {
		if (compteur > 31000) {						// if 30 sec passed
			Fan(OFF);											// Mes_tot en uS -> x1000
			ratio = mes_tot /(sampletime_ms /100); 	// Integer percentage 0=>100
			concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
			printf("Mesure total: %Lu",mes_tot);
			printf("     Ratio: %f",ratio);
			printf("     Concentration: %f \n",concentration);

//			if (sampletime_ms < 3600000) {
//				concentration = concentration * ( sampletime_ms / 3600000.0 );
//			}
//			aqiPM10 = getACQI(1,concentration);
			lcd_gotoxy(1,2);
			printf(lcd_putc,"Cc:%f p.  ",concentration);
			mes_tot = 0;
			compteur = 0;
			flag_d = false;
			if (concentration < 1000){
				lcd_gotoxy(11,1);
				lcd_putc("CLEAN   ");
				out_leds(1,0,0);
			}
			if (concentration > 1000 && concentration < 10000) {
				lcd_gotoxy(11,1);
				lcd_putc("GOOD   ");
				out_leds(1,1,0);
			}	
			if (concentration > 10000 && concentration < 20000) {
				lcd_gotoxy(11,1);
				lcd_putc("ACCEP   ");
				out_leds(0,1,0);				
			}
			if (concentration > 20000 && concentration < 50000) {
				lcd_gotoxy(11,1);
				lcd_putc("HEAVY   ");
				out_leds(0,1,1);	
			}
			if (concentration > 50000 ) {
				lcd_gotoxy(11,1);
				lcd_putc("HAZARD  ");
				out_leds(0,0,1);	
			}
			Fan(ON);
		}
		if (!Button) {
			delay_ms(2000);
			if (!Button){
				if (flgB){
					Fan(OFF);
					Disable_interrupts(INT_TIMER1);
					lcd_gotoxy(1,2);
					lcd_putc("* STOP MESURE * ");
					flgB = false;	
				}else{
					Fan(ON);
					Enable_interrupts(INT_TIMER1);
					lcd_gotoxy(1,2);
					lcd_putc("                ");
					flgB = true;
				}
			}
			while (!Button);
		}
	lcd_gotoxy(1,1);
	printf(lcd_putc,"MT:%Lu   ",mes_tot);
	delay_ms(100);
	}
}