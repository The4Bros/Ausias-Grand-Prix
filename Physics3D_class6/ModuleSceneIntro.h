#pragma once
#include "Module.h"
#include "p2DynArray.h"
#include "Globals.h"
#include "Primitive.h"

#define MAX_SNAKE 2

struct PhysBody3D;
struct PhysMotor3D;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void OnCollision(PhysBody3D* body1, PhysBody3D* body2);

public:
	uint checkpointCounter;
	Timer* timerCount;
	float time;
	float last_time;
	float best_time;
	Cube s;
	PhysBody3D* sensor;
	Cube r;
	PhysBody3D* sensor2;
	Cube t;
	PhysBody3D* sensor3;
	Cube v;
	PhysBody3D* sensor4;
};
