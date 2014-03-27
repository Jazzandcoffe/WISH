/*
* Kommunikationsmodul.c
*
* Created: 3/26/2014 1:42:57 PM
*  Author: hermo276
*/


#include <avr/io.h>
#include <avr/interrupt.h>

//Globala variabler
volatile int init_transmit; //För att hålla reda på när vi ska använda buss.
volatile int transmit_buffer; //Data som ska skickas
volatile int recieve_buffer; //Data som tas emot

//Initierar SPI Master
void SPI_init(void){

	DDRB = ((1<<DDB7)|(1<<DDB5)|(1<<DDB4)|(1<<DDB3)); //spi pins on port b MOSI SCK,SS1,SS2 outputs

	SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0));  // SPI enable, Master, f/16

}

//Transmit function. cData på MOSI. Return MISO.
char SPI_Transmit(char cData){

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
	PORTB = (0<<PB4);
	SPI_Transmit(0x00);
	PORTB = (1<<PB4);
}

int main(void)
{
	SPI_init();
	timer1_init();

	sei(); // set Global Interrupt Enable

	transmit_buffer = 0;

	// loop forever
	for (;;)
	{
		if(init_transmit==1)
		{
			//Testkod för bussen
			if (transmit_buffer == 0xF)
			{
				transmit_buffer = 0x0;
			}
			else
			{
				transmit_buffer++;
			}

			PORTB = (0<<PB4);
			recieve_buffer = SPI_Transmit(transmit_buffer);
			PORTB = (1<<PB4);
			init_transmit = 0;
		}
	}
}
