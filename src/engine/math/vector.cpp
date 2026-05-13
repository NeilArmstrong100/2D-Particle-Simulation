#include "vector.h"
#include "../utility/utility.h"

using namespace engine;

float math::distance(const Vec2& a, const Vec2& b)
{
	const float dx = (a.x - b.x);
	const float dy = (a.y - b.y);
	return std::sqrt(dx * dx + dy * dy);
}

float math::magnitude(const	Vec2& v)
{
	return std::sqrt(v.x * v.x + v.y * v.y);
}

float math::dot(const Vec2& a, const Vec2& b)
{
	return a.x * b.x + a.y * b.y;
};

Vec2 math::normalize(const Vec2& v)
{
	const float mag = magnitude(v);
	return { .x = v.x / mag, .y = v.y / mag };
};

float math::qq_interactions(const Vec3& ac, const Vec3& bc, const float k, const float r)
{
	constexpr float alpha_s = 0.1183f;
	const float f = ac == bc ? 4.0f / 3 : -4.0f / 3;
	return f * (alpha_s / r) - (k * r);
}

float math::qaq_interactions(const Vec3& ac, const Vec3& bc, const float b, const float r)
{
	constexpr float alpha_s = 0.1183f;
	const float f = ac == bc ? 1.0f : -1.0f;
	return f * (alpha_s / r) - (b * r);
}