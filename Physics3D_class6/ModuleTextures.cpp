#include "Globals.h"
#include "Application.h"
#include "ModuleTextures.h"

#pragma comment( lib, "SDL_image/libx86/SDL2_image.lib" )

ModuleTextures::ModuleTextures(Application* app, bool start_enabled) : Module(app, start_enabled)
{}

// Destructor
ModuleTextures::~ModuleTextures()
{}

// Called before render is available
bool ModuleTextures::Init()
{
	LOG("Init Image library");
	bool ret = true;
	// load support for the PNG image format
	int flags = IMG_INIT_PNG;
	int init = IMG_Init(flags);

	if ((init & flags) != flags)
	{
		LOG("Could not initialize Image lib. IMG_Init: %s", IMG_GetError());
		//ret = false;
	}

	return ret;
}

// Called before the first frame
bool ModuleTextures::Start()
{
	LOG("start textures");
	bool ret = true;
	return ret;
}

// Called before quitting
bool ModuleTextures::CleanUp()
{
	LOG("Freeing textures and Image library");
	p2List_item<SDL_Texture*>* item;

	for (item = textures.getFirst(); item != NULL; item = item->next)
	{
		SDL_DestroyTexture(item->data);
	}

	textures.clear();
	IMG_Quit();
	return true;
}

// Load new texture from file path
SDL_Texture* const ModuleTextures::Load(const char* path)
{
	SDL_Texture* texture = NULL;
	//SDL_Surface* surface = IMG_Load_RW(App->fs->Load(path), 1);

	texture = SDL_CreateTexture(App->render2D->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 500, 482);

	return texture;
}

// Unload texture
bool ModuleTextures::UnLoad(SDL_Texture* texture)
{
	p2List_item<SDL_Texture*>* item;

	for (item = textures.getFirst(); item != NULL; item = item->next)
	{
		if (texture == item->data)
		{
			SDL_DestroyTexture(item->data);
			textures.del(item);
			return true;
		}
	}

	return false;
}

// Translate a surface into a texture
SDL_Texture* const ModuleTextures::LoadSurface(SDL_Surface* surface)
{
	SDL_Texture* texture = SDL_CreateTextureFromSurface(App->render2D->renderer, surface);

	if (texture == NULL)
	{
		LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
	}
	else
	{
		textures.add(texture);
	}

	return texture;
}

// Retrieve size of a texture
void ModuleTextures::GetSize(const SDL_Texture* texture, uint& width, uint& height) const
{
	SDL_QueryTexture((SDL_Texture*)texture, NULL, NULL, (int*)&width, (int*)&height);
}
