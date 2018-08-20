	#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <openssl/md5.h>
#include <ctype.h>


#define MAXBUFSIZE 1000
#define LS_BUFFER_SIZE 500
#define COMMAND_SIZE 20
#define FILE_NAME_SIZE 20
#define FILECHUNKSIZE 1000
#define SEND_BUFF_SIZE 1000
#define MAX_SEQ_NUM 10
#define ACK_SIZE 6

/* You will have to modify the program below */

typedef struct{
	char sendfile_buff[SEND_BUFF_SIZE];
	int seq_num;
}packets_send;

typedef struct{
	char buffer_put[MAXBUFSIZE];
	int seq_num;
}packets_recv;


int writeDataToClient(int sckt, const void *data, int datalen)
{
    const char *pdata = (const char*) data;
    int numSent=0;

    while (datalen > 0){
        if(datalen<1024){
            numSent = send(sckt, pdata, datalen, 0);
        }
        else{
            numSent = send(sckt, pdata, 1024, 0);
        }
        //printf("\nnumSent=%d    datalen=%d",numSent,datalen);
        if (numSent <= 0){
            if (numSent == 0){
                printf("The client was not written to: disconnected\n");
            } else {
                perror("The client was not written to");
            }
            return 0;
        }
        pdata += numSent;
        datalen -= numSent;
    }

    return (1);
}



void get_file(int sock_put[], struct sockaddr_in remote_put[], int remote_length_put){

		int nbytes;
		int fp;
		int packet_num = 0;
		//char buffer_put[MAXBUFSIZE+2]; 
		int last_seq;
		int len_buffer_put;

		
		packets_recv rec_packet;
		
		//char* ack_send;
		char* start_transfer;
		char user_input[FILE_NAME_SIZE];

		/*ack_send = (char *)malloc(ACK_SIZE);
		ack_send = "ACK";*/

		start_transfer = (char *)malloc(ACK_SIZE);
		start_transfer = "START";


		//rec_packet = (packets *)malloc(sizeof(packets));
		//rec_packet->seq_num = 0;


		/*current_seqS = (char *)malloc(MAX_SEQ_NUM);*/

		//FILE *fp;
		printf("\nEnter file name to be copied in:");
		gets(user_input);
		fp=open(user_input,O_RDWR |O_APPEND | O_CREAT,0666);
		if(fp == -1)
			printf("\nError opening file");

		sendto(sock_put[0], start_transfer, sizeof(start_transfer), 0, (struct sockaddr *) &remote_put, sizeof(remote_put));
		printf("\n before recv");
		

		bzero(&rec_packet,sizeof(rec_packet));
		//nbytes = recvfrom( sock_put, buffer_put, sizeof(buffer_put), 0, (struct sockaddr *)&remote_put, (socklen_t *) &remote_length_put);
		nbytes = recvfrom( sock_put[0], &rec_packet, sizeof(packets_recv), 0, (struct sockaddr *)&remote_put, (socklen_t *) &remote_length_put);
		printf("\n nbytes = %d", nbytes);

		last_seq = 10;
		
		sendto(sock_put[0], &rec_packet.seq_num, sizeof(rec_packet.seq_num), 0, (struct sockaddr *) &remote_put, sizeof(remote_put));
		printf("\nSending ACK%d",rec_packet.seq_num);

		while(nbytes)							//(struct sockaddr *) &from_addr, addr_length);//**** CALL RECVFROM() HERE ****;  
		{	

			++packet_num;
			printf("File receiving");//("Server says %s\n", buffer);

			if(last_seq != rec_packet.seq_num){

				printf("\nEnter if. last seq=%d rec seq=%d\n",last_seq, rec_packet.seq_num);

				for(int i=0; i<(nbytes-4); i++)
					rec_packet.buffer_put[i] = rec_packet.buffer_put[i] ^ 0xFF;

				if(write(fp,rec_packet.buffer_put,nbytes-4)<0)
				{
					printf("error writing file\n");
					break;
				}

			}
			

			/*len_buffer_put = strlen(buffer_put);
			last_seqS[0] = buffer_put[len_buffer_put-2];
			printf("\nlast_seqS=%c\n",last_seqS[0]);*/
			last_seq = rec_packet.seq_num;	
			bzero(&rec_packet,sizeof(rec_packet));
			//nbytes = recvfrom( sock_put, buffer_put, sizeof(buffer_put), 0, (struct sockaddr *) &remote_put, (socklen_t *) &remote_length_put);

			
			nbytes = recvfrom( sock_put[0], &rec_packet, sizeof(packets_recv), 0, (struct sockaddr *) &remote_put, (socklen_t *) &remote_length_put);	
			printf("\n nbytes = %d", nbytes);

			printf("\nEnter if. last seq=%d rec seq=%d\n",last_seq, rec_packet.seq_num);

			//printf("\nRECEIVING: %c",buffer_put[nbytes-2]);

			/*for(int i = strlen(buffer_put)-10; i<strlen(buffer_put);i++)
				current_seqS[i] = buffer_put[i];

			last_seq = current_seq;
			current_seq = atoi(current_seqS);
			printf("\nCurrent:%d", current_seq);
			printf("\nLast:%d", current_seq);*/

			sendto(sock_put[0], &rec_packet.seq_num, sizeof(rec_packet.seq_num), 0, (struct sockaddr *) &remote_put, sizeof(remote_put));
			printf("\nSending ACK%d",rec_packet.seq_num);
			

		}
			
		
		printf("Number of packets received = %d", packet_num);		
		close(fp);

		//free(ack_send);
		free(start_transfer);
		//free(rec_packet);
}






void send_file(int sock_send[], struct sockaddr_in remote_send[], char *file){
	FILE *fp;				//This is used to open file
	
	//char sendfile_buff[SEND_BUFF_SIZE+2];	   //buffer to send file content
	/*packets send_packet;*/
	
	long int read_bytes; 
	long int nbytes;

	char *file_content;
	char *file_part[4];
	
	struct timeval tv;
    long fsize;
    long fsize_div;
    long fsize_last;
	unsigned char md5_result[MD5_DIGEST_LENGTH];
	//unsigned long int last_16[3];
	//char *eptr;
	uint8_t md5_int;
	int server_sequence[8];


	struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);
	//int lostPacketCount = 0;

	packets_send send_packet;



	fp=fopen(file, "rb");
	if(!fp)
		printf("\nFile not found");




        if (fseek(fp, 0, SEEK_END) == -1){
            perror("The file was not seeked");
            //exit(1);
            return;
        }
        printf("\nFile seeked");

        fsize = ftell(fp);
        if (fsize == -1) {
            perror("The file size was not retrieved");
            //exit(1);
            return;
        }

        
        printf("\nGot file size");
        rewind(fp);


        file_content = (char *) malloc(fsize);
        //char *mesg = (char*) malloc(fsize);
        if (!file_content){
            perror("The file buffer was not allocated\n");
            //exit(1);
            return;
        }

        if (fread(file_content, fsize, 1, fp) != 1){
            perror("The file was not read\n");
            //exit(1);
            return;
        }

        rewind(fp);

		MD5(file_content, fsize, md5_result);
		
		md5_int= md5_result[15];  	
		printf("\n\rmd5_int=%d\n", md5_int);
		printf("MD5  = ");
		for (int i=0; i < MD5_DIGEST_LENGTH; i++)
		{
			printf("%02x",  md5_result[i]);
		}
		
		if(md5_int%4 == 0)
			{server_sequence[0]=0; server_sequence[1]=1; server_sequence[2]=1; server_sequence[3]=2; server_sequence[4]=2; server_sequence[5]=3; server_sequence[6]=3; server_sequence[7]=0; }
		else if(md5_int%4 == 1)
			{server_sequence[0]=3; server_sequence[1]=0; server_sequence[2]=0; server_sequence[3]=1; server_sequence[4]=1; server_sequence[5]=2; server_sequence[6]=2; server_sequence[7]=3; }
		else if(md5_int%4 == 2)
			{server_sequence[0]=2; server_sequence[1]=3; server_sequence[2]=3; server_sequence[3]=0; server_sequence[4]=0; server_sequence[5]=1; server_sequence[6]=1; server_sequence[7]=2; }
		else if(md5_int%4 == 3)
			{server_sequence[0]=1; server_sequence[1]=2; server_sequence[2]=2; server_sequence[3]=3; server_sequence[4]=3; server_sequence[5]=0; server_sequence[6]=0; server_sequence[7]=1; }

		
		printf("\narray intialised: ");
		for(int i=0; i<8; i++)
			printf("server_sequence[%d]:%d ", i, server_sequence[i]);
		

        fsize_div = fsize/4;
        fsize_last = fsize_div + fsize%4;

        printf("\n\rfsize_div=%ld, fsize_last=%ld", fsize_div, fsize_last);
    	printf("\n\rsizes obtained");


	bzero(&send_packet,sizeof(send_packet));

	
    long file_part_size[4];
    int j=0;

    for(int i=0; i<4; i++){

		if(i == 3)
			file_part_size[i] = fsize_last;
		else
			file_part_size[i] = fsize_div;

		file_part[i] = (char *) malloc(file_part_size[i]);
        //char *mesg = (char*) malloc(fsize);
        if (!file_part[i]){
            perror("The file buffer was not allocated\n");
            //exit(1);
            return;
        }


		if(fread(file_part[i], file_part_size[i], 1, fp) != 1){
            perror("The file was not read\n");
            //exit(1);
            return;
        }
    }


	for(int i=0; i<8; i++){

		if(i == 0)
			j = 0;
		if(i%2 == 0){
			j += 1;


			if (connect(sock_send[j], (struct sockaddr *)&remote_send[j], sizeof(remote_send[j]))<0){
				//fprintf(stderr, "Connection Failure\n");
				perror("connect");
				exit(1);
			}
		}


       /* char *part_temp = file_part[i];
        for(int j=0; j<file_part_size; j++){
					*file_part[i] = (*file_part[i]) ^ 0xFF;
					++file_part[i];
		}
		file_part[i] = part_temp;*/

        if (!writeDataToClient(sock_send[j], file_part[server_sequence[i]], file_part_size[server_sequence[j]])){
            close(sock_send[i]);
            //continue;
            return;
        }

        printf("The file part %d was sent successfully\n", i);
        //close(sock_send[i]);

	}

	fclose(fp);
	free(file_content);
	//free(send_packet);
}



int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sock[4];                               //this will be our socket
	char buffer[MAXBUFSIZE];
	char rec_buff[FILECHUNKSIZE];
	char user_input[COMMAND_SIZE];
	char ls_buffer[LS_BUFFER_SIZE];

	struct sockaddr_in remote[4];              //"Internet socket address structure"

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/

	for(int i =0; i<4; i++){
		bzero(&remote[i],sizeof(remote[i]));               //zero the struct
		remote[i].sin_family = AF_INET;                 //address family
		
		remote[i].sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
	}
	
	remote[0].sin_port = htons(10001);      //sets port to network byte order
	remote[1].sin_port = htons(10002);      //sets port to network byte order
	remote[2].sin_port = htons(10003);      //sets port to network byte order
	remote[3].sin_port = htons(10004);      //sets port to network byte order


	int fp;

	while(1){
			
		printf("\n*******************************************************************************************************");
		printf("\n\t\t\tUSER MENU");
		printf("\n*******************************************************************************************************");
		printf("\n1. get filename");
		printf("\n2. put filename");
		printf("\n3. deletefilename");
		printf("\n4. ls\n");
		printf("\n5. Exit\n");



		/******************
		  sendto() sends immediately.  
		  it will report an error if the message fails to leave the computer
		  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
		 ******************/
		//char user_input[] = "get foo1";	
		char *command;//[COMMANDSIZE];
		char *filename;//[nbytes];



		//Causes the system to create a generic socket of type UDP (datagram)
		for(int i=0; i<4; i++){
			if ((sock[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("unable to create socket");
			}
			printf("\nsocket created");

		}
		

		for(int i = 0; i<1; i++){
			if (connect(sock[i], (struct sockaddr *)&remote[i], sizeof(remote[i]))<0) {
				//fprintf(stderr, "Connection Failure\n");
				perror("connect");
				exit(1);
			}
		}

		




		printf("\nEnter command:");
		gets(user_input);
		//strcat(user_input, "\0");
		printf("\nuser_input is: %s",user_input);
		for(int i=0; i<1; i++)
			nbytes = send(sock[0], user_input, strlen(user_input), 0); //, (struct sockaddr *)&remote[i], sizeof(remote[i]));//**** CALL SENDTO() HERE ****;

		//Parsing user input
		command = strtok(user_input, " ");
		filename = strtok(NULL, " ");

		printf("command: %s\n",command);
		printf("filename: %s\n",filename);

	
		// Blocks till bytes are received
		struct sockaddr_in from_addr;
		int addr_length = sizeof(struct sockaddr);
		bzero(buffer,sizeof(buffer));





		///TO RECEIVE ORANGE FROM SERVER////
		/*nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&from_addr, (socklen_t *) &addr_length);//**** CALL RECVFROM() HERE ****;  

		printf("Server says %s\n", buffer);

		if(nbytes <= 0)
			printf("Not received anything nbytes= %d\n",nbytes);
		else
			printf("received %d %s\n", nbytes, buffer);*/


		if(strcmp(command, "get") == 0){


			get_file(sock,remote,sizeof(remote[0]));

			/*int packets = 0;
			//FILE *fp;
			fp=open("file_client_copy",O_RDWR |O_APPEND | O_CREAT,0666);
			if(fp == -1)

			printf("\nError opening file");

			nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
			++packets;

			while (nbytes)//(struct sockaddr *) &from_addr, addr_length);//**** CALL RECVFROM() HERE ****;  
			{	
				printf("File receiving");//("Server says %s\n", buffer);
				if(write(fp,buffer,nbytes)<0)
				{
					printf("error writing file\n");
					//exit(1);
				}
				nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
				++packets;
			}

			printf("Number of packets receieved= %d", packets);
			close(fp);*/
		}
		
		else if(strcmp(command, "put") == 0){

				nbytes = recv( sock[0], buffer, MAXBUFSIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
				if(strcmp(buffer,"START") == 0)
					send_file(sock, remote, filename);
		}
	
		else if(strcmp(command, "ls") == 0){
			printf("\nInside ls");
			bzero(ls_buffer,sizeof(ls_buffer));
			nbytes = recv( sock[0], ls_buffer, LS_BUFFER_SIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);	
			printf("\n%s",ls_buffer);
		
		}
	
		else if(strcmp(command, "exit") == 0)
			exit(1);
		else{
			printf("\nwrong command!");
		}
	

	}

	close(sock[0]);

	
}

