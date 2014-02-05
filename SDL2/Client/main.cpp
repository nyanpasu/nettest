#include <iostream>
#include <time.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "player.hpp"
#include "client.hpp"
#include "game.hpp"

// Constant and global declaration.

int main(int argc, const char *argv[]){
    srand(time(NULL));
    Game game;
    game.game_loop();
    return 0;
}
