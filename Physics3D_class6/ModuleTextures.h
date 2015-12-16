#ifndef __MODULETEXTURES_H__
#define __MODULETEXTURES_H__

#include "Module.h"
#include "p2List.h"

#include "SDL_image/include/SDL_image.h"

struct SDL_Texture;
struct SDL_Surface;

class ModuleTextures : public Module
{
public:

	ModuleTextures(Application* app, bool start_enabled);

	// Destructor
	virtual ~ModuleTextures();

	bool Init();
	bool Start();


	// Called before quitting
	bool CleanUp();

	// Load Texture
	SDL_Texture* const	Load(const char* path);
	bool				UnLoad(SDL_Texture* texture);
	SDL_Texture* const	LoadSurface(SDL_Surface* surface);
	void				GetSize(const SDL_Texture* texture, uint& width, uint& height) const;

public:

	p2List<SDL_Texture*>	textures;
};


#endif // __MODULETEXTURES_H__