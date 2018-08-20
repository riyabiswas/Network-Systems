CC=gcc -w -s
CFLAGS=-std=c99 -w
HEADERS=$(wildcard *.h)

%.o: %.c $(HEADERS)
	@$(CC) $(CFLAGS) -c $< -o $@

LIBS= -L/usr/lib -lpthread -lm -lrt

all : server_http

server_http : server_http.o
	@$(CC) $(CFLAGS) server_http.o -o $@ $(LIBS)


	@echo -e "\e[01;34mBuild was successful.\e[00m"
	


clean:
	@rm -f *.o
	@rm -f server_http
	@echo -e "\e[01;33mObject files were removed.\e[00m"


