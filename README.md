# software-defined-routing
Demonstrates software defined routing using the distance vector protocol to update forwarding tables between routers.  
  
**NOTE: Controller Program was not written by me. This program virtualizes a controller used for SDN routing. It sends and receives messages from the routers via a TCP socket communication.  

# Compile  
There is an included makefile which will compile the program  
# Starting the router  
The program takes one argument - the port to start the router on. This port will be the port used by the controller to initiate a TCP connection with the router.  
# Controller Information  
The controller can send multiple different commands use ./controller -h  
The first command that should be sent once all routers are up is the init command, this will initialize all routers. From here, the distance vector protocol will begin and after a few rounds all routers will have the updated shortest path to all other routers in the graph.  
# Topology files  
The controller must take in a topology file argument for the INIT command. The topology file contains the graph of the network. It should contain the following.  
Line 1 - Number of routers (N)  
Line 2 - N: Each line will represent a router with the following information <ID> <IP> <Controller Port> <Router Port> <Data Port>
Line 3 - ?? : Each line will contain router link costs. Example 1 2 4. This means  link 1 to 2 has cost 4.  
  
Take a look at the example.topology file and the test.topology for details.  

*Controller Port: This is the port the router needs to be started on. The router will listen on this port for the controller to connect.  
*Router Port: This is the UDP port used for other routers to send messages to that router.
*Data Port: This is the TCP port the router will listen on for any routers trying to connect and send packets.  

** Note the data plane of this program is not implemented. Currently only router distance vector updates and controller communication is working.
