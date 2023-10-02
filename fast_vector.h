//
// Based on original work: https://github.com/sigerror/fast-vector
// Copyright (c) Vladislav Lukyanov
//
// Advanced & fixed version by Marian Krivos
//

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring> // std::memcpy()
#include <new>
#include <stdexcept>
#include <type_traits>

// Helper functions

template <class T>
inline void construct_range(T* begin, T* end)
{
    while (begin != end)
    {
        new (begin) T;
        begin++;
    }
}

template <class T>
inline void copy_range(const T* begin, const T* end, T* dest)
{
    while (begin != end)
    {
        new (dest) T(*begin);
        begin++;
        dest++;
    }
}

template <class T>
inline T* find_item(T* begin, T* end, const T& value)
{
    while (begin != end)
    {
        if (*begin == value)
            return begin;
        begin++;
    }
    return end;
}

template <class T>
inline void destruct_range(T* begin, T* end)
{
    while (begin != end)
    {
        begin->~T();
        begin++;
    }
}

/**
 * The fast & light-weight std::vector replacement, best used for plain POD types.
 */
template <class T>
class fast_vector
{
public:
    using size_type = std::size_t;
    using value_type = T;

    fast_vector() = default;
    fast_vector(const fast_vector& other);
    fast_vector(std::initializer_list<T>&& other);
    fast_vector(fast_vector&& other) noexcept;
    fast_vector& operator=(const fast_vector& other);
    fast_vector& operator=(fast_vector&& other) noexcept;
    fast_vector(const T a[], const T b[]);

    ~fast_vector();

    // Element access

    T& operator[](size_type pos);
    const T& operator[](size_type pos) const;

    T& at(size_type pos);
    const T& at(size_type pos) const;

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;

    T* data() noexcept;
    const T* data() const noexcept;

    // Iterators

    T* begin() noexcept;
    const T* begin() const noexcept;

    T* end() noexcept;
    const T* end() const noexcept;

    // Capacity

    bool empty() const noexcept;
    size_type size() const noexcept;
    void reserve(size_type new_cap);
    size_type capacity() const noexcept;
    void shrink_to_fit();

    // Modifiers

    void clear() noexcept;

    void push_back(const T& value);
    void push_back(T&& value);

    template< class... Args >
    void emplace_back(Args&&... args);
    
    void append(const T value[], size_t count);

    void pop_back();
    void resize(size_type count);
    void erase(const T value);

    void swap(fast_vector<T>& a, fast_vector<T>& b);

    static constexpr size_type grow_factor = 2;

private:
    T* m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
};

template <class T>
void fast_vector<T>::swap(fast_vector<T>& a, fast_vector<T>& b)
{
    std::swap(a.m_data, b.m_data);
    std::swap(a.m_size, b.m_size);
    std::swap(a.m_capacity, b.m_capacity);
}

template <class T>
fast_vector<T>::fast_vector(std::initializer_list<T>&& other) :
    fast_vector(other.begin(), other.end())
{
}

template <class T>
fast_vector<T>::fast_vector(const T a[], const T b[])
  : m_size(b - a)
  , m_capacity(b - a)
{
    m_data = reinterpret_cast<T*>(std::malloc(sizeof(T) * m_capacity));

    if (std::is_trivial_v<T>)
    {
        std::memcpy(m_data, a, sizeof(T) * m_capacity);
    }
    else
    {
        copy_range(a, b, m_data);
    }
}

template <class T>
fast_vector<T>::fast_vector(const fast_vector& other)
    : m_size(other.m_size)
    , m_capacity(other.m_size)
{
    m_data = reinterpret_cast<T*>(std::malloc(sizeof(T) * m_size));

    if (std::is_trivial_v<T>)
    {
        std::memcpy(m_data, other.m_data, sizeof(T) * m_size);
    }
    else
    {
        copy_range(other.begin(), other.end(), m_data);
    }
}

template <class T>
fast_vector<T>::fast_vector(fast_vector&& other) noexcept
    : m_data(other.m_data)
    , m_size(other.m_size)
    , m_capacity(other.m_capacity)
{
    other.m_data = nullptr;
}

template <class T>
fast_vector<T>& fast_vector<T>::operator=(const fast_vector& other)
{
    m_size = other.m_size;
    m_capacity = other.m_size;

    m_data = reinterpret_cast<T*>(std::malloc(sizeof(T) * m_size));

    if (std::is_trivial_v<T>)
    {
        std::memcpy(m_data, other.m_data, sizeof(T) * m_size);
    }
    else
    {
        copy_range(other.begin(), other.end(), m_data);
    }

    return *this;
}

template <class T>
fast_vector<T>& fast_vector<T>::operator=(fast_vector&& other) noexcept
{
    m_data = other.m_data;
    m_size = other.m_size;
    m_capacity = other.m_size;

    other.m_data = nullptr;

    return *this;
}

template <class T>
fast_vector<T>::~fast_vector()
{
    if (m_data)
    {
        if (!std::is_trivial_v<T>)
        {
            destruct_range(begin(), end());
        }
        std::free(m_data);
    }
}

// Element access

template <class T>
T& fast_vector<T>::operator[](size_type pos)
{
    assert(pos < m_size && "Position is out of range");
    return m_data[pos];
}

template <class T>
const T& fast_vector<T>::operator[](size_type pos) const
{
    assert(pos < m_size && "Position is out of range");
    return m_data[pos];
}

template <class T>
T& fast_vector<T>::at(size_type pos)
{
    if (pos >= m_size)
        throw std::range_error{"Position is out of range"};

    return operator [](pos);
}

template <class T>
const T& fast_vector<T>::at(size_type pos) const
{
    if (pos >= m_size)
        throw std::range_error{"Position is out of range"};

    return operator [](pos);
}

template <class T>
T& fast_vector<T>::front()
{
    assert(m_size > 0 && "Container is empty");
    return m_data[0];
}

template <class T>
const T& fast_vector<T>::front() const
{
    assert(m_size > 0 && "Container is empty");
    return m_data[0];
}

template <class T>
T& fast_vector<T>::back()
{
    assert(m_size > 0 && "Container is empty");
    return m_data[m_size - 1];
}

template <class T>
const T& fast_vector<T>::back() const
{
    assert(m_size > 0 && "Container is empty");
    return m_data[m_size - 1];
}

template <class T>
T* fast_vector<T>::data() noexcept
{
    return m_data;
}

template <class T>
const T* fast_vector<T>::data() const noexcept
{
    return m_data;
}

// Iterators

template <class T>
T* fast_vector<T>::begin() noexcept
{
    return m_data;
}

template <class T>
const T* fast_vector<T>::begin() const noexcept
{
    return m_data;
}

template <class T>
T* fast_vector<T>::end() noexcept
{
    return m_data + m_size;
}

template <class T>
const T* fast_vector<T>::end() const noexcept
{
    return m_data + m_size;
}

// Capacity

template <class T>
bool fast_vector<T>::empty() const noexcept
{
    return m_size == 0;
}

template <class T>
typename fast_vector<T>::size_type fast_vector<T>::size() const noexcept
{
    return m_size;
}

template <class T>
void fast_vector<T>::reserve(size_type new_cap)
{
    assert(new_cap > m_capacity && "Capacity is already equal to or greater than the passed value");

    if constexpr (std::is_trivial_v<T>)
    {
        m_data = reinterpret_cast<T*>(std::realloc(m_data, sizeof(T) * new_cap));
        assert(m_data != nullptr && "Reallocation failed");
    }
    else
    {
        T* new_data_location = reinterpret_cast<T*>(std::malloc(sizeof(T) * new_cap));
        assert(new_data_location != nullptr && "Allocation failed");

        copy_range(begin(), end(), new_data_location);
        destruct_range(begin(), end());

        std::free(m_data);

        m_data = new_data_location;
    }

    m_capacity = new_cap;
}

template <class T>
typename fast_vector<T>::size_type fast_vector<T>::capacity() const noexcept
{
    return m_capacity;
}

template <class T>
void fast_vector<T>::shrink_to_fit()
{
    if (m_size && m_size < m_capacity)
    {
        if constexpr (std::is_trivial_v<T>)
        {
            m_data = reinterpret_cast<T*>(std::realloc(m_data, sizeof(T) * m_size));
            assert(m_data != nullptr && "Reallocation failed");
        }
        else
        {
            T* new_data_location = reinterpret_cast<T*>(std::malloc(sizeof(T) * m_size));
            assert(new_data_location != nullptr && "Allocation failed");

            copy_range(begin(), end(), new_data_location);
            destruct_range(begin(), end());

            std::free(m_data);

            m_data = new_data_location;
        }
    }
}

// Modifiers

template <class T>
void fast_vector<T>::clear() noexcept
{
    if constexpr (!std::is_trivial_v<T>)
    {
        destruct_range(begin(), end());
    }

    m_size = 0;
}

template <class T>
void fast_vector<T>::append(const T values[], size_t count)
{
    if (m_size + count >= m_capacity)
    {
        for (size_t i = 0; i < count; i++)
        {
            push_back(values[i]);
        }
    }
    else
    {
        if constexpr (std::is_trivial_v<T>)
        {
            std::memcpy(m_data+m_size, values, count*sizeof(T));
        }
        else
        {
            copy_range(values, values + count, m_data + m_size);
        }
        m_size += count;
    }
}

template <class T>
void fast_vector<T>::erase(const T value)
{
    T* position = find_item(begin(), end(), value);
    if (position < end())
    {
        if constexpr (!std::is_trivial_v<T>)
        {
            position->~T();
        }

        if (position < end()-1)
        {
            size_t count = end() - position - 1;
            if constexpr (std::is_trivial_v<T>)
            {
                std::memcpy(position, position + 1, count*sizeof(T));
            }
            else
            {
                copy_range(position, position + 1, count);
            }
        }
        --m_size;
    }
}

template <class T>
void fast_vector<T>::push_back(const T& value)
{
    if (m_size == m_capacity)
    {
        reserve(m_capacity * fast_vector::grow_factor + 1);
    }

    if constexpr (std::is_trivial_v<T>)
    {
        m_data[m_size] = value;
    }
    else
    {
        new (m_data + m_size) T(value);
    }

    m_size++;
}

template <class T>
void fast_vector<T>::push_back(T&& value)
{
    if (m_size == m_capacity)
    {
        reserve(m_capacity * fast_vector::grow_factor + 1);
    }

    if constexpr (std::is_trivial_v<T>)
    {
        m_data[m_size] = value;
    }
    else
    {
        new (m_data + m_size) T(std::move(value));
    }

    m_size++;
}

template <class T>
template< class... Args >
void fast_vector<T>::emplace_back(Args&&... args)
{
    static_assert(!std::is_trivial_v<T>, "Use push_back() instead of emplace_back() with trivial types");

    if (m_size == m_capacity)
    {
        reserve(m_capacity * fast_vector::grow_factor + 1);
    }

    new (m_data + m_size) T(std::forward<Args>(args)...);

    m_size++;
}

template <class T>
void fast_vector<T>::pop_back()
{
    assert(m_size > 0 && "Container is empty");

    if constexpr (!std::is_trivial_v<T>)
    {
        m_data[m_size - 1].~T();
    }

    m_size--;
}

template <class T>
void fast_vector<T>::resize(size_type count)
{
    if (count == m_size)
        return;

    if (count > m_capacity)
    {
        reserve(count);
    }

    if constexpr (!std::is_trivial_v<T>)
    {
        if (count > m_size)
        {
            construct_range(m_data + m_size, m_data + count);
        }
        else if (count < m_size)
        {
            destruct_range(m_data + count, m_data + m_size);
        }
    }

    m_size = count;
}
