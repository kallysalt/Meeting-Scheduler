- developed a system to fasten the meeting scheduling process
- there are one client, one main server and two backend servers (A and B) in our project
- server A and server B each can receive a list of names involved in the meeting
- the availability of all users is stored on backend servers and cannot be accessed by any other servers or clients
- when a user wants to schedule a meeting among a group of people:
    - user will start a client program, enter all names involved in the meeting and request the main server for the time intervals that works for every participant
    - once the main server receives the request, it decides which backend server the participants’ availability is stored in and sends a request to the responsible backend server to request time intervals that works for all participants belonging to this backend server
    - once the backend server receives the request from the main server, it searches in the database to get all requested users’ availability, runs an algorithm to find the intersection among them and sends the result back to the main server
    - the main server receives the time slots from different backend servers, runs an algorithm to get the final time slots that works for all participants, and sends the result back to the client. The scheduler then can decide the final meeting time according to the available time recommendations and schedule it in the involved users’ calendars
- main server manages which backend server a user’s info is stored in
- the main server also plays a role of handling requests from clients

references: beej

additional file: project.h, which is used to share common code (e.g. libraries to include, type definitions, global variables) between different files

assumption: okay for client input to contain more than one consecutive white spaces