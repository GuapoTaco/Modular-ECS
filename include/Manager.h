#pragma once

#include <boost/hana.hpp>

#include <vector>
#include <utility>
#include <bitset>
#include <type_traits>
#include <deque>
#include <cassert>
#include <unordered_map>

#include "SegmentedMap.h"

#include "MiscMetafunctions.h"
#include "Entity.h"


#undef max

template<typename T>
struct ManagerData{};

template<typename... T>
constexpr auto make_type_tuple = boost::hana::make_tuple(boost::hana::type_c<T>...);

// we have to have these here because lambdas aren't allowed to be static, and because we need to have them in unevaluated contexts
namespace detail 
{
namespace lambdas
{
	
auto myStorageComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple, boost::hana::append(tuple, newElement));
	};
	
auto myTagComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), boost::hana::append(tuple, newElement), tuple);
	};
	
auto allStorageComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), tuple, boost::hana::append(tuple, newElement));
	};
	
auto allTagComponents_LAM = [](auto tuple, auto newElement)
	{
		return boost::hana::if_(boost::hana::traits::is_empty(newElement), boost::hana::append(tuple, newElement), tuple);
	};
auto removeTypeAddVec = [](auto arg)
	{
		return std::vector<typename decltype(arg)::type>{};
	};
auto removeTypeAddSegmentedMap = [](auto arg)
	{
		return SegmentedMap<size_t, typename decltype(arg)::type>{};
	};
auto removeTypeAddPtr = [](auto arg)
	{
		return (typename decltype(arg)::type*){};
	};
auto getAllManagers = [](auto arg)
	{
		return decltype(arg)::type::allManagers;
	};
auto getAllComponents = [](auto arg)
	{
		return decltype(arg)::type::allComponents;
	};
}
}


struct ManagerBase{};

template <typename Components_, typename Bases_ = boost::hana::tuple<> >
struct Manager : ManagerBase
{
	static_assert(decltype(is_tuple<Components_>())::value, "Components_ must be a boost::hana::tuple");
	static_assert(decltype(is_tuple<Bases_>())::value, "Bases_ must be a boost::hana::tuple");
	
	// make sure there are no duplicates in the components
	
	
	static constexpr auto myComponents = boost::hana::make<Components_>();
	static constexpr auto myBases = boost::hana::make<Bases_>();
	using This_t = Manager<Components_, Bases_>;
	
	static constexpr auto allManagers = decltype(boost::hana::append(remove_dups(boost::hana::concat(boost::hana::fold(
		boost::hana::transform(myBases, detail::lambdas::getAllManagers), boost::hana::make_tuple(), boost::hana::concat), myBases)) , boost::hana::type_c<This_t>)){};

	static constexpr auto allComponents = decltype(remove_dups(boost::hana::concat(boost::hana::fold(boost::hana::transform(myBases, detail::lambdas::getAllComponents), 
		boost::hana::make_tuple(), boost::hana::concat), myComponents))){}; 

	static constexpr auto myStorageComponents = decltype(boost::hana::fold(myComponents, boost::hana::make_tuple(), detail::lambdas::myStorageComponents_LAM)){};
	static constexpr auto myTagComponents = decltype(boost::hana::fold(myComponents, boost::hana::make_tuple(), detail::lambdas::myTagComponents_LAM)){};
	static constexpr auto allStorageComponents = decltype(boost::hana::fold(allComponents, boost::hana::make_tuple(), detail::lambdas::allStorageComponents_LAM)){};
	static constexpr auto allTagComponents = decltype(boost::hana::fold(allComponents, boost::hana::make_tuple(), detail::lambdas::allTagComponents_LAM)){};

	template<typename... Args>
	using TupleOfPtrs = std::tuple<Args*...>;

	// CONSTEXPR FUNCTIONS/TESTS


	static constexpr auto numComponents = boost::hana::size(allComponents);
	
	static constexpr auto numMyComponents = boost::hana::size(myComponents);
	
	template<typename T> 
	static constexpr auto isComponent(T componentToTest)
	{
		return boost::hana::contains(allComponents, componentToTest);
	}
	template<typename T> 
	static constexpr auto isMyComponent(T componentToTest)
	{
		return boost::hana::contains(myComponents, componentToTest);
	}
	template <typename T> 
	static constexpr auto getComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		return get_index_of_first_matching(allComponents, component);
	}
	template <typename T> 
	static constexpr auto getMyComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		return get_index_of_first_matching(myComponents, component);
	}

	static constexpr auto numStorageComponents = boost::hana::size(allStorageComponents);
	static constexpr auto numMyStorageComponents = boost::hana::size(allStorageComponents);
	
	template<typename T> 
	static constexpr auto isStorageComponent(T componentToTest)
	{
		return boost::hana::contains(allStorageComponents, componentToTest);
	}
	template<typename T> 
	static constexpr auto isMyStorageComponent(T componentToTest)
	{
		return boost::hana::contains(myStorageComponents, componentToTest);
	}
	template <typename T> 
	static constexpr auto getMyStorageComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		return get_index_of_first_matching(myStorageComponents, component);
	}
	template <typename T> 
	static constexpr auto getStorageComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		return get_index_of_first_matching(allStorageComponents, component);
	}
	
	template<typename T> 
	static constexpr auto isTagComponent(T componentToTest)
	{
		return boost::hana::contains(allTagComponents, componentToTest);
	}
	template<typename T> 
	static constexpr auto isMyTagComponent(T componentToTest)
	{
		return boost::hana::contains(myTagComponents, componentToTest);
	}
	template <typename T> 
	static constexpr auto getTagComponentID(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isTagComponent(component));
		
		return get_index_of_first_matching(allTagComponents, component);
	}
	
	static constexpr auto numManagers = boost::hana::size(allManagers);
	
	template<typename T>
	static constexpr auto isManager(T toTest)
	{
	//	static_assert(std::is_base_of<ManagerBase, typename decltype(toTest)::type>::value, "Error, needs to be a manager");
		
		return boost::hana::contains(allManagers, toTest);
	}
	template <typename T> 
	static constexpr auto getManagerID(T manager)
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(manager));  
		
		return get_index_of_first_matching(allManagers, manager);
	}
	template<typename T> 
	static constexpr auto isBase(T baseToTest)
	{
		return boost::hana::contains(myBases, baseToTest);
	}

	static constexpr auto numBases = boost::hana::size(myBases);
	
	template <typename T> 
	static constexpr auto getBaseID(T base)
	{
		BOOST_HANA_CONSTANT_CHECK(isBase(base));
		
		return get_index_of_first_matching(myBases, base);
	}
	
	template<typename T>
	static constexpr auto isSignature(T signature)
	{
		return boost::hana::all_of(signature, [](auto type){ return isComponent(type); });
	}

	template<typename T>
	static constexpr auto getManagerFromComponent(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		return boost::hana::fold(allManagers, boost::hana::type_c<boost::hana::none_t>, [component](auto last, auto toTest)
			{
				return boost::hana::if_(decltype(toTest)::type::isMyComponent(component), toTest, last);
			}
		);
	}


	template<typename T>
	static constexpr auto isolateStorageComponents(T toIsolate)
	{
		return boost::hana::fold(toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isStorageComponent(toTest), boost::hana::append(currentSet, toTest), currentSet);
			}
		);
	}
	template<typename T>
	static constexpr auto isolateTagComponents(T toIsolate)
	{
		return boost::hana::fold(toIsolate, boost::hana::make_tuple(), [](auto currentSet, auto toTest)
			{
				return boost::hana::if_(isTagComponent(toTest), boost::hana::append(currentSet, toTest), currentSet);
			}
		);
	}
	template<typename T>
	static constexpr auto isolateMyComponents(T toIsolate)
	{
		return boost::hana::fold(toIsolate, boost::hana::make_set(), [](auto toTest, auto currentSet)
			{
				return boost::hana::if_(isMyComponent(toTest), boost::hana::append(currentSet, toTest), currentSet);
			}
		);
	}
	
	template<typename T>
	static constexpr auto findDirectBaseManagerForSignature(T signature)
	{
		
		return boost::hana::fold(myBases, boost::hana::type_c<This_t>, [&signature](auto toTest, auto currentRet)
			{
				return boost::hana::if_(decltype(toTest)::type::isSignature(signature), toTest, currentRet);
			}
		);
	}
	
	template<typename T>
	static constexpr auto findMostBaseManagerForSignature(T signature)
	{
		using namespace boost::hana::literals;
		auto ret = boost::hana::while_([](auto pair){ return pair[0_c] != pair[1_c]; }, 
			boost::hana::make_tuple(boost::hana::type_c<This_t>, findDirectBaseManagerForSignature(signature)), [&signature](auto tup)
			{
				return boost::hana::make_tuple(tup[1_c], decltype(tup[0_c])::type::findDirectBaseManagerForSignature(signature)); 
			}
		);
		
		BOOST_HANA_CONSTANT_CHECK(isManager(ret[0_c]));
		return ret[0_c];
		
	}
	
	using RuntimeSignature_t = std::bitset<This_t::numComponents>;

	template<typename T>
	static RuntimeSignature_t generateRuntimeSignature(T signature)
	{

		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));

		RuntimeSignature_t ret;

		boost::hana::for_each(signature, [&ret](auto type)
			{
				ret[decltype(getComponentID(type))::value] = true;
			}
		);

		return ret;
	}

	template<typename T, typename Components>
	auto& newEntity(T signature, Components&& components
		= decltype(components){} /*tuple of the components*/)
	{
		using namespace boost::hana::literals;
		
		entityStorage.emplace_back();
		const size_t newEntityIndex = entityStorage.size() - 1;
		Entity<This_t>& newEntityRef = entityStorage[newEntityIndex];
		newEntityRef.signature = generateRuntimeSignature(signature);
		newEntityRef.ID = newEntityIndex;
		newEntityRef.bases[boost::hana::size(newEntityRef.bases) - boost::hana::size_c<1>] = &newEntityRef;
		newEntityRef.destroy = [this, ID = newEntityRef.ID, signature]() 
			{
				// delete components
				boost::hana::for_each(isolateStorageComponents(signature), [this, ID](auto componentToDestroy)
					{
						getComponentStorage(componentToDestroy).erase(ID);
					});
				
				// delete entities
				boost::hana::for_each(entityStorage[ID].bases, [this, ID](auto basePtr)
					{
						constexpr auto baseType = std::remove_pointer_t<decltype(basePtr)>::managerType;
						BOOST_HANA_CONSTANT_CHECK(isManager(baseType));
						
						getRefToManager(baseType).freeEntitySlots.push_back(ID);
					});
			};
		
		// construct the components
		boost::hana::for_each(components, [this, newEntityIndex, &newEntityRef](auto& component) {
			constexpr auto component_type = boost::hana::type_c<std::decay_t<decltype(component)>>;
			BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component_type));
			
			constexpr auto manager_for_component = decltype(getManagerFromComponent(component_type)){};
			constexpr auto manager_id = getManagerID(manager_for_component);
			auto& refToManagerForComponent = getRefToManager(manager_for_component);
			
			auto& ptrToEntity = newEntityRef.bases[manager_id];
			if (ptrToEntity == nullptr)
			{
				refToManagerForComponent.entityStorage.emplace_back();
				size_t baseEntityID = refToManagerForComponent.entityStorage.size() - 1;
				auto& baseEntityRef = refToManagerForComponent.entityStorage[baseEntityID];
				baseEntityRef.ID = baseEntityID;
				baseEntityRef.bases[boost::hana::size(baseEntityRef.bases) - boost::hana::size_c<1>] = &baseEntityRef;
				baseEntityRef.destroy = newEntityRef.destroy;
				
				ptrToEntity = &baseEntityRef;
				
			}
			
			constexpr auto my_component_id = decltype(manager_for_component)::type::getMyStorageComponentID(component_type);
			constexpr auto all_component_id = decltype(manager_for_component)::type::getMyComponentID(component_type);
			
			refToManagerForComponent.storageComponentStorage[my_component_id][ptrToEntity->ID] = std::move(component);
			refToManagerForComponent.componentEntityStorage[decltype(all_component_id)::value].push_back(ptrToEntity->ID);			
		});
		
		return newEntityRef;
		
	}

	template<typename...Args>
	using TupleOfVectorRefrences = std::tuple<std::vector<Args>&...>;
	
	// returns the elements created [first, last)
	template<typename T, typename Components>
	std::pair<size_t, size_t> createEntityBatch(T signature, Components components, size_t numToConstruct)
	{
		// TODO: implement
	}
	void destroyEntity(Entity<This_t>* handle)
	{
		handle->destroy();
	}

	template<typename T>
	auto getStorageComponent(T component, Entity<This_t>* handle) -> typename decltype(component)::type&
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		constexpr auto managerForComponent = decltype(getManagerFromComponent(component)){};

		constexpr auto staticID = decltype(managerForComponent)::type::template getMyStorageComponentID(component);
		
		return getRefToManager(managerForComponent).storageComponentStorage[staticID][handle->ID];
		
	}

	template<typename T>
	bool hasComponent(T component, Entity<This_t>* entity)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		constexpr auto managerForComponent = decltype(getManagerFromComponent(component)){};

		auto ent = getEntityPtr(managerForComponent, entity);

		return ent->signature[decltype(decltype(managerForComponent)::type::template getComponentID(component))::value];
	}
	

	
	template<typename T>
	static auto getEntityPtr(T managerToGet, Entity<This_t>* ent) -> Entity<typename decltype(managerToGet)::type>*
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(managerToGet));
		
		return ent->bases[getManagerID(managerToGet)];
	}
	
	template<typename T>
	auto getRefToManager(T manager) -> typename decltype(manager)::type&
	{
		BOOST_HANA_CONSTANT_CHECK(isManager(manager));
		
		return *basePtrStorage[getManagerID(manager)];
	}
	
	
	template<typename T>
	auto getComponentStorage(T component) -> SegmentedMap<size_t, typename decltype(component)::type>&
	{
		BOOST_HANA_CONSTANT_CHECK(isStorageComponent(component));
		
		constexpr auto manager = decltype(getManagerFromComponent(component)){};
		
		const constexpr auto ID = decltype(manager)::type::template getMyStorageComponentID(component);
		
		return getRefToManager(manager).storageComponentStorage[ID];
		
	}


	template<typename T>
	std::vector<Entity<This_t>*>& getComponentEntityStorage(T component)
	{
		BOOST_HANA_CONSTANT_CHECK(isComponent(component));
		
		static constexpr auto manager = getManagerFromComponent(component);
		
		const constexpr auto ID = decltype(manager)::type::template getMyComponentID(component);
		
		return getRefToManager(manager).componentEntityStorage[ID];
	}

public:

	// CALLING FUNCTIONS ON ENTITIES
	template<typename T, typename F>
	void callFunctionWithSigParams(Entity<This_t>* ent, T signature, F&& func)
	{
		// get components and put them in a tuple
		
		auto components = boost::hana::fold(signature, boost::hana::make_tuple(), [this, ent](auto retTuple, auto nextType) -> decltype(auto)
			{
				BOOST_HANA_CONSTANT_CHECK(isStorageComponent(nextType));
				return boost::hana::append(retTuple, getStorageComponent(nextType, ent));
			});
		
		// expand
		boost::hana::unpack(components, std::forward<F>(func));
	}

	template<typename T, typename F>
	void runAllMatching(T signature, F&& functor)
	{
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));
		
		static constexpr auto manager = decltype(findMostBaseManagerForSignature(signature)){};

		getRefToManager(manager).runAllMatchingIMPL(signature, std::forward<F>(functor));
	}

	template<typename T, typename F>
	void runAllMatchingIMPL(T signature, F&& functor)
	{
		using namespace boost::hana::literals;
		
		BOOST_HANA_CONSTANT_CHECK(isSignature(signature));
		
		// TODO: use shortest
		
		auto& entityVector = entityStorage;
		
		const auto runtimeSig = generateRuntimeSignature(signature);
		constexpr auto sigStorageCompsOnly = decltype(isolateStorageComponents(signature)){};
		
		for(Entity<This_t>& entity : entityVector)
		{
			if((runtimeSig & entity.signature) == runtimeSig) 
			{
				callFunctionWithSigParams(&entity, sigStorageCompsOnly, std::forward<F>(functor));
			}
		}
	}

	ManagerData<This_t> myManagerData;

	// storage for the actual components
	decltype(boost::hana::transform(myStorageComponents, detail::lambdas::removeTypeAddSegmentedMap)) storageComponentStorage;
	std::array<std::vector<size_t>, This_t::numMyComponents> componentEntityStorage;
	decltype(boost::hana::transform(allManagers, detail::lambdas::removeTypeAddPtr)) basePtrStorage;
	std::vector<Entity<This_t>> entityStorage;
	std::deque<size_t> freeEntitySlots;
	
	bool hasBegunPlay = false;
	bool hasBeenCleandUp = false;
	
	size_t tickNumber = 0;
	
	ManagerData<This_t>& getManagerData()
	{
		return myManagerData;
	}


	Manager(const decltype(boost::hana::transform(myBases, detail::lambdas::removeTypeAddPtr))& bases = decltype(bases){})
	{
		using namespace boost::hana::literals;
		
		assert(boost::hana::all_of(bases, [](auto ptr){ return ptr != 0; }));
		
		auto tempBases = boost::hana::remove_at(basePtrStorage, boost::hana::size(basePtrStorage) - boost::hana::size_c<1>);
		
		boost::hana::for_each(tempBases, [&bases](auto& baseToSet)
			{
				
				auto constexpr managerType = boost::hana::type_c<std::remove_pointer_t<std::decay_t<decltype(baseToSet)>>>;
				BOOST_HANA_CONSTANT_CHECK(isManager(managerType));
				
				auto hasBase = [&managerType](auto typeToCheck)
				{
					return decltype(typeToCheck)::type::isManager(managerType);
				};
				
				constexpr auto directBaseThatHasPtr = boost::hana::find_if(myBases, hasBase);
				BOOST_HANA_CONSTANT_CHECK(boost::hana::is_just(directBaseThatHasPtr));
				
				baseToSet = &bases[getBaseID(*directBaseThatHasPtr)]->getRefToManager(managerType);
				return directBaseThatHasPtr;
			});
		
		
		basePtrStorage = boost::hana::append(tempBases, this);		
		
	}

	~Manager()
	{
		// TODO: add callbacks
	}
};
