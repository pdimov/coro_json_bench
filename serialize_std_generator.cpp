#include "string_sink.hpp"
#include "file_sink.hpp"
#include "buffered_file_sink.hpp"
#include <boost/json.hpp>
#include <boost/cobalt.hpp>
#include <boost/capy.hpp>
#include <boost/capy/test/run_blocking.hpp>
#include <string>
#include <generator>
#include <io.h>
#include <fcntl.h>

//

namespace
{

std::generator<std::string_view, std::string> serialize( boost::json::value const& v );

std::generator<std::string_view, std::string> write( std::nullptr_t /*v*/ )
{
    co_yield "null";
}

std::generator<std::string_view, std::string> write( bool v )
{
    if( v )
    {
        co_yield "true";
    }
    else
    {
        co_yield "false";
    }
}

std::generator<std::string_view, std::string> write( std::int64_t v )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    co_yield std::string_view( buffer, r.ptr - buffer );
}

std::generator<std::string_view, std::string> write( std::uint64_t v )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    co_yield std::string_view( buffer, r.ptr - buffer );
}

std::generator<std::string_view, std::string> write( double v )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v, std::chars_format::scientific );
    co_yield std::string_view( buffer, r.ptr - buffer );
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

std::generator<std::string_view, std::string> write( std::string_view v )
{
    co_yield "\"";

    for( std::size_t i = 0;; )
    {
        std::size_t j = v.find_first_of( "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F\"\\", i );

        if( j == std::string_view::npos )
        {
            j = v.size();
        }

        if( j > i )
        {
            co_yield v.substr( i, j - i );
        }

        if( j == v.size() ) break;

        char ch = v[ j ];

        if( ch == '"' )
        {
            co_yield "\\\"";
        }
        else if( ch == '\\' )
        {
            co_yield "\\\\";
        }
        else
        {
            char buffer[] = "\\u0000";

            buffer[ 4 ] = hex_digit( (unsigned char)ch / 16  );
            buffer[ 5 ] = hex_digit( (unsigned char)ch % 16  );

            co_yield { buffer, 6 };
        }

        i = j + 1;
    }

    co_yield "\"";
}

std::generator<std::string_view, std::string> write( boost::json::array const& v )
{
    co_yield "[";

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) co_yield ",";
        first = false;

        co_yield std::ranges::elements_of( ::serialize( x ) );
    }

    co_yield "]";
}

std::generator<std::string_view, std::string> write( boost::json::object const& v )
{
    co_yield "{";

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) co_yield ",";
        first = false;

        co_yield std::ranges::elements_of( write( x.key() ) );
        co_yield ":";
        co_yield std::ranges::elements_of( ::serialize( x.value() ) );
    }

    co_yield "}";
}

std::generator<std::string_view, std::string> serialize( boost::json::value const& v )
{
    return visit( [&]( auto const& v ){ return write( v ); }, v );
}

} // unnamed namespace

std::string serialize_std_generator_cobalt_str( std::string_view /*name*/, boost::json::value const& jv )
{
    string_sink ws;

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

    }( jv, ws ) );

    return std::move( ws.str );
}

std::string serialize_std_generator_cobalt_file( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    file_sink ws{ fd };

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

    }( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}

std::string serialize_std_generator_cobalt_buf( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    buffered_file_sink ws{ fd };

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

        co_await ws.write_eof();

    }( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}

std::string serialize_std_generator_capy_str( std::string_view /*name*/, boost::json::value const& jv )
{
    string_sink ws;

    boost::capy::test::run_blocking()( []( auto const& jv, auto& ws ) -> boost::capy::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

    }( jv, ws ) );

    return std::move( ws.str );
}

std::string serialize_std_generator_capy_file( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    file_sink ws{ fd };

    boost::capy::test::run_blocking()( []( auto const& jv, auto& ws ) -> boost::capy::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

    }( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}

std::string serialize_std_generator_capy_buf( std::string_view name, boost::json::value const& jv )
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    buffered_file_sink ws{ fd };

    boost::capy::test::run_blocking()( []( auto const& jv, auto& ws ) -> boost::capy::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

        co_await ws.write_eof();

    }( jv, ws ) );

    _close( fd );

    return std::move( ws.str );
}
