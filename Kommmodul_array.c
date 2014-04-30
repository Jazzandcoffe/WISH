/*
*  Kommunikationsmodul.c
*
*  Created: 3/26/2014 1:42:57 PM
*  Author:	Herman Molinder		hermo276@student.liu.se
*			Tore Landén			torla816@student.liu.se
*/

#define F_CPU 14745600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Globala variabler
volatile uint16_t		init_transmit;		// För att hålla reda på när vi ska använda buss.
volatile uint16_t		auto_or_manual; 	// autonomt läge = 0/manuellt läge = 1
volatile unsigned char	recieve_buffer; 	// Data som tas emot
volatile unsigned char	type_sens;			// typ-byte till protokollet
volatile unsigned char	check;				// check-byte till protokollet
volatile unsigned char	type_styr;			// typ-byte till protokollet
volatile unsigned char	data_styr;
volatile unsigned char	data_sens;
volatile unsigned char	data[32];			// data-byte till protokollet
volatile unsigned char	bt_buffer[32];		// Buffer för mottagen bluetoothdata
volatile uint16_t		i;					// index till bt_buffer


// Initiera avbrottsport int0
void INT0_init()
{
	EICRA = (1<<ISC01);
	EIMSK = (1<<INT0);
}

// Initiera USART0 för BT kommunikation.
void USART0_init(long baud_rate)
{
	//ställa in register för kommunikation i 115200Hz
	UBRR0H = ((F_CPU/16) / baud_rate - 1) >> 8;
	UBRR0L = ((F_CPU/16) / baud_rate - 1);
	// Enable receiver and transmitter & their interrupts.
	UCSR0B = (1<<RXCIE0)|(1<<TXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	// Set frame format: 8data
	UCSR0C = (1 << UCSZ00)|(1<<UCSZ01);
	//PD4 (RTS) output, PD5 (CTS) input
	DDRD = (1<<DDD4);
}

void USART0_transmit(unsigned char to_send)
{
	if(!(PIND & (1<<PD5)))
	{
		// Vänta tills det är ok att skriva till UDR
		while((UCSR0A & (1<<UDRE0)) == 0);
		UDR0 = to_send;
	}
}

// Recieve complete
ISR(USART0_RX_vect)
{
	// Vänta - tills mottagningen klar och ok att läsa från UDR0
	while ((UCSR0A & (1 << RXC0)) == 0);
	bt_buffer[i] = UDR0;
	i++;
}

//Initierar SPI Master
void SPI_init(void)
{
	//spi pins on port b MOSI SCK,SS1,SS2 outputs
	DDRB = ((1<<DDB7)|(1<<DDB5)|(1<<DDB4)|(1<<DDB3));
	// SPI enable, Master, (1<<SPR0) för f/16
	SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0));
}

//Transmit function. to_send på MOSI. Return MISO.
unsigned char SPI_transmit(unsigned char to_send)
{
	SPDR = to_send;
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

/*
* SS för sensor, vi skickar aldrig,
* så denna funktion läser vad som
* sensor lagt på bussen
*/
unsigned char ss_sensor()
{
	PORTB = PORTB & 0b11110111;
	recieve_buffer = SPI_transmit(0x00);
	PORTB = PORTB | 0b00001000;
	return recieve_buffer;
}

/*
* SS för styr, vi skickar sensordata,
* tar emot styrbeslut.(som ska skickas via BT)
*/
unsigned char ss_styr(unsigned char to_send)
{
	PORTB = PORTB & 0b11101111;
	recieve_buffer = SPI_transmit(to_send);
	PORTB = PORTB | 0b00010000;
	return recieve_buffer;
}

// check_decoder returnerar 1 om check == type XOR data, annars 0.
unsigned char check_decoder(unsigned char type, unsigned char data, unsigned char check)
{
	unsigned char is_check = type^data;
	return (check == is_check);
}

// Returnerar en "checksum" = type XOR data.
unsigned char check_creator(unsigned char type, unsigned char data)
{
	return type^data;
}

void USART0_recieve()
{
	while(i > 1)
	{
		type_styr = bt_buffer[i-2];
		data[type_styr] = bt_buffer[i-1];
		i = i-2;
		if(type_styr == 0x00)
		{
			if(data[type_styr] == 0x00)
			{
				auto_or_manual = 1;
				//Skicka kommando för manuellt läge.
				ss_styr(0x00);
				_delay_us(20);
				ss_styr(0x00);
				_delay_us(20);
				ss_styr(check_creator(0x00, 0x00));
			}
			else if(data[type_styr] == 0xFF)
			{
				auto_or_manual = 0;
				//Skicka kommando för autonomt läge
				ss_styr(0x00);
				_delay_us(20);
				ss_styr(0xFF);
				_delay_us(20);
				ss_styr(check_creator(0x00, 0xFF));
			}
		}
	}
}

ISR(INT0_vect)
{
	auto_or_manual = 0;
	//Skicka kommando för autonomt läge
	ss_styr(0x00);
	_delay_us(20);
	ss_styr(0xFF);
	_delay_us(20);
	ss_styr(check_creator(0x00, 0xFF));
}

ISR(USART0_TX_vect){}

int main(void)
{
	USART0_init(115200);
	SPI_init();
	timer1_init();
	INT0_init();
	// set Global Interrupt Enable
	sei();
	auto_or_manual = 1;
	
	i = 0;
	//RTS låg
	PORTD = (0<<PD4);
	
	//Skicka kommando för manuellt läge.
	ss_styr(0x00);
	_delay_us(20);
	ss_styr(0x00);
	_delay_us(20);
	ss_styr(check_creator(0x00, 0x00));

	// loop forever
	for (;;)
	{
		if (init_transmit==1)
		{
			//Autonomt läge
			if(auto_or_manual == 0)
			{
				for (int n = 0; n < 20; n++)
				{
					type_sens = ss_sensor();
					_delay_us(20);
					data[type_sens] = ss_sensor();
					_delay_us(20);
					check = ss_sensor();
					
					if(check_decoder(type_sens, data[type_sens], check))
					{
						USART0_transmit(type_sens);
						USART0_transmit(data[type_sens]);
						
						type_styr = ss_styr(type_sens);
						_delay_us(20);
						data[type_styr] = ss_styr(data[type_sens]);
						_delay_us(20);
						check = ss_styr(check);
						
						if(check_decoder(type_styr, data[type_styr], check))
						{
							USART0_transmit(type_styr);
							USART0_transmit(data[type_styr]);
						}
					}
				}
				init_transmit = 0;
			}
			//Manuellt läge
			else
			{
				for (unsigned char n = 1; n < 5; n++)
				{
					type_styr = n;
					data_styr = data[type_styr];
					ss_styr(type_styr);
					_delay_us(20);
					ss_styr(data_styr);
					_delay_us(20);
					check = check_creator(type_styr, data_styr);
					ss_styr(check);
					_delay_us(20);
				}
				
				for (int n = 0; n < 20; n++)
				{
					type_sens = ss_sensor();
					_delay_us(20);
					data[type_sens] = ss_sensor();
					_delay_us(20);
					check = ss_sensor();
					if(check_decoder(type_sens, data[type_sens], check))
					{
						USART0_transmit(type_sens);
						USART0_transmit(data[type_sens]);
					}
				}
				init_transmit = 0;
			}
			//kontrollera mottagen data
			USART0_recieve();
		}
	}
}