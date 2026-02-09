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

		boost::json::value jv2 = boost::json::parse( r, {}, { .numbers = boost::json::number_precision::precise } );
		BOOST_TEST( jv2 == jv );
	}
	catch( std::exception const& x )
	{
		std::cout << "Exception when parsing the output of " << name << ": " << x.what() << std::endl;
	}
}

std::string serialize_sync( boost::json::value const& jv );
std::string serialize_cobalt_task_imm( boost::json::value const& jv );
std::string serialize_cobalt_task_def( boost::json::value const& jv );
std::string serialize_cobalt_promise_imm( boost::json::value const& jv );
std::string serialize_cobalt_promise_def( boost::json::value const& jv );
std::string serialize_cobalt_promise_2_imm( boost::json::value const& jv );
std::string serialize_cobalt_promise_2_def( boost::json::value const& jv );
std::string serialize_std_generator_cobalt( boost::json::value const& jv );
std::string serialize_std_generator_capy( boost::json::value const& jv );
std::string serialize_capy_task( boost::json::value const& jv );
std::string serialize_capy_task_2( boost::json::value const& jv );

int main()
{
	char const* fn = "../develop/libs/json/bench/data/random.json";

	std::cout << "Using " << fn << " as input\n\n";

	std::ifstream is( fn );
	auto jv = boost::json::parse( is );

	bench( "boost::json::serialize", []( auto const& jv ){ return boost::json::serialize( jv ); }, jv );
	bench( "serialize_sync", serialize_sync, jv );
	bench( "serialize_cobalt_task_imm", serialize_cobalt_task_imm, jv );
	bench( "serialize_cobalt_task_def", serialize_cobalt_task_def, jv );
	bench( "serialize_cobalt_promise_imm", serialize_cobalt_promise_imm, jv );
	bench( "serialize_cobalt_promise_def", serialize_cobalt_promise_def, jv );
	bench( "serialize_cobalt_promise_2_imm", serialize_cobalt_promise_2_imm, jv );
	bench( "serialize_cobalt_promise_2_def", serialize_cobalt_promise_2_def, jv );
	bench( "serialize_std_generator_cobalt", serialize_std_generator_cobalt, jv );
	bench( "serialize_std_generator_capy", serialize_std_generator_capy, jv );
	bench( "serialize_capy_task", serialize_capy_task, jv );
	bench( "serialize_capy_task_2", serialize_capy_task_2, jv );

	return boost::report_errors();
}
