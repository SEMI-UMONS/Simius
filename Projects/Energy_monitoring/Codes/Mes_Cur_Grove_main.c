//-----------------------------------------------------
//---------- Program Mesure current Grove -------------
//-----------------------------------------------------
//-- Projet: Monitor Energy 			             --
//-- Date:   02-10-2019                              --
//-- Progr:  Mes_Cur_Grove_Main						 --
//-- Auteur: SEMI1009 - UMONS (Binon Daniel)		 --
//-----------------------------------------------------
//-- Programme de monitoring de la consommation de   --
//-- Courant au travers d'un module Grove.           --
//-- Le module Grove est connecte sur l'entree ADC   --
//--                                                 --
//--                                                 --
//--                                                 --
//--                                                 --
//--                  								 --
//----------------------------------------------------- 


#include <18F4550.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,VREGEN
#use delay(clock=24M)
#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=RS232)

#INCLUDE <math.h>								// Librairie utile pour la fonction 'POW'
#include "..\include\LCD420_S3.c"				// Driver gestion écran LCD Simius V3
//------------------------------------------------------
#define ledR PIN_D0
#define ledO PIN_D3
#define ledG PIN_D1

#define ledtgl	output_toggle(led)
#define led_R(x) output_bit(ledR,x)
#define led_O(x) output_bit(ledO,x)
#define led_G(x) output_bit(ledG,x)
#define LOW 0
#define HIGH 1
//------------------------------------------------------
int16 sensor_max,offset;
float amplitude_current;
float effective_value,puissance;

//------------------------------------------------------
int16 getMaxValue(){
int16 sensorValue,x,sensorMax = 0;

	x = 100;
	while (x!=0) {
		sensorValue = read_adc();
		if (sensorValue > sensorMax) sensorMax = sensorValue;
		x = x -1;
		delay_ms(1);
	}
	return (sensorMax);
}
//------------------------------------------------------
int16 mes_offset(){
int16 x;
int32 mes=0;

	for (x=0;x<100;x++){
		mes = mes + read_adc();
		delay_ms(1);
	}
	mes = mes / 100;
	return (mes);
}
//------------------------xxx---------------------
void out_leds(int1 green,int1 orang,int1 red){		// select LED

	led_G(green);
	led_O(orang);
	led_R(red);
}

//------------------------------------------------------
void monitor_led(byte entree){

	switch (entree){

		case 0: out_leds(0,0,0);
			break;
		case 1: out_leds(0,0,1);
			break;
		case 2: out_leds(1,0,0);
			break;
		case 3: out_leds(0,1,0);
			break;
		case 4: out_leds(1,1,0);
			break;
		case 5: out_leds(0,1,1);
			break;
		case 6: out_leds(1,0,1);
			break;
		case 7: out_leds(1,1,1);
			break;
	}
}

//------------------------------------------------------
void init(){									// initialisation

	setup_adc_ports(AN0_TO_AN3|VSS_VDD);		
	setup_adc(ADC_CLOCK_INTERNAL);				// utilisation de l'ADC
	set_adc_channel(1);
  	lcd_init();									// init
   	lcd_putc("\f Mesure Current \n");			// Message de bienvenue
	lcd_putc(" (C)SEMI - 2019");			
	cursor(0);									// curseur off
	delay_ms(1500);								
	lcd_putc("\fSm:      P:\n");
	lcd_putc("C:             ");
}
//------------------------------------------------------
void selftest(){
byte x,z;

	for (x=0;x<5;x++){
		for(z=1;z<4;z++){
			monitor_led(z);
			delay_ms(100);
		}	
	}
	for (x=0;x<3;x++){
		monitor_led(7);
		delay_ms(100);
		monitor_led(0);
		delay_ms(100);
	}
}

//------------------------------------------------------
void main(){

	init();
	selftest();
	offset = mes_offset();
	while(true) {
		printf("Offset: %Lu\n",offset);
		sensor_max = getMaxValue()-offset;
		lcd_gotoxy(4,1);
		printf(lcd_putc,"%Lu  ",sensor_max);
		printf("Current = %Lu\n",sensor_max);
		lcd_gotoxy(4,2);
		amplitude_current = (float)sensor_max*5000/1024/800*2000;		// 1000 si 2 boucles - 2000 si  1 boucle
		printf(lcd_putc,"%f  ",amplitude_current);
		printf("The amplitude of the current is(in mA) -> %f\n",amplitude_current);
		effective_value = amplitude_current/1.414;
		puissance = 220 * effective_value / 1000;						// On suppose que la tension est bien de 220V
		printf("The effective value of the current is(in mA) -> %f\n",effective_value);
		lcd_gotoxy(12,1);
		printf(lcd_putc,"%f  ",puissance);
		printf("Puissance = %f4.2 W\n",puissance);
		if (sensor_max <= 10) monitor_led(0);
		if ((sensor_max > 10)&&(sensor_max <= 60)) monitor_led(2);		// Mesure a 
		if ((sensor_max > 60)&&(sensor_max <= 70)) monitor_led(3);		// Mesure a
		if (sensor_max > 70) monitor_led(1);							// Mesure a
	}
}