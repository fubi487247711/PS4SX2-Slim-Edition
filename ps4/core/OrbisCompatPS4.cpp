// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

// (AI-assisted) Minimal defaults for optional instrumentation inherited from the PS5 port.

#include "pcsx2/OrbisEEHle.h"
#include "common/Pcsx2Defs.h"

#include <atomic>
#include <cstdio>
#include <unistd.h>

bool OrbisFlag(const char* name)
{
	if (!name || !*name)
		return false;

	char path[256];
	std::snprintf(path, sizeof(path), "/data/PCSX2/flags/%s", name);
	if (access(path, F_OK) == 0)
		return true;

	std::snprintf(path, sizeof(path), "/data/PCSX2/%s", name);
	return access(path, F_OK) == 0;
}

void OrbisCpuSample(int)
{
}

void OrbisVUProfStart()
{
}

void OrbisEEVsyncClock()
{
}

// Preserve the upstream-safe behavior of PS5-only fast paths until the PS4
// build has a runtime configuration adapter for them.
std::atomic<int> g_orbis_hle_flush{0};
std::atomic<int> g_orbis_pfw{0};
std::atomic<int> g_orbis_gs_pfw{0};
std::atomic<int> g_orbis_hle_tid{0};
std::atomic<int> g_orbis_eret_fast{0};
std::atomic<int> g_orbis_vif1_fuse{0};
u64 g_orbis_vif1_fused = 0;
OrbisTidCache g_orbis_tid{};
u64 g_orbis_hle_flushes = 0;
u64 g_orbis_tid_hits = 0;
u64 g_orbis_tid_checks = 0;
u64 g_orbis_tid_mismatches = 0;
u64 g_orbis_eret_skips = 0;
u64 g_orbis_eret_block[4] = {};
