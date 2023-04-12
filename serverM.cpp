/* a tcp server and a udp client 
** tcp server >> receive meeting request from users
** decides which backend server  are these users in
** udp client >> send requests to corresponding server to get user's time availability
*/

#include "project.h"

int main(int argc, const char* argv[]){

    // print boot up msg
    cout << "Main Server is up and running." << endl;

    // get server M's udp port's address information
    struct addrinfo hints, *servinfo, *udp_p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(localhost, UDP_PORT_M, &hints, &servinfo)) != 0) {
        fprintf(stderr, "serverM udp getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket for server M's udp port
    int udp_sockfd;
    for (udp_p = servinfo; udp_p != NULL; udp_p = udp_p->ai_next) 
    {
        if ((udp_sockfd = socket(udp_p->ai_family, udp_p->ai_socktype, udp_p->ai_protocol)) == -1) 
        {
            perror("serverM udp: socket");
            continue;
        }
        if (bind(udp_sockfd, udp_p->ai_addr, udp_p->ai_addrlen) == -1) 
        {
            close(udp_sockfd);
            perror("serverM udp: bind");
            continue;
        }
        break;
    }
    // handle error cases
    if (udp_p == NULL) 
    { 
        fprintf(stderr, "serverM udp: failed to create socket\n");
        return 2;
    }
    // free the linked-list
    freeaddrinfo(servinfo); 

    // receive usernames sent from server A via UDP over UDP_PORT_M 
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char names_buf_a[USERNAMES_BUF_SIZE];
    if ((recvfrom(udp_sockfd, names_buf_a, USERNAMES_BUF_SIZE - 1 , 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
        perror("serverM udp: recvfrom");
        exit(1);
    }

    // maintain a list of usernames corresponding to backend server A
    set<string> usernames_a;
    char *username;
    username = strtok(names_buf_a, " ");
    while (username != NULL) {
        usernames_a.insert(string(username));
        username = strtok(names_buf_a, " ");
    }
    for (set<string>::iterator it = usernames_a.begin(); it != usernames_a.end(); ++it) {
        cout << *it << endl;
    }
    // print correct on screen msg indicating the success of these operations
    cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;

    return 0;
}