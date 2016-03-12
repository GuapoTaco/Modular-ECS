#include <boost/test/unit_test.hpp>

#include <Manager.h>

#define GET_HANA_CONSTANT_VALUE(name) ::std::decay_t<decltype(name)>::value

BOOST_AUTO_TEST_CASE(get_index_of_first_matching_test)
{
	auto tup = make_type_tuple<int, char, double>;

	BOOST_TEST(
		GET_HANA_CONSTANT_VALUE(get_index_of_first_matching(tup, boost::hana::type_c<int>)) == 0);
	BOOST_TEST(
		GET_HANA_CONSTANT_VALUE(get_index_of_first_matching(tup, boost::hana::type_c<char>)) == 1);
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(
				   get_index_of_first_matching(tup, boost::hana::type_c<double>)) == 2);
}

BOOST_AUTO_TEST_CASE(remove_dups_test1)
{
	auto tup = make_type_tuple<int, int, char, char, double, int>;

	auto removed = remove_dups(tup);

	BOOST_TEST(GET_HANA_CONSTANT_VALUE(boost::hana::contains(removed, boost::hana::type_c<int>)));
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(boost::hana::contains(removed, boost::hana::type_c<char>)));
	BOOST_TEST(
		GET_HANA_CONSTANT_VALUE(boost::hana::contains(removed, boost::hana::type_c<double>)));
	

	BOOST_TEST(GET_HANA_CONSTANT_VALUE(boost::hana::size(removed)) == 3);
}

BOOST_AUTO_TEST_CASE(remove_dups_test2)
{
	auto tup = make_type_tuple<int, char, double>;

	auto removed = remove_dups(tup);

	BOOST_TEST(GET_HANA_CONSTANT_VALUE(boost::hana::contains(removed, boost::hana::type_c<int>)));
	BOOST_TEST(GET_HANA_CONSTANT_VALUE(boost::hana::contains(removed, boost::hana::type_c<char>)));
	BOOST_TEST(
		GET_HANA_CONSTANT_VALUE(boost::hana::contains(removed, boost::hana::type_c<double>)));

	BOOST_TEST(GET_HANA_CONSTANT_VALUE(boost::hana::size(removed)) == 3);
}
