#include <unistd.h> //unix standard for POSIX API: read, write
#include <stdlib.h> //for exit()
#include <string.h> //for bzero()
#include <arpa/inet.h> //<netinet/in.h>
#include <stdio.h> //for FILE
void error(const char *);

void SendImage(FILE* img, int img_size, int to, int n) {
    char buf[80];
    strcpy(buf, "200 OK\0");
    write(to, buf, sizeof(buf));
    printf("Send image %d 'size': %d\n", n, img_size);
    write(to, &img_size, sizeof(img_size));
    printf("Send image %d 'data'\n", n);
    int count = 0;
    while (!feof(img)) {
        n = fread(buf, 1, sizeof(buf), img);
        write(to, buf, n);
        count += n;
        printf("%.1f%%\r", (double)(count) / img_size * 100);
    }
    puts("\ndone!");
}

int main(int argc, char *argv[])
{
    if (argc != 4)
        error("wrong argument size");

    int sockfd;
    struct sockaddr_in serv_addr;
    char buf[80];

    //[BEGIN] Read_images_size
    FILE* img1 = fopen(argv[2], "r");
    if (img1 < 0)
        error("image 1 open error");
    fseek(img1, 0, SEEK_END);
    int img1_size = ftell(img1);
    fseek(img1, 0, SEEK_SET);

    FILE* img2 = fopen(argv[3], "r");
    if (img2 < 0)
        error("image 2 open error");
    fseek(img2, 0, SEEK_END);
    int img2_size = ftell(img2);
    fseek(img2, 0, SEEK_SET);
    //[END] Read_images_size

    //[BEGIN] Bind_socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("creating socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("binding socket");
    //[END] Bind_socket

    puts("listen...");
    listen(sockfd, 5);

    while (1) {
        int newsockfd, n;
        socklen_t clilen;
        struct sockaddr_in cli_addr;

        clilen =  sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("accepting");
        puts("accept!");
        n = read(newsockfd, buf, sizeof(buf));
        puts("A connection has been established");

        //[BEGIN] Response
        n = atoi(buf);
        if (n == 1)
            SendImage(img1, img1_size, newsockfd, n);
        else if (n == 2)
            SendImage(img2, img2_size, newsockfd, n);
        else {
            strcpy(buf, "400 Bad Request\0");
            write(newsockfd, buf, sizeof(buf));
            puts("400 Bad Request\ndone!");
        }
        //[END] Response

        close(newsockfd);
    }

    close(sockfd);
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
