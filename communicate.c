/*
 * communicate.c
 * Fang (Daisy) Tang @ 2009
 */

#include "communicate.h"

struct sockaddr_in listen_addr[2];
struct sockaddr_in send_addr[2];
struct sockaddr_in broadcast_addr[2];

/*
 * converts an integer base 10 to ascii (string)
 */
void itoa(int n, char s[])
{
    short i,j,k;
    int c,sign;

    if((sign=n)<0) n=-n;
    i = 0;
    do{ s[i++] = n % 10 + '0';
    } while((n/=10)>0);

    if(sign<0) s[i++] = '-';
    s[i] = '\0';
    k = i-1;
    for(j=0; j<k; j++) {
        c = s[j];
        s[j] = s[k];
        s[k] = c;
        k--;
    }
}

/*
 * Create a UDP socket to listen to any incoming message
 */
int create_listen(int portnum, int index)
{
    int fd;
    int yes = 1;


    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }


    // to avoid the "address already in use" error
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	perror("setsockopt");
	exit(1);
    }

    // non-blocking?
    fcntl(fd, F_SETFL, O_NONBLOCK);



    listen_addr[index].sin_family = AF_INET;
    listen_addr[index].sin_port = htons(portnum);
    listen_addr[index].sin_addr.s_addr = INADDR_ANY;
    memset(&(listen_addr[index].sin_zero), '\0', 8);

//	printf("test from communicate.c (create_listen) \n");


    if (bind(fd, (struct sockaddr *) &listen_addr[index], sizeof(struct sockaddr)) == -1) {
	perror("bind");
	exit(1);
    }

//	printf("test #2 from communicate.c (create_listen) \n");

    return fd;
}	

/*
 * Create a UDP socket to send messages to others
 */
int create_send(char *host, int portnum, int index)
{
    int fd;
    int yes = 1;
    struct hostent *hn;

    if ((hn = gethostbyname(host)) == NULL) {
	perror("gethostbyname");
	exit(1);
    }

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // to avoid the "address already in use" error
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // non-blocking?
    //fcntl(fd, F_SETFL, O_NONBLOCK);

    send_addr[index].sin_family = AF_INET;
    send_addr[index].sin_port = htons(portnum);
    send_addr[index].sin_addr = *((struct in_addr *)hn->h_addr);
    memset(&(send_addr[index].sin_zero), '\0', 8);

    return fd;
}


/*
 * Create a UDP socket to broadcast messages to others
 */
int create_broadcast(int portnum, int index)
{
    char ip_addr[16];
    int fd;
    int yes = 1;
    struct hostent *hn;
//	printf("test from communicate.c (create_broadcast) \n");
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // broadcast
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // to avoid the "address already in use" error
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

//    sprintf(ip_addr, "134.71.185.255"); // the broadcast address, customize here
    sprintf(ip_addr, "127.0.0.0"); // the broadcast address, customize here
    broadcast_addr[index].sin_addr.s_addr = inet_addr(ip_addr);

    broadcast_addr[index].sin_family = AF_INET;
    broadcast_addr[index].sin_port = htons(portnum);
    memset(&(broadcast_addr[index].sin_zero), '\0', 8);
		
		
    return fd;
}


/*
 * listen_to_robot: receive messages
 */
int listen_to_robot(int sockfd, char *msg)
{
    struct sockaddr_in their_addr; // connector's address information
    int nbytes; // number of bytes
    socklen_t addr_len; 

    addr_len = sizeof(struct sockaddr);
    /*if ((nbytes = recvfrom(sockfd, msg, MAXBUF-1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
	perror("listen_to_robot:recvfrom");
	exit(1);
    }*/
    nbytes = recvfrom(sockfd, msg, MAXBUF-1, 0, (struct sockaddr *) &their_addr, &addr_len);
    //printf("got packet from %s\n", inet_ntoa(their_addr.sin_addr));
    if (nbytes < 0) nbytes = 0;
    msg[nbytes] = '\0';
    //printf("%d bytes: \"%s\"\n", nbytes, msg);

    if (nbytes > 0) return nbytes; 
    return 0;
}

/*
 * broadcasting a message
 */
int talk_to_all(int sockfd, char *msg, int index)
{
    int nbytes;
    int addr_len;

    addr_len = sizeof(struct sockaddr);

    if ((nbytes = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr[index], addr_len)) == -1) {
	perror("sendto");
	exit(1);
    }
//    printf("broadcast %s (%d bytes) n", msg, nbytes);

    if (nbytes > 0)
	return nbytes;
    return 0;
}

/*
 * send message to a robot
 */
int talk_to_one(int sockfd, char *msg, int index)
{
    int nbytes;
    char ip_addr[16];
    int addr_len;

    addr_len = sizeof(struct sockaddr);

    if ((nbytes = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&send_addr[index], addr_len)) == -1) {
        perror("sendto");
        exit(1);
    }
    //printf("sent %s (%d bytes) to %s\n", msg, nbytes, inet_ntoa(send_addr[index].sin_addr));

    if (nbytes > 0)
        return nbytes;
    return 0;
}
