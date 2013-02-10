#include "precompiled.h"
#include <assert.h>

#include "timer.h"

#ifndef WIN32
	#include <sys/time.h>
	#include <unistd.h>
#endif

#ifdef WIN32
	Win32PerformanceCounter win32counter;
#endif
	
MyTimer::MyTimer()
{
	start();
}

void MyTimer::start()
{
	paused = false;
	timeValue = absTime();
}

void MyTimer::pause()
{
	assert(!paused && "Called pause() on a timer which was not running!");

	paused = true;
	timeValue = absTime() - timeValue;
}

void MyTimer::resume()
{
	assert(paused && "Called resume() on a timer which was not paused!");
	
	paused = false;
	timeValue = absTime() - timeValue;
}

void MyTimer::setTime(float t)
{
	if (paused)
		timeValue = (long long)(t * 1000);
	else
		timeValue = absTime() - (long long)(t * 1000);
}

float MyTimer::getTime()
{
	return (absTime() - timeValue) * (1.0f / 1000.0f);
}

long long MyTimer::absTime()
{
	#ifdef WIN32
		long long t;
		QueryPerformanceCounter((LARGE_INTEGER*)(&t));
		return t / win32counter.frequency;
	#else
		struct timeval t;
		long long s;
		gettimeofday(&t, 0);
		s = t.tv_sec;
		s *= 1000;
		s += (t.tv_usec / 1000);
		return s;
	#endif
}

void MyTimer::sleep(long long interval)
{
	#ifdef WIN32
		Sleep(interval);
	#else
		usleep(interval * 1000);
	#endif
}
