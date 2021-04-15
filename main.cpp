#include "src/Game.h"
#include "src/ecs/ECS.h"
#include "src/ecs/components/Transform.h"
#include "src/ecs/components/Camera.h"
#include "src/ecs/components/Renderable.h"
#include "src/ecs/components/Light.h"
#include "src/ecs/components/Rigidbody.h"
#include <random>

ECS ecs;

int main(void) {
	ecs.init();

	Game g;
	g.applicationName = "NeigeEngine Test";

	window.extent.width = 720;
	window.extent.height = 480;

	g.init();

	Entity sceneCamera = ecs.createEntity();
	ecs.addComponent(sceneCamera, Camera{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		45.0f,
		0.3f,
		200.0f,
		"../modelfiles/sunset_forest_8k.hdr"
		});

	Entity bistro = ecs.createEntity();
	ecs.addComponent(bistro, Renderable{
		"../modelfiles/Bistro_v5_1/bistrointe.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(bistro, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});

	Entity bistroLight1 = ecs.createEntity();
	ecs.addComponent(bistroLight1, Light{
		LightType::SPOT,
		glm::vec3(2.63273f, 1.44006f, -0.314734f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(40.0f, 50.0f)
		});

	Entity bistroLight2 = ecs.createEntity();
	ecs.addComponent(bistroLight2, Light{
		LightType::SPOT,
		glm::vec3(4.23652f, 1.44006f, 0.513211f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(40.0f, 50.0f)
		});

	Entity bistroLight3 = ecs.createEntity();
	ecs.addComponent(bistroLight3, Light{
		LightType::SPOT,
		glm::vec3(2.25823f, 1.44006f, -2.64679f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(40.0f, 50.0f)
		});

	Entity bistroLight4 = ecs.createEntity();
	ecs.addComponent(bistroLight4, Light{
		LightType::SPOT,
		glm::vec3(3.24595f, 1.44006f, -4.52136),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(40.0f, 50.0f)
		});

	/*Entity porsche = ecs.createEntity();
	ecs.addComponent(porsche, Renderable{
		"../modelfiles/free_1975_porsche_911_930_turbo/scene.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(porsche, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});*/

	/*Entity alphablendmode = ecs.createEntity();
	ecs.addComponent(alphablendmode, Renderable{
		"../../glTF-Sample-Models-master/2.0/AlphaBlendModeTest/glTF/AlphaBlendModeTest.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(alphablendmode, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});

	Entity sr = ecs.createEntity();
	ecs.addComponent(sr, Renderable{
		"../modelfiles/spherered.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(sr, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(5.0f, 5.0f, 5.0f)
		});

	Entity sb = ecs.createEntity();
	ecs.addComponent(sb, Renderable{
		"../modelfiles/sphereblue.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(sb, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(3.0f, 3.0f, 3.0f)
		});

	Entity sv = ecs.createEntity();
	ecs.addComponent(sv, Renderable{
		"../modelfiles/spheregreen.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(sv, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});*/

	/*std::default_random_engine generator;
	std::uniform_real_distribution<float> randomPosition(-10.0f, 10.0f);

	std::vector<Entity> createdEntities(2000);
	for (float i = -1000.0f; i < 1000.0f; i++) {
			Entity e = ecs.createEntity();
			ecs.addComponent(e, Renderable{
			"../modelfiles/Sphere.gltf",
			"../shaders/pbr.vert",
			"../shaders/pbr.frag",
			"",
			"",
			"",
			Topology::TRIANGLE_LIST
				});
			ecs.addComponent(e, Transform{
				glm::vec3(randomPosition(generator), -2.0f, randomPosition(generator)),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(1.0f)
				});
	}*/

	/*Entity boomBox = ecs.createEntity();
	ecs.addComponent(boomBox, Renderable{
		"../modelfiles/BoomBox.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(boomBox, Transform{
		glm::vec3(3.0f, 2.0f, 0.0f),
		glm::vec3(180.0f, 0.0f, 0.0f),
		glm::vec3(100.0f, 100.0f, 100.0f)
		});

	Entity entity2 = ecs.createEntity();
	ecs.addComponent(entity2, Renderable{
		"../modelfiles/Duck2.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(entity2, Transform{
		glm::vec3(5.0f, 1.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});*/

	/*Entity entity8 = ecs.createEntity();
	ecs.addComponent(entity8, Renderable{
		"../modelfiles/Duck2.gltf",
		"../shaders/pbr.vert",
		"../shaders/water.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(entity8, Transform{
		glm::vec3(3.0f, 1.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});*/

	/*Entity entity3 = ecs.createEntity();
	ecs.addComponent(entity3, Renderable{
		"../modelfiles/DamagedHelmet.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(entity3, Transform{
		glm::vec3(0.0f, 1.5f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});*/

	/*Entity entity4 = ecs.createEntity();
	ecs.addComponent(entity4, Renderable{
		"../modelfiles/sphere.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(entity4, Transform{
		glm::vec3(5.0f, 1.0f, -4.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});

	Entity sponza = ecs.createEntity();
	ecs.addComponent(sponza, Renderable{
		"../../glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf",
		"../shaders/pbr.vert",
		"../shaders/pbr.frag",
		"",
		"",
		"",
		Topology::TRIANGLE_LIST
		});
	ecs.addComponent(sponza, Transform{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
		});*/

	/*Entity light1 = ecs.createEntity();
	ecs.addComponent(light1, Light{
		LightType::DIRECTIONAL,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(0.0f, 0.0f)
		});

	Entity light2 = ecs.createEntity();
	ecs.addComponent(light2, Light{
		LightType::SPOT,
		glm::vec3(5.0f, 4.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec2(40.0f, 50.0f)
		});

	Entity light3 = ecs.createEntity();
	ecs.addComponent(light3, Light{
		LightType::POINT,
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec2(0.0f)
		});

	Entity light4 = ecs.createEntity();
	ecs.addComponent(light4, Light{
		LightType::POINT,
		glm::vec3(-1.0f, 0.0f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec2(0.0f)
		});

	Entity light5 = ecs.createEntity();
	ecs.addComponent(light5, Light{
		LightType::POINT,
		glm::vec3(1.0f, 0.0f, -1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec2(0.0f)
		});

	Entity light6 = ecs.createEntity();
	ecs.addComponent(light6, Light{
		LightType::POINT,
		glm::vec3(-1.0f, 0.0f, -1.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec2(0.0f)
		});

	Entity light7 = ecs.createEntity();
	ecs.addComponent(light7, Light{
		LightType::POINT,
		glm::vec3(1.0f, 2.0f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f, 0.25f, 0.75f),
		glm::vec2(0.0f)
		});

	Entity light8 = ecs.createEntity();
	ecs.addComponent(light8, Light{
		LightType::POINT,
		glm::vec3(1.0f, 2.0f, -1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.5f, 1.0f, 0.75f),
		glm::vec2(0.0f)
		});

	Entity light9 = ecs.createEntity();
	ecs.addComponent(light9, Light{
		LightType::POINT,
		glm::vec3(-1.0f, 2.0f, 1.0f),
		glm::vec3(0.0f),
		glm::vec3(1.0f, 0.5f, 0.5f),
		glm::vec2(0.0f)
		});

	Entity light10 = ecs.createEntity();
	ecs.addComponent(light10, Light{
		LightType::POINT,
		glm::vec3(-1.0f, 2.0f, -1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.5f, 0.0f, 0.5f),
		glm::vec2(0.0f)
		});*/

	g.launch();
}