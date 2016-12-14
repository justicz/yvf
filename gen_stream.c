/* minimal code example showing how to call the zfp (de)compressor */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "zfp/inc/zfp.h"

/* compress or decompress array */
static int
compress(float* array, size_t nx, size_t nt, float tolerance, char *outdir, char *name, size_t n)
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

	snprintf(filename, sizeof(filename), "%s/out%s_%06d.fcp", outdir, name, n + 1);

	printf("writing %s\n", filename);

	fout = fopen(filename, "w");

	type = zfp_type_float;
	field = zfp_field_2d(array, type, nx, nt);

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

void parse_floats(size_t start, size_t num_files, size_t num_verts, char *indir, float **out)
{

	char filename[100];
	char line[256];
	FILE *fp;

	for (int i = 0; i < 3; i++)
	{
		out[i] = (float *) malloc(sizeof(float) * num_files * num_verts);
	}

	printf("num_files: %zu, opening %zu-%zu\n", num_files, start + 1, start + num_files);

	size_t ind = 0;

	for (int f = 0; f < num_files; f++)
	{
		snprintf(filename, sizeof(filename), "%s/out0_%06d.txt", indir, start + f + 1);

		fp = fopen(filename, "r");

		printf("opening %s\n", filename);

		while(fgets(line, sizeof(line), fp))
		{
			out[ind % 3][ind / 3] = (float)atof(line);
			ind++;
		}

		fclose(fp);
	}

}

size_t count_verts(char *indir)
{
	char filename[100];
	char line[256];
	FILE * fp;
	snprintf(filename, sizeof(filename), "%s/out0_000001.txt", indir);
	fp = fopen(filename, "r");

	size_t num_lines = 0;

	while(fgets(line, sizeof(line), fp))
	{
		num_lines++;
	}

	fclose(fp);

	// Should always have xyz, xyz, xyz...
	assert(num_lines % 3 == 0);

	return num_lines / 3;
}

int main(int argc, char* argv[])
{
	if (argc != 6)
	{
		printf("Need input folder, outdir, num_files, frames_per_chunk, tolerance\n");
		exit(1);
	}

	// Enter the output directory
	char *indir = argv[1];
	char *outdir = argv[2];
	size_t count = atoi(argv[3]);
	size_t frames_per_chunk = atoi(argv[4]);
	float tolerance = atof(argv[5]);
	size_t len = count_verts(indir);
	size_t file_no = 0;

	printf("%zu vertices per file\n", len);

	for (int i = 0; i < count; i += frames_per_chunk)
	{
		size_t fpc = frames_per_chunk;
		if ((int)(count - i) - (int)frames_per_chunk < 0)
		{
			fpc = count % frames_per_chunk;
		}
		float *xyz_per_t[3];
		parse_floats(i, fpc, len, indir, xyz_per_t);
		compress(xyz_per_t[0], len, fpc, tolerance, outdir, "x", file_no);
		compress(xyz_per_t[1], len, fpc, tolerance, outdir, "y", file_no);
		compress(xyz_per_t[2], len, fpc, tolerance, outdir, "z", file_no);
		file_no++;
	}

	return 0;
}
