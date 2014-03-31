/*
 *  slave_styr.c
 *
 *  Created: 3/31/2014 1:42:57 PM
 *  Author:	Herman Molinder		hermo276@student.liu.se
 *			Tore Land�n			torla816@student.liu.se
 */

#include <avr/io.h>
#include <avr/interrupt.h>

//Globala variabler
volatile int	package_counter;	//H�ller reda p� vilken typ av data som f�rv�ntas p� bussen. F�rv�ntar ny s�ndning d� == 0
volatile char	transmit_buffer;	//Data som ska skickas
volatile char	recieve_buffer;		//Data som tas emot
volatile char	type_recieved;		//SPI
volatile char	data_recieved;		//SPI
volatile char	check_recieved;		//SPI
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

//Transmit function. cData p� MISO. Return MOSI.
char SPI_read(void)
{
	while(!(SPSR & (1<<SPIF)))
	;
	return SPDR;
}


// Returnerar en "checksum" = type XOR data.
char check_creator(char type,char data)
{
	return type^data;
}


// check_decoder returnerar 1 om check == type XOR data, annars 0.  
unsigned int check_decoder(char type, char data, char check)
{
	char is_check = type^data;
	if(is_check == check)
	return 1;
	else return 0;
}

//
void SPI_write()
{
	/*
	type_transmit = ny;
	data_transmit = ny;
	
	check_transmit = check_creator(type_transmit, data_transmit);
	
	if ( check_decoder(type_recieved, data_recieved, check_recieved) )
	{
	ny = type_recieved;
	ny = data_recieved;
	}
	*/
}

// Denna funktion hanterar inkommande och utg�ende data p� bussen.
void SPI_decoder()
{
	switch(package_counter)
	{
		// type
		case 0: 
		type_recieved = recieve_buffer;
		transmit_buffer = data_transmit;
		
		//uppdatera package_counter
		package_counter++;
		
		// data
		case 1: 
		data_recieved = recieve_buffer;
		transmit_buffer = check_transmit;
		
		//uppdatera package_counter
		package_counter++;
		
		// check
		case 3: 
		check_recieved = recieve_buffer;
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
	recieve_buffer = SPI_read();
	SPI_decoder();
	SPDR =  transmit_buffer;
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
	// set Global Interrupt Enabel
	sei();
	// loop forever
	for (;;)
	{
		
	}
}