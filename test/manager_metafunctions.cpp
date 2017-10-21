#include <boost/test/unit_test.hpp>

#include <ecs/manager.hpp>

using namespace boost::hana::literals;
using namespace boost::hana;
using namespace ecs;

struct data1
{
	char a;
};
struct data2
{
};
struct data3
{
	char b;
};

auto base = create_manager(make_type_tuple<data1>);

auto sister1 = create_manager(make_type_tuple<data2>, make_tuple(&base));
auto sister2 = create_manager(make_type_tuple<data3>, make_tuple(&base));

auto child = create_manager(make_type_tuple<>, make_tuple(&sister1, &sister2));

#define TEST_CONST(a) BOOST_TEST(decltype(a)::value)

BOOST_AUTO_TEST_CASE(my_components_test)
{
	TEST_CONST(base.my_components == make_type_tuple<data1>);
	TEST_CONST(sister1.my_components == make_type_tuple<data2>);
	TEST_CONST(sister2.my_components == make_type_tuple<data3>);

	TEST_CONST(is_empty(child.my_components));
}

BOOST_AUTO_TEST_CASE(my_bases_test)
{
	TEST_CONST(base.my_bases == make_type_tuple<>);
	TEST_CONST(sister1.my_bases == make_type_tuple<decltype(base)>);
	TEST_CONST(sister2.my_bases == make_type_tuple<decltype(base)>);
	TEST_CONST((child.my_bases == make_type_tuple<decltype(sister1), decltype(sister2)>));
}

BOOST_AUTO_TEST_CASE(all_managers_test)
{
	TEST_CONST(base.all_managers == make_type_tuple<decltype(base)>);

	TEST_CONST((contains(sister1.all_managers, type_c<decltype(base)>)));
	TEST_CONST((contains(sister1.all_managers, type_c<decltype(sister1)>)));

	TEST_CONST((contains(sister2.all_managers, type_c<decltype(base)>)));
	TEST_CONST((contains(sister2.all_managers, type_c<decltype(sister2)>)));

	TEST_CONST((contains(child.all_managers, type_c<decltype(base)>)));
	TEST_CONST((contains(child.all_managers, type_c<decltype(sister2)>)));
	TEST_CONST((contains(child.all_managers, type_c<decltype(sister1)>)));
	TEST_CONST((contains(child.all_managers, type_c<decltype(child)>)));
}

BOOST_AUTO_TEST_CASE(all_components_test)
{
	TEST_CONST(base.all_components == make_type_tuple<data1>);

	TEST_CONST((contains(sister1.all_components, type_c<data1>)));
	TEST_CONST((contains(sister1.all_components, type_c<data2>)));

	TEST_CONST((contains(sister2.all_components, type_c<data1>)));
	TEST_CONST((contains(sister2.all_components, type_c<data3>)));

	TEST_CONST((contains(child.all_components, type_c<data1>)));
	TEST_CONST((contains(child.all_components, type_c<data2>)));
	TEST_CONST((contains(child.all_components, type_c<data3>)));
}

BOOST_AUTO_TEST_CASE(my_storage_components_test)
{
	TEST_CONST((contains(base.my_storage_components, type_c<data1>)));

	TEST_CONST((is_empty(sister1.my_storage_components)));

	TEST_CONST((contains(sister2.my_storage_components, type_c<data3>)));

	TEST_CONST((is_empty(child.my_storage_components)));
}

BOOST_AUTO_TEST_CASE(my_tag_components_test)
{
	TEST_CONST((is_empty(base.my_tag_components)));

	TEST_CONST((contains(sister1.my_tag_components, type_c<data2>)));

	TEST_CONST((is_empty(sister2.my_tag_components)));

	TEST_CONST((is_empty(child.my_tag_components)));
}

BOOST_AUTO_TEST_CASE(all_storage_components_test)
{
	TEST_CONST((contains(base.all_storage_components, type_c<data1>)));

	TEST_CONST((contains(sister1.all_storage_components, type_c<data1>)));

	TEST_CONST((contains(sister2.all_storage_components, type_c<data1>)));
	TEST_CONST((contains(sister2.all_components, type_c<data3>)));

	TEST_CONST((contains(child.all_storage_components, type_c<data1>)));
	TEST_CONST((contains(child.all_storage_components, type_c<data3>)));
}

BOOST_AUTO_TEST_CASE(all_tag_components_test)
{
	TEST_CONST((is_empty(base.all_tag_components)));

	TEST_CONST((contains(sister1.all_tag_components, type_c<data2>)));

	TEST_CONST((is_empty(sister2.all_tag_components)));

	TEST_CONST((contains(child.all_tag_components, type_c<data2>)));
}

BOOST_AUTO_TEST_CASE(get_component_id_test)
{
	base.get_component_id(type_c<data1>);
	TEST_CONST((base.get_component_id(type_c<data3>) == boost::hana::nothing));

	TEST_CONST(
		(sister1.get_component_id(type_c<data1>) != sister1.get_component_id(type_c<data2>)));
	TEST_CONST((sister1.get_component_id(type_c<data3>) == boost::hana::nothing));

	TEST_CONST(
		(sister2.get_component_id(type_c<data1>) != sister2.get_component_id(type_c<data3>)));
	TEST_CONST((sister2.get_component_id(type_c<data2>) == boost::hana::nothing));

	TEST_CONST((child.get_component_id(type_c<data1>) != child.get_component_id(type_c<data2>)));
	TEST_CONST((child.get_component_id(type_c<data2>) != child.get_component_id(type_c<data3>)));
	TEST_CONST((child.get_component_id(type_c<data1>) != child.get_component_id(type_c<data3>)));
}
