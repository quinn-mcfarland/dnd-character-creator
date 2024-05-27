#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <Windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//Main menu button constants
const int MAIN_MENU_BUTTON_WIDTH = 400;
const int MAIN_MENU_BUTTON_HEIGHT = 100;
const int TOTAL_MAIN_MENU_BUTTONS = 2;

// Enumerations
enum keyPressSurfaces
{
	KEY_PRESS_SURFACE_ESC,
	KEY_PRESS_SURFACE_TOTAL
};

enum LButtonSprite
{
	BUTTON_SPRITE_MOUSE_OUT = 0,
	BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
	BUTTON_SPRITE_MOUSE_DOWN = 2,
	BUTTON_SPRITE_MOUSE_UP = 3,
	BUTTON_SPRITE_TOTAL = 4
};

// Function prototypes
bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture(string path);

// Texture wrapper class
class LTexture
{
public:
	LTexture();
	~LTexture();
	bool loadFromFile(string path);
	bool loadFromRenderedText(string textureText, SDL_Color textColor);
	void free();
	void setColor(Uint8 red, Uint8 green, Uint8 blue);
	void setBlendMode(SDL_BlendMode blending);
	void setAlpha(Uint8 alpha);
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	int getWidth();
	int getHeight();
private:
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
};

// Mouse button
class LButton
{
public:
	LButton();
	void setPosition(int x, int y);
	void handleEvent(SDL_Event* e);
	void render();
private:
	SDL_Point mPosition;
	LButtonSprite mCurrentSprite;
};

//Global variables
SDL_Window* window = NULL; // Window being rendered to
SDL_Surface* surface = NULL; // Surface contained by window
LTexture foregroundTexture; // Foreground texture
LTexture backgroundTexture; // Background texture
SDL_Renderer* renderer = NULL;
SDL_Texture* keyPressSurfaces[KEY_PRESS_SURFACE_TOTAL];
LTexture newCharacterTextTexture;
LTexture existingCharacterTextTexture;
TTF_Font* font = NULL;
SDL_Rect gSpriteClips[2];
LTexture gSpriteSheetTexture;
bool pauseMenuOpen = false;
bool processBegun = false;
int creationStage = 0;

int main(int argc, char* argv[])
{
	// Start up SDL and create window
	if (!init())
	{
		cout << "Failed to initialize!" << endl;
	}
	else
	{
		// Load media
		if (!loadMedia())
		{
			cout << "Failed to load media!" << endl;
		}
		else
		{
			// Main loop flag
			bool quit = false;
			
			// Event handler
			SDL_Event e;

			// While application is running
			while (!quit)
			{
				// Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					// User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}

					else if (e.type == SDL_KEYDOWN)
					{
						while (processBegun)
						{
							for (int i = 0; i < TOTAL_MAIN_MENU_BUTTONS; i++)
							{
								gButtons[i].render();
							}
							switch (e.key.keysym.sym)
							{
							case SDLK_ESCAPE:
								if (pauseMenuOpen)
								{
									pauseMenuOpen = false;
								}
								else
								{
									pauseMenuOpen = true;
								}
							}
						}
					}
				}

				// Clear screen
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(renderer);

				//render background texture to screen
				backgroundTexture.render(0, 0);

				// Render everything else
				if (!processBegun)
				{
					// Render "new character" box
					SDL_Rect newCharacter = { SCREEN_WIDTH / 3, (SCREEN_HEIGHT / 4) + 50 , 400, 100 };
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderFillRect(renderer, &newCharacter);

					// Render "new character" border
					SDL_Rect newCharacterBorder;
					for (int i = 0; i < 10; i++)
					{
						newCharacterBorder = { (SCREEN_WIDTH / 3) + i, ((SCREEN_HEIGHT / 4) + 50) + i, 400, 100};
						SDL_SetRenderDrawColor(renderer, 71, 132, 85, 255);
						SDL_RenderDrawRect(renderer, &newCharacterBorder);
					}

					// Render "new character" text
					newCharacterTextTexture.render((SCREEN_WIDTH / 3) + 32, (SCREEN_HEIGHT / 4) + 57);

					// Render New Character Button Sprite
					gSpriteSheetTexture.render(0, 0, &gSpriteClips[0]);

					// Render "existing character" box
					SDL_Rect existingCharacter = { SCREEN_WIDTH / 3, (SCREEN_HEIGHT / 4) + 200, 400, 100 };
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderFillRect(renderer, &existingCharacter);

					// Render "existing character" border
					SDL_Rect existingCharacterBorder;
					for (int i = 0; i < 10; i++)
					{
						existingCharacterBorder = { (SCREEN_WIDTH / 3) + i, ((SCREEN_HEIGHT / 4) + 200) + i, 400, 100 };
						SDL_SetRenderDrawColor(renderer, 71, 132, 85, 255);
						SDL_RenderDrawRect(renderer, &existingCharacterBorder);
					}

					// Render "existing character" text
					existingCharacterTextTexture.render((SCREEN_WIDTH / 3) + 22, (SCREEN_HEIGHT / 4) + 212);
				}
				if (pauseMenuOpen)
				{
					SDL_Rect fillRect = { (SCREEN_WIDTH / 3) + 100, SCREEN_HEIGHT / 4, 200, 300 };
					SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderFillRect(renderer, &fillRect);
				}

				// Update screen
				SDL_RenderPresent(renderer);
			}
		}
	}
	// Free resources and close SDL
	close();

	return 0;
}

bool init()
{
	// Initialization flag
	bool success = true;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "SDL could not initialize! SDL Error: %s" << SDL_GetError() << endl;
		success = false;
	}
	else
	{
		// Create window
		window = SDL_CreateWindow("Project2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			cout << "Window could not be created! SDL Error: %s" << SDL_GetError() << endl;
			success = false;
		}
		else
		{
			// Create renderer for window
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL)
			{
				cout << "Renderer could not be created! SDL Error: %s" << SDL_GetError() << endl;
				success = false;
			}
			else
			{
				// Intitialize renderer color
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				
				// Initialize JPG loading
				int imgFlags = IMG_INIT_JPG;
				if (!IMG_Init(imgFlags) & imgFlags)
				{
					cout << "SDL_image could not initialize! SDL_image Error: %s" << IMG_GetError() << endl;
					success = false;
				}

				// Initialize SDL_ttf
				if (TTF_Init() == -1)
				{
					cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	// Initialization flag
	bool success = true;

	// Load background texture
	if (!backgroundTexture.loadFromFile("assets/background_image.jpg"))
	{
		cout << "Failed to load background texture image!" << endl;
		success = false;
	}

	// Sprites
	// New Character Button
	gSpriteClips[0].x = SCREEN_WIDTH / 3;
	gSpriteClips[0].y = (SCREEN_HEIGHT / 4) + 50;
	gSpriteClips[0].w = 400;
	gSpriteClips[0].h = 100;

	// Font work
	// New Character text
	font = TTF_OpenFont("assets/dungeon_font.TTF", 75);
	if (font == NULL)
	{
		cout << "Failed to load Dungeon Font! SDL_ttf Error: " << TTF_GetError() << endl;
		success = false;
	}
	else
	{
		if (!processBegun)
		{
			SDL_Color textColor = { 255, 255, 255 };
			if (!newCharacterTextTexture.loadFromRenderedText("New Character", textColor))
			{
				cout << "Failed to render text texture at main menu!" << endl;
				success = false;
			}
		}
	}
	TTF_CloseFont(font);

	// Existing character text
	font = TTF_OpenFont("assets/dungeon_font.TTF", 67);
	if (font == NULL)
	{
		cout << "Failed to load dungeon font! SDL_ttf Error: " << TTF_GetError() << endl;
		success = false;
	}
	else
	{
		if (!processBegun)
		{
			SDL_Color textColor = { 255, 255, 255 };
			if (!existingCharacterTextTexture.loadFromRenderedText("Existing Character", textColor))
			{
				cout << "Failed to render text texture at main menu!" << endl;
				success = false;
			}
		}
	}


	return success;
}

void close()
{
	// Free loaded images
	backgroundTexture.free();

	// Free global font
	TTF_CloseFont(font);
	font = NULL;

	// Destroy window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	// Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

SDL_Surface* loadSurface(string path)
{
	// The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	// Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		cout << "Unable to load image ! SDL_image Error: " << path.c_str() << IMG_GetError() << endl;
	}
	else
	{
		// Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, surface->format, 0);
		if (optimizedSurface == NULL)
		{
			cout << "Unable to optimize image ! SDL_image Error: " << path.c_str() << IMG_GetError() << endl;
		}

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
}

SDL_Texture* loadTexture(string path)
{
	// The final texture
	SDL_Texture* newTexture = NULL;

	// Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		cout << "Unable to load image ! SDL_image Error: " << path.c_str() << IMG_GetError() << endl;
	}
	else
	{
		// Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			cout << "Unable to create texture from %s! SDL Error: %s" << path.c_str() << SDL_GetError() << endl;
		}

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}

LTexture::LTexture()
{
	// Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	// Deallocate
	free();
}

bool LTexture::loadFromFile(string path)
{
	// Get rid of preexisting texture
	free();
	// the final texture
	SDL_Texture* newTexture = NULL;
	// Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		cout << "Unable to load image! SDL_image Error: " << IMG_GetError() << endl;
	}
	else
	{
		// Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
		// Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			cout << "Unable to create texture from! SDL Error: " << SDL_GetError() << endl;
		}
		else
		{
			// Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	// return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{
	// Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(int x, int y)
{
	// Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	// Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopy(renderer, mTexture, clip, &renderQuad);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor)
{
	// Get rid of preexisting texture
	free();

	// Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		cout << "Unable to render text surface! SDL_ttf Error:" << TTF_GetError() << endl;
	}
	else
	{
		// Create Texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (mTexture == NULL)
		{
			cout << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << endl;
		}
		else
		{
			// Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		// Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	// Return success
	return mTexture != NULL;
}

LButton::LButton()
{
	mPosition.x = 0;
	mPosition.y = 0;
	mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
}

void LButton::setPosition(int x, int y)
{
	mPosition.x = x;
	mPosition.y = y;
}

void LButton::handleEvent(SDL_Event* e)
{
	// If mouse event happend
	if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP)
	{
		// get mouse position
		int x, y;
		SDL_GetMouseState(&x, &y);

		// Check if mouse is in button
		bool inside = true;

		// Mouse is left of the button
		if (x < mPosition.x)
		{
			inside = false;
		}
		//Mouse is right of the button
		else if (x > mPosition.x + BUTTON_WIDTH)
		{
			inside = false;
		}
		// Mouse is above the button
		else if (y < mPosition.y)
		{
			inside = false;
		}
		// Mouse is below the button
		else if (y > mPosition.y + BUTTON_HEIGHT)
		{
			inside = false;
		}

		// Mouse is outside button
		if (!inside)
		{
			mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
		}
		// Mouse is inside button
		else
		{
			// Set mouse over sprite
			switch (e->type)
			{
			case SDL_MOUSEMOTION:
				mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
				break;
			case SDL_MOUSEBUTTONUP:
				mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
				break;
			}
		}
	}
}

void LButton::render()
{
	gButtonSpriteSheetTexture.render(mPosition.x, mPosition.y, &gSpriteClips[mCurrentSprite]);
}