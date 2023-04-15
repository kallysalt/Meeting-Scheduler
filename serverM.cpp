/* a tcp server and a udp client 
** tcp server >> receive meeting request from users
** decides which backend server  are these users in
** udp client >> send requests to corresponding server to get user's time availability
*/

#include "project.h"

set<string> userset_a;
set<string> userset_b;

// get sockaddr (IPv4)
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*) sa)->sin_addr);

}

// get socket port number (IPv4)
void get_in_port(struct sockaddr_storage &their_addr, char *port) 
{
    sockaddr *sa = (struct sockaddr *) &their_addr;
    uint16_t port_num = ntohs(((struct sockaddr_in *) sa)->sin_port);
    sprintf(port, "%u", port_num);
}

// signal handler
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

set<string> buf_to_set(char *buf)
{
    set<string> names;
    char *name;
    name = strtok(buf, " ");
    while (name != NULL) 
    {
        names.insert(string(name));
        name = strtok(NULL, " ");
    }
    return names;
}

int validate_client_input(char *buf, vector<string> &names, vector<int> &servers)
{
    // check if buf is empty
    if (strlen(buf) == 0) 
    {
        // TODO: handle invalid input 
        // tell client which input is not valid
        return 0;
    }
    
    // check if usernames being entered are valid
    char *name;
    name = strtok(buf, " ");
    while (name != NULL) 
    {
        if (strlen(name) > 20) // valid username must not exceed 20 characters
        {
            // TODO: handle invalid input 
            // set entries in names and servers to null
            // tell client which input is not valid
            return 0;
        }
        else  // valid username must belongs to either server a or b
        {
            int belongs_to_a = (userset_a.find(string(name)) != userset_a.end());
            int belongs_to_b =  (userset_b.find(string(name)) != userset_b.end());
            if ((!belongs_to_a) && (!belongs_to_b)) 
            {
                // TODO: handle invalid input 
                // set all entries in names and servers to null
                // tell client which client is not valid
                return 0;
            }
            names.push_back(string(name));
            servers.push_back(belongs_to_a ? 0 : 1);
            name = strtok(NULL, " ");
        }
    }
    return 1;
}

int main(int argc, const char* argv[]){

    // print boot up msg
    cout << "Main Server is up and running." << endl;

    // get server M's udp port's address information
    struct addrinfo hints_udp_m, *servinfo_udp_m , *p_udp_m;
    int rv_udp_m;
    memset(&hints_udp_m, 0, sizeof hints_udp_m);
    hints_udp_m.ai_family = AF_INET;
    hints_udp_m.ai_socktype = SOCK_DGRAM;
    hints_udp_m.ai_flags = AI_PASSIVE;
    if ((rv_udp_m = getaddrinfo(localhost, UDP_PORT_M, &hints_udp_m, &servinfo_udp_m )) != 0) 
    {
        fprintf(stderr, "serverM udp getaddrinfo: %s\n", gai_strerror(rv_udp_m));
        return 1;
    }
    // loop through all the results and make a socket for server M's udp port
    int sockfd_udp_m;
    for (p_udp_m = servinfo_udp_m ; p_udp_m != NULL; p_udp_m = p_udp_m->ai_next) 
    {
        if ((sockfd_udp_m = socket(p_udp_m->ai_family, p_udp_m->ai_socktype, p_udp_m->ai_protocol)) == -1) 
        {
            perror("serverM udp: socket");
            continue;
        }
        if (bind(sockfd_udp_m, p_udp_m->ai_addr, p_udp_m->ai_addrlen) == -1) 
        {
            close(sockfd_udp_m);
            perror("serverM udp: bind");
            continue;
        }
        break;
    }
    // handle error cases
    if (p_udp_m == NULL) 
    { 
        fprintf(stderr, "serverM udp: failed to create socket\n");
        return 2;
    }
    // free the linked-list
    freeaddrinfo(servinfo_udp_m); 

    // receive usernames sent from server A/B via UDP over UDP_PORT_M 
    struct sockaddr_storage their_addr_udp;
    socklen_t udp_addr_len = sizeof their_addr_udp;
    char names_buf[USERNAMES_BUF_SIZE];
    int numbytes;
    
    if ((numbytes = recvfrom(sockfd_udp_m, names_buf, USERNAMES_BUF_SIZE - 1 , 0, 
        (struct sockaddr *) &their_addr_udp, &udp_addr_len)) == -1) 
    {
        perror("serverM udp: recvfrom");
        exit(1);
    }
    names_buf[numbytes] = '\0';
    cout << names_buf << endl;

    char src_port[10]; // ?
    get_in_port(their_addr_udp, src_port);
    if (strcmp(src_port, UDP_PORT_A) == 0) 
    {
        // maintain a list of usernames corresponding to backend server A
        userset_a = buf_to_set(names_buf);
        // print correct on screen msg indicating the success of reeiving usernames from server A
        cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;
    }
    else if (strcmp(src_port, UDP_PORT_B) == 0) 
    {
        // maintain a list of usernames corresponding to backend server B
        userset_b = buf_to_set(names_buf);
        // print correct on screen msg indicating the success of reeiving usernames from server B
        cout << "Main Server received the username list from server B using UDP over " << UDP_PORT_M << "." << endl;
    }
    
    // receive usernames sent from server A/B via UDP over UDP_PORT_M 
    memset(names_buf, 0, sizeof(names_buf));
    if ((numbytes = recvfrom(sockfd_udp_m, names_buf, USERNAMES_BUF_SIZE - 1 , 0, 
        (struct sockaddr *) &their_addr_udp, &udp_addr_len)) == -1) 
    {
        perror("serverM udp: recvfrom");
        exit(1);
    }
    names_buf[numbytes] = '\0';
    cout << names_buf << endl;

    memset(src_port, 0, sizeof(src_port));
    get_in_port(their_addr_udp, src_port);
    if (strcmp(src_port, UDP_PORT_A) == 0) 
    {
        // maintain a list of usernames corresponding to backend server A
        userset_a = buf_to_set(names_buf);
        // print correct on screen msg indicating the success of reeiving usernames from server A
        cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;
    }
    else if (strcmp(src_port, UDP_PORT_B) == 0) 
    {
        // maintain a list of usernames corresponding to backend server B
        userset_b = buf_to_set(names_buf);
        // print correct on screen msg indicating the success of reeiving usernames from server B
        cout << "Main Server received the username list from server B using UDP over " << UDP_PORT_M << "." << endl;
    }

    // get server M's tcp port's address information
    struct addrinfo hints_tcp_m, *servinfo_tcp_m, *p_tcp_m;
    int rv_tcp_m;
    memset(&hints_tcp_m, 0, sizeof hints_tcp_m);
    hints_tcp_m.ai_family = AF_UNSPEC;
    hints_tcp_m.ai_socktype = SOCK_STREAM;
    hints_tcp_m.ai_flags = AI_PASSIVE;
    if ((rv_tcp_m = getaddrinfo(localhost, TCP_PORT_M, &hints_tcp_m, &servinfo_tcp_m)) != 0) 
    {
        fprintf(stderr, "serverM tcp getaddrinfo: %s\n", gai_strerror(rv_tcp_m));
        return 1;
    }

    // loop through all the results and bind to the first we can
    int sockfd_tcp_m;  // listen on sock_fd
    int yes = 1;
    for (p_tcp_m = servinfo_tcp_m; p_tcp_m != NULL; p_tcp_m = p_tcp_m->ai_next) 
    {
        if ((sockfd_tcp_m = socket(p_tcp_m->ai_family, p_tcp_m->ai_socktype, p_tcp_m->ai_protocol)) == -1) 
        {
            perror("serverM tcp: socket");
            continue;
        }
        if (setsockopt(sockfd_tcp_m, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
        {
            perror("serverM tcp: setsockopt");
            exit(1);
        }
        if (bind(sockfd_tcp_m, p_tcp_m->ai_addr, p_tcp_m->ai_addrlen) == -1) 
        {
            close(sockfd_tcp_m);
            perror("serverM tcp: bind");
            continue;
        }
        break;
    }

    // all done with this structure
    freeaddrinfo(servinfo_tcp_m); 

    // handle error cases
    if (p_tcp_m == NULL)  
    {
        fprintf(stderr, "serverM tcp: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd_tcp_m, BACKLOG) == -1) {
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
        struct sockaddr_storage their_addr_tcp; // connector's address information
        sin_size = sizeof their_addr_tcp;
        new_fd = accept(sockfd_tcp_m, (struct sockaddr *) &their_addr_tcp, &sin_size);
        if (new_fd == -1) 
        {
            perror("accept");
            continue;
        }
        // printf("server: got connection");

        if (!fork()) // this is the child process
        { 
            close(sockfd_tcp_m); // child doesn't need the listener
            if (send(new_fd, "start client!", 13, 0) == -1) 
            {
                perror("client: send");
            }

            // receive names sent from client via tcp
            char names_buf[USERNAMES_BUF_SIZE];
            if ((numbytes = recv(new_fd, names_buf, USERNAMES_BUF_SIZE - 1, 0)) == -1) 
            {
                perror("serverM tcp: recv");
                exit(1);
            }
            names_buf[numbytes] = '\0';
            cout << names_buf << endl;

            // validate the input
            // TODO: for usernames that do not exist, reply the client with a msg saying which usernames do not exist
            // for those do exist, split them into two sublists based on where the user information is located
            vector<string> users;
            vector<int> servers; // store the server each user belongs to (a=0, b=1) 
            while (!validate_client_input(names_buf, users, servers)) 
            {
                // if (send(new_fd, "invalid input!", 14, 0) == -1) 
                // {
                //     perror("client: send");
                // }
            }

            // forward valid usernames to the corresponding backend server via udp

            // receive time slots from different backend servers via udp

            // run an algo to get the final time slots that works for all participants
            
            // sends the result back to the client via tcp

            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    close(sockfd_udp_m);
    return 0;
}