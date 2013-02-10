#ifndef _TIMER_H_
#define _TIMER_H_

/// Measures time intervals.
class MyTimer
{
public:

	/// Standard constructor, starts the timer. You don't need to call start explicitly.
	MyTimer();

	/// Resets the timer to zero.
	void start();
	
	/// Pauses the timer.
	void pause();
	
	/// Resumes the timer.
	void resume();
	
	/// Sets the time.
	void setTime(float t);
	
	/// Returns the time in seconds since the timer was started.
	float getTime();
	
	/// Returns the time in milliseconds since the system is running.
	static long long absTime();
	
	/// Sleeps the specified number of milliseconds
	static void sleep(long long interval);
	
protected:

	/// If running, the time the timer was started; if paused, the time the timer was running before
	long long timeValue;
	bool paused;
};

#ifdef WIN32
	
	/// If using windows, we need this dummy class which retrieves the frequency of the timer
	class Win32PerformanceCounter
	{
	public:
		Win32PerformanceCounter()
		{
			QueryPerformanceFrequency((LARGE_INTEGER*)(&frequency));
			frequency = frequency / 1000;
		}
		long long frequency;
	};
	
	extern Win32PerformanceCounter win32counter;

#endif

#endif
