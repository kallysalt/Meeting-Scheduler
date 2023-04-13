/* a tcp client >> request time availability of all users in the meeting */

#include "project.h"

// TODO: dynamic port assignment

// get sockaddr (IPv4)
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);

}

int main(int argc, const char* argv[]){

    // TODO: after servers are booted up and required usernames are transferred 
    // from backend servers to the main server, the client will be started

    // print boot up msg
    cout << "Client is up and running." << endl;

    // get server M's tcp port's address information
    struct addrinfo hints, *servinfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(localhost, TCP_PORT_M, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "client getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    int sockfd;  
    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    // handle error cases
    if (p == NULL) 
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    printf("client: connecting to server\n");

    // all done with this structure
    freeaddrinfo(servinfo); 

    char buf[CLIENT_MAXDATASIZE];
    int numbytes;
    if ((numbytes = recv(sockfd, buf, CLIENT_MAXDATASIZE-1, 0)) == -1)
    {
        perror("client: recv");
        exit(1);
    }
    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    // show a prompt: Please enter the usernames to check schedule availability:
    cout << "Please enter the usernames to check schedule availability:" << endl;

    // wait for the usernames to be taken as inputs from the user 
    // user can enter up to 10 usernames, all of which are separated by a single space

    // store the usernames
    close(sockfd);
    return 0;
}

