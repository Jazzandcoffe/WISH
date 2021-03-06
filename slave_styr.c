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
volatile int	package_counter;	// H�ller reda p� vilken typ av data som f�rv�ntas p� bussen. F�rv�ntar ny s�ndning d� == 0
volatile char	transmit_buffer;	// Data som ska skickas
volatile char	recieve_buffer;		// Data som tas emot
// Mottagen data
volatile char	type_recieved;		// SPI
volatile char	data_recieved;		// SPI
volatile char	check_recieved;		// SPI
// Data att s�nda
volatile char	type_transmit;		// SPI
volatile char	data_transmit;		// SPI
volatile char	check_transmit;		// SPI


//Initierar SPI Slave
void SPI_init(void)
{
	//spi pins on port b, MISO output, all other input
	DDRB = (1<<DDB6);
	// SPI enable, Slave 
	SPCR = (1<<SPE)|(1<<SPIE);
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

/*
 * DENNA SKRIVER STYR
 */
void SPI_transmit_update()
{
	type_transmit = 0xAA; //Dummyv�rde
	data_transmit = 0x11; //Dummyv�rde
	check_transmit = check_creator(type_transmit, data_transmit);
}

/* 
 * DENNA SKRIVER STYR
 *
 * Kontroll att mottagen data �r ok!
 * Samt uppdatering av r�knare f�r data att skicka
 */
void SPI_control()
{
	if (check_decoder(type_recieved, data_recieved, check_recieved) )
	{
		switch (type_recieved)
		{
		case 0x00:
		// example
		// sensor_1 = data_recieved;
		break;
		case 0x01:
		break;
		case 0x02:
		break;
		case 0x03:
		break;
		case 0x04:
		break;
		case 0x05: 
		break;
		case 0x06:
		break;
		case 0x07:
		break;
		case 0x08:
		break;
		case 0x09:
		break;
		case 0x0A:
		break;
		case 0x0B:
		break;
		case 0x0C:
		break;
		case 0x0D:
		break;
		case 0x0E:
		break;
		case 0x0F: 
		break;
		case 0x10:
		break;
		case 0x11:
		break;
		case 0x12:
		break;
		// und so weiter
		case 0x13:
		// sista typen som ska uppdateras
		break;
		}
	}
	SPI_transmit_update();
}

// Denna funktion hanterar inkommande och utg�ende data p� bussen.
void SPI_transfer_update()
{
	switch(package_counter)
	{
		// type
		case 0: 
		type_recieved = recieve_buffer;
		transmit_buffer = data_transmit;
		package_counter = 1;
		break;
		
		// data
		case 1: 
		data_recieved = recieve_buffer;
		transmit_buffer = check_transmit;
		package_counter = 2;
		break;
		
		// check
		case 2: 
		check_recieved = recieve_buffer;
		SPI_control();
		transmit_buffer = type_transmit;
		package_counter = 0;
	}
}

// Avbrottsvektorn SPI Serial Transfer Complete
ISR(SPI_STC_vect)
{
	recieve_buffer = SPDR;
	SPI_transfer_update();
	SPDR = transmit_buffer;
	// Reset timer1
	TCNT1 = 0;
}

// Nollst�ller package_counter.
ISR(TIMER1_OVF_vect)
{
	package_counter = 0;
}

int main()
{
	//Initiera SPI
	SPI_init();
	//Initeiera timer function.
	timer1_init();
	// set Global Interrupt Enabel
	sei();
	// loop forever
	for (;;)
	{
		
	}
}