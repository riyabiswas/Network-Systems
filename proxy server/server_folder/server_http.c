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
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h> //for threading , link with lpthread



#define MAXBUFSIZE 1000
#define FILECHUNKSIZE 1000
#define SEND_BUFF_SIZE 1000
#define COMMANDSIZE 10
#define MAX_SEQ_NUM 11
#define MAXLINELENGTH 128
#define FILE_NAME_SIZE 100
#define CONTENT_TYPE 30
#define ACK_SIZE 6

//unsigned char md5_result[MD5_DIGEST_LENGTH];

int SEG_FLAG;
char PORT_STR[6];
static uint16_t PORT_NO;
int create_socket;

long int timeout;

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


/*
    Get ip from domain name
 */
 
int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
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

int find_ip(char * hostname, char * ip){

    char * str;
    char * input_line;
    int nread;
    size_t len = 128;

    str = (char *)malloc(MAXLINELENGTH);
    input_line = (char *)malloc(MAXLINELENGTH);

    FILE *fpIP = fopen("ip_hostname.txt", "r+b");
    if (!fpIP){
        perror("The file was not opened");
        return (-1);
    }

    while((nread = getline(&input_line, &len, fpIP)) != -1){

        str = strtok(input_line, " ");

        if(strcmp(str,hostname) == 0){

            str = strtok(NULL, " ");
            memcpy(ip, str, strlen(str)+1);
            printf("\nIP: %s\n", ip);
            fclose(fpIP);
            return 1;
        }

    }

    if(!hostname_to_ip(hostname , ip)){

        char * host_ip;
        host_ip = (char *)malloc(2*FILE_NAME_SIZE);
        strcat(host_ip, "\n");
        strcat(host_ip, hostname);
        strcat(host_ip, " ");
        strcat(host_ip, ip);
        
        printf("\nHOSTNAME IP: %s\n", host_ip);

        if(!fwrite(host_ip , strlen(host_ip), 1 , fpIP )){
            perror("File write failed");
        }
        fclose(fpIP);

        return 1;
    }
    else{

        fclose(fpIP);
        return (-1);
    }
}


int blockIP(char * ip){

    char * str;
    char * input_line;
    int nread;
    size_t len = 128;

    printf("\nblockIP() accessed");
    str = (char *)malloc(MAXLINELENGTH);
    input_line = (char *)malloc(MAXLINELENGTH);

    FILE *fpBlock = fopen("block_list.txt", "r+b");
    if (!fpBlock){
        perror("The file was not opened");
        return (-1);
    }

    printf("\n\rFile block_list.txt opened");
    while((nread = getline(&input_line, &len, fpBlock)) != -1){

        printf("\n\rInside while of blockIP() IP:%s", ip);
        //str = strtok(input_line, " ");
        if(strcmp(input_line, ip) == 0){
            printf("\nIP needs to be blocked");
            fclose(fpBlock);
            return 0;
        }
    }

    fclose(fpBlock);
    return 1;

}


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
    char *hostname;
    char *host_port;
    char *server_portno_str;
    char *ip;
    int n;
    int server_portno = 80;
    char * proxy_buffer;
    char *temp;
    //int fpCache;
    //char md5string[33];
    unsigned char md5_result[MD5_DIGEST_LENGTH];
    char md5string[33];
    struct stat stbuf;
    time_t timestamp_sec;


    // unsigned char digest[16];
    // struct MD5Context context;

    
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
    hostname = (char *)malloc(FILE_NAME_SIZE);
    ip = (char*)malloc(COMMANDSIZE);
    server_portno_str = (char *)malloc(COMMANDSIZE);
    host_port = (char *)malloc(FILE_NAME_SIZE);
    proxy_buffer = (char *)malloc(4096);


    //content_type = (char *)malloc(CONTENT_TYPE);

     //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
    puts("\n\n\rHandler assigned\n\r");    

        // I will leave it as an exercise for you to implement
        // a proper HTTP request parser here...
        
        //int numRead = recv(sock, buffer_recv, bufsize_recv, 0);
    while(numRead = recv(sock, buffer_recv, bufsize_recv, 0)){

        // char * check_buffer = (char *)malloc(bufsize_recv);
        // strcpy(check_buffer, buffer_recv);
        // MD5Init(&context);
        // MD5Update(&context, buffer_recv, numRead);
        // MD5Final(digest, &context);
        MD5(buffer_recv, numRead, (unsigned char*)&md5_result); 

        
        for(int i = 0; i < 16; i++)
            sprintf(&md5string[i*2], "%02x", (unsigned int)md5_result[i]);


        printf("RECV REQUEST: %s", buffer_recv);
        
        
        printf("\nMD5 result=%s", md5string);

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
        if(strcmp(command, "GET")){

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
            

            return 0;

        }
        
       
        host_port = strtok(NULL, " ");
        
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

        //Parse hostname and port number
        printf("\nhost_port: %s\n",host_port);


        server_portno_str = strstr(host_port, ":");
        if(server_portno_str && (strncmp(host_port, "http", 4) != 0)){

            server_portno = atoi(++server_portno_str);
           
            temp = hostname;
            while(*host_port != ':'){
                *hostname++ = *host_port++;
            }
            hostname = temp;
        }
        else if(strncmp(host_port, "http", 4) == 0){
            strcpy(hostname, host_port + 7);
            hostname = strtok(hostname, "/");
            // filename = strtok(NULL, " ");
            // printf("filename: %s", filename);
            // strcpy(filename, host_port + 7);
            // filename = strtok(filename, "?");
            // // printf("filename: %s", filename);
            // while(*filename != '/')
            //     ++filename;
            
        }
        else{

            hostname = host_port;
        }

        if(server_portno == 0)
            server_portno = 80;

        printf("\nHostname: %s\n", hostname);
        printf("\nServer_port: %d", server_portno);
        // hostname_to_ip(hostname , ip);


        if(find_ip(hostname, ip) < 1){

            printf("Couldn't resolve host name");

            if (!writeStrToClient(sock, "<html><body>400 Bad Request: Server not found</body></html>\r\n\r\n")){
                close(sock);
                //continue;
                return 0;
            }

            return 0;
        }   


    if(blockIP(ip) < 1){
        printf("\n\r IP BLOCKED!!");

        if (!writeStrToClient(sock, "HTTP/1.1 ERROR 403 Forbidden\r\n\r\n")){
            close(sock);
            //continue;
            return 0;
        }

        if (!writeStrToClient(sock, "<html><body>ERROR 403 Forbidden<html><body>")){
                    close(sock);
                    //continue;
                    return 0;
                }

        return 0;

    }
 
    //Define server address
    int sockfd, n;
    struct sockaddr_in serv_addr;


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        perror("ERROR opening socket");


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(server_portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) perror("ERROR connecting");
    char * request;
    request = (char *)malloc(100);
    strcpy(request, command);
    strcat(request, " / HTTP/1.0\r\nHost: ");//google.com\r\nConnection: close\r\n\r\n");
    char * host_request;
    // host_request = hostname + 4;
    strcat(request, hostname);
    strcat(request, "\r\nConnection: close\r\n\r\n");
    // n = write(sockfd,request,strlen(request));
    // if (n < 0) error("ERROR writing to socket");
    printf("REQUEST: %s", request);
    // if (!writeStrToClient(sockfd, request)){
    //     close(sockfd);
    //     //continue;
    //     return 0;
    // }
    
     if (!writeStrToClient(sockfd, request)){
        close(sockfd);
        //continue;
        return 0;
    }
    
    bzero(filename, FILE_NAME_SIZE);
    strcpy(filename, md5string);
    strcat(filename, ".cache");

    long fsize;
    int mod_flag = 1;

    printf("\n\nfilename: %s", filename);
    if( access( filename, F_OK ) != -1 ) {
        printf("\n\nFile Exists");


        if(stat(filename,&stbuf) == -1){
            perror("\nStatus of file error:");
            return 0;
        }
        printf("\nModification time  = %ld\n",stbuf.st_mtime);
        time(&timestamp_sec);  // get current time; same as: timestamp_sec = time(NULL)  
        printf("\ncurrent time  = %ld\n",timestamp_sec);
        long int time_dif = timestamp_sec - stbuf.st_mtime;
        printf("\nTime since last modification  = %ld\n",(timestamp_sec - stbuf.st_mtime));

        if(time_dif < timeout){

            mod_flag = 0;

            FILE * fpCache= fopen(filename, "r");  

            if (fseek(fpCache, 0, SEEK_END) == -1){
                perror("The file was not seeked");
                //exit(1);
                return 0;
            }
            printf("\nFile seeked");

            fsize = ftell(fpCache);
            if (fsize == -1) {
                perror("The file size was not retrieved");
                //exit(1);
                return 0;
            }

            arguments.file_size = fsize;

            printf("\nGot file size");
            rewind(fpCache);

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

            if ((fread(arguments.message, fsize, 1, fpCache)) < 1){
                perror("\n\nThe file was not read");
                fclose(fpCache);
                //exit(1);
                return 0;
            }
     
            fclose(fpCache);

            if (!writeDataToClient(sock, arguments.message, arguments.file_size)){
                close(sock);
                //continue;
                return 0;
            }

            //end of if(dif <120)
        }


        // file exists
    } 
    if(mod_flag == 1) {

        printf("\n\nFile does not exist");
        FILE * fpCache= fopen(filename, "w");
        if(!fpCache){

            printf("\nError opening file %s errno: %d", filename, errno);
            perror("The file was not opened"); 
            
        }
        else
            printf("File %s created", filename);

        while (1){
            bzero(proxy_buffer,4096);
            n = recv(sockfd,proxy_buffer,4095, 0);
            if (n < 0) {
                    perror("ERROR reading from socket");
                    break;
            }
            if (n == 0) {
                // server end has closed socket
                break;
            }

            if(!fwrite(proxy_buffer , n, 1 , fpCache )){
                perror("File write failed");
            }            
            //printf("\nproxy_buffer: %s", proxy_buffer);

            if (!writeDataToClient(sock, proxy_buffer, strlen(proxy_buffer))){
                close(sock);
                //continue;
                return 0;
            }
        }

        fclose(fpCache);
        
    }


    

      /*  if(strcmp(filename, "/") == 0){
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

    */   

     
    /*
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

    */

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



int main(int argc, char * argv[]){
    int new_socket;  
    char *buffer;
    int bufsize = 1024;    
    struct sockaddr_in address;   
    arg_struct * args; 
    socklen_t addrlen; 
    SEG_FLAG = 0;  


    args = (arg_struct *)malloc(sizeof(arg_struct));

    if (argc != 3)
	{
		printf ("USAGE:  <port> <timeout>\n");
		exit(1);
	}

    // char *command;//[COMMANDSIZE];

    signal(SIGINT, exit_function); 
    //signal(SIGSEGV, segFault_function); 

    buffer = (char*) malloc(bufsize);    
    if (!buffer){
        printf("The receive buffer was not allocated\n");
        exit(1);    
    }


    parse_config(); 
    PORT_NO = atoi(argv[1]);
    timeout = atol(argv[2]);
    printf("\nPORT_NO:%u\n", PORT_NO);


    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (create_socket == -1){    
        perror("The socket was not created");    
        exit(1);    
    }

    printf("The socket was created\n");

    memset(&address, 0, sizeof(address));    
    address.sin_family = AF_INET;    
    address.sin_addr.s_addr = inet_addr("127.0.0.1");//INADDR_ANY;    
    address.sin_port = htons(PORT_NO);  

    int true = 1; 

    setsockopt(create_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)); 

    if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == -1){    
        perror("The socket was not bound");    
        exit(1);    
    }

    printf("The socket is bound\n");    


    if (listen(create_socket, 50) == -1){
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
            //args->client_port = ntohs(address.sin_port);

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