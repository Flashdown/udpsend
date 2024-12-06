# udpsend ![build](https://github.com/buttons/github-buttons/workflows/build/badge.svg) [![Github All Releases](https://img.shields.io/github/downloads/Flashdown/udpsend/total.svg)]()
udpsend is a simple udp sender tool written in C++ that allows to send a string/message via UDP to a specific IP and port.

## Download binary Windows release of udpsend V0.1 from here:
https://github.com/Flashdown/udpsend/releases/download/udpsend_0.1/udpsend.exe

## Usage:

```console
udpsend <server> <port> <message>
```

Example sending a string:
```console
udpsend myinvalidserver.invalid 1234 HelloWorld
```
Example sending a message:
```console
udpsend myinvalidserver.invalid 1234 'Hello World, does anyone read me?'
```
* server: IPv4, IPv6 address or FQDN
* port:   Port number to send the message to
* message: Message to send via UDP
