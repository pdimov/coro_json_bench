#ifndef FILE_SINK_HPP_INCLUDED
#define FILE_SINK_HPP_INCLUDED

#include <string>
#include <coroutine>
#include <io.h>

namespace boost::capy
{
    struct io_env;
};

struct file_sink
{
    int fd;
    std::string str;

    struct awaitable
    {
        file_sink* sink_;
        void const* p_;
        std::size_t n_;

        bool await_ready() const
        {
            return false;
        }

        std::coroutine_handle<> await_suspend( std::coroutine_handle<> h ) const noexcept
        {
            sink_->str.append( static_cast<char const*>( p_ ), n_ );
            _write( sink_->fd, p_, n_ );
            return h;
        }

        // capy IoAwaitable
        std::coroutine_handle<> await_suspend( std::coroutine_handle<> h, boost::capy::io_env const* /*env*/) const noexcept
        {
            return this->await_suspend( h );
        }

        void await_resume() const noexcept
        {
        }
    };

    awaitable write( void const* p, std::size_t n )
    {
        return { this, p, n };
    }
};

#endif // #ifndef FILE_SINK_HPP_INCLUDED
