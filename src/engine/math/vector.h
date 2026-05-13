#pragma once

namespace engine
{
	struct Vec2;
	struct Vec3;

	namespace math
	{
		float distance(const Vec2&, const Vec2&);
		float magnitude(const Vec2&);
		float dot(const Vec2&, const Vec2&);
		Vec2 normalize(const Vec2&);
		float qq_interactions(const Vec3&, const Vec3&, float, float);
		float qaq_interactions(const Vec3&, const Vec3&, float, float);
	}
}