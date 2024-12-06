[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](#) [![Debian](https://img.shields.io/badge/Debian-A81D33?logo=debian&logoColor=fff)](#) [![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0)](#)
[![Build udpsend](https://github.com/Flashdown/udpsend/actions/workflows/build.yml/badge.svg)](https://github.com/Flashdown/udpsend/actions/workflows/build.yml) [![Github All Releases](https://img.shields.io/github/downloads/Flashdown/udpsend/total.svg)](https://github.com/Flashdown/udpsend/releases/latest) [![Latest release](https://img.shields.io/github/v/release/Flashdown/udpsend?color=blue&label=latest%20release)](https://github.com/Flashdown/udpsend/releases/latest)

# udpsend 
udpsend is a simple udp sender tool written in C++ that allows to send a string or message via UDP to a specific IP and port.

## udpsend v0.2 - Download latest binary releases

* Windows: https://github.com/Flashdown/udpsend/releases/download/udpsend_0.2/udpsend.exe
* Linux: https://github.com/Flashdown/udpsend/releases/download/udpsend_0.2/udpsend_linux
* MacOS: https://github.com/Flashdown/udpsend/releases/download/udpsend_0.2/udpsend_macos

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
udpsend myinvalidserver.invalid 1234 "Hello World, does anyone read me?"
```
* server: IPv4, IPv6 address or FQDN
* port:   Port number to send the message to
* message: Message to send via UDP
