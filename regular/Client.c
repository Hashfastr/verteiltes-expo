#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "1337"

int sockfd, numbytes, count, rv;
double num, primesqrt;
struct addrinfo hints, *servinfo, *p;
char s[INET6_ADDRSTRLEN], buf[256];

double is_prime(double prime) {
    //printf("testing %f\n", prime);
    if (prime == 1) return 0;
    if (prime == 3 || prime == 5) return 1;

    primesqrt = sqrt(prime);

   	if (primesqrt==floor(primesqrt)) return 0;

    for (double i = 3; i < primesqrt; i+=2) if ((floor(prime/i)*i) == prime) return 0;

return 1;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("usage exe <hostname>\n");
        return -1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(   p->ai_family, 
                                p->ai_socktype, 
                                p->ai_protocol)) == -1) {
            perror("prime client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("prime client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(  p->ai_family,
                get_in_addr((struct sockaddr *)p->ai_addr),
                s,
                sizeof s);
    printf("prime client: connecting to mother node on %s\n",
            argv[1]);

    freeaddrinfo(servinfo);

    for (;;) {
        if ((numbytes = recv(sockfd, buf, sizeof buf-1, 0)) == -1) {
            perror("recv");
            return 3;
        }

        if (buf[0]=='d') {
            printf("computation done, computed %d primes\n", count);
            send(sockfd, "d", 1, 0);
            return 0;
        }
            num=atoi(buf);

            if(is_prime(num)==1) {
                send(sockfd, "p", 1, 0);
                count++;
            }
            else {
                send(sockfd, "n", 1, 0);
            }
    }
}
