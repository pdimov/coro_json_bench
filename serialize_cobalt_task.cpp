#include "string_sink.hpp"
#include "file_sink.hpp"
#include <boost/json.hpp>
#include <boost/cobalt.hpp>
#include <string>
#include <io.h>
#include <fcntl.h>

//

namespace
{

template<class WriteSink> boost::cobalt::task<void> serialize( boost::json::value const& v, WriteSink& ws );

template<class WriteSink> boost::cobalt::task<void> write( std::nullptr_t /*v*/, WriteSink& ws )
{
    co_await ws.write( "null", 4 );
}

template<class WriteSink> boost::cobalt::task<void> write( bool v, WriteSink& ws )
{
    if( v )
    {
        co_await ws.write( "true", 4 );
    }
    else
    {
        co_await ws.write( "false", 5 );
    }
}

template<class WriteSink> boost::cobalt::task<void> write( std::int64_t v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    co_await ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> boost::cobalt::task<void> write( std::uint64_t v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    co_await ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> boost::cobalt::task<void> write( double v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v, std::chars_format::scientific );
    co_await ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> boost::cobalt::task<void> write( std::string_view v, WriteSink& ws )
{
    // ignore quoting for now
    co_await ws.write( "\"", 1 );
    co_await ws.write( v.data(), v.size() );
    co_await ws.write( "\"", 1 );
}

template<class WriteSink> boost::cobalt::task<void> write( boost::json::array const& v, WriteSink& ws )
{
    co_await ws.write( "[", 1 );

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) co_await ws.write( ",", 1 );
        first = false;

        co_await serialize( x, ws );
    }

    co_await ws.write( "]", 1 );
}

template<class WriteSink> boost::cobalt::task<void> write( boost::json::object const& v, WriteSink& ws )
{
    co_await ws.write( "{", 1 );

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) co_await ws.write( ",", 1 );
        first = false;

        co_await write( x.key(), ws );
        co_await ws.write( ":", 1 );
        co_await serialize( x.value(), ws );
    }

    co_await ws.write( "}", 1 );
}

template<class WriteSink> boost::cobalt::task<void> serialize( boost::json::value const& v, WriteSink& ws )
{
    return visit( [&]( auto const& v ){ return write( v, ws ); }, v );
}

} // unnamed namespace

std::string serialize_cobalt_task_imm( std::string_view /*name*/, boost::json::value const& jv )
{
    immediate_string_sink ws;
    boost::cobalt::run( serialize( jv, ws ) );
    return std::move( ws.str );
}

std::string serialize_cobalt_task_def( std::string_view /*name*/, boost::json::value const& jv )
{
    deferred_string_sink ws;
    boost::cobalt::run( serialize( jv, ws ) );
    return std::move( ws.str );
}

std::string serialize_cobalt_task_file( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    file_sink ws{ fd };

    boost::cobalt::run( serialize( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}
