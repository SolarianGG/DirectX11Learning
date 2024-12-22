#pragma once

#include <cinttypes>

namespace lea {

	class LeaTimer {
		LeaTimer();
	public:
		static LeaTimer& Instance()
		{
			static LeaTimer timer;

			return timer;
		}

		float TotalTime() const;
		float DeltaTime() const { return static_cast<float>(deltaTime_); };
		void Reset();
		void Start();
		void Stop();
		void Tick();
	private:
		double secondsPerCount_;
		double deltaTime_;

		int64_t baseTime_;
		int64_t pausedTime_;
		int64_t stopTime_;
		int64_t prevTime_;
		int64_t currTime_;

		bool isStopped_;
	};
}

#define TIMER lea::LeaTimer::Instance()