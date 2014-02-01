/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

using namespace std;

// Constant and global declaration.
const int WINDOW_WIDTH        = 800;
const int WINDOW_HEIGHT       = 600;
char WINDOW_TITLE[]           = "Just liek maek gaem";

const int GRID_SIZE           = 9;
const int MAX_X               = 80;
const int MAX_Y               = 60;

const int SERVER_MESSAGE_JOIN = 1;
const int SERVER_MESSAGE_QUIT = 2;
const int SERVER_MESSAGE_MOVE = 3;

// Struct declarations.
struct colourstruct
{
        int red;
        int green;
        int blue;
};


// Class declarations.
class Player
{
public:
        int id;
        colourstruct colour;
        int x;
        int y;
        
};


class Players
{
        vector<Player> everyone;
public:
        void add(Player);
        void kill(int);
        void move(int, int, int);
        Player at(size_t);
        size_t total();


private:
        /* data */
};

void Players::add(Player new_player)
{
        everyone.push_back(new_player);
}

void Players::kill(int id)
{
        for (int i = 0; i < total(); i++)
        {
                if (everyone[i].id == id)
                {
                        everyone.erase(everyone.begin()+i);
                }
        }
}

void Players::move(int id, int new_x, int new_y)
{
        for (int i = 0; i < total(); i++)
        {
                if (everyone[i].id == id)
                {
                        everyone[i].x = new_x;
                        everyone[i].y = new_y;
                }
        }
}

Player Players::at(size_t element)
{
        return everyone[element];
}

size_t Players::total()
{
        return everyone.size();
}

class Game
{
        SDL_Window   * window   = NULL;
        SDL_Renderer * renderer = NULL;

public:

        void initSDL();
        void process(Players, Player);
        void render();
        void drawRedGrid();
        void drawGreenSquare(int,int);
        void drawBlueSquare(int, int);
        void draw_square(int, int, colourstruct);
        void draw_players(Players);
        void resetGrid();

private:
        /* data */
};

void Game::initSDL()
{
        if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        {
                printf("I-I couldn't even get SDL to start: %s\n", SDL_GetError() );
        }
        
        this->window = SDL_CreateWindow( WINDOW_TITLE,
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         WINDOW_WIDTH, WINDOW_HEIGHT,
                                         0
                                         );

        this->renderer = SDL_CreateRenderer( window, -1, 0 );
}

void Game::process(Players everyone, Player me)
{
        resetGrid();
        draw_players(everyone);
        drawGreenSquare(me.x, me.y);
}

void Game::render()
{
        SDL_RenderPresent(renderer);
}

void Game::drawRedGrid()
{
        // Set up rect that will be drawn repeatedly.
        SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255 );
        SDL_Rect rect;
        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;

        // Draw a grid.
        for ( int x = 0; x < MAX_X; x++ )
        {
                for ( int y = 0; y < MAX_Y; y++ )
                {
                        rect.x = x * 10;
                        rect.y = y * 10;

                        SDL_RenderFillRect( renderer, &rect );
                }
        }
}

void Game::drawGreenSquare( int x, int y )
{
        SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255 );
        SDL_Rect rect;

        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;
        rect.x = x * 10;
        rect.y = y * 10;

        SDL_RenderFillRect( renderer, &rect );
}

void Game::drawBlueSquare( int x, int y )
{
        SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
        SDL_Rect rect;

        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;
        rect.x = x * 10;
        rect.y = y * 10;

        SDL_RenderFillRect( renderer, &rect );
}

void Game::draw_square(int x, int y, colourstruct colour)
{
        SDL_Rect rect;
        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;
        rect.x = x * 10;
        rect.y = y * 10;

        SDL_SetRenderDrawColor( renderer, colour.red, colour.green, colour.blue, 255 );

        SDL_RenderFillRect( renderer, &rect );
}

void Game::draw_players(Players everyone)
{
        resetGrid();

        // Draw other clients.
        for ( int i = 0; i < everyone.total(); i++ )
        {
                Player player = everyone.at(i);
                draw_square(player.x, player.y, player.colour);
        }
}

void Game::resetGrid()
{
        // Clear screen first.
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderClear( renderer );

        drawRedGrid();
}


class Server
{
        string ip;
        int port;
        TCPsocket server;
        SDLNet_SocketSet server_monitor;

public:
        Server(char[], int);

        void connect();
        void get_data(Player*);
        void get_data(colourstruct*);
        void get_data(int*);
        void send_data(int);
        void update(Player);
        void update_positions(Players);
        void add_client(Players);
        void remove_client(Players);
        bool activity();
        void handle_server(Players);

private:
};

Server::Server(char server_ip[], int server_port)
{
        ip = server_ip;
        port = server_port;
}

void Server::connect()
{
        IPaddress resolved_ip;

        SDLNet_ResolveHost(&resolved_ip, ip.c_str(), port);
        server_monitor = SDLNet_AllocSocketSet(1);

        server = SDLNet_TCP_Open(&resolved_ip);
        if ( !server )
        {
                printf("BAKAYAUROU! I COULDN'T CONNECT! YOU HANDED ME A... N-no don't cry, please ;_;\n");
                printf("k, exiting, sod this shit\n");
                exit(-1);
        } else
        {
                printf("Connected to a remote server! Surprisingly.\n");

                // Add it to the socket set so that it can be monitored for activity.
                SDLNet_TCP_AddSocket( server_monitor, server );
        }
}

void Server::send_data(int data)
{
        int response = SDLNet_TCP_Send(server, &data, sizeof(int));
        if ( response<sizeof(int) )
        {
                printf("Auu: Failed to send data!\n");
                printf("Disconnecting to save face.\n");
                SDLNet_TCP_Close( server );
                printf("Disconnected. Commiting senpuku.\n");

                SDL_Quit();
                SDLNet_Quit();
                exit(0);
        }
}

void Server::get_data(int *value)
{
        int response = SDLNet_TCP_Recv( server, value, sizeof(int));
        if ( response <= 0 )
        {
                printf("Got an error while trying to get some data. Server must've crashed tragically.\n");
                printf("... So I'll crash too. Bye!\n");
                exit(1);
        }
}

void Server::get_data(Player *value)
{
        int response = SDLNet_TCP_Recv( server, value, sizeof(Player));
        if ( response <= 0 )
        {
                printf("Got an error while trying to get some data. Server must've crashed tragically.\n");
                printf("... So I'll crash too. Bye!\n");
                exit(1);
        }
}

void Server::get_data(colourstruct *value)
{
        int response = SDLNet_TCP_Recv( server, value, sizeof(colourstruct));
        if ( response <= 0 )
        {
                printf("Got an error while trying to get some data. Server must've crashed tragically.\n");
                printf("... So I'll crash too. Bye!\n");
                exit(1);
        }
}

void Server::update(Player me)
{
        int x = me.x, y = me.y;
        printf("X: %d, Y: %d\n", x, y);
        send_data(x);
        send_data(y);
}

void Server::update_positions(Players everyone)
{
        // Get the client id to update.
        int id;
        get_data(&id);
        // Then the coords.
        int x; int y;
        get_data(&x); get_data(&y);

        // Find and update player.
        printf("Received positional update; id: %d, x: %d, y: %d.\n",id, x, y);
        everyone.move(id, x, y);
}

void Server::add_client(Players everyone)
{
        Player new_client;

        // TODO experiment and see if you can directly throw in newclient.id instead of id.
        // Likewise for the other elements.
        int id;
        get_data(&id);
        colourstruct colour;
        get_data(&colour);
        int x; int y;
        get_data(&x); get_data(&y);

        new_client.id     = id;
        new_client.x      = x;
        new_client.y      = y;
        new_client.colour = colour;

        everyone.add(new_client);
        printf("Someone new joined: ID: %d, at (%d, %d)\n", new_client.id, new_client.x, new_client.y);
        printf("New number of players: %d\n", everyone.total());
}

void Server::remove_client(Players everyone)
{
        // Get the ID of the person who left and remove him for the players array.
        int id;
        get_data(&id);

        everyone.kill(id);

        printf("Client %d left.\n", id);
}

bool Server::activity()
{
        if (SDLNet_CheckSockets(server_monitor, 0)) return true;
        return false;
}

void Server::handle_server(Players everyone)
{
        if (!activity()) return;
        // 1. Determine what the server is trying to send us.
        // It can be
        //  a. Client join             ( message = 1 )
        //  b. Client leave            ( message = 2 )
        //  c. Client position update. ( message = 3 )
        //
        // In each case, this client has to handle each situation gracefully.
        int message;
        get_data(&message);

        switch (message) {
                case SERVER_MESSAGE_JOIN:
                        printf("Received message: SERVER_MESSAGE_JOIN\n");
                        add_client(everyone);
                        printf("alright, add_client ran...\n");
                        printf("total: %d\n",everyone.total());
                        break;
                case SERVER_MESSAGE_QUIT:
                        printf("Received message:  SERVER_MESSAGE_QUIT\n");
                        remove_client(everyone);
                        break;
                case SERVER_MESSAGE_MOVE:
                        printf("Received message:  SERVER_MESSAGE_MOVE\n");
                        update_positions(everyone);
                        break;
                default:
                        printf("Woah, invalid resonse: %d\n", message);
                        break;
        }
}


class TimeoutHandler
{
        Uint32 lastTick;
        Uint32 currentTick;

        const static int TIMEOUT       = 100;
public:
        TimeoutHandler ();
        bool timeout();

private:
        void updateTick();
        void resetTick();
};

TimeoutHandler::TimeoutHandler()
{
        updateTick();
        resetTick();
}

bool TimeoutHandler::timeout()
{
        updateTick();
        cout << "Here: Current:" << currentTick << ", Last:" << lastTick << endl;

        if (currentTick > (lastTick + TIMEOUT))
        {
                resetTick();
                return true;
        } else
        {
                return false;
        }
}

void TimeoutHandler::updateTick()
{
        currentTick = SDL_GetTicks();
}

void TimeoutHandler::resetTick()
{
        lastTick = currentTick;
}


// Main
int main(int argc, const char *argv[])
{
        srand(time(NULL)); // RANDUM LEL

        Game game;
        game.initSDL();

        Server server("127.0.0.1", 8000);
        server.connect();

        // Timer init
        TimeoutHandler timer;
        bool loop = true;

        // Initialize player self.
        Players everyone;
        Player me;
        me.x = 0;
        me.y = 0;

        // Main program loop
        while (loop) 
        {
                // Yea, Calculate and draw shit.
                game.process(everyone, me);
                game.render();

                // Rest of the loop that's executed once every so often.
                if (timer.timeout())
                {
                        // Handle keyboard and movement of self.
                        SDL_PumpEvents();
                        const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

                        // Process for each key...
                        // A bit ugly, but formatted for one line per key.
                        if ( keyboard_state[SDL_SCANCODE_SPACE] ) loop = false;
                        if ( keyboard_state[SDL_SCANCODE_RIGHT] ) if ( me.x + 1< MAX_X ) me.x++;
                        if ( keyboard_state[SDL_SCANCODE_LEFT]  ) if ( me.x > 0        ) me.x--;
                        if ( keyboard_state[SDL_SCANCODE_UP]    ) if ( me.y > 0        ) me.y--;
                        if ( keyboard_state[SDL_SCANCODE_DOWN]  ) if ( me.y + 1< MAX_Y ) me.y++;

                        server.handle_server(everyone);
                }
        }
        return 0;
}
