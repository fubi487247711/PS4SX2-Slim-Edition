// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+
#include "Host.h"
#include "GS/GS.h"
#include "VMManager.h"
#include "MTGS.h"
#include "Config.h"
#include "BuildVersion.h"
#include "common/Console.h"
#include "common/FileSystem.h"
#include "common/WindowInfo.h"
#include "INISettingsInterface.h"
#include "PerformanceMetrics.h"
#include "common/Timer.h"
#include <string>
#include <mutex>
#include <cstring>
#include <deque>
#include <future>
#include <pthread.h>
#include <atomic>

// (AI-assisted) CPU work is queued to the thread that owns VMManager and its memory.
static pthread_t s_cpu_thread;
static std::mutex s_queue_mutex;
static std::deque<std::function<void()>> s_cpu_queue;
static std::atomic<bool> s_cpu_queue_pending{false};
static u64 s_cpu_queue_drain_locks = 0; // CPU thread only.
u64 VMHostGetCPUQueueLockCount() { return s_cpu_queue_drain_locks; }
void VMHostSetCPUThread() { s_cpu_thread = pthread_self(); }
void VMHostDrainCPUQueue()
{
	// (AI-assisted) This runs at every EE event. Avoid locking an empty queue;
	// producers and the consumer update the hint while holding the same mutex.
	// Concurrent enqueue near the last pop still needs proper console testing.
	while (s_cpu_queue_pending.load(std::memory_order_acquire))
	{
		std::function<void()> task;
		{
			std::lock_guard lock(s_queue_mutex);
			++s_cpu_queue_drain_locks;
			if (s_cpu_queue.empty())
			{
				s_cpu_queue_pending.store(false, std::memory_order_release);
				break;
			}
			task = std::move(s_cpu_queue.front());
			s_cpu_queue.pop_front();
			s_cpu_queue_pending.store(!s_cpu_queue.empty(), std::memory_order_release);
		}
		task();
	}
}
void VMHostPoll();
extern void ProbeLog(const char*, ...);

namespace BuildVersion
{
	int GitTagHi = 0, GitTagMid = 0, GitTagLo = 1;
}

void Host::CommitBaseSettingChanges()
{
	// The PS4 frontend stores its base settings in the persistent INI layer.
	// Keep the settings lock while saving so no other thread can mutate it.
	auto lock = Host::GetSettingsLock();
	SettingsInterface* settings = Host::Internal::GetBaseSettingsLayer();
	if (settings && !settings->Save())
		Console.Error("Failed to save PS4SX2 settings INI");
}

void Host::AddOSDMessage(std::string message, float)
{
	Console.WriteLn("OSD: %s", message.c_str());
}

void Host::AddIconOSDMessage(std::string key, const char*, std::string_view message, float)
{
	Console.WriteLn("OSD [%s]: %.*s", key.c_str(), static_cast<int>(message.size()), message.data());
}

void Host::AddKeyedOSDMessage(std::string key, std::string message, float)
{
	Console.WriteLn("OSD [%s]: %s", key.c_str(), message.c_str());
}

void Host::RemoveKeyedOSDMessage(std::string)
{
}

void Host::ClearOSDMessages()
{
}

void Host::ReportInfoAsync(std::string_view, std::string_view)
{
}

void Host::ReportErrorAsync(std::string_view title, std::string_view message)
{
	Console.Error("Host Error [%.*s]: %.*s", static_cast<int>(title.size()), title.data(),
		static_cast<int>(message.size()), message.data());
}

bool Host::RequestResetSettings(bool, bool, bool, bool, bool)
{
	return false;
}

void Host::RequestResizeHostDisplay(s32, s32)
{
}

void Host::RunOnCPUThread(std::function<void()> function, bool block)
{
	if (pthread_equal(pthread_self(), s_cpu_thread))
	{
		function();
		return;
	}
	auto task = std::make_shared<std::packaged_task<void()>>(std::move(function));
	auto done = task->get_future();
	{
		std::lock_guard lock(s_queue_mutex);
		s_cpu_queue.emplace_back([task]() { (*task)(); });
		s_cpu_queue_pending.store(true, std::memory_order_release);
	}
	if (block)
		done.wait();
}
void Host::RunOnGSThread(std::function<void()> function)
{
	MTGS::RunOnGSThread(std::move(function));
}

void Host::RefreshGameListAsync(bool)
{
}

void Host::CancelGameListRefresh()
{
}

void Host::RequestVMShutdown(bool, bool, bool)
{
	VMManager::SetState(VMState::Stopping);
}

void Host::OnVMStarting()
{
	Console.WriteLn("VM starting");
}

void Host::OnVMStarted()
{
	Console.WriteLn("VM started");
}

void Host::OnVMDestroyed()
{
	Console.WriteLn("VM destroyed");
}

void Host::OnVMPaused()
{
}

void Host::OnVMResumed()
{
}

void Host::OnGameChanged(const std::string& title, const std::string&, const std::string&, const std::string&, u32, u32)
{
	Console.WriteLn("Game changed: %s", title.c_str());
}

std::string Host::TranslatePluralToString(const char*, const char*, const char*, int)
{
	return "";
}

bool Host::InBatchMode()
{
	return false;
}

bool Host::InNoGUIMode()
{
	return true;
}

void Host::OpenURL(const std::string_view)
{
}

bool Host::CopyTextToClipboard(const std::string_view)
{
	return false;
}

std::string Host::GetTextFromClipboard()
{
	return "";
}

std::optional<WindowInfo> Host::AcquireRenderWindow(bool)
{
	WindowInfo info{};
	info.type = WindowInfo::Type::Surfaceless;
	info.surface_width = 1920;
	info.surface_height = 1080;
	info.surface_refresh_rate = 60.0f;
	return info;
}

void Host::ReleaseRenderWindow()
{
}

void Host::BeginPresentFrame()
{
}

bool Host::IsFullscreen()
{
	return false;
}

void Host::SetFullscreen(bool)
{
}

void Host::SetDefaultUISettings(SettingsInterface&)
{
}

std::unique_ptr<ProgressCallback> Host::CreateHostProgressCallback()
{
	return nullptr;
}

int Host::LocaleSensitiveCompare(std::string_view lhs, std::string_view rhs)
{
	if (lhs < rhs)
		return -1;
	if (lhs > rhs)
		return 1;
	return 0;
}

void Host::LoadSettings(SettingsInterface& si, std::unique_lock<std::mutex>&)
{
	// No persistent settings storage on PS4; use defaults.
}

void Host::OnPerformanceMetricsUpdated()
{
}

void Host::CheckForSettingsChanges(const Pcsx2Config&)
{
}

void Host::PumpMessagesOnCPUThread()
{
	::VMHostPoll();
}

s32 Host::Internal::GetTranslatedStringImpl(const std::string_view, const std::string_view message, char* buffer, size_t capacity)
{
	if (capacity < message.size())
		return -1;
	std::memcpy(buffer, message.data(), message.size());
	return static_cast<s32>(message.size());
}
