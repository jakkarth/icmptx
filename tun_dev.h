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

#ifndef _DRIVER_H
#define _DRIVER_H

/* Definitions for device and protocol drivers 
 * Interface will be completely rewritten in 
 * version 3.0
 */

int tun_open();
int tun_close(int fd);
int tun_write(int fd, char *buf, int len);
int tun_read(int fd, char *buf, int len);

#endif
