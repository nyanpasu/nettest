#include <iostream>
#include <time.h>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "client.hpp"

// Constant and global declaration.
const int kWindowWidth = 800;
const int kWindowHeight = 600;
const char* kWindowTitle = "Just liek maek gaem";
const int kGridSize = 9;
const int kMaxX = 80;
const int kMaxY = 60;
const int kTimeOut = 100;
const char* kServerIP = "127.0.0.1";
const int kPort = 8000;

const int kServerMessageJoin = 1;
const int kServerMessageQuit = 2;
const int kServerMessageMove = 3;

/
// Function declarations.


int main(int argc, const char *argv[]){
    // For random numbers
    srand(time(NULL));
    inititalize_game();
    // Connect to the serber
    IPaddress ip; TCPsocket server;
    SDLNet_ResolveHost(&ip, kServerIP, kPort);
    SDLNet_SocketSet server_monitor = SDLNet_AllocSocketSet(1);
    server = SDLNet_TCP_Open(&ip);
    if (!server){
        std::cout << ("Could't connect to server\nWill carry on nonetheless. Don't worry.\n");
    } else {
        std::cout << ("Connected to a remote server! Surprisingly.") << std::endl;
        // Add it to the socket set so that it can be monitored for activity.
        SDLNet_TCP_AddSocket( server_monitor, server );
    }
    // Timer init
    loop = true;
    bool timeout = false;
    Uint32 lastTick = 0;
    Uint32 currentTick = SDL_GetTicks();
    // Initialize vector of players
    player me;
    me.x = 0;
    me.y = 0;
    // //
    // Program loop starts here.
    // //
    while (loop) {
        // Handle the server... see what it's trying to tell the local client.
        if (server){
            // Only bother trying if the server actually sent something.
            if (SDLNet_CheckSockets(server_monitor, 0)){ 
                handle_server(server);
            }
        }
        draw_grid(me);
        // Render everything that's been drawn.
        SDL_RenderPresent( renderer );
        // Timeout handler.
        if (timeout == true){
            currentTick = SDL_GetTicks();
            if (currentTick > lastTick + kTimeOut){
                timeout = false;
            }
        }
        // Rest of the loop that's executed once every 300 milliseconds.
        if (timeout == false){
            timeout = true;
            lastTick = SDL_GetTicks();
            // Handle keyboard and movement of self.
            SDL_PumpEvents();
            get_input(me);
            // Update server.
            if (server) 
                update_server(server, me);
        }
    }
        return 0;
}
