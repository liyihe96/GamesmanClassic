/************************************************************************
**
** NAME:	db.c
**
** DESCRIPTION:	Gamescrafters Database Test Program
**
** AUTHOR:	GamesCrafters Research Group, UC Berkeley
**		Supervised by Dan Garcia <ddgarcia@cs.berkeley.edu>
**
** DATE:	2005-01-11
**
** LICENSE:	This file is part of GAMESMAN,
**		The Finite, Two-person Perfect-Information Game Generator
**		Released under the GPL:
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program, in COPYING; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "db.h"

int main(int argc, char *argv[])
{
	gamesdb_position totalrecs = 0x1ffff;
	size_t recsize = sizeof(short);
	size_t buffers = 10;
	size_t mem_used = 8; //approx. 8KB of ram will be used
	char dbname[] = "testdb\0";

	gamesdb *testdb = gamesdb_create(recsize, buffers, mem_used, dbname);
	printf("Starting DB test...\n");
	
	//sleep(10);
	
	gamesdb_position i;
	short data = 0, result = 0;

	//sequencial read/write

	for (i=0;i<totalrecs;i++) {
		data = i % (1<<16);
		printf("reading: %llu\n", i);
		if (i == 85)
			printf("\n");
		gamesdb_put(testdb, (void*)&data, i);
		gamesdb_get(testdb, (void*)&result, i);
		if (data!=result) {
			printf("ERROR in seq read: position %llu, saved %d, got %d\n", i, data, result);
			break;
		}
	}
	//random reads
/*	for (i=0;i<totalrecs;i++) {
		data = i % (1<<16);
		if(i == 170)
			printf("GAHHHHHH!\n");
		printf("reading: %llu\n", i);
		db_get(testdb, (void*)&result, i);
		if (data!=result) {
			printf("ERROR in seq read: position %llu, saved %d, got %d\n", i, data, result);
			break;
		}
	}*/
	gamesdb_position val;
	for (i=0;i<totalrecs;i++) {
		while (totalrecs <= (val = rand() / (RAND_MAX/totalrecs)));\
		printf("reading: %llu\n", val);
		data = val % (1 << 16);
		//db_put(testdb, (void*)&data, i);
		gamesdb_get(testdb, (void*)&result, val);
		if (data!=result) {
			printf("ERROR in rand read: position %llu, saved %d, got %d\n", i, data, result);
			break;
		}
	}

	printf("Ending DB test...\n");
	gamesdb_destroy(testdb);
	return 0;
}
