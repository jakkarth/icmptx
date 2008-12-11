/*
    This file is part of ICMPTX

    Original code copyright date unknown, edi/teso.
    Copyright (C) 2006       Thomer M. Gil <thomer@thomer.com>
    Copyright (C) 2008       John Plaxco <john@johnplaxco.com>

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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "tun_dev.h"

/*
 * standard ICMP header
 */
struct icmp {
  u_int8_t	type;
  u_int8_t	code;
  u_int16_t	cksum;
  u_int16_t	id;
  u_int16_t	seq;
};

/*
 * standard IP header
 */
struct ip {
  unsigned int	ip_hl:4,
    ip_v:4;			
  unsigned char	ip_tos;			
  unsigned short	ip_len;			
  unsigned short	ip_id;			
  unsigned short	ip_off;			
  unsigned char	ip_ttl;			
  unsigned char	ip_p;			
  unsigned short	ip_sum;			
  struct	in_addr ip_src,ip_dst;
};

unsigned short in_cksum(unsigned short *, int);

/* int sock - ICMP socket used to communicate
   int proxy - 0 means send echo requests, 1 means send echo replies
   struct sockaddr_in *target - For the client, points to the server. For the server, the default place to send replies
   int tun_fd - Input/output file descriptor
   int packetsize - maximum size of ICMP data payload
   u_int16_t id - tunnel id field
*/
int icmp_tunnel(int sock, int proxy, struct sockaddr_in *target, int tun_fd, int packetsize, u_int16_t id) {
  int len, result, fromlen, num;
  char* packet;
  fd_set fs;
  unsigned char didSend, didReceive;
  struct icmp *icmp, *icmpr;
  struct timeval tv;
  struct sockaddr_in from;

  len = sizeof (struct icmp);

  if ((packet = malloc (len+packetsize)) == NULL) {
    fprintf(stderr, "Error allocating packet buffer");
    exit(1);
  }
  memset (packet, 0, len+packetsize);

  icmp = (struct icmp*)(packet);
  icmpr = (struct icmp*)(packet+sizeof(struct ip));

  /* here's the infinite loop that shovels packets back and forth while the tunnel's up */
  while (1) {
    FD_ZERO (&fs);
    FD_SET (tun_fd, &fs);
    FD_SET (sock, &fs);

    didSend = didReceive = 0;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    select (tun_fd>sock?tun_fd+1:sock+1, &fs, NULL, NULL, &tv);/* block until data's available in one direction or the other, or it's time to poll */

    /* data available on tunnel device, need to transmit over icmp */
    if (FD_ISSET(tun_fd, &fs)) {
      result = tun_read(tun_fd, packet+len, packetsize);
      if (!result) {/*eof*/
        return 0;
      } else if (result==-1) {
        perror("read");
        return -1;
      }
      icmp->type = proxy ? 0 : 8;/*echo response or echo request*/
      icmp->code = 0;
      icmp->id = id;/* mark the packet so the other end knows it's a tunnel packet */
      icmp->seq = 0;
      icmp->cksum = 0;
      icmp->cksum = in_cksum((unsigned short*)packet, len+result);
      result = sendto(sock, (char*)packet, len+result, 0, (struct sockaddr*)target, sizeof (struct sockaddr_in));
      if (result==-1) {
        perror ("sendto");
        return -1;
      }
      didSend = 1;
    }

    /* data available on socket from icmp, need to pass along to tunnel device */
    if (FD_ISSET(sock, &fs)) {
      fromlen = sizeof (struct sockaddr_in);
      num = recvfrom(sock, packet, len+packetsize, 0, (struct sockaddr*)&from, (socklen_t*) &fromlen);

      /* make the destination be the source of the most recently received packet (this can be dangerous) */
      memcpy(&(target->sin_addr.s_addr), &(from.sin_addr.s_addr), 4*sizeof(char));
      if (icmpr->id == id) {/*this filters out all of the other ICMP packets I don't care about*/
        tun_write(tun_fd, packet+sizeof(struct ip)+sizeof(struct icmp), num-sizeof(struct ip)-sizeof(struct icmp));
        didReceive = 1;
      } else if (icmpr->type == 8) {/* some normal ping request */
        icmpr->type = 0;/*echo response*/
        icmpr->code = 0;
        icmpr->id = icmpr->id;
        icmpr->seq = icmpr->seq;
        icmpr->cksum = 0;
        icmpr->cksum = in_cksum((unsigned short*)packet, num);
        result = sendto(sock, (char*)packet+sizeof(struct ip), len+num-sizeof(struct ip)-sizeof(struct icmp), 0, (struct sockaddr*)target, sizeof (struct sockaddr_in));
      }
    }    /* end of data available */

    /*
     * if we didn't send or receive anything, the select timed out
     * so lets send an echo request poll to the server (helps with
     * stateful firewalls)
     */
    if (!proxy && !didSend && !didReceive) {
      icmp->type = 8;/*echo request*/
      icmp->code = 0;
      icmp->id = id;/*mark the packet so the other end knows it's a tunnel packet*/
      icmp->seq = 0;
      icmp->cksum = 0;
      icmp->cksum = in_cksum((unsigned short*)packet, len);
      result = sendto(sock, (char*)packet, len, 0, (struct sockaddr*)target, sizeof (struct sockaddr_in));
      if (result==-1) {
        perror ("sendto");
        return -1;
      }
    }
  }  /* end of while(1) */

  return 0;
}

/*
 * this is the function that starts it all rolling
 * id - the id value for the icmp stream, to distinguish it from any other incoming ICMP packets
 * packetsize - I think this is the mtu value for the packets going across the tunnel, seems to be used in buffer allocations
 * isServer - 0 for client mode, 1 for server mode
 * serverNameOrIP - the server's host name or IP address
 * tun_fd - the file descriptor of the socket we read and write from
 */
int run_icmp_tunnel (int id, int packetsize, int isServer, char *serverNameOrIP, int tun_fd) {
  struct sockaddr_in target;
  struct in_addr inp;
  int s;

  if (!inet_aton(serverNameOrIP, &inp)) {
    struct hostent* he;
    if (!(he = gethostbyname (serverNameOrIP))) {
      herror ("gethostbyname");
      return -1;
    }
    memcpy (&target.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
  } else {
    target.sin_addr.s_addr = inp.s_addr;
  }
  target.sin_family = AF_INET;

  if ( (s = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
    perror ("socket");
    return -1;
  }

  icmp_tunnel(s, isServer, &target, tun_fd, packetsize, (u_int16_t) id);

  close(s);

  return 0;
}

/*
 * calculate the icmp checksum for the packet, including data
 */
unsigned short in_cksum (unsigned short *addr, int len) {
  int nleft = len;
  unsigned short *w = addr;
  int sum = 0;
  unsigned short answer = 0;
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  if (nleft == 1) {
    *(unsigned char *) (&answer) = *(unsigned char *) w;
    sum += answer;
  }
  sum = (sum >> 16) + (sum & 0xffff);   /* add hi 16 to low 16 */
  sum += (sum >> 16);           /* add carry */
  answer = ~(sum & 0xffff);                /* truncate to 16 bits */
  return (answer);
}
