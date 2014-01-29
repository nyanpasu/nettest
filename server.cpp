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

const int PORT = 8000;

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

struct player
{
        int id;
        colourstruct colour;
        int x;
        int y;
        TCPsocket socket;
};
vector <player> players;

void update_set(SDLNet_SocketSet set)
{
        SDLNet_FreeSocketSet (set);

        set = SDLNet_AllocSocketSet(players.size());
        for ( int i = 0; i < players.size(); i++ )
        {
                SDLNet_TCP_AddSocket(set, players[i].socket);
        }

}

void send_data(TCPsocket target, int data)
{
        SDLNet_TCP_Send( target, &data, sizeof(int));
}

void send_data(TCPsocket target, colourstruct data)
{
        SDLNet_TCP_Send( target, &data, sizeof(colourstruct));
}

void get_data(player target, int *value)
{
        int response = SDLNet_TCP_Recv(target.socket, value, sizeof(int));
        if ( response <= 0 )
        {
                        printf("Received error for client #%d. Disconnecting.\n", target.id);
                        exit(1);
        }
}

void handle_client(int element)
{
        player &target = players[element];

        // Only bother if the client had activity.
        if ( SDLNet_SocketReady(target.socket) )
        {
                int new_x, new_y;
                get_data(target, &new_x);
                get_data(target, &new_y);

                target.x = new_x;
                target.y = new_y;
        }

        printf("handle_client: %d, %d\n", target.x, target.y);
}

// Informs every player about the sudden addition of the new_player
void handle_join(player new_player)
{
        // For every client connected
        for (int i = 0; i < players.size(); i++)
        {
                player target = players[i];
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

void handle_new_client(player new_player)
{
        // For every client connected
        for (int i = 0; i < players.size(); i++)
        {
                player target = players[i];
                // Ignore if same client. Client doesn't need to know about himself.
                if (target.id != new_player.id)
                {
                        printf("Sending: SERVER_MESSAGE_JOIN: %d, id: %d, x: %d, y: %d\n",SERVER_MESSAGE_JOIN, new_player.id, new_player.x, new_player.y);
                        send_data(new_player.socket, SERVER_MESSAGE_JOIN);
                        send_data(new_player.socket, target.id);
                        send_data(new_player.socket, target.colour);
                        send_data(new_player.socket, target.x); send_data(new_player.socket, target.y);
                }
        }
}

// Informs every player about the disconnection of a player.
void handle_leave(player old_player)
{
        // For every client connected
        for (int i = 0; i < players.size(); i++)
        {
                player target = players[i];
                // Ignore if same client. Client doesn't need to know about himself.
                if (target.id != old_player.id)
                {
                        send_data(target.socket, SERVER_MESSAGE_JOIN);
                        send_data(target.socket, old_player.id);
                        send_data(target.socket, old_player.colour);
                        send_data(target.socket, old_player.x); send_data(target.socket, old_player.y);
                }
        }
}

// Sends positional update to every player.
void handle_move(player current_player)
{
        for (int i = 0; i < players.size(); i++)
        {
                player target = players[i];

                // Ignore if same player.
                if (target.id == current_player.id) continue;

                send_data(target.socket, SERVER_MESSAGE_MOVE);
                send_data(target.socket, current_player.id);
                send_data(target.socket, current_player.x);
                send_data(target.socket, current_player.y);
        }
}

int main(int argc, const char *argv[])
{
        // All pre program routines... inits and so.
        SDLNet_Init();

        // Init variables...
        IPaddress ip, *remote_ip;
        TCPsocket server;
        SDLNet_SocketSet clients_set = SDLNet_AllocSocketSet(1);

        // Init the vector of clients set up and so.
        vector <TCPsocket> clients;
        int last_id = 0;
        
        // For random numbers
        srand(time(NULL));

        // Open the server's gates to the unknown!
        SDLNet_ResolveHost( &ip, NULL, PORT );

        server = SDLNet_TCP_Open(&ip);
        if(!server)
        {
                printf("Couldn't start server: %s\n",SDLNet_GetError());
                printf("Gee, that was anti climatic.\n");
                exit(4);
        } else {
                printf("Server started! o/\n");
        }

        // Main program loop
        bool loop = true;
        while ( loop ) 
        {
                // Handle new client connections.
                TCPsocket client;
                client = SDLNet_TCP_Accept(server);

                if (client)
                {
                        printf("A new client connected. %d currently connected.\n", clients.size() + 1 );

                        remote_ip = SDLNet_TCP_GetPeerAddress( client );
                        if ( !remote_ip )
                        {
                                printf( "For some arcane reason, someone connected... and then.. ;_;: %s\n", SDLNet_GetError() );
                                exit(1);
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
                        player new_player;
                        new_player.socket = client;
                        new_player.id = last_id++;
                        new_player.colour.red   = rand() % 255;
                        new_player.colour.green = rand() % 255;
                        new_player.colour.blue  = rand() % 255;
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

                // Handle existing clients.
                // Check all sockets for apparent activity.
                int result = SDLNet_CheckSockets(clients_set, 100);

                // Handle client activity.
                for ( int i = 0; i < players.size() && result; i++ )
                {
                        handle_client(i);
                        printf("main: %d, %d\n", players[i].x, players[i].y);
                }

                // Send position changes for every player.
                for ( int i = 0; i < players.size(); i++ )
                {
                        printf("x: %d, y: %d\n", players[i].x, players[i].y);
                        handle_move(players[i]);
                }
        }

        return 0;
}
