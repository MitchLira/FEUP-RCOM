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
#include <fcntl.h>
#include <sys/stat.h>


#define FTP_PORT        21
#define BUF_SIZE        2048
#define STR_MAX_SIZE    4096


typedef unsigned int ui;

typedef struct {
    char string[STR_MAX_SIZE];
    ui length;
} String;

typedef struct {
    String user;
    String password;
    String hostname;
    String path;
} ClientInfo;


const String USER_PREFIX = { "USER ", sizeof("USER ") };
const String PASS_PREFIX = { "PASS ", sizeof("PASS ") };
const String PASV_PREFIX = { "PASV", sizeof("PASV") };
const String RETR_PREFIX = { "RETR ", sizeof("RETR ") };
const String NULL_STR = { "", 0 };



char* getIP(char *hostname);
int initTCP(char *address, ui port);
int receiveFromServer(int sockfd, char *msg, ui size);
void ftp_login(int sockfd, String username, String password);
ui ftp_passive_mode(int sockfd, char *address);
int ftp_change_path(int sockfd, String path);
int ftp_download(int sockfd, ClientInfo info);
String buildMessage(String prefix, String value);
ClientInfo parseURL(char *url);
void readData(int datafd);


int main(int argc, char** argv) {
    char *address;
    char response[BUF_SIZE];
    int sockfd;
    ClientInfo info;

    info = parseURL(argv[1]);
    printf("\n\n");
    printf("User - %s\n", info.user.string);
    printf("Password - %s\n", info.password.string);
    printf("Hostname - %s\n", info.hostname.string);
    printf("Url-path - %s\n", info.path.string);



    if ( (address = getIP(info.hostname.string)) == NULL ) {
        exit(-1);
    }

    printf("\n%s = %s.\n", info.hostname.string, address);

   if ( (sockfd = initTCP(address, FTP_PORT)) == -1) {
        exit(-2);
    }

    printf("Connected to %s.\n", info.hostname.string);


    receiveFromServer(sockfd, response, BUF_SIZE);
    printf("\n%s", response);

    ftp_login(sockfd, info.user, info.password);
    ftp_download(sockfd, info);

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


ui ftp_passive_mode(int sockfd, char *address) {
    char response[BUF_SIZE];
    int a1, a2, a3, a4;
    int p1, p2;
    ui port;

    String pasv = buildMessage(PASV_PREFIX, NULL_STR);

    if (write(sockfd, pasv.string, pasv.length) < 0) {
        exit(-1);
    }

    receiveFromServer(sockfd, response, BUF_SIZE);
    printf("\n%s", response);

    sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", &a1, &a2, &a3, &a4, &p1, &p2);

    sprintf(address, "%d.%d.%d.%d", a1, a2, a3, a4);
    port = 256 * p1 + p2;

    return port;
}



int ftp_retrieve(int sockfd, String path) {
    char response[BUF_SIZE];

    String cmdPath = buildMessage(RETR_PREFIX, path);
    printf("%s\n", cmdPath.string);

    if (write(sockfd, cmdPath.string, cmdPath.length) < 0) {
      exit(-1);
    }

    receiveFromServer(sockfd, response, BUF_SIZE);
    printf("%s", response);

    return 0;
}





int ftp_download(int sockfd, ClientInfo info) {
    ui port;
    int datafd;
    char address[BUF_SIZE];

    port = ftp_passive_mode(sockfd, address);
    printf("IP = %s\n", address);
    printf("Port = %d\n", port);

    if ( (datafd = initTCP(address, port)) == -1) {
         exit(-1);
    }

    ftp_retrieve(sockfd, info.path);
    readData(datafd);

    return 0;
}


String buildMessage(String prefix, String value) {
    String msg;

    memcpy(msg.string, prefix.string, prefix.length);
    memcpy(msg.string + prefix.length -1, value.string, value.length);

    ui size = prefix.length + value.length -1;
    msg.string[size++] = '\n';
    msg.length = size;

    return msg;
}




ClientInfo parseURL(char *url) {
    ClientInfo info;
    char *token;

    token = strtok(&url[6], ":");
    strcpy(info.user.string, token);

    info.user.length = strlen(info.user.string);
    token = strtok(NULL, "@");

    strcpy(info.password.string, token);
    info.password.length = strlen(info.password.string);

    token = strtok(NULL, "/");
    strcpy(info.hostname.string, token);
    info.hostname.length = strlen(info.hostname.string);

    token = strtok(NULL, "");
    strcpy(info.path.string, token);
    info.path.length = strlen(info.path.string);

    return info;
}


void readData(int datafd) {
    int filefd, bytes;
    char buffer[BUF_SIZE];

    filefd = open("data", O_CREAT | O_WRONLY | O_TRUNC | S_IRWXU);
    if (filefd < 0) {
    }

    while ( (bytes = read(datafd, buffer, BUF_SIZE)) > 0 ) {
        write(filefd, buffer, bytes);
    }
}
