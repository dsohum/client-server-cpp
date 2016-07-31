# client-server-cpp


Instructions
----------------------------------------------------------------------------------------------
1. Go to project folder, do 
make clean
make
2. run server/ user/ worker executables using following format

./server <server-port>

./worker <server ip/host-name> <server-port>

./user <server ip/host-name> <server-port> <hash> <passwd-length> <binary-string>

The config.txt file is used to specify the IP address to which Server binds.



Server Side Documentation
----------------------------------------------------------------------------------------------------------
Server uses the config file to get the IP address and takes the Port No. as input for binding.
It creates a socket "listener" to listen to users and workers and accepts connections from 
users and workers.
It identifies the other host as user/worker and redirects the request of the user to the workers,
 dividing the work by equally partitioning the search domain for each worker. As soon as a 
worker cracks the password the user is retuened the password and all the workers assigned to 
theis user are stopped. When all workers are not free, as soon as aleast half of them are free user's
 request is forwarded to these free workers. Till then the requests of the user is maintained in a queue.
(Service is First Come First Serve)
Server sends every worker the message of the user along with user's socket-file-descriptor and the 
start and end character position specifying the search domain.

Initialize function initializes the FD_SETS, addresses. Rest of the fun is in main function.
(The code has been throughly commented.)


