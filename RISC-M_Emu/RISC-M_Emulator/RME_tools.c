#include <stdint.h>

int32_t extend_sign(uint32_t val_in, uint8_t width)
{
	uint32_t bmsk;

	bmsk = ((1 << width) - 1);
	bmsk = ~bmsk;

	if (val_in & (1 << (width-1)))
		return (int32_t)(val_in | bmsk);
	else
		return (int32_t)val_in;
}
