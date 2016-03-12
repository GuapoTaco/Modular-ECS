/// @brief This defines the Manager class
/// @author Russell Greene

#pragma once

#include <boost/hana.hpp>

#include <vector>
#include <utility>
#include <bitset>
#include <type_traits>
#include <deque>
#include <cassert>
#include <iostream>

#include "SegmentedMap.h"

#include "MiscMetafunctions.h"
#include "Entity.h"

#undef max

/// @brief The templated class that holds custom storage for managers
/// Just override the template for your manager to use this!
template <typename T>
struct ManagerData
{
};

/// @brief Convenience function that creates a boost::hana::tuple of boost::hana::type_c<>s from a
/// list of types
template <typename... T>
constexpr auto make_type_tuple = boost::hana::make_tuple(boost::hana::type_c<T>...);

namespace detail
{
auto removeTypeAddSegmentedMap = [](auto arg)
{
	return SegmentedMap<size_t, typename decltype(arg)::type>{};
};
auto removeTypeAddPtr = [](auto arg)
{
	return (typename decltype(arg)::type*){};
};
}

/// @brief For distinguising if a class is a manager at all
struct ManagerBase
{
};

/// @brief The core class of the library; Defines components,
template <typename Components_, typename Bases_ = boost::hana::tuple<>>
struct Manager : ManagerBase
{
	static_assert(decltype(is_tuple<Components_>())::value,
				  "Components_ must be a boost::hana::tuple");
	static_assert(decltype(is_tuple<Bases_>())::value, "Bases_ must be a boost::hana::tuple");

	/**
	 * @brief Gets the list of components owned by the manager. The size of this should be
	 * getNumMyComponents()
	 *
	 * @return a boost::hana::tuple<> of components
	 */
	static constexpr auto myComponents() { return boost::hana::make<Components_>(); }
	/**
	 * @brief returns the direct bases of the manager. The size of this should be getNumBases()
	 *
	 * @return a boost::hana::tuple<> of components
	 */
	static constexpr auto myBases() { return boost::hana::make<Bases_>(); }
	/**
	 * @brief Returns all the managers that are accessable to this managers--in the order of all
	 * base managers (direct and indirect) then Manager (this manager class)
	 *
	 * @return a boost::hana::tuple<> of all the accessable managers
	 */
	static constexpr auto allManagers()
	{
		auto foldLam = [](auto running, auto arg)
		{
			return boost::hana::concat(decltype(arg)::type::allManagers(), running);
		};
		return decltype(boost::hana::append(
			remove_dups(boost::hana::concat(
				boost::hana::fold(myBases(), boost::hana::make_tuple(), foldLam), myBases())),
			boost::hana::type_c<Manager>)){};
	}

	/**
	 * @brief Gets all the components, including the ones that are owned by base (direct and
	 * indirect) managers
	 *
	 * @return A boost::hana::tuple<>
	 */
	static constexpr auto allComponents()
	{
		auto foldLam = [](auto arg)
		{
			return decltype(arg)::type::allComponents();
		};
		return decltype(remove_dups(
			boost::hana::concat(boost::hana::fold(boost::hana::transform(myBases(), foldLam),
												  boost::hana::make_tuple(), boost::hana::concat),
								myComponents()))){};
	}

	/**
	 * @brief Gets the StorageComponents that this manager controls
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto myStorageComponents()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple,
									boost::hana::append(tuple, newElement));
		};

		return decltype(boost::hana::fold(myComponents(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets the TagComponents that this manager controls
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto myTagComponents()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement),
									boost::hana::append(tuple, newElement), tuple);
		};

		return decltype(boost::hana::fold(myComponents(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets all the storage components, even those defined by base (direct and indirect)
	 * managers
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto allStorageComponents()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple,
									boost::hana::append(tuple, newElement));
		};

		return decltype(boost::hana::fold(allComponents(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets all the tag components, including those defined by base (direct and indirect)
	 * managers
	 *
	 * @return a boost::hana::tuple<>
	 */
	static constexpr auto allTagComponents()
	{
		auto foldLam = [](auto tuple, auto newElement)
		{
			return boost::hana::if_(boost::hana::traits::is_empty(newElement),
									boost::hana::append(tuple, newElement), tuple);
		};

		return decltype(boost::hana::fold(allComponents(), boost::hana::make_tuple(), foldLam)){};
	}

	/**
	 * @brief Gets the ID of a component type in allComponents()
	 *
	 * @param component A boost::hana::type_c<...>
	 * @return A boost::hana::size_c<...> if \a component is a component, else boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto getComponentID(T component)
	{
		return decltype(boost::hana::if_(boost::hana::contains(allComponents(), component),
										 get_index_of_first_matching(allComponents(), component),
										 boost::hana::nothing)){};
	}
	/**
	 * @brief Gets the ID of a component type in myComponents()
	 *
	 * @param component a boost::hana::type_c<...> of the component to test
	 * @return A boost::hana::size_c<...> if \a component is a component, else boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto getMyComponentID(T component)
	{
		return decltype(boost::hana::if_(isComponent(component),
										 get_index_of_first_matching(myComponents(), component),
										 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in allStorageComponents()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in allStorageComponents(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto getStorageComponentID(T component)
	{
		return decltype(
			boost::hana::if_(isStorageComponent(component),
							 get_index_of_first_matching(allStorageComponents(), component),
							 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in myStorageComponents()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in myStorageComponents(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto getMyStorageComponentID(T component)
	{
		return decltype(boost::hana::if_(
			isStorageComponent(component),
			get_index_of_first_matching(myStorageComponents(), component), boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in allTagComponents()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in allTagComponents(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto getTagComponentID(T component)
	{
		return decltype(boost::hana::if_(isStorageComponent(component),
										 get_index_of_first_matching(allTagComponents(), component),
										 boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of a component in myTagComponents()
	 *
	 * @param component the component to to get the ID of
	 * @return A boost::hana::size_c<...> if \a component is in myTagComponents(), else
	 * boost::hana::nothing
	 */
	template <typename T>
	static constexpr auto getMyTagComponentID(T component)
	{
		return decltype(boost::hana::if_(
			isStorageComponent(component),
			get_index_of_first_matching(myStorageComponents(), component), boost::hana::nothing)){};
	}

	/**
	 * @brief Gets the ID of the \c manager in allManagers()
	 *
	 * @param manager The manager to get the ID of. Should be a boost::hana::type_c<...>
	 * @return A boost::hana::size_c<...> if \c manager is in allManagers(), else
	 * boost::hana::nothing.
	 */
	template <typename T>
	static constexpr auto getManagerID(T manager)
	{
		return decltype(boost::hana::if_(boost::hana::contains(allManagers(), manager),
										 get_index_of_first_matching(allManagers(), manager),
										 boost::hana::nothing)){};
	}

	/**
	* @brief Gets the ID of \c base in myBases()
	*
	* @param base The base to get the ID of. Should be a boost::hana::type_c<...>
	* @return A boost::hana::type_c<...> if \c base if in myBases(), else boost::hana::nothing
	*/
	template <typename T>
	static constexpr auto getMyBaseID(T base)
	{
		return decltype(boost::hana::if_(boost::hana::contains(myBases(), base),
										 get_index_of_first_matching(myBases(), base),
										 boost::hana::nothing)){};
	}

	/**
	* @brief Sees if \c signature is an okay signature, as in all of the elements of \c signature
	* are in allComponents()
	*
	* @param base The singature to check, a boost:hana::tuple<boost::hana::type_c<...>, ...>.
	* @return A boost::hana::bool_c<...>
	*/
	template <typename T>
	static constexpr auto isPossibleSignature(T signature)
	{
		return decltype(boost::hana::all_of(signature, [](auto type)
											{
												return isComponent(type);
											})){};
	}

	template <typename T>
	static constexpr auto getManagerFromComponent(T component)
	{
		auto foldLam = [component](auto last, auto toTest)
		{
			return boost::hana::if_(decltype(toTest)::type::isMyComponent(component), toTest, last);
		};

		return decltype(boost::hana::if_(
			isComponent(component),
			boost::hana::fold(allManagers(), boost::hana::type_c<boost::hana::none_t>, foldLam))){};
	}

	template <typename T>
	static constexpr auto isolateStorageComponents(T toIsolate)
	{
		return boost::hana::fold(
			toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isStorageComponent(toTest),
										boost::hana::append(currentSet, toTest), currentSet);
			});
	}
	template <typename T>
	static constexpr auto isolateTagComponents(T toIsolate)
	{
		return boost::hana::fold(
			toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isTagComponent(toTest),
										boost::hana::append(currentSet, toTest), currentSet);
			});
	}
	template <typename T>
	static constexpr auto isolateMyComponents(T toIsolate)
	{
		return boost::hana::fold(
			toIsolate, boost::hana::make_set(), [](auto toTest, auto currentSet)
			{
				return boost::hana::if_(isMyComponent(toTest),
										boost::hana::append(currentSet, toTest), currentSet);
			});
	}

	template <typename T>
	static constexpr auto findDirectBaseManagerForSignature(T signature)
	{
		return boost::hana::fold(
			myBases(), boost::hana::type_c<Manager>, [&signature](auto toTest, auto currentRet)
			{
				return boost::hana::if_(decltype(toTest)::type::isSignature(signature), toTest,
										currentRet);
			});
	}

	template <typename T>
	static constexpr auto findMostBaseManagerForSignature(T signature)
	{
		using namespace boost::hana::literals;
		auto ret = boost::hana::while_(
			[](auto pair)
			{
				return pair[0_c] != pair[1_c];
			},
			boost::hana::make_tuple(boost::hana::type_c<Manager>,
									findDirectBaseManagerForSignature(signature)),
			[&signature](auto tup)
			{
				return boost::hana::make_tuple(
					tup[1_c],
					decltype(tup[0_c])::type::findDirectBaseManagerForSignature(signature));
			});

		BOOST_HANA_CONSTANT_CHECK(isManager(ret[0_c]));
		return ret[0_c];
	}

	using RuntimeSignature_t = std::bitset<decltype(boost::hana::size(allComponents()))::value>;

	template <typename T>
	static RuntimeSignature_t generateRuntimeSignature(T signature)
	{
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		RuntimeSignature_t ret;

		boost::hana::for_each(signature, [&ret](auto type)
							  {
								  ret[decltype(getComponentID(type))::value] = true;
							  });

		return ret;
	}

	template <typename T, typename Components>
	auto& newEntity(T signature,
					Components&& components = decltype(components){} /*tuple of the components*/)
	{
		using namespace boost::hana::literals;

		entityStorage.emplace_back();
		const size_t newEntityIndex = entityStorage.size() - 1;
		Entity<Manager>& newEntityRef = entityStorage[newEntityIndex];
		newEntityRef.signature = generateRuntimeSignature(signature);
		newEntityRef.ID = newEntityIndex;
		newEntityRef.bases[boost::hana::size(newEntityRef.bases) - boost::hana::size_c<1>] =
			&newEntityRef;
		newEntityRef.destroy = [ this, ID = newEntityRef.ID, signature ]()
		{
			// delete components
			boost::hana::for_each(isolateStorageComponents(signature),
								  [this, ID](auto componentToDestroy)
								  {
									  getComponentStorage(componentToDestroy).erase(ID);
								  });

			// delete entities
			boost::hana::for_each(entityStorage[ID].bases, [this, ID](auto basePtr)
								  {
									  constexpr auto baseType =
										  std::remove_pointer_t<decltype(basePtr)>::managerType;
									  BOOST_HANA_CONSTANT_CHECK(isManager(baseType));

									  getRefToManager(baseType).freeEntitySlots.push_back(ID);
								  });
		};

		// construct the components
		boost::hana::for_each(
			components, [this, newEntityIndex, &newEntityRef](auto& component)
			{
				constexpr auto component_type =
					boost::hana::type_c<std::decay_t<decltype(component)>>;
				BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component_type));

				constexpr auto manager_for_component =
					decltype(getManagerFromComponent(component_type)){};
				constexpr auto manager_id = getManagerID(manager_for_component);
				auto& refToManagerForComponent = getRefToManager(manager_for_component);

				auto& ptrToEntity = newEntityRef.bases[manager_id];
				if (ptrToEntity == nullptr)
					{
						refToManagerForComponent.entityStorage.emplace_back();
						size_t baseEntityID = refToManagerForComponent.entityStorage.size() - 1;
						auto& baseEntityRef = refToManagerForComponent.entityStorage[baseEntityID];
						baseEntityRef.ID = baseEntityID;
						baseEntityRef.bases[boost::hana::size(baseEntityRef.bases) -
											boost::hana::size_c<1>] = &baseEntityRef;
						baseEntityRef.destroy = newEntityRef.destroy;

						ptrToEntity = &baseEntityRef;
					}

				constexpr auto my_component_id =
					decltype(manager_for_component)::type::getMyStorageComponentID(component_type);
				constexpr auto all_component_id =
					decltype(manager_for_component)::type::getMyComponentID(component_type);

				refToManagerForComponent.storageComponentStorage[my_component_id][ptrToEntity->ID] =
					std::move(component);
				refToManagerForComponent.componentEntityStorage[decltype(all_component_id)::value]
					.push_back(ptrToEntity->ID);
			});

		return newEntityRef;
	}

	// returns the elements created [first, last)
	template <typename T, typename Components>
	std::pair<size_t, size_t> createEntityBatch(T signature, Components components,
												size_t numToConstruct)
	{
		// TODO: implement
	}
	void destroyEntity(Entity<Manager>* handle) { handle->destroy(); }
	template <typename T>
	auto getStorageComponent(T component, Entity<Manager>* handle) ->
		typename decltype(component)::type &
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));

		constexpr auto managerForComponent = decltype(getManagerFromComponent(component)){};

		constexpr auto staticID =
			decltype(managerForComponent)::type::template getMyStorageComponentID(component);

		return getRefToManager(managerForComponent).storageComponentStorage[staticID][handle->ID];
	}

	template <typename T>
	bool hasComponent(T component, Entity<Manager>* entity)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));

		constexpr auto managerForComponent = decltype(getManagerFromComponent(component)){};

		auto ent = getEntityPtr(managerForComponent, entity);

		return ent->signature[decltype(
			decltype(managerForComponent)::type::template getComponentID(component))::value];
	}

	template <typename T>
	static auto getEntityPtr(T managerToGet, Entity<Manager>* ent)
		-> Entity<typename decltype(managerToGet)::type> *
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(managerToGet));

		return ent->bases[getManagerID(managerToGet)];
	}

	template <typename T>
	decltype(auto) getRefToManager(T manager)
	{
		BOOST_HANA_CONSTANT_ASSERT(boost::hana::contains(allManagers(), manager));

		return *basePtrStorage[getManagerID(manager)];
	}

	template <typename T>
	auto getComponentStorage(T component)
		-> SegmentedMap<size_t, typename decltype(component)::type> &
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));

		constexpr auto manager = decltype(getManagerFromComponent(component)){};

		const constexpr auto ID =
			decltype(manager)::type::template getMyStorageComponentID(component);

		return getRefToManager(manager).storageComponentStorage[ID];
	}

	template <typename T>
	std::vector<Entity<Manager>*>& getComponentEntityStorage(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));

		static constexpr auto manager = getManagerFromComponent(component);

		const constexpr auto ID = decltype(manager)::type::template getMyComponentID(component);

		return getRefToManager(manager).componentEntityStorage[ID];
	}

	// CALLING FUNCTIONS ON ENTITIES
	template <typename T, typename F>
	void callFunctionWithSigParams(Entity<Manager>* ent, T signature, F&& func)
	{
		// get components and put them in a tuple

		auto components = boost::hana::fold(
			signature, boost::hana::make_tuple(),
			[this, ent](auto retTuple, auto nextType) -> decltype(auto)
			{
				BOOST_HANA_CONSTANT_CHECK(isStorageComponent(nextType));
				return boost::hana::append(retTuple, getStorageComponent(nextType, ent));
			});

		// expand
		boost::hana::unpack(components, std::forward<F>(func));
	}

	template <typename T, typename F>
	void runAllMatching(T signature, F&& functor)
	{
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		static constexpr auto manager = decltype(findMostBaseManagerForSignature(signature)){};

		getRefToManager(manager).runAllMatchingIMPL(signature, std::forward<F>(functor));
	}

	template <typename T, typename F>
	void runAllMatchingIMPL(T signature, F&& functor)
	{
		using namespace boost::hana::literals;

		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		// TODO: use shortest

		auto& entityVector = entityStorage;

		const auto runtimeSig = generateRuntimeSignature(signature);
		constexpr auto sigStorageCompsOnly = decltype(isolateStorageComponents(signature)){};

		for (Entity<Manager>& entity : entityVector)
			{
				if ((runtimeSig & entity.signature) == runtimeSig)
					{
						callFunctionWithSigParams(&entity, sigStorageCompsOnly,
												  std::forward<F>(functor));
					}
			}
	}

	ManagerData<Manager> myManagerData;

	// storage for the actual components
	decltype(boost::hana::transform(myStorageComponents(),
									detail::removeTypeAddSegmentedMap)) storageComponentStorage;
	std::array<std::vector<size_t>, boost::hana::size(myComponents())> componentEntityStorage;
	decltype(boost::hana::transform(allManagers(), detail::removeTypeAddPtr)) basePtrStorage;
	std::vector<Entity<Manager>> entityStorage;

	ManagerData<Manager>& getManagerData() { return myManagerData; }
	Manager(const decltype(boost::hana::transform(myBases(), detail::removeTypeAddPtr))& bases = {})
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
				BOOST_HANA_CONSTANT_CHECK(boost::hana::contains(allManagers(), baseToSet_type));

				// a lambda that checks if a contains the base we want
				auto hasBase = [&baseToSet_type](auto typeToCheck)
				{
					return boost::hana::contains(decltype(typeToCheck)::type::allManagers(),
												 baseToSet_type);
				};

				constexpr auto directBaseThatHasPtr_opt =
					decltype(boost::hana::find_if(myBases(), hasBase)){};
				BOOST_HANA_CONSTANT_CHECK(boost::hana::is_just(directBaseThatHasPtr_opt));

				constexpr auto directBaseThatHasPtr = *directBaseThatHasPtr_opt;
				BOOST_HANA_CONSTANT_CHECK(boost::hana::contains(myBases(), directBaseThatHasPtr));

				constexpr auto directBaseThatHasPtrID =
					decltype(Manager::getMyBaseID(directBaseThatHasPtr)){};

				baseToSet = &(bases[directBaseThatHasPtrID]->getRefToManager(baseToSet_type));

				if (!baseToSet)
					{
						std::cerr << "Could not find base: " << typeid(baseToSet).name()
								  << "; Did you forget to add it in the constructor?" << std::endl;
						std::terminate();
					}
			});

		basePtrStorage = boost::hana::append(tempBases, this);
	}

	~Manager()
	{
		// TODO: add callbacks
	}
};
