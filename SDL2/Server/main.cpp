#include <iostream>
#include <time.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "util.hpp"
#include "server.hpp"

int main(int argc, const char *argv[]){
    Server server;
    server.server_loop();
    return 0;
}
