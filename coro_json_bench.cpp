#include <boost/json.hpp>
#include <boost/core/lightweight_test.hpp>
#include <fstream>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

template<class F> void bench( std::string_view name, F f, boost::json::value const& jv )
{
	try
	{
		auto t1 = std::chrono::high_resolution_clock::now();

		std::string r = f( name, jv );

		auto t2 = std::chrono::high_resolution_clock::now();

		std::cout << name << ": " << ( t2 - t1 ) / 1us << " us.\n";

		boost::json::value jv2 = boost::json::parse( r, {}, { .numbers = boost::json::number_precision::precise } );
		BOOST_TEST( jv2 == jv );
	}
	catch( std::exception const& x )
	{
		std::cout << "Exception when parsing the output of " << name << ": " << x.what() << std::endl;
	}
}

std::string serialize_sync_str( std::string_view name, boost::json::value const& jv );
std::string serialize_sync_file( std::string_view name, boost::json::value const& jv );
std::string serialize_sync_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_cobalt_task_str( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_task_file( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_task_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_cobalt_promise_str( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_promise_file( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_promise_file_async( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_promise_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_cobalt_promise_2_str( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_promise_2_file( std::string_view name, boost::json::value const& jv );
std::string serialize_cobalt_promise_2_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_std_generator_cobalt_str( std::string_view name, boost::json::value const& jv );
std::string serialize_std_generator_cobalt_file( std::string_view name, boost::json::value const& jv );
std::string serialize_std_generator_cobalt_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_std_generator_capy_str( std::string_view name, boost::json::value const& jv );
std::string serialize_std_generator_capy_file( std::string_view name, boost::json::value const& jv );
std::string serialize_std_generator_capy_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_capy_task_str( std::string_view name, boost::json::value const& jv );
std::string serialize_capy_task_file( std::string_view name, boost::json::value const& jv );
std::string serialize_capy_task_buf( std::string_view name, boost::json::value const& jv );

std::string serialize_capy_task_2_str( std::string_view name, boost::json::value const& jv );
std::string serialize_capy_task_2_file( std::string_view name, boost::json::value const& jv );
std::string serialize_capy_task_2_buf( std::string_view name, boost::json::value const& jv );

int main()
{
	char const* fn = "../develop/libs/json/bench/data/gsoc-2018.json";

	std::cout << "Using " << fn << " as input\n\n";

	std::ifstream is( fn );
	auto jv = boost::json::parse( is );

	bench( "boost::json::serialize", []( std::string_view /*name*/, auto const& jv ){ return boost::json::serialize( jv ); }, jv );
	std::cout << std::endl;

	bench( "serialize_sync_str", serialize_sync_str, jv );
	bench( "serialize_sync_file", serialize_sync_file, jv );
	bench( "serialize_sync_buf", serialize_sync_buf, jv );
	std::cout << std::endl;

	bench( "serialize_cobalt_task_str", serialize_cobalt_task_str, jv );
	bench( "serialize_cobalt_task_file", serialize_cobalt_task_file, jv );
	bench( "serialize_cobalt_task_buf", serialize_cobalt_task_buf, jv );
	std::cout << std::endl;

	bench( "serialize_cobalt_promise_str", serialize_cobalt_promise_str, jv );
	bench( "serialize_cobalt_promise_file", serialize_cobalt_promise_file, jv );
	// bench( "serialize_cobalt_promise_file_async", serialize_cobalt_promise_file_async, jv );
	bench( "serialize_cobalt_promise_buf", serialize_cobalt_promise_buf, jv );
	std::cout << std::endl;

	bench( "serialize_cobalt_promise_2_str", serialize_cobalt_promise_2_str, jv );
	bench( "serialize_cobalt_promise_2_file", serialize_cobalt_promise_2_file, jv );
	bench( "serialize_cobalt_promise_2_buf", serialize_cobalt_promise_2_buf, jv );
	std::cout << std::endl;

	bench( "serialize_std_generator_cobalt_str", serialize_std_generator_cobalt_str, jv );
	bench( "serialize_std_generator_cobalt_file", serialize_std_generator_cobalt_file, jv );
	bench( "serialize_std_generator_cobalt_buf", serialize_std_generator_cobalt_buf, jv );
	std::cout << std::endl;

	bench( "serialize_std_generator_capy_str", serialize_std_generator_capy_str, jv );
	bench( "serialize_std_generator_capy_file", serialize_std_generator_capy_file, jv );
	bench( "serialize_std_generator_capy_buf", serialize_std_generator_capy_buf, jv );
	std::cout << std::endl;

	bench( "serialize_capy_task_str", serialize_capy_task_str, jv );
	bench( "serialize_capy_task_file", serialize_capy_task_file, jv );
	bench( "serialize_capy_task_buf", serialize_capy_task_buf, jv );
	std::cout << std::endl;

	bench( "serialize_capy_task_2_str", serialize_capy_task_2_str, jv );
	bench( "serialize_capy_task_2_file", serialize_capy_task_2_file, jv );
	bench( "serialize_capy_task_2_buf", serialize_capy_task_2_buf, jv );
	std::cout << std::endl;

	return boost::report_errors();
}
