# GetMyMusic

# Description:

GetMyMusic is a file/music synchronization application
that enables multiple machines to ensure that they have the same files in their music directory.



## Structure: 

We put the song into the data.dat files. server and client would have different databse files.  

### Server: 

server would wait for the client to respond, in orderto handle the muti-Clients, we used Pthread_detach function. 

