/*!\file spi.c \brief Stellt die SPI-Schnittstelle bereit */
//***************************************************************************
//*            spi.c
//*
//*  Mon Jul 31 21:46:47 2006
//*  Copyright  2006  User
//*  Email
///	\ingroup hardware
///	\defgroup SPI Die SPI-Schnittstelle (spi.c)
///	\code #include "spi.h" \endcode
///	\par Uebersicht
///		Die SPI-Schnittstelle fuer den AVR-Controller
//****************************************************************************/
//@{
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "spi.h"
#include <avr/io.h>

unsigned char SPI_InitState = 0;
 
/* -----------------------------------------------------------------------------------------------------------*/
/*! Die Init fuer dir SPI-Schnittstelle. Es koennen verschiedene Geschwindigkeiten eingestellt werden.
 * \param 	Options		Hier kann die Geschwindigkeit der SPI eingestellt werden.
 * \retval	Null wenn alles richtig eingestellt wurde.
 */
/* -----------------------------------------------------------------------------------------------------------*/
unsigned int SPI_init( unsigned int Options )
{

	// SCK auf Hi setzen
	SPI_PORT |= 1<<SCK;
	// MOSI, SCK, SS als Output
	SPI_DDR  |= 1<<MOSI | 1<<SCK | 0<<SS; // mosi, sck, ss output
	// MISO als Input
	SPI_DDR  &= ~( 1<<MISO ); // miso input
	// Master mode
	SPCR |= 1<<MSTR | 1<<SPE;
	
	if ( Options == SPI_HALF_SPEED )
	{
		SPSR |= 0<<SPI2X;
		SPI_InitState = SPI_HALF_SPEED;
	}

	if ( Options == SPI_FULL_SPEED )
	{
		SPSR |= 1<<SPI2X;
		SPI_InitState = SPI_FULL_SPEED;
	}
	return( 0 );
}

/* -----------------------------------------------------------------------------------------------------------*/
/*! Schreibt einen Wert auf den SPI-Bus. Gleichzeitig wird ein Wert von diesem im Takt eingelesen.
 * \warning	Auf den SPI-Bus sollte vorher per Chip-select ein Baustein ausgewaehlt werden. Dies geschied nicht in der SPI-Routine sondern
 * muss von der Aufrufenden Funktion gemacht werden.
 * \param 	Data	Der Wert der uebertragen werden soll.
 * \retval  Data	Der wert der gleichzeit empfangen wurde.
 */
/* -----------------------------------------------------------------------------------------------------------*/
unsigned char SPI_ReadWrite( unsigned char Data )
{
	// daten senden
	SPDR = Data;
	// auf fertig warten
	while(!(SPSR & (1<<SPIF)));
	// empfangende daten einlesen
	Data = SPDR;
	// daten zurueckgeben
	return( Data );
}

/* -----------------------------------------------------------------------------------------------------------*/
/*! Eine schnelle MEM->SPI Blocksende Routine mit optimierungen auf Speed.
 * \param	buffer		Zeiger auf den Puffer der gesendet werden soll.
 * \param	Datalenght	Anzahl der Bytes die gesedet werden soll.
 */
/* -----------------------------------------------------------------------------------------------------------*/
void SPI_FastMem2Write( unsigned char * buffer, unsigned int Datalenght )
{
	unsigned int Counter = 0;
	unsigned char data;
	
	// erten Wert senden
	SPDR = buffer[ Counter++ ];
	while( Counter < Datalenght )
	{
		// Wert schon mal in Register holen, schneller da der Wert jetzt in einem Register steht und nicht mehr aus dem RAM geholt werden muss
		// nachdem das senden des vorherigen Wertes fertig ist,
		data = buffer[ Counter ];
		// warten auf fertig
		while(!(SPSR & (1<<SPIF)));
		// Wert aus Register senden
		SPDR = data;
		Counter++;
	}
	while(!(SPSR & (1<<SPIF)));
	return;
}

/* -----------------------------------------------------------------------------------------------------------*/
/*! Eine schnelle SPI->MEM Blockempfangroutine mit optimierungen auf Speed.
 * \warning Auf einigen Controller laufen die Optimierungen nicht richtig. Bitte teil des Sourcecode der dies verursacht ist auskommentiert.
 * \param	buffer		Zeiger auf den Puffer wohin die Daten geschrieben werden sollen.
 * \param	Datalenght	Anzahl der Bytes die empfangen werden sollen.
 */
/* -----------------------------------------------------------------------------------------------------------*/
void SPI_FastRead2Mem( unsigned char * buffer, unsigned int Datalenght )
{
	unsigned int Counter = 0;
	unsigned char data;
	
	// dummywrite
	SPDR = 0x00;

	while( Counter < Datalenght )
	{
		// warten auf fertig
		while(!(SPSR & (1<<SPIF)));
/*
		// einfache Optimierung
		// Daten einlesen
		data = SPDR;
		// dummy-write
		SPDR = 0x00;		
*/
		// bessere Optimierung, aber nicht auf jeden controller
		// dummy-write
		SPDR = 0x00;		
		// Daten einlesen
		data = SPDR;

		// speichern
		buffer[ Counter++ ] = data;
	}
	while(!(SPSR & (1<<SPIF)));
	return;
}

/* -----------------------------------------------------------------------------------------------------------*/
/*! Hier wird der InitStatus abgefragt um zu sehen ob die Schnittstelle schon Eingestellt worden ist.
 * \retval  Status
 */
/* -----------------------------------------------------------------------------------------------------------*/
unsigned char SPI_GetInitState( void )
{
	return( SPI_InitState );
}

//@}
