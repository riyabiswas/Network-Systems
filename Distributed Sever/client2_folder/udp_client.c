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
#define ACK_SIZE 10
#define MAXLINELENGTH 128
//#define recv_buffer_size 2000




/* You will have to modify the program below */

typedef struct{
	char sendfile_buff[SEND_BUFF_SIZE];
	int seq_num;
}packets_send;

typedef struct{
	char buffer_put[MAXBUFSIZE];
	int seq_num;
}packets_recv;

typedef struct{
	char * ls_filename;
	int parts[4];
	int server_part[4];
}ls_file;
ls_file files[500];
int file_count;

void get_ls(int sock_ls[],int unconnected[]){

	char *ls_buffer[4]; //[LS_BUFFER_SIZE];
	char *bufferTemp;
	char *fileTemp;
	char *fileTempPointer;
	int nbytes;
	//ls_file files[500];
	char *partTemp;
	file_count = 0;
	int fileExists = 0;
	int part_int;



	bufferTemp = (char *)malloc(FILE_NAME_SIZE);
	partTemp = (char *)malloc(2);
	fileTemp = (char *)malloc(FILE_NAME_SIZE);
	for(int i=0; i<4; i++)
		ls_buffer[i] = (char *)malloc(LS_BUFFER_SIZE);

	for(int i=0; i<4; i++)
		files[file_count].server_part[i] = -1;
	
	
	printf("\nInside ls");

	for(int i=0; i<4; i++){

		bzero(ls_buffer[i],sizeof(ls_buffer));

		if(unconnected[i] == 1)
			continue;
		nbytes = recv( sock_ls[i], ls_buffer[i], LS_BUFFER_SIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
		printf("\nnbytes = %d", nbytes);
		printf("\nls=%s\n", ls_buffer[i]);
		bufferTemp = strtok(ls_buffer[i], "\t");

		while(bufferTemp){

			printf("\nBufferTemp = %s\n", bufferTemp);

			/*if(strcmp(bufferTemp, ".") == 0 || strcmp(bufferTemp, "..")){
				bufferTemp = strtok(NULL, "\t");
				continue;
			}*/

			int dot_count = 0;
			fileTempPointer = fileTemp;
			//printf("\nassining fileTempPointer");

			while(dot_count<2){

				if(*bufferTemp == '.')
					dot_count++;
				if(dot_count == 2)
					break;
				*fileTemp = *bufferTemp;
				fileTemp++;
				bufferTemp++;

			}

			*fileTemp = '\0';
			*partTemp = *(++bufferTemp);
			strcat(partTemp, "\0");
			fileTemp = fileTempPointer;

			printf("\n\rFileTemp = %s\tPartname=%s", fileTemp, partTemp);

			if((file_count == 0) && (strcmp(fileTemp, ".") != 0)){

				files[file_count].ls_filename = (char *)malloc(FILE_NAME_SIZE);
				strcpy(files[file_count].ls_filename, fileTemp);

				part_int= atoi(partTemp);
				printf("\npart_int=%d\n", part_int);
				files[file_count].parts[part_int] = 1;
				files[file_count].server_part[part_int] = i;
				printf("\n\rfiles[%d].server_part[%d]=%d", file_count,part_int,files[file_count].server_part[part_int]);
				file_count++;
				for(int x=0; x<4; x++)
					files[file_count].server_part[x] = -1;

			}

			else{
				
				for(int j=0; j<file_count; j++){
				//if(file_count>0){
					if(strncmp(files[j].ls_filename, fileTemp, strlen(fileTemp)) == 0){
						part_int= atoi(partTemp);
						printf("\nFile exists: filename[%d]=%s, part_int=%d\n", j, files[j].ls_filename, part_int);
						files[j].parts[part_int] = 1;
						files[j].server_part[part_int] = i;
						printf("\n\rfiles[%d].server_part[%d]=%d",j,part_int,files[j].server_part[part_int]);
						fileExists = 1;
					}
				}
				
	

				if((fileExists==0) && (strcmp(fileTemp, ".") != 0)){

					//file_count++;
					files[file_count].ls_filename = (char *)malloc(FILE_NAME_SIZE);
					strcpy(files[file_count].ls_filename, fileTemp);
					int part_int= atoi(partTemp);
					printf("\nFile does not exist: filename[%d]=%s, part_int=%d\n", file_count, files[file_count].ls_filename, part_int);
					files[file_count].parts[part_int] = 1;
					files[file_count].server_part[part_int] = i;
					printf("\n\rfiles[%d].server_part[%d]=%d", file_count, part_int, files[file_count].server_part[part_int]);
					file_count++;
					for(int x=0; x<4; x++)
						files[file_count].server_part[x] = -1;
					
				}

				fileExists = 0;

			}
			
			bzero(fileTemp,FILE_NAME_SIZE);
			bzero(partTemp, 2);
			

			bufferTemp = strtok(NULL, "\t");
		}
		
	}

	printf("\n\rfile_count=%d",file_count);
	for(int i=0; i<=file_count; i++){
		printf("\n\rFilename[%d]:%s", i,files[i].ls_filename);
		printf("\n\rParts:%d  %d  %d  %d\n",files[i].parts[0], files[i].parts[1], files[i].parts[2], files[i].parts[3]);
	}


}

void print_ls(void){
	printf("\n\r");
	for(int i=0; i<file_count; i++){
		printf("\t%s", files[i].ls_filename);
		if((files[i].parts[0] != 1) || (files[i].parts[1] != 1) || (files[i].parts[2] != 1) || (files[i].parts[3] != 1))
			printf("(Incomplete)");
		//printf("\tServers: %d %d %d %d", files[i].server_part[0], files[i].server_part[1], files[i].server_part[2], files[i].server_part[3]);
	}
}



void send_credentials(int socket[], int unconnected[]){
	char * str;
    char * input_line;
    char * username;
    char * password;
    char * username_password;

    int nread;
    int nbytes;
    size_t len = 128;

    FILE *fp_conf = fopen("dfc.conf", "rb");

    str = (char *)malloc(MAXLINELENGTH);
    input_line = (char *)malloc(MAXLINELENGTH);
    username = (char *)malloc(COMMAND_SIZE);
    password = (char *)malloc(COMMAND_SIZE);

    if (!fp_conf){
            perror("The file was not opened");    
            //exit(1);    
            return;
    }

    while((nread = getline(&input_line, &len, fp_conf)) != -1){

    	str = strtok(input_line, " ");

        if(strcmp(str,"Username") == 0){

            str = strtok(NULL, " ");
            //username = strtok()
            strncpy(username, str, strlen(str));

            printf("\nUsername: %s\n", username);
        }

        else if(strcmp(str,"Password") == 0){

            str = strtok(NULL, " ");
            strncpy(password, str, strlen(str));
            printf("\nPassword: %s\n", password);
        }

        
    }

    for(int i=0; i<4; i++)
			printf("not_connected[%d]: %d", i, unconnected[i]);

 	strncat(username, " ", 1);
    strncat(username, password, strlen(password));

    printf("\nusername and password: %s", username);


    for(int i=0; i<4; i++){
    	if(unconnected[i] == 0)
			nbytes = send(socket[i], username, strlen(username), 0); //, (struct sockaddr *)&remote[i], sizeof(remote[i]));//**** CALL SENDTO() HERE ****;
    }

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



void get_file(int sock_put[], struct sockaddr_in remote_put[], int remote_length_put, char *fileRecv, int unconnected[]){

		int nbytes;
		int fp;
		int packet_num = 0;
		//char buffer_put[MAXBUFSIZE+2]; 
		int last_seq;
		int len_buffer_put;
		int fileNumber = -1;
		long file_part_size;
		//packets_recv rec_packet;
		
		//char* ack_send;
		char* start_transfer;
		char *file_info;
		char * recv_buffer;
		char partno_str[2];
		//char user_input[FILE_NAME_SIZE];
		int recv_buffer_size = 1024;

		file_info = (char *)malloc(FILE_NAME_SIZE);
		recv_buffer = (char *)malloc(recv_buffer_size);
		start_transfer = (char *)malloc(ACK_SIZE);


		//fp=open(fileRecv, O_RDWR |O_APPEND | O_CREAT,0666);
		fp=open(fileRecv, O_RDWR | O_CREAT,0666);
		if(fp == -1)
			printf("\nError opening file");


		for(int i=0; i<file_count; i++){
			if(strcmp(files[i].ls_filename, fileRecv) == 0)
				fileNumber = i;
		}
		

		if(fileNumber == -1){
			printf("\nFile does not exist");
			return;
		}
		// bzero(file_info,sizeof(file_info));

		// strcpy(file_info, files[fileNumber].ls_filename);
		if((files[fileNumber].parts[0] != 1) || (files[fileNumber].parts[1] != 1) || (files[fileNumber].parts[2] != 1) || (files[fileNumber].parts[3] != 1)){

			printf("(File incomplete)");
			return;

		}

		for(int partno=0; partno<4; partno++){

			bzero(file_info,FILE_NAME_SIZE);
			strcpy(file_info, files[fileNumber].ls_filename);
			strcat(file_info, ".");
			int length = snprintf( NULL, 0, "%d", partno);
			snprintf( partno_str, length + 1, "%d", partno);
			strcat(file_info, partno_str);
			strcat(file_info, "\0");
			printf("\nfile_info: %s", file_info);

			printf("\nConnecting to server %d", files[fileNumber].server_part[partno]);

			if (!writeStrToClient(sock_put[files[fileNumber].server_part[partno]], "SEND")){
	                    close(sock_put[files[fileNumber].server_part[partno]]);
	                    //continue;
	                    return;
	                }

	        bzero(recv_buffer, recv_buffer_size);
			nbytes = recv(sock_put[files[fileNumber].server_part[partno]], recv_buffer, FILE_NAME_SIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
			if(strcmp(recv_buffer,"PARTNO") == 0){

				send(sock_put[files[fileNumber].server_part[partno]], file_info, strlen(file_info), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
				// if (!writeDataToClient(sock_put[files[fileNumber].server_part[2]], file_info, strlen(file_info))){
			 //            close(sock_put[files[fileNumber].server_part[2]]);
			 //            //continue;
			 //            return;
			 //        }
		
			 }	

			bzero(start_transfer,ACK_SIZE);
			strcpy(start_transfer, "SIZE");

			send(sock_put[files[fileNumber].server_part[partno]], start_transfer, strlen(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
			printf("\n before receving filepartsize");

			bzero(recv_buffer,recv_buffer_size);
			nbytes = recv(sock_put[files[fileNumber].server_part[partno]], recv_buffer, FILE_NAME_SIZE, 0); //, (struct sockaddr *) &remote, (socklen_t *)&remote_length);//**** CALL RECVFROM() HERE ****;
			file_part_size = atoi(recv_buffer);
			printf("\n\rpartsize=%ld",file_part_size);

			bzero(start_transfer, ACK_SIZE);
			strcpy(start_transfer,"START");


			//printf("\nGet file part 0");
			//gets(user_input);
			// fp=open(user_input,O_RDWR |O_APPEND | O_CREAT,0666);
			// if(fp == -1)
			// 	printf("\nError opening file");

			send(sock_put[files[fileNumber].server_part[partno]], start_transfer, strlen(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
			printf("\n sent START");



			int loop_count = 0;
			if((file_part_size % 1024) == 0)
				loop_count = file_part_size/1024;
			else 
				loop_count = (file_part_size/1024) + 1;

			int totalBytesReceived = 0;

			for(int i=0; i<loop_count; i++)							//(struct sockaddr *) &from_addr, addr_length);//**** CALL RECVFROM() HERE ****;  
			{	

				printf("File receiving");
				bzero(recv_buffer,1024);

				if(i == loop_count-1)
					recv_buffer_size = file_part_size%1024;
				else
					recv_buffer_size = 1024;
				
				printf("\n\rSize to be received: %d", recv_buffer_size);
				nbytes = recv(sock_put[files[fileNumber].server_part[partno]], recv_buffer, recv_buffer_size, 0);//, (struct sockaddr *) &remote_put, (socklen_t *) &remote_length_put);	
				//printf("\n nbytes = %d", nbytes);

		       /* for(int j=0; j<bufsize_recv; j++){
					*buff_recv= *buff_recv ^ 0xFF;
					++buff_recv;
		        }*/
				
				if(write(fp,recv_buffer,nbytes)<0)
				{
					printf("error writing file\n");
					break;
				}
				totalBytesReceived += nbytes;
				printf("\ntotalBytesReceived=%d", totalBytesReceived);		
				
			}


		}
		
		

		for(int j=0; j<4; j++){
			bzero(start_transfer, ACK_SIZE);
			strcpy(start_transfer,"END");
			if(unconnected[j] != 1){
				send(sock_put[j], start_transfer, strlen(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
				printf("\n sent END");
			}
			
		}
		
		close(fp);
		// bzero(start_transfer, ACK_SIZE);
		// strcpy(start_transfer,"FIN");

		// send(sock_put[files[fileNumber].server_part[2]], start_transfer, strlen(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
		// printf("\n sent FIN");

		// //int j;
		// for(int j=0; j<4; j++){

		// 	bzero(start_transfer, ACK_SIZE);
		// 	strcpy(start_transfer,"END");
		// 	if(unconnected[j] != 1){
		// 		send(sock_put[j], start_transfer, strlen(start_transfer), 0);//, (struct sockaddr *) &remote_put, sizeof(remote_put));
		// 		printf("\n sent END");

		// 	}
			
		// }

		// close(fp);

}






void send_file(int sock_send[], struct sockaddr_in remote_send[], char *file){
	FILE *fp;				//This is used to open file

	
	long int read_bytes; 
	long int nbytes;

	char *file_content;
	char *file_part[4];
	char send_partname[6];
	char send_size[10];
	char partno[2];
	
	struct timeval tv;
    long fsize;
    long fsize_div;
    long fsize_last;
    long file_part_size[4];
	unsigned char md5_result[MD5_DIGEST_LENGTH];
	//unsigned long int last_16[3];
	//char *eptr;
	uint8_t md5_int;
	int server_sequence[8];



	struct sockaddr_in from_addr;
	int addr_length = sizeof(struct sockaddr);

	strcpy(send_partname, "part");

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
		else if(i%2 == 0){
			
			j += 1;


		}
		
			 /* char *part_temp = file_part[i];
        for(int j=0; j<file_part_size; j++){
					*file_part[i] = (*file_part[i]) ^ 0xFF;
					++file_part[i];
		}
		file_part[i] = part_temp;*/
		char buffer[MAXBUFSIZE];
		bzero(buffer,sizeof(buffer));


		bzero(&send_partname,6);
		bzero(&partno,2);
		strcpy(send_partname, "part");
		int length = snprintf( NULL, 0, "%d", server_sequence[i]);
		snprintf( partno, length + 1, "%d", server_sequence[i]);
		strcat(send_partname, partno);
		printf("\nSend_partname = %s", send_partname);
		
		bzero(buffer,sizeof(buffer));
		nbytes = recv(sock_send[j], buffer, MAXBUFSIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
		if(strcmp(buffer,"PARTNO") == 0){
			if (!writeDataToClient(sock_send[j], partno, sizeof(partno))){
		            close(sock_send[j]);
		            //continue;
		            return;
		        }
	
		 }	


		bzero(buffer,sizeof(buffer));
		nbytes = recv(sock_send[j], buffer, MAXBUFSIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
		if(strcmp(buffer,"SIZE") == 0){
			int length = snprintf( NULL, 0, "%ld", file_part_size[server_sequence[i]]);
			snprintf( send_size, length + 1, "%ld", file_part_size[server_sequence[i]]);
			if (!writeDataToClient(sock_send[j], send_size, length+1)){
		            close(sock_send[j]);
		            //continue;
		            return;
		        }
	
		 }


		bzero(buffer,sizeof(buffer));
		nbytes = recv(sock_send[j], buffer, MAXBUFSIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
		if(strcmp(buffer,"START") == 0){

	        if (!writeDataToClient(sock_send[j], file_part[server_sequence[i]], file_part_size[server_sequence[i]])){
	            close(sock_send[j]);
	            //continue;
	            return;
	        }




        	printf("The file part %d was sent successfully\n", server_sequence[i]);
		}

       
        //close(sock_send[i]);
 

	}

	fclose(fp);
	free(file_content);
	//free(send_packet);
}



int main (void)//int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sock[4];                               //this will be our socket
	char buffer[MAXBUFSIZE];
	char rec_buff[FILECHUNKSIZE];
	char user_input[COMMAND_SIZE];
	
	int not_connected[4];

	struct sockaddr_in remote[4];              //"Internet socket address structure"

	// if (argc < 3)
	// {
	// 	printf("USAGE:  <server_ip> <server_port>\n");
	// 	exit(1);
	// }

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/

	for(int i =0; i<4; i++){
		bzero(&remote[i],sizeof(remote[i]));               //zero the struct
		remote[i].sin_family = AF_INET;                 //address family
		
		remote[i].sin_addr.s_addr = inet_addr("127.0.0.1");//argv[1]); //sets remote IP address
	}
	
	remote[0].sin_port = htons(10001);      //sets port to network byte order
	remote[1].sin_port = htons(10002);      //sets port to network byte order
	remote[2].sin_port = htons(10003);      //sets port to network byte order
	remote[3].sin_port = htons(10004);      //sets port to network byte order

	for(int i=0; i<4; i++)
		not_connected[i] = 0;

	int fp;

	// while(1){
			
	// 	printf("\n*******************************************************************************************************");
	// 	printf("\n\t\t\tUSER MENU");
	// 	printf("\n*******************************************************************************************************");
	// 	printf("\n1. get filename");
	// 	printf("\n2. put filename");
	// 	printf("\n3. ls\n");
	// 	printf("\n4. Exit\n");



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
		

		for(int i = 0; i<4; i++){
			if (connect(sock[i], (struct sockaddr *)&remote[i], sizeof(remote[i]))<0) {
				//fprintf(stderr, "Connection Failure\n");
				printf("Can't connect to server %d", i);
				perror("connect");
				not_connected[i]=1;
				//exit(1);
			}
		}

		printf("\n");

		for(int i=0; i<4; i++)
			printf("not_connected[%d]: %d", i, not_connected[i]);


		send_credentials(sock, not_connected);


		while(1){
			
			printf("\n*******************************************************************************************************");
			printf("\n\t\t\tUSER MENU");
			printf("\n*******************************************************************************************************");
			printf("\n1. GET filename");
			printf("\n2. PUT filename");
			printf("\n3. LIST");
			printf("\n4. MKDIR subfolder");
			printf("\n5. Exit\n");


			printf("\nEnter command:");
			gets(user_input);
			//strcat(user_input, "\0");
			printf("\nuser_input is: %s",user_input);
			for(int i=0; i<4; i++){
			
				bzero(buffer,sizeof(buffer));
				if(not_connected[i] == 1)
					continue;
				nbytes = recv(sock[i], buffer, MAXBUFSIZE, 0); //, (struct sockaddr *)&from_addr,(socklen_t *) &addr_length);
				if(strcmp(buffer,"CMND") == 0){

					nbytes = send(sock[i], user_input, strlen(user_input), 0); //, (struct sockaddr *)&remote[i], sizeof(remote[i]));//**** CALL SENDTO() HERE ****;
				}
			}

			//Parsing user input
			command = strtok(user_input, " ");
			filename = strtok(NULL, " ");

			printf("command: %s\n",command);
			printf("filename: %s\n",filename);

		
			// Blocks till bytes are received
			struct sockaddr_in from_addr;
			int addr_length = sizeof(struct sockaddr);
			bzero(buffer,sizeof(buffer));




			if(strcmp(command, "GET") == 0){

				get_ls(sock, not_connected);
				printf("\nCalling get_file()");
				
				get_file(sock,remote,sizeof(remote[0]), filename,  not_connected);

			}
			
			else if(strcmp(command, "PUT") == 0){
						for(int i =0; i<4; i++){
							if(not_connected[i] == 1){
								printf("\n\rCannot connect to all servers");
								exit(1);
							}
						}

						send_file(sock, remote, filename);
			}
		
			else if(strcmp(command, "LIST") == 0){
				get_ls(sock, not_connected);
				print_ls();
				
			}

			else if(strcmp(command, "MKDIR") == 0){
				printf("\nMaking folder %s", filename);
			}
		
			else if(strcmp(command, "Exit") == 0)
				exit(1);
			else{
				printf("\nwrong command!");
			}
		

	}

	for(int i=0; i<4; i++)
		close(sock[i]);

	
}

