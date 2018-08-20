CC=gcc -w -s
CFLAGS=-std=c99 -w
HEADERS=$(wildcard *.h)

%.o: %.c $(HEADERS)
	@$(CC) $(CFLAGS) -c $< -o $@

all : udp_client

udp_client : udp_client.o
	@$(CC) $(CFLAGS) udp_client.o -o $@


	@echo -e "\e[01;34mBuild was successful.\e[00m"
	@make clean


clean:
	@rm -f *.o
	@echo -e "\e[01;33mObject files were removed.\e[00m"


