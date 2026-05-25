#include "physics.h"
#include "../utility/utility.h"
#include "../math/vector.h"
#include "../math/force.h"

#include <chrono>
#include <algorithm>
#include <functional>
#include <vector>
#include <cmath>

using namespace engine;

static float wavelength(const float total_energy)
{
	return (math::LIGHTSPEED * 10.0f) / total_energy;
}

static Vec2 center_of_mass(const Particle* particle)
{
	Vec2 com = particle->position * particle->mass;
	float total_mass = particle->mass;

	for (const Particle* p : particle->bonds)
	{
		com = com + (p->position * p->mass);
		total_mass += p->mass;
	}

	if (total_mass > 0.0f)
		com = com / total_mass;

	return com;
}

static Vec2 center_of_mass(const Nucleus* nucleus)
{
	Vec2 com = { 0.0f, 0.0f };
	float total_mass = 0.0f;

	for (Particle* p : nucleus->hadrons)
	{
		com = com + (p->position * p->mass);
		total_mass += p->mass;
	}

	if (total_mass > 0.0f)
		com = com / total_mass;

	return com;
}

//static void electron_shell_collision(Particle* electron, const Particle* nucleus) 
//{
//	const Vec2 center = nucleus->parent ? center_of_mass(nucleus->parent) : center_of_mass(nucleus);
//	const Vec2 offset = electron->position - center;
//	const float dist = math::magnitude(offset);
//
//	if (dist < electron->energy) 
//	{
//		const Vec2 normal = offset / dist;
//		const float penetration = electron->energy - dist;
//
//		electron->position = electron->position + normal * penetration;
//
//		if (const float radial_velocity = electron->velocity.x * normal.x + electron->velocity.y * normal.y; radial_velocity < 0.0f) 
//			electron->velocity = electron->velocity - normal * radial_velocity;
//	}
//}

void physics::start(const uint32_t target_framerate, const bool electrons_orbit, const bool photons_waves)
{
	framerate = target_framerate;
	electron_orbit = electrons_orbit;
	photon_waves = photons_waves;
}

void physics::run()
{
	// Defer deleting nuclei until the simulation thread is about to exit to avoid
	// potential read-access from other threads (e.g. rendering) referencing those pointers.
	std::vector<Nucleus*> deferred_deletions;

	auto target = std::chrono::nanoseconds(1'000'000'000 / framerate);
	auto last = std::chrono::high_resolution_clock::now();
	while (true)
	{
		auto now = std::chrono::high_resolution_clock::now();

		if (auto elapsed = now - last; elapsed >= target)
		{
			last = now;

			float dt = static_cast<float>(elapsed.count()) / 1'000'000'000;

			size_t size;

			for (Nucleus* nucleus : nuclei)
			{
				size = nucleus->hadrons.size();

				for (size_t i = 0; i < size; ++i)
					for (size_t j = i + 1; j < size; ++j)
					{
						Particle* a = nucleus->hadrons[i];
						Particle* b = nucleus->hadrons[j];

						const Vec2 a_com = center_of_mass(a);
						const Vec2 b_com = center_of_mass(b);

						float a_total_mass = a->mass;
						float b_total_mass = b->mass;
						for (Particle* p : a->bonds)
							a_total_mass += p->mass;
						for (Particle* p : b->bonds)
							b_total_mass += p->mass;

						const Vec2 dir = math::normalize(b_com - a_com);
						const float r = std::max(math::distance(a_com, b_com), 0.01f);

						const float yukawa = math::yukawa(1.8f, r);

						const Vec2 a_force = dir * math::acceleration(yukawa, a_total_mass) * dt;
						const Vec2 b_force = dir * math::acceleration(yukawa, b_total_mass) * dt;

						a->velocity = a->velocity + a_force;
						b->velocity = b->velocity - b_force;

						for (Particle* p : a->bonds)
							p->velocity = p->velocity + a_force;
						for (Particle* p : b->bonds)
							p->velocity = p->velocity - b_force;

						Vec2 momentum = { 0, 0 };
						float total_mass = 0.0f;

						for (Particle* p : nucleus->hadrons)
						{
							momentum = momentum + p->velocity * p->mass;
							total_mass += p->mass;
						}

						Vec2 drift_velocity = momentum / total_mass;
						for (Particle* p : nucleus->hadrons)
							p->velocity = p->velocity - drift_velocity;
					}
			}

			size = particles.size();

			for (size_t i = 0; i < size; ++i)
				for (size_t j = i + 1; j < size; ++j)
				{
					Particle* a = particles[i];
					Particle* b = particles[j];

					float r = math::distance(a->position, b->position);
					float r2 = r * r;

					std::function<void()> annihilate = [&]() {
						a->is_electron = false;
						a->is_boson = true;
						a->is_photon = true;
						a->is_antimatter = false;

						b->is_electron = false;
						b->is_boson = true;
						b->is_photon = true;
						b->is_antimatter = false;

						const float total_energy = a->mass + b->mass;
						a->energy = wavelength(total_energy / 2);
						b->energy = wavelength(total_energy / 2);

						a->mass = 0.0f;
						b->mass = 0.0f;

						a->color = { .x = 1, .y = 1, .z = 1 };
						b->color = { .x = 1, .y = 1, .z = 1 };

						a->charge = 0.0f;
						b->charge = 0.0f;

						a->radius = 0.1f;
						b->radius = 0.1f;

						const Vec2 dir_a = math::normalize(a->velocity);
						const Vec2 dir_b = math::normalize(b->velocity);

						a->velocity = dir_a * math::LIGHTSPEED;
						b->velocity = dir_b * math::LIGHTSPEED;

						for (Particle* p : a->bonds)
						{
							if (auto it = std::ranges::find(p->bonds.begin(), p->bonds.end(), a); it != p->bonds.end())
								p->bonds.erase(it);
						}

						for (Particle* p : b->bonds)
						{
							
							if (auto it = std::ranges::find(p->bonds.begin(), p->bonds.end(), b); it != p->bonds.end())
								p->bonds.erase(it);
						}

						a->bonds.clear();
						b->bonds.clear();
					};

					if (a->is_anti_quark() && !b->is_anti_quark() && a->mass == b->mass && a->is_electron == b->is_electron && r < 1.0f)
						annihilate();

					if (!a->is_anti_quark() && b->is_anti_quark() && a->mass == b->mass && a->is_electron == b->is_electron && r < 1.0f)
						annihilate();

					float gravity = math::gravity(a->mass, b->mass, r2);				// Gravitational Force
					float coulomb = math::coulomb(a->charge, b->charge, r2);			// Electromagnetic Force
					float qq = math::qq_interactions(a->color, b->color, 0.18f, r);		// Strong Force
					float qaq = math::qaq_interactions(a->color, b->color, 0.18f, r);

					if (a->is_anti_quark() == b->is_anti_quark())
						qaq = 0;
					else
						qq = 0;

					float strong = qq + qaq;

					for (Particle* p : a->bonds)
					{
						if (p == b && r >= 50.0f)
						{
							a->bonds.erase(std::ranges::find(a->bonds.begin(), a->bonds.end(), b));
							b->bonds.erase(std::ranges::find(b->bonds.begin(), b->bonds.end(), a));

							Vec2 midpoint = (a->position + b->position) / 2.0f;
							Vec2 epsilon = { .x = 0.1f, .y = 0.1f };

							Particle* new_a = new Particle{ midpoint + epsilon, a->get_anti_color(), a->mass, -a->charge, a->radius };
							Particle* new_b = new Particle{ midpoint - epsilon, b->get_anti_color(), b->mass, -b->charge, b->radius };

							new_a->velocity = a->velocity;
							new_b->velocity = b->velocity;

							a->bond(new_a);
							b->bond(new_b);

							particles.push_back(new_a);
							particles.push_back(new_b);
						}
					}

					if (r < (a->radius + b->radius) * 2 && a->bonds.size() < 2 && b->bonds.size() < 2 && 
						a->color != b->color && !a->is_electron && !b->is_electron && !a->is_boson && !b->is_boson)
					{
						bool flag = false;
						for (Particle* p : a->bonds)
							if (p == b)
								flag = true;
						if (!flag)
						{
							a->bonds.push_back(b);
							b->bonds.push_back(a);
						}
					}

					if (a->bonds.size() == 2 && b->bonds.size() == 2 && r < 10.0f)
					{
						if (a->parent && b->parent && a->parent != b->parent)
						{
							Nucleus* old = b->parent;
							auto& oldh = old->hadrons;
							for (Particle* p : oldh)
								p->parent = a->parent;
							// Instead of deleting 'old' immediately (which can cause read-access
							// violations if other threads are reading nuclei), schedule it for
							// deletion when the simulation thread exits.
							if (auto it = std::ranges::find(nuclei.begin(), nuclei.end(), old); it != nuclei.end())
							{
								// remove from active nuclei list now so it won't be processed later
								nuclei.erase(it);
								deferred_deletions.push_back(old);
							}
						}
						if (!a->parent && !b->parent)
						{
							Nucleus* n = new Nucleus{};
							n->hadrons.push_back(a);
							n->hadrons.push_back(b);
							n->make_parent();
							nuclei.push_back(n);
						}
						if (a->parent && !b->parent)
						{
							b->parent = a->parent;
							a->parent->hadrons.push_back(b);
						}
						if (b->parent && !a->parent)
						{
							a->parent = b->parent;
							b->parent->hadrons.push_back(a);
						}
					}

					bool flag = false;
					for (Particle* p : a->bonds)
					{
						if (p == b)
							flag = true;
					}
					if (!flag)
						strong = 0;

					// Electron interactions (strengthened for stability)
					if (a->is_electron || b->is_electron)
					{
						coulomb *= 10000.0f;

						// electron-electron repulsion (weakened for stability)
						if (a->is_electron && b->is_electron)
							coulomb *= 0.05f;
					}

					// Nucleus-nucleus electromagnetic force (strengthened)
					if (!a->is_electron && !b->is_electron)
						if (a->parent != b->parent)
							coulomb *= 10000.0f; // tune this

					// Electron-shell collision handling (disabled for now, as it can cause instability)
					if (a->is_electron &&
						!b->is_electron &&
						!b->is_boson)
					{
						//electron_shell_collision(a, b);
						coulomb *= -1.0f;
					}

					if (b->is_electron &&
						!a->is_electron &&
						!a->is_boson)
					{
						//electron_shell_collision(b, a);
						coulomb *= -1.0f;
					}

					const Vec2 dir = math::normalize(b->position - a->position);

					float a_acceleration = math::acceleration(gravity + coulomb - strong, a->mass);
					const Vec2 force_a = dir * a_acceleration * dt;

					float b_acceleration = math::acceleration(gravity + coulomb - strong, b->mass);
					const Vec2 force_b = dir * b_acceleration * dt;

					if (a->mass != 0.0f)
						a->velocity = a->velocity + force_a;
					if (b->mass != 0.0f)
						b->velocity = b->velocity - force_b;

					// Electron movement
					if (electron_orbit)
						math::electron_orbit(a, b);

					// Photon wave movement (life is used as theta)
					if (a->is_photon)
					{
						auto[dx, dy] = math::normalize(a->velocity);
						Vec2 perp = { .x = -dy, .y = dx };

						if (a->energy != 0.0f)
						{
							a->life += dt / a->energy;

							float wave = std::sin(a->life) * (math::WAVE_HEIGHT / a->energy);

							a->virtual_position = a->position + perp * wave;
						}
						else
							a->virtual_position = a->position;
					}
					if (b->is_photon)
					{
						auto[dx, dy] = math::normalize(b->velocity);
						Vec2 perp = { .x = -dy, .y = dx };

						if (b->energy != 0.0f)
						{
							b->life += dt / b->energy;

							float wave = std::sin(b->life) * (math::WAVE_HEIGHT / b->energy);

							b->virtual_position = b->position + perp * wave;
						}
						else
							b->virtual_position = b->position;
					}

					if (a->mass > 0.0f &&
						b->mass > 0.0f &&
						r <= a->radius + b->radius)
					{
						Vec2 normal = math::normalize(b->position - a->position);

						float penetration = a->radius + b->radius - r;

						Vec2 correction = normal * (penetration * 0.5f);

						a->position = a->position - correction;
						b->position = b->position + correction;

						Vec2 rel = b->velocity - a->velocity;

						float separating = rel.x * normal.x + rel.y * normal.y;

						if (separating < 0.0f)
						{
							Vec2 impulse = normal * separating * 0.5f;

							a->velocity = a->velocity + impulse;
							b->velocity = b->velocity - impulse;
						}
					}

					// Quark decay
					math::decay(a, b, dt);
				}

			for (Particle* particle : particles)
			{
				particle->velocity.x = std::clamp(particle->velocity.x, -math::LIGHTSPEED, math::LIGHTSPEED);
				particle->velocity.y = std::clamp(particle->velocity.y, -math::LIGHTSPEED, math::LIGHTSPEED);

				particle->position.x += particle->velocity.x;
				particle->position.y += particle->velocity.y;

				if (photon_waves)
				{
					if (!particle->is_photon)
						particle->virtual_position = particle->position;
				}
				else
					particle->virtual_position = particle->position;
				
			}

			if (stop_flag.load())
			{
				// Safe deletion of previously-removed nuclei now that the simulation is stopping.
				for (Nucleus* n : deferred_deletions)
					delete n;
				deferred_deletions.clear();
				return;
			}
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
void physics::stop()
{
	stop_flag.store(true);
}