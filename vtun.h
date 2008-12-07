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
 * vtun.h,v 1.7.2.6 2001/12/29 17:01:01 bergolth Exp
 */ 
#ifndef _VTUN_H
#define _VTUN_H

#include "llist.h"

/* Default VTUN port */
#define VTUN_PORT 5000

/* Default VTUN connect timeout in sec */
#define VTUN_CONNECT_TIMEOUT 30

/* General VTUN timeout for several operations, in sec */
#define VTUN_TIMEOUT 30

/* Number of seconds for delay after pppd startup*/
#define VTUN_DELAY_SEC  10 

/* Statistic interval in seconds */
#define VTUN_STAT_IVAL  5*60  /* 5 min */

/* Max lenght of device name */
#define VTUN_DEV_LEN  20 
 
/* End of configurable part */

struct vtun_sopt {
    char *dev;
    char *laddr;
    int  lport;
    char *raddr;
    int  rport;
};

struct vtun_stat {
   unsigned long byte_in;
   unsigned long byte_out;
   unsigned long comp_in;
   unsigned long comp_out;
   FILE *file;
};

struct vtun_cmd {
   char *prog;
   char *args;
   int  flags;
};
/* Command flags */
#define VTUN_CMD_WAIT	0x01 
#define VTUN_CMD_DELAY  0x02
#define VTUN_CMD_SHELL  0x04

struct vtun_addr {
   char *name;
   char *ip;
   int port;
   int type;
};
/* Address types */
#define VTUN_ADDR_IFACE	0x01 
#define VTUN_ADDR_NAME  0x02

struct vtun_host {
   char *host;
   char *passwd;
   char *dev;

   llist up;
   llist down;

   int  flags;
   int  timeout;
   int  spd_in;
   int  spd_out;
   int  zlevel;

   int  rmt_fd;
   int  loc_fd;

   /* Persist mode */
   int  persist;

   /* Multiple connections */
   int  multi;

   /* Keep Alive */
   int ka_interval;
   int ka_failure;

   /* Source address */
   struct vtun_addr src_addr;

   struct vtun_stat stat;

   struct vtun_sopt sopt;
};

extern llist host_list;

/* Flags definitions */
#define VTUN_TTY        0x0100
#define VTUN_PIPE       0x0200
#define VTUN_ETHER      0x0400
#define VTUN_TUN        0x0800
#define VTUN_TYPE_MASK  (VTUN_TTY | VTUN_PIPE | VTUN_ETHER | VTUN_TUN) 

#define VTUN_TCP        0x0010  
#define VTUN_UDP        0x0020  
#define VTUN_PROT_MASK  (VTUN_TCP | VTUN_UDP) 
#define VTUN_KEEP_ALIVE 0x0040	

#define VTUN_ZLIB       0x0001
#define VTUN_LZO        0x0002
#define VTUN_SHAPE      0x0004
#define VTUN_ENCRYPT    0x0008

/* Mask to drop the flags which will be supplied by the server */
#define VTUN_CLNT_MASK  0xf000

#define VTUN_STAT	0x1000
#define VTUN_PERSIST    0x2000

/* Constants and flags for VTun protocol */
#define VTUN_FRAME_SIZE     2048
#define VTUN_FRAME_OVERHEAD 100
#define VTUN_FSIZE_MASK 0x0fff

#define VTUN_CONN_CLOSE 0x1000
#define VTUN_ECHO_REQ	0x2000
#define VTUN_ECHO_REP	0x4000
#define VTUN_BAD_FRAME  0x8000

/* Authentication message size */
#define VTUN_MESG_SIZE	50

/* Support for multiple connections */
#define VTUN_MULTI_DENY		0  /* no */ 
#define VTUN_MULTI_ALLOW	1  /* yes */
#define VTUN_MULTI_KILL		2

/* keep interface in persistant mode */
#define VTUN_PERSIST_KEEPIF     2

/* Values for the signal flag */

#define VTUN_SIG_TERM 1
#define VTUN_SIG_HUP  2

/* Global options */
struct vtun_opts {
   int  timeout;
   int  persist;

   char *cfg_file;

   char *shell; 	 /* Shell */
   char *ppp;		 /* Command to configure ppp devices */
   char *ifcfg;		 /* Command to configure net devices */
   char *route;		 /* Command to configure routing */
   char *fwall; 	 /* Command to configure FireWall */
   char *iproute;	 /* iproute command */

   char *svr_name;       /* Server's host name */
   int  svr_port;	 /* Server's port */
   int  svr_type;	 /* Server mode */
   int  syslog; 	 /* Facility to log messages to syslog under */
};
#define VTUN_STAND_ALONE	0 
#define VTUN_INETD		1	

extern struct vtun_opts vtun;

void server(int sock);
void client(struct vtun_host *host);
int  tunnel(struct vtun_host *host);
int  read_config(char *file);
struct vtun_host * find_host(char *host);

#endif
