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
    //Read command line
    char* UDPServerHostName = argv[1];
    char* UDPServerPort = argv[2];
    char* name = argv[3];

    //print to user
    printf("Server Host: %s\n", UDPServerHostName);
    printf("Server Port: %s\n", UDPServerPort);
    printf("User Name: %s\n", name);

    struct addrinfo udpConnectionInfo;
    struct addrinfo *udpServerInfo;

    memset(&udpConnectionInfo, 0, sizeof(udpConnectionInfo));

    udpConnectionInfo.ai_family = PF_INET;
    udpConnectionInfo.ai_socktype = SOCK_DGRAM;
    udpConnectionInfo.ai_flags = AI_PASSIVE;

    if(getaddrinfo(UDPServerHostName, UDPServerPort, &udpConnectionInfo, &udpServerInfo) != 0){
        exit(-1);
    }

    int UDPSocket = socket(udpServerInfo->ai_family, udpServerInfo->ai_socktype, udpServerInfo->ai_protocol);
    if(UDPSocket == -1){
        exit(-1);
    }

    /*Begin sending/receiving UDP Portion*/
    char buffer[100];
    int bytesSent = sendto(UDPSocket, name, strlen(name), 0, udpServerInfo->ai_addr, udpServerInfo->ai_addrlen);
    printf("UDP Data sent: %s\n", name);
    int bytesRecieved = recvfrom(UDPSocket, buffer, sizeof(buffer), 0, udpServerInfo->ai_addr, &udpServerInfo->ai_addrlen);
    printf("UDP Data rcvd: %s\n", buffer);
    

    int spaceIndex = -1;
    for(int i=0; i<bytesRecieved; i++){
        if(buffer[i] == ' '){
            spaceIndex = i;
            break;
        }
    }

    /*Parse reply from UDP Server*/
    char* TCPServerHostName = (char *) calloc(spaceIndex, sizeof(char));
    for(int i=0; i<spaceIndex; i++){
        TCPServerHostName[i] = buffer[i];
    }

    char* TCPServerPort = (char *) calloc(bytesRecieved-spaceIndex, sizeof(char));
    for(int i=spaceIndex+1; i<bytesRecieved; i++){
        TCPServerPort[i-(spaceIndex+1)] = buffer[i]; 
    }
     
    /*Setup TCP Connection*/
    struct addrinfo tcpConnectionInfo;
    struct addrinfo *tcpServerInfo;

    memset(&tcpConnectionInfo, 0, sizeof(tcpConnectionInfo));

    tcpConnectionInfo.ai_family = PF_INET;
    tcpConnectionInfo.ai_socktype = SOCK_STREAM;
    tcpConnectionInfo.ai_flags = AI_PASSIVE;

    if(getaddrinfo(TCPServerHostName, TCPServerPort, &tcpConnectionInfo, &tcpServerInfo) != 0){
        exit(-1);
    }

    int tcpSocket = socket(tcpServerInfo->ai_family, tcpServerInfo->ai_socktype, tcpServerInfo->ai_protocol);
    if(tcpSocket == -1){
        exit(-1);
    }

    if(connect(tcpSocket, tcpServerInfo->ai_addr, tcpServerInfo->ai_addrlen) == -1){
        exit(-1);
    }

    send(tcpSocket, "sam-morgan", strlen("sam-morgan"), 0);
    printf("TCP Data Sent: %s\n", "sam-morgan");
    char tcpBuffer[100];
    recv(tcpSocket, tcpBuffer, sizeof(buffer), 0);
    printf("TCP Data rcvd: %s\n", tcpBuffer);

    bool done = false;
    char sendBuffer[50];
    char receiveBuffer[50];
    while(!done){
        printf("Next message?");
        fgets(sendBuffer, sizeof(sendBuffer), stdin);
        printf("TCP data sent: %s", sendBuffer);

        send(tcpSocket, sendBuffer, sizeof(sendBuffer), 0);
        recv(tcpSocket, receiveBuffer, sizeof(receiveBuffer), 0);
        printf("TCP data rcvd: %s", receiveBuffer);

        if(strcmp("quit\n", sendBuffer) == 0){
            close(tcpSocket);
            done = true;
        }
        memset(sendBuffer, 0, sizeof(sendBuffer));
        memset(receiveBuffer, 0, sizeof(receiveBuffer));
    }

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