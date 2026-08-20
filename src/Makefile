CC      = gcc

OPTS    = -I. -I/usr/include -DWITH_OPENSSL -DCPLOG -Wall -std=gnu11 -g3 -O0 -DLIBRESSL_HAS_TLS1_3 -DWITH_NO_C_LOCALE #-DCPTEST

OBJS    = config.o \
          logging.o \
	  cups-pdf.o \
	  webservice/soapClient.o \
	  webservice/soapC.o \
	  read-file.o \
	  stdsoap2.o \
	  stub.o

OBJS2   = logging3.o \
	  pappl-main.o \
	  webservice/soapClient.o \
	  webservice/soapC.o \
	  read-file.o \
	  stdsoap2.o \
	  stub3.o

LIBS    = -lcups -lgsoap -lssl -lcrypto $(pkg-config --cflags --libs gsoapssl)

all: cups-fax

$(OBJS): %.o: %.c
	$(CC) -c -o $@ $(OPTS) $<

cups-fax: $(OBJS)
	$(CC) -s -o cups-fax $(OBJS) $(OPTS) $(LIBS)

stub:
	cp ./gsoap-stdsoap2/208135/stdsoap2.c stdsoap2.c
	wsdl2h -c -o webservice/ws-cups-fax.h https://www.gtc-online.de/ws-cups-fax/WS?wsdl
	soapcpp2 -CL webservice/ws-cups-fax.h -I/usr/share/gsoap/import/ -dwebservice/

TARGET = cups3-fax
CFLAGS = -Wall -std=gnu11 -g3 -O0 $(shell pkg-config --cflags pappl) -DCPV3 -DCPTEST
MLIBS   = -lgsoap -lssl -lcrypto $(pkg-config --cflags --libs gsoapssl) $(shell pkg-config --libs pappl)

SRC    = pappl-main.c
OBJ    = $(SRC:.c=.o)

$(TARGET): $(OBJS2)
	$(CC) $(CFLAGS) $(OPTS) -o $(TARGET) $(OBJS2) $(MLIBS)

stub3.o: stub.c
	$(CC) $(CFLAGS) -c stub.c -o stub3.o

logging3.o: logging.c
	$(CC) $(CFLAGS) -c logging.c -o logging3.o

pappl-main.o: pappl-main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f cups-fax .o $(OBJS) $(OBJS2) $(TARGET)

.PHONY: all clean

