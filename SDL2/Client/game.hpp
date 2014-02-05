#ifndef GAME_H_
#define GAME_H_

const uint kGridSize = 9;
const int kMaxX = 80;
const int kMaxY = 60;
const uint kWindowWidth = 800;
const uint kWindowHeight = 600;
const uint kTimeOut = 100;
const std::string kWindowTitle = "Just liek maek gaem";

class Game
{
public: 
    Game();
public:
    void game_loop();
    void get_input(Player&);
    void drawRedGrid();
    void drawGreenSquare(int, int);
    void drawBlueSquare(int, int);
    void draw_square(int, int, SDL_Color);
    void resetGrid();
    void draw_grid(Player, std::vector <Player>);
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    bool loop;
    bool timeout;
    Client client;
    Player me;
    Uint32 lastTick;
    Uint32 currentTick;
};

Game::Game()
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cout << "Couldn't start SDL: " << SDL_GetError() << std::endl;
    }    
    window = SDL_CreateWindow( 
        kWindowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kWindowWidth, kWindowHeight,
        0 
    );
    renderer = SDL_CreateRenderer( window, -1, 0 );
    loop = true;
    timeout = false;
    lastTick = 0;
    currentTick = SDL_GetTicks();
}

void Game::game_loop()
{
    while (loop) 
    {
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                loop = false;
            }
        }
        // Handle the server... see what it's trying to tell the local client.
        if (client.server)
        {
            // Only bother trying if the server actually sent something.
            if (SDLNet_CheckSockets(client.server_monitor, 0))
            {
                client.handle_server(client.server);
            }
        }
        draw_grid(me, client.players);
        // Render everything that's been drawn.
        SDL_RenderPresent(renderer);
        // Timeout handler.
        if (timeout == true)
        {
            currentTick = SDL_GetTicks();
            if (currentTick > lastTick + kTimeOut){
                timeout = false;
            }
        }
        // Rest of the loop that's executed once every 300 milliseconds.
        if (timeout == false)
        {
            timeout = true;
            lastTick = SDL_GetTicks();
            // Handle keyboard and movement of self.
            SDL_PumpEvents();
            get_input(me);
            // Update server.
            if (client.server){
                client.update_server(client.server, me);
            }
        }
    }
}

void Game::get_input(Player& me)
{
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    if (keyboard_state[SDLK_ESCAPE])
    { 
        loop = false;
    }
    if (keyboard_state[SDLK_RIGHT])
    { 
        if (me.x + 1< kMaxX )
        { 
            me.x++;
        }
    }
    if (keyboard_state[SDLK_LEFT])
    { 
        if (me.x > 0)
        { 
            me.x--;
        }
    }
    if (keyboard_state[SDLK_UP])
    {
        if (me.y > 0)
        { 
            me.y--;
        }
    }
    if (keyboard_state[SDLK_DOWN])
    { 
        if (me.y + 1 < kMaxY)
        { 
            me.y++;
        }
    }
}

void Game::drawRedGrid()
{
    // Set up rect that will be drawn repeatedly.
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    // Draw a grid.
    for ( uint x = 0; x < kMaxX; x++ )
    {
        for ( uint y = 0; y < kMaxY; y++ )
        {
            rect.x = x * 10;
            rect.y = y * 10;
            SDL_RenderFillRect( renderer, &rect );
        }
    }
}

void Game::drawGreenSquare(int x, int y)
{
    SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255 );
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    rect.x = x * 10;
    rect.y = y * 10;
    SDL_RenderFillRect(renderer, &rect);
}

void Game::drawBlueSquare(int x, int y)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    rect.x = x * 10;
    rect.y = y * 10;
    SDL_RenderFillRect(renderer, &rect);
}

void Game::draw_square(int x, int y, SDL_Color color)
{
    SDL_Rect rect;
    rect.w = kGridSize;
    rect.h = kGridSize;
    rect.x = x * 10;
    rect.y = y * 10;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255 );
    SDL_RenderFillRect(renderer, &rect);
}

void Game::resetGrid()
{
    // Clear screen first.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    drawRedGrid();
}

void Game::draw_grid(Player me, std::vector <Player> players)
{
    resetGrid();
    // Draw me
    drawGreenSquare(me.x, me.y);
    // Draw other clients.
    for (uint i = 0; i < players.size(); i++ )
    {
        Player remote_player = players[i];
        draw_square(remote_player.x, remote_player.y, remote_player.color);
    }
}

#endif