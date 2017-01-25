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


#include "App.h"
#include "Listener.h"
#include "Worker.h"
#include "Downloader.h"


namespace cdnget
{


App app;


App::App()
{
	createMutex(&this->mutex);
	_terminate = false;
	exitcode = 0;
}

App::~App()
{
	pthread_mutex_destroy(&this->mutex);
}

int App::run(int argc, char *argv[])
{
	int result = 0;
	stringstream ss;
	cout << appname << " " << appver << "\n" << appdesc << "\n";
	log("INFO", "APP_START");
	if (FCGX_Init())
	{
		log("ERR", "FCGI_INIT");
		result = 10;
	}
	long numCPU = sysconf(_SC_NPROCESSORS_ONLN);
	if (numCPU < 1) numCPU = 1;
	if (0 == result)
	{
		for (long l = 0; l < numCPU; l++)
		{
			Listener *listener;
			listener = new Listener();
			if (!listener->create())
			{
				delete listener;
				Listener::terminateAll();
				result = 11;
			}
		}
	}
	if (0 == result)
	{
		while (true)
		{
			usleep(100*1000);
			pthread_mutex_lock(&mutex);
			if (_terminate)
			{
				log("INFO", "APP_TERM");
				Listener::terminateAll();
				Worker::terminateAll();
				result = exitcode;
				pthread_mutex_unlock(&mutex);
				break;
			}
			pthread_mutex_unlock(&mutex);
			if (Listener::getListenersCount() != numCPU)
			{
				log("ERR", "LISTENER_COUNT");
				Listener::terminateAll();
				Worker::terminateAll();
				result = 12;
				break;
			}
		}
	}
	log("INFO", "APP_TERM_WAIT");
	while (Listener::getListenersCount() || Worker::getWorkersCount())
	{
		usleep(100*1000);
	}
	ss.str("");
	ss << result;
	log("INFO", "APP_EXIT", ss.str());
	return result;
}

void App::signal(int signo)
{
	pthread_mutex_lock(&mutex);
	/*if (_terminate)
	{
		pthread_mutex_unlock(&mutex);
		return;
	}*/
	switch (signo)
	{
	case SIGINT:
		log("INFO", "SIGINT");
		exitcode = 0;
		_terminate = true;
		break;
	case SIGTERM:
		log("INFO", "SIGTERM");
		exitcode = 0;
		_terminate = true;
		break;
	case SIGKILL:
		log("INFO", "SIGKILL");
		exitcode = 128+signo;
		_terminate = true;
		break;
	case SIGHUP:
		log("INFO", "SIGHUP");
		break;
	default:
		SIG_DFL(signo);
	}
	pthread_mutex_unlock(&mutex);
}

void App::log(const string &severity, const string &summary, const string &args)
{
	stringstream ss;
	char s[256];
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", (timeinfo->tm_year+1900), (timeinfo->tm_mon+1), timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	ss << s << " " << syscall(SYS_getpid) << " " << syscall(SYS_gettid) << " " << Worker::getWorkersCount() << " " << severity << " " << summary << " " << args << "\n";
	cerr << ss.str();
}


}
