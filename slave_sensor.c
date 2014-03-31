/*
 *  slave_sensor.c
 *
 *  Created: 03/31/2014 1:42:57 PM
 *  Author:	Herman Molinder		hermo276@student.liu.se
 *			Tore Land�n			torla816@student.liu.se
 */

#include <avr/io.h>
#include <avr/interrupt.h>

//Globala variabler
volatile int	package_counter;	//H�ller reda p� vilken typ av data som f�rv�ntas p� bussen. F�rv�ntar ny s�ndning d� == 0
volatile char	transmit_buffer;	//Data som ska skickas
volatile char	type_transmit;		//SPI
volatile char	data_transmit;		//SPI
volatile char	check_transmit;		//SPI


//Initierar SPI Slave
void SPI_init(void)
{
	//spi pins on port b, MISO output, all other input
	DDRB = (1<<DDB6);
	// SPI enable, Slave 
	SPCR = (1<<SPE); 
}


// Returnerar en "checksum" = type XOR data.
char check_creator(char type,char data)
{
	return type^data;
}

//
void SPI_write()
{
	/*
	type_transmit = ny;
	data_transmit = ny;
	
	check_transmit = check_creator(type_transmit, data_transmit);
	
	*/
}

// Denna funktion hanterar inkommande och utg�ende data p� bussen.
void SPI_transmitter()
{
	switch(package_counter)
	{
		// type
		case 0: 
		transmit_buffer = data_transmit;
		
		//uppdatera package_counter
		package_counter++;
		
		// data
		case 1: 
		transmit_buffer = check_transmit;
		
		//uppdatera package_counter
		package_counter++;
		
		// check
		case 3: 
		SPI_write();
		transmit_buffer = type_transmit;
		
		//uppdatera package_counter
		package_counter = 0;
	}
}

/*
 * Avbrottsvektorn SPI Serial Transfer Complete
 * I denna funktion ska "buffer = SPI_Transmit()" k�ras f�r att
 * lagra inskiftad data i buffer.
 * Sedan ska man skriva ny data till SPDR
 */
ISR(SPI_STC_vect)
{
	SPI_transmitter();
	//reset timer1
	TCNT1 = 0;
}

// Nollst�ller package_counter.
ISR(TIMER1_OVF_vect)
{
	package_counter = 0;
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


int main(void)
{
	//Initiera SPI
	SPI_init();
	package_counter = 0;
	timer1_init();
	// set Global Interrupt Enabel
	sei();
	// loop forever
	for (;;)
	{
		
	}
}