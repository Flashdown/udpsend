[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](#) [![Debian](https://img.shields.io/badge/Debian-A81D33?logo=debian&logoColor=fff)](#) [![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0)](#)
![build](https://github.com/buttons/github-buttons/workflows/build/badge.svg) [![Github All Releases](https://img.shields.io/github/downloads/Flashdown/udpsend/total.svg)]() [![Latest release](https://img.shields.io/github/v/release/Flashdown/udpsend?color=blue&label=latest%20release)](https://github.com/Flashdown/udpsend/releases/latest)

# udpsend 
udpsend is a simple udp sender tool written in C++ that allows to send a string or message via UDP to a specific IP and port.

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
