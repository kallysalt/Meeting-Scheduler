/* a tcp server and a udp client 
** tcp server >> receive meeting request from users
** decides which backend server  are these users in
** udp client >> send requests to corresponding server to get user's time availability
*/

#include "project.h"

int init_udp_socket(char *port, struct addrinfo *&p) 
{
    int sockfd;
    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // TODO: use IPv4 or IPv6?
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(localhost, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "serverM udp getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("serverM udp: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("serverM udp: bind");
            continue;
        }
        break;
    }

    if (p == NULL) { 
        fprintf(stderr, "serverM udp: failed to create socket\n");
        return 2;
    }

    freeaddrinfo(servinfo); // free the linked-list
    return sockfd;
}

int main(int argc, const char* argv[]){

    // print boot up msg
    cout << "Main Server is up and running." << endl;

    // create udp socket and associate it with a port
    struct addrinfo *udp_p;
    int udp_sockfd = init_udp_socket(UDP_PORT_M, udp_p);
    if (udp_sockfd <= 0) {
        return udp_sockfd;
    }

    // receive usernames sent from a and b via UDP over port
    struct sockaddr_storage their_addr;
    int size = MAX_LEN_USERNAME * MAX_NUM_USER + MAX_NUM_USER;
    char buf_a[size];
    socklen_t addr_len = sizeof their_addr;
    if ((recvfrom(udp_sockfd, buf_a, size , 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
        perror("serverM udp: recvfrom");
        exit(1);
    }

    // maintain a list of usernames corresponding to backend server a
    string input(buf_a);
    istringstream iss(input);
    set<string> a_usernames;
    string username;
    while (iss >> username) {
        a_usernames.insert(username);
    }
    for (set<string>::iterator it = a_usernames.begin(); it != a_usernames.end(); ++it) {
        cout << *it << endl;
    }
    // print correct on screen msg indicating the success of these operations
    cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;

    // receive usernames sent from b via UDP over port
    char buf_b[size];
    addr_len = sizeof their_addr;
    if ((recvfrom(udp_sockfd, buf_b, size , 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
        perror("serverM udp: recvfrom");
        exit(1);
    }

    // maintain a list of usernames corresponding to backend server a
    string input2(buf_b);
    istringstream iss2(input2);
    set<string> b_usernames;
    while (iss2 >> username) {
        b_usernames.insert(username);
    }
    for (set<string>::iterator it = b_usernames.begin(); it != b_usernames.end(); ++it) {
        cout << *it << endl;
    }
    // print correct on screen msg indicating the success of these operations
    cout << "Main Server received the username list from server B using UDP over " << UDP_PORT_M << "." << endl;

    while(1){}

    return 0;
}