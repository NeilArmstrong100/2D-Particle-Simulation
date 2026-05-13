#pragma once

#include "../utility/utility.h"

namespace engine::renderer
{
	struct Vertex
	{
		Vec2 position;
		Vec3 color;
		float radius;
	};

	struct Shader
	{
		unsigned int id;
		Shader(const char*, const char*);
		void use() const;
		void del() const;
	};

	struct VAO
	{
		unsigned int id;
		VAO();
		void bind() const;
		void unbind() const;
		void del() const;
	};

	struct VBO
	{
		unsigned int id;
		VBO(long long int, float*);
		void bind() const;
		void unbind() const;
		void del() const;
	};

	struct Camera
	{
		Vec2 position;
		float zoom = 1.0f;
	};
}