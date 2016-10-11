#include <unistd.h> //unix standard for POSIX API: read, write
#include <stdlib.h> //for exit()
#include <string.h> //for bzero()
#include <arpa/inet.h> //<netinet/in.h>
#include <stdio.h> //for FILE
void error(const char *);

int main(int argc, char *argv[])
{
    if (argc != 5)
        error("wrong argument size");

    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buf[82];

    //[BEGIN] Connect_server
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Creating socket");
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Connecting");
    puts("connect!");
    //[END] Connect_server

    printf("Send your message: image num: %s\n", argv[3]);
    write(sockfd, argv[3], sizeof(argv[3]));

    //[BEGIN] Receive
    n = read(sockfd, buf, 80);
    if (strcmp(buf, "200 OK\0") == 0) {
        int size, count = 0;
        read(sockfd, &size, sizeof(int));
        printf("Image size: %d\n", size);

        char img_data[size];
        printf("Save image to %s\n", argv[4]);
        FILE* img = fopen(argv[4], "w");

        while (n) {
            n = read(sockfd, img_data, size);
            fwrite(img_data, 1, n, img);
            count += n;
            printf("%.1f%%\r", (double)(count) / size * 100);
            if (size == count) {
                puts("");
                break;
            }
        }

        fclose(img);
        puts("done!");
    }
    else if (strcmp(buf, "400 Bad Request\0") == 0)
        puts(buf);
    //[END] Receive

    close(sockfd);
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
