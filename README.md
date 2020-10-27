# s-talk
SFU CMPT 300 Fall 2020 Assignment 2, a UDP two-way text chat.
Written by Trevor Bonas.

s-talk provides two-way local communication but only if the two
users know the other's ip and listening socket information
and firewalls don't get in the way (which they often do).

Arguments passed as ./s-talk (local port) (remote ip address) (remote port).

Linux supported and MacOS is supported with make macos.

All threads communcicate with one another using a static array of nodes and heads.
The number of heads and nodes can be edited in list.h.
Setting node number to one and having all lists share just that one node
is fully supported.
