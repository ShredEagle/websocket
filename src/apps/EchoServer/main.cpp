#include <websocket/WebSocket.h>

#include <iostream>

int main(int argc, char ** argv)
{
    std::vector<ad::WebSocket> sockets;

    ad::NetworkContext network("0.0.0.0", 4321, [&sockets](ad::WebSocket aWS)
    {
        sockets.push_back(std::move(aWS));
        sockets.back().onmessage([&socket = sockets.back()](const std::string & aData)
        {
            std::cerr << "Received: " << aData << "\n";
            socket.send(aData);
        });
    });

    network.run();

    for(std::string command; std::cin && (command != "q"); std::cin >> command)
    {
        std::cout << "You typed: " << command << std::endl;
    }

    return EXIT_SUCCESS;
}
