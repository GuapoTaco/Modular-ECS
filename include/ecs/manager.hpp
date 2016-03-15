/// @brief This defines the manager class
/// @author Russell Greene

#pragma once

#include <boost/hana.hpp>

#include <vector>
#include <utility>
#include <bitset>
#include <type_traits>
#include <deque>
#include <cassert>
#include <functional>
#include <iostream>

#include "ecs/segmented_map.hpp"
#include "ecs/misc_metafunctions.hpp"

namespace ecs {

/// @brief The templated class that holds custom storage for managers
/// Just override the template for your manager to use this!
template <typename T>
struct manager_data
{
};

/// @brief Convenience function that creates a boost::hana::tuple of boost::hana::type_c<>s from a
/// list of types
template <typename... T>
constexpr auto make_type_tuple = boost::hana::make_tuple(boost::hana::type_c<T>...);

namespace detail
{
auto removeTypeAddsegmented_map = [](auto arg)
{
	return segmented_map<size_t, typename decltype(arg)::type>{};
};
auto removeTypeAddPtr = [](auto arg)
{
	return (typename decltype(arg)::type*){};
};
}


/// @brief The core class of the library; Defines components,
template <typename components_, typename bases_ = boost::hana::tuple<>>
struct manager
{
	/**
	 * @brief Gets the list of components owned by the manager. 
	 *
	 * @return a boost::hana::tuple<> of components
	 */
	static constexpr auto my_components() { return boost::hana::make<components_>(); }
	/**
	 * @brief returns the direct bases of the manager. 
	 *
	 * @return a boost::hana::tuple<> of components
	 */
	static constexpr auto my_bases() { return boost::hana::make<bases_>(); }
	/**
	 * @brief Returns all the managers that are accessable to this managers--in the order of all
	 * base managers (direct and indirect) then manager (this manager class)
	 *
	 * @return a boost::hana::tuple<> of all the accessable managers
	 */
	static constexpr auto all_managers()
	{
		auto foldLam = [](auto running, auto arg)
		{
			return boost::hana::concat(decltype(arg)::type::all_managers(), running);
		};
		return decltype(boost::hana::append(
			remove_dups(boost::hana::concat(
				boost::hana::fold(my_bases(), boost::hana::make_tuple(), foldLam), my_bases())),
			boost::hana::type_c<manager>)){};
	}

	/**
	 * @brief Gets all the components, including the ones that are owned by base (direct and
	 * indirect) managers
	 *
	 * @return A boost::hana::tuple<>
	 */
	static constexpr auto all_components()
	{
		auto foldLam = [](auto arg)
		{
			return decltype(arg)::type::all_components();
		};
		return decltype(remove_dups(
			boost::hana::concat(boost::hana::fold(boost::hana::transform(my_bases(), foldLam),
												  boost::hana::make_tuple(), boost::hana::concat),
								my_components()))){};
	}

	/**
	 * @brief Gets the StorageComponents that this manager controls
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto my_storage_components()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple,
									boost::hana::append(tuple, newElement));
		};

		return decltype(boost::hana::fold(my_components(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets the TagComponents that this manager controls
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto my_tag_components()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement),
									boost::hana::append(tuple, newElement), tuple);
		};

		return decltype(boost::hana::fold(my_components(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets all the storage components, even those defined by base (direct and indirect)
	 * managers
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto all_storage_components()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple,
									boost::hana::append(tuple, newElement));
		};

		return decltype(boost::hana::fold(all_components(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets all the tag components, including those defined by base (direct and indirect)
	 * managers
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto all_tag_components()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement),
									boost::hana::append(tuple, newElement), tuple);
		};

		return decltype(boost::hana::fold(all_components(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets the ID of a component type in all_components()
	 *
	 * @param component A boost::hana::type_c<...>
	 * @return A boost::hana::size_c<...> if \a component is a component, else boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_component_id(T component)
	{
		return decltype(boost::hana::if_(boost::hana::contains(all_components(), component),
										 get_index_of_first_matching(all_components(), component),
										 boost::hana::nothing)){};
	}
	/**
	 * @brief Gets the ID of a component type in my_components()
	 *
	 * @param component a boost::hana::type_c<...> of the component to test
	 * @return A boost::hana::size_c<...> if \a component is a component, else boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_my_component_id(T component)
	{
		return decltype(boost::hana::if_(isComponent(component),
										 get_index_of_first_matching(my_components(), component),
										 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in all_storage_components()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in all_storage_components(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_storage_component_id(T component)
	{
		return decltype(
			boost::hana::if_(isStorageComponent(component),
							 get_index_of_first_matching(all_storage_components(), component),
							 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in my_storage_components()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in my_storage_components(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_my_stoarge_component_id(T component)
	{
		return decltype(boost::hana::if_(
			isStorageComponent(component),
			get_index_of_first_matching(my_storage_components(), component), boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in all_tag_components()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in all_tag_components(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_tag_component_id(T component)
	{
		return decltype(boost::hana::if_(isStorageComponent(component),
										 get_index_of_first_matching(all_tag_components(), component),
										 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in my_tag_components()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in my_tag_components(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_my_tag_component_id(T component)
	{
		return decltype(boost::hana::if_(
			isStorageComponent(component),
			get_index_of_first_matching(my_storage_components(), component), boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of the \c manager in all_managers()
	 *
	 * @param manager The manager to get the ID of. Should be a boost::hana::type_c<...>
	 * @return A boost::hana::size_c<...> if \c manager is in all_managers(), else
	 * boost::hana::nothing.
	 */
	template <typename T>
	static constexpr auto get_manager_id(T manager)
	{
		return decltype(boost::hana::if_(boost::hana::contains(all_managers(), manager),
										 get_index_of_first_matching(all_managers(), manager),
										 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of \c base in my_bases()
	 *
	 * @param base The base to get the ID of. Should be a boost::hana::type_c<...>
	 * @return A boost::hana::size_c<...> if \c base if in my_bases(), else boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_my_base_id(T base)
	{
		return decltype(boost::hana::if_(boost::hana::contains(my_bases(), base),
										 get_index_of_first_matching(my_bases(), base),
										 boost::hana::nothing)){};
	}

	/**
	 * @brief Sees if \c signature is an okay signature, as in all of the elements of \c signature
	 * are in all_components()
	 *
	 * @param base The singature to check, a boost:hana::tuple<boost::hana::type_c<...>, ...>.
	 * @return A boost::hana::bool_c<...>
	 */
	template <typename T>
	static constexpr auto is_possible_signature(T signature)
	{
		return decltype(boost::hana::all_of(signature, [](auto type)
											{
												return isComponent(type);
											})){};
	}

	/**
	 * @brief Gets the owning manager from a given component
	 * 
	 * @param component A boost::hana::type_c<component>. 
	 * 
	 * @return A boost::hana::type_c<...> of the managers that owns \c component. If \c component isn't a component, then it returns boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto get_manager_from_component(T component)
	{
		auto foldLam = [component](auto last, auto toTest)
		{
			return boost::hana::if_(decltype(toTest)::type::isMyComponent(component), toTest, last);
		};

		return decltype(boost::hana::if_(
			isComponent(component),
			boost::hana::fold(all_managers(), boost::hana::type_c<boost::hana::none_t>, foldLam), boost::hana::nothing)){};
	}

	template <typename T>
	static constexpr auto isolate_storage_components(T toIsolate)
	{
		return boost::hana::fold(
			toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isStorageComponent(toTest),
										boost::hana::append(currentSet, toTest), currentSet);
			});
	}
	template <typename T>
	static constexpr auto isolate_tag_components(T toIsolate)
	{
		return boost::hana::fold(
			toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isTagComponent(toTest),
										boost::hana::append(currentSet, toTest), currentSet);
			});
	}
	template <typename T>
	static constexpr auto isolate_my_components(T toIsolate)
	{
		return boost::hana::fold(
			toIsolate, boost::hana::make_set(), [](auto toTest, auto currentSet)
			{
				return boost::hana::if_(isMyComponent(toTest),
										boost::hana::append(currentSet, toTest), currentSet);
			});
	}

	template <typename T>
	static constexpr auto find_direct_base_manager_for_signature(T signature)
	{
		return boost::hana::fold(
			my_bases(), boost::hana::type_c<manager>, [&signature](auto toTest, auto currentRet)
			{
				return boost::hana::if_(decltype(toTest)::type::isSignature(signature), toTest,
										currentRet);
			});
	}

	template <typename T>
	static constexpr auto find_most_base_manager_for_signature(T signature)
	{
		using namespace boost::hana::literals;
		auto ret = boost::hana::while_(
			[](auto pair)
			{
				return pair[0_c] != pair[1_c];
			},
			boost::hana::make_tuple(boost::hana::type_c<manager>,
									find_direct_base_manager_for_signature(signature)),
			[&signature](auto tup)
			{
				return boost::hana::make_tuple(
					tup[1_c],
					decltype(tup[0_c])::type::find_direct_base_manager_for_signature(signature));
			});

		BOOST_HANA_CONSTANT_CHECK(ismanager(ret[0_c]));
		return ret[0_c];
	}
	
	struct entity {
		size_t id;
		std::function<void()> destroy;
	};

	using RuntimeSignature_t = std::bitset<decltype(boost::hana::size(all_components()))::value>;

	template <typename T>
	static RuntimeSignature_t generate_runtime_signature(T signature)
	{
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		RuntimeSignature_t ret;

		boost::hana::for_each(signature, [&ret](auto type)
							  {
								  ret[decltype(get_component_id(type))::value] = true;
							  });

		return ret;
	}

	template <typename T, typename Components>
	entity new_entity(T signature,
					Components&& components = decltype(components){} /*tuple of the components*/)
	{
		// TODO: implement
	}

	// returns the elements created [first, last)
	template <typename T, typename Components>
	std::vector<entity> create_entity_batch(T signature, Components components,
												size_t numToConstruct)
	{
		// TODO: implement
	}
	void destroy_entity(size_t handle) { 
		// TODO: implement 
		
	}
	template <typename T>
	auto get_storage_component(T component, size_t handle) ->
		typename decltype(component)::type &
	{
		// TODO: implement
	}

	template <typename T>
	bool has_component(T component, entity entity)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));

		constexpr auto managerForComponent = decltype(get_manager_from_component(component)){};

		auto ent = getEntityPtr(managerForComponent, entity);

		return ent->signature[decltype(
			decltype(managerForComponent)::type::template get_component_id(component))::value];
	}

	template <typename T>
	decltype(auto) get_ref_to_manager(T manager)
	{
		BOOST_HANA_CONSTANT_ASSERT(boost::hana::contains(all_managers(), manager));

		return *basePtrStorage[get_manager_id(manager)];
	}

	template <typename T>
	auto get_component_storage(T component)
		-> segmented_map<size_t, typename decltype(component)::type> &
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));

		constexpr auto manager = decltype(get_manager_from_component(component)){};

		const constexpr auto ID =
			decltype(manager)::type::template get_my_stoarge_component_id(component);

		return get_ref_to_manager(manager).stoarge_component_storage[ID];
	}
	
	// CALLING FUNCTIONS ON ENTITIES
	template <typename T, typename F>
	void call_function_with_signature_params(entity ent, T signature, F&& func)
	{
		// get components and put them in a tuple

		auto components = boost::hana::fold(
			signature, boost::hana::make_tuple(),
			[this, ent](auto retTuple, auto nextType) -> decltype(auto)
			{
				BOOST_HANA_CONSTANT_CHECK(isStorageComponent(nextType));
				return boost::hana::append(retTuple, get_storage_component(nextType, ent));
			});

		// expand
		boost::hana::unpack(components, std::forward<F>(func));
	}

	template <typename T, typename F>
	void run_all_matching(T signature, F&& functor)
	{
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		static constexpr auto manager = decltype(find_most_base_manager_for_signature(signature)){};

		get_ref_to_manager(manager).run_all_matchingIMPL(signature, std::forward<F>(functor));
	}

	template <typename T, typename F>
	void run_all_matchingIMPL(T signature, F&& functor)
	{
		// TODO: implement
	}

	manager_data<manager> my_manager_data;

	// storage for the actual components
	decltype(boost::hana::transform(my_storage_components(),
									detail::removeTypeAddsegmented_map)) stoarge_component_storage;
	std::array<std::vector<size_t>, boost::hana::size(my_components())> componentEntityStorage;
	decltype(boost::hana::transform(all_managers(), detail::removeTypeAddPtr)) basePtrStorage;

	manager_data<manager>& get_manager_data() { return my_manager_data; }
	manager(const decltype(boost::hana::transform(my_bases(), detail::removeTypeAddPtr))& bases = {})
	{
		using namespace boost::hana::literals;

		// we don't need to assign this, it is just this!
		auto tempBases = boost::hana::drop_back(basePtrStorage);

		boost::hana::for_each(
			tempBases, [&bases](auto& baseToSet)
			{

				// get a hana type_c of the basetoset
				auto constexpr baseToSet_type =
					boost::hana::type_c<std::remove_pointer_t<std::decay_t<decltype(baseToSet)>>>;
				BOOST_HANA_CONSTANT_CHECK(boost::hana::contains(all_managers(), baseToSet_type));

				// a lambda that checks if a contains the base we want
				auto hasBase = [&baseToSet_type](auto typeToCheck)
				{
					return boost::hana::contains(decltype(typeToCheck)::type::all_managers(),
												 baseToSet_type);
				};

				constexpr auto directBaseThatHasPtr_opt =
					decltype(boost::hana::find_if(my_bases(), hasBase)){};
				BOOST_HANA_CONSTANT_CHECK(boost::hana::is_just(directBaseThatHasPtr_opt));

				constexpr auto directBaseThatHasPtr = *directBaseThatHasPtr_opt;
				BOOST_HANA_CONSTANT_CHECK(boost::hana::contains(my_bases(), directBaseThatHasPtr));

				constexpr auto directBaseThatHasPtrID =
					decltype(manager::get_my_base_id(directBaseThatHasPtr)){};

				baseToSet = &(bases[directBaseThatHasPtrID]->get_ref_to_manager(baseToSet_type));

				if (!baseToSet)
					{
						std::cerr << "Could not find base: " << typeid(baseToSet).name()
								  << "; Did you forget to add it in the constructor?" << std::endl;
						std::terminate();
					}
			});

		basePtrStorage = boost::hana::append(tempBases, this);
	}

	~manager()
	{
		// TODO: add callbacks
	}
};

}
