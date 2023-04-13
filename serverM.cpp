/* a tcp server and a udp client 
** tcp server >> receive meeting request from users
** decides which backend server  are these users in
** udp client >> send requests to corresponding server to get user's time availability
*/

#include "project.h"

// get sockaddr (IPv4)
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);

}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main(int argc, const char* argv[]){

    // print boot up msg
    cout << "Main Server is up and running." << endl;

    // get server M's udp port's address information
    struct addrinfo udp_hints, *udp_servinfo, *udp_p;
    int udp_rv;
    memset(&udp_hints, 0, sizeof udp_hints);
    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;
    udp_hints.ai_flags = AI_PASSIVE;
    if ((udp_rv = getaddrinfo(localhost, UDP_PORT_M, &udp_hints, &udp_servinfo)) != 0) {
        fprintf(stderr, "serverM udp getaddrinfo: %s\n", gai_strerror(udp_rv));
        return 1;
    }
    // loop through all the results and make a socket for server M's udp port
    int udp_sockfd;
    for (udp_p = udp_servinfo; udp_p != NULL; udp_p = udp_p->ai_next) 
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
    freeaddrinfo(udp_servinfo); 

    // receive usernames sent from server A via UDP over UDP_PORT_M 
    struct sockaddr_storage udp_their_addr;
    socklen_t udp_addr_len = sizeof udp_their_addr;
    char names_buf_a[USERNAMES_BUF_SIZE];
    int numbytes;
    if ((numbytes = recvfrom(udp_sockfd, names_buf_a, USERNAMES_BUF_SIZE - 1 , 0, 
        (struct sockaddr *)&udp_their_addr, &udp_addr_len)) == -1) 
    {
        perror("serverM udp: recvfrom");
        exit(1);
    }
    names_buf_a[numbytes] = '\0';
    // cout << strlen(names_buf_a) << endl;
    // maintain a list of usernames corresponding to backend server A
    set<string> usernames_a;
    char *username;
    username = strtok(names_buf_a, " ");
    while (username != NULL) 
    {
        usernames_a.insert(string(username));
        username = strtok(NULL, " ");
    }
    // for (set<string>::iterator it = usernames_a.begin(); it != usernames_a.end(); ++it) 
    // {
    //     cout << *it << endl;
    // }
    
    // print correct on screen msg indicating the success of reeiving usernames from server A
    cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;
    // char s[INET_ADDRSTRLEN];
    // cout << "Main Server received the username list from server " << inet_ntop(udp_their_addr.ss_family, get_in_addr((struct sockaddr *)&udp_their_addr), s, sizeof s) << " using UDP over " << UDP_PORT_M << "." << endl;

    // receive usernames sent from server B via UDP over UDP_PORT_M 
    char names_buf_b[USERNAMES_BUF_SIZE];
    if ((numbytes = recvfrom(udp_sockfd, names_buf_b, USERNAMES_BUF_SIZE - 1 , 0, 
        (struct sockaddr *)&udp_their_addr, &udp_addr_len)) == -1) 
    {
        perror("serverM udp: recvfrom");
        exit(1);
    }
    names_buf_b[numbytes] = '\0';
    // cout << strlen(names_buf_b) << endl;
    // maintain a list of usernames corresponding to backend server B
    set<string> usernames_b;
    username = strtok(names_buf_b, " ");
    while (username != NULL) 
    {
        usernames_b.insert(string(username));
        username = strtok(NULL, " ");
    }
    // for (set<string>::iterator it = usernames_b.begin(); it != usernames_b.end(); ++it) 
    // {
    //     cout << *it << endl;
    // }

   // print correct on screen msg indicating the success of reeiving usernames from server B
    cout << "Main Server received the username list from server B using UDP over " << UDP_PORT_M << "." << endl;

    // get server M's tcp port's address information
    struct addrinfo tcp_hints, *tcp_servinfo, *tcp_p;
    int tcp_rv;
    memset(&tcp_hints, 0, sizeof tcp_hints);
    tcp_hints.ai_family = AF_UNSPEC;
    tcp_hints.ai_socktype = SOCK_STREAM;
    tcp_hints.ai_flags = AI_PASSIVE;
    if ((tcp_rv = getaddrinfo(localhost, TCP_PORT_M, &tcp_hints, &tcp_servinfo)) != 0) 
    {
        fprintf(stderr, "serverM tcp getaddrinfo: %s\n", gai_strerror(udp_rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    int tcp_sockfd;  // listen on sock_fd
    int yes = 1;
    for (tcp_p = tcp_servinfo; tcp_p != NULL; tcp_p = tcp_p->ai_next) {
        if ((tcp_sockfd = socket(tcp_p->ai_family, tcp_p->ai_socktype, tcp_p->ai_protocol)) == -1) 
        {
            perror("serverM tcp: socket");
            continue;
        }
        if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
        {
            perror("serverM tcp: setsockopt");
            exit(1);
        }
        if (bind(tcp_sockfd, tcp_p->ai_addr, tcp_p->ai_addrlen) == -1) 
        {
            close(tcp_sockfd);
            perror("serverM tcp: bind");
            continue;
        }
        break;
    }

    // all done with this structure
    freeaddrinfo(tcp_servinfo); 

    // handle error cases
    if (tcp_p == NULL)  
    {
        fprintf(stderr, "serverM tcp: failed to bind\n");
        exit(1);
    }
    if (listen(tcp_sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    socklen_t sin_size;
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
        perror("sigaction");
        exit(1);
    }
    // printf("serverM tcp: waiting for connections...\n");

    while (1) // main accept() loop
    {  
        int new_fd; // new connection on new_fd
        struct sockaddr_storage tcp_their_addr; // connector's address information
        sin_size = sizeof tcp_their_addr;
        new_fd = accept(tcp_sockfd, (struct sockaddr *)&tcp_their_addr, &sin_size);
        if (new_fd == -1) 
        {
            perror("accept");
            continue;
        }
        // printf("server: got connection");

        if (!fork()) // this is the child process
        { 
            close(tcp_sockfd); // child doesn't need the listener
            if (send(new_fd, "start client!", 13, 0) == -1) 
            {
                perror("client: send");
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    close(udp_sockfd);
    return 0;
}