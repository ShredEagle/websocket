//
// See: https://www.boost.org/doc/libs/develop/libs/beast/example/websocket/server/async/websocket_server_async.cpp
//

#include "WebSocket.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp> // not usefull in our single threaded context

#include <iostream>
#include <memory>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace std::placeholders;

namespace ad {

boost::asio::io_context WebSocket::gIoc{};

//--------------

void fail(beast::error_code ec, char const* what)
{
    std::ostringstream oss;
    oss << what << ": " << ec.message() << "\n";
    throw std::runtime_error{oss.str()};
}

class session : public std::enable_shared_from_this<session>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit session(tcp::socket&& socket) :
        ws_(std::move(socket))
    {}

    // Get on the correct executor
    void run(NetworkContext::Accept_fun aOnAccept)
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(
            ws_.get_executor(),
            beast::bind_front_handler(
                &session::on_run,
                shared_from_this(),
                std::move(aOnAccept)));
    }

    // Start the asynchronous operation
    void on_run(NetworkContext::Accept_fun aOnAccept)
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-async");
            }));
        // Accept the websocket handshake
        ws_.async_accept(
            boost::beast::bind_front_handler(
                &session::on_accept,
                shared_from_this(),
                std::move(aOnAccept)));
    }

    void on_accept(NetworkContext::Accept_fun aOnAccept, beast::error_code ec)
    {
        if(ec) return fail(ec, "accept");

        // *this will be destructed when this function returns because its only shared_ptr is
        aOnAccept(WebSocket(std::make_unique<WebSocket::Impl>(std::move(*this))));
    }

    /*
     * API for WebSocket use
     */
    void registerRead(WebSocket::Message_fun aOnMessage)
    {
        ws_.async_read(
            buffer_,
            [userCallback = std::move(aOnMessage), this]
            (beast::error_code const& ec, std::size_t bytes_written)
            {
                /// \TODO We are not relying on implicit shared_ptr being destructed anymore
                /// user code can keep the WebSocket instance around after close
                /// better WebSocket API is needed to handle that
                if(ec == websocket::error::closed) return;

                if(ec) fail(ec, "onmessage");

                userCallback(beast::buffers_to_string(this->buffer_.data()));
                this->buffer_.consume(this->buffer_.size());
                this->registerRead(std::move(userCallback));
            });
    }

    template <class T_data>
    void write(const T_data & aData)
    {
        ws_.async_write(
            aData,
            [](const beast::error_code & ec, std::size_t /*bytes_transferred*/)
            {
                if(ec) return fail(ec, "write");
            });
    }

    /*
     * Not used anymore
     */
    void do_read()
    {
        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t /*bytes_transferred*/)
    {
        // This indicates that the session was closed
        if(ec == websocket::error::closed) return;

        if(ec) fail(ec, "read");

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write(
            buffer_.data(),
            beast::bind_front_handler(
                &session::on_write,
                shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t /*bytes_transferred*/)
    {
        if(ec) return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};

//------------------

// Accepts incoming connections and launches the sessions
// TODO no need to enable shared from this
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    NetworkContext::Accept_fun onAccept_;

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint, NetworkContext::Accept_fun aOnAccept) :
        ioc_(ioc),
        acceptor_(ioc),
        onAccept_(aOnAccept)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    //~listener()
    //{
    //    acceptor_.cancel();
    //    acceptor_.close();
    //}

    // Start accepting incoming connections
    void run()
    {
        do_accept();
    }

private:
    void do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            std::bind(&listener::on_accept, this, _1, _2));
    }

    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            if (ec != boost::system::errc::operation_canceled)
            {
                fail(ec, "accept");
            }
            return;
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(std::move(socket))->run(onAccept_);
        }

        // Accept another connection
        do_accept();
    }
};

//--------------

struct WebSocket::Impl
{
    Impl(session aSession) :
        isession(std::move(aSession))
    {}

    session isession;
};

WebSocket::WebSocket(std::unique_ptr<Impl> aImpl) :
    mImpl(std::move(aImpl))
{}

WebSocket::~WebSocket()
{}

WebSocket::WebSocket(WebSocket && aOther) :
    mImpl(std::move(aOther.mImpl))
{}

void WebSocket::send(const std::string & aText)
{
    mImpl->isession.write(net::const_buffer{aText.data(), aText.size()});
}

void WebSocket::onmessage(Message_fun aOnMessage)
{
    mImpl->isession.registerRead(aOnMessage);
}

struct NetworkContext::Impl
{
    Impl(boost::asio::io_context & aIoc,
         const std::string & aUrl,
         unsigned short aPort,
         Accept_fun aOnAccept) :
            ilistener{aIoc,
                      tcp::endpoint{net::ip::make_address(aUrl), aPort},
                      std::move(aOnAccept)}
    {
        ilistener.run();
    }

    listener ilistener;
};

NetworkContext::NetworkContext(const std::string & aUrl,
                               unsigned short aPort,
                               Accept_fun aOnAccept):
    mImpl(std::make_unique<Impl>(mIoc, aUrl, aPort, std::move(aOnAccept)))
{}

NetworkContext::~NetworkContext()
{}

void NetworkContext::run()
{
    mIOThread = std::unique_ptr<std::thread, std::function<void(std::thread*)>>(
            new std::thread{[this](){ mIoc.run(); }},
            [](std::thread *obj){obj->join(); delete obj;});
}

} // namespace ad
