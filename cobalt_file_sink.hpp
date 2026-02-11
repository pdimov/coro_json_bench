#ifndef COBALT_FILE_SINK_HPP_INCLUDED
#define COBALT_FILE_SINK_HPP_INCLUDED

#include <boost/cobalt/io/stream_file.hpp>
#include <boost/cobalt/io/write.hpp>
#include <string>
#include <coroutine>
#include <io.h>

struct cobalt_file_sink
{
    boost::cobalt::io::stream_file file;
    std::string str;

    auto write( void const* p, std::size_t n )
    {
        str.append( static_cast<char const*>( p ), n );
        return boost::cobalt::io::write( file, { { p, n } } );
    }
};

#endif // #ifndef COBALT_FILE_SINK_HPP_INCLUDED
