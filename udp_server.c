#include "headsock.h"

void str_ser(int sockfd);

int main(int argc, char *argv[])
{

    struct sockaddr_in my_addr;

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("%s\n", "Error: Couldn't connect to a socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // Address family; must be AF_INET
    my_addr.sin_port = htons(MYUDP_PORT); // Internet Protocol (IP) port.
    my_addr.sin_addr.s_addr = INADDR_ANY; // IP address in network byte order. INADDR_ANY is 0.0.0.0 meaning "all the addr"
    bzero(&(my_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Error: Couldn't bind to a socket\n");
        exit(1);
    }

    str_ser(sockfd);

    close(sockfd);
    exit(0);
}

// transmitting and receiving function
void str_ser(int sockfd)
{
    FILE *file;
    char buffer[BUFSIZE];
    struct ack_so ack;
    struct pack_so packet;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen = sizeof(struct sockaddr_in);

    printf("Listening...\n");
    srand(time(NULL));
    int flag = 1, quit = 0, n = 0;
    long receviedsofar = 0;
    while (!quit)
    {
        if ((n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&cliaddr, &cliaddrlen)) == -1)
        {
            printf("%s\n", "Error: Couldn't receive");
            exit(1);
        }
      
        if (rand() % 10)
        {
            ack.len = packet.len;
            ack.num = !(packet.num);
            if ((n = sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&cliaddr, cliaddrlen)) == -1)
            {
                printf("%s\n", "Error: ACK couldn't send!");
                exit(1);
            }
            else
            {
                printf("%s\n", "ACK successfully send!");
            }
        }
        else
        {
            printf("%s\n", "ACK lost!");
        }
        if (packet.num != flag)
        {
            if (packet.data[packet.len - 1] == '\0')
            {
                quit = 1;
                packet.len--;
            }
            memcpy((buffer + receviedsofar), packet.data, packet.len);
            receviedsofar += packet.len;
        }
        flag = packet.num;
    }

    if ((file = fopen("received.txt", "wt")) == NULL)
    {
        printf("%s\n", "Error: File doesn't exist");
        exit(0);
    }

    fwrite(buffer, 1, receviedsofar, file);
    fclose(file);
    printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)receviedsofar);
}