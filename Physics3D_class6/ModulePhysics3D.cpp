#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "PhysBody3D.h"
#include "PhysVehicle3D.h"
#include "Primitive.h"

#ifdef _DEBUG
	#pragma comment (lib, "Bullet/bin/BulletDynamics_vs2010_debug.lib")
	#pragma comment (lib, "Bullet/bin/BulletCollision_vs2010_debug.lib")
	#pragma comment (lib, "Bullet/bin/LinearMath_vs2010_debug.lib")
#else
	#pragma comment (lib, "Bullet/bin/BulletDynamics_vs2010.lib")
	#pragma comment (lib, "Bullet/bin/BulletCollision_vs2010.lib")
	#pragma comment (lib, "Bullet/bin/LinearMath_vs2010.lib")
#endif

ModulePhysics3D::ModulePhysics3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	debug = true;

	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	debug_draw = new DebugDrawer();
}

// Destructor
ModulePhysics3D::~ModulePhysics3D()
{
	delete debug_draw;
	delete solver;
	delete broad_phase;
	delete dispatcher;
	delete collision_conf;
}

// Render not available yet----------------------------------
bool ModulePhysics3D::Init()
{
	LOG("Creating 3D Physics simulation");
	bool ret = true;

	return ret;
}

// ---------------------------------------------------------
bool ModulePhysics3D::Start()
{
	LOG("Creating Physics environment");

	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);
	vehicle_raycaster = new btDefaultVehicleRaycaster(world);

	

	// Big plane as ground
	{
		btCollisionShape* colShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

		btDefaultMotionState* myMotionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape);

		btRigidBody* body = new btRigidBody(rbInfo);
		world->addRigidBody(body);
	}

	return true;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PreUpdate(float dt)
{
	world->stepSimulation(dt, 15);

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			PhysBody3D* pbodyA = (PhysBody3D*)obA->getUserPointer();
			PhysBody3D* pbodyB = (PhysBody3D*)obB->getUserPointer();

			if(pbodyA && pbodyB)
			{
				p2List_item<Module*>* item = pbodyA->collision_listeners.getFirst();
				while(item)
				{
					item->data->OnCollision(pbodyA, pbodyB);
					item = item->next;
				}

				item = pbodyB->collision_listeners.getFirst();
				while(item)
				{
					item->data->OnCollision(pbodyB, pbodyA);
					item = item->next;
				}
			}
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::Update(float dt)
{
	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(debug == true)
	{
		world->debugDrawWorld();

		// Render vehicles
		p2List_item<PhysVehicle3D*>* item = vehicles.getFirst();
		while(item)
		{
			item->data->Render();
			item = item->next;
		}

		


		if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		{
			Sphere s(1);
			s.SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
			float force = 30.0f;
			AddBody(s)->Push(-(App->camera->Z.x * force), -(App->camera->Z.y * force), -(App->camera->Z.z * force));
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");

	// Remove from the world all collision bodies
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	for(p2List_item<btTypedConstraint*>* item = constraints.getFirst(); item; item = item->next)
	{
		world->removeConstraint(item->data);
		delete item->data;
	}
	
	constraints.clear();

	for(p2List_item<btDefaultMotionState*>* item = motions.getFirst(); item; item = item->next)
		delete item->data;

	motions.clear();

	for(p2List_item<btCollisionShape*>* item = shapes.getFirst(); item; item = item->next)
		delete item->data;

	shapes.clear();

	for(p2List_item<PhysBody3D*>* item = bodies.getFirst(); item; item = item->next)
		delete item->data;

	bodies.clear();

	for(p2List_item<PhysVehicle3D*>* item = vehicles.getFirst(); item; item = item->next)
		delete item->data;

	vehicles.clear();

	for (p2List_item<Cube*>* item = RenderCubes.getFirst(); item; item = item->next)
		delete item->data;

	RenderCubes.clear();

	delete vehicle_raycaster;
	delete world;

	return true;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Sphere& sphere, float mass)
{
	btCollisionShape* colShape = new btSphereShape(sphere.radius);
	shapes.add(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&sphere.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	world->addRigidBody(body);
	bodies.add(pbody);

	return pbody;
}


// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Cube& cube, float mass)
{
	btCollisionShape* colShape = new btBoxShape(btVector3(cube.size.x*0.5f, cube.size.y*0.5f, cube.size.z*0.5f));
	shapes.add(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&cube.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	world->addRigidBody(body);
	bodies.add(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody(const Cylinder& cylinder, float mass)
{
	btCollisionShape* colShape = new btCylinderShapeX(btVector3(cylinder.height*0.5f, cylinder.radius, 0.0f));
	shapes.add(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&cylinder.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	world->addRigidBody(body);
	bodies.add(pbody);

	return pbody;
}

// ---------------------------------------------------------
PhysVehicle3D* ModulePhysics3D::AddVehicle(const VehicleInfo& info)
{
	btCompoundShape* comShape = new btCompoundShape();
	shapes.add(comShape);

	btCollisionShape* base = new btBoxShape(btVector3(info.chassis_size.x*0.5f, info.chassis_size.y*0.5f, info.chassis_size.z*0.5f));
	shapes.add(base);
	
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y, info.chassis_offset.z));

	comShape->addChildShape(trans, base);


	btCollisionShape* Cube1 = new btBoxShape(btVector3(info.chassis_size.x*0.5f, info.chassis_size.y*0.5f, info.chassis_size.z*0.25f));
	shapes.add(Cube1);

	btTransform trans2;
	trans2.setIdentity();
	trans2.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y + (info.chassis_size.y), info.chassis_offset.z - (info.chassis_size.z*0.25f)));

	comShape->addChildShape(trans2, Cube1);

	btCollisionShape* Sfere = new btSphereShape(info.chassis_offset.y * 1.8f);
	shapes.add(Sfere);

	btTransform trans3;
	trans3.setIdentity();
	trans3.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y + (info.chassis_size.y), info.chassis_offset.z + (info.chassis_size.z*0.20f)));

	comShape->addChildShape(trans3, Sfere);

	btCollisionShape* Sfere2 = new btSphereShape(info.chassis_offset.y * 1.5f);
	shapes.add(Sfere2);

	btTransform trans4;
	trans4.setIdentity();
	trans4.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y + (info.chassis_size.y*2.0f), info.chassis_offset.z - (info.chassis_size.z*0.20f)));

	comShape->addChildShape(trans4, Sfere2);

	




	btTransform startTransform;
	startTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);
	comShape->calculateLocalInertia(info.mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(info.mass, myMotionState, comShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	body->setActivationState(DISABLE_DEACTIVATION);

	world->addRigidBody(body);

	btRaycastVehicle::btVehicleTuning tuning;
	tuning.m_frictionSlip = info.frictionSlip;
	tuning.m_maxSuspensionForce = info.maxSuspensionForce;
	tuning.m_maxSuspensionTravelCm = info.maxSuspensionTravelCm;
	tuning.m_suspensionCompression = info.suspensionCompression;
	tuning.m_suspensionDamping = info.suspensionDamping;
	tuning.m_suspensionStiffness = info.suspensionStiffness;

	btRaycastVehicle* vehicle = new btRaycastVehicle(tuning, body, vehicle_raycaster);

	vehicle->setCoordinateSystem(0, 1, 2);

	for(int i = 0; i < info.num_wheels; ++i)
	{
		btVector3 conn(info.wheels[i].connection.x, info.wheels[i].connection.y, info.wheels[i].connection.z);
		btVector3 dir(info.wheels[i].direction.x, info.wheels[i].direction.y, info.wheels[i].direction.z);
		btVector3 axis(info.wheels[i].axis.x, info.wheels[i].axis.y, info.wheels[i].axis.z);

		vehicle->addWheel(conn, dir, axis, info.wheels[i].suspensionRestLength, info.wheels[i].radius, tuning, info.wheels[i].front);
	}
	// ---------------------

	PhysVehicle3D* pvehicle = new PhysVehicle3D(body, vehicle, info);
	world->addVehicle(vehicle);
	vehicles.add(pvehicle);

	return pvehicle;
}

// ---------------------------------------------------------
void ModulePhysics3D::AddConstraintP2P(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec3& anchorA, const vec3& anchorB)
{
	btTypedConstraint* p2p = new btPoint2PointConstraint(
		*(bodyA.body), 
		*(bodyB.body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z), 
		btVector3(anchorB.x, anchorB.y, anchorB.z));
	world->addConstraint(p2p);
	constraints.add(p2p);
	p2p->setDbgDrawSize(2.0f);
}

void ModulePhysics3D::AddConstraintHinge(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec3& anchorA, const vec3& anchorB, const vec3& axisA, const vec3& axisB, bool disable_collision)
{
	btHingeConstraint* hinge = new btHingeConstraint(
		*(bodyA.body), 
		*(bodyB.body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z),
		btVector3(anchorB.x, anchorB.y, anchorB.z),
		btVector3(axisA.x, axisA.y, axisA.z), 
		btVector3(axisB.x, axisB.y, axisB.z));

	world->addConstraint(hinge, disable_collision);
	constraints.add(hinge);
	hinge->setDbgDrawSize(2.0f);
}

void ModulePhysics3D::AddStraightRoad(int large, const vec3& pos, int orientation){
	float x = pos.x;
	float z = pos.z;
	
	if (orientation == 0)
	{
		Cube* c  = new Cube();
		c->size = vec3(large, 2, 1);
		c->SetPos(x, 1, z);
		RenderCubes.add(c);
		PhysBody3D* road;
		road = App->physics->AddBody(*c, 0.0f);
		road->SetAsSensor(false);
		road->collision_listeners.add(this);
		

		Cube* c2 = new Cube();
		c2->size = vec3(large, 2, 1);
		c2->SetPos(x, 1, 30 + z);
		PhysBody3D* road2;
		road2 = App->physics->AddBody(*c2, 0.0f);
		road2->SetAsSensor(false);
		road2->collision_listeners.add(this);
		RenderCubes.add(c2);
	}

	else
	{

		Cube* c = new Cube();;
		c->size = vec3(1, 2, large);
		c->SetPos(x, 1, z);
		PhysBody3D* road;
		road = App->physics->AddBody(*c, 0.0f);
		road->SetAsSensor(false);
		road->collision_listeners.add(this);
		RenderCubes.add(c);
		Cube* c2 = new Cube();;
		c2->size = vec3(1, 2, large);
		c2->SetPos(30 + x, 1, z);
		PhysBody3D* road2;
		road2 = App->physics->AddBody(*c2, 0.0f);
		road2->SetAsSensor(false);
		road2->collision_listeners.add(this);
		RenderCubes.add(c2);
	}	
}
void ModulePhysics3D::AddCurve(const int large, const vec3& pos, int orientation){

	float x = pos.x;
	float z = pos.z;

	if (orientation == 0)
	{
		
		
		for (int i = 0; i < large; i++){

			Cube* c = new Cube();;
			c->size = vec3(1 , 2, 1);
			c->SetPos(x + i, 1, z + i );
			
			RenderCubes.add(c);
			PhysBody3D* road;
			road = App->physics->AddBody(*c, 0.0f);
			road->SetAsSensor(false);
			road->collision_listeners.add(this);
			
			Cube* c2 = new Cube();
			c2->size = vec3(1, 2, 1);
			c2->SetPos(x + 30 + i, 1, z  + i );
			PhysBody3D* road2;
			road2 = App->physics->AddBody(*c2, 0.0f);
			road2->SetAsSensor(false);
			road2->collision_listeners.add(this);
			RenderCubes.add(c2);
		}
	}

	else
	{
		for (int i = 0; i < large; i++){

			Cube* c = new Cube();
			c->size = vec3(1, 2, 1);
			c->SetPos(x - i, 1, z + i);
			PhysBody3D* road;
			road = App->physics->AddBody(*c, 0.0f);
			road->SetAsSensor(false);
			road->collision_listeners.add(this);
			RenderCubes.add(c);
			Cube* c2 = new Cube();
			c2->size = vec3(1, 2, 1);
			c2->SetPos(x + 30 - i, 1, z + i);
			PhysBody3D* road2;
			road2 = App->physics->AddBody(*c2, 0.0f);
			road2->SetAsSensor(false);
			road2->collision_listeners.add(this);
			RenderCubes.add(c2);
		}
	}
}

void ModulePhysics3D::AddCurveX(const int large, const vec3& pos, int orientation){

	float x = pos.x;
	float z = pos.z;

	if (orientation == 0)
	{


		for (int i = 0; i < large; i++){

			Cube* c = new Cube();
			c->size = vec3(1, 2, 1);
			c->SetPos(x + i, 1, z + i);

			RenderCubes.add(c);
			PhysBody3D* road;
			road = App->physics->AddBody(*c, 0.0f);
			road->SetAsSensor(false);
			road->collision_listeners.add(this);
			

			Cube* c2 = new Cube();
			c2->size = vec3(1, 2, 1);
			c2->SetPos(x + i, 1, z + 30 + i);
			PhysBody3D* road2;
			road2 = App->physics->AddBody(*c2, 0.0f);
			road2->SetAsSensor(false);
			road2->collision_listeners.add(this);
			RenderCubes.add(c2);
		}
	}

	else
	{
		for (int i = 0; i < large; i++){

			Cube* c = new Cube();
			c->size = vec3(1, 2, 1);
			c->SetPos(x + i, 1, z - i);
			PhysBody3D* road;
			road = App->physics->AddBody(*c, 0.0f);
			road->SetAsSensor(false);
			road->collision_listeners.add(this);
			RenderCubes.add(c);

			Cube* c2 = new Cube();
			c2->size = vec3(1, 2, 1);
			c2->SetPos(x + i, 1, z + 30 - i);
			PhysBody3D* road2;
			road2 = App->physics->AddBody(*c2, 0.0f);
			road2->SetAsSensor(false);
			road2->collision_listeners.add(this);
			RenderCubes.add(c2);
		}
	}
}

void ModulePhysics3D::AddCorner(const int large, const vec3& pos, int orientation){

	float x = pos.x;
	float z = pos.z;

	if (orientation == 0)
	{
		Cube* c = new Cube();
		c->size = vec3(1 , 2, large);
		c->SetPos(x , 1, z);
		PhysBody3D* road;
		road = App->physics->AddBody(*c, 0.0f);
		road->SetAsSensor(false);
		road->collision_listeners.add(this);
		RenderCubes.add(c);
		Cube* c2 = new Cube();
		c2->size = vec3(30, 2, 1);
		c2->SetPos(x - 15 , 1, z + (large/2));
		PhysBody3D* road2;
		road2 = App->physics->AddBody(*c2, 0.0f);
		road2->SetAsSensor(false);
		road2->collision_listeners.add(this);
		RenderCubes.add(c2);
	}

	else if (orientation == 1)
	{
		Cube* c = new Cube();
		c->size = vec3(large, 2, 1);
		c->SetPos(x , 1, z+30);
		PhysBody3D* road;
		road = App->physics->AddBody(*c, 0.0f);
		road->SetAsSensor(false);
		road->collision_listeners.add(this);
		RenderCubes.add(c);
		Cube* c2 = new Cube();
		c2->size = vec3(1, 2, 30);
		c2->SetPos(x - 15, 1, z  + 30- (large / 2));
		PhysBody3D* road2;
		road2 = App->physics->AddBody(*c2, 0.0f);
		road2->SetAsSensor(false);
		road2->collision_listeners.add(this);
		RenderCubes.add(c2);

	}
	else if ( orientation == 2 )
	{

		Cube* c = new Cube();
		c->size = vec3(1, 2,  large);
		c->SetPos(x, 1, z);
		PhysBody3D* road;
		road = App->physics->AddBody(*c, 0.0f);
		road->SetAsSensor(false);
		road->collision_listeners.add(this);
		RenderCubes.add(c);
		Cube* c2 = new Cube();
		c2->size = vec3(30, 2, 1);
		c2->SetPos(x + 15, 1, z - (large / 2));
		PhysBody3D* road2;
		road2 = App->physics->AddBody(*c2, 0.0f);
		road2->SetAsSensor(false);
		road2->collision_listeners.add(this);
		RenderCubes.add(c2);

	}

	else
	{
		Cube* c = new Cube();
		c->size = vec3(1, 2, large);
		c->SetPos(x, 1, z);
		PhysBody3D* road;
		road = App->physics->AddBody(*c, 0.0f);
		road->SetAsSensor(false);
		road->collision_listeners.add(this);
		RenderCubes.add(c);
		Cube* c2 = new Cube();
		c2->size = vec3(30, 2, 1);
		c2->SetPos(x - 15, 1, z - (large / 2));
		PhysBody3D* road2;
		road2 = App->physics->AddBody(*c2, 0.0f);
		road2->SetAsSensor(false);
		road2->collision_listeners.add(this);
		RenderCubes.add(c2);

	}
}

void ModulePhysics3D::RenderWalls(){

	p2List_item<Cube*>* item2 = RenderCubes.getFirst();
	while (item2)
	{

		item2->data->Render();
		item2 = item2->next;
	}

}



// =============================================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	line.origin.Set(from.getX(), from.getY(), from.getZ());
	line.destination.Set(to.getX(), to.getY(), to.getZ());
	line.color.Set(color.getX(), color.getY(), color.getZ());
	line.Render();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.transform.translate(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes) debugMode;
}

int	 DebugDrawer::getDebugMode() const
{
	return mode;
}
