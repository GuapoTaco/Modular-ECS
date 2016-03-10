#include <boost/test/unit_test.hpp>

#include <Manager.h>

using namespace boost::hana::literals;
using namespace boost::hana;

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

auto base = Manager<decltype(make_type_tuple<data1>)>{};

auto sister1 = Manager<decltype(make_type_tuple<data2>), decltype(make_type_tuple<decltype(base)>)>{
	make_tuple(&base)};
auto sister2 = Manager<decltype(make_type_tuple<data3>), decltype(make_type_tuple<decltype(base)>)>{
	make_tuple(&base)};

auto child = Manager<decltype(make_type_tuple<>),
					 decltype(make_type_tuple<decltype(sister1), decltype(sister2)>)>{
	::make_tuple(&sister1, &sister2)};

#define TEST_CONST(a) BOOST_TEST(decltype(a)::value)

BOOST_AUTO_TEST_CASE(myComponents_test)
{
	TEST_CONST(base.myComponents() == make_type_tuple<data1>);
	TEST_CONST(sister1.myComponents() == make_type_tuple<data2>);
	TEST_CONST(sister2.myComponents() == make_type_tuple<data3>);

	TEST_CONST(is_empty(child.myComponents()));
}

BOOST_AUTO_TEST_CASE(myBases_test)
{
	TEST_CONST(base.myBases() == make_type_tuple<>);
	TEST_CONST(sister1.myBases() == make_type_tuple<decltype(base)>);
	TEST_CONST(sister2.myBases() == make_type_tuple<decltype(base)>);
	TEST_CONST((child.myBases() == make_type_tuple<decltype(sister1), decltype(sister2)>));
}

BOOST_AUTO_TEST_CASE(allManagers_test)
{
	TEST_CONST(base.allManagers() == make_type_tuple<decltype(base)>);

	TEST_CONST((contains(sister1.allManagers(), type_c<decltype(base)>)));
	TEST_CONST((contains(sister1.allManagers(), type_c<decltype(sister1)>)));

	TEST_CONST((contains(sister2.allManagers(), type_c<decltype(base)>)));
	TEST_CONST((contains(sister2.allManagers(), type_c<decltype(sister2)>)));

	TEST_CONST((contains(child.allManagers(), type_c<decltype(base)>)));
	TEST_CONST((contains(child.allManagers(), type_c<decltype(sister2)>)));
	TEST_CONST((contains(child.allManagers(), type_c<decltype(sister1)>)));
	TEST_CONST((contains(child.allManagers(), type_c<decltype(child)>)));
}

BOOST_AUTO_TEST_CASE(allComponents_test)
{
	TEST_CONST(base.allComponents() == make_type_tuple<data1>);

	TEST_CONST((contains(sister1.allComponents(), type_c<data1>)));
	TEST_CONST((contains(sister1.allComponents(), type_c<data2>)));

	TEST_CONST((contains(sister2.allComponents(), type_c<data1>)));
	TEST_CONST((contains(sister2.allComponents(), type_c<data3>)));

	TEST_CONST((contains(child.allComponents(), type_c<data1>)));
	TEST_CONST((contains(child.allComponents(), type_c<data2>)));
	TEST_CONST((contains(child.allComponents(), type_c<data3>)));
}

BOOST_AUTO_TEST_CASE(myStorageComponents_test)
{
	TEST_CONST((contains(base.myStorageComponents(), type_c<data1>)));

	TEST_CONST((is_empty(sister1.myStorageComponents())));

	TEST_CONST((contains(sister2.myStorageComponents(), type_c<data3>)));

	TEST_CONST((is_empty(child.myStorageComponents())));
}

BOOST_AUTO_TEST_CASE(myTagComponents_test)
{
	TEST_CONST((is_empty(base.myTagComponents())));

	TEST_CONST((contains(sister1.myTagComponents(), type_c<data2>)));

	TEST_CONST((is_empty(sister2.myTagComponents())));

	TEST_CONST((is_empty(child.myTagComponents())));
}

BOOST_AUTO_TEST_CASE(allStorageComponents_test)
{
	TEST_CONST((contains(base.allStorageComponents(), type_c<data1>)));

	TEST_CONST((contains(sister1.allStorageComponents(), type_c<data1>)));

	TEST_CONST((contains(sister2.allStorageComponents(), type_c<data1>)));
	TEST_CONST((contains(sister2.allComponents(), type_c<data3>)));

	TEST_CONST((contains(child.allStorageComponents(), type_c<data1>)));
	TEST_CONST((contains(child.allStorageComponents(), type_c<data3>)));
}

BOOST_AUTO_TEST_CASE(allTagComponents_test)
{
	TEST_CONST((is_empty(base.allTagComponents())));

	TEST_CONST((contains(sister1.allTagComponents(), type_c<data2>)));

	TEST_CONST((is_empty(sister2.allTagComponents())));

	TEST_CONST((contains(child.allTagComponents(), type_c<data2>)));
}

BOOST_AUTO_TEST_CASE(numComponents_test)
{
	TEST_CONST((base.numComponents() == size_c<1>));
	TEST_CONST((sister1.numComponents() == size_c<2>));
	TEST_CONST((sister2.numComponents() == size_c<2>));
	TEST_CONST((child.numComponents() == size_c<3>));
}

BOOST_AUTO_TEST_CASE(numMyComponents_test)
{
	TEST_CONST((base.numMyComponents() == size_c<1>));
	TEST_CONST((sister1.numMyComponents() == size_c<1>));
	TEST_CONST((sister2.numMyComponents() == size_c<1>));
	TEST_CONST((child.numMyComponents() == size_c<0>));
}

BOOST_AUTO_TEST_CASE(isComponent_test)
{
	TEST_CONST(base.isComponent(type_c<data1>));
	TEST_CONST(!base.isComponent(type_c<data2>));

	TEST_CONST(sister1.isComponent(type_c<data1>));
	TEST_CONST(sister1.isComponent(type_c<data2>));
	TEST_CONST(!sister1.isComponent(type_c<data3>));

	TEST_CONST(sister2.isComponent(type_c<data1>));
	TEST_CONST(!sister2.isComponent(type_c<data2>));
	TEST_CONST(sister2.isComponent(type_c<data3>));

	TEST_CONST(child.isComponent(type_c<data1>));
	TEST_CONST(child.isComponent(type_c<data2>));
	TEST_CONST(child.isComponent(type_c<data3>));
}

BOOST_AUTO_TEST_CASE(isMyComponent_test)
{
	TEST_CONST(base.isComponent(type_c<data1>));
	TEST_CONST(!base.isComponent(type_c<data2>));

	TEST_CONST(!sister1.isComponent(type_c<data1>));
	TEST_CONST(sister1.isComponent(type_c<data2>));
	TEST_CONST(!sister1.isComponent(type_c<data3>));

	TEST_CONST(!sister2.isComponent(type_c<data1>));
	TEST_CONST(!sister2.isComponent(type_c<data2>));
	TEST_CONST(sister2.isComponent(type_c<data3>));

	TEST_CONST(!child.isComponent(type_c<data1>));
	TEST_CONST(!child.isComponent(type_c<data2>));
	TEST_CONST(!child.isComponent(type_c<data3>));
}

BOOST_AUTO_TEST_CASE(getComponentID_test)
{
	base.getComponentID(type_c<data1>);
	TEST_CONST((base.getComponentID(type_c<data3>) == boost::hana::nothing));

	TEST_CONST((sister1.getComponentID(type_c<data1>) != sister1.getComponentID(type_c<data2>)));
	TEST_CONST((sister1.getComponentID(type_c<data3>) == boost::hana::nothing));

	TEST_CONST((sister2.getComponentID(type_c<data1>) != sister2.getComponentID(type_c<data3>)));
	TEST_CONST((sister2.getComponentID(type_c<data2>) == boost::hana::nothing));

	TEST_CONST((child.getComponentID(type_c<data1>) != child.getComponentID(type_c<data2>)));
	TEST_CONST((child.getComponentID(type_c<data2>) != child.getComponentID(type_c<data3>)));
	TEST_CONST((child.getComponentID(type_c<data1>) != child.getComponentID(type_c<data3>)));
}

BOOST_AUTO_TEST_CASE(getMyComponentID_test)
{
	base.getComponentID(type_c<data1>);

	TEST_CONST((sister1.getComponentID(type_c<data1>) != sister1.getComponentID(type_c<data2>)));

	TEST_CONST((sister2.getComponentID(type_c<data1>) != sister2.getComponentID(type_c<data3>)));

	TEST_CONST((child.getComponentID(type_c<data1>) != child.getComponentID(type_c<data2>)));
	TEST_CONST((child.getComponentID(type_c<data2>) != child.getComponentID(type_c<data3>)));
	TEST_CONST((child.getComponentID(type_c<data1>) != child.getComponentID(type_c<data3>)));
}
