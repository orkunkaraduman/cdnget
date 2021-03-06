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


#include "common.h"
#include "App.h"


using namespace cdnget;


void sighandler(int signo)
{
	app.signal(signo);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGKILL, sighandler);
	signal(SIGHUP, sighandler);
	return app.run(argc, argv);
}
