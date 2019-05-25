#include "csapp.h"

int port_check(char *str){
    while(*str)
    {
        if(isdigit(*str))
            str++;
        else
        {    
            return 0;
            fprintf(stderr, "\033[31m<port> must be a number!\n\033[0m");
        }
    }
    return 1;
}

void read_headers(rio_t *rio){
    char buf[MAXLINE];
    Rio_readlineb(rio, buf, MAXLINE);
    while (strcmp(buf, "\r\n")){
        Rio_readlineb(rio, buf, MAXLINE);
        printf("%s", buf);
    }
}

void parse_uri(char *uri, char *filename, char *root_dir){
    strcpy(filename, root_dir);
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')
        strcat(filename, "index.html");
}

void get_file_type(char *filename, char *file_type){
    if (strstr(filename, ".html"))
        strcpy(file_type, "text/html");
    else if (strstr(filename, ".jpg"))
        strcpy(file_type, "image/jpeg");
    else if (strstr(filename, ".ico"))
        strcpy(file_type, "image/x-icon");
    else if (strstr(filename, ".css"))
        strcpy(file_type, "text/css");
    else
        strcpy(file_type, "text/plain");
}

void server(int connect_fd, char *filename, int file_size){
    int src_fd;
    char *src, file_type[MAXLINE], buf[MAXBUF];
    get_file_type(filename, file_type);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Naive Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, file_size);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, file_type);
    Rio_writen(connect_fd, buf, strlen(buf));
    src_fd = Open(filename, O_RDONLY, 0);
    src = Mmap(0, file_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    Close(src_fd);
    Rio_writen(connect_fd, src, file_size);
    Munmap(src, file_size);
}

static int connect_fd;

void SigHandler(int signum)
{
    if(fcntl(connect_fd, F_GETFD) != -1)
        Close(connect_fd);
    exit(0);
}

struct sigaction act, oldact;

void parse(int connect_fd, char *root_dir){
    rio_t rio;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], filename[MAXLINE];
    Rio_readinitb(&rio, connect_fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    read_headers(&rio);
    parse_uri(uri, filename, root_dir);
    if (stat(filename, &sbuf) < 0){
        fprintf(stderr, "\033[31m404: Cannot find this file.\n\033[0m");
    return;
    }
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
        fprintf(stderr, "\033[31m403: Forbidden\n\033[0m");
        return;
    }
    server(connect_fd, filename, sbuf.st_size);
}

int main(int argc, char *argv[]) {
    /*For graceful shutdown*/
    act.sa_handler = SigHandler;
    act.sa_flags = SA_NODEFER;
    sigaction(SIGINT, &act, &oldact);
    /*End Signal Handler*/
    char root_dir[MAXLINE];
    int flag = 0; 
    if ((argc == 3 || argc == 4) && 
        (strcmp(argv[1], "-p") == 0 ||
        strcmp(argv[1], "--port") == 0) &&
        port_check(argv[2]))
        flag = 1;
    if (argc == 3)
        strcpy(root_dir, ".");
    else
        strcpy(root_dir, argv[3]);
    if(flag)
    {
        int listen_fd, client_len;
        struct sockaddr_in client_addr;
        listen_fd = open_listenfd(argv[2]);
        while (1){
            client_len = sizeof(client_addr);
            connect_fd = Accept(listen_fd, (SA*) &client_addr, (socklen_t*)&client_len);
            parse(connect_fd, root_dir);
            Close(connect_fd);
        }
    }
    else
        fprintf(stderr, "\033[31mUsage: ./httpd --port/-p <port> <dir>\n\033[0m");
    return 0;
}
