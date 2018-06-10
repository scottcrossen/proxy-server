# Proxy Server

A simple proxy server

### Description

This project is a simple proxy server designed to handle requests built with the standard specifications of the system. A docker-build system is included for cross-platform development and isolation. It also faciliates the rebuilding process whenever a change is made to source files.

Many of the files contained in this repository also fulfill some of the requirements for the BYU course titled CS 324.
It is written and maintained by Scott Leland Crossen.

### Select Files

- proxy.c, csapp.h, csapp.c
  These are starter files.  csapp.c and csapp.h are described in
  the textbook. The files `port-for-user.pl' or 'free-port.sh' are used to generate
  unique ports for the proxy or tiny server.
- Makefile
  This is the makefile that builds the proxy program.  Type "make"
  to build your solution, or "make clean" followed by "make" for a
  fresh build. Type "make handin" to create the tarfile that will be handed in. The instructor will use your Makefile to build your proxy from source.
- port-for-user.pl
  Generates a random port for a particular user
  usage: ./port-for-user.pl <userID>
- free-port.sh
  Handy script that identifies an unused TCP port that you can use
  for your proxy or tiny.
  usage: ./free-port.sh
- driver.sh
  The autograder for Basic, Concurrency, and Cache.
  usage: ./driver.sh
- nop-server.py
  helper for the autograder.
- tiny
  Tiny Web server from the CS:APP text

### Contributors

1. Scott Leland Crossen  
<http://scottcrossen.com>  
<scottcrossen42@gmail.com>  
