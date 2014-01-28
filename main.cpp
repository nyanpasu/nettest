/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 9;
const int MAX_X = 80;
const int MAX_Y = 60;
const int TIMEOUT = 100;

const char server_ip[] = "124.82.1.157";
const int port = 8000;

// Hurr globals are bad hurr durr
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

struct client
{
        int x = 0;
        int y = 0;
};

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

void resetGrid()
{
        // Set grid to black first
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderClear( renderer );

        drawRedGrid();

}

int main(int argc, const char *argv[])
{
        // Initialize SDL and the window/renderer.
        if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        {
                printf("%s\n", SDL_GetError() );
        }
        

        window = SDL_CreateWindow( "Just liek maek gaem",
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          WINDOW_WIDTH, WINDOW_HEIGHT,
                          0 );

        renderer = SDL_CreateRenderer( window, -1, 0 );

        // Connect to the serber
        IPaddress ip;
        TCPsocket server;
        SDLNet_ResolveHost(&ip, server_ip, port);
        server = SDLNet_TCP_Open(&ip);
        SDLNet_SocketSet server_monitor = SDLNet_AllocSocketSet(1);
        if ( !server )
        {
                printf("Could't connect to server\nWill carry on nonetheless. Don't worry.\n");
        } else {
                printf("Connected! Surprisingly.\n");
                SDLNet_TCP_AddSocket( server_monitor, server );
        }

        // User process stuff
        // Events and the likes.
        bool loop = true;
        bool timeout = false;
        Uint32 lastTick = 0;
        Uint32 currentTick = SDL_GetTicks();

        client player;
        vector <client> players;
        // //
        // End of all the init stuff
        // //

        // Program loop
        while ( loop ) 
        {
                // Do all the drawing and handle the server
                resetGrid();
                drawGreenSquare( player.x, player.y );
                // Draw other clients.
                for ( int i = 0; i < players.size(); i++ )
                {
                        client remote_player = players[i];
                        drawBlueSquare( remote_player.x, remote_player.y );
                }
                // Update positions of other clients.
                if ( server )
                {
                        int response = 1; // Used for various function calls.
                        while ( response == 1 ) 
                        {
                                response = SDLNet_CheckSockets( server_monitor, 0 );

                                if ( response == 1 )
                                {
                                        // socket activity happened.
                                        // 1. Get number of clients.
                                        int number_of_clients;
                                        response = SDLNet_TCP_Recv( server,&number_of_clients,sizeof(int));
                                        if ( response <= 0 )
                                        {
                                                printf("Eeeeh? W-what happened? I got an error from SDLNet_TCP_Recv.\n");
                                        }
                                        // 2. Draw position of each client.
                                        for ( int i = 0; i < number_of_clients; i++ )
                                        {
                                                client remote_player;
                                                response = SDLNet_TCP_Recv( server,&remote_player,sizeof(client));
                                                if ( response <= 0 )
                                                {
                                                        printf("Eeeeh? W-what happened? I got an error from SDLNet_TCP_Recv.\n");
                                                }
                                                if ( i + 1 > players.size() )
                                                {
                                                        players.push_back(remote_player);
                                                }
                                                players[i] = remote_player;
                                        }
                                }
                        }
                }
                // FINALLY render everything seen.
                SDL_RenderPresent( renderer );

                // Timeout handler. Will only process one key event
                // every 300 milliseconds.
                if ( timeout == true )
                {
                        currentTick = SDL_GetTicks();
                        if ( currentTick > lastTick + TIMEOUT )
                        {
                                timeout = false;
                        }
                }

                if ( timeout == false )
                {
                        timeout = true;
                        lastTick = SDL_GetTicks();

                        // Handle keyboard
                        SDL_PumpEvents();
                        const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

                        if ( keyboard_state[SDL_SCANCODE_SPACE] )
                        {
                                loop = false;
                        }
                        if ( keyboard_state[SDL_SCANCODE_RIGHT] )
                        {
                                if ( player.x + 1< MAX_X )
                                {
                                        player.x++;
                                }
                        }
                        if ( keyboard_state[SDL_SCANCODE_LEFT] )
                        {
                                if ( player.x > 0 )
                                {
                                        player.x--;
                                }
                        }
                        if ( keyboard_state[SDL_SCANCODE_UP] )
                        {
                                if ( player.y > 0 )
                                {
                                        player.y--;
                                }
                        }
                        if ( keyboard_state[SDL_SCANCODE_DOWN] )
                        {
                                if ( player.y + 1< MAX_Y )
                                {
                                        player.y++;
                                }
                        }

                        // Send position to server.
                        if ( server )
                        {
                                int response = SDLNet_TCP_Send(server,&player,sizeof(client));
                                if ( response<sizeof(client) )
                                {
                                        printf("Auu: Failed to send position.\n");
                                        printf("Disconnecting to save face.\n");
                                        SDLNet_TCP_Close( server );
                                        printf("Disconnected. Commiting senpuku.\n");

                                        SDL_Quit();
                                        SDLNet_Quit();
                                        return 1;
                                }
                        }

                }

        }
        
        return 0;
}
