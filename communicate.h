/*
 * communicate.h
 */

#ifndef COMM_H
#define COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>           
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <values.h>
#include <errno.h>
#include <limits.h>

#define PORT_H 4950 // the port for the help thread 4950
#define PORT_R 4951 // the port for the request thread 4951
#define MAXBUF 1024 // maximum length of the buffer
#define TIMEOUT 0  // 5 seconds

#define H 0 // used in help
#define R 1 // used in request

/*
struct sockaddr_in listen_addr[2]; 
struct sockaddr_in send_addr[2];
struct sockaddr_in broadcast_addr[2];
*/

// converts an integer base 10 to ascii (string)
void itoa(int n, char s[]);

// Create a UDP socket for sending messages
int create_listen(int, int);
int create_send(char *, int, int);
int create_broadcast(int, int);

// Receive a message
int listen_to_robot(int sockfd, char *msg);

// Broadcast a message
int talk_to_all(int sockfd, char *msg, int);

// Send a message to a robot with "id"
int talk_to_one(int sockfd, char *msg, int);

#endif
