#ifndef PLAYER_H_
#define PLAYER_H_

class Player{
public:
    Player();
    Player(int, int);
public:
    int id;
    SDL_Color color;
    int x;
    int y;
};

Player::Player(){
    x = 0;
    y = 0;
}

Player::Player(int x, int y){
    this->x = x;
    this->y = y;
}

#endif