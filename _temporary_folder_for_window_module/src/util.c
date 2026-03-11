#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char *file_alloc_and_read_to_buffer(const char *path)
{
	FILE  *fp = NULL;
	size_t sz = SIZE_MAX;
	char  *b  = NULL;

	/* Open file */
	fp = fopen(path, "rb");
	assert(fp);

	/* Go to end and get length */
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	assert(sz);

	/* Allocate buffer with size (+1) */
	b = malloc(sz + 1u);
	assert(b);

	/* Go to beginning of file and read data in */
	fseek(fp, 0, SEEK_SET);
	fread(b, 1u, sz, fp);
	b[sz] = '\0'; /* Make sure to terminate */

	/* Close the file */
	fclose(fp);

	/* Now we have our new buffer~! >w< */
	return b;
}
