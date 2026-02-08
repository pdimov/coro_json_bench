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

		std::string r = f( jv );

		auto t2 = std::chrono::high_resolution_clock::now();

		std::cout << name << ": " << ( t2 - t1 ) / 1us << " us.\n";

		boost::json::value jv2 = boost::json::parse( r );
		BOOST_TEST( jv2 == jv );
	}
	catch( std::exception const& x )
	{
		std::cout << "Exception when parsing the output of " << name << ": " << x.what() << std::endl;
	}
}

std::string serialize_sync( boost::json::value const& jv );
std::string serialize_cobalt_task( boost::json::value const& jv );
std::string serialize_cobalt_promise( boost::json::value const& jv );

int main()
{
	char const* fn = "../develop/libs/json/bench/data/numbers.json";

	std::cout << "Using " << fn << " as input\n\n";

	std::ifstream is( fn );
	auto jv = boost::json::parse( is );

	bench( "boost::json::serialize", []( auto const& jv ){ return boost::json::serialize( jv ); }, jv );
	bench( "serialize_sync", serialize_sync, jv );
	bench( "serialize_cobalt_task", serialize_cobalt_task, jv );
	bench( "serialize_cobalt_promise", serialize_cobalt_promise, jv );

	return boost::report_errors();
}
