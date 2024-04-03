# CS370 - FTP Server
### Intro

Due to the nature of C++ socketing, the both the server and client are built to run exclusively on Windows systems. 
### Commands
`login` - Allows the user to sign in through the client
`logout` - Logs the user out of current account
`ls` - Displays current directory on client
`cd [directory]` - Changes client directory
`ls_s` - Displays current directory in server
`cd_s [directory]` - Changes server directory
`upload [filename]` - Uploads [filename] to server from client
`download [filename]` - Downloads [filename] from server to client
`exit` - Closes client program
`shutdown` - Closes client and server program
