/*	
	SCEP-Series Memory allocation
    Copyright (C) 1997-2004 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

/**
  * allocate a memory block from the patition caculated.
  * 
  * @param ptr - The pointer to get the block.
  * @param name - Name assigned to the new block.
  * @param size - Size of the memory block, in bytes.
  * @param flags - 0: dynamic allocate. 
  *				   1: allocate from the patition in which there is the largest memory block.
  *
  * @returns The UID of the new block, or if less than 0 an error.
  */
extern void * pspAlloc(SceUID * blockID, const char * name, SceSize size, int flag);

/**
  * Free a memory block allocated with pspAlloc.
  * 
  * @param blockID  - UID of the memory block.
  *
  * @returns ? on success, less than 0 on error.
  */
extern int pspFree(SceUID blockID);

#endif
