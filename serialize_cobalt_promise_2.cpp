#include "string_sink.hpp"
#include "file_sink.hpp"
#include "buffered_file_sink.hpp"
#include <boost/json.hpp>
#include <boost/cobalt.hpp>
#include <string>
#include <io.h>
#include <fcntl.h>

//

namespace
{

template<class WriteSink> boost::cobalt::promise<void> serialize( boost::json::value const& v, WriteSink& ws );

template<class WriteSink> auto write( std::nullptr_t /*v*/, WriteSink& ws, char (&/*buffer*/)[32])
{
    return ws.write( "null", 4 );
}

template<class WriteSink> auto write( bool v, WriteSink& ws, char (&/*buffer*/)[32])
{
    if( v )
    {
        return ws.write( "true", 4 );
    }
    else
    {
        return ws.write( "false", 5 );
    }
}

template<class WriteSink> auto write( std::int64_t v, WriteSink& ws, char (&buffer)[ 32 ] )
{
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    return ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> auto write( std::uint64_t v, WriteSink& ws, char (&buffer)[ 32 ] )
{
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    return ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> auto write( double v, WriteSink& ws, char (&buffer)[ 32 ] )
{
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v, std::chars_format::scientific );
    return ws.write( buffer, r.ptr - buffer );
}

static inline char hex_digit( int v )
{
    if( v > 9 )
    {
        return 'A' + v - 10;
    }
    else
    {
        return '0' + v;
    }
}

template<class WriteSink> boost::cobalt::promise<void> write( std::string_view v, WriteSink& ws, char (&/*buffer*/)[32])
{
    co_await ws.write( "\"", 1 );

    for( std::size_t i = 0;; )
    {
        std::size_t j = v.find_first_of( "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F\"\\", i );

        if( j == std::string_view::npos )
        {
            j = v.size();
        }

        if( j > i )
        {
            co_await ws.write( v.data() + i, j - i );
        }

        if( j == v.size() ) break;

        char ch = v[ j ];

        if( ch == '"' )
        {
            co_await ws.write( "\\\"", 2 );
        }
        else if( ch == '\\' )
        {
            co_await ws.write( "\\\\", 2 );
        }
        else
        {
            char buffer[] = "\\u0000";

            buffer[ 4 ] = hex_digit( (unsigned char)ch / 16  );
            buffer[ 5 ] = hex_digit( (unsigned char)ch % 16  );

            co_await ws.write( buffer, 6 );
        }

        i = j + 1;
    }

    co_await ws.write( "\"", 1 );
}

template<class WriteSink> boost::cobalt::promise<void> write( boost::json::object const& v, WriteSink& ws, char (&buffer)[ 32 ] );

template<class WriteSink> boost::cobalt::promise<void> write( boost::json::array const& v, WriteSink& ws, char (&buffer)[ 32 ] )
{
    co_await ws.write( "[", 1 );

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) co_await ws.write( ",", 1 );
        first = false;

        switch( x.kind() )
        {
        case boost::json::kind::null:

            co_await write( nullptr, ws, buffer );
            break;

        case boost::json::kind::bool_:

            co_await write( x.as_bool(), ws, buffer);
            break;

        case boost::json::kind::int64:

            co_await write( x.as_int64(), ws, buffer);
            break;

        case boost::json::kind::uint64:

            co_await write( x.as_uint64(), ws, buffer);
            break;

        case boost::json::kind::double_:

            co_await write( x.as_double(), ws, buffer);
            break;

        case boost::json::kind::string:

            co_await write( x.as_string(), ws, buffer);
            break;

        case boost::json::kind::array:

            co_await write( x.as_array(), ws, buffer);
            break;

        case boost::json::kind::object:

            co_await write( x.as_object(), ws, buffer);
            break;
        }
    }

    co_await ws.write( "]", 1 );
}

template<class WriteSink> boost::cobalt::promise<void> write( boost::json::object const& v, WriteSink& ws, char (&buffer)[ 32 ] )
{
    co_await ws.write( "{", 1 );

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) co_await ws.write( ",", 1 );
        first = false;

        co_await write( x.key(), ws, buffer );
        co_await ws.write( ":", 1 );
        co_await serialize( x.value(), ws );
    }

    co_await ws.write( "}", 1 );
}

template<class WriteSink> boost::cobalt::promise<void> serialize( boost::json::value const& v, WriteSink& ws )
{
    char buffer[ 32 ];

    switch( v.kind() )
    {
    case boost::json::kind::null:

        co_await write( nullptr, ws, buffer );
        break;

    case boost::json::kind::bool_:

        co_await write( v.as_bool(), ws, buffer);
        break;

    case boost::json::kind::int64:

        co_await write( v.as_int64(), ws, buffer);
        break;

    case boost::json::kind::uint64:

        co_await write( v.as_uint64(), ws, buffer);
        break;

    case boost::json::kind::double_:

        co_await write( v.as_double(), ws, buffer);
        break;

    case boost::json::kind::string:

        co_await write( v.as_string(), ws, buffer);
        break;

    case boost::json::kind::array:

        co_await write( v.as_array(), ws, buffer);
        break;

    case boost::json::kind::object:

        co_await write( v.as_object(), ws, buffer);
        break;
    }
}

} // unnamed namespace

std::string serialize_cobalt_promise_2_str( std::string_view /*name*/, boost::json::value const& jv )
{
    string_sink ws;

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        co_await serialize( jv, ws );

    }( jv, ws ) );

    return std::move( ws.str );
}

std::string serialize_cobalt_promise_2_file( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    file_sink ws{ fd };

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        co_await serialize( jv, ws );

    }( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}

std::string serialize_cobalt_promise_2_buf( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    buffered_file_sink ws{ fd };

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        co_await serialize( jv, ws );
        co_await ws.write_eof();

    }( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}
