/*
 *  slave_sensor.c
 *
 *  Created: 03/31/2014 1:42:57 PM
 *  Author:	Herman Molinder		hermo276@student.liu.se
 *			Tore Landén			torla816@student.liu.se
 */

#include <avr/io.h>
#include <avr/interrupt.h>

//Globala variabler
volatile int	package_counter;	//Håller reda på vilken typ av data som förväntas på bussen. Förväntar ny sändning då == 0
volatile char	transmit_buffer;	//Data som ska skickas
volatile char	type_transmit;		//SPI
volatile char	data_transmit;		//SPI
volatile char	check_transmit;		//SPI

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

//Initierar SPI Slave
void SPI_init(void)
{
	//spi pins on port b, MISO output, all other input
	DDRB = (1<<DDB6);
	// SPI enable, Slave 
	SPCR = (1<<SPE) | (1<<SPIE); 
}


// Returnerar en "checksum" = type XOR data.
char check_creator(char type,char data)
{
	return type^data;
}

//
void SPI_write()
{
	//Här ska det läggas in ny data att skicka nästa gång på bussen
	type_transmit = 0xFF;
	data_transmit = 0x0F;
	
	check_transmit = check_creator(type_transmit, data_transmit);
}

// Denna funktion hanterar inkommande och utgående data på bussen.
void SPI_transfer_update()
{
	switch(package_counter)
	{
		// type
		case 0: 
		transmit_buffer = data_transmit;
		//uppdatera package_counter
		package_counter = 1;
		break;
		
		// data
		case 1: 
		transmit_buffer = check_transmit;
		//uppdatera package_counter
		package_counter = 2;
		break;
		
		// check
		case 2: 
		SPI_write();
		transmit_buffer = type_transmit;	
		//uppdatera package_counter
		package_counter = 0;
	}
}

/*
 * Avbrottsvektorn SPI Serial Transfer Complete
 * I denna funktion ska "buffer = SPI_Transmit()" köras för att
 * lagra inskiftad data i buffer.
 * Sedan ska man skriva ny data till SPDR
 */
ISR(SPI_STC_vect)
{
	SPI_transfer_update();
	SPDR = transmit_buffer;
	//reset timer1
	TCNT1 = 0;
}

// Nollställer package_counter.
ISR(TIMER1_OVF_vect)
{
	package_counter = 0;
}

int main(void)
{
	//Initiera SPI
	SPI_init();
	package_counter = 0;
	timer1_init();
	// set Global Interrupt Enable
	sei();
	// loop forever
	
	for (;;)
	{
		
	}
}