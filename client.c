#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    /************************************************************************************/
    /*Read command line Arguments                                                        /
    /------------------------------------------------------------------------------------/
    /*the 0th argument is the path to the exectuable so indexing technically starts at 1 /
    /*All the arguments are stored in character arrays which are essentially strings     /
    /************************************************************************************/
    char* udpServerHostName = argv[1]; //reads the first argument from command line which is the host name of the udp server
    char* udpServerPort = argv[2];     //reads the second argument from command line which is the host port number
    char* name = argv[3];              //reads the third argument from command line which is the name of the user

    /************************************************************************************************************** /
    / Create addrinfo structs to pass to network functions                                                          /
    /---------------------------------------------------------------------------------------------------------------/
    / These structs are a recent addition to C and contain info about the connections that passed to the functions  /
    / since there were specifically designed for C's netowrking functions, they make it very easy to use netowrking /
    /***************************************************************************************************************/
    //this struct is basically a hint at what kind of connection you want to be using to send data
    struct addrinfo udpConnectionInfo; 
    //this struct will hold info about the servers connection and is a linked list of structs which is why itls a pointer
    struct addrinfo *udpServerInfo;    
    
    /*************************************************************************************************************** /
    / Initialize connectionInfo struct                                                                               /
    /----------------------------------------------------------------------------------------------------------------/
    / This struct needs to be initialized so that the getaddrinfo function knows how to initially contact the server /
    / Everything else is set to 0 so that the C language will auto choose the defaults                               /
    /****************************************************************************************************************/
    memset(&udpConnectionInfo, 0, sizeof(udpConnectionInfo)); //memset sets every value in the struct to 0 
    //PF_INET, SOCK_DGRAM, and AI_PASSIVE are all constant ints provided by the socket.h header
    udpConnectionInfo.ai_family = PF_INET;      //first set the family to PF_INET, this indicates you want IPv4 connections to the socket
    udpConnectionInfo.ai_socktype = SOCK_DGRAM; //set socktype to SOCK_DGRAM, this means you want a UDP socket
    udpConnectionInfo.ai_flags = AI_PASSIVE;    //set flags to AI_PASSIVE, this means you will let C fill in the IP of your machine for you
    
    /********************************************************************************************************************************* /
    / Resolve Host Name                                                                                                                /
    /----------------------------------------------------------------------------------------------------------------------------------/
    / the getaddrinfo function will essentially do a DNS look up for you and change a hostname like www.google.com into an ip addresss /
    / that can be used by C to actually make a connection. It takes a hostname, port number, and the two structs that were created     /
    / earlier. These structs are then loaded with the correct data automatically by the function, and can then be passed to later      /
    / functions to connect and send/receive data.                                                                                      /
    /**********************************************************************************************************************************/
    //if the function doesn't return 0 then there was an error resolving the host
    if(getaddrinfo(udpServerHostName, udpServerPort, &udpConnectionInfo, &udpServerInfo) != 0){
        exit(-1);
    }

    /********************************************************************************************************************************** /
    / Create Socket                                                                                                                     /
    /-----------------------------------------------------------------------------------------------------------------------------------/
    / Here I need to create the socket where the UDP packets will be sent and receieved from                                            /              /
    / Since getaddrinfo filled out the udpServerInfo with the correct values that it received from the server, I can pass the server's  /
    / specifications to the sokcet and C will take care of everything for me. After creating the socket C then returns an integer that  /
    / describes the socket that is meant to be passed to the send and receive functions to exchange data                                /
    /***********************************************************************************************************************************/
    int UDPSocket = socket(udpServerInfo->ai_family, udpServerInfo->ai_socktype, udpServerInfo->ai_protocol);
    if(UDPSocket == -1){
        exit(-1);
    }

    /************************************************************************************************************************************** /
    / Send and Receive UDP data                                                                                                             /
    /---------------------------------------------------------------------------------------------------------------------------------------/
    / Since this is a UDP socket I do not need to call connect before I starting sending and receiving packets. This is because UDP packets /
    / are essentially a mail box where letters are being sent back and forth rather than a phone call where both parties need to agree to   /
    / talk. Additionally I did not need to call bind, becuae if you don't call it then C will automatically assign a port number for you.   /
    / I use the sendto() and recvfrom() functions, since they are the ones you need to use if you don't use connect. Because of this, I need/ 
    / to pass the server info each so the socket knows where to send the data. It is possible, even with a UDP socket to call connect first /
    / and then use the send() and recv() functions. What this will essentially do is fill out the server info for you so that you don't need/
    / to do it everytime like I did.                                                                                                        /
    /***************************************************************************************************************************************/
    char buffer[100]; //this is a buffer that the received data will be read into
    //This function will send the data given to it with UDP, flags set to 0 is default. I also pass the server info so it knows where to send
    sendto(UDPSocket, name, strlen(name), 0, udpServerInfo->ai_addr, udpServerInfo->ai_addrlen); 
    printf("UDP Data sent: %s\n", name);
    //This function will block until it receives data from the server given. The data will be read into the buffer passed to the function
    int bytesReceived = recvfrom(UDPSocket, buffer, sizeof(buffer), 0, udpServerInfo->ai_addr, &udpServerInfo->ai_addrlen); 
    printf("UDP Data rcvd: %s\n", buffer);

    /*************************************************************************************************************************** /
    / Find index of space between server name and port                                                                           /
    /----------------------------------------------------------------------------------------------------------------------------/
    /This loop just finds where the space is in the message that was given to me. I do this so I can allocate the correct amount /
    / of memory to hold the TCP Server Host name and TCP Server Port number. After finding the space I then go through the       /
    / message and read everyhting before the space as the host name, and everything after the space as the port number. I can    /
    / make these assumptions since that is what was given to us as part of the project.                                          /
    / bytesRecieved here is equal to the total number of characters the that was received in the UDP message. This is because    /
    / each character takes on byte of memory.                                                                                    /
    /****************************************************************************************************************************/
    int spaceIndex = -1; //int to hold index of where space is
    for(int i=0; i<bytesReceived; i++){
        //test for space
        if(buffer[i] == ' '){
            //if space set index and break out of loop
            spaceIndex = i;
            break; 
        }
    }

    //This line allocates exactly as much space as i need to hold the host name. It uses calloc, but malloc would work equally well.
    char* tcpServerHostName = (char *) calloc(spaceIndex, sizeof(char));
    //this simple loop just reads every character up to the space and stores it as the TCP Server Host Name
    for(int i=0; i<spaceIndex; i++){
        tcpServerHostName[i] = buffer[i];
    }

    //This line allocates exactly as much space as i need to hold the host name. It uses calloc, but malloc would work equally well.
    char* tcpServerPort = (char *) calloc(bytesReceived - spaceIndex, sizeof(char));
    //this simple loop just reads every character up to the space and stores it as the TCP Server Port Name
    for(int i=spaceIndex+1; i<bytesReceived; i++){
        tcpServerPort[i - (spaceIndex + 1)] = buffer[i];
    }

    /******************************************************************************************************************************* /   
    / Setup TCP Connection                                                                                                           /
    /--------------------------------------------------------------------------------------------------------------------------------/
    / In this next chunk of code I setup the TCP Connection the great thing about layers and abstraction is that this code is almost /
    / exactly the same as the UDP Portion. Because of that, after this comment, I will not go quite as in depth with my comments, but/
    / I will comment on the few small differences in creating and sending data using a TCP socket vs. doing it with a UDP socket.    /
    / Below I create the same structs as I did before but there is one small change                                                  /
    /********************************************************************************************************************************/
    struct addrinfo tcpConnectionInfo;
    struct addrinfo *tcpServerInfo;

    memset(&tcpConnectionInfo, 0, sizeof(tcpConnectionInfo));
    tcpConnectionInfo.ai_family = PF_INET;
    /*this time instead of setting the socktype to DGRAM it is set to SOCK_STREAM since we want a TCP connection, instead of UDPd. */
    tcpConnectionInfo.ai_socktype = SOCK_STREAM;
    tcpConnectionInfo.ai_flags = AI_PASSIVE;

    /*resolve host name in exactly the same way as with the UDP portion*/
    if(getaddrinfo(tcpServerHostName, tcpServerPort, &tcpConnectionInfo, &tcpServerInfo) != 0){
        exit(-1);
    }

    /*create socket in exactly the same way as the UDP portion. This time getaddrinfo() resolved that it was TCP protocol*/
    int tcpSocket = socket(tcpServerInfo->ai_family, tcpServerInfo->ai_socktype, tcpServerInfo->ai_protocol);
    if(tcpSocket == -1){
        exit(-1);
    }
    /***************************************************************************************************************************** /
    / Initiate TCP Connection                                                                                                      /
    /------------------------------------------------------------------------------------------------------------------------------/
    / This is one of the main differences between a TCP and UDP Connection. Here we call the connect() function which will open the/
    / the connection between the TCP socket created and the server. This absolutely has to be done before data can be exchanged    /
    / since TCP acts like a phone call. Similar to a phone call, no data can be excahgned before the line is open and connected    / 
    / The connect function just takes a socket discripter, the address of the server it wants to connect to, and the length of the /
    / address. The connect function will intiate the connection and return -1 if there was an error establishing connection.       /
    /******************************************************************************************************************************/
    if(connect(tcpSocket, tcpServerInfo->ai_addr, tcpServerInfo->ai_addrlen) == -1){
        exit(-1);
    }

    /******************************************************************************************************************************** /
    / Main Loop                                                                                                                       /
    /---------------------------------------------------------------------------------------------------------------------------------/
    / Below is the main loop of the code. This loop will is set up as a do/while loop since the first thing we want to do is send the /
    / name that was originally passed in the begginning of the program. After that is sent and the first response is received the user/
    / is asked to input another message to send. When they press enter the message is sent and an echo response is received. If the   /
    / user enters "quit" then the connection is closed. Below I will comment on the loop to show what is happening                    /
    /*********************************************************************************************************************************/
    bool done = false; //boolean to keep track of when the loop should exit
    char sendBuffer[512]; //send buffer holds the message that will be sent to the server
    char receiveBuffer[512]; //receive buffer holds the response from the server
    //this next line of code just copies the name passed into the program into the send buffer and adds a \n to keep formatting
    strcpy(sendBuffer, strcat(name,"\n"));
    int count = 0;
    //enter loop

    send(tcpSocket, sendBuffer, sizeof(sendBuffer), 0);
    printf("TCP data sent: %s", sendBuffer);

    recv(tcpSocket, receiveBuffer, sizeof(receiveBuffer), 0);
    printf("TCP data rcvd: %s", receiveBuffer);

    memset(sendBuffer, 0, sizeof(sendBuffer));
    memset(receiveBuffer, 0, sizeof(receiveBuffer));

    do{
        /*Send TCP message - Since connect()  was already already called I just need to pass the socket, pointer to the buffer, size and 0 */
        /*for the flag. The server info is already encapsulated in the socket descripter*/
        send(tcpSocket, sendBuffer, sizeof(sendBuffer), 0);
        printf("TCP data sent: %s", sendBuffer);
        /* Receive TCP message - Similar to above, since connect was called already I just need to pass socket, pointer to the buffer, size,   /
        /  and set 0 for the flags. Note: this receive is a blocking receive so if we never got a response from the server it would  be stuck  /
        /  here                                                                                                                               */
        recv(tcpSocket, receiveBuffer, sizeof(receiveBuffer), 0);
        printf("TCP data rcvd: (%d) Message is: %s", count, receiveBuffer);

        /* Here the buffers are cleared and set to 0 so messages don't get mixed together */
        memset(sendBuffer, 0, sizeof(sendBuffer));
        memset(receiveBuffer, 0, sizeof(receiveBuffer));

        /* This part asks the user what they want their message to be and reads the line into the send buffer */
        printf("Next message? ");
        fgets(sendBuffer, sizeof(sendBuffer), stdin);

        /*If the message is quit\n then the user wants to exit the program*/
        if(strcmp("quit\n", sendBuffer) == 0){
            //first the last messages is sent so the server knows to close
            send(tcpSocket, sendBuffer, sizeof(sendBuffer), 0);
            printf("TCP data sent: %s", sendBuffer);
            recv(tcpSocket, receiveBuffer, sizeof(receiveBuffer), 0);
            printf("TCP data rcvd: (%d) Message is: %s", count, receiveBuffer);

            /*then close() is called. close() prevents further read and writes going through the socket. This is true even for the server /
            / if they try to read or send data to the socket they will get an error.                                                     */
            close(tcpSocket);
            //lastly done is set to true so the loop will exit
            done = true;
        }
        count++;
    }while(!done);
        
    exit(1);
}