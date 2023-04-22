/* a tcp server and a udp client 
** tcp server >> receive meeting request from users
** decides which backend server  are these users in
** udp client >> send requests to corresponding server to get user's time availability
*/

#include "project.h"

set<string> userset_a;
set<string> userset_b;

// get sockaddr (IPv4) (from beej's guide)
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*) sa)->sin_addr);
}

// get socket port number (IPv4) (from beej's guide)
void get_in_port(struct sockaddr_storage &their_addr, char *port) 
{
    sockaddr *sa = (struct sockaddr *) &their_addr;
    uint16_t port_num = ntohs(((struct sockaddr_in *) sa)->sin_port);
    sprintf(port, "%u", port_num);
}

// signal handler (from beej's guide)
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// convert a names buffer (names are separated by ', ') to a set of strings
set<string> buf_to_set(char *buf)
{
    set<string> names;
    char *name;
    name = strtok(buf, ", ");
    while (name != NULL) 
    {
        names.insert(string(name));
        name = strtok(NULL, ", ");
    }
    return names;
}

// validate client input (names are separated by ' ')
void validate_client_input(char *buf, vector<string> &invalid, vector<string> &valid, vector<int> &servers)
{   
    char *name;
    name = strtok(buf, " ");

    while (name != NULL) 
    {
        if (strlen(name) > 20) // check if username is valid (no more than 20 characters)
        {
            invalid.push_back(string(name));
        }
        else // check if username belongs to either server a or b
        {
            int belongs_to_a = (userset_a.find(string(name)) != userset_a.end());
            int belongs_to_b = (userset_b.find(string(name)) != userset_b.end());
            if ((!belongs_to_a) && (!belongs_to_b)) 
            {
                invalid.push_back(string(name));
            }
            else {
                valid.push_back(string(name));
                servers.push_back(belongs_to_a ? 0 : 1);
            }
        }
        name = strtok(NULL, " ");
    }
}

// convert a vector of strings (names) to a buffer (names are separated by ', ')
void str_vec_to_buf(vector<string> &vec, char *buf)
{
    char* curr = buf;
    for (size_t i = 0; i < vec.size(); i++) 
    {
        const string &str = vec[i];
        copy(str.begin(), str.end(), curr);
        curr += str.size();
        // add a space after each string, except the last one
        if (i < vec.size() - 1) {
            *curr = ',';
            curr++;
            *curr = ' ';
            curr++;
        }
        // add a null-terminator after the last one
        else{
            *curr = '\0';
        }
    }
}

// convert a vector of integers (times) to a buffer (integers are separated by ' ')
void int_vec_to_buf(vector<int> &vec, char *buf)
{
    char* curr = buf;
    for (size_t i = 0; i < vec.size(); i++) 
    {
        stringstream ss;
        ss << vec[i];
        const string &str = ss.str();

        copy(str.begin(), str.end(), curr);
        curr += str.size();
        // add a space after each string, except the last one
        if (i < vec.size() - 1) {
            *curr = ' ';
            curr++;
        }
        // add a null-terminator after the last one
        else{
            *curr = '\0';
        }
    }
}

// convert a buffer (times separated by ' ') to a vector of integers
vector<int> buf_to_int_vec(char *buf)
{
    vector<int> times;
    char *time;
    time = strtok(buf, " ");
    while (time != NULL) 
    {
        times.push_back(atoi(time));
        time = strtok(NULL, " ");
    }
    return times;
}

// find final time slots
vector<int> find_final_time_slots(vector<int> &times_a, vector<int> &times_b)
{
    vector<int> final_times;
    // if any of the users has no time slots, return immediately
    if (times_a.size() == 0 || times_b.size() == 0) 
    {
        // cout << "dbg: times_a.size() == 0 || times_b.size() == 0" << endl;
        return final_times;
    }
    // otherwise, find intersection of time slots
    for (size_t i = 0; i < times_a.size(); i+=2) 
    {
        for (size_t j = 0; j < times_b.size(); j+=2) 
        {
            int x_start = times_a[i];
            int x_end = times_a[i + 1];
            int y_start = times_b[j];
            int y_end = times_b[j + 1];

            // check if there is an intersection
            if ((x_end > y_start && x_start < y_end) || (y_end > x_start && y_start < x_end))
            {
                final_times.push_back(max(x_start, y_start));
                final_times.push_back(min(x_end, y_end));
            }
        }
    }
    return final_times;
}

int main(int argc, const char* argv[]){

    // print boot up msg ////////////////////////////////////////////////////////////////////////////////////////////
    cout << "Main Server is up and running." << endl;

    // get server M's udp port's address information (from beej's guide)
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

    // loop through all the results and make a socket for server M's udp port (from beej's guide)
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

    // handle error cases (from beej's guide)
    if (p_udp_m == NULL) 
    { 
        fprintf(stderr, "serverM udp: failed to create socket\n");
        return 2;
    }

    // free the linked-list (from beej's guide)
    freeaddrinfo(servinfo_udp_m); 

    // initialize server A and server B's udp port's address information (from beej's guide)
    struct sockaddr_storage addr_udp_a;
    struct sockaddr_storage addr_udp_b;

    // receive usernames sent from server A/B via udp over UDP_PORT_M (from beej's guide)
    struct sockaddr_storage their_addr_udp;
    socklen_t udp_addr_len = sizeof their_addr_udp;
    char names_buf[USERNAMES_BUF_SIZE];
    memset(names_buf, 0, sizeof(names_buf));
    int numbytes;
    if ((numbytes = recvfrom(sockfd_udp_m, names_buf, USERNAMES_BUF_SIZE - 1 , 0, 
        (struct sockaddr *) &their_addr_udp, &udp_addr_len)) == -1) 
    {
        perror("serverM udp: recvfrom");
        exit(1);
    }
    names_buf[numbytes] = '\0';

    // TODO: are both servers guaranteed to send at least one username?

    // check which server sent the usernames
    char src_port[10]; // ?
    memset(src_port, 0, sizeof(src_port));
    get_in_port(their_addr_udp, src_port);
    if (strcmp(src_port, UDP_PORT_A) == 0) 
    {
        // maintain a list of usernames corresponding to backend server A
        userset_a = buf_to_set(names_buf);
        addr_udp_a = their_addr_udp;
        // print correct on screen msg indicating the success of reeiving usernames from server A
        cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;
    }
    else if (strcmp(src_port, UDP_PORT_B) == 0) 
    {
        // maintain a list of usernames corresponding to backend server B
        userset_b = buf_to_set(names_buf);
        addr_udp_b = their_addr_udp;
        // print correct on screen msg indicating the success of reeiving usernames from server B
        cout << "Main Server received the username list from server B using UDP over " << UDP_PORT_M << "." << endl;
    }
    
    // receive usernames sent from server A/B via udp over UDP_PORT_M (from beej's guide)
    memset(names_buf, 0, sizeof(names_buf));
    if ((numbytes = recvfrom(sockfd_udp_m, names_buf, USERNAMES_BUF_SIZE - 1 , 0, 
        (struct sockaddr *) &their_addr_udp, &udp_addr_len)) == -1) 
    {
        perror("serverM udp: recvfrom");
        exit(1);
    }
    names_buf[numbytes] = '\0';

    // check which server sent the usernames
    memset(src_port, 0, sizeof(src_port));
    get_in_port(their_addr_udp, src_port);
    if (strcmp(src_port, UDP_PORT_A) == 0) 
    {
        // maintain a list of usernames corresponding to backend server A
        userset_a = buf_to_set(names_buf);
        addr_udp_a = their_addr_udp;
        // print correct on screen msg indicating the success of reeiving usernames from server A
        cout << "Main Server received the username list from server A using UDP over " << UDP_PORT_M << "." << endl;
    }
    else if (strcmp(src_port, UDP_PORT_B) == 0) 
    {
        // maintain a list of usernames corresponding to backend server B
        userset_b = buf_to_set(names_buf);
        addr_udp_b = their_addr_udp;
        // print correct on screen msg indicating the success of reeiving usernames from server B
        cout << "Main Server received the username list from server B using UDP over " << UDP_PORT_M << "." << endl;
    }

    // get server M's tcp port's address information (from beej's guide)
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

    // loop through all the results and bind to the first we can (from beej's guide)
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

    // all done with this structure (from beej's guide)
    freeaddrinfo(servinfo_tcp_m); 

    // handle error cases (from beej's guide)
    if (p_tcp_m == NULL)  
    {
        fprintf(stderr, "serverM tcp: failed to bind\n");
        exit(1);
    }

    // listen() for incoming connection from the client (from beej's guide)
    if (listen(sockfd_tcp_m, BACKLOG) == -1) 
    {
        perror("listen");
        exit(1);
    }
    
    // prepare for accepting tcp connections from the client (from beej's guide)
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

    int new_fd; // new connection on new_fd
    struct sockaddr_storage their_addr_tcp; // connector's address information
    sin_size = sizeof their_addr_tcp;
    new_fd = accept(sockfd_tcp_m, (struct sockaddr *) &their_addr_tcp, &sin_size);
    if (new_fd == -1) 
    {
        perror("accept");
        // continue;
    }

    // set up finishes ///////////////////////////////////////////////////////////////////////////////////////////

    while (1) // main accept loop (from beej's guide)
    {  
        // cout << "dbg: main Server is ready for next iteration." << endl;

        // receive names sent from client via tcp (from beej's guide) (todo: make sure prepared)
        char names_buf[USERNAMES_BUF_SIZE];
        memset(names_buf, 0, sizeof(names_buf));
        if ((numbytes = recv(new_fd, names_buf, USERNAMES_BUF_SIZE - 1, 0)) == -1) 
        {
            perror("serverM tcp: recv");
            exit(1);
        }
        names_buf[numbytes] = '\0';

        // print correct on screen msg indicating the success of receiving usernames from client
        cout << "Main Server received the request from client using TCP over port " << TCP_PORT_M << "." << endl;

        // validate client input
        vector<string> invalid_users;
        vector<string> valid_users;
        vector<int> servers; // store the server each user belongs to (a=0, b=1) 
        validate_client_input(names_buf, invalid_users, valid_users, servers);

        // reply to client with a msg indicating validity of client input
        // TODO: if all usernames are invalid, buf is empty, or full of empty spaces

        // reply "fail" to client and keep requesting for valid usernames until at least one of them is valid
        if (valid_users.size() == 0) 
        {
            if(send(new_fd, "fail", 4, 0) == -1)
            {
                perror("serverM tcp: send");
            }
            continue;
            // cout << "dbg: continue does't work " << endl;
        }
        // if some users are invalid, reply a msg to client saying which usernames are invalid
        else if (invalid_users.size() > 0) 
        {
            char invalid_users_buf[USERNAMES_BUF_SIZE];
            memset(invalid_users_buf, 0, sizeof(invalid_users_buf));
            str_vec_to_buf(invalid_users, invalid_users_buf);
            if (send(new_fd, invalid_users_buf, strlen(invalid_users_buf), 0) == -1) 
            {
                perror("serverM tcp: send");
            }
            // print correct on screen msg after sending invalid usernames to client
            cout << invalid_users_buf << " do not exist. Send a reply to the client." << endl;
        }
        else // if all users are valid, reply "pass" to indicate all usernames exist
        {
            if (send(new_fd, "pass", 4, 0) == -1) 
            {
                perror("serverM tcp: send");
            }
        }

        // split them into two sublists based on where the user information is located
        vector<string> users_a;
        vector<string> users_b;
        for (int i = 0; i < valid_users.size(); i++) 
        {
            if (servers[i] == 0) 
            {
                users_a.push_back(valid_users[i]);
            }
            else if (servers[i] == 1) 
            {
                users_b.push_back(valid_users[i]);
            }
        }
        
        // forward valid usernames to the corresponding backend server via udp
        
        // if there are valid usernames for server A
        char users_a_buf[USERNAMES_BUF_SIZE];
        memset(users_a_buf, 0, sizeof(users_a_buf));
        str_vec_to_buf(users_a, users_a_buf);
        char times_buf_a[TIME_SLOTS_BUF_SIZE];
        memset(times_buf_a, 0, sizeof(times_buf_a));
        socklen_t addr_len_udp_a;
        addr_len_udp_a = sizeof addr_udp_a;
        vector<int> times_a;
        if (users_a.size() > 0) 
        {
            // send names managed by server A to server A (from beej's guide)
            if ((numbytes = sendto(sockfd_udp_m, users_a_buf, strlen(users_a_buf), 0, (struct sockaddr *) &addr_udp_a, sizeof addr_udp_a)) == -1) 
            {
                perror("serverM udp: sendto");
                exit(1);
            }
            // print correct on screen msg after sending usernames to server A
            if (numbytes > 0) {
                cout << "Found " << users_a_buf << " located at Server A. Send to Server A." << endl;
            }
            // receive timeslots from server A (from beej's guide)
            if ((numbytes = recvfrom(sockfd_udp_m, times_buf_a, TIME_SLOTS_BUF_SIZE - 1, 0, (struct sockaddr *) &addr_udp_a, &addr_len_udp_a)) == -1) 
            {
                perror("serverM udp: recvfrom");
                exit(1);
            }
            times_buf_a[numbytes] = '\0';
            // cout << "dbg: times buf a: " << times_buf_a << endl;
        }

        // if there are valid usernames for server B
        char users_b_buf[USERNAMES_BUF_SIZE];
        memset(users_b_buf, 0, sizeof(users_b_buf));
        str_vec_to_buf(users_b, users_b_buf);
        char times_buf_b[TIME_SLOTS_BUF_SIZE];
        memset(times_buf_b, 0, sizeof(times_buf_b));
        socklen_t addr_len_udp_b;
        addr_len_udp_b = sizeof addr_udp_b;
        vector<int> times_b;

        if (users_b.size() > 0) 
        {
            // send names managed by server B to server B (from beej's guide)
            if ((numbytes = sendto(sockfd_udp_m, users_b_buf, strlen(users_b_buf), 0, (struct sockaddr *) &addr_udp_b, sizeof addr_udp_b)) == -1) 
            {
                perror("serverM udp: sendto");
                exit(1);
            }
            // print correct on screen msg after sending usernames to server B
            if (numbytes > 0) 
            {
                cout << "Found " << users_b_buf << " located at Server B. Send to Server B." << endl;
            }
            // receive timeslots from server B (from beej's guide)
            if ((numbytes = recvfrom(sockfd_udp_m, times_buf_b, TIME_SLOTS_BUF_SIZE - 1, 0, (struct sockaddr *) &addr_udp_b, &addr_len_udp_b)) == -1) 
            {
                perror("serverM udp: recvfrom");
                exit(1);
            }
            times_buf_b[numbytes] = '\0';
            // cout << "dbg: times buf b: " << times_buf_b << endl;
        }

        // print correct on screen msg after receiving timeslots from server A
        if (users_a.size() > 0) {
            cout << "Main Server received from server A the intersection result using UDP over port " << UDP_PORT_M << ":" << endl;
            cout << "[";
            if (strcmp(times_buf_a, "empty") != 0) {
                times_a = buf_to_int_vec(times_buf_a);
                memset(times_buf_a, 0, sizeof(times_buf_a));
                if (times_a.size() != 0) 
                {
                    for (int i = 0; i < times_a.size(); i += 2) 
                    {
                        cout << "[" << times_a[i] << "," << times_a[i + 1] << "]";
                        // print "," if not the last element
                        if (i != times_a.size() - 2) 
                        {
                            cout << ",";
                        }
                    }
                }
            }
            cout << "]." << endl;
        }

        // print correct on screen msg after receiving timeslots from server B
        if (users_b.size() > 0) { 
            cout << "Main Server received from server B the intersection result using UDP over port " << UDP_PORT_M << ":" << endl;
            cout << "[";
            if (strcmp(times_buf_b, "empty") != 0) {
                times_b = buf_to_int_vec(times_buf_b);
                memset(times_buf_b, 0, sizeof(times_buf_b));
                if (times_b.size() != 0) 
                {
                    for (int i = 0; i < times_b.size(); i += 2) 
                    {
                        cout << "[" << times_b[i] << "," << times_b[i + 1] << "]";
                        // print "," if not the last element
                        if (i != times_b.size() - 2) 
                        {
                            cout << ",";
                        }
                    }
                }
            }
            cout << "]." << endl;
        }
            
        // run an algo to get the final time slots that works for all participants
        vector<int> intersects;
        if (users_a.size() == 0) 
        {
            intersects = times_b;
        }
        else if (users_b.size() == 0) 
        {
            intersects = times_a;
        }
        else {
            intersects = find_final_time_slots(times_a, times_b);
        }
        // cout << "dbg: intersects: ";
        // for (int i = 0; i < intersects.size(); i++) {
        //     cout << intersects[i] << " ";
        // }
        // cout << "." << endl;

        // print correct on screen msg after finding the final time slots
        cout << "Found the intersection between the results from server A and B:" << endl;
        cout << "[";
        if (intersects.size() != 0) {
            for (int i = 0; i < intersects.size(); i += 2) {
                cout << "[" << intersects[i] << "," << intersects[i + 1] << "]";
                // print "," if not the last element
                if (i != intersects.size() - 2) {
                    cout << ",";
                }
            }
            
            // send the result back to the client via tcp (from beej's guide)
            char intersects_buf[INTERSECTS_BUF_SIZE];
            memset(intersects_buf, 0, sizeof(intersects_buf));
            int_vec_to_buf(intersects, intersects_buf);
            if (send(new_fd, intersects_buf, strlen(intersects_buf), 0) == -1) 
            {
                perror("serverM: send");
            }
        }
        else {
            // send the result back to the client via tcp (from beej's guide)
            if (send(new_fd, "empty", 5, 0) == -1) 
            {
                perror("serverM: send");
            }
        }
        cout << "]." << endl;
        
        // print correct on screen msg after sending the final time slots
        // cout << "dbg: intersects_buf is " << intersects_buf << endl;
        cout << "Main Server sent the result to the client." << endl;

        // TODO: send valid usernames to client (from beej's guide)
        char valid_users_buf[CLIENT_MAXDATASIZE];
        str_vec_to_buf(valid_users, valid_users_buf);
        if (send(new_fd, valid_users_buf, strlen(valid_users_buf), 0) == -1) 
        {
            perror("client: send");
        }
    }

    return 0;
}