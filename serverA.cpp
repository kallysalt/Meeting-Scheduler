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

void schedules_to_buffer(schedules sched, char *buf)
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

int main(int argc, const char* argv[])
{
    // print boot up msg
    cout << "ServerA is up and running using UDP on port " << UDP_PORT_A << "." << endl; // ?

    // read input file and store the information in a data structure
    schedules a = read_input_file("a.txt");

    // get receiver's (server M udp port) address information
    struct addrinfo hints, *servinfo, *p;
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 
    if ((rv = getaddrinfo(localhost, UDP_PORT_M, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "serverA talker getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
        {
            perror("serverA talker: socket");
            continue;
        }
        break;
    }
    // handle error cases
    if (p == NULL) 
    { 
        fprintf(stderr, "serverA talker: failed to create socket\n");
        return 2;
    }
    // free the linked-list
    freeaddrinfo(servinfo); 
    
    // store all usernames in a char buffer 
    char usernames[USERNAMES_BUF_SIZE];
    schedules_to_buffer(a, usernames);
    

    // send all usernames it has to the main server via UDP over specified port
    if ((sendto(sockfd, usernames, strlen(usernames), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        cout << (sendto(sockfd, usernames, strlen(usernames), 0, p->ai_addr, p->ai_addrlen)) << endl;
        perror("serverA talker: sendto");
        exit(1);
    }

    // print correct on screen msg indicating the success of sending usernames to the main server
    cout << "ServerA finished sending a list of usernames to Main Server." << endl;

    close(sockfd);
    return 0;
}
