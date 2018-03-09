/*!\file spi.h \brief Stellt die SPI-Schnittstelle bereit*/
//***************************************************************************
//*            spi.h
//*
//*  Mon Jul 31 21:46:47 2006
//*  Copyright  2006  User
//*  Email
///	\ingroup hardware
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
 
#ifndef _SPI_H
#define _SPI_H
	
#include <avr/io.h>

unsigned int SPI_init( unsigned int Options );
unsigned char SPI_ReadWrite( unsigned char Data );
unsigned char SPI_GetInitState( void );
void SPI_FastRead2Mem( unsigned char * buffer, unsigned int Datalenght );
void SPI_FastMem2Write( unsigned char * buffer, unsigned int Datalenght );
	
#define SPI_NOT_INIT	0x00
#define SPI_HALF_SPEED	0x01
#define SPI_FULL_SPEED	0x02
	
#define SPI_PORT		PORTB
#define SPI_DDR			DDRB
#define SS				PB4
#define MISO			PB6
#define MOSI			PB5
#define SCK				PB7

#endif /* _SPI_H */
//@}
