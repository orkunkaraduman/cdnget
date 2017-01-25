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


#include "Path.h"


namespace cdnget
{


Path::Path()
{
	Path("");
}

Path::Path(string strPath)
{
	this->strPath = strPath;
	this->relative = true;
	this->dir = true;
	size_t strPathLen = strPath.length();
	if (0 == strPathLen) return;
	this->dir = false;
	size_t u;
	char c;
	string part = "";
	for (u = 0; u <= strPathLen-1; u++)
	{
		c = strPath[u];
		if (c == '/')
		{
			if (u == 0) this->relative = false;
			if (u >= strPathLen-1) this->dir = true;
		} else
		{
			part += c;
		}
		if (c == '/' || u >= strPathLen-1)
		{
			if ("." == part)
			{
				if (u >= strPathLen-1) this->dir = true;
				parts.push_back(part);
			} else
			if (".." == part)
			{
				if (u >= strPathLen-1) this->dir = true;
				parts.push_back(part);
			} else
			{
				if ("" != part) parts.push_back(part);
			}
			part = "";
		}
	}
	makeStrPath();
}

Path::Path(const Path &path)
{
	this->strPath = path.strPath;
	this->basename = path.basename;
	this->dirname = path.dirname;
	this->parts = path.parts;
	this->relative = path.relative;
	this->dir = path.dir;
}

Path::~Path()
{
}

string Path::getStrPath()
{
	return strPath;
}

string Path::getBasename()
{
	return basename;
}

string Path::getDirname()
{
	return dirname;
}

bool Path::isRelative()
{
	return relative;
}

bool Path::isDir()
{
	return dir;
}

bool Path::makeStrPath(bool (*proc)(string part, string strPath, bool dir))
{
	bool result = true;
	if (relative) strPath = ""; else strPath = "/";
	Path::PARTS::iterator it = parts.begin();
	while (it != parts.end())
	{
		string part = *it;
		if (it != parts.end()) ++it;
		if (it == parts.end()) basename = strPath;
		strPath += part;
		bool dir = (it != parts.end() || this->dir);
		if (dir) strPath += "/";
		if (it == parts.end() && this->dir) dirname = strPath; else dirname = basename;
		if (NULL != proc) result &= proc(part, strPath, dir);
	}
	return result;
}

bool Path::normalize()
{
	bool result = true;
	Path::PARTS::iterator it = parts.begin();
	while (it != parts.end())
	{
		string part = *it;
		if ("." == part)
		{
			it = parts.erase(it);
		} else
		if (".." == part)
		{
			if (it != parts.begin())
			{
				--it;
				it = parts.erase(it);
			} else
			{
				result = false;
			}
			it = parts.erase(it);
		}
		if (it != parts.end()) ++it;
	}
	makeStrPath();
	return result;
}

string Path::append(Path &path)
{
	if (!dir && !parts.empty()) parts.pop_back();
	Path::PARTS::iterator it = path.parts.begin();
	while (it != path.parts.end())
	{
		parts.push_back(*it);
		if (it != path.parts.end()) ++it;
	}
	dir = path.dir;
	makeStrPath();
	return strPath;
}

string Path::append(string strPath)
{
	Path path(strPath);
	return append(path);
}

bool Path::exec(bool (*proc)(string part, string strPath, bool dir))
{
	if (NULL == proc) return false;
	return this->makeStrPath(proc);
}


}
