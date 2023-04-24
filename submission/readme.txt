Full Name: Zhubo Zhou
Student ID: 6548633687
USC username: zhubozho

In the assignment, I've completed phase1, phase2, phase3, phase4, and the extra credit part. I developed a system to fasten the meeting scheduling process. There are one client, one main server and two backend servers (A and B).

My work contains 5 code files:
1. serverM.cpp
   - the main server >> manages which backend server a user’s info is stored in
   - receives user's request from the client
   - decides which backend server the participants’ availability is stored in
   - sends a request to each involved backend server to request valid time slot(s) for participants belonging to that backend server
   - receives valid time slot(s) from involved backend server(s)
   - merge results sent from involved backend server(s) to get the final time slot(s) that works for all participants
   - sends the result back to the client
   - receives final meeting time from the client (if any)
   - sends final scheduled meeting time to all involved backend server(s)
2. serverA.cpp: 
   - backend server A >> stores and manages the availability of all users in "a.txt"
   - receives request from the main server
   - searches in its database to get all requested users’ availability
   - finds the intersection among them and sends the result back to the main server
   - receives final meeting time from the main server (if any)
   - modifies its database after meeting has been scheduled (if any)
3. serverB.cpp: 
   - backend server B >> stores and manages the availability of all users in "b.txt"
   - receives request from the main server
   - searches in its database to get all requested users’ availability
   - finds the intersection among them and sends the result back to the main server
   - receives final meeting time from the main server (if any)
   - modifies its database after meeting has been scheduled (if any)
4. client.cpp: 
   - allows user to enter all names involved in the meeting
   - request the main server for the time intervals that works for every participant
   - receives available time recommendations from the main server
   - allows user to schedule the final meeting time in the involved users’ calendars
5. project.h：
   - contains common code (e.g. libraries to include, type definitions, global variables) shared by four cpp files

The format of all the messages exchanged:
- usernames are concatenated and delimited by a comma
- time intervals are concatenated and delimited by white spaces
  for example, [[1,2],[3,4]] will be transmitted as “1 2 3 4”
- final meeting time entered by user will be transmitted as "[start_time, end_time]"

Idiosyncrasy: none

Assumptions: 
- for phase 2, it is okay for client input to contain more than one consecutive white spaces
- for phase 2, if none of the names entered by the user is valid, the program will print "None of the usernames being entered is valid." 
- for the extra credit part, user must enter final meeting time in the form "[start_time, end_time]", otherwise might trigger underfined behavior.

Reused Code:
I used code from Beej’s Guide to Network Programming to get address information, make sockets, received and send messages. They are identified with comments in my source code.