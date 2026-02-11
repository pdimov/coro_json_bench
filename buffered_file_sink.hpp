#ifndef BUFFERED_FILE_SINK_HPP_INCLUDED
#define BUFFERED_FILE_SINK_HPP_INCLUDED

#include <string>
#include <coroutine>
#include <io.h>

namespace boost::capy
{
    struct io_env;
};

struct buffered_file_sink
{
    int fd;
    std::string str;

    std::string buffer_;

    static constexpr std::size_t N = 16384; // buffer size

    struct write_op
    {
        buffered_file_sink* sink_;
        void const* p_;
        std::size_t n_;

        bool await_ready() const
        {
            if( sink_->buffer_.size() + n_ <= N )
            {
                sink_->buffer_.append( static_cast<char const*>( p_ ), n_ );
                return true;
            }

            return false;
        }

        std::coroutine_handle<> await_suspend( std::coroutine_handle<> h ) const noexcept
        {
            sink_->str.append( sink_->buffer_ );
            _write( sink_->fd, sink_->buffer_.data(), sink_->buffer_.size() );

            sink_->buffer_.resize( 0 );

            if( n_ < N )
            {
                sink_->buffer_.append( static_cast<char const*>( p_ ), n_ );
            }
            else
            {
                sink_->str.append( static_cast<char const*>( p_ ), n_ );
                _write( sink_->fd, p_, n_ );
            }

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

    write_op write( void const* p, std::size_t n )
    {
        return { this, p, n };
    }

    struct eof_op
    {
        buffered_file_sink* sink_;

        bool await_ready() const
        {
            return false;
        }

        std::coroutine_handle<> await_suspend( std::coroutine_handle<> h ) const noexcept
        {
            sink_->str.append( sink_->buffer_ );
            _write( sink_->fd, sink_->buffer_.data(), sink_->buffer_.size() );

            sink_->buffer_.resize( 0 );

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

    eof_op write_eof()
    {
        return eof_op{ this };
    }
};

#endif // #ifndef BUFFERED_FILE_SINK_HPP_INCLUDED
