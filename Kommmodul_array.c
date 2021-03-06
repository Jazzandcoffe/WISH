/*
*  Komm.modul array.c
*
*  Created:		04/30/2014 1:42:57 PM
*  Author:		Herman Molinder		hermo276@student.liu.se
*				Tore Land�n			torla816@student.liu.se
*/

#define F_CPU 14745600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

//Globala variabler
volatile uint16_t		init_transmit;		// F�r att h�lla reda p� n�r vi ska anv�nda buss.
volatile uint16_t		auto_or_manual; 	// autonomt l�ge = 0/manuellt l�ge = 1
volatile uint16_t		i;					// index till bt_buffer
volatile unsigned char	type_sens;			// typ-byte till protokollet
volatile unsigned char	type_styr;			// typ-byte till protokollet
volatile unsigned char	check;				// check-byte till protokollet
volatile unsigned char	spi_recieve_buffer; // Data som tas emot
volatile unsigned char	data[35];			// data-byte till protokollet
volatile unsigned char	bt_buffer[32];		// Buffer f�r mottagen bluetoothdata



// Initiera avbrottsport int0
void INT0_init()
{
	EICRA = (1<<ISC01);
	EIMSK = (1<<INT0);
}

// Initiera USART0 f�r BT kommunikation.
void USART0_init(long baud_rate)
{
	//st�lla in register f�r kommunikation i 115200Hz
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
		// V�nta tills det �r ok att skriva till UDR
		while((UCSR0A & (1<<UDRE0)) == 0);
		UDR0 = to_send;
	}
}

// Recieve complete
ISR(USART0_RX_vect)
{
	// V�nta - tills mottagningen klar och ok att l�sa fr�n UDR0
	while ((UCSR0A & (1 << RXC0)) == 0);
	bt_buffer[i] = UDR0;
	i++;
}

//Initierar SPI Master
void SPI_init(void)
{
	//spi pins on port b MOSI SCK,SS1,SS2 outputs
	DDRB = ((1<<DDB7)|(1<<DDB5)|(1<<DDB4)|(1<<DDB3));
	// SPI enable, Master, (1<<SPR0) f�r f/16
	SPCR = ((1<<SPE)|(1<<MSTR));
}

//Transmit function. to_send p� MOSI. Return MISO.
unsigned char SPI_transmit(unsigned char to_send)
{
	SPDR = to_send;
	while(!(SPSR & (1<<SPIF)))
	;
	return SPDR;
}
//initierar timer1 (28Hz)
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
* SS f�r sensor, vi skickar aldrig,
* s� denna funktion l�ser vad som
* sensor lagt p� bussen
*/
unsigned char ss_sensor()
{
	PORTB = PORTB & 0b11110111;
	spi_recieve_buffer = SPI_transmit(0x00);
	PORTB = PORTB | 0b00001000;
	return spi_recieve_buffer;
}

/*
* SS f�r styr, vi skickar sensordata,
* tar emot styrbeslut.(som ska skickas via BT)
*/
unsigned char ss_styr(unsigned char to_send)
{
	PORTB = PORTB & 0b11101111;
	spi_recieve_buffer = SPI_transmit(to_send);
	PORTB = PORTB | 0b00010000;
	return spi_recieve_buffer;
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
	// Hantering av data som lagrats i en buffer. All data hanteras och buffern t�ms.
	while(i > 1)
	{
		type_styr = bt_buffer[i-2];
		data[type_styr] = bt_buffer[i-1];
		i = i-2;
		
		// N�dstopp
		if(type_styr == 0x04)
		{
			ss_styr(0x04);
			_delay_us(20);
			ss_styr(0x00);
			_delay_us(20);
			ss_styr(check_creator(0x04, 0x00));
		}
		//Kp - regleringsparameter
		else if(type_styr == 0x21)
		{
			ss_styr(type_styr);
			_delay_us(20);
			ss_styr(data[type_styr]);
			_delay_us(20);
			ss_styr(check_creator(type_styr, data[type_styr]));
		}
		//Kd - regleringsparameter
		else if(type_styr == 0x22)
		{
			ss_styr(type_styr);
			_delay_us(20);
			ss_styr(data[type_styr]);
			_delay_us(20);
			ss_styr(check_creator(type_styr, data[type_styr]));
		}
		// sensorfeedback!				///////ej implementerat.
		else if(type_styr == 0x23)
		{
			ss_styr(type_styr);
			_delay_us(20);
			ss_styr(data[type_styr]);
			_delay_us(20);
			ss_styr(check_creator(type_styr, data[type_styr]));
		}
		// Manuell/Autonom
		else if(type_styr == 0x00)
		{
			if(data[type_styr] == 0x00)
			{
				auto_or_manual = 1;
				//Skicka kommando f�r manuellt l�ge.
				ss_styr(0x00);
				_delay_us(20);
				ss_styr(0x00);
				_delay_us(20);
				ss_styr(check_creator(0x00, 0x00));
			}
			else if(data[type_styr] == 0xFF)
			{
				auto_or_manual = 0;
				//Skicka kommando f�r autonomt l�ge
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
	//Skicka kommando f�r autonomt l�ge
	ss_styr(0x00);
	_delay_us(20);
	ss_styr(0xFF);
	_delay_us(20);
	ss_styr(check_creator(0x00, 0xFF));
}

ISR(USART0_TX_vect){}

void InitProcessor()
{
	//Initiera USART f�r BT-kommunikation
	USART0_init(115200);
	//RTS l�g
	PORTD = (0<<PD4);
	//Initiera SPI-bussen
	SPI_init();
	//Initiera Timer1 f�r aktivering i 28Hz
	timer1_init();
	//Initiera INT0 f�r avbrott
	INT0_init();
	// set Global Interrupt Enable
	sei();
	//Starta i manuellt l�ge
	auto_or_manual = 1;
	//Skicka kommando f�r manuellt l�ge.
	ss_styr(0x00);
	_delay_us(20);
	ss_styr(0x00);
	_delay_us(20);
	ss_styr(check_creator(0x00, 0x00));
	//Index f�r BT-buffer
	i = 0;
}

int main(void)
{
	// Initiera processorn
	InitProcessor();
	
	// loop forever (main loop)
	for (;;)
	{
		//Autonomt l�ge
		if(auto_or_manual == 0)
		{
			// Uppdatera sensordata och vidarebefordra till styr och klient.
			for (int n = 0; n < 27; n++)
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
				}
			}
			if(check_decoder(type_styr, data[type_styr], check))
			{
				USART0_transmit(type_styr);
				USART0_transmit(data[type_styr]);
			}
		}
		//Manuellt l�ge
		else
		{
			// Uppdatera styrparametrar n = 1:4 - s�nt till styr
			for (unsigned char n = 1; n < 4; n++)
			{
				type_styr = n;
				ss_styr(type_styr);
				_delay_us(20);
				ss_styr(data[type_styr]);
				_delay_us(20);
				ss_styr(check_creator(type_styr, data[type_styr]));
				_delay_us(20);
			}
			// Uppdatera och vidarebefordra sensordata till klient.
			for (int n = 0; n < 27; n++)
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
		}
		init_transmit = 0;
		// Hantera bt_buffer och uppdatera styrbeslut
		USART0_recieve();
		while(init_transmit == 0)
		{
			sleep_enable();
			sleep_cpu();
			sleep_disable();
		}
	}
}