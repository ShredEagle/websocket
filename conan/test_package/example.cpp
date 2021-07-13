#include <websocket/WebSocket.h>

#include <cstdlib>

int main()
{
    ad::NetworkContext context{"0.0.0.0", 33454, [](ad::WebSocket){}};
    context.run();
    return EXIT_SUCCESS;
}
