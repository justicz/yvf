/* minimal code example showing how to call the zfp (de)compressor */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"
#include "zfp/inc/zfp.h"

static size_t frame_number = 1;
static size_t num_vertices;
static size_t num_frames;
static size_t frames_per_chunk;
static char *indir;
static float *xyz[3];
static float tolerance;

/* compress or decompress array */
static int
decompress(float* array, char *name, size_t n)
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
	field = zfp_field_2d(array, type, num_vertices, frames_per_chunk);

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

float xyz_to_frame(float **xyz, float *frame, size_t offset)
{
	for (size_t i = 0; i < num_vertices; i++)
	{
		frame[3*i + 0] = xyz[0][i + offset * num_vertices];
		frame[3*i + 1] = xyz[1][i + offset * num_vertices];
		frame[3*i + 2] = xyz[2][i + offset * num_vertices];
	}	
}

void get_frame(float *frame)
{
	// Stop if we're at the end of the animation
	if (frame_number == num_frames)
	{
		return;
	}
	size_t offset = frame_number % frames_per_chunk;
	// Decompress a frame if we need to
	if (offset == 0)
	{
		size_t file_number = frame_number / frames_per_chunk;
		decompress(xyz[0], "z", file_number);
		decompress(xyz[1], "x", file_number);
		decompress(xyz[2], "y", file_number);
	}
	xyz_to_frame(xyz, frame, offset);
	printf("decoded frame %zu\n", frame_number);
	frame_number++;
}

void init_decoder(char *dir, size_t n_vertices, size_t n_frames, size_t fpc, float tol)
{
	num_frames = n_frames;
	num_vertices = n_vertices;
	indir = dir;
	tolerance = tol;
	frames_per_chunk = fpc;
	for (size_t i = 0; i < 3; i++)
	{
		xyz[i] = (float *) malloc(sizeof(float) * num_vertices * frames_per_chunk);
	}
	if (!xyz[0] || !xyz[1] || !xyz[2])
	{
		fprintf(stderr, "couldn't allocate xyz buffer");
		exit(1);
	}
	printf("decoder initialized\n");
}

