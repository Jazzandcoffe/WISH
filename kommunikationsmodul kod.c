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

int main(void)
{
	SPI_init();
	
	__enable_interrupt(); // set Global Interrupt Enable
	
	// loop forever
	for (;;)
	{
	}
}
