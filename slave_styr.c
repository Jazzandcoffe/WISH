/*
 *  slave_styr.c
 *
 *  Created: 3/31/2014 1:42:57 PM
 *  Author:	Herman Molinder		hermo276@student.liu.se
 *			Tore Landén			torla816@student.liu.se
 */

#include <avr/io.h>
#include <avr/interrupt.h>

//Globala variabler
volatile int	package_counter;	// Håller reda på vilken typ av data som förväntas på bussen. Förväntar ny sändning då == 0
volatile char	transmit_buffer;	// Data som ska skickas
volatile char	recieve_buffer;		// Data som tas emot
// Mottagen data
volatile char	type_recieved;		// SPI
volatile char	data_recieved;		// SPI
volatile char	check_recieved;		// SPI
// Data att sända
volatile char	type_transmit;		// SPI
volatile char	data_transmit;		// SPI
volatile char	check_transmit;		// SPI


//Initierar SPI Slave
void SPI_init(void)
{
	//spi pins on port b, MISO output, all other input
	DDRB = (1<<DDB6);
	// SPI enable, Slave 
	SPCR = (1<<SPE) | (1<<SPIE); 
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

// Denna funktion hanterar inkommande och utgående data på bussen.
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
		SPI_control();
		transmit_buffer = type_transmit;
		//uppdatera package_counter
		package_counter = 0;
	}
}

/* 
 * Kontroll att mottagen data är ok!
 * Samt uppdatering av räknare för data att skicka
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
		// update data_transmit och type_transmit
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05: 
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0A: 
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F: 
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		// und so weiter
		case 0xFF:
		// sista typen som ska uppdateras
		// update_transmit_buffer();
		default:
		// update_transmit_buffer();			
		}
		check_transmit = check_creator(type_transmit, data_transmit);
	}
	else
	{
		// update_transmit_buffer()
	}
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

// Avbrottsvektorn SPI Serial Transfer Complete
ISR(SPI_STC_vect)
{
	// Vänta på att transfer blir klar.
	while(!(SPSR & (1<<SPIF)))
	;
	// Läs av inskiftad data
	recieve_buffer = SPDR;
	// Uppdatera SPI Data Register för utskiftning.
	SPDR = transmit_buffer;
	// kontroll och hantering av dataflöde
	SPI_decoder();
	//reset timer1
	TCNT1 = 0;
}

// Nollställer package_counter.
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