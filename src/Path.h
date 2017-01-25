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


class Path
{
typedef list<string> PARTS;

private:
	string strPath, basename, dirname;
	PARTS parts;
	bool relative, dir;
	bool makeStrPath(bool (*proc)(string part, string strPath, bool dir) = NULL);
	
public:
	Path();
	Path(string strPath);
	Path(const Path &path);
	virtual ~Path();
	string getStrPath();
	string getBasename();
	string getDirname();
	bool isRelative();
	bool isDir();
	bool normalize();
	string append(Path &path);
	string append(string strPath);
	bool exec(bool (*proc)(string part, string strPath, bool dir));
	
};


}
