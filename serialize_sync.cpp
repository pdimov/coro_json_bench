#include <boost/json.hpp>
#include <string>
#include <io.h>
#include <fcntl.h>

//

namespace
{

template<class WriteSink> void serialize( boost::json::value const& v, WriteSink& ws );

template<class WriteSink> void write( std::nullptr_t const& /*v*/, WriteSink& ws )
{
    ws.write( "null", 4 );
}

template<class WriteSink> void write( bool const& v, WriteSink& ws )
{
    if( v )
    {
        ws.write( "true", 4 );
    }
    else
    {
        ws.write( "false", 5 );
    }
}

template<class WriteSink> void write( std::int64_t const& v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> void write( std::uint64_t const& v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v );
    ws.write( buffer, r.ptr - buffer );
}

template<class WriteSink> void write( double const& v, WriteSink& ws )
{
    char buffer[ 32 ];
    auto r = std::to_chars( buffer, buffer + sizeof(buffer), v, std::chars_format::scientific );
    ws.write( buffer, r.ptr - buffer );
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

template<class WriteSink> void write( std::string_view v, WriteSink& ws )
{
    ws.write( "\"", 1 );

    for( std::size_t i = 0;; )
    {
        std::size_t j = v.find_first_of( "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F\"\\", i );

        if( j == std::string_view::npos )
        {
            j = v.size();
        }

        if( j > i )
        {
            ws.write( v.data() + i, j - i );
        }

        if( j == v.size() ) break;

        char ch = v[ j ];

        if( ch == '"' )
        {
            ws.write( "\\\"", 2 );
        }
        else if( ch == '\\' )
        {
            ws.write( "\\\\", 2 );
        }
        else
        {
            char buffer[] = "\\u0000";

            buffer[ 4 ] = hex_digit( (unsigned char)ch / 16  );
            buffer[ 5 ] = hex_digit( (unsigned char)ch % 16  );

            ws.write( buffer, 6 );
        }

        i = j + 1;
    }

    ws.write( "\"", 1 );
}

template<class WriteSink> void write( boost::json::array const& v, WriteSink& ws )
{
    ws.write( "[", 1 );

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) ws.write( ",", 1 );
        first = false;

        serialize( x, ws );
    }

    ws.write( "]", 1 );
}

template<class WriteSink> void write( boost::json::object const& v, WriteSink& ws )
{
    ws.write( "{", 1 );

    bool first = true;

    for( auto const& x: v )
    {
        if( !first ) ws.write( ",", 1 );
        first = false;

        write( x.key(), ws );
        ws.write( ":", 1 );
        serialize( x.value(), ws );
    }

    ws.write( "}", 1 );
}

template<class WriteSink> void serialize( boost::json::value const& v, WriteSink& ws )
{
    visit( [&]( auto const& v ){ write( v, ws ); }, v );
}

struct sync_string_sink
{
    std::string str;

    void write( void const* p, std::size_t n )
    {
        str.append( static_cast<char const*>( p ), n );
    }
};

struct sync_file_sink
{
    int fd;
    std::string str;

    void write( void const* p, std::size_t n )
    {
        str.append( static_cast<char const*>( p ), n );
        _write( fd, p, n );
    }
};

} // unnamed namespace

std::string serialize_sync_str( std::string_view /*name*/, boost::json::value const& jv)
{
    sync_string_sink ws;
    serialize( jv, ws );
    return std::move( ws.str );
}

std::string serialize_sync_file( std::string_view name, boost::json::value const& jv)
{
    auto fn = std::string( name ) + ".json";
    int fd = _open( fn.c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE );

    sync_file_sink ws{ fd };
    serialize( jv, ws );

    _close( fd );

    return std::move( ws.str );
}
