# udpsend 
udpsend is a simple udp sender tool written in C++ that allows to send a string/message via UDP to a specific IP and port.

## Usage:

```console
udpsend.exe <server> <port> <message>
```

Example:
```console
udpsend.exe myinvalidserver.invalid 1234 HelloWorld
```
* server: IPv4, IPv6 address or FQDN
* port:   Port number to send the message to
* message: Message to send via UDP
