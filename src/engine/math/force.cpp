#include "force.h"
#include "vector.h"
#include "../utility/utility.h"

#include <cmath>
#include <numbers>

using namespace engine;

constexpr float SCALE = 1e-11f;
constexpr float PI = std::numbers::pi;

float math::gravity(const float m1, const float m2, const float r2)
{
	constexpr float G = 1.0f * SCALE;
	return G * (m1 * m2) / r2;
}

float math::acceleration(float const F, const float m)
{
	return m == 0.0f ? LIGHTSPEED : F / m;
}

float math::coulomb(const float q1, const float q2, const float r2)
{
	constexpr float epsilon = 55.26349406f;
	return -(1/(4 * PI * epsilon)) * (q1 * q2 / r2);
}

float math::yukawa(const float g, const float r)
{
	constexpr float DECAY = 0.0f;
	return (g * g) / (4 * PI) * (std::exp(-DECAY * r) / r);
}

void math::electron_orbit(Particle* a, Particle* b)
{
	constexpr float RATE = 0.025f;
	constexpr float SPEED = 0.05f;

	if (a->is_electron && !b->is_electron && !b->is_boson)
	{
		Vec2 rvec = a->position - b->position;
		const float dist = math::magnitude(rvec);

		auto[rx, ry] = math::normalize(rvec);
		const Vec2 tangent = { .x = -ry, .y = rx };

		// ideal orbital speed (derived from centripetal balance)
		const float target = std::sqrt(SPEED / dist);
		const float current = math::dot(a->velocity, tangent);

		const Vec2 dir = tangent * (target - current) * RATE;

		// gently correct toward stable orbit
		a->velocity = a->velocity + dir;
	}

	if (b->is_electron && !a->is_electron && !a->is_boson)
	{
		Vec2 rvec = b->position - a->position;
		const float dist = math::magnitude(rvec);

		auto[rx, ry] = math::normalize(rvec);
		const Vec2 tangent = { .x = -ry, .y = rx };

		const float target = std::sqrt(SPEED / dist);
		const float current = math::dot(b->velocity, tangent);

		const Vec2 dir = tangent * (target - current) * RATE;

		b->velocity = b->velocity + dir;
	}
}

void math::decay(Particle* a, Particle* b, float dt)
{
	if (!a->is_boson && !a->is_electron)
	{
		a->life += dt;
	}
	if (!b->is_boson && !b->is_electron)
	{
		b->life += dt;
	}
}