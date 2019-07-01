CFLAGS=-g -Wall -Werror -pipe
c_token: c_token.c
	cppcheck --quiet --enable=all $<
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f c_token
