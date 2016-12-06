#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>


#define FTP_PORT        21
#define BUF_SIZE        2048
#define STR_MAX_SIZE    4096

typedef unsigned int ui;
typedef struct {
    char string[STR_MAX_SIZE];
    ui length;
} String;


const String USER_PREFIX = { "USER ", sizeof("USER ") };
const String PASS_PREFIX = { "PASS ", sizeof("PASS ") };


char* getIP(char *hostname);
int initTCP(char *address, ui port);
int receiveFromServer(int sockfd, char *msg, ui size);
void ftp_login(int sockfd, String username, String password);
String buildMessage(String prefix, String value);

int main(int argc, char** argv) {
    char *address;
    char response[BUF_SIZE];
    int sockfd;

    if ( (address = getIP(argv[1])) == NULL ) {
        exit(-1);
    }

    printf("\n%s = %s.\n", argv[1], address);

   if ( (sockfd = initTCP(address, FTP_PORT)) == -1) {
        exit(-2);
    }

    printf("Connected to %s.\n", argv[1]);


    receiveFromServer(sockfd, response, BUF_SIZE);
    printf("\n%s", response);


    String user;
    strcpy(user.string, "");
    user.length = sizeof("");
    
    


    String password;
    strcpy(password.string, "");
    password.length = sizeof("");

    ftp_login(sockfd, user, password);
    
    return 0;
}


char* getIP(char *hostname) {
    struct hostent *h;

    h = gethostbyname(hostname);
    if (h == NULL) {
        herror("gethostbyname");
        return NULL;
    }
            
    return inet_ntoa(*((struct in_addr *)h->h_addr));
}


int initTCP(char *address, ui port) {
    int	sockfd;
    struct	sockaddr_in server_addr;

    /*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		        /*server TCP port must be network byte ordered */

    /*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        exit(-1);
    }

	/*connect to the server*/
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        perror("connect()");
		exit(-1);
	}

    return sockfd;
}


int receiveFromServer(int sockfd, char *msg, ui size) {
    int bytes;

    bytes = recv(sockfd, msg, size, 0);
    if (bytes != size) {
        msg[bytes] = '\0';
    }

    return bytes;
}



void ftp_login(int sockfd, String username, String password) {
    char response[BUF_SIZE];

    String user = buildMessage(USER_PREFIX, username);
    
    if (write(sockfd, user.string, user.length) < 0) {
        exit(-1);
    }

	receiveFromServer(sockfd, response, BUF_SIZE);
    printf("\n%s", response);
    
    String pass = buildMessage(PASS_PREFIX, password);

    if (write(sockfd, pass.string, pass.length) < 0) {
        exit(-1);
    }
    
    receiveFromServer(sockfd, response, BUF_SIZE);
    printf("\n%s", response);
}



String buildMessage(String prefix, String value) {
    String msg;

    memcpy(msg.string, prefix.string, prefix.length);
    memcpy(msg.string + prefix.length - 1, value.string, value.length);

    ui size = prefix.length + value.length - 2;
    msg.string[size++] = '\n';
    msg.length = size;

    return msg;
}
