#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

//function takes a addrinfo struct and prints associated IPv4 address
void printIP(struct addrinfo *serverInfo);

int main(int argc, char *argv[]){
    /*Read command line*/
    char* udpServerHostName = argv[1];
    char* udpServerPort = argv[2];
    char* name = argv[3];

    /*create addrinfo structs to pass to network functions*/
    struct addrinfo udpConnectionInfo;
    struct addrinfo *udpServerInfo;

    /*initialize connectionInfo struct*/
    memset(&udpConnectionInfo, 0, sizeof(udpConnectionInfo));
    udpConnectionInfo.ai_family = PF_INET;
    udpConnectionInfo.ai_socktype = SOCK_DGRAM;
    udpConnectionInfo.ai_flags = AI_PASSIVE;

    /*resolve host name*/
    if(getaddrinfo(udpServerHostName, udpServerPort, &udpConnectionInfo, &udpServerInfo) != 0){
        exit(-1);
    }

    /*create socket*/
    int UDPSocket = socket(udpServerInfo->ai_family, udpServerInfo->ai_socktype, udpServerInfo->ai_protocol);
    if(UDPSocket == -1){
        exit(-1);
    }

    /*send and receive UDP data*/
    char buffer[100];
    int bytesSent = sendto(UDPSocket, name, strlen(name), 0, udpServerInfo->ai_addr, udpServerInfo->ai_addrlen);
    printf("UDP Data sent: %s\n", name);
    int bytesRecieved = recvfrom(UDPSocket, buffer, sizeof(buffer), 0, udpServerInfo->ai_addr, &udpServerInfo->ai_addrlen);
    printf("UDP Data rcvd: %s\n", buffer);

    /*Find index of space between server name and port*/
    int spaceIndex = -1;
    for(int i=0; i<bytesRecieved; i++){
        if(buffer[i] == ' '){
            spaceIndex = i;
            break;
        }
    }

    /*Parse reply from UDP Server*/
    char* tcpServerHostName = (char *) calloc(spaceIndex, sizeof(char));
    for(int i=0; i<spaceIndex; i++){
        tcpServerHostName[i] = buffer[i];
    }

    char* tcpServerPort = (char *) calloc(bytesRecieved - spaceIndex, sizeof(char));
    for(int i=spaceIndex+1; i<bytesRecieved; i++){
        tcpServerPort[i - (spaceIndex + 1)] = buffer[i];
    }

    /**********************/   
    /*Setup TCP Connection*/
    /**********************/
    struct addrinfo tcpConnectionInfo;
    struct addrinfo *tcpServerInfo;

    memset(&tcpConnectionInfo, 0, sizeof(tcpConnectionInfo));
    tcpConnectionInfo.ai_family = PF_INET;
    tcpConnectionInfo.ai_socktype = SOCK_STREAM;
    tcpConnectionInfo.ai_flags = AI_PASSIVE;

    /*resolve host name*/
    if(getaddrinfo(tcpServerHostName, tcpServerPort, &tcpConnectionInfo, &tcpServerInfo) != 0){
        exit(-1);
    }

    /*create socket*/
    int tcpSocket = socket(tcpServerInfo->ai_family, tcpServerInfo->ai_socktype, tcpServerInfo->ai_protocol);
    if(tcpSocket == -1){
        exit(-1);
    }

    /*initiate connection*/
    if(connect(tcpSocket, tcpServerInfo->ai_addr, tcpServerInfo->ai_addrlen) == -1){
        exit(-1);
    }

    /*initialize send/receive buffers*/
    bool done = false;
    char sendBuffer[50];
    char receiveBuffer[50];
    //initialize first tcp message to send
    strcpy(sendBuffer, strcat(name,"\n"));
    do{
        //send tcp message
        send(tcpSocket, sendBuffer, sizeof(sendBuffer), 0);
        printf("TCP data sent: %s", sendBuffer);
        //receive tcp message
        recv(tcpSocket, receiveBuffer, sizeof(receiveBuffer), 0);
        printf("TCP data rcvd: %s", receiveBuffer);

        //clear buffers
        memset(sendBuffer, 0, sizeof(sendBuffer));
        memset(receiveBuffer, 0, sizeof(receiveBuffer));

        //get next message from user
        printf("Next message? ");
        fgets(sendBuffer, sizeof(sendBuffer), stdin);

        //check if "quit" to close connection and exit loop
        if(strcmp("quit\n", sendBuffer) == 0){
            //if yes send and recieve last quit message
            send(tcpSocket, sendBuffer, sizeof(sendBuffer), 0);
            printf("TCP data sent: %s", sendBuffer);
            recv(tcpSocket, receiveBuffer, sizeof(receiveBuffer), 0);
            printf("TCP data rcvd: %s", receiveBuffer);

            //then close socket and exit loop
            close(tcpSocket);
            done = true;
        }
    }while(!done);
        
    

    exit(1);
}

//function takes a addrinfo struct and prints associated IPv4 address
void printIP(struct addrinfo *serverInfo){
    char ipstr[INET6_ADDRSTRLEN];   
    int i=0;
    char* ipver;
    void *addr;
    if(serverInfo->ai_family == PF_INET){
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)serverInfo->ai_addr;
        addr = &(ipv4->sin_addr);
        ipver = "IPv4";
    }
    inet_ntop(serverInfo->ai_family, addr, ipstr, sizeof(ipstr));
    printf("%s: %s\n", ipver, ipstr);
}