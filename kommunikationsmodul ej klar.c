/*
* Kommunikationsmodul.c
*
* Created: 3/26/2014 1:42:57 PM
*  Author: hermo276
*/

#define F_CPU 14745600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Globala variabler
volatile int init_transmit; //För att hålla reda på när vi ska använda buss.
volatile char transmit_buffer; //Data som ska skickas
volatile char recieve_buffer; //Data som tas emot
volatile int auto_or_manual; //autonomt läge = 0/manuellt läge = 1
volatile char type_sens; //typ-byte till protokollet
volatile char data_sens; //data-byte till protokollet
volatile char check_sens; //check-byte till protokollet
volatile char type_styr; //typ-byte till protokollet
volatile char data_styr; //data-byte till protokollet
volatile char check_styr; //ckeck-byte till protokollet


//Initierar SPI Master
void SPI_init(void)
{
	//spi pins on port b MOSI SCK,SS1,SS2 outputs
	DDRB = ((1<<DDB7)|(1<<DDB5)|(1<<DDB4)|(1<<DDB3)); 
	// SPI enable, Master, f/16
	SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0));  

}

//Transmit function. cData på MOSI. Return MISO.
char SPI_transmit(char cData)
{
	SPDR = cData;
	while(!(SPSR & (1<<SPIF)))
	;
	return SPDR;
}

//initierar timer1
void timer1_init()
{
	//Timer prescaler = 8
	TCCR1B |= (1<<CS11);
	//Start timer
	TCNT1 = 0;
	//Enable overflow interrupt
	TIMSK1 |= (1<<TOIE1);
}



//Avbrottsvektorn till timer overflow
ISR(TIMER1_OVF_vect)
{
	init_transmit = 1;
}

ISR(INT0_vect)
{
	if(auto_or_manual == 0)
	{
		auto_or_manual = 1;
	}
	else
	{
		auto_or_manual = 0;
	}
	PORTB = (0<<PB4);
	SPI_transmit(0x00);
	PORTB = (1<<PB4);
}

/* 
SS för sensor, vi skickar aldrig, 
så denna funktion läser vad som 
sensor lagt på bussen
*/
char ss_sensor() 	
{
	PORTB = (0 << PB3);
	recieve_buffer = SPI_transmit(0x00);
	PORTB = (1 << PB3);
	return recieve_buffer;
}

/*
SS för styr, vi skickar sensordata,
tar emot styrbeslut.(som ska skickas via BT)
*/
char ss_styr(char to_send)
{
	PORTB = (0 << PB4);
	recieve_buffer = SPI_transmit(to_send);
	PORTB = (1 << PB4);
	return recieve_buffer;
}

int main(void)
{

	SPI_init();
	timer1_init();
	// set Global Interrupt Enable
	sei(); 
	auto_or_manual = 1;
	
	//testkod
	type_styr = 0x08;
	data_styr = 0x0F;
	check_styr = 0xFF;
	//testkod

	// loop forever
	for (;;)
	{
		//Autonomt läge
		if(auto_or_manual == 0)
		{
			if(init_transmit==1)
			{
				for (int i=0;i<10;i++)
				{
					type_sens = ss_sensor();
					_delay_us(10);
					data_sens = ss_sensor();
					_delay_us(10);
					check_sens = ss_sensor();
					/*
					Här ska det vara kod för ckeck
					*/
					//if(check_ok)
					type_styr = ss_styr(type_sens);
					_delay_us(10);
					data_styr = ss_sensor(data_sens);
					_delay_us(10);
					check_styr = ss_sensor(check_sens);
					/*
					Här ska det vara kod för check
					*/
					//if(ckeck_ok)
					/*
					Här ska det vara kod för bluetoothsändning 
					av styrbeslut och sensorvärden
					*/
				}
				
				init_transmit = 0;
			}
		}
		//Manuellt läge
		else 
		{
			if(init_transmit==1)
			{
				/*
				Här ska det vara kod för bluetoothsändning 
				av styrbeslut och sensorvärden
				*/
				type_sens = ss_styr(type_styr);
				_delay_us(10);
				data_sens = ss_styr(data_styr);
				_delay_us(10);
				check_sens = ss_styr(check_styr);
				init_transmit = 0;
			}
		}
	}
}