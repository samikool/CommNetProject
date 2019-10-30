#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

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

    struct addrinfo connectionInfo;
    struct addrinfo *serverInfo;

    memset(&connectionInfo, 0, sizeof(connectionInfo));

    connectionInfo.ai_family = PF_INET;
    connectionInfo.ai_socktype = SOCK_DGRAM;
    connectionInfo.ai_flags = AI_PASSIVE;

    if(getaddrinfo(UDPServerHostName, UDPServerPort, &connectionInfo, &serverInfo) != 0){
        exit(-1);
    }

    printIP(serverInfo);

    int UDPSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if(UDPSocket == -1){
        exit(-1);
    }

    /*Begin sending/receiving UDP Portion*/
    int status = 4;
    status = connect(UDPSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
    printf("%d\n", status);
    status = 4;

    char buffer[100];

    status = send(UDPSocket, name, strlen(name), 0);
    printf("%d\n", status);
    status = 4;

    status = recv(UDPSocket, buffer, sizeof(buffer), 0);
    printf("%d\n", status);
    status = 4;




    /*
    char buffer[100];

    status = sendto(UDPSocket, name, strlen(name), 0, serverInfo->ai_addr, serverInfo->ai_addrlen);
    printf("%d\n", status);
    status = recvfrom(UDPSocket, buffer, sizeof(buffer), 0, serverInfo->ai_addr, &serverInfo->ai_addrlen);
    printf("%d\n", status);
     */
    
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