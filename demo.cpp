//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string>
#include <bits/stdc++.h>
#include <cmath>

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int fps = 60;
//Texture wrapper 

string convertToString(char* a, int size) 
{ 
    int i; 
    string s = ""; 
    for (i = 0; i < size; i++) { 
        s = s + a[i]; 
    } 
    return s; 
} 

bool sortinrev(const pair<long long, string> &a,  
               const pair<long long, string> &b) 
{ 
       return (a.first > b.first); 
} 

class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );

		//Deallocates texture
		void free();

		//position
		int x, y;

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Font
TTF_Font *gFont = NULL;

//Scene texture
LTexture gBackground, gHero, gGround, menubg, coin, gRedOb[3], menuStart, menuHighscore, inst, menuExit, scorecard, highscoreBack, gameScore, highscore[3], gPromptTextTexture, gInputTextTexture, menuContinue;
SDL_Rect gHeroSprite[6];

//The music that will be played
Mix_Music *gMusic = NULL;


LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Blended( gFont, textureText.c_str(), textColor );
	if( textSurface == NULL )
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}
	else
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	
	//Return success
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

string enterText(char finalScoreString[20]){
	//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//Set text color as black
			SDL_Color textColor = { 0, 0, 0, 0xFF };

			//The current input text.
			std::string inputText = "Username";
			gInputTextTexture.loadFromRenderedText( inputText.c_str(), textColor );

			//Enable text input
			SDL_StartTextInput();

			//While application is running
			while( !quit )
			{
				//The rerender text flag
				bool renderText = false;

				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}
					//Special key input
					else if( e.type == SDL_KEYDOWN )
					{
						//Handle backspace
						if( e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0 )
						{
							//lop off character
							inputText.pop_back();
							renderText = true;
						}
						else if( e.key.keysym.sym == SDLK_RETURN){
							return inputText;
						}
					}
					//Special text input event
					else if( e.type == SDL_TEXTINPUT )
					{
							//Append character
							inputText += e.text.text;
							renderText = true;
					}
				}

				//Rerender text if needed
				if( renderText )
				{
					//Text is not empty
					if( inputText != "" )
					{
						//Render new text
						gInputTextTexture.loadFromRenderedText( inputText.c_str(), textColor );
					}
					//Text is empty
					else
					{
						//Render space texture
						gInputTextTexture.loadFromRenderedText( " ", textColor );
					}
				}

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				menubg.render(0, 0, NULL, NULL, NULL, SDL_FLIP_NONE);

				scorecard.render(( SCREEN_WIDTH - scorecard.getWidth() ) / 2, ( SCREEN_HEIGHT - scorecard.getHeight() ) / 2, NULL, NULL, NULL, SDL_FLIP_NONE);
				gameScore.loadFromRenderedText(finalScoreString, textColor);
				gameScore.render(( SCREEN_WIDTH - gameScore.getWidth() ) / 2, 280);

				//Render text textures
				gPromptTextTexture.render( ( SCREEN_WIDTH - gPromptTextTexture.getWidth() ) / 2, 340 );
				gInputTextTexture.render( ( SCREEN_WIDTH - gInputTextTexture.getWidth() ) / 2, 340 + gPromptTextTexture.getHeight() );



				//Update screen
				SDL_RenderPresent( gRenderer );
			}
			
			//Disable text input
			SDL_StopTextInput();

			close();
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "Tom's Stairway to Heaven", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
				if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }

                 //Initialize SDL_mixer
				if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
				{
					printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	gMusic = Mix_LoadMUS( "res/game.mp3" );
	if( gMusic == NULL )
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}
	//Load arrow
	if( !gBackground.loadFromFile( "res/bg.png" ) )
	{
		printf( "Failed to load bg texture!\n" );
		success = false;
	}

	if( !gHero.loadFromFile( "res/blueshirt.png" ) )
	{
		printf( "Failed to load hero texture!\n" );
		success = false;
	}
	else{
		gHeroSprite[ 0 ].x =   0;
		gHeroSprite[ 0 ].y =   0;
		gHeroSprite[ 0 ].w = 38;
		gHeroSprite[ 0 ].h = 60;

		
		gHeroSprite[ 1 ].x = 38;
		gHeroSprite[ 1 ].y =  0;
		gHeroSprite[ 1 ].w = 45;
		gHeroSprite[ 1 ].h = 60;
		
		
		gHeroSprite[ 2 ].x =   80;
		gHeroSprite[ 2 ].y = 0;
		gHeroSprite[ 2 ].w = 48;
		gHeroSprite[ 2 ].h = 60;
		
		gHeroSprite[ 3 ].x = 128;
		gHeroSprite[ 3 ].y = 0;
		gHeroSprite[ 3 ].w = 44;
		gHeroSprite[ 3 ].h = 60;

		gHeroSprite[ 4 ].x =  172;
		gHeroSprite[ 4 ].y =   0;
		gHeroSprite[ 4 ].w = 41;
		gHeroSprite[ 4 ].h = 60;

		gHeroSprite[ 5 ].x =  213;
		gHeroSprite[ 5 ].y =   0;
		gHeroSprite[ 5 ].w = 43;
		gHeroSprite[ 5 ].h = 60;
	}

	if( !coin.loadFromFile( "res/coin.png" ) )
	{
		printf( "Failed to load coin texture!\n" );
		success = false;
	}
	if( !menubg.loadFromFile( "res/menubg.png" ) )
	{
		printf( "Failed to load menubg texture!\n" );
		success = false;
	}
	for(int i=0; i<3; i++){
		if( !gRedOb[i].loadFromFile( "res/rock.png" ) )
		{
			printf( "Failed to load rock texture!\n" );
			success = false;
		}
	}

	if( !gGround.loadFromFile( "res/brick.png" ) )
	{
		printf( "Failed to load brick texture!\n" );
		success = false;
	}
	if( !inst.loadFromFile( "res/inst.png" ) )
	{
		printf( "Failed to load inst texture!\n" );
		success = false;
	}

	gFont = TTF_OpenFont( "prstart.ttf", 28 );
	if( gFont == NULL )
	{
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
		success = false;
	}
	//Render text
	SDL_Color textColor = { 0x00, 0x00, 0x00 };
	if( !menuStart.loadFromRenderedText( "START", textColor ) )
	{
		printf( "Failed to render text texture!\n" );
		success = false;
	}
	if( !menuHighscore.loadFromRenderedText( "HIGHSCORE", textColor ) )
	{
		printf( "Failed to render text texture!\n" );
		success = false;
	}
	if( !menuExit.loadFromRenderedText( "EXIT", textColor ) )
	{
		printf( "Failed to render text texture!\n" );
		success = false;
	}
	if( !highscoreBack.loadFromRenderedText( "BACK", textColor ) )
	{
		printf( "Failed to render text texture!\n" );
		success = false;
	}
	if( !scorecard.loadFromFile( "res/scorecard.png" ) )
	{
		printf( "Failed to load score texture!\n" );
		success = false;
	}
	if( !gPromptTextTexture.loadFromRenderedText( "Enter Usename:", textColor ) )
	{
		printf( "Failed to render prompt text!\n" );
		success = false;
	}
	if( !menuContinue.loadFromRenderedText( "CONTINUE", textColor ) )
	{
		printf( "Failed to render prompt text!\n" );
		success = false;
	}


	return success;
}

void close()
{
	//Free loaded images
	gBackground.free();
	gGround.free();
	coin.free();
	for(int i=0; i<3; i++)gRedOb[i].free();
	gHero.free();
	menuStart.free();
	menuHighscore.free();
	menuExit.free();
	scorecard.free();
	highscoreBack.free();
	gameScore.free();
	for(int i=0; i<3; i++)highscore[i].free();
	gPromptTextTexture.free();
	gInputTextTexture.free();
	menuContinue.free();
	menubg.free();
	inst.free();

	Mix_FreeMusic( gMusic );
	gMusic = NULL;

	//close Font
	TTF_CloseFont( gFont );
	gFont = NULL;

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;
	
	//Quit SDL subsystems
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}


int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
			//Score array
			vector< pair <long long int, string> > pairhs;
			long long scoreRank[4], score;
			float y_pos;
			int lastob=2;
			char scoreRankString[3][30], nameRank[3][15];
			FILE *highScoreInput;
			highScoreInput=fopen("highscore.txt", "r");

			for(int i=0;i<3;i++){
				fscanf(highScoreInput, "%lld %s", &scoreRank[i], nameRank[i]);
				pairhs.push_back(make_pair(scoreRank[i], convertToString(nameRank[i], sizeof(nameRank[i]))));
			}

			fclose(highScoreInput);

			Mix_PlayMusic( gMusic, -1 );
			bool newgame = false;
			FILE *stateInput;
			stateInput=fopen("state.txt", "r");

			fscanf(stateInput, "%f", &y_pos);
			fscanf(stateInput, "%d", &gRedOb[0].x);
			fscanf(stateInput, "%d", &gRedOb[1].x);
			fscanf(stateInput, "%d", &gRedOb[2].x);
			fscanf(stateInput, "%d", &coin.x);
			fscanf(stateInput, "%lld", &score);

			fclose(stateInput);
			
			int menu = 1;
			//Main loop flag
			SDL_Color textColor = { 0x00, 0x00, 0x00 };
			int bg_pos=0;
			hello:
			bool quit = false;
			if(newgame){
				score = 0;
				gRedOb[0].x = 400;
				lastob = 2;

				for(int i=1;i<3;i++)gRedOb[i].x=gRedOb[i-1].x+ 200 +(rand()%600);
				coin.x = 400; 
				y_pos = SCREEN_HEIGHT - gGround.getHeight() - gHero.getHeight();

			}
			for(int i=0;i<3;i++){
				if(gRedOb[i].x>gRedOb[lastob].x)lastob=i;
			}
			coin.y=300;
			char scoreString[20], finalScoreString[25];
			int sprite = 0;

			//Event handler
			SDL_Event e;

			//Angle of rotation
			double degrees = 0;
			int i =0;
			int up=0;
			int down=0;
			int ground =1;

			if(y_pos<=480)ground = 0 ;

			float x_pos = 100;
			int x_vel = 400;
			int y_velocity = 0;
			int gravity = 80;

			int mouse_x, mouse_y, buttons;

			//Flip type
			SDL_RendererFlip flipType = SDL_FLIP_NONE;

			int menuSelect = 0;

			//While application is running
			while( !quit )
			{
				
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}
					//Handle key press
					else if( e.type == SDL_KEYDOWN )
					{
						switch( e.key.keysym.sym )
						{
							
							case SDLK_m:
							//If there is no music playing
							if( Mix_PlayingMusic() == 0 )
							{
								//Play the music
								Mix_PlayMusic( gMusic, -1 );
							}
							//If music is being played
							else
							{
								//If the music is paused
								if( Mix_PausedMusic() == 1 )
								{
									//Resume the music
									Mix_ResumeMusic();
								}
								//If the music is playing
								else
								{
									//Pause the music
									Mix_PauseMusic();
								}
							}
							break;
						}
					}
					if(menu == 0){
						if( e.type == SDL_KEYDOWN )
						{
							switch( e.key.keysym.scancode)
							{
								case SDLK_UP:
								case SDL_SCANCODE_W:
									if(ground){
                						up=1;
                						down=0;
                					}
                					break;

                				case SDL_SCANCODE_Q:
                					menu=1;
                					break;
							}
						}
					}
					else if (menu==1)
					{
						if( e.type == SDL_KEYDOWN )
						{
							switch( e.key.keysym.scancode)
							{
								case SDL_SCANCODE_H:
									menu=4;
									break;

							}
						}
					}
					else if (menu==4)
					{
						if( e.type == SDL_KEYDOWN )
						{
							switch( e.key.keysym.scancode)
							{
								case SDL_SCANCODE_Q:
									menu=1;
									break;

							}
						}
					}
				}

				newgame = false;
				if(menu == 0)
				{
					SDL_Rect *currentsprite= &gHeroSprite[sprite/4];
					//mechanics
					if(ground && up && !down){
						y_velocity = 20;
						ground =0;
					}

					if(up){
						y_velocity -= gravity/fps;
						y_pos -= y_velocity;
						//degrees += 12;
					}
					if(down){
						y_velocity += gravity/fps;
						y_pos += y_velocity;
						//degrees += 12;
					}

					if(!ground && y_velocity<=0){
						down = 1;
						up =0;
					}
					//collision detectors

					if(y_pos >= SCREEN_HEIGHT - gGround.getHeight() - gHero.getHeight()){
						y_pos=SCREEN_HEIGHT - gGround.getHeight() - gHero.getHeight();
						up=0;
						down=0;
						ground = 1;
						y_velocity=0;
						degrees = 0;
					}

					for (int i = 0; i < 3; i++)
					{
						if((x_pos + 43) >= gRedOb[i].x && x_pos +43 <= gRedOb[i].x + gRedOb[i].getWidth() && (y_pos + 60) >= SCREEN_HEIGHT - gGround.getHeight() - gRedOb[i].getHeight())
						{
							sprintf(finalScoreString, "Score: %llu", score);
							newgame = true;
							pairhs.push_back(make_pair(score, enterText(finalScoreString)));

							sort(pairhs.begin(), pairhs.end(), sortinrev);

							pairhs.pop_back();
							menu = 1;
							score=0;

							goto hello;
						}
						if(x_pos>=gRedOb[i].x && x_pos <= gRedOb[i].x + gRedOb[i].getWidth() && (y_pos + 60) >= SCREEN_HEIGHT - gGround.getHeight() - gRedOb[i].getHeight())
						{
							sprintf(finalScoreString, "Score: %llu", score);
							newgame = true;
							pairhs.push_back(make_pair(score, enterText(finalScoreString)));

							sort(pairhs.begin(), pairhs.end(), sortinrev);

							pairhs.pop_back();
							menu = 1;
							score=0;

							goto hello;
						}
					}

					//coin obtacle
					if((x_pos + 43) >= coin.x && x_pos +43 <= coin.x + coin.getWidth() && (y_pos + 60) <= SCREEN_HEIGHT - gGround.getHeight() - coin.getHeight())
					{
						score+=100;
						coin.x += 800 + rand()%400;
					}
					if(x_pos>=coin.x && x_pos <= coin.x + coin.getWidth() && (y_pos + 60) <= SCREEN_HEIGHT - gGround.getHeight() - coin.getHeight())
					{
						score+=100;
						coin.x += 800 + rand()%400;
					}

					//osbtacle spawn
					coin.x -= x_vel/fps;

					if(coin.x <= -coin.getWidth())coin.x += 800 + rand()%400;

					for (int i = 0; i < 3; i++)
					{
						if(gRedOb[i].x <= -gRedOb[i].getWidth()){
							gRedOb[i].x = gRedOb[lastob].x + 250 + rand()%300;
							lastob=i;
						}
					}

					score++;
					sprintf(scoreString, "%lld", score);
					bg_pos -= x_vel/fps;
					if(bg_pos<=-gBackground.getWidth())bg_pos=0;


					//Clear screen
					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
					SDL_RenderClear( gRenderer );

					//Render arrow
					gBackground.render( bg_pos, 0, NULL, NULL, NULL, flipType );

					gBackground.render( gBackground.getWidth()+bg_pos, 0, NULL, NULL, NULL, flipType );

					for(int i = 0; i<= SCREEN_WIDTH; i += gGround.getWidth())
						gGround.render(i, SCREEN_HEIGHT - gGround.getHeight(), NULL, NULL, NULL, flipType);

					for(i=0; i<3; i++)
					gRedOb[i].render( gRedOb[i].x, SCREEN_HEIGHT - gGround.getHeight() - gRedOb[i].getHeight(), NULL, NULL, NULL, flipType );
					gHero.render(x_pos, y_pos, currentsprite, degrees, NULL, flipType);
					coin.render(coin.x, coin.y, NULL, degrees, NULL, flipType);
					gameScore.loadFromRenderedText(scoreString, textColor);
					gameScore.render(( SCREEN_WIDTH - gameScore.getWidth() ) / 2, 60);

					sprite++;
					if(sprite/4 >= 6)sprite=0;

					for(i=0;i<3;i++)gRedOb[i].x -= x_vel/fps;
				}
				//menu flag
				else if(menu==1){

					//get mouse coordinates
					buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

					if(mouse_x>=(SCREEN_WIDTH - menuContinue.getWidth() ) / 2 && mouse_x<= (SCREEN_WIDTH + menuContinue.getWidth() ) / 2 && mouse_y>= 180 && mouse_y<= 180+menuContinue.getHeight())
					{
						int menuSelect=0;
						if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)){
							menu=0;
							newgame=false;
							goto hello;
						}

					}

					//start
					if(mouse_x>=(SCREEN_WIDTH - menuStart.getWidth() ) / 2 && mouse_x<= (SCREEN_WIDTH + menuStart.getWidth() ) / 2 && mouse_y>= 240 && mouse_y<= 240+menuStart.getHeight())
					{
						int menuSelect=0;
						if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)){
							menu=0;
							newgame = true;
							goto hello;
						}

					}
					//highscore
					if(mouse_x>=(SCREEN_WIDTH - menuHighscore.getWidth() ) / 2 && mouse_x<= (SCREEN_WIDTH + menuHighscore.getWidth() ) / 2 && mouse_y>= 300 && mouse_y<= 300+menuHighscore.getHeight())
					{
						int menuSelect=1;
						if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)){
							menu=2;
						}
					}
					//exit
					if(mouse_x>=(SCREEN_WIDTH - menuExit.getWidth() ) / 2 && mouse_x<= (SCREEN_WIDTH + menuExit.getWidth() ) / 2 && mouse_y>= 360 && mouse_y<= 360+menuExit.getHeight())
					{
						int menuSelect=2;
						if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
							quit=true;
					}

					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
					SDL_RenderClear( gRenderer );
					//Render current frame
					menubg.render( 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE );

					SDL_Rect fillstart = {( SCREEN_WIDTH - menuStart.getWidth() ) / 2 - 50, 240, menuStart.getWidth() + 100, menuStart.getHeight()};
					SDL_Rect fillhighscore = {( SCREEN_WIDTH - menuHighscore.getWidth() ) / 2 - 50, 300, menuHighscore.getWidth() + 100, menuHighscore.getHeight()};
					SDL_Rect fillexit = {( SCREEN_WIDTH - menuExit.getWidth() ) / 2 - 50, 360, menuExit.getWidth() + 100, menuExit.getHeight()};
					SDL_Rect fillcontinue = {( SCREEN_WIDTH - menuContinue.getWidth() ) / 2 - 50, 180, menuContinue.getWidth() + 100, menuContinue.getHeight()};
					SDL_SetRenderDrawColor( gRenderer, 0xD9, 0xC5, 0x34, 0xFF );        


					if(score){
						SDL_RenderFillRect( gRenderer, &fillcontinue );
						menuContinue.render( ( SCREEN_WIDTH - menuContinue.getWidth() ) / 2, 180 );
					}
					SDL_RenderFillRect( gRenderer, &fillstart );
					SDL_RenderFillRect( gRenderer, &fillexit );
					SDL_RenderFillRect( gRenderer, &fillhighscore );
					menuStart.render( ( SCREEN_WIDTH - menuStart.getWidth() ) / 2, 240 );

					menuHighscore.render( ( SCREEN_WIDTH - menuHighscore.getWidth() ) / 2, 300 );

					menuExit.render( ( SCREEN_WIDTH - menuStart.getWidth() ) / 2, 360 );
				}
				//high score flag
				else if(menu==2)
				{
					//get mouse coordinates
					buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

					if(mouse_x>=(SCREEN_WIDTH - highscoreBack.getWidth() ) / 2 && mouse_x<= (SCREEN_WIDTH + highscoreBack.getWidth() ) / 2 && mouse_y>= 420 && mouse_y<= 420+highscoreBack.getHeight())
					{
						int menuSelect=0;
						if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)){
							menu=1;
							goto hello;
						}
					}
					SDL_RenderClear( gRenderer );
					SDL_Rect fill1 = {( SCREEN_WIDTH - 500 ) / 2, 200, 500, menuStart.getHeight()};
					SDL_Rect fill2 = {( SCREEN_WIDTH - 500 ) / 2, 260, 500, menuHighscore.getHeight()};
					SDL_Rect fill3 = {( SCREEN_WIDTH - 500) / 2, 320, 500, menuExit.getHeight()};
					SDL_Rect fillback = {( SCREEN_WIDTH - 200) / 2, 420, 200, menuContinue.getHeight()};
					SDL_SetRenderDrawColor( gRenderer, 0xD9, 0xC5, 0x34, 0xFF );
					//Render current frame
					menubg.render( 0, 0, NULL, NULL, NULL, SDL_FLIP_NONE );
					SDL_RenderFillRect( gRenderer, &fill1 );
					SDL_RenderFillRect( gRenderer, &fill2 );
					SDL_RenderFillRect( gRenderer, &fill3 );
					SDL_RenderFillRect( gRenderer, &fillback );

					for(int i=0; i<3; i++){
						sprintf(scoreRankString[i], "%d. %s = %lld", i+1, pairhs[i].second.c_str(), pairhs[i].first);
						highscore[i].loadFromRenderedText(scoreRankString[i], textColor);
						highscore[i].render(( SCREEN_WIDTH - highscore[i].getWidth() ) / 2, 200 + 60*i);
					}
					highscoreBack.render( ( SCREEN_WIDTH - highscoreBack.getWidth() ) / 2, 420 );
				}
				else if(menu==4)
				{
					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
					SDL_RenderClear( gRenderer );

					inst.render(0, 0, 0, 0, 0, flipType);
				}

				//Update screen
				SDL_RenderPresent( gRenderer );

				SDL_Delay(1000/fps);

				if(quit)
				{
					FILE *writescore;
					writescore=fopen("highscore.txt", "w");

					for(i=0;i<3;i++)fprintf(writescore, "%lld %s\n", pairhs[i].first, pairhs[i].second.c_str());
					fclose(writescore);

					FILE *stateInput;
					stateInput=fopen("state.txt", "w");
					if(!newgame){

						fprintf(stateInput, "%f\n", y_pos);
						fprintf(stateInput, "%d\n", gRedOb[0].x);
						fprintf(stateInput, "%d\n", gRedOb[1].x);
						fprintf(stateInput, "%d\n", gRedOb[2].x);
						fprintf(stateInput, "%d\n", coin.x);
						fprintf(stateInput, "%lld", score);

					}
					else fprintf(stateInput, "%f\n%d\n%d\n%d\n%d\n%lld", y_pos, gRedOb[0].x, gRedOb[1].x, gRedOb[2].x, coin.x, score);

					fclose(stateInput);

				}
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}