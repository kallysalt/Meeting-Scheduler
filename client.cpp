/* a tcp client >> request time availability of all users in the meeting */

#include "project.h"

// get sockaddr (IPv4) (from beej's guide)
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

// get socket port number (IPv4) (from beej's guide)
void get_in_port(struct sockaddr_storage &their_addr, char *port) 
{
    sockaddr *sa = (struct sockaddr *) &their_addr;
    uint16_t port_num = ntohs(((struct sockaddr_in *) sa)->sin_port);
    sprintf(port, "%u", port_num);
}

// convert a buffer (separated by ' ') to a vector of strings
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

// convert a names buffer (names are separated by ', ') to a set of strings
set<string> buf_to_string_set(char *buf)
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

int main(int argc, const char* argv[]){

    // print boot up msg ( after servers are booted up backedn servers sent usernames to the main server?) //////////
    cout << "Client is up and running." << endl;
    
    // get server M's tcp port's address information (from beej's guide)
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

    // loop through all the results and connect to the first we can (from beej's guide)
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

    // handle error cases (from beej's guide)
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

    // error checking (from beej's guide)
    if (getsock_check== -1) {
        perror("getsockname");
        exit(1);
    }

    // all done with this structure (from beej's guide)
    freeaddrinfo(servinfo); 

    // set up finishes //////////////////////////////////////////////////////////////////////////////////////////////

    // wait for the usernames to be taken as inputs from the user (from beej's guide)
    // user can enter up to 10 usernames, all of which are separated by a single space
    string input;
    while (1) 
    {
        // show a prompt: Please enter the usernames to check schedule availability:
        cout << "Please enter the usernames to check schedule availability:" << endl;
        getline(cin, input);

        // send these names to the main server over tcp (from beej's guide)
        if (send(sockfd, input.c_str(), input.length(), 0) == -1)
        {
            perror("client: send");
            exit(1);
        }

        // print on screen msg after sending usernames to the main server
        cout << "Client finished sending the usernames to Main Server." << endl;

        // receive a msg saying which usernames do not exist from the main server over tcp (from beej's guide)
        char invalid_buf[USERNAMES_BUF_SIZE];
        memset(invalid_buf, 0, sizeof(invalid_buf));
        int numbytes;
        if ((numbytes = recv(sockfd, invalid_buf, USERNAMES_BUF_SIZE - 1, 0)) == -1)
        {
            perror("client: recv");
            exit(1);
        }
        invalid_buf[numbytes] = '\0';

        // TODO: if none of the usernames is valid, stop this iteration to request for another client input
        if (strcmp(invalid_buf, "fail") == 0) 
        {
            // ASK: print some error msg?
            continue;
        }

        // if some of the usernames do not exist, print: <username1, username2, ...> do not exist
        if (strcmp(invalid_buf, "pass") != 0) {
            cout << "Client received the reply from Main Server using TCP over port " << tcp_port_client << ":" << endl;
            cout << invalid_buf << " do not exist." << endl;
        }

        // wait for time availability of all users in the meeting from the main server over tcp (from beej's guide)
        char intersects_buf[INTERSECTS_BUF_SIZE];
        memset(intersects_buf, 0, sizeof(intersects_buf));
        if ((numbytes = recv(sockfd, intersects_buf, USERNAMES_BUF_SIZE - 1, 0)) == -1)
        {
            perror("client: recv");
            exit(1);
        }
        intersects_buf[numbytes] = '\0';

        // print on screen msg after receiving availability of all users in the meeting from the main server
        cout << "Client received the reply from Main Server using TCP over port " << tcp_port_client << ":" << endl;
        cout << "Time intervals [";
        if (strcmp(intersects_buf, "empty") != 0) 
        {
            vector<string> intersects = buf_to_string_vec(intersects_buf);
            for (int i = 0; i < intersects.size(); i += 2) 
            {
                cout << "[" << intersects[i] << "," << intersects[i + 1] << "]";
                // print "," if not the last element
                if (i != intersects.size() - 2) {
                    cout << ",";
                }
            }
        }
        cout << "]";

        // print valid names
        // parse input, store in a vector
        char input_buf [USERNAMES_BUF_SIZE];
        strcpy(input_buf, input.c_str());
        vector<string> input_names = buf_to_string_vec(input_buf);
        // parse invalid puf, store in a set, if name is not in the invalid set, print it
        cout << " works for ";
        if (strcmp(invalid_buf, "pass") == 0) 
        {
            for (int i = 0; i < input_names.size(); i++) 
            {
                cout << input_names[i];
                // print "," if not the last element
                if (i != input_names.size() - 1) {
                    cout << ", ";
                }
            }
        }
        else {
            set<string> invalid_names;
            invalid_names = buf_to_string_set(invalid_buf);
            int num_valid_names = input_names.size() - invalid_names.size();
            for (int i = 0; i < input_names.size(); i++) 
            {
                if (invalid_names.find(input_names[i]) == invalid_names.end()) 
                {
                    cout << input_names[i];
                    // print "," if not the last element
                    if (i != num_valid_names - 1) {
                        cout << ", ";
                    }
                }
            }
        }
        cout << "." << endl;

        // start a new request 
        input.clear();
        cout << "-----Start a new request-----" << endl;
    }

    close(sockfd);
    return 0;
}