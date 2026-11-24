// SPDX-License-Identifier: GPL-3.0+

#include "ThreadAffinityPS4.h"

#include <cstdio>

// OpenOrbis's public header declares these without parameters. Keep the
// verified ABI declaration available if per-thread affinity is reintroduced.
extern "C" void* scePthreadSelf();
extern "C" int scePthreadSetaffinity(void* thread, unsigned long long mask);

namespace PS4ThreadAffinity
{
	void PinCurrentThread(const char* role, int core)
	{
		// On base PS4, fixed core masks can over-constrain the scheduler when
		// the EE, VU1, GS and audio workers contend. Let the system scheduler
		// place these threads instead of pinning each worker to one core.
		(void)core;
		std::printf("PS4 thread affinity: %s left to system scheduler (default policy)\n", role);
		std::fflush(stdout);
	}
} // namespace PS4ThreadAffinity
