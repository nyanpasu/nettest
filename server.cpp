/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
using namespace std;

// Constant and global declarations.

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

class Client
{
        public:
        TCPsocket socket;
        Player entity;
};

class Clients {
public:
        Clients ();

        void add(Player);
        void kill(int);
        void move(int, int, int);
        Client at(int);
        size_t total();
        TCPsocket socket(int);

private:
        vector<Client> connected_clients;
};

Client Clients::at(int element)
{
        return connected_clients[element];
}

size_t Clients::total()
{
        return connected_clients.size();
}

TCPsocket Clients::socket(int id)
{
        for (int i = 0; i < total(); i++)
        {
                if (connected_clients[i].id == id)
                {
                        return connected_clients[i].socket;
                }
        }
}

class Server {
        public:
        Server();
        void start(int);
        void accept_new_connections(Clients);
        void update_set(Clients);
        void send_data(TCPsocket, int);
        void send_data(TCPsocket, colourstruct);
        void get_data(Client, int *);
        void handle_client(Client);
        void handle_new_client(Clients, Client);
        void handle_leave(Clients, Client);
        void handle_move(Clients, Client);

private:
        TCPsocket server;
        SDLNet_SocketSet set;
};

void Server::start(int port)
{
        // All pre program routines... inits and so.
        SDLNet_Init();

        // Init variables...
        IPaddress ip, *remote_ip;
        TCPsocket server;
        set = SDLNet_AllocSocketSet(1);

        // Open the server's gates to the unknown!
        SDLNet_ResolveHost( &ip, NULL, port );

        server = SDLNet_TCP_Open(&ip);

        if(!server)
        {
                printf("Couldn't start server: %s\n",SDLNet_GetError());
                printf("Gee, that was anti climatic.\n");
                exit(-1);
        } else {
                printf("Server started! o/\n");
        }
}

void Server::accept_new_connections(Clients everyone)
{
                TCPsocket client;
                client = SDLNet_TCP_Accept(server);

                if (client)
                {
                        printf("A new client connected. %d currently connected.\n", everyone.total() + 1 );

                        IPaddress remote_ip = SDLNet_TCP_GetPeerAddress( client );
                        if ( !remote_ip )
                        {
                                printf( "For some arcane reason, someone connected... and then.. ;_;: %s\n", SDLNet_GetError() );
                                exit(-1);
                        }

                        Uint32 ipaddr = SDL_SwapBE32( remote_ip->host ); // Get the actual value of the address
                        // Run some rather clever bitwise operations to output each octet and port.
                        printf("Accepted a connection from %d.%d.%d.%d:%hu\n",
                                                                                       ipaddr>>24,
                                                                                      (ipaddr>>16) &0xff,
                                                                                      (ipaddr>>8)  &0xff,
                                                                                       ipaddr&0xff,
                                                                                       remote_ip->port);

                        // Update the arrays.
                        Client new_player;
                        new_player.socket              = client;
                        new_player.entity.id           = last_id++;
                        new_player.entity.colour.red   = rand() % 255;
                        new_player.entity.colour.green = rand() % 255;
                        new_player.entity.colour.blue  = rand() % 255;
                        // TODO proper x y
                        // new_player.x = 5;
                        // new_player.y = 4;
                        everyone.add(new_player);

                        // Update clients about the new client.
                        handle_join(new_player);
                        // update new client about other clients.
                        handle_new_client(new_player);

                        update_set(clients_set);
                }

}
void Server::update_set(Clients clients)
{
        SDLNet_FreeSocketSet (set);

        set = SDLNet_AllocSocketSet(clients.total());
        for ( int i = 0; i < clients.total(); i++ )
        {
                SDLNet_TCP_AddSocket(set, clients.at(i).socket);
        }

}

void Server::send_data(TCPsocket target, int data)
{
        SDLNet_TCP_Send( target, &data, sizeof(int));
}

void Server::send_data(TCPsocket target, colourstruct data)
{
        SDLNet_TCP_Send( target, &data, sizeof(colourstruct));
}

void Server::get_data(Client target, int *value)
{
        int response = SDLNet_TCP_Recv(target.socket, value, sizeof(int));
        if ( response <= 0 )
        {
                printf("Received error for client #%d. Disconnecting.\n", target.entity.id);
        }
}

void handle_client(Client target)
{
        // Only bother if the client had activity.
        if ( SDLNet_SocketReady(target.socket) )
        {
                int received_x, received_y;
                get_data(target, &received_x);
                get_data(target, &received_y);

                target.x = received_x;
                target.y = received_y;
        }

}

// Informs every player about the sudden addition of the new_player
void Server::handle_join(Clients everyone, Client new_target)
{
        // For every client connected
        for (int i = 0; i < everyone.total(); i++)
        {
                Client target = everyone.at(i);
                // Ignore if same client. Client doesn't need to know about himself.
                if (target.id != new_player.id)
                {
                        printf("Sending: SERVER_MESSAGE_JOIN: %d, id: %d, x: %d, y: %d\n",SERVER_MESSAGE_JOIN, new_player.id, new_player.x, new_player.y);
                        send_data(target.socket, SERVER_MESSAGE_JOIN);
                        send_data(target.socket, new_player.id);
                        send_data(target.socket, new_player.colour);
                        send_data(target.socket, new_player.x); send_data(target.socket, new_player.y);
                }
        }
}

// Update a newcomer about the existing clients.
void Server::handle_new_client(Clients everyone, Client new_player)
{
        // For every client connected
        for (int i = 0; i < everyone.total(); i++)
        {
                Client target = everyone.at(i);
                // Ignore if same client. Client doesn't need to know about himself.
                if (target.entity.id != new_player.entity.id)
                {
                        printf("Sending: SERVER_MESSAGE_JOIN: %d, id: %d, x: %d, y: %d\n",SERVER_MESSAGE_JOIN, new_player.entity.id, new_player.entity.x, new_player.entity.y);
                        send_data(new_player.socket, SERVER_MESSAGE_JOIN);
                        send_data(new_player.socket, target.entity.id);
                        send_data(new_player.socket, target.entity.colour);
                        send_data(new_player.socket, target.entity.x); send_data(new_player.socket, target.entity.y);
                }
        }
}

// Informs every player about the disconnection of a player.
void Server::handle_leave(Clients everyone, Client old_player)
{
        // For every client connected
        for (int i = 0; i < everyone.total(); i++)
        {
                Client target = everyone.at(i);
                // Ignore if same client. Client doesn't need to know about himself.
                if (target.entity.id != old_player.entity.id)
                {
                        send_data(target.socket, SERVER_MESSAGE_QUIT);
                        send_data(target.socket, old_player.entity.id);
                }
        }
}

// Sends positional update to every player.
void Server::handle_move(Clients everyone, Client current_player)
{
        for (int i = 0; i < everyone.total(); i++)
        {
                Client target = everyone.at(i);

                // Ignore if same player.
                if (target.entity.id == current_player.entity.id) continue;

                send_data(target.socket, SERVER_MESSAGE_MOVE);
                send_data(target.socket, current_player.entity.id);
                send_data(target.socket, current_player.entity.x);
                send_data(target.socket, current_player.entity.y);
        }
}


int main(int argc, const char *argv[])
{
        // For random numbers
        srand(time(NULL));

        Server serber;
        serber.start(8000);

        // Main program loop
        bool loop = true;
        while ( loop ) 
        {
                // Handle new client connections.
                serber.accept_new_connections();
                // Handle existing clients.
                // Check all sockets for apparent activity.
                int result = SDLNet_CheckSockets(clients_set, 100);

                // Handle client activity.
                for ( int i = 0; i < players.size() && result; i++ )
                {
                        handle_client(i);
                }

                // Send position changes for every player.
                for ( int i = 0; i < players.size(); i++ )
                {
                        handle_move(players[i]);
                }
        }

        return 0;
}
