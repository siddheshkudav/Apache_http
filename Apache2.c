/*
     * CS 656 / Fall 2019 / ProjectN2
     * Group - W1
     Leader: Harshil Sharma (hs722)
     * Group Members: Siddhesh (sbk37), Karan (kk594), Raunak (rk776), Yash (yt282), Avaneesh (apm65)
     *
     */
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <ctype.h>
    #include <stdlib.h>
    #include <sys/time.h>
    #include <ifaddrs.h>
    #include <sys/ioctl.h>
    #include <net/if.h>
    #include <errno.h>
    #define BUFFERLEN 65535
    #define DEFAULTPORT 34355
    #define BSIZE 0x1000 
    #define atoa(x) #x

    char HOST[65535];
    char CPYHOST[65535];
    char URL[1024];  // don't change this!
    int PORT, HPORT,CPORT;

    char gethostip(char *ip_address) /*Get the ip address of host*/
    {
        int newsockfd;
        struct ifreq ifr;
        newsockfd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        memcpy(ifr.ifr_name, "ens192", IFNAMSIZ-1);
        ioctl(newsockfd, SIOCGIFADDR, &ifr);
        close(newsockfd);
        strcpy(ip_address,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        return *ip_address;
    }

    int parse(char* getreq, long length, char* host, char* HPORT, char* path)
    {
        char s1[BUFFERLEN], s2[BUFFERLEN], s3[BUFFERLEN], s4[BUFFERLEN], current[BUFFERLEN], format[255], *endofbuffer = getreq+length;
        short res=0, parse_res=1, i;
        sprintf(format,"%%%li[^\r\n]\r\n", length-1);
        sscanf(getreq,format,current);
        sscanf(current,"%s %s HTTP/1.%*c\r\n",s1,s2);

        if(strcmp(s1,"GET")!=0){
            parse_res = -1;
        }
        if(s2[0]!='/')
        {
            for(i=0;i<strlen(s2)&&s2[i]!=':';i++)
                s2[i]=tolower(s2[i]);
            if((res=sscanf(s2,"%[^:\r\n\f\t\v ]://%[^/]%s",s1,s3,s4))>1)
            {
                if(res>2)
                    strcpy(path,s4);//path
                res=sscanf(s3,"%[^:\r\n\f\t\v ]:%[0-9]",s2,s4);
                if(res>0)
                    strcpy(host,s2);//host
                
                if(res==2)
                    strcpy(HPORT,s4);
                else
                    strcpy(HPORT,"80");
            }
            if(strcmp(s1,"http")!=0){
            	parse_res = -1;
	    }
        }
        else
        strcpy(path,s2);
        getreq+=strlen(current)+2;
        while(getreq<(endofbuffer-1))
        {
            if(memcmp(getreq,"\r\n",2)==0)
            {
                if(parse_res!=-1)
                    parse_res = 0;
                return parse_res;
            }
            sprintf(format,"%%%li[^\r\n]\r\n", endofbuffer-getreq);
            sscanf(getreq,format,current);
            for(i=0;i<strlen(current)&&current[i]!=':';i++)
                current[i]=tolower(current[i]);
            if((res=sscanf(current,"host: %[^:\r\n\f\t\v ]:%[0-9]",s1,s2))>0)
            {
                strcpy(host,s1);
                if(res==2)
                    strcpy(HPORT,s2);
                else
                    strcpy(HPORT,"80");
            }
            getreq+=strlen(current)+2;
        }
        return parse_res;
    }

    int do_local_file_transfer (int connect_fd, char * file_path) { 
        int c = 0,position = 2;
        long total_read = 0;
        char file_name[1000];
        int length = strlen(file_path) - 1;
        while (c < length) {
            file_name[c] = file_path[position+c-1];
            c++;}
        file_name[c] = '\0';
        printf("\nREQUESTING FILE :%s", file_name);
        FILE *fp = fopen(file_name,"rb");
        if(fp==NULL)
        {return -1;}
        while(1)
        {unsigned char buff[256]={0};
            int nread = fread(buff,1,256,fp);
            total_read=total_read + 1;
            if(nread > 0)
            { write(connect_fd, buff, nread);
            }if (nread < 256)
            {   if (feof(fp))
                if (ferror(fp))
                    printf("RESP: Error in Reading\n");
                    break;
            }}
        total_read=total_read * 256;
        printf(" / RESP: (%ld bytes transfered.)\n",total_read);
        return 0;
    }

    char * write_error_to_client(int connect_fd, int error_code){
        char error_message[1024];
        if(error_code == 404) {
            strcpy(error_message, "PAGE NOT FOUND\n");
        } else if (error_code == 400) {
            strcpy(error_message, "BAD HTTP REQUEST\n");
        } else if (error_code == 431) {
            strcpy(error_message, "BIG REQUEST ERROR\n");
        } else if (error_code == 408) {
            strcpy(error_message, "REQUEST TIMEOUT\n");
        } else if (error_code == 501) {
            strcpy(error_message, "REQ DOES NOT HAVE A GET OR IS A HTTPS REQUEST OR HAS BEEN MOVED PERMANENTLY\n"); 
        } else if (error_code == 504) {
            strcpy(error_message, "GETWAY CONNECTION TIMEOUT\n");
        } else if (error_code == 101) {
            strcpy(error_message, "FILE TRANSFER ERROR\n");
        } else {
            strcpy(error_message, "UNKOWN ERROR CODE\n");
        }
        char * return_error_pointer = error_message;
	char response[BUFFERLEN];
sprintf(response, "HTTP/1.1 %i %s"
              "Connection: close\r\n"
              "Content-Type: text/html; charset=iso-8859-1\r\n"
              "\r\n"
              "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n" //51
              "<html><head>\n" 
              "<title>Error</title>\n" 
              "</head><body>\n" 
              "<h1>%i</h1>\n" 
              "<p>%s<br />\n" 
              "</p>\n" 
              "</body></html>\n",error_code,error_message,error_code,error_message);               
        write(connect_fd,response, strlen(response));
        return return_error_pointer;
    }


    int dns(char *host,char * prefferedString, int sockfd, int connect_fd)      /*Find the ipaddress for the given hostname*/
    {char iplen[INET_ADDRSTRLEN];
        struct addrinfo hints, *result, *res;
        int error;
        const char *ipAddress="";
        
        double smallestResponseTime = 1000000; // Inititally assigned a very big value
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_flags = AI_CANONNAME;
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        char *endspace=host + strlen(host)-1;
        while(isspace(*endspace)){
            endspace=endspace-1;}
        *(endspace+1)='\0';
        error = getaddrinfo((const char*)host,NULL, &hints, &result);
        if (error != 0)
        { return -1;
        }
        else {
            for (res=result;res!=NULL;res=res->ai_next){
                if(res->ai_family == AF_INET){                                         // Get human readable ip4 address
                    struct sockaddr_in * ip = (struct sockaddr_in * ) res->ai_addr;
                    void *ptr;
                    ptr = &(ip->sin_addr);
                    ipAddress = inet_ntop(AF_INET,ptr,iplen,sizeof(iplen));
                }struct timeval  pingStartTime, pingEndTime;
                gettimeofday(&pingStartTime, NULL);                                //Time stamp that marks the start of ping
                getnameinfo(res->ai_addr, res->ai_addrlen, HOST,1025, NULL, 0, 0);
                gettimeofday(&pingEndTime, NULL);                                  // Time stamp that marks the end of ping
                double resposeTime = (double) (pingEndTime.tv_usec - pingStartTime.tv_usec) / 1000000 + (double) (pingStartTime.tv_sec - pingEndTime.tv_sec);         // Response time by subtracting start and end time
                if(resposeTime < smallestResponseTime) {
                    smallestResponseTime = resposeTime;
                    if(ipAddress != NULL) {
                        strcpy(prefferedString, ipAddress);
                    } 
                }
                ipAddress = NULL;
            }
            freeaddrinfo(res);
            freeaddrinfo(result);
            return *prefferedString;  }}

long setconnection(char* message,long length,char* val)
{
    char cline[BUFFERLEN], form[255], cpval[BUFFERLEN], *end = message+length;
    long newlength = -1;
    int i;
    while((message<(end-1)) && (memcmp(message,"\r\n",2)!=0))
    {
        sprintf(form,"%%%li[^\r\n]\r\n", (end-message)-2);
        sscanf(message,form,cline);
        for(i=0;i<strlen(cline)&&cline[i]!=':';i++)
            cline[i]=tolower(cline[i]);
            if(sscanf(cline,"connection: %s",cpval) || sscanf(cline,"proxy-connection: %s",cpval))
           {  
            if(val==NULL)
            {
                memmove(message,message+strlen(cline)+2,end-(message+strlen(cline)+2));//shift the data to remove the connection field
                newlength = length-(strlen(cline)+2);
                break;
            }
            else
            {
                sprintf(message,"connection: %s",val);
                message[12+strlen(val)] = '\r';
                message[13+strlen(val)] = '\n';
                newlength = length-(strlen(cline)-(strlen(val)+12));
                memmove(message+14+strlen(val),message+strlen(cline)+2,end-(message+strlen(cline)+2));
                break;
            }
        }
        message+=strlen(cline)+2;
    }
    if(newlength==-1)
        newlength = length;
    return newlength;
}

 int ht_fetch(int connect_fd, char * host, char * CPYHOST, char * prefferedString,char * HPORT,char * path, int count)
 {int send_status; // fetch request from actuall server flag
  long read_length = 0;
  char web_socket_buffer[BUFFERLEN];
  int file_transfer;
 
  if(strcmp(host, "localfile") == 0) {
                file_transfer = do_local_file_transfer(connect_fd,path);
                if(file_transfer == -1) {
                char* error = write_error_to_client(connect_fd,101);
                printf(" / RESP: ERROR!101 %s\n", error);
                }else {
                    close(connect_fd);
                    }
					}
            else{
                printf("\n REQ: %s ",host);
                    struct sockaddr_in urlServerAddr;
                    urlServerAddr.sin_addr.s_addr = inet_addr(prefferedString);
                    urlServerAddr.sin_family=AF_INET;
                    urlServerAddr.sin_port=htons(atoi(HPORT));
                    int tcpSocket = socket(AF_INET, SOCK_STREAM, 0); 
                    urlServerAddr.sin_family = AF_INET;
                    urlServerAddr.sin_port = htons(atoi(HPORT));
                    connect(tcpSocket, (struct sockaddr *) &urlServerAddr, sizeof(urlServerAddr)); // connect to that server

                    send_status = write(tcpSocket, CPYHOST, strlen(CPYHOST)); // request that came from browser is sent to server
                    if(send_status == -1) 
		    {  char* error  = write_error_to_client(connect_fd, 404); // Check which error code should come here
                        printf(" / RESP: ERROR!404 %s\n", error);
                        close(connect_fd);
                        close(tcpSocket);
                    }
					read_length = read(tcpSocket, web_socket_buffer, BUFFERLEN);
                    read_length = setconnection(web_socket_buffer,read_length,NULL);
                    if(strstr(web_socket_buffer, "HTTP/1.1 301 Moved Permanently") != NULL) {
                      char* error  = write_error_to_client(connect_fd, 501);
                      printf(" / RESP: ERROR!501 %s\n", error);
                      close(connect_fd);
                      close(tcpSocket);
                      return 0;
                    }
                    send_status = write(connect_fd, web_socket_buffer, read_length);
                    if(send_status == -1) {char* error  = write_error_to_client(connect_fd, 404);
                     printf(" / RESP: ERROR!404 %s\n", error);
                     close(connect_fd);
                    }
                    while((read_length = read(tcpSocket, web_socket_buffer, BUFFERLEN))>0)//get the rest of the data and send it to the client
                    {send_status = write(connect_fd, web_socket_buffer, read_length);
                        if(send_status <= 0){ 
                        close(connect_fd);
                        continue; }
                        
                        }
                    printf(" / RESP sen: (%d byes transfered.)\n",send_status);
                    close(tcpSocket);
                    close(connect_fd);}	 
	                  return 0;
 }
 


    int main(int argc, char * argv[]) {
        int sockfd, connect_fd, count = 0, value = 1;
        struct sockaddr_in server_add, client_add;
        char myAddr[256];
        char prefferedString[BUFFERLEN]="";
        char host[BUFFERLEN] = "";
        char HPORT[5] = "";
        char path[BUFFERLEN] = "";
        int dns_parse;
        int parse_out; // request parse error flag

        char split_request_buffer[BUFFERLEN];
        socklen_t add_len;
        if(argc >= 2){
        CPORT = atoi(argv[1]);
        if (CPORT>65535||CPORT<1024){
        printf("Input port invalid, defaulting to %i\n",DEFAULTPORT);
        PORT = DEFAULTPORT;}
        else PORT = CPORT;}
	else {
	printf("NO INPUT PORT PROVIDED, defaulting to %i\n",DEFAULTPORT);
	PORT = DEFAULTPORT;}
        sockfd = socket(AF_INET, SOCK_STREAM, 0); //Socket creation
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, & value, sizeof value); // Reuse the socket for connections
        if (sockfd < 0) {
            fprintf(stderr, "\nUNABLE TO OPEN SOCKET\n");exit(1);}
        bzero((char * ) & server_add, sizeof(server_add));
        server_add.sin_family = AF_INET; //Define address
        server_add.sin_port = htons(PORT);
        server_add.sin_addr.s_addr = INADDR_ANY;
        if (bind(sockfd, (struct sockaddr * ) & server_add, sizeof(server_add)) < 0) //Binding the socket
        {perror("\nBINDING UNSUCCESSFUL\n");exit(1);}
        if (listen(sockfd, 5) != 0) {
            perror("\nLISTENING FAILED\n"); exit(1);
        } //Listening on socket
        else {
            printf("\nApache server running on port:%i\n", PORT);}
        do {add_len = sizeof(client_add);
            connect_fd = accept(sockfd, (struct sockaddr * ) & client_add, & add_len); //Accept connection from client
            gethostip(myAddr);
            if (connect_fd < 0) {
                perror("\nSERVER UNABLE TO ACCEPT\n");
                exit(1);
            } else {
                count = count + 1; // variable name change to index / request_counter
                printf("\n(%d)Incoming client connection from [%s:%d] to me [%s:%d].", count, inet_ntoa(client_add.sin_addr), ntohs(client_add.sin_port), myAddr, PORT);
            }
            long count = read(connect_fd, HOST, BUFFERLEN);
            if (count < 0) {
                perror("ERROR IN READING FROM SOCKET");
                break; 
                }
            int parseLength = 0;
            if((parse_out=parse(HOST,count, host, HPORT, path)==1)){
            while((parse_out=parse(HOST,count, host, HPORT, path)==1)){
                    parseLength=read(connect_fd, split_request_buffer, BUFFERLEN);
                    if(parseLength<0) { 
                       char* error  = write_error_to_client(connect_fd, 504); 
                       printf("/ RESP: ERROR!504 %s\n",error); }
                    else if(parseLength==0) {//client closed connection without finishing the request, we're done
                        close(connect_fd);}
                    if(count+parseLength > BUFFERLEN) { 
                    char* error = write_error_to_client(connect_fd,431);
                    printf(" / RESP: ERROR!431 %s\n", error);
                        close(connect_fd);
                        continue;}
                    memmove(HOST+count,split_request_buffer,parseLength);//continue building our complete request
                    count+=parseLength; }
            } else {
                parse_out=parse(HOST,count, host, HPORT, path);
            }
	    strcpy(CPYHOST, HOST);
		if(parse_out == -1) { // This is if we get error while parsing request
                char* error  = write_error_to_client(connect_fd, 501); // Send appropriate code for wrong request
                printf(" / RESP: ERROR!501 %s\n", error);
                close(connect_fd);
                continue;}
      dns_parse =dns(host,prefferedString,sockfd,connect_fd); 
      if (dns_parse==-1 && (strcmp(host,"localfile")!=0)) // Preffered Address error
                {  char* error  = write_error_to_client(connect_fd, 404);
                    printf(" / RESP: ERROR!404 %s\n", error);
                    close(connect_fd);
                    continue;}
			ht_fetch(connect_fd,host,CPYHOST,prefferedString,HPORT,path,count);
            bzero(prefferedString,65535);
            bzero(host,sizeof host);
            bzero(HOST, sizeof HOST);
            bzero(CPYHOST, sizeof CPYHOST);
            bzero(path,sizeof path);
            bzero(HPORT,sizeof HPORT);
        } while (1);
        return 0;}



