#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <print>

namespace engine
{
	static std::string read_file(const char* path)
	{
		std::ifstream file{path};
		if (!file.is_open())
			return {};

		std::string content;
		std::string line;

		while (std::getline(file, line))
			content += line + '\n';

		return content;
	}

	struct Vec2
	{
		float x, y;

		Vec2 operator+(const Vec2& other) const
		{
			return { .x = this->x + other.x, .y = this->y + other.y };
		}

		Vec2 operator-(const Vec2& other) const
		{
			return { .x = this->x - other.x, .y = this->y - other.y };
		}

		Vec2 operator*(const float num) const
		{
			return { .x = this->x * num, .y = this->y * num };
		}

		Vec2 operator/(const float num) const
		{
			return { .x = this->x / num, .y = this->y / num };
		}
	};

	struct Vec3
	{
		float x, y, z;

		bool operator==(const Vec3& other) const
		{
			constexpr float epsilon = 1e-9f;
			return std::fabs(this->x - other.x) < epsilon &&
					std::fabs(this->y - other.y) < epsilon &&
					std::fabs(this->z - other.z) < epsilon;
		}
	};

	extern std::vector<struct Particle*> particles;

	struct Particle
	{
		std::vector<Particle*> bonds;
		struct Nucleus* parent = nullptr;
		Vec3 color{ .x = 0, .y = 0, .z = 0 };
		Vec2 position, velocity{ .x = 0.0f, .y = 0.0f }, virtual_position{ .x = 0.0f, .y = 0.0f };
		float mass, charge, radius, energy = 0.0f, life = 0.0f;
		bool is_electron = false, is_boson = false, is_photon = false, is_antimatter = false;

		Particle(const Vec2 position, const float mass, const float charge, const float radius)
			: position(position), mass(mass), charge(charge), radius(radius) {}

		Particle(const Vec2 position, const Vec3 color, const float mass, const float charge, const float radius)
			: position(position), color(color), mass(mass), charge(charge), radius(radius) {}

		[[nodiscard]] Vec3 get_anti_color() const
		{
			return Vec3{ .x = std::abs(1 - color.x), .y = std::abs(1 - color.y), .z = std::abs(1 - color.z) };
		}

		[[nodiscard]] bool is_anti_quark() const
		{
			constexpr float epsilon = 1e-9f;
			return (std::fabs(color.x + color.y + color.z - 2) < epsilon) || is_antimatter;
		}

		void bond(Particle* other)
		{
			bonds.push_back(other);
			other->bonds.push_back(this);
		}
	};

	struct ParticleType
	{
		static Particle* up() 
		{ 
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
										2.16f, 2.0f / 3.0f, 0.25f };
		};
		static Particle* down() 
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
										4.7f, -1.0f / 3.0f, 0.25f };
		};
		static Particle* charm()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
										1273.0f, 2.0f / 3.0f, 0.35f };
		};
		static Particle* strange()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
										93.5f, -1.0f / 3.0f, 0.35f };
		};
		static Particle* top()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
										172570.0f, 2.0f / 3.0f, 0.4f };
		};
		static Particle* bottom()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
										4183.0f, -1.0f / 3.0f, 0.4f };
		};
		static Particle* electron()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 },
										0.511f, -1.0f, 0.1f };
		};
		static Particle* muon()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 }, 
										105.66f, -1.0f, 0.15f };
		};
		static Particle* tau()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 }, 
										1776.93f, -1.0f, 0.2f };
		};
		static Particle* electron_neutrino()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 }, 
										0.0000008f, 0.0f, 0.05f };
		};
		static Particle* muon_neutrino()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 }, 
										0.19f, 0.0f, 0.1f };
		};
		static Particle* tau_neutrino()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 }, 
										18.2f, 0.0f, 0.15f };
		};
		static Particle* photon()
		{
			return new Particle{ { .x = 0.0f, .y = 0.0f }, 
									{ .x = 1, .y = 1, .z = 1 }, 
										0.0f, 0.0f, 0.1f };
		}
	};

	extern std::vector<struct Nucleus*> nuclei;

	struct Nucleus
	{
		std::vector<Particle*> hadrons{};

		void make_parent() const
		{
			for (Particle* p : hadrons)
				p->parent = const_cast<Nucleus*>(this);
		}

		[[nodiscard]] std::vector<Particle*> get_all_particles() const
		{
			auto result = std::vector<Particle*>{};
			for (Particle* p : hadrons)
			{
				result.push_back(p);
				for (Particle* bond : p->bonds)
					result.push_back(bond);
			}
			return result;
		}
	};

	static Particle* create_proton(const Vec2 position)
	{
		Particle* up1 = ParticleType::up();
		Particle* up2 = ParticleType::up();
		Particle* down = ParticleType::down();

		up1->color.x = 1;
		up2->color.y = 1;
		down->color.z = 1;

		up1->position = { .x = position.x - 0.45f, .y = position.y - 0.225f };
		up2->position = { .x = position.x + 0.45f, .y = position.y - 0.225f };
		down->position = { .x = position.x, .y = position.y + 0.225f };

		particles.push_back(up1);
		particles.push_back(up2);
		particles.push_back(down);

		down->bond(up1);
		down->bond(up2);
		up1->bond(up2);

		return down;
	}

	static Particle* create_neutron(const Vec2 position)
	{
		Particle* down1 = ParticleType::down();
		Particle* down2 = ParticleType::down();
		Particle* up = ParticleType::up();

		down1->color.x = 1;
		down2->color.y = 1;
		up->color.z = 1;

		down1->position = { .x = position.x - 0.45f, .y = position.y - 0.225f };
		down2->position = { .x = position.x + 0.45f, .y = position.y - 0.225f };
		up->position = { .x = position.x, .y = position.y + 0.225f };

		particles.push_back(down1);
		particles.push_back(down2);
		particles.push_back(up);

		up->bond(down1);
		up->bond(down2);
		down1->bond(down2);

		return up;
	}

	static Particle* create_electron(const Vec2 position, const float energy)
	{
		Particle* electron = ParticleType::electron();
		electron->position = position;
		electron->energy = energy;
		electron->is_electron = true;

		particles.push_back(electron);

		return electron;
	}

	static Particle* create_antiproton(const Vec2 position)
	{
		Particle* up1 = ParticleType::up();
		Particle* up2 = ParticleType::up();
		Particle* down = ParticleType::down();

		up1->color = { .x = 0, .y = 1, .z = 1 };
		up2->color = { .x = 1, .y = 0, .z = 1 };
		down->color = { .x = 1, .y = 1, .z = 0 };

		up1->charge *= -1.0f;
		up2->charge *= -1.0f;
		down->charge *= -1.0f;

		up1->position = { .x = position.x - 0.45f, .y = position.y - 0.225f };
		up2->position = { .x = position.x + 0.45f, .y = position.y - 0.225f };
		down->position = { .x = position.x, .y = position.y + 0.225f };

		up1->is_antimatter = true;
		up2->is_antimatter = true;
		down->is_antimatter = true;

		particles.push_back(up1);
		particles.push_back(up2);
		particles.push_back(down);

		down->bond(up1);
		down->bond(up2);
		up1->bond(up2);

		return down;
	}

	static Particle* create_antineutron(const Vec2 position)
	{
		Particle* down1 = ParticleType::down();
		Particle* down2 = ParticleType::down();
		Particle* up = ParticleType::up();

		down1->color = { .x = 0, .y = 1, .z = 1 };
		down2->color = { .x = 1, .y = 0, .z = 1 };
		up->color = { .x = 1, .y = 1, .z = 0 };

		down1->charge *= -1.0f;
		down2->charge *= -1.0f;
		up->charge *= -1.0f;

		down1->position = { .x = position.x - 0.45f, .y = position.y - 0.225f };
		down2->position = { .x = position.x + 0.45f, .y = position.y - 0.225f };
		up->position = { .x = position.x, .y = position.y + 0.225f };

		down1->is_antimatter = true;
		down2->is_antimatter = true;
		up->is_antimatter = true;

		particles.push_back(down1);
		particles.push_back(down2);
		particles.push_back(up);

		up->bond(down1);
		up->bond(down2);
		down1->bond(down2);

		return up;
	}

	static Particle* create_positron(const Vec2 position, const float energy)
	{
		Particle* positron = ParticleType::electron();
		positron->charge *= -1.0f;
		positron->position = position;
		positron->energy = energy;
		positron->is_electron = true;
		positron->is_antimatter = true;

		particles.push_back(positron);

		return positron;
	}

	static Particle* create_photon(const Vec2 position, const float wavelength, const float theta)
	{
		Particle* photon = ParticleType::photon();
		photon->position = position;
		photon->energy = wavelength;
		photon->is_boson = true;
		photon->is_photon = true;
		photon->velocity = Vec2{ .x = std::cos(theta), .y = std::sin(theta) };

		particles.push_back(photon);

		return photon;
	}

	static Particle* create_pion(const Vec2 position, const bool is_positive)
	{
		Particle* up = ParticleType::up();
		Particle* down = ParticleType::down();

		up->position = position - Vec2{ .x = 0.5f, .y = 0.0f };
		down->position = position + Vec2{ .x = 0.5f, .y = 0.0f };

		if (is_positive)
		{
			up->color = { .x = 0, .y = 0, .z = 1 };
			down->color = { .x = 1, .y = 1, .z = 0 };
			down->charge *= -1.0f;
		}
		else
		{
			up->color = { .x = 1, .y = 1, .z = 0 };
			up->charge *= -1.0f;
			down->color = { .x = 0, .y = 0, .z = 1 };
		}

		up->bond(down);

		particles.push_back(up);
		particles.push_back(down);

		return up;
	}

	static Particle* create_neutral_pion(const Vec2 position, const bool is_up)
	{
		Particle* p1;
		Particle* p2;

		if (is_up)
		{
			p1 = ParticleType::up();
			p2 = ParticleType::up();
		}
		else
		{
			p1 = ParticleType::down();
			p2 = ParticleType::down();
		}

		p1->color = { .x = 0, .y = 0, .z = 1 };
		p1->position = position - Vec2{ .x = 0.5f, .y = 0.0f };

		p2->color = { .x = 1, .y = 1, .z = 0 };
		p2->position = position + Vec2{ .x = 0.5f, .y = 0.0f };
		p2->charge *= -1.0f;

		p1->bond(p2);

		particles.push_back(p1);
		particles.push_back(p2);

		return p1;
	}

	static void create_nucleus(const Vec2 position, const unsigned int protons, const unsigned int neutrons)
	{
		std::vector<Particle*> temp;
		Nucleus* nucleus = new Nucleus{};
		temp.reserve((static_cast<std::vector<Particle*>::size_type>(protons) + neutrons) * 3);

		const float center = static_cast<float>(protons + neutrons) * 0.25f;
		float x_offset;
		float y_offset = 0.0f;

		for (unsigned int i = 0; i < protons; i++)
		{
			if (i % 4 == 0 && i != 0)
				y_offset+=1;
			x_offset = static_cast<float>(i % 4) * 2.0f;
			Particle* hadron = create_proton(Vec2{ .x = position.x + x_offset - center, .y = position.y + y_offset - center });
			temp.push_back(hadron);
			temp.push_back(hadron->bonds[0]);
			temp.push_back(hadron->bonds[1]);
		}
		for (unsigned int i = 0; i < neutrons; i++)
		{
			if (i % 4 == 0)
				y_offset+=1;
			x_offset = static_cast<float>(i % 4) * 2.0f;
			Particle* hadron = create_neutron(Vec2{ .x = position.x + x_offset - center, .y = position.y + y_offset - center });
			temp.push_back(hadron);
			temp.push_back(hadron->bonds[0]);
			temp.push_back(hadron->bonds[1]);
		}
		nucleus->hadrons = temp;
		nuclei.push_back(nucleus);
	}

	static void create_antinucleus(const Vec2 position, const unsigned int antiprotons, const unsigned int antineutrons)
	{
		std::vector<Particle*> temp;
		Nucleus* nucleus = new Nucleus{};
		temp.reserve((static_cast<std::vector<Particle*>::size_type>(antiprotons) + antineutrons) * 3);

		const float center = static_cast<float>(antiprotons + antineutrons) * 0.25f;
		float x_offset = 0.0f;
		float y_offset = 0.0f;

		for (unsigned int i = 0; i < antiprotons; i++)
		{
			if (i % 4 == 0 && i != 0)
				y_offset += 1;
			x_offset = static_cast<float>(i % 4) * 2.0f;
			Particle* hadron = create_antiproton(Vec2{ .x = position.x + x_offset - center, .y = position.y + y_offset - center });
			temp.push_back(hadron);
			temp.push_back(hadron->bonds[0]);
			temp.push_back(hadron->bonds[1]);
		}
		for (unsigned int i = 0; i < antineutrons; i++)
		{
			if (i % 4 == 0)
				y_offset += 1;
			x_offset = static_cast<float>(i % 4) * 2.0f;
			Particle* hadron = create_antineutron(Vec2{ .x = position.x + x_offset - center, .y = position.y + y_offset - center });
			temp.push_back(hadron);
			temp.push_back(hadron->bonds[0]);
			temp.push_back(hadron->bonds[1]);
		}
		nucleus->hadrons = temp;
		nuclei.push_back(nucleus);
	}

	namespace input
	{
		static bool w, a, s, d, q, e;
	}
}