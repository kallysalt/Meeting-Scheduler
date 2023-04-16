/* UDP is connectionless, so a single socket can be used to send and receive data */

#include "project.h"

schedules read_input_file(const string &filename) 
{
    ifstream f;
    f.open(filename.c_str());
    schedules sched;

    if(f.is_open())
    {
        string raw_line;
        while(getline(f, raw_line))
        {
            // remove the extra spaces in your preprocessing
            string line;
            for (int i = 0; i < raw_line.size(); i++) {
                char c = raw_line[i];
                if (c != ' ') {
                    line.push_back(c);
                }
            }
            
            // store the information in a data structure
            // get indexes of '[' and ']'
            vector<int> bracket_idx;
            for(int i = 0; i < line.size(); i++)
            {
                if(line[i] == '[' || line[i] == ']')
                {
                    bracket_idx.push_back(i);
                }
            }
            bracket_idx.erase(bracket_idx.begin());
            bracket_idx.erase(bracket_idx.end() - 1);
            
            // get time intervals
            string username = line.substr(0, line.find(";"));
            for(int i = 0; i < bracket_idx.size(); i += 2)
            {
                string timeval = line.substr(bracket_idx[i] + 1, bracket_idx[i + 1] - bracket_idx[i] - 1);
                int start_time = atoi((timeval.substr(0, timeval.find(","))).c_str());
                int end_time = atoi((timeval.substr(timeval.find(",") + 1, timeval.size() - timeval.find(","))).c_str());
                sched[username].push_back(make_pair(start_time, end_time));
            }
        }
    }
    f.close();
    return sched;
}

void schedules_to_buf(schedules sched, char *buf)
{
    int curr = 0;
    for (schedules::const_iterator it = sched.begin(); it != sched.end(); it++)
    {
        string key = it->first;
        for (int i = curr; i < curr + key.size(); i++) {
            buf[i] = key[i - curr];
        }
        buf[curr + key.size()] = ' ';
        curr += key.size() + 1;
    }
    buf[curr - 1] = '\0'; // ?
}

vector<string> buf_to_vec(char *buf)
{
    vector<string> names;
    char *name;
    name = strtok(buf, " ");
    while (name != NULL) 
    {
        names.push_back(string(name));
        name = strtok(NULL, " ");
    }
    return names;
}

int main(int argc, const char* argv[])
{
    // print boot up msg
    cout << "Server A is up and running using UDP on port " << UDP_PORT_A << "." << endl; 

    // read input file and store the information in a data structure
    schedules scheds = read_input_file("a.txt");

    // get itself's (server A udp port) address information
    struct addrinfo hints_udp_a, *servinfo_udp_a;
    int rv_udp_a;
    memset(&hints_udp_a, 0, sizeof hints_udp_a);
    hints_udp_a.ai_family = AF_INET; 
    hints_udp_a.ai_socktype = SOCK_DGRAM;
    hints_udp_a.ai_flags = AI_PASSIVE; 
    if ((rv_udp_a = getaddrinfo(localhost, UDP_PORT_A, &hints_udp_a, &servinfo_udp_a)) != 0) 
    {
        fprintf(stderr, "serverA talker getaddrinfo: %s\n", gai_strerror(rv_udp_a));
        return 1;
    }

    // loop through all the results and make a udp socket
    int sockfd;
    struct addrinfo *p_udp_a;
    
    for (p_udp_a = servinfo_udp_a; p_udp_a != NULL; p_udp_a = p_udp_a->ai_next) 
    {
        if ((sockfd = socket(p_udp_a->ai_family, p_udp_a->ai_socktype, p_udp_a->ai_protocol)) == -1) 
        {
            perror("serverA talker: socket");
            continue;
        }
        if (bind(sockfd, p_udp_a->ai_addr, p_udp_a->ai_addrlen) == -1) 
        {
            close(sockfd);
            perror("serverM udp: bind");
            continue;
        }
        break;
    }

    // handle error cases
    if (p_udp_a == NULL) 
    { 
        fprintf(stderr, "serverA talker: failed to create socket\n");
        return 2;
    }

    // free the linked-list
    freeaddrinfo(servinfo_udp_a); 
    
    // store all usernames in a char buffer 
    char usernames[USERNAMES_BUF_SIZE];
    memset(usernames, 0, sizeof(usernames));
    schedules_to_buf(scheds, usernames);

    // get receiver's (server M udp port) address information
    struct addrinfo hints_udp_m, *servinfo_udp_m;
    int rv_udp_m;
    memset(&hints_udp_m, 0, sizeof hints_udp_m);
    hints_udp_m.ai_family = AF_INET; 
    hints_udp_m.ai_socktype = SOCK_DGRAM;
    hints_udp_m.ai_flags = AI_PASSIVE; 
    if ((rv_udp_m = getaddrinfo(localhost, UDP_PORT_M, &hints_udp_m, &servinfo_udp_m)) != 0) 
    {
        fprintf(stderr, "serverA talker getaddrinfo: %s\n", gai_strerror(rv_udp_m));
        return 1;
    }

    // send all usernames it has to server M via udp over specified port
    if ((sendto(sockfd, usernames, strlen(usernames), 0, servinfo_udp_m->ai_addr, servinfo_udp_m->ai_addrlen)) == -1) 
    {
        perror("serverA talker: sendto");
        exit(1);
    }

    // print correct on screen msg indicating the success of sending usernames to server M
    cout << "Server A finished sending a list of usernames to Main Server." << endl;

    // receive users from main server via udp over specified port
    char buf[USERNAMES_BUF_SIZE];
    memset(buf, 0, sizeof(buf));
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    addr_len = sizeof their_addr;
    int numbytes;
    if ((numbytes = recvfrom(sockfd, buf, USERNAMES_BUF_SIZE - 1, 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
        perror("serverA talker: recvfrom");
        exit(1);
    }
    buf[numbytes] = '\0';

    // print correct on screen msg indicating the success of receiving usernames from the main server
    if (numbytes != 0) {
        cout << "Server A received the usernames from Main Server using UDP over port " << UDP_PORT_A << "." << endl;
    } 

    // search in database to get all requested users' availability
    vector<string> names = buf_to_vec(buf);


    // // find the times intersection among them
    // cout << "Found the intersection result: <[[t1_start, t1_end], [t2_start, t2_end], ... ]> for <username1, username2, ...>." << endl;

    // // send the result back to the main server
    // cout << "Server A finished sending the response to Main Server." << endl;

    // free the linked-list
    freeaddrinfo(servinfo_udp_m); 

    close(sockfd);
    return 0;
}
