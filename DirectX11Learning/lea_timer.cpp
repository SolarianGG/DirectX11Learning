#include "lea_timer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/*
TODO:
Our demo framework creates an instance of GameTimer for measuring the total time since the application started,
and the time elapsed between frames; however, you can also create additional instances and use them as generic
“stopwatches.” For example, when a bomb is ignited, you could start a new GameTimer, and when the TotalTime
reaches 5 seconds, you could raise an event that the bomb exploded.
*/

namespace lea {

	LeaTimer::LeaTimer()
		: secondsPerCount_(0.0), deltaTime_(-1.0), baseTime_(0), stopTime_(0),
		pausedTime_(0), prevTime_(0), currTime_(0), isStopped_(false)
	{
		int64_t countsPerSec{};
		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
		secondsPerCount_ = 1.0 / static_cast<double>(countsPerSec);
	}

	float LeaTimer::TotalTime() const
	{
		if (isStopped_)
		{
			return static_cast<float>(((stopTime_ - pausedTime_) - baseTime_) * secondsPerCount_);
		}
		return static_cast<float>(((currTime_ - pausedTime_) - baseTime_) * secondsPerCount_);
	}

	void LeaTimer::Reset()
	{
		int64_t currTime{};
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

		baseTime_ = currTime;
		prevTime_ = currTime;
		stopTime_ = 0;
		isStopped_ = false;
	}

	void LeaTimer::Start()
	{
		int64_t currTime{};
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
		if (isStopped_)
		{
			pausedTime_ += (currTime - stopTime_);

			prevTime_ = currTime;

			stopTime_ = 0;
			isStopped_ = false;
		}
	}

	void LeaTimer::Stop()
	{
		if (!isStopped_)
		{
			int64_t currTime{};
			QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

			stopTime_ = currTime;
			isStopped_ = true;
		}
	}

	void LeaTimer::Tick()
	{
		if (isStopped_)
		{
			deltaTime_ = 0.0;
			return;
		}

		int64_t currTime{};

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
		currTime_ = currTime;

		deltaTime_ = (currTime_ - prevTime_) * secondsPerCount_;

		prevTime_ = currTime_;

		if (deltaTime_ < 0.0)
		{
			deltaTime_ = 0.0;
		}
	}
}