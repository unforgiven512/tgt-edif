#ifndef __STRING_CACHE_H__
#define __STRING_CACHE_H__

/*
 * Copyright (c) 2001 Vladimir Dergachev (volodya@users.sourceforge.net)
 *    
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


typedef struct {
	long size;
	long string_hash_size;
	long free;
	long mod;
	long mul;
	char **string;
	void **data;
	long *string_hash;
	long *next_string;
	} STRING_CACHE;
	
STRING_CACHE * new_string_cache(void);
long lookup_string(STRING_CACHE *sc, char * string);
long add_string(STRING_CACHE *sc, char *string);
int valid_id(STRING_CACHE *sc, long string_id);
void * do_alloc(long, long);

#endif
