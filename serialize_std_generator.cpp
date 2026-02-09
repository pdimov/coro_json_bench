#include <boost/json.hpp>
#include <boost/cobalt.hpp>
#include <boost/capy.hpp>
#include <boost/capy/test/run_blocking.hpp>
#include <string>
#include <generator>

std::string serialize_std_generator_cobalt( boost::json::value const& jv );
std::string serialize_std_generator_capy( boost::json::value const& jv );

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

std::generator<std::string_view, std::string> write( std::string_view v )
{
    // ignore quoting for now
    co_yield "\"";
    co_yield v;
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

struct write_sink_cobalt
{
    std::string r;

    boost::cobalt::promise<void> write( void const* p, std::size_t n )
    {
        r.append( static_cast<char const*>( p ), n );
        co_return;
    }
};

struct write_sink_capy
{
    std::string r;

    boost::capy::task<void> write( void const* p, std::size_t n )
    {
        r.append( static_cast<char const*>( p ), n );
        co_return;
    }
};

} // unnamed namespace

std::string serialize_std_generator_cobalt( boost::json::value const& jv )
{
    write_sink_cobalt ws;

    boost::cobalt::run( []( auto const& jv, auto& ws ) -> boost::cobalt::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

    }( jv, ws ) );

    return std::move( ws.r );
}

std::string serialize_std_generator_capy( boost::json::value const& jv )
{
    write_sink_capy ws;

    boost::capy::test::run_blocking()( []( auto const& jv, auto& ws ) -> boost::capy::task<void> {

        for( auto const& sv: ::serialize( jv ) )
        {
            co_await ws.write( sv.data(), sv.size() );
        }

        }( jv, ws ) );

    return std::move( ws.r );
}
