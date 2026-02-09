#include "string_sink.hpp"
#include <boost/json.hpp>
#include <boost/capy.hpp>
#include <boost/capy/test/run_blocking.hpp>
#include <string>

#if !defined(BOOST_CAPY_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_CAPY_NO_LIB)
#define BOOST_LIB_NAME boost_capy
#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_COBALT_DYN_LINK)
#define BOOST_DYN_LINK
#endif
#include <boost/config/auto_link.hpp>
#endif

//

namespace
{

template<class WriteSink> boost::capy::task<void> serialize( boost::json::value const& v, WriteSink& ws );

template<class WriteSink> boost::capy::task<void> write( std::nullptr_t /*v*/, WriteSink& ws )
{
    co_await ws.write( "null", 4 );
}

template<class WriteSink> boost::capy::task<void> write( bool v, WriteSink& ws )
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

template<class WriteSink> boost::capy::task<void> write( std::int64_t v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    co_await ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> boost::capy::task<void> write( std::uint64_t v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    co_await ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> boost::capy::task<void> write( double v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v, std::chars_format::scientific );
    co_await ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> boost::capy::task<void> write( std::string_view v, WriteSink& ws )
{
    // ignore quoting for now
    co_await ws.write( "\"", 1 );
    co_await ws.write( v.data(), v.size() );
    co_await ws.write( "\"", 1 );
}

template<class WriteSink> boost::capy::task<void> write( boost::json::array const& v, WriteSink& ws )
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

template<class WriteSink> boost::capy::task<void> write( boost::json::object const& v, WriteSink& ws )
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

template<class WriteSink> boost::capy::task<void> serialize( boost::json::value const& v, WriteSink& ws )
{
    return visit( [&]( auto const& v ){ return write( v, ws ); }, v );
}

} // unnamed namespace

std::string serialize_capy_task_imm( boost::json::value const& jv )
{
    immediate_string_sink ws;
    boost::capy::test::run_blocking()( serialize( jv, ws ) );
    return std::move( ws.str );
}

std::string serialize_capy_task_def( boost::json::value const& jv )
{
    deferred_string_sink ws;
    boost::capy::test::run_blocking()( serialize( jv, ws ) );
    return std::move( ws.str );
}
