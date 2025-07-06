[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](#) [![Debian](https://img.shields.io/badge/Debian-A81D33?logo=debian&logoColor=fff)](#) [![macOS](https://img.shields.io/badge/macOS-000000?logo=apple&logoColor=F0F0F0)](#) [![Android](https://img.shields.io/badge/Android-3DDC84?logo=android&logoColor=fff)](https://play.google.com/store/apps/details?id=com.flashdown.udpsend)
[![Build udpsend](https://github.com/Flashdown/udpsend/actions/workflows/build.yml/badge.svg)](https://github.com/Flashdown/udpsend/actions/workflows/build.yml) [![Github All Releases](https://img.shields.io/github/downloads/Flashdown/udpsend/total.svg)](https://github.com/Flashdown/udpsend/releases/latest) [![Latest release](https://img.shields.io/github/v/release/Flashdown/udpsend?color=blue&label=latest%20release)](https://github.com/Flashdown/udpsend/releases/latest)

# udpsend 
udpsend is a simple UDP sender tool written in C++ that allows sending a string or message via UDP to a specific IP and port.

## udpsend v0.9 - Download latest binary releases

* **Windows**: https://github.com/Flashdown/udpsend/releases/download/udpsend_v0.9/udpsend.exe
* **Linux**: https://github.com/Flashdown/udpsend/releases/download/udpsend_v0.9/udpsend_linux
* **MacOS**: https://github.com/Flashdown/udpsend/releases/download/udpsend_v0.9/udpsend_macos
* **Android**: I have created udpsend as a free GUI based Android App as well: https://play.google.com/store/apps/details?id=com.flashdown.udpsend (not open source).

## Features
- Send messages/strings via UDP to a specified IP and port.
- Supports IPv4, IPv6, or FQDN as server address.
- Supports interactive mode for entering messages without using command-line arguments.
- IPv4/IPv6 selection with `-4` and `-6` flags to force address family.
- Hex input support with `-h` flag to send messages as hexadecimal strings (converted to binary).
- UTF-8 encoded command-line arguments on Windows.
- ICMP `-i`, send message as ICMP/ICMPv6 echo request payload (max 1472 bytes)

## ICMP Usage `-i`
Send message as ICMP/ICMPv6 echo request payload:
```console
udpsend -i [options] <server>
```

## Interactive Usage
Enter the message interactively by omitting the `<message>` argument:
```console
udpsend [options] <server> <port>
```

## Normal Usage
```console
udpsend [options] <server> <port> <message>
```
### Options
- `-4`: Force IPv4 address family.
- `-6`: Force IPv6 address family.
- `-h`: Send message as hexadecimal string (e.g., `48656C6C6F` for "Hello").
- `-i`: Send message as ICMP/ICMPv6 echo request payload (max 1472 bytes)

* **server**: IPv4, IPv6 address, or FQDN
* **port**: Port number to send the message to
* **message**: Message to send via UDP

### Examples
Sending a string:
```console
udpsend myinvalidserver.invalid 1234 HelloWorld
```
Sending a message:
```console
udpsend myinvalidserver.invalid 1234 "Hello World, does anyone read me?"
```
Sending a hexadecimal message:
```console
udpsend -h myinvalidserver.invalid 1234 48656C6C6F
```
Forcing IPv6:
```console
udpsend -6 myinvalidserver.invalid 1234 "Hello IPv6"
```
