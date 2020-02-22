#pragma once

#include <boost/asio/io_context.hpp>

#include <thread>

namespace ad {

/// \brief Naive single thread processing loop
class Threading
{
    friend class WebSocket;

public:
    Threading() :
        mIoc(1),
        mThread([this](){ this->mIoc.run(); })
    {}

    // Non-copiable, non-movable (lambda captures this)
    Threading(const Threading &) = delete;
    Threading & operator=(const Threading &) = delete;

private:
    boost::asio::io_context & ioc()
    {
        return mIoc;
    }

private:
    boost::asio::io_context mIoc;
    std::thread mThread;
};

} // namespace ad
