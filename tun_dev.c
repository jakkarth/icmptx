/*  
    This file is part of ICMPTX

    VTun - Virtual Tunnel over TCP/IP network.

    Copyright (C) 1998-2000  Maxim Krasnyansky <max_mk@yahoo.com>
    Copyright (C) 2006       Thomer M. Gil <thomer@thomer.com>
    Copyright (C) 2008       John Plaxco <john@johnplaxco.com>

    VTun has been derived from VPPP package by Maxim Krasnyansky. 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this ICMPTX.  If not, see <http://www.gnu.org/licenses/>.

*/

/* #include "config.h" */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

int snprintf(char *str, size_t size, const char *format, ...);

/* 
 * Allocate TUN device, returns opened fd. 
 */  
int tun_open_old() {
  char tunname[14];
  int i, fd;

  for(i=0; i < 255; i++){
    sprintf(tunname, "/dev/tun%d", i);
    /* Open device */
    if( (fd=open(tunname, O_RDWR)) > 0 ){
      return fd;
    }
  }
  return -1;
}

#include <linux/if_tun.h>

/* pre 2.4.6 compatibility */
#define OTUNSETIFF     (('T'<< 8) | 202) 

/*
 * find a tun device and open it, returning the file descriptor on success or -1 on failure with errno possibly set.
 */
int tun_open() {
  struct ifreq ifr;
  int fd;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
    return tun_open_old();
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
    if (errno == EBADFD) {
      /* Try old ioctl */
      if (ioctl(fd, OTUNSETIFF, (void *) &ifr) < 0) 
        goto failed;
    } else
      goto failed;
  } 

  return fd;

 failed:
  close(fd);
  return -1;
}

int tun_close(int fd) {
  return close(fd);
}

int tun_write(int fd, char *buf, int len) {
  return write(fd, buf, len);
}

int tun_read(int fd, char *buf, int len) {
  return read(fd, buf, len);
}
