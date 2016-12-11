TARGETS = gen_stream decoder

all: $(TARGETS)

gen_stream: gen_stream.c zfp/lib/libzfp.a
	$(CC) -std=c99 gen_stream.c -L./zfp/lib -lzfp -lm -o gen_stream

decoder: decoder.c zfp/lib/libzfp.a
	$(CC) -std=c99 decoder.c -L./zfp/lib -lzfp -lm -o decoder

