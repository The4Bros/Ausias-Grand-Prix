#pragma once
#include "Module.h"
#include "Globals.h"
#include "p2Point.h"

struct PhysVehicle3D;

#define MAX_ACCELERATION 1000.0f
#define TURN_DEGREES 15.0f * DEGTORAD
#define BRAKE_POWER 1000.0f

enum CameraStates
{
	FREE = 0,
	PILOT_VIEW,
	THIRD_PERSON_VIEW
};

class CameraController
{
public:
	CameraController();

	bool Start(Application* App);
	void Update();
	bool Follow(PhysVehicle3D* _vehicle, CameraStates _state = CameraStates::THIRD_PERSON_VIEW);

public:
	Application* App;
	PhysVehicle3D* vehicle;
	CameraStates state;
};

class ModulePlayer : public Module
{
public:
	ModulePlayer(Application* app, bool start_enabled = true);
	virtual ~ModulePlayer();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

public:
	void Respawn(float degrees, vec3 pos);
	PhysVehicle3D* vehicle;
	float turn;
	float acceleration;
	float brake;

	CameraController camera;
};