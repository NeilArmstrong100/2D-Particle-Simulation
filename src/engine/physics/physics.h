#pragma once

#include <thread>

namespace engine::physics
{
	static uint32_t framerate;
	static std::atomic stop_flag{ false };
	static bool electron_orbit = true;
	static bool photon_waves = true;

	void start(uint32_t, bool, bool);
	void stop();
	void run();
	void quark_interactions();
	void nuclei_interactions();
	void update();
}