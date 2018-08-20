#include <netinet/in.h>    
#include <stdio.h>    
#include <string.h>
#include <stdlib.h>    
#include <sys/socket.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h>   
#include <arpa/inet.h> //inet_addr 
#include <pthread.h> //for threading , link with lpthread
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#define MAXBUFSIZE 1000
#define FILECHUNKSIZE 1000
#define SEND_BUFF_SIZE 1000
#define COMMANDSIZE 10
#define MAX_SEQ_NUM 11
#define MAXLINELENGTH 128
#define FILE_NAME_SIZE 20
#define CONTENT_TYPE 30
#define ACK_SIZE 6

int SEG_FLAG;
char PORT_STR[6];
static uint16_t PORT_NO;
int create_socket;

char ROOT_DIR[50];
char DEFAULT_FILE_1[10];
char DEFAULT_FILE_2[10];
char DEFAULT_FILE_3[10];

char HTML_TYPE[10];
char HTM_TYPE[10];
char TXT_TYPE[10];
char PNG_TYPE[10];
char GIF_TYPE[10];
char JPG_TYPE[10];
char CSS_TYPE[10];
char JS_TYPE[10];
char ICO_TYPE[10];
time_t KEEP_ALIVE;


typedef struct{
    int new_sock;  
    char *message;
    long file_size;
    in_port_t client_port;
    //int arg2;
}arg_struct;



void exit_function(int sig){

    int option = 1;
    close(create_socket);
    setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    printf("\n\rEXIT FUNCTION!");
    exit(1);
}


/*void segFault_function(int sig){
    SEG_FLAG = 1;   
}*/


void parse_config(void){

    char * str;
    char * input_line;
    int nread;
    size_t len = 128;
    FILE *fp_conf = fopen("ws.conf", "rb");
    str = (char *)malloc(MAXLINELENGTH);
    input_line = (char *)malloc(MAXLINELENGTH);
    if (!fp_conf){
            perror("The file was not opened");    
            //exit(1);    
            return;
        }
    //while (fgets(input, MAXLINELENGTH, fp_conf)){
        while((nread = getline(&input_line, &len, fp_conf)) != -1){
            if (*input_line == '#')
                continue;
            else{
                    str = strtok(input_line, " ");

                    if(strcmp(str,"Listen") == 0){

                        str = strtok(NULL, " ");
                        memcpy(PORT_STR, str, strlen(str)+1);
                        PORT_NO = atoi(PORT_STR);
                        printf("\n%d\n", PORT_NO);
                    }

                    else if(strcmp(str, "DocumentRoot") == 0){
                        str = strtok(NULL, " ");
                        memcpy(ROOT_DIR, str, strlen(str));
                        ROOT_DIR[0] = ROOT_DIR[1];
                        //printf("STRLEN OF ROOM_DIR = %d", strlen(ROOT_DIR));
                        ROOT_DIR[strlen(ROOT_DIR) - 2] = '\0';
                        printf("\n%s\n", ROOT_DIR);
                    }
                    

                   else if(strcmp(str, "DirectoryIndex") == 0){
                        str = strtok(NULL, " ");
                        memcpy(DEFAULT_FILE_1, str, strlen(str));
                        printf("\nDefault file 1: %s\n", DEFAULT_FILE_1);

                        str = strtok(NULL, " ");
                        memcpy(DEFAULT_FILE_2, str, strlen(str));
                        printf("\nDefault file 2: %s\n", DEFAULT_FILE_2);

                        str = strtok(NULL, " ");
                        memcpy(DEFAULT_FILE_3, str, strlen(str));
                        printf("\nDefault file 3: %s\n", DEFAULT_FILE_3);
                   }


                   else if(strcmp(str,".html") == 0){
                        str = strtok(NULL, " ");
                        memcpy(HTML_TYPE, str, strlen(str));
                        printf("\nHTML_TYPE: %s\n", HTML_TYPE);
                   }



                   else if(strcmp(str,".htm") == 0){
                        str = strtok(NULL, " ");
                        memcpy(HTM_TYPE, str, strlen(str));
                        printf("\nHTM_TYPE: %s\n", HTM_TYPE);
                   }



                   else if(strcmp(str,".txt") == 0){
                        str = strtok(NULL, " ");
                        memcpy(TXT_TYPE, str, strlen(str));
                        printf("\nTXT_TYPE: %s\n", TXT_TYPE);
                   }



                   else if(strcmp(str,".jpg") == 0){
                        str = strtok(NULL, " ");
                        memcpy(JPG_TYPE, str, strlen(str));
                        printf("\nJPG_TYPE: %s\n", JPG_TYPE);
                   }



                   else if(strcmp(str,".png") == 0){
                        str = strtok(NULL, " ");
                        memcpy(PNG_TYPE, str, strlen(str));
                        printf("\nPNG_TYPE: %s\n", PNG_TYPE);
                   }



                   else if(strcmp(str,".gif") == 0){
                        str = strtok(NULL, " ");
                        memcpy(GIF_TYPE, str, strlen(str));
                        printf("\nGIF_TYPE: %s\n", GIF_TYPE);
                   }


                   else if(strcmp(str,".css") == 0){
                        str = strtok(NULL, " ");
                        memcpy(CSS_TYPE, str, strlen(str));
                        printf("CSS_TYPE: %s\n", CSS_TYPE);
                   }


                   else if(strcmp(str,".js") == 0){
                        str = strtok(NULL, " ");
                        memcpy(JS_TYPE, str, strlen(str));
                        printf("\nJS_TYPE: %s\n", JS_TYPE);
                   }




                   else if(strcmp(str,".ico") == 0){
                        str = strtok(NULL, " ");
                        memcpy(ICO_TYPE, str, strlen(str));
                        printf("\nICO_TYPE: %s\n", ICO_TYPE);
                   }

                   else if (strcmp, "keep-alive" == 0){
                        KEEP_ALIVE = 10;
                   }



                }   
                printf("\nIn while loop\n");
                //continue;
            }

        }



void *connection_handler(void *arg_recvd){
    //Get the socket descriptor
    //int sock = *(int*)socket_desc;
    arg_struct arguments = *(arg_struct *)arg_recvd;
    int sock = arguments.new_sock;//*(int*)arguments.new_sock;
    char *buffer_recv;
    int bufsize_recv = 1024;  


    char *command;//[COMMANDSIZE];
    char *filename;//[nbytes];
    char *version;
    char *file_type;
    char *filename_temp;
    struct timeval tv;
    int numRead = 0;

    //char *content_type;
    tv.tv_sec = KEEP_ALIVE;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval)) < 0) 
                printf("\nError setting timeout %d",errno);



    command = (char *)malloc(COMMANDSIZE);
    filename = (char *)malloc(FILE_NAME_SIZE);
    filename_temp = (char *)malloc(FILE_NAME_SIZE);
    version = (char *)malloc(COMMANDSIZE);
    file_type = (char *)malloc(COMMANDSIZE);
    buffer_recv = (char *)malloc(bufsize_recv);
    //content_type = (char *)malloc(CONTENT_TYPE);

     //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
    puts("\n\n\rHandler assigned\n\r");    

        // I will leave it as an exercise for you to implement
        // a proper HTTP request parser here...
        
        //int numRead = recv(sock, buffer_recv, bufsize_recv, 0);
    while(numRead = recv(sock, buffer_recv, bufsize_recv, 0)){



        printf("\nClient socket = %d\n\rClient_port = %hu\n\r", sock, arguments.client_port);
        if (numRead < 1){
            if (numRead == 0){
                printf("The client was not read from: disconnected\n");
            } else {
                perror("The client was not read from");
            }
            close(sock);
            //continue;
            return 0; 
        }



        command = strtok(buffer_recv, " ");
        printf("command: %s\n",command);
        if(strcmp(command, "GET") && strcmp(command, "POST")){

            if(!strcmp(command, "PUT") || !strcmp(command, "HEAD") || !strcmp(command, "OPTIONS") || !strcmp(command, "DELETE") || !strcmp(command, "CONNECT")){
                if (!writeStrToClient(sock, "HTTP/1.1 501 Not Implemented\r\n\r\n")){
                    close(sock);
                    //continue;
                    return 0;
                }

                if (!writeStrToClient(sock, "<html><body>501 Not Implemented Reason HTTP REQUEST NOT HANDLED BY SERVER</body></html>")){
                    close(sock);
                    //continue;
                    return 0;
                }

                if (!writeStrToClient(sock, "\r\n\r\n")){
                    close(sock);
                    //continue;
                    return 0;
                }
            }
            else{
                if (!writeStrToClient(sock, "HTTP/1.1 400 Bad Request\r\n\r\n")){
                    close(sock);
                    //continue;
                    return 0;
                }

                if (!writeStrToClient(sock, "<html><body>400 Bad Request Reason: Invalid Method :")){
                    close(sock);
                    //continue;
                    return 0;
                }

                 if (!writeStrToClient(sock, command)){
                    close(sock);
                    //continue;
                    return 0;
                }
                 if (!writeStrToClient(sock, "</body></html>")){
                    close(sock);
                    //continue;
                    return 0;
                }
            }
                

                return 0;

        }
        
        filename = strtok(NULL, " ");
        printf("filename: %s\n",filename);

      
        version = strtok(NULL, " ");
        printf("version: %s\n",version);
 	 	if(strncmp(version,"HTTP/1.1",8)!=0 && strncmp(version, "HTTP/1.0",8)!=0){
            if (!writeStrToClient(sock, "HTTP/1.1 400 Bad Request\r\n\r\n")){
                    close(sock);
                    //continue;
                    return 0;
                }

                if (!writeStrToClient(sock, "<html><body>400 Bad Request Reason: Invalid version:")){
                    close(sock);
                    //continue;
                    return 0;
                }


                if (!writeStrToClient(sock, version)){
                    close(sock);
                    //continue;
                    return 0;
                }


                if (!writeStrToClient(sock, "</body></html>")){
                    close(sock);
                    //continue;
                    return 0;
                }

                return 0;
        }


        if(strcmp(filename, "/") == 0){
            //printf("\nIn if statement");
            strcat(filename, DEFAULT_FILE_1);//memcpy(filename, DEFAULT_FILE_1, strlen(DEFAULT_FILE_1)+1);
            memcpy(filename_temp, DEFAULT_FILE_1, strlen(DEFAULT_FILE_1)+1);
            printf("filename: %s\n",filename);

        }

        int len = strlen(filename);
        
        filename_temp = &filename[len-1];
        while(*filename_temp != '.'){
            --filename_temp;
        }
        file_type = ++filename_temp;

        printf("file_type: %s\n",file_type);
        printf("filename_temp: %s\n",filename_temp);




        long fsize;
        
        filename = ++filename;  
        printf("\nFilename:%s\n", filename);
        FILE *fp = fopen(filename, "rb");
        if (!fp){
            perror("The file was not opened");    
            if(errno == 2){
                if (!writeStrToClient(sock, "HTTP/1.1 404 NOT FOUND\r\n\r\n")){
                    close(sock);
                    //continue;
                    return 0;
                }

                if (!writeStrToClient(sock, "<html><body>404 Not Found Reason URL does not exist :</body></html>")){
                    close(sock);
                    //continue;
                    return 0;
                }

                 if (!writeStrToClient(sock, filename)){
                    close(sock);
                    //continue;
                    return 0;
                }

                 if (!writeStrToClient(sock, "<br /></body></html>")){
                    close(sock);
                    //continue;
                    return 0;
                }

                 if (!writeStrToClient(sock, "\n\rContent-Type: text/plain\r\n\r\n")){//html\r\n")){
                    close(sock);
                    //continue;
                    return 0;
                }
            }
            //exit(1);    
            return 0;
        }

        printf("The file was opened\n");





        if (fseek(fp, 0, SEEK_END) == -1){
            perror("The file was not seeked");
            //exit(1);
            return 0;
        }
        printf("\nFile seeked");

        fsize = ftell(fp);
        if (fsize == -1) {
            perror("The file size was not retrieved");
            //exit(1);
            return 0;
        }

        arguments.file_size = fsize;

        printf("\nGot file size");
        rewind(fp);

        arguments.message = (char *) malloc(fsize);

        //char *mesg = (char*) malloc(fsize);
        if (!arguments.message){
            perror("The file buffer was not allocated\n");
             if (!writeStrToClient(sock, "<html><body>HTTP/1.1 500 Internal Server Error: cannot allocate memory</body></html>")){
                close(sock);
                //continue;
                return 0;
            }
            //exit(1);
            return 0;
        }

        if (fread(arguments.message, fsize, 1, fp) != 1){
            perror("The file was not read\n");
            //exit(1);
            return 0;
        }

 
        fclose(fp);

        printf("The file size is %ld\n", fsize);




        printf("%.*s\n", numRead, buffer_recv);    

        if (!writeStrToClient(sock, "HTTP/1.1 200 OK\r\n")){
            close(sock);
            //continue;
            return 0;
        }

        char clen[40];
        sprintf(clen, "Content-length: %ld\r\n", arguments.file_size);
        if (!writeStrToClient(sock, clen)){
            close(sock);
            //continue;
            return 0;
        }

        // content_type = "Content-Type: text/";
        // content_type =  strcat(content_type, file_type);
        // content_type = strcat(content_type, "/html\r\n");

        if (!writeStrToClient(sock, "Content-Type: text/")){//html\r\n")){
            close(sock);
            //continue;
            return 0;
        }
        if (!writeStrToClient(sock, file_type)){//html\r\n")){
            close(sock);
            //continue;
            return 0;
        }
        if (!writeStrToClient(sock, "\r\n")){//html\r\n")){
            close(sock);
            //continue;
            return 0;
        }


        if (!writeStrToClient(sock, "Connection: keep-alive\r\n\r\n") == -1){
            close(arguments.new_sock);
            //continue;
            return 0;
        }


        if(strcmp(command, "POST") == 0){
            if (!writeStrToClient(sock, "<html><body><pre><h1>POSTDATA</h1></pre>\r\n\r\n") == -1){
                close(arguments.new_sock);
                //continue;
                return 0;
            }
        }
        


        //if (!writeStrToClient(new_socket, "<html><body><H1>Hello world</H1></body></html>")){
        if (!writeDataToClient(sock, arguments.message, arguments.file_size)){
            close(sock);
            //continue;
            return 0;
        }

        printf("The file was sent successfully\n");



        bzero(command,strlen(command));
        bzero(filename,strlen(filename));
        bzero(filename_temp,strlen(filename_temp));
        bzero(file_type,strlen(file_type));
        bzero(version,strlen(version));
        bzero(buffer_recv,strlen(buffer_recv));



    }
        
        int true = 1;
        close(sock);   
        setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)); 
        printf("\n\rSocket num %d closed\n", sock);
        pthread_exit(NULL);

        free(filename);
        free(file_type);

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
        printf("\nnumSent=%d    datalen=%d",numSent,datalen);
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

int main(int argc, char * argv){
    int new_socket;  
    char *buffer;
    int bufsize = 1024;    
    struct sockaddr_in address;   
    arg_struct * args; 
    socklen_t addrlen; 
    SEG_FLAG = 0;   

    args = (arg_struct *)malloc(sizeof(arg_struct));
    // char *command;//[COMMANDSIZE];

    signal(SIGINT, exit_function); 
    //signal(SIGSEGV, segFault_function); 

    buffer = (char*) malloc(bufsize);    
    if (!buffer){
        printf("The receive buffer was not allocated\n");
        exit(1);    
    }


    parse_config(); 
    PORT_NO = atoi(argv);

    printf("\nPORT_NO:%u\n", PORT_NO);


    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (create_socket == -1){    
        perror("The socket was not created");    
        exit(1);    
    }

    printf("The socket was created\n");

    memset(&address, 0, sizeof(address));    
    address.sin_family = AF_INET;    
    address.sin_addr.s_addr = INADDR_ANY;    
    address.sin_port = htons(PORT_NO);   

    if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == -1){    
        perror("The socket was not bound");    
        exit(1);    
    }

    printf("The socket is bound\n");    


    if (listen(create_socket, 10) == -1){
        perror("The socket was not opened for listening");    
        exit(1);    
    }    

    printf("The socket is listening\n");

    addrlen = sizeof(struct sockaddr_in);

    while (args->new_sock = accept(create_socket, (struct sockaddr *) &address, (socklen_t*)&addrlen)) {    

            
            //new_socket = accept(create_socket, (struct sockaddr *) &address, (socklen_t*)&addrlen);
            printf("Socket accepted\n");

            if (args->new_sock == -1) {    
                perror("A client was not accepted");    
                exit(1);    
            }    

            printf("A client is connected from %s:%hu...\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            args->client_port = ntohs(address.sin_port);

            pthread_t sniffer_thread;
            //args->new_sock = (int *)malloc(1);
            //args->new_sock = new_socket;
            //args->file_size = fsize;
             
            if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) args) < 0)
            {
                perror("could not create thread");
                return 1;
            }
  
        }    

   close(create_socket);    
   return 0;    
}