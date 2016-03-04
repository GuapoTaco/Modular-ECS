#pragma once

#include <bitset>
#include <array>


#include <boost/hana.hpp>

template<typename>
struct Entity;

namespace detail 
{
namespace lambdas 
{

auto removeTypeAddEntityPtr = [](auto manager)
	{
		return (Entity<typename decltype(manager)::type>*){};
	};
	
}
}

template<typename ManagerType_>
struct Entity
{
	static constexpr auto managerType = boost::hana::type_c<ManagerType_>;
	
	decltype(boost::hana::transform(decltype(managerType)::type::allManagers(), detail::lambdas::removeTypeAddEntityPtr)) bases;
	
	typename decltype(managerType)::type::RuntimeSignature_t signature;
	
	size_t ID;
	
	std::function<void()> destroy;

};
