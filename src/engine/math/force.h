#pragma once

namespace engine
{
	struct Particle;

	namespace math
	{
		constexpr float spring_distance_min = 2.0f;
		constexpr float LIGHTSPEED = 1.0f;
		constexpr float WAVE_HEIGHT = 10.0f;

		float gravity(float, float, float);
		float acceleration(float, float);
		float coulomb(float, float, float);
		float yukawa(float, float);

		void electron_orbit(Particle*, Particle*);
		void decay(Particle*, Particle*, float);
	}
}