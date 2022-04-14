#include "headsock.h"

float str_cli(FILE *file, int sockfd, long *bytecount, struct sockaddr *cliaddr, int cliaddrlen, socklen_t *seraddrlen);
void tv_sub(struct timeval *out, struct timeval *in);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("%s\n", "Error: Parameters not match");
        exit(0);
    }

    struct hostent *sh;
    if (!(sh = gethostbyname(argv[1])))
    {
        printf("%s\n", "Error: Wrong hostname");
        exit(0);
    }

    struct in_addr **addrs = (struct in_addr **)sh->h_addr_list;
    printf("Canonical server name: %s\n", sh->h_name);
    for (char **pptr = sh->h_aliases; *pptr != NULL; pptr++)
        printf("The aliases name is: %s\n", *pptr);

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("%s\n", "Error: Couldn't connect to a socket");
        exit(1);
    }

    struct sockaddr_in ser_addr;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(MYUDP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);

    FILE *file;
    if ((file = fopen("send.txt", "r+t")) == NULL)
    {
        printf("%s\n", "Error: File doesn't exist");
        exit(0);
    }

    long bytecount;
    socklen_t seraddrlen;
    float ti = str_cli(file, sockfd, &bytecount, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &seraddrlen);

    float rt = ((bytecount - 1) / (float)ti);
    printf("Time: %.3f (ms)\nData sent: %d (bytes)\nThroughput: %f (Kbytes/s)\n", ti, (int)bytecount - 1, rt);

    close(sockfd);
    fclose(file);
    exit(0);
}

float str_cli(FILE *file, int sockfd, long *bytecount, struct sockaddr *cliaddr, int cliaddrlen, socklen_t *seraddrlen)
{
    struct pack_so packet;
    struct ack_so ack;
    struct timeval sendt, recvt, msgtime;

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);
    printf("The file length is %d bytes\n", (int)filesize);
    printf("The packet length is %d bytes\n", PACKETSIZE);

    char *buffer = (char *)malloc(filesize);
    if (!buffer)
        exit(2);

    fread(buffer, 1, filesize, file);

    // the whole file is loaded in the buffer
    buffer[filesize] = '\0'; // append the end of file
    long sentsofar = 0;
    int n, ready = 1, flag = 0;
    gettimeofday(&sendt, NULL); // get the current time
    while (sentsofar <= filesize)
    {
        if (ready)
        {
            packet.len = (filesize - sentsofar + 1) <= PACKETSIZE ? filesize - sentsofar + 1 : PACKETSIZE; // last data unit
            packet.num = flag;
            memcpy(packet.data, (buffer + sentsofar), packet.len);

            gettimeofday(&msgtime, NULL);
            if ((n = sendto(sockfd, &packet, sizeof(packet), 0, cliaddr, cliaddrlen)) == -1) // send message
            {
                printf("%s\n", "Error: Couldn't send!");
                exit(1);
            }
            // update the sequence
            flag = !(packet.num);
            sentsofar += packet.len;
            ready = 0;
        }
        if ((n = recvfrom(sockfd, &ack, sizeof(ack), MSG_DONTWAIT, cliaddr, seraddrlen)) == -1)
        {
            struct timeval curtime;
            gettimeofday(&curtime, NULL);
            if (curtime.tv_sec - msgtime.tv_sec >= TIMEOUTSEC)
            {
                printf("%s\n", "Error: Timeout, Resending the packet...");
                gettimeofday(&msgtime, NULL);
                if ((n = sendto(sockfd, &packet, sizeof(packet), 0, cliaddr, cliaddrlen)) == -1)
                {
                    printf("%s\n", "Error: Couldn't send!");
                    exit(1);
                }
            }
        }
        else
        {
            if (ack.num != flag || ack.len != packet.len)
            {
                gettimeofday(&msgtime, NULL);
                if ((n = sendto(sockfd, &packet, sizeof(packet), 0, cliaddr, cliaddrlen)) == -1)
                {
                    printf("%s\n", "Error: Transmission failed, Resending the packet...");
                    printf("%s\n", "Error: Couldn't send!");
                    exit(1);
                }
            }
            else
            {
                printf("%s\n", "ACK successfully receieved, Sending the next packet...");
                ready = 1;
            }
        }
    }

    gettimeofday(&recvt, NULL);
    *bytecount = sentsofar;
    tv_sub(&recvt, &sendt);

    float totaltime = (recvt.tv_sec) * 1000.0 + (recvt.tv_usec) / 1000.0;
    return totaltime;
}

// calculate the time interval between out and in
void tv_sub(struct timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }

    out->tv_sec -= in->tv_sec;
}
