# NCP
The goal of the project is to implement a TCP-like protocol over an UDP socket

## Tree hierarchy
- ./bin -> C executable, server and client
- ./src -> .c source files
- ./pres -> Our presentation slideshow
- ./files -> the files the server can acceed and distribute over the network.
- ./subject -> the project instructions and expectations .pdf
- ./obj -> .o C object files

## How to use :
Files serve by the server MUST be located in the "files" directory.

## How to write to file nicely
DO NOT use string specific fn because we gonna work on not string files !!

Use thing like
- int n = fread()
- int n = sendto (buffer, n+taille_no_seg)
- int n = recvfrom (...)
- memcopy ()

## Some optimization
If C or Java :
1. Read a piece of file and buffering
2. Send
3. receive and buffering
4. Write

!! Multiple client : 1 principal thread 3 way handshake : syn syn-ack+port ack then new thread which communicate on port

Start the thread BEFORE the synack+port or we loose the first packet !

## TODO
- --Calcul du débit--
- RTT to calcul the TimeOut
- How to change de mode ?
- Slow start
- Congestion avoidance
- NewReno

## Point to talk about during presentation
- What we get about the client...
- How we handle multiple client : 1 process per client + process to accept client
- How we parse the file -> all format in one Array
- Congestion windows : No slow start because random
- Explain that we hardcode the initial timeout instead of calculing it on the synack-ack because it was struggling for peanuts
- Explain the karn's algorithm for RTT calcul, the infinite loop problem on timeout and its fix, RTTd and RTTs



