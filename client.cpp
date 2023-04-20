/* a tcp client >> request time availability of all users in the meeting */

#include "project.h"

// get sockaddr (IPv4)
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

// get socket port number (IPv4)
void get_in_port(struct sockaddr_storage &their_addr, char *port) 
{
    sockaddr *sa = (struct sockaddr *) &their_addr;
    uint16_t port_num = ntohs(((struct sockaddr_in *) sa)->sin_port);
    sprintf(port, "%u", port_num);
}

// convert a times buffer (times are separated by ' ') to a vector of strings
vector<string> buf_to_string_vec(char *buf)
{
    vector<string> times;
    char *time;
    time = strtok(buf, " ");
    while (time != NULL) 
    {
        times.push_back(string(time));
        time = strtok(NULL, " ");
    }
    return times;
}

int main(int argc, const char* argv[]){

    // get server M's tcp port's address information ////////////////////////////////////////////////////////////////
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

    // TODO: assign dyanmic port address to the client's tcp socket
    struct sockaddr_storage my_addr;
    socklen_t my_addrlen = sizeof my_addr;
    int getsock_check = getsockname(sockfd, (struct sockaddr*) &my_addr, (socklen_t *) &my_addrlen);

    // get port number
    char tcp_port_client[10];
    get_in_port(my_addr, tcp_port_client);

    // error checking
    if (getsock_check== -1) {
        perror("getsockname");
        exit(1);
    }

    // all done with this structure
    freeaddrinfo(servinfo); 

    // TODO: receive 'ok' from the main server?
    char buf[20];
    int numbytes;
    if ((numbytes = recv(sockfd, buf, 19, 0)) == -1)
    {
        perror("client: recv");
        exit(1);
    }
    buf[numbytes] = '\0';

    // after servers are booted up and required usernames are transferred 
    // from backend servers to the main server, the client will be started
    // print boot up msg
    cout << "Client is up and running." << endl;

    // set up finishes //////////////////////////////////////////////////////////////////////////////////////////////

    // wait for the usernames to be taken as inputs from the user 
    // user can enter up to 10 usernames, all of which are separated by a single space
    int flag = 1;
    string input;
    while (flag) 
    {
        // show a prompt: Please enter the usernames to check schedule availability:
        cout << "Please enter the usernames to check schedule availability:" << endl;
        getline(cin, input);

        // TODO: if the none of the input is valid, keep requesting for client input

        flag = 0;
    }

    // send these names to the main server over tcp
    if (send(sockfd, input.c_str(), input.length(), 0) == -1)
    {
        perror("client: send");
        exit(1);
    }

    // print on screen msg after sending usernames to the main server
    cout << "Client finished sending the usernames to Main Server." << endl;

    // receive a msg saying which usernames do not exist from the main server over tcp
    char invalid_buf[USERNAMES_BUF_SIZE];
    if ((numbytes = recv(sockfd, invalid_buf, USERNAMES_BUF_SIZE - 1, 0)) == -1)
    {
        perror("client: recv");
        exit(1);
    }
    invalid_buf[numbytes] = '\0';

    // ASK: no print out if all exists
    // if there are usernames that do not exist, print: <username1, username2, ...> do not exist
    if (strcmp(invalid_buf, "none") != 0) {
        cout << "Client received the reply from Main Server using TCP over port " << tcp_port_client << ":" << endl;
        cout << invalid_buf << " do not exist." << endl;
    }

    // receive time availability of all users in the meeting from the main server over tcp
    char buf3[INTERSECTS_BUF_SIZE];
    if ((numbytes = recv(sockfd, buf3, USERNAMES_BUF_SIZE - 1, 0)) == -1)
    {
        perror("client: recv");
        exit(1);
    }
    buf3[numbytes] = '\0';

    vector<string> intersects = buf_to_string_vec(buf3);
    
    // print on screen msg after receiving availability of all users in the meeting from the main server
    cout << "Client received the reply from Main Server using TCP over port " << tcp_port_client << ":" << endl;
    cout << "Time intervals [";
    if (intersects.size() != 0) {
        for (int i = 0; i < intersects.size(); i += 2) {
            cout << "[" << intersects[i] << "," << intersects[i + 1] << "]";
            // print "," if not the last element
            if (i != intersects.size() - 2) {
                cout << ",";
            }
        }
    }
    cout << "]" << " works for " << "<username1, username2, ...>" << "." << endl;
    
    // start a new request 
    cout << "-----Start a new request-----" << endl;
    cout << "Please enter the usernames to check schedule availability:" << endl;

    close(sockfd);
    return 0;
}

