#ifndef UTIL_H_
#define UTIL_H

struct player
{
    int id;
    SDL_Color colour;
    int x;
    int y;
    TCPsocket socket;
};

#endif