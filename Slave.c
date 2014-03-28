//Slave
#include <avr/io.h>
#include <avr/interrupt.h>

//Globala variabler
volatile char transmit_buffer; //Data som ska skickas
volatile char recieve_buffer; //Data som tas emot
volatile char type; //typ-byte till bussprotokoll
volatile char data; //data-byte till bussprotokoll
volatile char check; //check-byte till bussprotokoll
volatile int SPI_count; //Håller reda på om busspaket är type, data eller check

//Initierar SPI Slave
void SPI_init(void)
{
	DDRB = (1<<DDB6); //spi pins on port b, MISO output, all other input
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

	/*
	Här ska det vara kod för att spara undan recieve buffer och
	lägga in ny data i transmit_buffer
	*/

	SPDR =  transmit_buffer;
}

int main(void)
{
	SPI_init();

	sei(); // set Global Interrupt Enable

	// loop forever
	for (;;)
	{

	}
}
