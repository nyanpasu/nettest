#ifndef CLIENT_H_
#define CLIENT_H_

const uint kServerMessageJoin = 1;
const uint kServerMessageQuit = 2;
const uint kServerMessageMove = 3;
const uint kMaxBuffer = 1024;
const uint kPort = 8000;
const std::string kServerIP = "127.0.0.1";

class Client
{
public:
    Client();
public:
    void send_data(TCPsocket, int);
    void update_server(TCPsocket, Player);
    void get_data(TCPsocket, int&);
    void get_data(TCPsocket, SDL_Color&);
    void get_data(TCPsocket, Player&);
    int get_client_position(int);
    void update_positions(TCPsocket);
    void add_client(TCPsocket);
    void remove_client(TCPsocket);
    void handle_server(TCPsocket);
public:
    std::vector <Player> players;
    IPaddress ip;
    TCPsocket server;
    SDLNet_SocketSet server_monitor;
};

Client::Client()
{
    SDLNet_ResolveHost(&ip, kServerIP.c_str(), kPort);
    server_monitor = SDLNet_AllocSocketSet(1);
    server = SDLNet_TCP_Open(&ip);
    if (!server)
    {
        std::cout <<
            "Could't connect to server\nWill carry on nonetheless. Don't worry."
        << std::endl;
    }
    else
    {
        std::cout << "Connected to a remote server! Surprisingly." << std::endl;
        // Add it to the socket set so that it can be monitored for activity.
        SDLNet_TCP_AddSocket( server_monitor, server );
    }
}

void Client::send_data(TCPsocket server, int data)
{
    uint response = SDLNet_TCP_Send(server, &data, sizeof(int));
    if (response < sizeof(int))
    {
        std::cout << "Failed to send data" << std::endl;
        std::cout << "Disconnecting to save face." << std::endl;
        SDLNet_TCP_Close(server);
        std::cout << "Disconnected." << std::endl;
        SDL_Quit();
        SDLNet_Quit();
        exit(0);
    }
}

void Client::update_server(TCPsocket server, Player me)
{
    int x = me.x, y = me.y;
    std::cout << "X:" << x << ", Y: " << y << std::endl;
    send_data(server, x);
    send_data(server, y);
}

void Client::get_data(TCPsocket server, int& value)
{
    int response = SDLNet_TCP_Recv(server, &value, kMaxBuffer);
    if (response <= 0)
    {
        std::cout <<
            "Got an error while trying to get some data." <<
            "\nServer must've crashed tragically."
        << std::endl;
        std::cout << "... So I'll crash too. Bye!" << std::endl;
        exit(-1);
    }
}

void Client::get_data(TCPsocket server, SDL_Color& value)
{
    int response = SDLNet_TCP_Recv(server, &value, kMaxBuffer);
    if (response <= 0)
    {
        std::cout <<
            "Got an error while trying to get some data." <<
            "Server must've crashed tragically." <<
        std::endl;
        std::cout << "... So I'll crash too. Bye!" << std::endl;
        exit(-1);
    }
}

void Client::get_data(TCPsocket server, Player& value)
{
    int response = SDLNet_TCP_Recv( server, &value, kMaxBuffer);
    if (response <= 0)
    {
        std::cout << ("Got an error while trying to get some data. Server must've crashed tragically.\n");
        std::cout << ("... So I'll crash too. Bye!") << std::endl;
        exit(-1);
    }
}

int Client::get_client_position(int id)
{
    for (uint i = 0; i < players.size(); i++)
    {
        if (players[i].id == id)
        {
            return i;
        }
    }
}

void Client::update_positions(TCPsocket server)
{
    // Get the client id to update.
    int id;
    get_data(server, id);
    // Then the coords.
    int x;
    int y;
    get_data(server, x);
    get_data(server, y);
    // Find and update player.
    std::cout << 
        "in update_pos; id: " << id << ", x: " << x << ", y: " << y << "." << 
    std::endl;
    int element = get_client_position(id);
    players[element].x = x;
    players[element].y = y;
}

void Client::add_client(TCPsocket server)
{
    Player new_client;
    // TODO experiment and see if you can directly throw in newclient.id instead of id.
    // Likewise for the other elements.
    int id;
    get_data(server, id);
    SDL_Color color;
    get_data(server, color);
    int x;
    int y;
    get_data(server, x);
    get_data(server, y);
    new_client.id = id;
    new_client.x = x;
    new_client.y = y;
    new_client.color = color;
    players.push_back(new_client);
    std::cout << 
        "Someone new joined: ID: " << new_client.id << 
        ", at (" << new_client.x << ", "<< new_client.y << ")" 
    << std::endl;
    std::cout << "New number of players: " << players.size() << std::endl;
}

void Client::remove_client(TCPsocket server)
{
    // Get the ID of the person who left and remove him for the players array.
    int id;
    get_data(server, id);
    int pos = get_client_position(id);
    players.erase(players.begin() + pos);
    std::cout << "Client " << id << " left." << std::endl;
}

void Client::handle_server(TCPsocket server)
{
    // 1. Determine what the server is trying to send us.
    // It can be
    //  a. Client join             ( message = 1 )
    //  b. Client leave            ( message = 2 )
    //  c. Client position update. ( message = 3 )
    //
    // In each case, this client has to handle each situation gracefully.
    int message;
    get_data(server, message);
    switch (message) 
    {
        case kServerMessageJoin:
            std::cout << "Received message: SERVER_MESSAGE_JOIN" << std::endl;
            add_client(server);
            std::cout << "alright, add_client ran..." << std::endl;
            std::cout << "total: " << players.size() << std::endl;
            break;
        case kServerMessageQuit:
            std::cout << "Received message: SERVER_MESSAGE_QUIT" << std::endl;
            remove_client(server);
            break;
        case kServerMessageMove:
            std::cout << "Received message: SERVER_MESSAGE_MOVE" << std::endl;
            update_positions(server);
            break;
        default:
            std::cout << "Woah, I just got a pretty strange response." << std::endl;
            std::cout << message << std::endl;
            break;
    }
}

#endif
