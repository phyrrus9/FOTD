#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "buffers.h"

unsigned char * ito4c(unsigned int val)
{
	unsigned char *c = (unsigned char *)malloc(4);
	memcpy(c, &val, 4);
	return c;
}

unsigned int c4toi(unsigned char * in)
{
	unsigned int i;
	memcpy(&i, in, 4);
	return i;
}

void place_size(unsigned char * out, unsigned int size)
{
	unsigned char *data = ito4c(size);

	for (int i = 0; i < 4; i++)
	{
		out[i] = data[i];
		//printf("%d ", out[i]);
	}
	//printf("\n");
}
