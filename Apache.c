/*
* CS 656 / Fall 2019 / Term Project
* Group: W1 / Leader: Harshil Sharma(hs722)
* Group Members: Yash(yt282), Raunak(rk776), Karan(kk594), Siddhesh(sbk37), Avaneesh(apm65), Harshil(hs722) 
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

char HOST[1024]; // don't change this!
int  PORT ;
struct in_addr PREFERRED; /* you must set this to the preferred IP, in dns() below */

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

int parse(char *input)        /*check for whitespace and parse the input*/
{
 char *endspace=input + strlen(input)-1;
 while(isspace(*endspace)){
 endspace=endspace-1;}
 *(endspace+1)='\0';
 return *input;
}

void dns(int sockfd)      /*Find the ipaddress for the given hostname*/
{
    char iplen[INET_ADDRSTRLEN];
    struct addrinfo hints, *result, *res;
    int error;
    char prefferedString[256];
    const char *ipAddress;
    char newline = '\n';
    double smallestResponseTime = 1000000; // Inititally assigned a very big value
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    bzero(HOST, sizeof HOST);
    long count = read(sockfd,HOST,1024);
  if (count < 0) {
        perror("ERROR IN READING FROM SOCKET");
        exit(1);
    }
    parse(HOST);
    printf("\nREQ:%s",HOST);
    write(sockfd,"REQ:",4);
    write(sockfd,HOST,sizeof HOST);
    write(sockfd,&newline,sizeof newline);
    error = getaddrinfo((const char*)HOST, NULL, &hints, &result);
    if (error != 0)
    {printf(" / RESP: ERROR\n");
    write(sockfd,"This website can not be found.   ",33);
    write(sockfd,&newline,sizeof newline); exit(1);}
    for (res=result;res!=NULL;res=res->ai_next){
    if(res->ai_family == AF_INET){                                         // Get human readable ip4 address
        struct sockaddr_in * ip = (struct sockaddr_in * ) res->ai_addr;
        void *ptr;
        ptr = &(ip->sin_addr);
        ipAddress = inet_ntop(AF_INET,ptr,iplen,sizeof(iplen));
    }
        write(sockfd,ipAddress,sizeof iplen);                                 //Write IP Address to client
        write(sockfd,&newline,sizeof newline);
        struct timeval  pingStartTime, pingEndTime; 
        gettimeofday(&pingStartTime, NULL);                                //Time stamp that marks the start of ping
        getnameinfo(res->ai_addr, res->ai_addrlen, HOST,1025, NULL, 0, 0);
        gettimeofday(&pingEndTime, NULL);                                  // Time stamp that marks the end of ping                                                    
        double resposeTime = (double) (pingEndTime.tv_usec - pingStartTime.tv_usec) / 1000000 + (double) (pingStartTime.tv_sec - pingEndTime.tv_sec);         // Response time by subtracting start and end time
        if(resposeTime < smallestResponseTime) {
        smallestResponseTime = resposeTime;
        memset(prefferedString,0, sizeof(prefferedString));
        strcpy(prefferedString, ipAddress);
        }
        ipAddress = NULL;
    } 
    printf(" / RESP:%s\n",prefferedString);
    strcat(prefferedString,"(Preffered)");
    write(sockfd,prefferedString,sizeof prefferedString);
    write(sockfd,&newline,sizeof newline);
}

int main(int argc, char *argv[])
{
    int sockfd, connect_fd, count=1,value= 1;
    struct sockaddr_in server_add, client_add;
    char myAddr[256];
    socklen_t add_len;
    if (argc < 2) { fprintf(stderr,"\nNO PORT NUMBER PROVIDED\n");exit(1);}
    do{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);                              //Socket creation
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof value);        // Reuse the socket for connections
    if (sockfd < 0){ fprintf(stderr,"\nUNABLE TO OPEN SOCKET\n");exit (1);}  
    bzero((char *) &server_add, sizeof(server_add));
    PORT = atoi(argv[1]);
    server_add.sin_family = AF_INET;                                       //Define address
    server_add.sin_port = htons(PORT);
    server_add.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr *) &server_add,sizeof(server_add))<0)   //Binding the socket
    {perror("\nBINDING UNSUCCESSFUL\n");exit(1);}
    if(listen(sockfd,5)!=0) { perror("\nLISTENING FAILED\n"); exit(1);}         //Listening on socket
    else { printf("\nApache server running on port:%i\n",PORT); }
    add_len= sizeof(client_add);
    connect_fd = accept(sockfd,(struct sockaddr *)&client_add, &add_len);   //Accept connection from client
    gethostip(myAddr);
    if (connect_fd < 0){ perror("\nSERVER UNABLE TO ACCEPT\n"); exit(1);} else {
   printf("\n(%d)Incoming client connection from [%s:%d] to me [%s:%d].",count,inet_ntoa(client_add.sin_addr),ntohs(client_add.sin_port),myAddr,PORT);
    }
    dns(connect_fd);
    count=count +1;
    close(connect_fd);
    close(sockfd);
    }while(1);
    return 0;
}
