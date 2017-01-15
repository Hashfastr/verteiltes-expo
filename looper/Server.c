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
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define PORT "1337"
#define BACKLOG 20

fd_set master;
fd_set read_fds;

FILE *file;

clock_t t1, t2;

struct sockaddr_storage remoteaddr;
socklen_t addrlen;
struct addrinfo hints, *ai, *p;

struct data {
    int sock;
    char buffer[256];
};
struct data known;

char buf[256], notification[256], remoteIP[INET6_ADDRSTRLEN], quarters [4] = {0,0,0,0}, percent[3], lastpercent[3], *argptr;
int assignednums[1024], nbytes, notibit = 0, yes = 1, i, j, rv, count = 1, quota, v, v1, v2, u, clientnums, updatebit, newfd, listener, fdmax, primetest = 1, test, check, active = 0, quotastart;
float currentpercent, secs, diff;

//function by Beej, its just convenient
//gets sockaddr for IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa);

void handlenewconnection();

void handleknownconnection();

int stoppit(char *argv[]);

void percentages(char *argv[]);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: exe [compuation quota] -n [simple push key] [notification title] -u (set notification updates every 25, 50, and 75 percent)\n");
        return 0;
    }

    quota = atoi(argv[1]);
    printf("running with %d arguments\nnumber quota is %d\n",
            argc,
            quota);

    
    file = fopen("./data.txt", "a");

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    //get socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "Mother-Node: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) continue;

        //rid of the "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    //didn't get bound
    if (p == NULL) {
        fprintf(stderr, "Mother-Node: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);

    //listen
    if (listen(listener, BACKLOG) == -1) {
        perror("listen");
        exit(3);
    }

    FD_SET(listener, &master);

    fdmax = listener;

    //main loop

    //check notificiation setting
    for (v = 2; v < argc; v++) {
        if (strcmp(argv[v],"-n") == 0) {
            notibit=1;
            v++;
            v1 = v;
            v++;
            v2 = v;
            for (u = 2; u < argc; u++) {
                if (strcmp(argv[u],"-u") == 0) {
                    updatebit=1;
                    break;
                }
            }
            break;
        }
    }

    printf("Waiting for connections, enter anything to continue...\n");

    sleep(2);

    t1=clock();

    while(yes) {

        percentages(argv);

        //start quota stop check
        if (quota == count) {
            stoppit(argv);
            return 0;
        }
        //end quota stop check

        //printf("Main loop begun...\n");
        read_fds = master;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for (i = 0; i <= fdmax; i++) {
            //quota stop check
            if (quota == count) {
                stoppit(argv);
                return 0;
            }

            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) handlenewconnection();

                else handleknownconnection();
            }
        }
    }
}

/////////////////////////////////////////////////////
//////////////////////end main///////////////////////
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
//////////////////////functions//////////////////////
/////////////////////////////////////////////////////

//function by Beej, its just convenient
//gets sockaddr for IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handlenewconnection() { //transmission from new client
    addrlen = sizeof remoteaddr;
    newfd = accept(listener, 
            (struct sockaddr *)&remoteaddr, 
            &addrlen);

    if (newfd == -1) perror("accept");
    else {
        FD_SET(newfd, &master);
        if (newfd > fdmax) fdmax = newfd;
        printf("Mother-Node: new connection from %s on socket %d\n",
                inet_ntop(remoteaddr.ss_family, 
                    get_in_addr((struct sockaddr*)&remoteaddr), 
                    remoteIP, 
                    INET6_ADDRSTRLEN), 
                newfd);

        if(FD_ISSET(newfd, &master)) {
            if ((count + active) < quota) {
            assignednums[newfd] = primetest;
            sprintf(buf, "%d", assignednums[newfd]);
            primetest+=2;
            if(send(newfd, &buf, sizeof buf-1, 0) == -1) {
                printf("Error sending prime to test to socket %d", newfd);
                primetest--;
            }
            active++;
            }
            else {
            //kill client
            send(newfd, "d", 1, 0);
            //sleep(1);
            close(newfd);
            FD_CLR(newfd, &master);
            }
        }
    }
}

void handleknownconnection() { // transmission from known client
    memset(buf, 0, strlen(buf));
    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
        if (nbytes == 0) printf("Mother-Node: socket %d hung up\n", i);
        else {
            perror("recv");
        }
        active--;
        close(i);
        FD_CLR(i, &master);
    } else { 
        known.sock = i;
        memset(known.buffer, 0, strlen(known.buffer));
        strcpy(known.buffer, buf);


        /////////////
        //not prime//
        /////////////
        if (buf[0] == 'n') {
            //printf("%d is not prime\n", assignednums[i]);
            //printf("socket: %d, buffer: %s\n", i, buf);
            if ((count + active) <= quota) {
            assignednums[i] = primetest;
            sprintf(buf, "%d", assignednums[i]);
            primetest+=2;
            if(send(i, &buf, sizeof buf-1, 0) == -1) {
                printf("Error sending prime to test to socket %d\n", i);
            }
            }
            else {
            //kill client
            send(i, "d", 1, 0);
            active--;
            //sleep(1);
            close(i);
            FD_CLR(i, &master);
            }
        } 

        /////////////
        //  prime  //
        /////////////
        if (buf[0] == 'p') {
            //printf("recieved %s\n", buf);
            //printf("socket: %d, buffer: %s\n", i, buf);
            //fprintf(file, "%d\n", assignednums[i]);
            //printf("%d is prime\n", assignednums[i]);
            count++;

            if ((count + active) <= quota) {
            assignednums[i] = primetest;
            sprintf(buf, "%d", assignednums[i]);
            primetest+=2;
            if(send(i, &buf, sizeof buf-1, 0) == -1) {
                printf("Error sending prime to test to socket %d\n", i);
                primetest--; 
            }
            }
            else {
            //kill client
            send(i, "d", 1, 0);
            active--;
            //sleep(1);
            close(i);
            FD_CLR(i, &master);
            }

            currentpercent = (float)count/(float)quota;
            //printf("%.2f\n", currentpercent);
        }

        if (buf[0] == 'd') active--;
    }
}

int stoppit(char *argv[]) {
    t2=clock();
    diff = ((float)t2-(float)t1);
    secs = diff / CLOCKS_PER_SEC;
    fprintf(file, "%f ", secs);  

    fclose(file);

    if (notibit == 1) {
        sprintf(notification, "curl --data 'key=%s&title=%s&msg=%d primes computed in %f seconds' https://api.simplepush.io/send", argv[v1], argv[v2], quota, secs);    
        system(notification);
    }
    return 1;
}

void percentages(char *argv[]) {
    sprintf(percent, "%.2f", currentpercent);
    if (strcmp(lastpercent, percent) != 0) {
        clientnums = fdmax - 4;
        system("clear");
        printf("calculating %d primes...\n%d clients connected\n%s%%; done\n", quota, clientnums, percent);
        if (updatebit==1) {
            if (strcmp(percent, "0.25\0")==0) {
                sprintf(notification, "curl --data 'key=%s&title=%s&msg=25 percent done' https://api.simplepush.io/send", argv[v1], argv[v2]);
                //printf("%s\n", notification);
                //check = getchar();
                system(notification);
            }
            if (strcmp(percent, "0.50")==0) {

                sprintf(notification, "curl --data 'key=%s&title=%s&msg=50 percent done' https://api.simplepush.io/send", argv[v1], argv[v2]);
                system(notification);
            }
            if (strcmp(percent, "0.75")==0) {
                sprintf(notification, "curl --data 'key=%s&title=%s&msg=75 percent done' https://api.simplepush.io/send", argv[v1], argv[v2]);
                system(notification);
            }
        }
        strcpy(lastpercent, percent);
    }
}

