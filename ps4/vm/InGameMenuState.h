// SPDX-License-Identifier: GPL-3.0+
// (AI-assisted) Atomic CPU-to-GS menu snapshot, independent of renderer headers.
#pragma once

#include <algorithm>
#include <cstdint>

namespace PS4GameMenu
{
	struct InGameMenuState
	{
		bool visible;
		unsigned selected;
		float upscale;
		float max_upscale;
		unsigned aspect;
		bool show_overlay;
	};

	constexpr std::uint32_t PackInGameMenuState(InGameMenuState value)
	{
		if (!value.visible)
			return 0;
		const unsigned limit = std::clamp(static_cast<unsigned>(value.max_upscale * 4.0f + 0.5f), 3u, 16u);
		const unsigned upscale = std::clamp(static_cast<unsigned>(value.upscale * 4.0f + 0.5f), 3u, limit);
		return (std::min(value.selected, 5u) + 1u) |
			(upscale << 8) |
			(limit << 13) |
			(std::min(value.aspect, 4u) << 18) |
			(value.show_overlay ? (1u << 21) : 0u);
	}

	constexpr InGameMenuState UnpackInGameMenuState(std::uint32_t value)
	{
		// Resolution uses quarter-step units (0.75x..4.00x) in five bits.
		const unsigned selection = value & 0xffu;
		const unsigned limit = std::clamp((value >> 13) & 0x1fu, 3u, 16u);
		const unsigned upscale = std::clamp((value >> 8) & 0x1fu, 3u, limit);
		return {selection != 0, selection ? std::min(selection - 1u, 5u) : 0u,
			static_cast<float>(upscale) * 0.25f, static_cast<float>(limit) * 0.25f,
			std::min((value >> 18) & 0x07u, 4u), (value & (1u << 21)) != 0};
	}
} // namespace PS4GameMenu
