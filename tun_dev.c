/*  
    VTun - Virtual Tunnel over TCP/IP network.

    Copyright (C) 1998-2000  Maxim Krasnyansky <max_mk@yahoo.com>

    VTun has been derived from VPPP package by Maxim Krasnyansky. 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

/*
 * tun_dev.c,v 1.1.2.4 2001/09/13 05:02:22 maxk Exp
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
 * Stores dev name in the first arg(must be large enough).
 * FIXME how large is large enough??
 */  
int tun_open_old(char *dev) {
  char tunname[14];
  int i, fd;

  if(*dev) {
    snprintf(tunname, sizeof(tunname), "/dev/%s", dev);
    return open(tunname, O_RDWR);
  }

  for(i=0; i < 255; i++){
    sprintf(tunname, "/dev/tun%d", i);
    /* Open device */
    if( (fd=open(tunname, O_RDWR)) > 0 ){
      sprintf(dev, "tun%d", i);
      return fd;
    }
  }
  return -1;
}

#include <linux/if_tun.h>

/* pre 2.4.6 compatibility */
#define OTUNSETIFF     (('T'<< 8) | 202) 

/*
 *FIXME what is dev supposed to contain?
 * current guess is /dev/tunX, and if it's null then I try to find one
 * when it comes back out, it's got the device /dev/whatever in it, so it had better be big enough, but I don't think we care about this?
 */
int tun_open(char *dev) {
  struct ifreq ifr;
  int fd;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
    return tun_open_old(dev);
  }

  memset(&ifr, 0, sizeof(ifr));

  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
    if (errno == EBADFD) {
      /* Try old ioctl */
      if (ioctl(fd, OTUNSETIFF, (void *) &ifr) < 0) 
        goto failed;
    } else
      goto failed;
  } 

  strcpy(dev, ifr.ifr_name);
  return fd;

 failed:
  close(fd);
  return -1;
}

int tun_close(int fd, char *dev) {
  return close(fd);
}

/* Read/write frames from TUN device */
int tun_write(int fd, char *buf, int len) {
  return write(fd, buf, len);
}

int tun_read(int fd, char *buf, int len) {
  return read(fd, buf, len);
}
