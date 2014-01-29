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
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

using namespace std;

// Constant and global declaration.
const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;
char WINDOW_TITLE[] = "Just liek maek gaem";

const int GRID_SIZE     = 9;
const int MAX_X         = 80;
const int MAX_Y         = 60;

const int TIMEOUT       = 100;

char server_ip[] = "124.82.1.157";
const int port = 8000;

const int SERVER_MESSAGE_JOIN = 1;
const int SERVER_MESSAGE_QUIT = 2;
const int SERVER_MESSAGE_MOVE = 3;

// Hurr globals are bad hurr durr
SDL_Window   * window   = NULL;
SDL_Renderer * renderer = NULL;

// Struct declarations.
struct colourstruct
{
        int red;
        int green;
        int blue;
};

struct player
{
        int id;
        colourstruct colour;
        int x;
        int y;
};
vector<player> players;

// Function declarations.
void drawRedGrid()
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

void drawGreenSquare( int x, int y )
{
        SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255 );
        SDL_Rect rect;

        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;
        rect.x = x * 10;
        rect.y = y * 10;

        SDL_RenderFillRect( renderer, &rect );
}

void drawBlueSquare( int x, int y )
{
        SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
        SDL_Rect rect;

        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;
        rect.x = x * 10;
        rect.y = y * 10;

        SDL_RenderFillRect( renderer, &rect );
}

void draw_square(int x, int y, colourstruct colour)
{
        SDL_Rect rect;
        rect.w = GRID_SIZE;
        rect.h = GRID_SIZE;
        rect.x = x * 10;
        rect.y = y * 10;

        SDL_SetRenderDrawColor( renderer, colour.red, colour.green, colour.blue, 255 );

        SDL_RenderFillRect( renderer, &rect );
}

void resetGrid()
{
        // Clear screen first.
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderClear( renderer );

        drawRedGrid();
}

void inititalize_game()
{
        if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        {
                printf("I-I couldn't even get SDL to start: %s\n", SDL_GetError() );
        }
        
        window = SDL_CreateWindow( WINDOW_TITLE,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          WINDOW_WIDTH, WINDOW_HEIGHT,
                          0 );

        renderer = SDL_CreateRenderer( window, -1, 0 );
}

void send_data(TCPsocket server, int data)
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

void update_server(TCPsocket server, player me)
{
        int x = me.x, y = me.y;
        printf("X: %d, Y: %d\n", x, y);
        send_data(server, x);
        send_data(server, y);
}

void get_data(TCPsocket server, int *value)
{
        int response = SDLNet_TCP_Recv( server, value, sizeof(int));
        if ( response <= 0 )
        {
                printf("Got an error while trying to get some data. Server must've crashed tragically.\n");
                printf("... So I'll crash too. Bye!\n");
                exit(1);
        }
}

void get_data(TCPsocket server, colourstruct *value)
{
        int response = SDLNet_TCP_Recv( server, value, sizeof(colourstruct));
        if ( response <= 0 )
        {
                printf("Got an error while trying to get some data. Server must've crashed tragically.\n");
                printf("... So I'll crash too. Bye!\n");
                exit(1);
        }
}

void get_data(TCPsocket server, player *value)
{
        int response = SDLNet_TCP_Recv( server, value, sizeof(player));
        if ( response <= 0 )
        {
                printf("Got an error while trying to get some data. Server must've crashed tragically.\n");
                printf("... So I'll crash too. Bye!\n");
                exit(1);
        }
}

int get_client_position(int id)
{
        for (int i = 0; i < players.size(); i++)
        {
                if (players[i].id == id)
                {
                        return i;
                }
        }
}

void update_positions(TCPsocket server)
{
        // Get the client id to update.
        int id;
        get_data(server, &id);
        // Then the coords.
        int x; int y;
        get_data(server, &x); get_data(server, &y);

        // Find and update player.
        printf("in update_pos; id: %d, x: %d, y: %d.\n",id, x, y);
        int element = get_client_position(id);
        players[element].x = x;
        players[element].y = y;
}

void add_client(TCPsocket server)
{
        player new_client;

        // TODO experiment and see if you can directly throw in newclient.id instead of id.
        // Likewise for the other elements.
        int id;
        get_data(server, &id);
        colourstruct colour;
        get_data(server, &colour);
        int x; int y;
        get_data(server, &x); get_data(server, &y);

        new_client.id     = id;
        new_client.x      = x;
        new_client.y      = y;
        new_client.colour = colour;

        players.push_back(new_client);
        printf("Someone new joined: ID: %d, at (%d, %d)\n", new_client.id, new_client.x, new_client.y);
        printf("New number of players: %d\n", players.size());
}

void remove_client(TCPsocket server)
{
        // Get the ID of the person who left and remove him for the players array.
        int id;
        get_data(server, &id);

        int pos = get_client_position(id);
        players.erase(players.begin() + pos);

        printf("Client %d left.\n", id);
}

void handle_server(TCPsocket server)
{
        // 1. Determine what the server is trying to send us.
        // It can be
        //  a. Client join             ( message = 1 )
        //  b. Client leave            ( message = 2 )
        //  c. Client position update. ( message = 3 )
        //
        // In each case, this client has to handle each situation gracefully.
        int message;
        get_data(server, &message);

        switch (message) {
                case SERVER_MESSAGE_JOIN:
                        printf("Received message: SERVER_MESSAGE_JOIN\n");
                        add_client(server);
                        printf("alright, add_client ran...\n");
                        printf("total: %d\n",players.size());
                        break;
                case SERVER_MESSAGE_QUIT:
                        printf("Received message:  SERVER_MESSAGE_QUIT\n");
                        remove_client(server);
                        break;
                case SERVER_MESSAGE_MOVE:
                        printf("Received message:  SERVER_MESSAGE_MOVE\n");
                        update_positions(server);
                        break;
                default:
                        printf("Woah, I just got a pretty strange response.\n");
                        printf("%d\n",message);
                        break;
        }
}

void draw_grid(player me)
{
        resetGrid();

        // Draw me
        drawGreenSquare(me.x, me.y);

        // Draw other clients.
        for ( int i = 0; i < players.size(); i++ )
        {
                player remote_player = players[i];
                draw_square(remote_player.x, remote_player.y, remote_player.colour);
        }
}

int main(int argc, const char *argv[])
{
        // For random numbers
        srand(time(NULL));

        inititalize_game();

        // Connect to the serber
        IPaddress ip; TCPsocket server;
        SDLNet_ResolveHost(&ip, server_ip, port);
        SDLNet_SocketSet server_monitor = SDLNet_AllocSocketSet(1);

        server = SDLNet_TCP_Open(&ip);
        if ( !server )
        {
                printf("Could't connect to server\nWill carry on nonetheless. Don't worry.\n");
        } else
        {
                printf("Connected to a remote server! Surprisingly.\n");
                // Add it to the socket set so that it can be monitored for activity.
                SDLNet_TCP_AddSocket( server_monitor, server );
        }

        // Timer init
        bool loop          = true;
        bool timeout       = false;
        Uint32 lastTick    = 0;
        Uint32 currentTick = SDL_GetTicks();

        // Initialize vector of players
        player me;
        me.x = 0;
        me.y = 0;

        // //
        // Program loop starts here.
        // //
        while (loop) 
        {
                // Handle the server... see what it's trying to tell the local client.
                if (server)
                {
                        // Only bother trying if the server actually sent something.
                        if (SDLNet_CheckSockets(server_monitor, 0)) handle_server(server);
                }

                draw_grid(me);

                // Render everything that's been drawn.
                SDL_RenderPresent( renderer );

                // Timeout handler.
                if ( timeout == true )
                {
                        currentTick = SDL_GetTicks();
                        if ( currentTick > lastTick + TIMEOUT )
                        {
                                timeout = false;
                        }
                }

                // Rest of the loop that's executed once every 300 milliseconds.
                if ( timeout == false )
                {
                        timeout = true;
                        lastTick = SDL_GetTicks();

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

                        // Update server.
                        if (server) update_server(server, me);
                }

        }
        return 0;
}
