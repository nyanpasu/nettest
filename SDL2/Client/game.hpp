#ifndef GAME_H_
#define GAME_H_

class Game{
public: 
    Game(int, int);
public:
    void get_input(Player);
    void drawRedGrid();
    void drawGreenSquare(int, int);
    void drawBlueSquare(int, int);
    void draw_square(int, int, colourstruct);
    void resetGrid();
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool loop;
};

void Game::Game(int width, int height, const char* title){
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
        std::cout << "Couldn't start SDL: " << SDL_GetError() << std::endl;
    }    
    window = SDL_CreateWindow( 
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        0 
    );
    renderer = SDL_CreateRenderer( window, -1, 0 );
}

void Game::get_input(player& me){
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    // Process for each key...
    // A bit ugly, but formatted for one line per key.
    if (keyboard_state[SDL_SCANCODE_ESCAPE]){ 
        loop = false;
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]){ 
        if (me.x + 1< kMaxX ){ 
            me.x++;
        }
    }
    if (keyboard_state[SDL_SCANCODE_LEFT]){ 
        if (me.x > 0){ 
            me.x--;
        }
    }
    if (keyboard_state[SDL_SCANCODE_UP]){
        if (me.y > 0){ 
            me.y--;
        }
    }
    if (keyboard_state[SDL_SCANCODE_DOWN]){ 
        if (me.y + 1< kMaxY){ 
            me.y++;
        }
    }
}

void Game::drawRedGrid(){
    // Set up rect that will be drawn repeatedly.
    SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    // Draw a grid.
    for ( int x = 0; x < kMaxX; x++ ){
        for ( int y = 0; y < kMaxY; y++ ){
            rect.x = x * 10;
            rect.y = y * 10;
            SDL_RenderFillRect( renderer, &rect );
        }
    }
}

void Game::drawGreenSquare(int x, int y){
    SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255 );
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    rect.x = x * 10;
    rect.y = y * 10;
    SDL_RenderFillRect( renderer, &rect );
}

void Game::drawBlueSquare(int x, int y){
    SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    rect.x = x * 10;
    rect.y = y * 10;
    SDL_RenderFillRect( renderer, &rect );
}

void Game::draw_square(int x, int y, colourstruct colour){
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    rect.x = x * 10;
    rect.y = y * 10;
    SDL_SetRenderDrawColor( renderer, colour.red, colour.green, colour.blue, 255 );
    SDL_RenderFillRect( renderer, &rect );
}

void Game::resetGrid(){
    // Clear screen first.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    drawRedGrid();
}

#endif