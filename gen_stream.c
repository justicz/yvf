/* minimal code example showing how to call the zfp (de)compressor */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "zfp/inc/zfp.h"

/* compress or decompress array */
static int
compress(float* array, int nx, float tolerance, char *outdir, char *name, size_t n)
{
	int status = 0;		 /* return value: 0 = success */
	zfp_type type;		 /* array scalar type */
	zfp_field* field;  /* array meta data */
	zfp_stream* zfp;	 /* compressed stream */
	void* buffer;			 /* storage for compressed stream */
	size_t bufsize;		 /* byte size of compressed buffer */
	bitstream* stream; /* bit stream to write to or read from */
	size_t zfpsize;		 /* byte size of compressed stream */
	FILE *fout;
	char filename[100];

	snprintf(filename, sizeof(filename), "%s/out%s_%06d.fcp", outdir, name, n);

	fout = fopen(filename, "w");

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

	zfpsize = zfp_compress(zfp, field);
	if (!zfpsize) {
		fprintf(stderr, "compression failed\n");
		status = 1;
	}
	else
		fwrite(buffer, 1, zfpsize, fout);

	fclose(fout);
	zfp_field_free(field);
	zfp_stream_close(zfp);
	stream_close(stream);
	free(buffer);
	free(array);

	return status;
}

void parse_floats(char *fin, float **out, size_t *len)
{

	FILE *fp;
	fp = fopen(fin, "r");
	char line[256];

	size_t num_lines = 0;

	while(fgets(line, sizeof(line), fp))
	{
		num_lines++;
	}

	// Should always have xyz, xyz, xyz...
	assert(num_lines % 3 == 0);

	*len = num_lines / 3;

	for (int i = 0; i < 3; i++)
	{
		out[i] = (float *) malloc(*len * sizeof(float));
	}

	fprintf(stderr, "%d lines\n", num_lines);

	// Seek back to the beginning
	rewind(fp);

	size_t ind = 0;
	while(fgets(line, sizeof(line), fp))
	{
		if (ind < num_lines) {
			out[ind % 3][ind / 3] = (float)atof(line);
			ind++;
		}
	}

	fclose(fp);
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("Need input folder, outdir, num_files\n");
		exit(1);
	}

	// Enter the output directory
	char *indir = argv[1];
	char *outdir = argv[2];
	size_t count = atoi(argv[3]);
	char filename[100];

	for (size_t i = 1; i < count + 1; i++)
	{
		float *xyz[3];
		size_t len;
		snprintf(filename, sizeof(filename), "%s/out0_%06d.txt", indir, i);
		parse_floats(filename, xyz, &len);
		compress(xyz[0], len, 1e-4, outdir, "x", i);
		compress(xyz[1], len, 1e-4, outdir, "y", i);
		compress(xyz[2], len, 1e-4, outdir, "z", i);
	}

	return 0;
}
