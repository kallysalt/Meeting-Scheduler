#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <sstream>
#include <set>
#include <signal.h>

using namespace std;

typedef map<string,vector<pair< int,int> > > schedules;
typedef vector<pair< int,int> > schedule;

#define UDP_PORT_M "23687"
#define TCP_PORT_M "24687"
#define UDP_PORT_A "21687"
#define UDP_PORT_B "22687"
#define localhost "127.0.0.1"

#define BACKLOG 10 // how many pending connections queue will hold 

#define MAX_LEN_USERNAME 20
#define MAX_NUM_USER 100 
#define USERNAMES_BUF_SIZE (2 * (MAX_LEN_USERNAME * MAX_NUM_USER + MAX_NUM_USER)) // ?
#define MAX_NUM_INPUT 10
#define CLIENT_MAXDATASIZE (2 * (MAX_LEN_USERNAME * MAX_NUM_INPUT + MAX_NUM_INPUT)) // ?
#define TIME_SLOTS_BUF_SIZE 202 // ?
#define INTERSECTS_BUF_SIZE 402 // ?
#define INTERVAL_SIZE 20