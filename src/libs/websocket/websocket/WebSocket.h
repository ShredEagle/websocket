#pragma once

#include "Threading.h"

namespace ad {

class WebSocket
{
    struct Impl;
    friend class session;

public:
    using Accept_fun = std::function<void(WebSocket)>;
    using Message_fun = std::function<void(const std::string &)>;

    static void Listen(const std::string & aUrl, unsigned short aPort,
                       Accept_fun aOnAccept);

    // Cannot listen for incoming connections once it is called
    static void StartNetworking();

    ~WebSocket();

    WebSocket(WebSocket && aOther);

    void send(const std::string & aText);

    void onmessage(Message_fun aOnMessage);

private:
    explicit WebSocket(std::unique_ptr<Impl> aImpl);

private:
    static boost::asio::io_context gIoc;

    std::unique_ptr<Impl> mImpl;
};

} // namespace ad
