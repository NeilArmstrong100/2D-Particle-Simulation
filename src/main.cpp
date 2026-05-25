#include "engine/video/video.h"
#include "engine/physics/physics.h"
#include "engine/threads/thread_pool.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <print>

std::vector<engine::Particle*> engine::particles{};
std::vector<engine::Nucleus*> engine::nuclei{};

static float aspect_ratio = 4.0f / 3.0f;

static void key_callback(GLFWwindow*, int, int, int, int);
static void framebuffer_size_callback(GLFWwindow*, const int width, const int height)
{
	aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	glViewport(0, 0, width, height);
};

static void create_water(engine::Vec2);
static void create_antiwater(engine::Vec2);

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Particle Simulation", nullptr, nullptr);
	
	if (!window)
	{
		std::println("Failed to create OpenGL window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::println("Failed to initialize GLAD");
		glfwTerminate();
		return -2;
	}

	auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	std::println("{}", version);

	constexpr size_t MAX_POINTS = 10000;

	constexpr float sensitivity = 0.0003f;

	using namespace engine::renderer;
	Camera camera{ .position { .x = 0.0f, .y = 0.0f }, .zoom = 0.01f };
	const Shader shader{ "./src/engine/assets/shader.vert", "./src/engine/assets/shader.frag" };
	const VAO vao{};
	const VBO vbo{ MAX_POINTS * sizeof(Vertex), nullptr };

	const GLint camera_pos_id = glGetUniformLocation(shader.id, "cameraPos");
	const GLint zoom_id = glGetUniformLocation(shader.id, "zoom");
	const GLint aspect_ratio_id = glGetUniformLocation(shader.id, "aspectRatio");

	/*engine::create_nucleus({ 0.0f, 0.0f}, 6, 6);
	engine::create_electron({ 30.0f, 0.0f }, 30.0f);
	engine::create_electron({ -30.0f, 0.0f }, 30.0f);
	engine::create_electron({ 40.0f, 0.0f }, 40.0f);
	engine::create_electron({ -40.0f, 0.0f }, 40.0f);
	engine::create_electron({ 0.0f, 40.0f }, 40.0f);
	engine::create_electron({ 0.0f, -40.0f }, 40.0f);

	engine::create_nucleus({ 200.0f, 0.0f }, 8, 8);
	engine::create_electron({ 230.0f, 0.0f }, 30.0f);
	engine::create_electron({ 170.0f, 0.0f }, 30.0f);
	engine::create_electron({ 240.0f, 0.0f }, 40.0f);
	engine::create_electron({ 160.0f, 0.0f }, 40.0f);
	engine::create_electron({ 200.0f, 40.0f }, 40.0f);
	engine::create_electron({ 200.0f, -40.0f }, 40.0f);
	engine::create_electron({ 220.0f, 20.0f }, 40.0f);
	engine::create_electron({ 220.0f, -20.0f }, 40.0f);

	engine::create_nucleus({ -200.0f, 0.0f }, 8, 8);
	engine::create_electron({ -230.0f, 0.0f }, 30.0f);
	engine::create_electron({ -170.0f, 0.0f }, 30.0f);
	engine::create_electron({ -240.0f, 0.0f }, 40.0f);
	engine::create_electron({ -160.0f, 0.0f }, 40.0f);
	engine::create_electron({ -200.0f, 40.0f }, 40.0f);
	engine::create_electron({ -200.0f, -40.0f }, 40.0f);
	engine::create_electron({ -220.0f, 20.0f }, 40.0f);
	engine::create_electron({ -220.0f, -20.0f }, 40.0f);*/

	/*engine::create_proton({ -30.0f, 0.0f });
	engine::create_electron({ 0.0f, 10.0f }, 30.0f);
	engine::create_proton({ 30.0f, 0.0f });
	engine::create_electron({ 0.0f, -10.0f }, 30.0f);

	engine::create_proton({ -30.0f, 100.0f });
	engine::create_electron({ 0.0f, 110.0f }, 30.0f);
	engine::create_proton({ 30.0f, 100.0f });
	engine::create_electron({ 0.0f, 90.0f }, 30.0f);*/

	for (unsigned int i = 0; i < 10; i++)
		for (unsigned int j = 0; j < 10; j++)
			engine::create_nucleus({15.0f * i, 15.0f * j}, 1, 0);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof Vertex, reinterpret_cast<const void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof Vertex, reinterpret_cast<const void*>(offsetof(Vertex, color)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof Vertex, reinterpret_cast<const void*>(offsetof(Vertex, radius)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	engine::physics::start(1000, false, true);

	// TODO : split the work in the physics thread into multiple jobs to be processed in parallel by the thread pool, instead of just running the entire physics loop as a single job.
	ThreadPool* pool = new ThreadPool();
	pool->start(1);
	pool->queue_job(engine::physics::run);

	glEnable(GL_PROGRAM_POINT_SIZE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		using namespace engine::input;
		Vertex vertices[MAX_POINTS];

		uint32_t offset = 0;
		for (const engine::Particle* particle : engine::particles)
		{
			const Vertex src = { .position = particle->virtual_position, .color = particle->color, .radius = particle->radius * 800.0f };
			vertices[offset] = src;
			offset++;
		}

		glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(offset * sizeof(Vertex)), vertices);

		if (w) camera.position.y -= sensitivity / camera.zoom;
		if (a) camera.position.x += sensitivity / camera.zoom;
		if (s) camera.position.y += sensitivity / camera.zoom;
		if (d) camera.position.x -= sensitivity / camera.zoom;

		if (q) camera.zoom *= 1 - sensitivity;
		if (e) camera.zoom *= 1 + sensitivity;

		glClear(GL_COLOR_BUFFER_BIT);

		glUniform2f(camera_pos_id, camera.position.x, camera.position.y);
		glUniform1f(zoom_id, camera.zoom);
		glUniform1f(aspect_ratio_id, aspect_ratio);

		shader.use();
		glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(offset));

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	engine::physics::stop();

	shader.del();
	vao.del();
	vbo.del();

	pool->stop();

	glfwDestroyWindow(window);
	glfwTerminate();
}

static void key_callback(GLFWwindow* window, const int key, const int scan, const int action, const int mods)
{
	using namespace engine::input;
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_W) w = true;
		if (key == GLFW_KEY_A) a = true;
		if (key == GLFW_KEY_S) s = true;
		if (key == GLFW_KEY_D) d = true;
		if (key == GLFW_KEY_Q) q = true;
		if (key == GLFW_KEY_E) e = true;
	}
	else
	{
		if (key == GLFW_KEY_W) w = false;
		if (key == GLFW_KEY_A) a = false;
		if (key == GLFW_KEY_S) s = false;
		if (key == GLFW_KEY_D) d = false;
		if (key == GLFW_KEY_Q) q = false;
		if (key == GLFW_KEY_E) e = false;
	}
}

void create_water(const engine::Vec2 position)
{
	const float x = position.x;
	const float y = position.y;

	engine::create_nucleus({ .x = x, .y = y }, 8, 8);
	engine::create_electron({ .x = x + 15.0f, .y = y }, 15.0f);
	engine::create_electron({ .x = x - 15.0f, .y = y }, 15.0f);
	engine::create_electron({ .x = x + 20.0f, .y = y + 15.0f }, 25.0f);
	engine::create_electron({ .x = x + 20.0f, .y = y - 15.0f }, 25.0f);
	engine::create_electron({ .x = x - 20.0f, .y = y + 15.0f }, 25.0f);
	engine::create_electron({ .x = x - 20.0f, .y = y - 15.0f }, 25.0f);
	engine::create_electron({ .x = x + 15.0f, .y = y + 20.0f }, 25.0f);
	engine::create_electron({ .x = x + 15.0f, .y = y - 20.0f }, 25.0f);

	engine::create_nucleus({ .x = x + 100.0f, .y = y }, 1, 0);
	engine::create_electron({ .x = x + 75.0f, .y = y }, 25.0f);
	engine::create_nucleus({ .x = x - 100.0f, .y = y }, 1, 0);
	engine::create_electron({ .x = x - 75.0f, .y = y }, 25.0f);
}

void create_antiwater(const engine::Vec2 position)
{
	const float x = position.x;
	const float y = position.y;

	engine::create_antinucleus({ .x = x, .y = y }, 8, 8);
	engine::create_positron({ .x = x + 30.0f, .y = y }, 15.0f);
	engine::create_positron({ .x = x - 30.0f, .y = y }, 15.0f);
	engine::create_positron({ .x = x + 40.0f, .y = y + 10.0f }, 20.0f);
	engine::create_positron({ .x = x + 40.0f, .y = y - 10.0f }, 20.0f);
	engine::create_positron({ .x = x - 40.0f, .y = y + 10.0f }, 20.0f);
	engine::create_positron({ .x = x - 40.0f, .y = y - 10.0f }, 20.0f);
	engine::create_positron({ .x = x + 10.0f, .y = y + 40.0f }, 20.0f);
	engine::create_positron({ .x = x + 10.0f, .y = y - 40.0f }, 20.0f);

	engine::create_antinucleus({ .x = x + 100.0f, .y = y }, 1, 0);
	engine::create_positron({ .x = x + 60.0f, .y = y }, 20.0f);
	engine::create_antinucleus({ .x = x - 100.0f, .y = y }, 1, 0);
	engine::create_positron({ .x = x - 60.0f, .y = y }, 20.0f);
}