TARGETS = yfv gen_stream

all: $(TARGETS)

gen_stream: gen_stream.c zfp/lib/libzfp.a
	$(CC) -std=c99 gen_stream.c -L./zfp/lib -lzfp -lm -o gen_stream

decoder: decoder.c zfp/lib/libzfp.a
	$(CC) -std=c99 decoder.c -L./zfp/lib -lzfp -lm -o decoder

yfv: decoder.c gen_stream.c zfp/lib/libzfp.a draw.cpp
	gcc -std=c99 -O3 decoder.c -L./zfp/lib -lzfp -lm -c -o decoder.o
	g++ -std=c++11 -O3 draw.cpp -L./zfp/lib -lzfp -lGL -lSOIL -lGLEW -lglfw -DGLEW_STATIC -O3 -c -o draw.o 
	g++ -L./zfp/lib -lGL -lSOIL -lGLEW -lglfw -o draw draw.o decoder.o -lzfp
	rm decoder.o draw.o
