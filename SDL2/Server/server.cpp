#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

const int kPort = 8000;
const int kMaxClients = 16;

struct coord{
    int x;
    int y;
};

struct player{
    TCPsocket socket;
    coord position;
};

void update_monitor(SDLNet_SocketSet monitor, int number_of_clients, std::vector <TCPsocket> clients){
    SDLNet_FreeSocketSet (monitor);
    monitor = SDLNet_AllocSocketSet(number_of_clients);
    for (int i = 0; i < number_of_clients; i++){
        SDLNet_TCP_AddSocket( monitor, clients[i]);
    }
}

int main(int argc, const char *argv[]){   
    SDLNet_Init();
    IPaddress ip, *remote_ip;
    TCPsocket server;
    SDLNet_SocketSet moving_clients, new_clients;
    moving_clients = SDLNet_AllocSocketSet(0);
    std::vector <player> players;
    std::vector <TCPsocket> clients;
    int number_of_clients = 0;
    SDLNet_ResolveHost( &ip, NULL, kPort );
    /* open the server socket */
    server = SDLNet_TCP_Open(&ip);
    if(!server){
        std::cout << "Couldn't start server: " << SDLNet_GetError() << std::endl;
        exit(-1);
    } else {
        std::cout << ("Server started! o/\n") << std::endl;
    }
    bool loop = true;
    while (loop){
        TCPsocket client;
        client = SDLNet_TCP_Accept(server);
        // Handle client connections.
        if (client){
            // Client connected
            number_of_clients++;
            std::cout << "A new client connected." << number_of_clients << "currently connected." << std::endl;
            remote_ip = SDLNet_TCP_GetPeerAddress( client );
            if (!remote_ip){
                std::cout << "Couldn't GetPeerAddress: " << SDLNet_GetError() << std::endl;
            }
            Uint32 ipaddr = SDL_SwapBE32( remote_ip->host ); // Get the actual value of the address
            // Run some bitwise operations to output each octet and port.
            std::cout << 
                "Accepted a connection from " << (ipaddr>>24) <<
                "." << ((ipaddr>>16) &0xff) << 
                "." << ((ipaddr>>8)  &0xff) << 
                "." << (ipaddr&0xff) << 
                ":" << (remote_ip->port) << std::endl;
            player newbie;
            newbie.socket = client;
            clients.push_back(client);
            players.push_back(newbie);
            update_monitor( moving_clients, number_of_clients, clients );
        }
        // Handle existing clients.
        int result = SDLNet_CheckSockets(moving_clients, 100);
        // Get positional changes.
        for (int i = 0; i < number_of_clients && result; i++){
            TCPsocket client = players[i].socket;
            if (SDLNet_SocketReady(client)){
                coord position;
                int success = SDLNet_TCP_Recv(client, &position, sizeof(position));
                if (success <= 0){
                    std::cout << "Received error for client #" << i << ". Disconnecting." << std::endl;
                    clients.erase(clients.begin()+i);
                    players.erase(players.begin()+i);
                    number_of_clients--;
                    std::cout << number_of_clients << " Currently connected, now ;_;" << std::endl;
                    update_monitor( moving_clients, number_of_clients, clients );
                }
                players[i].position.x = position.x;
                players[i].position.y = position.y;
            }
        }
        // Send position changes
        for ( int i = 0; i < number_of_clients; i++ ){
            for ( int j = 0; j < number_of_clients; j++ ){
                if (i != j) { // same client, so don't send its position to itself.
                    int number_of_clients_for_client = number_of_clients-1;
                    SDLNet_TCP_Send( players[i].socket, &number_of_clients_for_client, sizeof(int));
                    SDLNet_TCP_Send( players[i].socket, &players[j].position, sizeof(coord));
                }
            }
        }
    }
    return 0;
}
