/*
 * cdnget - CDN Reverse Proxy
 * Copyright (C) 2014  Orkun Karaduman <orkunkaraduman@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once


#include "common.h"


namespace cdnget
{


class Locker
{
typedef set<string> FD_LOCKS;

private:
	static FD_LOCKS fd_locks;
	static pthread_mutex_t fd_locks_mutex;
	static string getfnfromfd(int fd);
	
public:
	Locker();
	virtual ~Locker();
	bool lock(int fd);
	bool lock(FILE *file);
	bool unlock(int fd);
	bool unlock(FILE *file);
	bool testLock(int fd);
	bool testLock(FILE *file);
	
};


extern Locker locker;


}
