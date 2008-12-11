/*
    This file is part of ICMPTX

    itunnel - an ICMP tunnel by edi / teso
    VTun - Virtual Tunnel over TCP/IP network.

    Copyright (C) 1998-2000  Maxim Krasnyansky <max_mk@yahoo.com>
    Copyright (C) 2006       Thomer M. Gil <thomer@thomer.com>
    Copyright (C) 2008       John Plaxco <john@johnplaxco.com>
 
    Original author unknown, but modified by Thomer M. Gil who found the original
    code through
      http://www.linuxexposed.com/Articles/Hacking/Case-of-a-wireless-hack.html
      (a page written by Siim Põder).

    Code updated by John Plaxco, cleaned up and added polling support to survive stateful firewalls.
 
    The (old) icmptx website is at http://thomer.com/icmptx/
    The current icmptx website is hosted at github, http://github.com/jakkarth/icmptx
*/

#include "tun_dev.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int run_icmp_tunnel (int id, int packetsize, int isServer, char *serverNameOrIP, int tun_fd);

/* size of the largest icmp data payload to send, NOT MTU of tun device */
const int mtu = 65536;

#define USAGE "Usage: %s [-s|-c] server\n       -s Server Mode\n       -c Client Mode\n   server The host name or IP address of the server\n"

int main(int argc, char **argv) {
  int tun_fd = 0;

  if (argc != 3) {
    fprintf(stderr, USAGE, argv[0]);
    return 1;
  }
  if (strcmp(argv[1],"-c") && strcmp(argv[1],"-s")) {
    fprintf(stderr, USAGE, argv[0]);
    return 1;
  }

  if ((tun_fd = tun_open()) < 1) {
    fprintf(stderr, "Could not create tunnel device. Fatal.\n");
    return 1;
  }

  run_icmp_tunnel(7537, mtu, !strcmp(argv[1],"-s"), argv[2], tun_fd);

  /* when run_icmp_tunnel returns, we must be finished */
  tun_close(tun_fd);

  return 0;
}
