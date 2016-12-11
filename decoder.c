/* minimal code example showing how to call the zfp (de)compressor */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zfp/inc/zfp.h"

/* compress or decompress array */
static int
decompress(float* array, int nx, float tolerance, char *indir, char *name, size_t n)
{
	int status = 0;		 /* return value: 0 = success */
	zfp_type type;		 /* array scalar type */
	zfp_field* field;  /* array meta data */
	zfp_stream* zfp;	 /* compressed stream */
	void* buffer;			 /* storage for compressed stream */
	size_t bufsize;		 /* byte size of compressed buffer */
	bitstream* stream; /* bit stream to write to or read from */
	size_t zfpsize;		 /* byte size of compressed stream */
	FILE *fin;
	char filename[100];

	snprintf(filename, sizeof(filename), "%s/out%s_%06d.fcp", indir, name, n);
	fin = fopen(filename, "r");

	type = zfp_type_float;
	field = zfp_field_1d(array, type, nx);

	/* allocate meta data for a compressed stream */
	zfp = zfp_stream_open(NULL);

	/* set compression mode and parameters via one of three functions */
/*	zfp_stream_set_rate(zfp, rate, type, 3, 0); */
/*	zfp_stream_set_precision(zfp, precision, type); */
	zfp_stream_set_accuracy(zfp, tolerance, type);

	/* allocate buffer for compressed data */
	bufsize = zfp_stream_maximum_size(zfp, field);
	buffer = malloc(bufsize);

	/* associate bit stream with allocated buffer */
	stream = stream_open(buffer, bufsize);
	zfp_stream_set_bit_stream(zfp, stream);
	zfp_stream_rewind(zfp);

	/* compress or decompress entire array */
	zfpsize = fread(buffer, 1, bufsize, fin);
	if (!zfp_decompress(zfp, field)) {
		fprintf(stderr, "decompression failed\n");
		status = 1;
	}

	/* clean up */
	fclose(fin);
	zfp_field_free(field);
	zfp_stream_close(zfp);
	stream_close(stream);
	free(buffer);

	return status;
}

int main(int argc, char* argv[])
{
	/* use -d to decompress rather than compress data */

	if (argc != 4)
	{
		printf("Need input folder, num vertices, num_files\n");
		exit(1);
	}

	char *indir = argv[1];
	size_t len = atoi(argv[2]);
	size_t count = atoi(argv[3]);
	float *xyz[3];

	for (size_t i = 0; i < 3; i++)
	{
		xyz[i] = (float *) malloc(sizeof(float) * len);
	}

	for (size_t i = 1; i < count + 1; i++)
	{
		decompress(xyz[0], len, 1e-4, indir, "x", i);
		decompress(xyz[1], len, 1e-4, indir, "y", i);
		decompress(xyz[2], len, 1e-4, indir, "z", i);
	}

	free(xyz[0]);
	free(xyz[1]);
	free(xyz[2]);
}

