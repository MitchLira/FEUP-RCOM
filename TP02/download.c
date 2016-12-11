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
    String filename;
} ClientInfo;


const String USER_PREFIX = { "USER ", sizeof("USER ") };
const String PASS_PREFIX = { "PASS ", sizeof("PASS ") };
const String PASV_PREFIX = { "PASV", sizeof("PASV") };
const String RETR_PREFIX = { "RETR ", sizeof("RETR ") };
const String NULL_STR = { "", 0 };



char* getIP(char *hostname);
int initTCP(char *address, ui port);
int receiveFromServer(int sockfd, char *msg, ui size);
int ftp_login(int sockfd, String username, String password);
ui ftp_passive_mode(int sockfd, char *address);
int ftp_change_path(int sockfd, String path);
int ftp_download(int sockfd, ClientInfo info);
String buildMessage(String prefix, String value);
int parseURL(ClientInfo *info, char *url);
int readData(int datafd, String filename);


int main(int argc, char** argv) {
    char *address;
    char response[BUF_SIZE];
    int sockfd;
    ClientInfo info;

    if (parseURL(&info, argv[1]) < 0) {
        exit(-1);
    }

    printf("\n");
    printf("User - %s\n", info.user.string);
    printf("Password - %s\n", info.password.string);
    printf("Hostname - %s\n", info.hostname.string);
    printf("Url-path - %s\n", info.path.string);
    printf("Filename - %s\n", info.filename.string);


    if ( (address = getIP(info.hostname.string)) == NULL ) {
        exit(-1);
    }

    printf("\n%s's address = %s.\n\n", info.hostname.string, address);

    printf("Connecting to server...\n");
    if ( (sockfd = initTCP(address, FTP_PORT)) == -1) {
        exit(-2);
    }

    if (receiveFromServer(sockfd, response, BUF_SIZE) < 0) {
        exit(-1);
    }
 
    printf("%s\n", response);    
    if (response[0] == '5') {
	exit(-1);
    }

    if (ftp_login(sockfd, info.user, info.password)) {
        exit(-1);
    }

    return ftp_download(sockfd, info);
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
    else if (bytes == -1) {
        perror("recv");
    }

    return bytes;
}



int ftp_login(int sockfd, String username, String password) {
    char response[BUF_SIZE];

    String user = buildMessage(USER_PREFIX, username);

    printf("Sending user...\n");
    if (write(sockfd, user.string, user.length) < 0) {
        perror("sending user");
        exit(-1);
    }

	if (receiveFromServer(sockfd, response, BUF_SIZE) < 0) {
        exit(-1);
    }

    printf("%s\n", response);
    if (response[0] == '5') {
	exit(-1);
    }

    String pass = buildMessage(PASS_PREFIX, password);

    printf("Sending password...\n");
    if (write(sockfd, pass.string, pass.length) < 0) {
        perror("sending password");
        exit(-1);
    }

    receiveFromServer(sockfd, response, BUF_SIZE);
    printf("%s\n", response);
    if (response[0] == '5') {
	exit(-1);
    }

    return 0;
}


ui ftp_passive_mode(int sockfd, char *address) {
    char response[BUF_SIZE];
    int a1, a2, a3, a4;
    int p1, p2;
    ui port;

    printf("Entering passive mode...\n");
    String pasv = buildMessage(PASV_PREFIX, NULL_STR);

    if (write(sockfd, pasv.string, pasv.length) < 0) {
        perror("passive mode");
        exit(0);
    }

    if (receiveFromServer(sockfd, response, BUF_SIZE) < 0) {
        exit(0);
    }

    printf("%s", response);
    if (response[0] == '5') {
	exit(-1);
    }

    sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", &a1, &a2, &a3, &a4, &p1, &p2);
    sprintf(address, "%d.%d.%d.%d", a1, a2, a3, a4);
    port = 256 * p1 + p2;

    return port;
}



int ftp_retrieve(int sockfd, String path) {
    char response[BUF_SIZE];

    String cmdPath = buildMessage(RETR_PREFIX, path);
    
    printf("Retrieving file...\n");
    if (write(sockfd, cmdPath.string, cmdPath.length) < 0) {
      perror("path");
      exit(-1);
    }

    if (receiveFromServer(sockfd, response, BUF_SIZE) < 0) {
        exit(-1);
    }

    printf("%s", response);
    if (response[0] == '5') {
	exit(-1);
    }

    return 0;
}



int ftp_download(int sockfd, ClientInfo info) {
    ui port;
    int datafd;
    char address[BUF_SIZE];

    port = ftp_passive_mode(sockfd, address);
    printf("IP = %s\n", address);
    printf("Port = %d\n\n", port);

    if ( (datafd = initTCP(address, port)) == -1) {
         exit(-1);
    }

    if (ftp_retrieve(sockfd, info.path) < 0) {
        exit(-1);
    }
    
    return readData(datafd, info.filename);
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




int parseURL(ClientInfo *info, char *url) {
    int n;
    char *token;
    char *last;
    char str[BUF_SIZE];

    n = sscanf(url, "ftp://%[^:]:%[^@]@%[^/]/%s", 
                info->user.string, 
                info->password.string,
                info->hostname.string,
                info->path.string);
    
    if (n != 4) {
        n = sscanf(url, "ftp://%[^/]/%s", 
                    info->hostname.string,
                    info->path.string);

        if (n != 2) {
            printf("Error: wrong URL syntax.\n");
            exit(-1);
        }

        strcpy(info->user.string, "anonymous");
        strcpy(info->password.string, "mail@domain");
    }

    strcpy(str, info->path.string);
    token = strtok(str, "/");
    last = token;
    while ( (token = strtok(NULL, "/")) != NULL ) {
        last = token;
    }

    strcpy(info->filename.string, last);

    info->user.length =     strlen(info->user.string);
    info->password.length = strlen(info->password.string);
    info->hostname.length = strlen(info->hostname.string);
    info->path.length =     strlen(info->path.string);
    info->filename.length = strlen(info->filename.string);

    return 0;
}


int readData(int datafd, String filename) {
    int filefd, bytes;
    char buffer[BUF_SIZE];

    filefd = open(filename.string, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    if (filefd < 0) {
        perror(filename.string);
        exit(-1);
    }

    while ( (bytes = read(datafd, buffer, BUF_SIZE)) > 0 ) {
        if (bytes == -1) {
            perror("read file");
            exit(-1);
        }

        if (write(filefd, buffer, bytes) < 0) {
            perror("write to file");
            exit(-1);
        }
    }

    return 0;
}
