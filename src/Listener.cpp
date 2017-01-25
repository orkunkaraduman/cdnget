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


#include "Listener.h"
#include "App.h"
#include "Locker.h"
#include "Worker.h"


namespace cdnget
{


pthread_mutex_t Listener::listeners_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
Listener::Listeners Listener::listeners;


Listener::Listener()
{
	createMutex(&this->mutex);
	this->_terminate = false;
	this->_accepting = false;
	this->th = 0;
}

Listener::~Listener()
{
	pthread_mutex_destroy(&this->mutex);
}

int Listener::getListenersCount()
{
	int result = 0;
	pthread_mutex_lock(&listeners_mutex);
	result = (int)listeners.size();
	pthread_mutex_unlock(&listeners_mutex);
	return result;
}

void Listener::terminateAll()
{
	pthread_mutex_lock(&listeners_mutex);
	Listeners::iterator it = listeners.begin();
	while (it != listeners.end())
	{
		Listeners::iterator cur = it++;
		Listener *o = (*cur).second;
		o->terminate();
	}
	pthread_mutex_unlock(&listeners_mutex);
}

void Listener::terminate()
{
	pthread_mutex_lock(&mutex);
	FCGX_ShutdownPending();
	if (!_accepting)
	{
		_terminate = true;
		pthread_mutex_unlock(&mutex);
	} else 
	{
		pthread_mutex_lock(&listeners_mutex);
		pthread_cancel(th);
		listeners.erase(th);
		pthread_mutex_unlock(&mutex);
		delete this;
		app.log("INFO", "LISTENER_FINISH");
		pthread_mutex_unlock(&listeners_mutex);
	}
}

bool Listener::create()
{
	bool result = false;
	pthread_mutex_lock(&mutex);
	
	if (th)
	{
		pthread_mutex_unlock(&mutex);
		return result;
	}
	
	pthread_mutex_lock(&listeners_mutex);
	if (!_terminate && (th = createThread((void *(*)(void *))&Listener::work, this, true)))
	{
		listeners[th] = this;
		result = true;
		app.log("INFO", "LISTENER_CREATE");
	} else
	{
		app.log("ERR", "LISTENER_CREATE");
	}
	pthread_mutex_unlock(&listeners_mutex);
	
	pthread_mutex_unlock(&mutex);
	return result;
}

void *Listener::work(Listener *listener)
{
	pthread_mutex_lock(&listeners_mutex);
	app.log("INFO", "LISTENER_START");
	pthread_mutex_unlock(&listeners_mutex);
	
	while (true)
	{
		usleep(1*1000);
		pthread_mutex_lock(&listener->mutex);
		if (listener->_terminate)
		{
			pthread_mutex_unlock(&listener->mutex);
			break;
		}
		pthread_mutex_unlock(&listener->mutex);
		pthread_mutex_lock(&listener->mutex);
		listener->_accepting = true;
		pthread_mutex_unlock(&listener->mutex);
		Worker *worker = new Worker();
		if (!worker->create())
		{
			delete worker;
			break;
		}
		pthread_mutex_lock(&listener->mutex);
		listener->_accepting = false;
		pthread_mutex_unlock(&listener->mutex);
	}
	
	pthread_mutex_lock(&listeners_mutex);
	listeners.erase(pthread_self());
	delete listener;
	app.log("INFO", "LISTENER_FINISH");
	pthread_mutex_unlock(&listeners_mutex);
	
	return NULL;
}


}
