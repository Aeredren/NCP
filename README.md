# TCPoUDP
The goal of the project is to implement a TCP-like protocol over an UDP socket

## Tree hierarchy
- ./bin -> C executable, server and client
- ./src -> .c source files
- ./pres -> Our presentation slideshow
- ./files -> the files the server can acceed and distribute over the network.
- ./subject -> the project instructions and expectations .pdf
- ./obj -> .o C object files

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
