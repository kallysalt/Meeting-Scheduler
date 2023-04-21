/* UDP is connectionless, so a single socket can be used to send and receive data */

#include "project.h"

// read input file and store the information in a data structure
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
            // get username
            string username = line.substr(0, line.find(";"));
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
            // handle extreme case
            if (bracket_idx.size() == 0) {
                pair<int, int> place_holder;
                sched[username].push_back(place_holder);
                continue;
            }
            // get time intervals
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

// convert schedules to a names buffer (names are separated by ', ')
void schedules_to_names_buf(schedules scheds, char *buf)
{
    int curr = 0;
    for (schedules::const_iterator it = scheds.begin(); it != scheds.end(); it++)
    {
        string key = it->first;
        for (int i = curr; i < curr + key.size(); i++) 
        {
            buf[i] = key[i - curr];
        }
        buf[curr + key.size()] = ',';
        buf[curr + key.size() + 1] = ' ';
        curr += key.size() + 2;
    }

    if (strlen(buf) == 0) // if scheds is empty, do nothing
    {
        return;
    }
    else // otherwise, remove the last ', '
    {
        buf[curr - 2] = '\0'; // remove the last ', '
    }
}

// convert a names buffer (names are separated by ', ') to a vector of strings
vector<string> names_buf_to_vec(char *buf)
{
    // TODO: what if names buffer is empty
    
    vector<string> names;
    char *name;
    name = strtok(buf, ", ");
    while (name != NULL) 
    {
        names.push_back(string(name));
        name = strtok(NULL, ", ");
    }
    return names;
}

// convert a vector of integers to a buffer (integers are separated by ' ')
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

// identify the time slots that are available for all the requested users
vector<int> find_intersection(vector<string> names, schedules &scheds) 
{
    // when there is no user
    if (names.size() == 0) 
    {
        return vector<int>();
    }
    
    // when there is at least one user, initialize the intersects vector
    vector<int> intersects;
    string name = names[0];
    schedule sched = scheds[name];

    for (int k = 0; k < sched.size(); k++) 
    {
        intersects.push_back(sched[k].first);
        intersects.push_back(sched[k].second);
    }

    // when there is only one user, return immediately
    if (names.size() == 1) 
    {
        return intersects;
    }

    // when there are more than one users, find the intersection iteratively
    for (int i = 1; i < names.size(); i++) 
    {
        name = names[i];
        sched = scheds[name];
        vector<int> new_intersects;

        for (int j = 0; j < intersects.size(); j += 2) 
        {
            int x_start = intersects[j];
            int x_end = intersects[j + 1];

            for (int k = 0; k < sched.size(); k++) {
                int y_start = sched[k].first;
                int y_end = sched[k].second;

                // check if there is an intersection
                if ((x_end > y_start && x_start < y_end) || (y_end > x_start && y_start < x_end))
                {
                    new_intersects.push_back(max(x_start, y_start));
                    new_intersects.push_back(min(x_end, y_end));
                }
            }
        }
        intersects.clear();
        intersects.insert(intersects.begin(), new_intersects.begin(), new_intersects.end());
    }
    return intersects;
}

int main(int argc, const char* argv[])
{
    // print boot up msg ////////////////////////////////////////////////////////////////////////////////////////////
    cout << "Server A is up and running using UDP on port " << UDP_PORT_A << "." << endl; 

    // read input file and store the information in a data structure
    schedules scheds = read_input_file("a.txt");

    // get itself's (server A udp port) address information (from beej's guide)
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

    // loop through all the results and make a udp socket (from beej's guide)
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
            perror("serverA udp: bind");
            continue;
        }
        break;
    }

    // handle error cases (from beej's guide)
    if (p_udp_a == NULL) 
    { 
        fprintf(stderr, "serverA talker: failed to create socket\n");
        return 2;
    }

    // free the linked-list (from beej's guide)
    freeaddrinfo(servinfo_udp_a); 

    // get receiver's (server M udp port) address information (from beej's guide)
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

    // store all usernames in a char buffer 
    char usernames[USERNAMES_BUF_SIZE];
    memset(usernames, 0, sizeof(usernames));
    schedules_to_names_buf(scheds, usernames);

    // send all usernames it has to server M via udp over specified port (from beej's guide)
    if ((sendto(sockfd, usernames, strlen(usernames), 0, servinfo_udp_m->ai_addr, servinfo_udp_m->ai_addrlen)) == -1) 
    {
        perror("serverA talker: sendto");
        exit(1);
    }

    // print correct on screen msg indicating the success of sending usernames to server M
    cout << "Server A finished sending a list of usernames to Main Server." << endl;

    // set up finishes //////////////////////////////////////////////////////////////////////////////////////////////

    while (1) 
    {
        // receive users from main server via udp over specified port (from beej's guide)
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
        cout << "dbg: received active users buf is " << buf << endl;

        // print correct on screen msg indicating the success of receiving usernames from the main server
        cout << "Server A received the usernames from Main Server using UDP over port " << UDP_PORT_A << "." << endl;

        // search in database to get all requested users' availability
        // make a copy of buf before calling strtok
        char names_buf[USERNAMES_BUF_SIZE];
        strcpy(names_buf, buf);
        vector<string> names = names_buf_to_vec(buf);

        // find the time intersection among them
        vector<int> intersects = find_intersection(names, scheds);

        // TODO: handle the case when one of the user's sched is empty
        if (intersects.size() == 2 && intersects[0] == 0 && intersects[1] == 0) {
            intersects.clear();
        }

        // format and print the result
        char intersects_buf[INTERSECTS_BUF_SIZE];
        int_vec_to_buf(intersects, intersects_buf);
        if (names.size() != 0) 
        {
            cout << "Found the intersection result: ";
            cout << "[";    

            for (int i = 0; i < intersects.size(); i += 2) {
                cout << "[" << intersects[i] << "," << intersects[i + 1] << "]";
                // print "," if not the last element
                if (i != intersects.size() - 2) 
                {
                    cout << ",";
                }
            }
            cout << "] for " << names_buf << "." << endl;
        }

        // send the result back to the main server (from beej's guide)
        if ((sendto(sockfd, intersects_buf, strlen(intersects_buf), 0, servinfo_udp_m->ai_addr, servinfo_udp_m->ai_addrlen)) == -1) 
        {
            perror("serverA talker: sendto");
            exit(1);
        }
        cout << "dbg: intersects_buf is " << intersects_buf << endl;

        // Print correct on screen msg indicating the success of sending the response to the main server
        cout << "Server A finished sending the response to Main Server." << endl;
    }

    // free the linked-list (from beej's guide)
    freeaddrinfo(servinfo_udp_m); 

    close(sockfd);
    return 0;
}
