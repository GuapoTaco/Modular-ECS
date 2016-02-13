
#define BOOST_TEST_MODULE Modular_ECS
#include <boost/test/included/unit_test.hpp>

#include <Manager.h>

namespace num_components
{

struct data1
{ double a; };

struct data2
{ };

struct data3
{ };

struct data4 
{ double p; };

struct data5
{ };

struct data6
{ };

#define GET_HANA_CONSTANT_VALUE(name) ::std::decay_t<decltype(name)>::value

BOOST_AUTO_TEST_CASE(num_components)
{
	Manager<decltype(make_type_tuple<data1, data2, data3>)> manager;
	
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(manager.numComponents) == 3);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(manager.numMyComponents) == 3);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(manager.numStorageComponents) == 1);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(manager.numMyStorageComponents) == 1);
}

BOOST_AUTO_TEST_CASE(num_components_inherantence)
{
	Manager<decltype(make_type_tuple<data1, data2, data3>)> base;
	
	Manager<decltype(make_type_tuple<data1, data4>), decltype(make_type_tuple<decltype(base)>)> sister1;
	Manager<decltype(make_type_tuple<data4, data5>), decltype(make_type_tuple<decltype(base)>)> sister2;
	
	Manager<decltype(make_type_tuple<data6>), decltype(make_type_tuple<decltype(sister1), decltype(sister2)>)> child;
	
	
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(base.numComponents) == 3);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(sister1.numComponents) == 4);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(sister1.numMyComponents) == 1);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(sister2.numComponents) == 5);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(sister2.numMyComponents) == 2);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(child.numComponents) == 6);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(child.numMyComponents) == 1);
	
}

}

