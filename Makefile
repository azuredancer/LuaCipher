version=0.01
name=lua-cipher
dist=$(name)-$(version)

CC=gcc
CFLAGS=-Wall -O3 -fPIC

#Uncomment if pkg-config available.
#LUA_CFLAGS=`pkg-config lua5.1 --cflags`

#Change to LUA include directory.
LUA_CFLAGS=-I/home/lz/lua/include

SRC_PATH=src/
CRYPTO_PATH=$(SRC_PATH)crypto/

#Change to LUA lib directory.
INSTALL_PATH=/home/lz/luax/

all: luacipher.so

luacipher.so: 
	$(CC) $(CFLAGS) $(LUA_CFLAGS) -shared $(SRC_PATH)luacipher.c $(CRYPTO_PATH)base64.c $(CRYPTO_PATH)des.c -o luacipher.so

clean:
	rm -f *.o *.so

install: luacipher.so
	install -D -s $< $(DESTDIR)$(INSTALL_PATH)/$<

dist:
	if [ -d $(dist) ]; then rm -r $(dist); fi
	mkdir $(dist)
	mkdir $(dist)/src
	mkdir $(dist)/src/crypto
	cp Makefile $(dist)/
	cp $(SRC_PATH)*.c $(dist)/src
	cp $(CRYPTO_PATH)*.c $(CRYPTO_PATH)*.h $(dist)/src/crypto
	tar czvf $(dist).tar.gz $(dist)/
