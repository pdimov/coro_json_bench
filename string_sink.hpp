#ifndef STRING_SINK_HPP_INCLUDED
#define STRING_SINK_HPP_INCLUDED

#include <string>
#include <coroutine>

struct string_sink
{
    std::string str;

    struct awaitable
    {
        string_sink* sink_;
        void const* p_;
        std::size_t n_;

        bool await_ready() const
        {
            sink_->str.append( static_cast<char const*>( p_ ), n_ );
            return true;
        }

        std::coroutine_handle<> await_suspend( std::coroutine_handle<> h ) const noexcept
        {
            return h;
        }

        // capy IoAwaitable
        template<class ExecRef, class StopToken>
        std::coroutine_handle<> await_suspend( std::coroutine_handle<> h, ExecRef const&, StopToken const& ) const noexcept
        {
            return h;
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

#endif // #ifndef STRING_SINK_HPP_INCLUDED
