// SPDX-License-Identifier: GPL-3.0+

#pragma once

namespace PS4ThreadAffinity
{
	// Optionally pins the calling emulation worker to one of the six game CPU cores.
	// The system scheduler is the default. Create /data/PS4SX2/vm-force-thread-affinity to enable pinning.
	void PinCurrentThread(const char* role, int core);
} // namespace PS4ThreadAffinity
