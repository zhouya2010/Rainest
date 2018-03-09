/*
 * write.c
 *
 * Created: 2016/4/29 9:31:10
 *  Author: Administrator
 */ 
#include "rw_buffer.h"
#include "ring_buffer.h"

#define BUFFER_IN_SIZE 1024
#define BUFFER_OUT_SIZE 1024

static struct ring_buffer ring_in;
static struct ring_buffer ring_out;

static uint8_t buffer_in[BUFFER_IN_SIZE];
static uint8_t buffer_out[BUFFER_OUT_SIZE];

void buffer_init(void)
{
	ring_in = ring_buffer_init(buffer_in, BUFFER_IN_SIZE);
	ring_out = ring_buffer_init(buffer_out, BUFFER_OUT_SIZE);
}


static int buffer_write(struct ring_buffer *ring, uint8_t * data, uint16_t len)
{	
	int i = 0;
	while(len && (!ring_buffer_is_full(ring)))
	{
		ring_buffer_put(ring, *data);
		data++;
		len--;
		i++;
	}
	return i;
}

static int buffer_read(struct ring_buffer *ring, uint8_t * buf, uint16_t len)
{
	int i = 0;
	while(len && (!ring_buffer_is_empty(ring)))
	{
		*buf = ring_buffer_get(ring);
		buf++;
		len--;
		i++;
	}
	return i;
}


static int buffer_available(struct ring_buffer *ring)
{
	return (ring->write_offset < ring->read_offset) ? (ring->write_offset + ring->size - ring->read_offset) : (ring->write_offset - ring->read_offset);
}

static void buffer_fresh(struct ring_buffer *ring)
{
	ring->write_offset = 0;
	ring->read_offset = 0; 
}

void buffer_out_fresh(void)
{
	buffer_fresh(&ring_out);
}

int16_t buffer_out_write(uint8_t * src , uint16_t len)
{	
	return buffer_write(&ring_out, src, len);	
}

int16_t buffer_out_read(uint8_t * buf, uint16_t len)
{
	return buffer_read(&ring_out, buf, len);
}

int16_t buffer_out_available(void)
{
	return buffer_available(&ring_out);
}

int16_t buffer_in_write(uint8_t * src , uint16_t len)
{
	return buffer_write(&ring_in, src, len);
}

int16_t buffer_in_read(uint8_t * buf, uint16_t len)
{
	return buffer_read(&ring_in, buf, len);
}

uint8_t buffer_in_read_byte(void)
{
	return ring_buffer_get(&ring_in);
}

int16_t buffer_in_available(void)
{
	return buffer_available(&ring_in);
}

void buffer_in_write_byte(uint8_t data)
{
	if (!ring_buffer_is_full(&ring_in))
	{
		ring_buffer_put(&ring_in,data);
	}
}

