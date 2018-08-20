CC=gcc -w -s
CFLAGS=-std=c99 -w
HEADERS=$(wildcard *.h)

%.o: %.c $(HEADERS)
	@$(CC) $(CFLAGS) -c $< -o $@

all : server_withfunc

server_withfunc : server_withfunc.o
	@$(CC) $(CFLAGS) server_withfunc.o -o $@


	@echo -e "\e[01;34mBuild was successful.\e[00m"
	@make clean


clean:
	@rm -f *.o
	@echo -e "\e[01;33mObject files were removed.\e[00m"


