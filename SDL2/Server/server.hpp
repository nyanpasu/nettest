#ifndef SERVER_H_
#define SERVER_H_

const int kPort = 8000;
const int kMaxBuffer = 1024;
const int kServerMessageJoin = 1;
const int kServerMessageQuit = 2;
const int kServerMessageMove = 3;

class Server
{
public:
    Server();
public:
    void server_loop();
    void update_set(SDLNet_SocketSet);
    void send_data(TCPsocket, int);
    void send_data(TCPsocket, SDL_Color);
    void get_data(player, int*);
    void handle_client(int);
    void handle_join(player);
    void handle_new_client(player);
    void handle_leave(player);
    void handle_move(player);
    void add_client();
public:
    int result;
    int success;
    int number_of_clients;
    int last_id;    
    bool loop;
    IPaddress ip;
    IPaddress* remote_ip;
    TCPsocket server;
    SDLNet_SocketSet clients_set;
    std::vector <player> players;
    std::vector <TCPsocket> clients;
};

Server::Server()
{
    // All pre program routines... inits and so.
    SDLNet_Init();
    // Init variables...
    clients_set = SDLNet_AllocSocketSet(1);
    // Init the vector of clients set up and so.
    last_id = 0;    
    // For random numbers
    srand(time(NULL));
    // Open the server's gates to the unknown!
    SDLNet_ResolveHost( &ip, 0, kPort);
    server = SDLNet_TCP_Open(&ip);
    if(!server)
    {
        std::cout << 
            "Couldn't start server: " << 
            SDLNet_GetError() << 
        std::endl;
        std::cout << "Gee, that was anti climatic." << std::endl;
        exit(-1);
    } 
    else 
    {
        std::cout << "Server started! o/" << std::endl;
    }
    // Main program loop
    loop = true; 
}

void Server::server_loop()
{
    while (loop)
    {
        // Handle new client connections.
        add_client();
        // Handle existing clients.
        // Check all sockets for apparent activity.
        result = SDLNet_CheckSockets(clients_set, 100);
        // Handle client activity.
        for (uint i = 0; i < players.size() && result; i++)
        {
            handle_client(i);
            std::cout << 
                "main: " << players[i].x << "," << players[i].y << 
            std::endl;
        }
        // Send position changes for every player.
        for (uint i = 0; i < players.size(); i++)
        {
            std::cout << 
                "x: "<< players[i].x << ", y: " << players[i].y <<
            std::endl;
            handle_move(players[i]);
        }
    }
}
void Server::add_client()
{
    TCPsocket client;
    client = SDLNet_TCP_Accept(server);
    if (client){
        std::cout << 
            "A new client connected. " << clients.size() + 1 << 
            "currently connected." << 
        std::endl;
        remote_ip = SDLNet_TCP_GetPeerAddress(client);
        if (!remote_ip)
        {
            std::cout <<
                "For some arcane reason, someone connected... and then.. ;_;: " << 
                SDLNet_GetError() <<
            std::endl;
            exit(-1);
        }
        Uint32 ipaddr = SDL_SwapBE32( remote_ip->host ); // Get the actual value of the address
        // Run some rather clever bitwise operations to output each octet and port.
        std::cout << 
            "Accepted a connection from " << 
            (ipaddr>> 24) << "." <<  
            ((ipaddr>> 16)& 0xff) << "." <<
            ((ipaddr>> 8)& 0xff) << "." <<            
            (ipaddr& 0xff)  << ":" << 
            (remote_ip -> port) <<
        std::endl;
        // Update the arrays.
        player new_player;
        new_player.socket = client;
        new_player.id = last_id++;
        new_player.colour.r  = rand() % 255;
        new_player.colour.g = rand() % 255;
        new_player.colour.b  = rand() % 255;
        // TODO proper x y
        // new_player.x = 5;
        // new_player.y = 4;
        players.push_back(new_player);
        // Update clients about the new client.
        handle_join(new_player);
        // update new client about other clients.
        handle_new_client(new_player);
        update_set(clients_set);
    }
}

void Server::update_set(SDLNet_SocketSet set)
{
    SDLNet_FreeSocketSet (set);
    set = SDLNet_AllocSocketSet(players.size());
    for (uint i = 0; i < players.size(); i++)
    {
        SDLNet_TCP_AddSocket(set, players[i].socket);
    }
}

void Server::send_data(TCPsocket target, int data)
{
    SDLNet_TCP_Send( target, &data, kMaxBuffer);
}

void Server::send_data(TCPsocket target, SDL_Color data)
{
    SDLNet_TCP_Send( target, &data, kMaxBuffer);
}

void Server::get_data(player target, int *value)
{
    int response = SDLNet_TCP_Recv(target.socket, value, kMaxBuffer);
    if ( response <= 0 )
    {
        std::cout << 
            "Received error for client #" << target.id << ". Disconnecting."<<
        std::endl;
        exit(-1);
    }
}

void Server::handle_client(int element)
{
    player &target = players[element];
    // Only bother if the client had activity.
    if (SDLNet_SocketReady(target.socket))
    {
        int new_x, new_y;
        get_data(target, &new_x);
        get_data(target, &new_y);
        target.x = new_x;
        target.y = new_y;
    }
    std::cout << 
        "handle_client: " <<  target.x << ", " << target.y <<
    std::endl;
}

// Informs every player about the sudden addition of the new_player
void Server::handle_join(player new_player)
{
    // For every client connected
    for (uint i = 0; i < players.size(); i++)
    {
        player target = players[i];
        // Ignore if same client. Client doesn't need to know about himself.
        if (target.id != new_player.id)
        {
            std::cout << 
                "Sending: SERVER MESSAGE JOIN: " << kServerMessageJoin << 
                ", id: " << new_player.id << 
                ", x: " << new_player.x << 
                ", y: " << new_player.y << 
            std::endl; 
            send_data(target.socket, kServerMessageJoin);
            send_data(target.socket, new_player.id);
            send_data(target.socket, new_player.colour);
            send_data(target.socket, new_player.x); send_data(target.socket, new_player.y);
        }
    }
}

void Server::handle_new_client(player new_player)
{
    // For every client connected
    for (uint i = 0; i < players.size(); i++)
    {
        player target = players[i];
        // Ignore if same client. Client doesn't need to know about himself.
        if (target.id != new_player.id)
        {
            std::cout << 
                "Sending: SERVER MESSAGE JOIN: " << kServerMessageJoin << 
                ", id: " << new_player.id << 
                ", x: " << new_player.x << 
                ", y: " << new_player.y << 
            std::endl;
            send_data(new_player.socket, kServerMessageJoin);
            send_data(new_player.socket, target.id);
            send_data(new_player.socket, target.colour);
            send_data(new_player.socket, target.x); send_data(new_player.socket, target.y);
        }
    }
}

// Informs every player about the disconnection of a player.
void Server::handle_leave(player old_player)
{
    // For every client connected
    for (uint i = 0; i < players.size(); i++)
    {
        player target = players[i];
        // Ignore if same client. Client doesn't need to know about himself.
        if (target.id != old_player.id)
        {
            send_data(target.socket, kServerMessageJoin);
            send_data(target.socket, old_player.id);
            send_data(target.socket, old_player.colour);
            send_data(target.socket, old_player.x); send_data(target.socket, old_player.y);
        }
    }
}

// Sends positional update to every player.
void Server::handle_move(player current_player)
{
    for (uint i = 0; i < players.size(); i++)
    {
        player target = players[i];
        // Ignore if same player.
        if (target.id == current_player.id)
        { 
            continue;
        }
        send_data(target.socket, kServerMessageMove);
        send_data(target.socket, current_player.id);
        send_data(target.socket, current_player.x);
        send_data(target.socket, current_player.y);
    }
}

#endif