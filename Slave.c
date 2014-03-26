//Slave-test
#include <avr/io.h>

//Initierar SPI Slave
void SPI_init(void)
{ 
	DDRB = ((1<<DDB6); //spi pins on port b, MISO output, all other input 
	SPCR = (1<<SPE);  // SPI enable, Slave 
} 

//Transmit function. cData på MISO. Return MOSI.
char SPI_Transmit(void){ 
	while(!(SPSR & (1<<SPIF))) 
		; 
	return SPDR; 
}

/*Avbrottsvektorn SPI Serial Transfer Complete
I denna funktion ska "buffer = SPI_Transmit()" köras för att
lagra inskiftad data i buffer.
Sedan ska man skriva ny data till SPDR*/
ISR(SPI_STC_vect)
{
	recieve_buffer = SPI_Transmit();
	
	transmit_buffer = recieve_buffer //testrad
	
	SPDR =  transmit_buffer
}

//Globala variabler
volatile int transmit_buffer; //Data som ska skickas
volatile int recieve_buffer; //Data som tas emot

int main(void)
{
	SPI_init();

	__enable_interrupt(); // set Global Interrupt Enable

	// loop forever
	for (;;)
	{
	
	}
}
