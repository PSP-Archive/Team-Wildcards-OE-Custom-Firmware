/* 
 * Copyright (C) 2006 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _CTRL_H
#define _CTRL_H

#define KEYSET0 0x006050
#define KEYSET1 0x0060A0
#define KEYSETF 0x00F3F0
#define KEYSETO 0x00E3F8
#define KEYSETW 0x006300

extern unsigned int ctrl_read();
extern void ctrl_waitrelease();
extern unsigned int ctrl_waitmask(unsigned int keymask);
extern unsigned int ctrl_waittime(unsigned int t);
extern unsigned int ctrl_input();
extern void get_keyname(unsigned int key, char * res);

#endif
