#ifndef _KOUKAN_VECTOR_HPP_
#define _KOUKAN_VECTOR_HPP_

#include <type_traits>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace koukan
{

template <typename T, int Increment = 1>
class Vector_iterator
{
public:
	Vector_iterator() = default;
	Vector_iterator(Vector_iterator const& other) = default;
	Vector_iterator(Vector_iterator&& other) = default;
	Vector_iterator(T* current);
	~Vector_iterator() = default;
	Vector_iterator&		operator=(Vector_iterator const& other) = default;
	Vector_iterator&		operator=(Vector_iterator&& other) = default;
	Vector_iterator&		operator++() noexcept;
	Vector_iterator			operator++(int);
	bool					operator==(Vector_iterator const& other) const;
	bool					operator!=(Vector_iterator const& other) const;
	T*						operator->() const;
	T&						operator*() const;

	T*	current = nullptr;
};

template <typename T, int CapacityTick = 16>
class Vector
{
public:
	using value_type = T;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = Vector_iterator<T, 1>;
	using const_iterator = Vector_iterator<T const, 1>;
	using reverse_iterator = Vector_iterator<T, -1>;
	using const_reverse_iterator = Vector_iterator<T const, -1>;

	Vector() = default;
	Vector(Vector const &other);
	Vector(Vector &&other);
	~Vector();
	Vector&					operator=(Vector const& other);
	Vector&					operator=(Vector&& other);

	T&						operator[](size_t pos) noexcept;
	T const&				operator[](size_t pos) const noexcept;
	T&						at(size_t pos);
	T const&				at(size_t pos) const;
	T&						front() noexcept;
	T const&				front() const noexcept;
	T&						back() noexcept;
	T const&				back() const noexcept;

	void					push_back(T const& element);
	template <typename... Args>
	void					emplace_back(Args&&... args);
	void					pop_back();
	inline iterator			erase(iterator const& it);

	void					clear();
	void					resize(size_t size);
	void					reserve(size_t size);

	size_t					size() const noexcept;
	bool					empty() const noexcept;
	T const*                data() const noexcept;

	iterator				begin();
	const_iterator			begin() const;
	const_iterator			cbegin() const;
	reverse_iterator		rbegin();
	const_reverse_iterator	rbegin() const;
	const_reverse_iterator	crbegin() const;
	iterator				end();
	const_iterator			end() const;
	const_iterator			cend() const;
	reverse_iterator		rend();
	const_reverse_iterator	rend() const;
	const_reverse_iterator	crend() const;

private:
	void					_allocate();
	void					_deallocate();

	template <typename A = T>
	typename std::enable_if<std::is_pod<A>::value, void>::type _allocate(size_t size);
	template <typename A = T>
	typename std::enable_if<!std::is_pod<A>::value, void>::type _allocate(size_t size);

	template <typename A = T>
	typename std::enable_if<std::is_pod<A>::value, void>::type _copy(Vector const& other);
	template <typename A = T>
	typename std::enable_if<!std::is_pod<A>::value, void>::type _copy(Vector const& other);

	template <typename A = T>
	typename std::enable_if<std::is_pod<A>::value, void>::type _erase(size_t idx);
	template <typename A = T>
	typename std::enable_if<!std::is_pod<A>::value, void>::type	_erase(size_t idx);

	template <typename A = T>
	typename std::enable_if<std::is_pod<A>::value, void>::type _clear();
	template <typename A = T>
	typename std::enable_if<!std::is_pod<A>::value, void>::type _clear();

	size_t	_capacity = 0;
	size_t	_size = 0;
	T*		_array = nullptr;
};



// Definitions
// Iterator
template <typename T, int Increment>
Vector_iterator<T, Increment>::Vector_iterator(T* current) : current(current)
{
}

template <typename T, int Increment>
Vector_iterator<T, Increment>& Vector_iterator<T, Increment>::operator++() noexcept
{
	current += Increment;
	return *this;
}

template <typename T, int Increment>
Vector_iterator<T, Increment> Vector_iterator<T, Increment>::operator++(int)
{
	Vector_iterator other(*this);
	current += Increment;
	return other;
}

template <typename T, int Increment>
bool Vector_iterator<T, Increment>::operator==(Vector_iterator const& other) const
{
	return current == other.current;
}

template <typename T, int Increment>
bool Vector_iterator<T, Increment>::operator!=(Vector_iterator const& other) const
{
	return !(*this == other);
}

template <typename T, int Increment>
T* Vector_iterator<T, Increment>::operator->() const
{
	return current;
}

template <typename T, int Increment>
T& Vector_iterator<T, Increment>::operator*() const
{
	return *current;
}

// Vector
template <typename T, int CapacityTick>
Vector<T, CapacityTick>::Vector(Vector const &other)
	: _capacity(other._capacity), _size(other._size), _array(reinterpret_cast<T*>(std::malloc(sizeof(T) * other._capacity)))
{
	this->_copy(other);
}

template <typename T, int CapacityTick>
Vector<T, CapacityTick>::Vector(Vector&& other)
	: _capacity(other._capacity), _size(other._size), _array(other._array)
{
	other._capacity = 0;
	other._size = 0;
	other._array = nullptr;
}

template <typename T, int CapacityTick>
Vector<T, CapacityTick>::~Vector()
{
	this->_clear();
	std::free(_array);
}

template <typename T, int CapacityTick>
Vector<T, CapacityTick>& Vector<T, CapacityTick>::operator=(Vector const& other)
{
	this->_clear();
	if (_capacity != other._capacity)
	{
		_capacity = other._capacity;
		std::free(_array);
		_array = reinterpret_cast<T*>(std::malloc(sizeof(T) * _capacity));
	}
	_size = other._size;
	std::memcpy(_array, other._array, _size * sizeof(T));
	return *this;
}

template <typename T, int CapacityTick>
Vector<T, CapacityTick>& Vector<T, CapacityTick>::operator=(Vector&& other)
{
	if (_array)
	{
		this->_clear();
		std::free(_array);
	}

	_capacity = other._capacity;
	_size = other._size;
	_array = other._array;
	other._capacity = 0;
	other._size = 0;
	other._array = nullptr;
	return *this;
}

template <typename T, int CapacityTick>
T& Vector<T, CapacityTick>::operator[](size_t pos) noexcept
{
	return _array[pos];
}

template <typename T, int CapacityTick>
T const& Vector<T, CapacityTick>::operator[](size_t pos) const noexcept
{
	return _array[pos];
}

template <typename T, int CapacityTick>
T& Vector<T, CapacityTick>::at(size_t pos)
{
	if (pos >= _size)
		throw std::out_of_range("koukan::Vector : out of range");
	return _array[pos];
}

template <typename T, int CapacityTick>
T const& Vector<T, CapacityTick>::at(size_t pos) const
{
	if (pos >= _size)
		throw std::out_of_range("koukan::Vector : out of range");
	return _array[pos];
}

template <typename T, int CapacityTick>
T& Vector<T, CapacityTick>::front() noexcept
{
	return *_array;
}

template <typename T, int CapacityTick>
T const& Vector<T, CapacityTick>::front() const noexcept
{
	return *_array;
}

template <typename T, int CapacityTick>
T& Vector<T, CapacityTick>::back() noexcept
{
	return _array[_size - 1];
}

template <typename T, int CapacityTick>
T const& Vector<T, CapacityTick>::back() const noexcept
{
	return _array[_size - 1];
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::push_back(T const& element)
{
	if (_capacity == _size)
		this->_allocate();
	new (&_array[_size]) T(element);
	++_size;
}

template <typename T, int CapacityTick>
template <typename... Args>
void Vector<T, CapacityTick>::emplace_back(Args&&... args)
{
	if (_capacity == _size)
		this->_allocate();
	new (&_array[_size]) T(std::forward<Args>(args)...);
	++_size;
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::pop_back()
{
	this->_erase(_size - 1);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::iterator Vector<T, CapacityTick>::erase(iterator const& it)
{
	size_t idx = it.current - _array;
	this->_erase(idx);
	return iterator(&_array[idx]);
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::clear()
{
	this->_clear();
	_size = 0;
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::resize(size_t size)
{
	this->reserve(size);
	_size = size;
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::reserve(size_t size)
{
	if (size < _capacity)
		return;
	if (size % CapacityTick)
		size = size + CapacityTick - (size % CapacityTick);
	this->_allocate(size);
}

template <typename T, int CapacityTick>
size_t Vector<T, CapacityTick>::size() const noexcept
{
	return _size;
}

template <typename T, int CapacityTick>
bool Vector<T, CapacityTick>::empty() const noexcept
{
	return _size == 0;
}

template <typename T, int CapacityTick>
T const* Vector<T, CapacityTick>::data() const noexcept
{
	return _array;
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::iterator Vector<T, CapacityTick>::begin()
{
	return iterator(_array);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_iterator Vector<T, CapacityTick>::begin() const
{
	return const_iterator(_array);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_iterator Vector<T, CapacityTick>::cbegin() const
{
	return const_iterator(_array);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::reverse_iterator Vector<T, CapacityTick>::rbegin()
{
	return reverse_iterator(&_array[_size - 1]);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_reverse_iterator Vector<T, CapacityTick>::rbegin() const
{
	return const_reverse_iterator(&_array[_size - 1]);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_reverse_iterator Vector<T, CapacityTick>::crbegin() const
{
	return const_reverse_iterator(&_array[_size - 1]);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::iterator Vector<T, CapacityTick>::end()
{
	return iterator(&_array[_size]);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_iterator Vector<T, CapacityTick>::end() const
{
	return const_iterator(&_array[_size]);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_iterator Vector<T, CapacityTick>::cend() const
{
	return const_iterator(&_array[_size]);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::reverse_iterator Vector<T, CapacityTick>::rend()
{
	return reverse_iterator(_array - 1);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_reverse_iterator Vector<T, CapacityTick>::rend() const
{
	return const_ireverse_iterator(_array - 1);
}

template <typename T, int CapacityTick>
typename Vector<T, CapacityTick>::const_reverse_iterator Vector<T, CapacityTick>::crend() const
{
	return const_reverse_iterator(_array - 1);
}


// Private
template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<std::is_pod<A>::value, void>::type Vector<T, CapacityTick>::_allocate(size_t size)
{
	_capacity = size;
	_array = reinterpret_cast<T*>(std::realloc(_array, sizeof(T) * _capacity));
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<!std::is_pod<A>::value, void>::type Vector<T, CapacityTick>::_allocate(size_t size)
{
	_capacity = size;
	auto tmp = reinterpret_cast<T*>(std::malloc(sizeof(T) * _capacity));
	for (size_t i = 0; i < _size; ++i)
		new (&tmp[i]) T(std::move(_array[i]));
	std::free(_array);
	_array = tmp;
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::_allocate()
{
	this->_allocate(_capacity + CapacityTick);
}

template <typename T, int CapacityTick>
void Vector<T, CapacityTick>::_deallocate()
{
	this->_allocate(_capacity - CapacityTick);
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<std::is_pod<A>::value, void>::type Vector<T, CapacityTick>::_copy(Vector const& other)
{
	std::memcpy(_array, other._array, sizeof(T) * other._size);
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<!std::is_pod<A>::value, void>::type	Vector<T, CapacityTick>::_copy(Vector const& other)
{
	for (size_t i = 0; i < _size; ++i)
		new (&_array[i]) T(other._array[i]);
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<std::is_pod<A>::value, void>::type Vector<T, CapacityTick>::_erase(size_t idx)
{
	--_size;
	size_t size = _size - idx;

	if (size > 0)
		std::memmove(&_array[idx], &_array[idx + 1], size * sizeof(T));
	if ((_capacity - _size) > CapacityTick)
		this->_deallocate();
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<!std::is_pod<A>::value, void>::type	Vector<T, CapacityTick>::_erase(size_t idx)
{
	_array[idx].~T();
	T*	elem = &_array[idx];
	for (idx = idx + 1; idx < _size; ++idx, ++elem)
		new (elem) T(std::move(_array[idx]));
	--_size;
	if ((_capacity - _size) > CapacityTick)
		this->_deallocate();
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<std::is_pod<A>::value, void>::type Vector<T, CapacityTick>::_clear()
{
}

template <typename T, int CapacityTick>
template <typename A>
typename std::enable_if<!std::is_pod<A>::value, void>::type Vector<T, CapacityTick>::_clear()
{
	for (size_t i = 0; i < _size; ++i)
		_array[i].~T();
}

}

#endif // _KOUKAN_VECTOR_HPP_