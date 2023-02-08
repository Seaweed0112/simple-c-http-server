/*
 * vodserver.c - A simple connection-based vod server
 * usage: vodserver <port>
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFSIZE 1024

#if 0
/* 
 * Structs exported from netinet/in.h (for easy reference)
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    int listenfd;                  /* listening socket */
    int connfd;                    /* connection socket */
    int portno;                    /* port to listen on */
    int clientlen;                 /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp;         /* client host info */
    char buf[BUFSIZE];             /* message buffer */
    char *hostaddrp;               /* dotted decimal host addr string */
    int optval;                    /* flag value for setsockopt */
    int n;                         /* message byte size */
    FILE *file;
    int file_size;
    char send_buffer[10240], read_buffer[256];
    int read_size, packet_index;
    char *file_contents;
    packet_index = 1;

    /* check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);

    /* socket: create a socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval, sizeof(int));

    /* build the server's internet address */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;                     /* we are using the Internet */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);      /* accept reqs to any IP addr */
    serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

    /* bind: associate the listening socket with a port */
    if (bind(listenfd, (struct sockaddr *)&serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /* listen: make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 500) < 0) /* allow 500 requests to queue up */
        error("ERROR on listen");

    /* main loop: wait for a connection request, echo input line,
       then close connection. */
    clientlen = sizeof(clientaddr);
    while (1) {
        /* accept: wait for a connection request */
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        if (connfd < 0)
            error("ERROR on accept");

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(0);
        }

        if (pid == 0) {
            // Child Process
            close(listenfd);
            /* gethostbyaddr: determine who sent the message */
            hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                                  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
            if (hostp == NULL)
                error("ERROR on gethostbyaddr");
            hostaddrp = inet_ntoa(clientaddr.sin_addr);
            if (hostaddrp == NULL)
                error("ERROR on inet_ntoa\n");
            printf("server established connection with %s (%s)\n",
                   hostp->h_name, hostaddrp);

            /* read: read input string from the client */
            bzero(buf, BUFSIZE);
            n = read(connfd, buf, BUFSIZE);
            printf("server received %d bytes:\n%s", n, buf);
            // Check for "GET" request
            if (strncmp(buf, "GET", 3) == 0) {
                char method[10];
                char url[20];
                char protocol[10];
                sscanf(buf, "%s %s %s", method, url, protocol);
                // printf("Method: %s\n", method);
                // printf("URL: %s\n", url);
                // printf("Protocol: %s\n", protocol);
                char file_path[30];
                sprintf(file_path, "content%s", url);
                // puts(file_path);

                // Date header
                time_t currentTime;
                time(&currentTime);
                struct tm *currentTimeGMT;
                currentTimeGMT = gmtime(&currentTime);
                char dateTimeString[32];
                strftime(dateTimeString, 80, "%a, %d %b %Y %H:%M:%S GMT", currentTimeGMT);

                file = fopen(file_path, "rb");
                if (file == NULL || strcmp(file_path, "content/") == 0) {
                    char html[200];
                    sprintf(html, "HTTP/1.1 404 Not Found\nDate: %s\nContent-Type: text/html\n\n<html><body><h1>404 Not Found</h1></body></html>\r\n", dateTimeString);
                    n = write(connfd, html, strlen(html));
                    close(connfd);
                    exit(0);
                }

                char contentType[30];
                // video
                if (strstr(file_path, ".ogg") != NULL) {
                    sprintf(contentType, "video/ogg");
                } else if (strstr(file_path, ".mp4") != NULL) {
                    sprintf(contentType, "video/mp4");
                } else if (strstr(file_path, ".webm") != NULL) {
                    sprintf(contentType, "video/webm");
                }
                // image
                else if (strstr(file_path, ".jpeg") != NULL || strstr(file_path, ".jpg") != NULL) {
                    sprintf(contentType, "image/jpeg");
                } else if (strstr(file_path, ".gif") != NULL) {
                    sprintf(contentType, "image/gif");
                } else if (strstr(file_path, ".png") != NULL) {
                    sprintf(contentType, "image/png");
                }
                // text
                else if (strstr(file_path, ".txt") != NULL) {
                    sprintf(contentType, "text/plain");
                } else if (strstr(file_path, ".html") != NULL || strstr(file_path, ".htm") != NULL) {
                    sprintf(contentType, "text/html");
                } else if (strstr(file_path, ".css") != NULL) {
                    sprintf(contentType, "text/css");
                }
                // application
                else if (strstr(file_path, ".js") != NULL) {
                    sprintf(contentType, "application/javascript");
                } else {
                    sprintf(contentType, "application/octet-stream");
                }

                // Last-Modified header
                struct stat fileStat;
                if (stat(file_path, &fileStat) < 0) {
                    perror("Error: file stat failed");
                    char *response = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\n\nError: stat failed\n";
                    write(connfd, response, strlen(response));
                    close(connfd);
                    exit(1);
                }
                struct tm *lastModifiedTime;
                char lastModifiedTimeString[32];
                lastModifiedTime = gmtime(&fileStat.st_mtime);
                strftime(lastModifiedTimeString, 32, "%a, %d %b %Y %H:%M:%S GMT", lastModifiedTime);

                fseek(file, 0, SEEK_END);
                file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                // Check of User-Agent" header
                char *user_agent = strstr(buf, "User-Agent: ");
                int isBrowser = 0;
                if (user_agent != NULL) {
                    user_agent += strlen("User-Agent: ");
                    if (strstr(user_agent, "Mozilla") != NULL || strstr(user_agent, "Chrome") != NULL || strstr(user_agent, "Safari") != NULL) {
                        isBrowser = 1;
                    }
                }
                // Check for "Range" header
                char *range = strstr(buf, "Range:");
                // Only allow curl to make a Range request
                if (range != NULL && isBrowser != 1) {
                    range += 7;  // skip "Range: "
                    int start, end;
                    sscanf(range, "bytes=%d-%d", &start, &end);
                    end = end > 0 ? end : file_size - 1;
                    int partial_file_size = end - start + 1;
                    fseek(file, start, SEEK_SET);

                    file_contents = malloc((partial_file_size + 1) * sizeof(*file_contents));
                    if (file_contents == NULL) {
                        // Generate "500 Internal Server Error" response
                        char *response = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\n\nError: Failed to allocate memory for file buffer.\n";
                        write(connfd, response, strlen(response));
                        close(connfd);
                        exit(1);
                    }
                    fread(file_contents, partial_file_size, 1, file);
                    file_contents[partial_file_size] = '\0';

                    char response[300];
                    sprintf(response, "HTTP/1.1 206 Partial Content\nLast-Modified: %s\nConnection: close\nContent-Type: %s\nAccept-Ranges: bytes\nContent-Range: bytes %d-%d/%d\nDate: %s\nContent-Length: %d\n\n%s", lastModifiedTimeString, contentType, start, end, file_size, dateTimeString, file_size, file_contents);
                    send(connfd, response, strlen(response), 0);

                } else {
                    char response[300];
                    // printf("200 OK\n");
                    sprintf(response, "HTTP/1.1 200 OK\nLast-Modified: %s\nConnection: close\nContent-Type: %s\nAccept-Ranges: bytes\nDate: %s\nContent-Length: %d\n\n", lastModifiedTimeString, contentType, dateTimeString, file_size);
                    n = write(connfd, response, strlen(response));
                    while (!feof(file)) {
                        // Read from the file into our send buffer
                        read_size = fread(send_buffer, 1, sizeof(send_buffer) - 1, file);

                        // Send data through our socket
                        n = write(connfd, send_buffer, read_size);

                        // printf("Packet Number: %i\n", packet_index);
                        // printf("Packet Size Sent: %i\n", read_size);
                        // packet_index++;

                        // Zero out our send buffer
                        bzero(send_buffer, sizeof(send_buffer));
                    }
                }
                fclose(file);
            }

            close(connfd);
            exit(0);
        } else if (pid > 0) {
            close(connfd);
        }
    }
    close(listenfd);
}