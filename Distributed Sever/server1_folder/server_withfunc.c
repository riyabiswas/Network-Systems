#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>
 
/* You will have to modify the program below */

#define MAXBUFSIZE 1000
#define FILECHUNKSIZE 1000
#define SEND_BUFF_SIZE 1000
#define COMMANDSIZE 8
#define MAX_SEQ_NUM 11
#define FILE_NAME_SIZE 100
#define ACK_SIZE 7
#define MAXLINELENGTH 128


typedef struct{
	char buffer_put[MAXBUFSIZE];
	int seq_num;
}packets_recv;

typedef struct{
	char sendfile_buff[MAXBUFSIZE];
	int seq_num;
}packets_send;


typedef struct{
    int new_socket;  
    
    //int arg2;
}arg_struct;



int check_config(char *creds){

	char * str;
    char * input_line;

	int nread;
    size_t len = 128;

    FILE *fp_conf = fopen("dfs.conf", "rb");
    str = (char *)malloc(MAXLINELENGTH);
    input_line = (char *)malloc(MAXLINELENGTH);

    if (!fp_conf){
            perror("The file was not opened");    
            //exit(1);    
            return (-1);
    }

    while((nread = getline(&input_line, &len, fp_conf)) != -1){

    	str = strtok(input_line, "\n");
    	printf("\n\rdfs.cnf:%s",str);
    	printf("\n\rcreds:%s",creds);

        if(strcmp(str, creds) == 0)
        	return 1;
    }

    return (-1);

}



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



int writeStrToClient(int sckt, const char *str)
{
    return writeDataToClient(sckt, str, strlen(str));
}



void put_file(int sock_put, char *input_file){

		int nbytes;
		int fp;
		int packet_num = 0;
		//char buffer_put[MAXBUFSIZE+2]; 
		int last_seq;
		int len_buffer_put;

		char *buff_recv;
	    int bufsize_recv;  
	    long int file_part_size;
	    char * file_part_name;

		//packets_recv rec_packet;
		
		char* ack_send;
		char* start_transfer;
		char user_input[FILE_NAME_SIZE];

		ack_send = (char *)malloc(ACK_SIZE);
		ack_send = "ACK";

		start_transfer = (char *)malloc(ACK_SIZE);
		buff_recv = (char *)malloc(bufsize_recv);

		//bzero(start_transfer,ACK_SIZE);
		start_transfer = "PARTNO";

		send(sock_put, start_transfer, sizeof(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
		printf("\n before receving filepartno");


		nbytes = recv(sock_put, user_input, 2, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);//**** CALL RECVFROM() HERE ****;
		

		file_part_name = (char *)malloc(20);
		strcpy(file_part_name, input_file);
		strcat(file_part_name, ".");
		strcat(file_part_name, user_input);
		printf("filename: %s\n",file_part_name);
		
		fp=open(file_part_name,O_RDWR | O_CREAT,0666);
		if(fp == -1)
			printf("\nError opening file");


		bzero(&start_transfer,ACK_SIZE);
		start_transfer = "SIZE";

		send(sock_put, start_transfer, sizeof(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
		printf("\n before receving filepartsize");

		bzero(&user_input,FILE_NAME_SIZE);
		nbytes = recv(sock_put, user_input, FILE_NAME_SIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);//**** CALL RECVFROM() HERE ****;
		file_part_size = atoi(user_input);
		printf("\n\rpartsize=%ld",file_part_size);



		bzero(&start_transfer,ACK_SIZE);
		start_transfer = "START";

		send(sock_put, start_transfer, sizeof(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
		printf("\n before recv");
		
		int loop_count = 0;
		if((file_part_size % 1024) == 0)
			loop_count = file_part_size/1024;
		else 
			loop_count = (file_part_size/1024) + 1;

		int totalBytesReceived = 0;

		for(int i=0; i<loop_count; i++)							//(struct sockaddr *) &from_addr, addr_length);//**** CALL RECVFROM() HERE ****;  
		{	

			printf("File receiving");
			bzero(buff_recv,bufsize_recv);

			if(i == loop_count-1)
				bufsize_recv = file_part_size%1024;
			else
				bufsize_recv = 1024;
			
			printf("\n\rSize to be received: %d", bufsize_recv);
			nbytes = recv(sock_put, buff_recv, bufsize_recv, 0);//, (struct sockaddr *) &remote_put, (socklen_t *) &remote_length_put);	
			//printf("\n nbytes = %d", nbytes);

	       /* for(int j=0; j<bufsize_recv; j++){
				*buff_recv= *buff_recv ^ 0xFF;
				++buff_recv;
	        }*/
			
			if(write(fp,buff_recv,nbytes)<0)
			{
				printf("error writing file\n");
				break;
			}
			totalBytesReceived += nbytes;
			printf("\ntotalBytesReceived=%d", totalBytesReceived);		
			
		}
		
		
		
		printf("Number of packets received = %d", packet_num);		

		close(fp);

		
		//free(start_transfer);
		//free(rec_packet);
}



void send_file(int sock_send){
	FILE *fp;				//This is used to open file
	
	//char sendfile_buff[SEND_BUFF_SIZE+2];	   //buffer to send file content
	/*packets send_packet;*/
	
	long int read_bytes; 
	long int nbytes;
	int sending_seq;
	int packet_num = 0;
	//int ack_sequence;
	char packet_numS[2];
	char send_size[10];
	//char *END_FLAG = "================END";
	struct timeval tv;

	struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);
	//int lostPacketCount = 0;

	packets_send send_packet;
	char * fileInfo;
	char* recvBuf;
	char* filePart;
	int fsize;
	char * file_content;

	int recvBufSize = 1024;

	//ack_sequence = 10;

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if (setsockopt(sock_send, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval)) < 0) 
	    printf("\nError setting timeout %d",errno);


	//send_packet=(packets *) malloc(sizeof(packets));
	//send_packet.seq_num = 0;
	//sending_seq	= 0;



	fileInfo = (char *)malloc(FILE_NAME_SIZE);
	recvBuf = (char *)malloc(recvBufSize);
	filePart = (char *)malloc(FILE_NAME_SIZE);



	bzero(recvBuf,recvBufSize);
	// nbytes = recv(sock_send, recvBuf, MAXBUFSIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);
	// while(strcmp(recvBuf,"SEND") != 0);

	bzero(fileInfo, FILE_NAME_SIZE);
	strcpy(fileInfo,"PARTNO");
	send(sock_send, fileInfo, strlen(fileInfo), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));

	bzero(recvBuf,recvBufSize);
	nbytes = recv(sock_send, recvBuf, MAXBUFSIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);
	printf("\nnbytes=%ld  recvBuf=%s", nbytes, recvBuf);
	bzero(filePart,FILE_NAME_SIZE);
	strncpy(filePart, recvBuf, strlen(recvBuf));
	printf("\nFilename:%s\n", filePart);



	fp=fopen(filePart, "rb");
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


    bzero(recvBuf,recvBufSize);
	nbytes = recv(sock_send, recvBuf, MAXBUFSIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
	if(strcmp(recvBuf,"SIZE") == 0){

		int length = snprintf( NULL, 0, "%d", fsize);
		snprintf( send_size, length + 1, "%d", fsize);
		if (!writeDataToClient(sock_send, send_size, length+1)){
	            close(sock_send);
	            //continue;
	            return;
	        }

	}

	bzero(recvBuf,recvBufSize);
	nbytes = recv(sock_send, recvBuf, MAXBUFSIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);
	if(strcmp(recvBuf,"START") == 0){
		//send_file(new_socket, address, filename_temp);



		if (!writeDataToClient(sock_send, file_content, fsize)){
	            close(sock_send);
	            //continue;
	            return;
	        }




	    	printf("The file part was sent successfully\n");
	}


	fclose(fp);
	//free(send_packet);
}



void send_ls(int sock_send){
	
	DIR *d;
	struct dirent *dir;
	
	char *list_buffer;
	char *sending_buffer;
	long int buffer_size;
	
	sending_buffer = (char *) malloc(2048);
	
	d = opendir(".");
	if (d)
	{
	while ((dir = readdir(d)) != NULL)
	{
		list_buffer = dir->d_name;;
		//printf("%s\n",  list_buffer);
		//sending_buffer = (char *) realloc(sending_buffer, strlen(list_buffer)+strlen(sending_buffer));	
		strcat(sending_buffer, "\t");
		strcat(sending_buffer, list_buffer);
		//*list_buffer++
		//printf("%s\n", dir->d_name);
	}

	printf("%s\n", sending_buffer);

	buffer_size = strlen(sending_buffer);


	send(sock_send, sending_buffer, buffer_size, 0); //, (struct sockaddr *) &remote_send, sizeof(remote_send));

	closedir(d);
	}
}

void *connection_handler(void *arg_recvd){


	int nbytes;                        //number of bytes we receive in our message
	
	char buffer[MAXBUFSIZE];             //a buffer to store our received message
	char credentials[2*FILE_NAME_SIZE];             //a buffer to store our received message
	char * info;
	char * client_username;
	char * folder_name;
	//char * client_password;

	char *command;//[COMMANDSIZE];
	char *filename;//[nbytes];


	arg_struct arguments = *(arg_struct *)arg_recvd;
	int client_socket = arguments.new_socket;

	//get client credentials
	bzero(&credentials,sizeof(credentials));
	nbytes = recv(client_socket, credentials, 2*FILE_NAME_SIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);//**** CALL RECVFROM() HERE ****;
	printf("The client says:%s\n", credentials);

	if(check_config(credentials) < 1){
		printf("\n\rInvalid user");
		exit(1);
	}


	printf("\n\rUser valid");
	//open folder with client username
	client_username = (char *)malloc(FILE_NAME_SIZE);
	folder_name = (char *)malloc(FILE_NAME_SIZE);
	client_username = strtok(credentials, " ");
	//strcpy(folder_name, "/Documents/netsys/PA3/udp_sept24/server1_folder/");
	strcpy(folder_name, client_username);
	//printf("\nFolder name: %s", folder_name);

	if (unshare(CLONE_FS) == -1)
       perror("unshare");

    struct stat sb;

    if (stat(folder_name, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        printf("Folder found\n");
    }
    else
    {
        printf("Folder not found\n");
        if (mkdir(folder_name,0777) == -1) {
        	perror("\nDirectory not found");
        	exit(EXIT_FAILURE);
    	}
	}
	if (chdir(folder_name) != 0)
	perror("chdir() to username_folder failed");


	//request command
	info = (char *)malloc(ACK_SIZE);
	bzero(info,ACK_SIZE);
	info = "CMND";
	send(client_socket, info, sizeof(info), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));

	
 				
	//waits for command
	bzero(buffer,sizeof(buffer));
	nbytes = recv(client_socket, buffer, MAXBUFSIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);//**** CALL RECVFROM() HERE ****;
	
	printf("The client says %s\n", buffer);
	
	//Parsing user input

	command = (char *)malloc(COMMANDSIZE);
	filename = (char *)malloc(FILE_NAME_SIZE);


	command = strtok(buffer, " ");
	filename = strtok(NULL, " ");
	
	printf("command: %s\n",command);
	printf("filename: %s\n",filename);

	char filename_temp[FILE_NAME_SIZE];
	if(filename!=NULL){
	for(int i=0;i<strlen(filename);i++){
		filename_temp[i]=filename[i];
	}
	filename_temp[strlen(filename)]='\0';	
	}



	//Send file to client
	if(strcmp(command, "GET") == 0){

		send_ls(client_socket);
		// printf("\nCalling send_file");
		// send_file(new_socket, remote);
		bzero(buffer,MAXBUFSIZE);
		while(nbytes = recv(client_socket, buffer, MAXBUFSIZE, 0)){

		// 	bzero(buffer,sizeof(buffer));
		// 	nbytes = recv(new_socket, buffer, MAXBUFSIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);//**** CALL RECVFROM() HERE ****;
			if(strcmp(buffer, "END") == 0)
				break;
			else if(strcmp(buffer, "SEND") == 0){
				printf("\nCalling send_file");
				send_file(client_socket);
			}

			bzero(buffer,sizeof(buffer));
		 }
		
	}
	
	else if(strcmp(command, "PUT") == 0){
		put_file(client_socket, filename_temp);
		put_file(client_socket, filename_temp);

	}
	
	else if(strcmp(command, "delete") == 0){
		if (remove(filename) == 0)
		      printf("Deleted successfully");
	   	else
      			printf("Unable to delete the file");
		 
	}

	else if(strcmp(command, "LIST") == 0){
		send_ls(client_socket);
	}

	else if(strcmp(command, "MKDIR") == 0){
		struct stat sb2;

	    if (stat(filename_temp, &sb2) == 0 && S_ISDIR(sb2.st_mode))
	    {
	        printf("Folder found\n");
	    }
	    else
	    {
	        printf("Folder not found\n");
	        if (mkdir(filename_temp,0777) == -1) {
	        	perror("\nDirectory not found");
	        	exit(EXIT_FAILURE);
	    	}
		}
		if (chdir(filename_temp) != 0)
		perror("chdir() to username_folder failed");

	}


}


int main (int argc, char * argv[] )
{


	int sock, new_socket;                           //This will be our socket
	arg_struct * args;
					    
	struct sockaddr_in address, remote;     //"Internet socket address structure"
	unsigned int remote_length,address_length;         //length of the sockaddr_in structure
	

	

	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with	
	  the information about our socket
	 ******************/
	bzero(&address,sizeof(address));                    //zero the struct
	address.sin_family = AF_INET;                   //address family
	address.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine
	address.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order


	struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);

	args = (arg_struct *)malloc(sizeof(arg_struct));


	//Causes the system to create a generic socket of type UDP (datagram)
	printf("Socket start\n");
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("unable to create socket");
	}


	printf("Socket created\n");

	/******************
	  Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *) &address, sizeof(address)) == -1){    
        perror("The socket was not bound");    
        exit(1);    
    }

    printf("The socket is bound\n");    


    if (listen(sock, 10) == -1){
        perror("The socket was not opened for listening");    
        exit(1);    
    }    

    printf("The socket is listening\n");	
	address_length = sizeof(address);

	while(args->new_socket = accept(sock, (struct sockaddr *) &address, (socklen_t*)&address_length)){

		if (args->new_socket == -1) {    
	        perror("A client was not accepted");    
	        exit(1);    
	    }    

	    printf("A client is connected from %s:%hu...\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

	    pthread_t sniffer_thread;
	    //args->new_sock = (int *)malloc(1);
	    //args->new_sock = new_socket;
	    //args->file_size = fsize;
	     
	    if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) args) < 0)
	    {
	        perror("could not create thread");
	        return 1;
	    }
	}//;
	

	/*char msg[] = "orange";*/
	/*nbytes = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &remote, sizeof(remote));*/ //**** CALL SENDTO() HERE ****;
	// free(command);
	// if(filename!=NULL)
	// free(filename);
	close(sock);
	
}

