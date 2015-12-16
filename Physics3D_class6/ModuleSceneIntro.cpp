#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	timerCount = new Timer();
	time = 0.0f;
	last_time = 0.0f;
	best_time = 500.0f;
	LOG("Loading Intro assets");
	bool ret = true;
	timerCount->Start();
	App->camera->Move(vec3(1.0f, 20.0f, 1.0f));
	App->camera->LookAt(vec3(0, 0, 0));


	//Circuit Walls ---------------------------------------------------------
	App->physics->AddStraightRoad(100, vec3(0.0f, 0.0f, 0.0f), 1);
	App->physics->AddCurve(20, vec3(0.0f, 0.0f, 50.0f),0);
	App->physics->AddStraightRoad(40, vec3(20.0f, 0.0f, 90.0f), 1);
	App->physics->AddCurve(20, vec3(20.0f, 0.0f, 110.0f), 1);
	App->physics->AddCurve(20, vec3(0.0f, 0.0f, 130.0f), 1);
	App->physics->AddCorner(30, vec3(10.0f, 0.0f, 165.0f), 0);
	App->physics->AddStraightRoad(40, vec3(-40.0f, 0.0f, 150.0f), 0);
	App->physics->AddCurveX(20, vec3(-80.0f, 0.0f, 170.0f), 1);
	App->physics->AddStraightRoad(80, vec3(-120.0f, 0.0f, 170.0f), 0);
	App->physics->AddCorner(30, vec3(-175.0f, 0.0f, 170.0f), 1);
	App->physics->AddStraightRoad(30, vec3(-190.0f, 0.0f, 155.0f), 1);
	App->physics->AddCorner(30, vec3(-190.0f, 0.0f, 125.0f), 2);
	App->physics->AddCurveX(60, vec3(-160.0f, 0.0f, 110.0f), 1);
	App->physics->AddCorner(30, vec3(-70.0f, 0.0f, 65.0f), 0);
	App->physics->AddStraightRoad(30, vec3(-100.0f, 0.0f, 35.0f), 1);
	App->physics->AddCorner(30, vec3(-70.0f, 0.0f, 5.0f), 3);
	App->physics->AddCurveX(60, vec3(-160.0f, 0.0f, -70.0f), 0);
	App->physics->AddCorner(30, vec3(-175.0f, 0.0f, -70.0f), 1);
	App->physics->AddStraightRoad(60, vec3(-190.0f, 0.0f, -100.0f), 1);
	App->physics->AddCurve(60, vec3(-130.0f, 0.0f, -190.0f), 1);
	App->physics->AddCorner(30, vec3(-130.0f, 0.0f, -205.0f), 2);
	App->physics->AddCorner(30, vec3(-70.0f, 0.0f, -205.0f), 3);
	App->physics->AddStraightRoad(100, vec3(-100.0f, 0.0f, -140.0f), 1);
	App->physics->AddCorner(30, vec3(-85.0f, 0.0f, -90.0f), 1);
	App->physics->AddStraightRoad(20, vec3(-60.0f, 0.0f, -90.0f), 0);
	App->physics->AddCorner(30, vec3(-20.0f, 0.0f, -75.0f), 0);
	App->physics->AddStraightRoad(100, vec3(-50.0f, 0.0f, -140.0f), 1);
	App->physics->AddCorner(30, vec3(-50.0f, 0.0f, -205.0f), 2);
	App->physics->AddStraightRoad(20, vec3(-10.0f, 0.0f, -220.0f), 0);
	App->physics->AddCorner(30, vec3(30.0f, 0.0f, -205.0f), 3);
	App->physics->AddStraightRoad(140, vec3(0.0f, 0.0f, -120.0f), 1);

	//Ramp------------------------------------------------------
	r.size = vec3(10, 10, 1);
	r.SetPos( 5, 1, -45);
	r.SetRotation(60, vec3(1, 0, 0));
	sensor2 = App->physics->AddBody(r, 0.0f);
	sensor2->SetAsSensor(false);
	sensor2->collision_listeners.add(this);


	//Sensors ----------------------------------------------------
	s.size = vec3(30, 50, 1);
	s.SetPos(15, 25.0f, 0);
	sensor = App->physics->AddBody(s, 0.0f);
	sensor->SetAsSensor(true);
	sensor->collision_listeners.add(this);

	t.size = vec3(30, 10, 1);
	t.SetPos(-175.0f, 0.0f, 155.0f);
	sensor3 = App->physics->AddBody(t, 0.0f);
	sensor3->SetAsSensor(true);
	sensor3->collision_listeners.add(this);

	v.size = vec3(1,10,30);
	v.SetPos(-55.0f, 1.0f, -75.0f);
	sensor4 = App->physics->AddBody(v, 0.0f);
	sensor4->SetAsSensor(true);
	sensor4->collision_listeners.add(this);
	

	w.size = vec3(60, 60, 1);
	w.SetPos(0, 40.0f, 60);
	sensor5 = App->physics->AddBody(w, 0.0f);
	sensor5->SetAsSensor(true);
	sensor5->collision_listeners.add(this);

	
	trial = App->textures->Load("snoop.png");

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	Plane p(0, 1, 0,0);
	p.color = Black;
	p.axis = true;
	p.Render();
	App->physics->RenderWalls();
	sensor->GetTransform(&s.transform);
	sensor2->GetTransform(&r.transform);
	sensor3->GetTransform(&t.transform);
	sensor4->GetTransform(&v.transform);
	r.Render();

	App->render2D->Blit(trial, 0, 0, NULL);
	
	char title[80];
	sprintf_s(title, "Last Time Lap %0.3f    -    Best Lap time : %0.3f", last_time, best_time);
	App->window->SetTitle(title);

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{
	
	if (body1 == sensor){

		if (checkpointCounter == 3){

			time = timerCount->Read() - time;
			last_time = (time/1000);

			if (last_time < best_time ){

				best_time = last_time;

			}

			LOG("LAST TIME %f", last_time);
			LOG("BEST TIME %f", best_time);
		}
		checkpointCounter = 1;

	}

	if (body1 == sensor3){

		checkpointCounter = 2;

	}

	if (body1 == sensor4){
		if (checkpointCounter == 2){
		checkpointCounter = 3;
		}
	}

	if (body1 == sensor5){
		
		

	}

}

