CFLAGS += -g `curl-config --cflags`

boundio-curl-sample: boundio-curl-sample.o
	$(CC) -o $@ $< `curl-config --libs` -ljson

boundio-curl-sample.o: boundio-curl-sample.c

clean:
	-rm *.o *~ boundio-curl-sample

indent:
	-indent -kr boundio-curl-sample.c
