#include "string_sink.hpp"
#include <boost/json.hpp>
#include <boost/cobalt.hpp>
#include <string>

std::string serialize_cobalt_promise_2( boost::json::value const& jv );

//

namespace
{

template<class WriteSink> boost::cobalt::promise<void> serialize( boost::json::value const& v, WriteSink& ws );

template<class WriteSink> auto write( std::nullptr_t /*v*/, WriteSink& ws, char (&buffer)[ 32 ])
{
    return ws.write( "null", 4 );
}

template<class WriteSink> auto write( bool v, WriteSink& ws, char (&buffer)[ 32 ] )
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

template<class WriteSink> boost::cobalt::promise<void> write( std::string_view v, WriteSink& ws, char (&buffer)[ 32 ] )
{
    // ignore quoting for now
    co_await ws.write( "\"", 1 );
    co_await ws.write( v.data(), v.size() );
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

std::string serialize_cobalt_promise_2_imm( boost::json::value const& jv )
{
    immediate_string_sink ws;

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        co_await serialize( jv, ws );

    }( jv, ws ) );

    return std::move( ws.str );
}

std::string serialize_cobalt_promise_2_def( boost::json::value const& jv )
{
    deferred_string_sink ws;

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        co_await serialize( jv, ws );

        }( jv, ws ) );

    return std::move( ws.str );
}
