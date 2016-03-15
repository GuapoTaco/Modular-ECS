#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/compressed_pair.hpp>
#include <boost/optional.hpp>

#include <exception>
#include <memory>
#include <vector>
#include <initializer_list>

// A map-like container (drop-in replacement so long `Key` is integral) that stores elements by
// segmenting it.
// Requiremts: Key must be integral
// Strengths: Very large amount of data, that is not very sparse, as in the elements are in groups.
// Memory usage: O(N)
// Concepts: satsifies AssociativeContainer
// (http://en.cppreference.com/w/cpp/concept/AssociativeContainer) fully
// Structure:
//                     _________________________________________________________________________
//                    |                                     |                                   |
// storage ---------->| 0|std::array<Value, segment_size>*  |             i|...                 |
//                    |_____________________________________|___________________________________|
//                                  |  |                                          |  |
//                                  |  |                                          |  |
//                                 \    /                                        \    /
//                                  \  /                                          \  /
//                                   \/                                            \/
//                               __________                                    __________
//                              |          |                                  |          |
//  index: 0 * segment_size + 0 | 0|Value  |      index: i * segment_size + 0 | 0|Value  |
//                              |__________|                                  |__________|
//                              |          |                                  |          |
//         0 * segment_size + 1 | 1|Value  |             i * segment_size + 1 | 1|Value  |
//                              |__________|                                  |__________|
//                              |          |                                  |          |
//         0 * segment_size + j | j|....   |             i * segment_size + k | j|....   |
//                              |__________|                                  |__________|
//
template <typename Key, typename Value, typename Compare = std::less<Key>,
		  typename Alloc = std::allocator<std::pair<Key, Value>>>
class segmented_map
{
public:
	static_assert(std::is_integral<Key>::value, "Must be integral");

	// the size of the segments
	static constexpr size_t segment_size = 64 * 4 / sizeof(Value);

	///////////
	// TYPEDEFS
	///////////

	// Container (http://en.cppreference.com/w/cpp/concept/Container) typedefs
	using value_type = std::pair<Key, Value>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using difference_type = ptrdiff_t;
	using size_type = size_t;

	// AssociativeContainer (http://en.cppreference.com/w/cpp/concept/AssociativeContainer) typedefs
	using key_type = Key;
	using key_compare = Compare;
	using value_compare = Compare;

	///////////////
	// CONSTRUCTORS
	///////////////

	// Default Constructor + Construct from the allocator
	segmented_map(const key_compare& comp_ = key_compare{}) : comp{comp_} {}
	// Range constructor (with or without compare)
	template <typename ForwardIterator>

	segmented_map(ForwardIterator begin, ForwardIterator end,
				 const key_compare& comp_ = key_compare{})
		: comp{comp_}
	{
		for (; begin != end; ++begin)
			{
				operator[](begin->first) = begin->second;
			}
	}

	// copy constructor
	segmented_map(const segmented_map& other) = default;

	// move constructor
	segmented_map(segmented_map&& other) = default;

	// initializer_list constuctor
	segmented_map(std::initializer_list<value_type> il) : segmented_map{il.begin(), il.end()} {}
	/////////////
	// DESTRUCTOR
	~segmented_map() = default;
	/////////////

	////////////
	// OPERATORS
	////////////

	// copy assignment operator
	segmented_map& operator=(const segmented_map& other)
	{
		// alloc_and_storage.second() = other.alloc_and_storage.second();
		return *this;
	}

	// move assignment operator
	segmented_map& operator=(segmented_map&& other)
	{
		// alloc_and_storage.second() = std::move(other.alloc_and_storage.second());
		return *this;
	}

	// initializer_list assignment operator
	segmented_map& operator=(std::initializer_list<value_type> il)
	{
		//*this = segmented_map{il};
		return *this;
	}

	// comparision operators
	bool operator==(const segmented_map& other);
	bool operator!=(const segmented_map& other);

	////////////
	// ITERATORS
	////////////

	struct const_iterator : boost::iterator_facade<const_iterator, const value_type,
												   boost::bidirectional_traversal_tag>
	{
		size_t index;
		const segmented_map* owning_container;

		bool is_valid()
		{
			return owning_container->alloc_and_storage.second()[index / segment_size] &&
				   (*owning_container->alloc_and_storage
						 .second()[index / segment_size])[index % segment_size];
		}

		const_reference dereference()
		{
			return **(owning_container->alloc_and_storage
						  .second()[index / segment_size])[index % segment_size];
		}

		bool equal(const_iterator other)
		{
			return index == other.index && owning_container == other.owning_container;
		}

		void increment()
		{
			do
				{
					++index;
				}
			while (!*(owning_container->alloc_and_storage
						  .second()[index / segment_size])[index % segment_size]);
		}

		void decrement()
		{
			std::array<boost::optional<Value>, segment_size>* arrPtr = nullptr;
			do
				{
					do
						{
						}
					while (
						(arrPtr =
							 (owning_container->alloc_and_storage.second()[index / segment_size])));
				}
			while (!(*arrPtr)[index % segment_size]);
		}

		void advance(difference_type dist)
		{
			for (; dist != 0; --dist)
				{
					decrement();
				}
		}

		difference_type distance_to(const_iterator i)
		{
			iterator cpy = *this;

			difference_type ret;
			while (cpy != i)
				{
					++ret;
					cpy.advance();
				}

			return ret;
		}
	};
	friend const_iterator;

	struct iterator
		: boost::iterator_facade<iterator, value_type, boost::bidirectional_traversal_tag>
	{
		size_t index;
		segmented_map* owning_container;

		bool is_valid()
		{
			return owning_container->alloc_and_storage.second()[index / segment_size] &&
				   (*owning_container->alloc_and_storage
						 .second()[index / segment_size])[index % segment_size];
		}

		reference dereference()
		{
			return **(owning_container->alloc_and_storage
						  .second()[index / segment_size])[index % segment_size];
		}

		bool equal(iterator other)
		{
			return index == other.index && owning_container == other.owning_container;
		}

		void increment()
		{
			do
				{
					++index;
				}
			while (!*(owning_container->alloc_and_storage
						  .second()[index / segment_size])[index % segment_size]);
		}

		void decrement()
		{
			std::array<boost::optional<Value>, segment_size>* arrPtr = nullptr;
			do
				{
					do
						{
						}
					while (
						(arrPtr =
							 (owning_container->alloc_and_storage.second()[index / segment_size])));
				}
			while (!(*arrPtr)[index % segment_size]);
		}

		void advance(difference_type dist)
		{
			for (; dist != 0; --dist)
				{
					decrement();
				}
		}

		difference_type distance_to(iterator i)
		{
			iterator cpy = *this;

			difference_type ret;
			while (cpy != i)
				{
					++ret;
					cpy.advance();
				}

			return ret;
		}

		operator const_iterator() { return {index, owning_container}; }
	};
	friend iterator;

	//////////////////
	// ITERATOR ACCESS
	//////////////////

	iterator begin()
	{
		auto vec_it = alloc_and_storage.second().begin();
		while (!*vec_it) ++vec_it;

		auto arr_it = 0;
		while (!(*vec_it)[arr_it]) ++arr_it;

		return {std::distance(alloc_and_storage.second().begin(), vec_it) * segment_size + arr_it,
				this};
	}
	iterator end()
	{
		auto vec_it = alloc_and_storage.second().end();
		while (!*vec_it) --vec_it;

		auto arr_it = segment_size - 1;
		while (!(*vec_it)[arr_it]) --arr_it;

		return {std::distance(alloc_and_storage.second().begin(), vec_it) * segment_size + arr_it,
				this};
	}

	const_iterator begin() const
	{
		auto vec_it = alloc_and_storage.second().begin();
		while (!*vec_it) ++vec_it;

		auto arr_it = 0;
		while (!(*vec_it)[arr_it]) ++arr_it;

		return {std::distance(alloc_and_storage.second().begin(), vec_it) * segment_size + arr_it,
				this};
	}
	const_iterator end() const
	{
		auto vec_it = alloc_and_storage.second().end();
		while (!*vec_it) --vec_it;

		auto arr_it = segment_size - 1;
		while (!(*vec_it)[arr_it]) --arr_it;

		return {std::distance(alloc_and_storage.second().begin(), vec_it) * segment_size + arr_it,
				this};
	}

	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }
	///////////////////////
	// ACCESS AND INSERTION
	///////////////////////

	// [] with bounds checking (throws std::out_of_range if no such `key` exists)
	reference at(const key_type& key)
	{
		// check that the size is good
		if (alloc_and_storage.second().size() >= key / segment_size ||
			alloc_and_storage.second()[key / segment_size])
			{
				throw std::out_of_range("Out of range in segmented_map");
			}
		return *(alloc_and_storage.second()[key / segment_size])[key % segment_size];
	}
	const_reference at(const key_type& key) const
	{
		// check that the size is good
		if (alloc_and_storage.second().size() >= key / segment_size ||
			alloc_and_storage.second()[key / segment_size])
			{
				throw std::out_of_range("Out of range in segmented_map");
			}
		return *(alloc_and_storage.second()[key / segment_size])[key % segment_size];
	}

	reference operator[](const key_type& key)
	{
		return *(alloc_and_storage.second()[key / segment_size])[key % segment_size];
	}
	const_reference operator[](const key_type& key) const
	{
		return *(alloc_and_storage.second()[key / segment_size])[key % segment_size];
	}

	// deletes all the elements
	void clear() { alloc_and_storage.second().clear(); }
	// insertion
	std::pair<iterator, bool> insert(const value_type& value)
	{
		size_t segment_id = value.first / segment_size;
		size_t id_in_segment = value.first % segment_size;

		// see if we need to allocate more on our vector
		if (alloc_and_storage.second().size() >= segment_id)
			{
				alloc_and_storage.second().resize(segment_id + 1);
			}

		auto&& arrayPtr = alloc_and_storage.second()[segment_id];
		// see if we need to allocate a new array
		if (!arrayPtr)
			{
				arrayPtr = std::make_unique<internal_array_type>();
			}

		// insert the element
		if ((*arrayPtr)[id_in_segment])
			{
				return {{value.first, this}, false};
			}
		(*arrayPtr)[id_in_segment] = value.second;
		return {{value.first, this}, true};
	}
	template <typename P,
			  typename = std::enable_if_t<std::is_constructible<value_type, P&&>::value>>
	std::pair<iterator, bool> insert(P&& value)
	{
		return emplace(std::forward<P>(value));
	}
	template <typename P>
	iterator insert(const_iterator hint, P&& value)
	{
		return emplace_hint(hint, std::forward<P>(value));
	}
	template <typename ForwardIterator>
	void insert(ForwardIterator begin, ForwardIterator end)
	{
		for (; begin != end; ++begin)
			{
				insert(*begin);
			}
	}
	void insert(std::initializer_list<value_type> il)
	{
		auto iter = il.begin();
		for (; iter != il.end(); ++iter)
			{
				insert(*iter);
			}
	}

	// insert or assign if it already exists.
	template <typename M>
	std::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj)
	{
		size_t segment_id = k / segment_size;
		size_t id_in_segment = k % segment_size;

		// see if we need to allocate more on our vector
		if (alloc_and_storage.second().size() >= segment_id)
			{
				alloc_and_storage.second().resize(segment_id + 1);
			}

		auto&& arrayPtr = alloc_and_storage.second()[segment_id];
		// see if we need to allocate a new array
		if (!arrayPtr)
			{
				arrayPtr = new internal_array_type();
			}

		(*arrayPtr)[id_in_segment] = obj;
		// insert the element
		if ((*arrayPtr)[id_in_segment])
			{
				return {{k, this}, false};
			}
		return {{k, this}, true};
	}
	template <typename M>
	iterator insert_or_assign(const_iterator /*hint*/, const key_type& k, M&& obj)
	{
		insert_or_assign(k, std::forward<M>(obj));
	}

	// perfect forward the construction of the value_type
	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		// TODO: optimize
		return insert(value_type(std::forward<Args>(args)...));
	}

	// perfect forward the construction of the value_type with hint
	template <typename... Args>
	iterator emplace_hint(const_iterator /*hint*/, Args&&... args)
	{
		return emplace(std::forward<Args>(args)...);
	}

	// counts the amount of keys equal to `key`. Either 0 or 1
	size_type count(const key_type& key) const
	{
		size_t segment_id = key / segment_size;

		if (alloc_and_storage().second().size() <= segment_id)
			{
				return 0;
			}

		if (!alloc_and_storage().second()[segment_id])
			{
				return 0;
			}

		if (!(*alloc_and_storage().second()[segment_id])[key % segment_size])
			{
				return 0;
			}

		return 1;
	}

	// gets an iterator with the key `key`, or end()
	iterator find(const key_type& key)
	{
		size_t segment_id = key / segment_size;

		if (alloc_and_storage().second().size() <= segment_id)
			{
				return end();
			}

		if (!alloc_and_storage().second()[segment_id])
			{
				return end();
			}

		if (!(*alloc_and_storage().second()[segment_id])[key % segment_size])
			{
				return end();
			}

		return {key, this};
	}
	const_iterator find(const key_type& key) const
	{
		size_t segment_id = key / segment_size;

		if (alloc_and_storage().second().size() <= segment_id)
			{
				return end();
			}

		if (!alloc_and_storage().second()[segment_id])
			{
				return end();
			}

		if (!(*alloc_and_storage().second()[segment_id])[key % segment_size])
			{
				return end();
			}

		return {key, this};
	}

	// returns a range of the elements in the container matching `key`
	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		auto iter = find(key);

		if (iter != end()) return {iter, iter++};

		return {iter, iter};
	}
	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		auto iter = find(key);

		if (iter != cend()) return {iter, iter++};

		return {iter, iter};
	}
	template <typename K>
	std::pair<iterator, iterator> equal_range(const K& x)
	{
		return equal_range(key_type{x});
	}
	template <typename K>
	std::pair<const_iterator, const_iterator> equal_range(const K& x) const
	{
		return equal_range(key_type{x});
	}

	// return an iterator pointing to the first elemnt not less than `key`
	iterator lower_bound(const key_type& key)
	{
		iterator iter = {key, this};

		if (iter.is_valid())
			return iter;
		else
			return ++iter;
	}
	const_iterator lower_bound(const key_type& key) const
	{
		const_iterator iter = {key, this};

		if (iter.is_valid())
			return iter;
		else
			return ++iter;
	}

	// retuns an iterator pointing to the first element greater than `key`
	iterator upper_bound(const key_type& key)
	{
		auto ret = iterator{key, this};
		ret.increment();

		return ret;
	}
	const_iterator upper_bound(const key_type& key) const
	{
		auto ret = const_iterator{key, this};
		ret.increment();

		return ret;
	}

	// erases elements
	iterator erase(const_iterator pos);
	iterator erase(const_iterator first, const_iterator last);
	size_type erase(const key_type& key);

	//////////////////
	// OTHER FUNCTIONS
	//////////////////

	void swap(const segmented_map& other)
	{
		return std::swap(alloc_and_storage, other.alloc_and_storage);
	}

	// size functions
	// WARNING: this is slow
	size_type size() const
	{
		auto it = const_iterator{0, this};

		size_t count = 0;
		if (it.is_valid())
			{
				count = 1;
			}

		while (it != end())
			{
				++count;
				++it;
			}

		return count;
	}
	size_type max_size() const { return alloc_and_storage().second().max_size() * segment_size; }
	bool empty() const { return alloc_and_storage().second().empty(); }
	Alloc& get_allocator() { return alloc_and_storage.first(); }
	key_compare key_comp() { return comp; }
	value_compare value_comp() { return comp; }
private:
	using internal_array_type = std::array<boost::optional<Value>, segment_size>;
	boost::compressed_pair<Alloc, std::vector<internal_array_type*>> alloc_and_storage;

	key_compare comp;
};
