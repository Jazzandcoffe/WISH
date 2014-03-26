//Kommunikationsmodul
#include <avr/io.h>

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

//Sätter tiden och startar watchdog timer i interrupt-mode 64 ms.
void WDT_Init(void)
{
	__disable_interrupt();
	__watchdog_reset();
	//Start timed sequence
	WDTCSR |= (1<<WDCE) | (0<<WDE) | (1<<WDIE);
	// Set new prescaler (time-out) value = 8k cycles (64 ms)
	WDTCSR = (1<<WDP1);
	__enable_interrupt();
}

//Avbrottsvektorn till watchdog time-out interrupt
ISR(WDT_vect)
{
	transmit = 1;
}

//Globala variabler
volatile int init_transmit; //För att hålla reda på när vi ska använda buss.
volatile int transmit_buffer; //Data som ska skickas
volatile int recieve_buffer; //Data som tas emot

int main(void)
{
	SPI_init();
	
	__enable_interrupt(); // set Global Interrupt Enable
	
	transmit_buffer = 0;
	
	// loop forever
	for (;;)
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
		
		if(init_transmit==1)
		{
			PORTB = (0<<PB4);
			recieve_buffer = SPI_Transmit(transmit_buffer);
			PORTB = (1<<PB4);
			transmit = 0; 
		}
	}
}
