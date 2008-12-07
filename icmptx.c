/* Code is ruthlessly ripped from vtun and itunnel with appropriate changes.
 * Guys, thanks for the great stuff!
 *
 * itunnel - an ICMP tunnel by edi / teso
 * VTun - Virtual Tunnel over TCP/IP network.
 *
 * Original author unknown, but modified by Thomer M. Gil who found the original
 * code through
 *   http://www.linuxexposed.com/Articles/Hacking/Case-of-a-wireless-hack.html
 * (a page written by Siim Põder).
 *
 * The icmptx website is at http://thomer.com/icmptx/
 */

#include "tun_dev.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* modified itunnel */
int run_icmp_tunnel (int id, int packetsize, char **argv, int tun_fd);

/* max transfered unit - capsuled packet size */
const int mtu = 65536;

int main(int argc, char **argv) {
  char *dev;
  int tun_fd = 0;

  if (argc != 3) {
    fprintf(stderr, "Error, incorrect number of arguments provided. -s for server mode and -c for client and destination host name?\n");
    return 1;
  }

  /* create the tunnel device */
  if ((dev = (char *) malloc(16)) == NULL) {
    fprintf(stderr, "If you have never had problems allocating 16 bytes\n"
            "of memory, then now is your first time. Fatal.\n");
    return 1;
  }
  dev[0] = 0;
  if ((tun_fd = tun_open(dev)) < 1) {
    fprintf(stderr, "Could not create tunnel device. Fatal.\n");
    return 1;
  } else {
    printf("Created tunnel device: %s\n", dev);
  }

  run_icmp_tunnel(7530, mtu, argv, tun_fd);

  tun_close(tun_fd, dev);

  return 0;
}
