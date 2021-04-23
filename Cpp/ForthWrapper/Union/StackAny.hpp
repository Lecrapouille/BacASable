//=====================================================================
// OpenGLCppWrapper: A C++11 OpenGL 'Core' wrapper.
// Copyright 2018-2019 Quentin Quadrat <lecrapouille@gmail.com>
//
// This file is part of OpenGLCppWrapper.
//
// OpenGLCppWrapper is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenGLCppWrapper is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenGLCppWrapper.  If not, see <http://www.gnu.org/licenses/>.
//=====================================================================
//
// This file is a derived work of
// https://gieseanw.wordpress.com/2017/05/03/a-true-heterogeneous-container-in-c/
//=====================================================================

#ifndef CONTAINER_ANY_HPP
#  define CONTAINER_ANY_HPP

#  include <unordered_map>>
#  include <vector>
#  include <string>
#  include <functional>
#  include <experimental/type_traits>

// *****************************************************************************
//! \brief Stack class holding elements of type T.
//! \note: The stack has fixed size and therefore no reallocations are made.
//! \tparam T Can be Cell, int, smart pointers, ...
// *****************************************************************************
template<typename T>
class Stack
{
public:

    //--------------------------------------------------------------------------
    //! \brief Canari: unused extra memory for detecting stack overflow or
    //! underflow.
    //--------------------------------------------------------------------------
    static constexpr size_t security_margin = 8;

    //--------------------------------------------------------------------------
    //! \brief Constructor. Initialize an empty stack. The name passed as param
    //! is used for error messages and logs.
    //! \param[in] name_ the name of the stack (debug purpose only).
    //--------------------------------------------------------------------------
    Stack(const char *name_)
        : sp(sp0), m_name(name_)
    {}

    //--------------------------------------------------------------------------
    //! \brief Reset the stack to initial states.
    //! The top of Stack (TOS) is restored and the stack has no data and its
    //! depth is 0.
    //--------------------------------------------------------------------------
    INLINE void reset() { sp = sp0; } // TODO zeros(m_data, sp0 - m_data);

    //--------------------------------------------------------------------------
    //! \brief Return the current depth of the stack.
    //--------------------------------------------------------------------------
    INLINE int32_t depth() const { return int32_t(sp - sp0); }

    //--------------------------------------------------------------------------
    //! \brief Push an element which will be on the top of the stack.
    //! \note this routine does not check against stack overflow.
    //--------------------------------------------------------------------------
    INLINE void push(T const& n) { *(sp++) = n; }

    //--------------------------------------------------------------------------
    //! \brief Push an element which will be on the top of the stack.
    //! This method is specialized for smart pointer and it is used for ie for
    //! memorizing included Forth files (INCLUDE word).
    //! \note this routine does not check against stack overflow.
    //--------------------------------------------------------------------------
    template<typename N>
    INLINE void push(std::unique_ptr<N> n) { *(sp++) = std::move(n); }

    //--------------------------------------------------------------------------
    //! \brief Remove the element place on the top of stack.
    //! \note this routine does not check against stack underflow.
    //--------------------------------------------------------------------------
    INLINE void drop() { --sp; }

    //--------------------------------------------------------------------------
    //! \brief Duplicate the top of stack.
    //! \note this routine does not check against stack overflow.
    //--------------------------------------------------------------------------
    INLINE void dup() { *(sp) = *(sp - 1); ++sp; }

    //--------------------------------------------------------------------------
    //! \brief Consum the top of stack.
    //! \note this routine does not check against stack underflow.
    //--------------------------------------------------------------------------
    INLINE T& pop() { return *(--sp); }

    //--------------------------------------------------------------------------
    //! \brief Access to the Nth element of stack from its top.
    //! \note this routine does not check against bad index access.
    //! \param nth the nth param to access (0 is TOS, 1 is the second element).
    //--------------------------------------------------------------------------
    INLINE T& pick(int const nth) { return *(sp - nth - 1); }

    //--------------------------------------------------------------------------
    //! \brief Access to the Nth element of stack from its top.
    //! \note this routine does not check against bad index access.
    //! \param nth the nth param to access (0 is TOS, 1 is the second element).
    //--------------------------------------------------------------------------
    INLINE T const& pick(int const nth) const { return *(sp - nth - 1); }

    //--------------------------------------------------------------------------
    //! \brief Access to the top of the stack (TOS).
    //! \note this routine does not check if the stack is empty.
    //--------------------------------------------------------------------------
    INLINE T& tos() { return *(sp - 1); }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack is enough deep.
    //! \param depth expected minimal stack depth.
    //! \return true if the stack depth is deeper than the param depth.
    //--------------------------------------------------------------------------
    INLINE bool hasDepth(int32_t const depth) const
    {
        return this->depth() >= depth;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack has overflowed.
    //! \return true if the stack has overflowed.
    //--------------------------------------------------------------------------
    INLINE bool hasOverflowed() const
    {
        return sp > spM;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the stack has underflowed.
    //! \return true if the stack has underflowed.
    //--------------------------------------------------------------------------
    INLINE bool hasUnderflowed() const
    {
        return sp < sp0;
    }

    //--------------------------------------------------------------------------
    //! \brief Display the content of a Forth stack. Elements are displayed
    //! in the given number base (initialy decimal).
    //--------------------------------------------------------------------------
    std::ostream& display(std::ostream& os, int base = 10)
    {
        os << m_name;
        if (hasOverflowed())
            os << "<overflowed>" << std::endl;
        else if (hasUnderflowed())
            os << "<underflowed>" << std::endl;
        else
        {
            // Display the stack depth
            std::ios_base::fmtflags ifs(os.flags());
            os << '<' << std::setbase(10) << base
               << ':' << std::setbase(10) << depth()
               << '>';

            T* s = sp0;
            while (s != sp)
                os << ' ' << std::setbase(base) << static_cast<T>(*s++);

            os << std::endl;
            os.flags(ifs);
        }
        os << std::dec;
        return os;
    }

    //--------------------------------------------------------------------------
    //! \brief Call a function on each element of the stack
    //! Example: stack.for_each([](forth::Cell c) { std::cout << c << std::endl; });
    //--------------------------------------------------------------------------
    template<class Fn, typename... Args>
    void for_each(Fn fun, Args&&... args) const
    {
        T* s = sp0;
        while (s != sp)
        {
            fun(*s++, std::forward<Args>(args)...);
        }
    }

    //--------------------------------------------------------------------------
    //! \brief Return the stack pointer.
    //--------------------------------------------------------------------------
    INLINE T*& top()
    {
        return sp;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the stack pointer.
    //--------------------------------------------------------------------------
    INLINE T const*& top() const
    {
        return sp;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the name of the stack.
    //--------------------------------------------------------------------------
    INLINE std::string const& name() const
    {
        return m_name;
    }

protected:

    constexpr size_t stack = 1024u;

    //--------------------------------------------------------------------------
    //! \brief A stack is a fixed-size memory segment of elements.
    //--------------------------------------------------------------------------
    T data[stack_size];

    //--------------------------------------------------------------------------
    //! \brief Initial address of the top of the stack (fix address).
    //--------------------------------------------------------------------------
    T* const sp0 = data + security_margin;
    T* const spM = data + size::stack - security_margin;

    //--------------------------------------------------------------------------
    //! \brief Stack pointer (refers to the top element of the stack).
    //--------------------------------------------------------------------------
    T* sp;

    //--------------------------------------------------------------------------
    //! \brief Name of the stack for logs and c++ exceptions.
    //--------------------------------------------------------------------------
    std::string m_name;
};

// *****************************************************************************
//! \brief Container typed of std::map but for storing heterogeneous elements.
//! Based on original code source:
//! https://gieseanw.wordpress.com/2017/05/03/a-true-heterogeneous-container-in-c/
//! Need C++14 since using variable template
// *****************************************************************************
struct Any
{
public:

    Any() = default;

    //--------------------------------------------------------------------------
    //! \brief Copy constructor.
    //--------------------------------------------------------------------------
    Any(Any const& other)
    {
       *this = other;
    }

    //--------------------------------------------------------------------------
    //! \brief Copy operator.
    //--------------------------------------------------------------------------
    Any& operator=(Any const& other)
    {
        clear();
        m_clear_functions = other.m_clear_functions;
        m_copy_functions = other.m_copy_functions;
        m_size_functions = other.m_size_functions;

        for (auto&& copy_function : m_copy_functions)
        {
            copy_function(other, *this);
        }
        return *this;
    }

    //--------------------------------------------------------------------------
    //! \brief Destructor. Call clear().
    //--------------------------------------------------------------------------
    ~Any()
    {
        clear();
    }

    //--------------------------------------------------------------------------
    //! \brief Clear the whole container. All elements are removed.
    //--------------------------------------------------------------------------
    void clear()
    {
        for (auto&& clear_func : m_clear_functions)
        {
            clear_func(*this);
        }
        m_clear_functions.clear();
        m_copy_functions.clear();
        m_size_functions.clear();
    }

    //--------------------------------------------------------------------------
    //! \brief Return the number of elements of type T.
    //--------------------------------------------------------------------------
    template<class T>
    size_t count() const
    {
        auto iter = s_items<T>.find(this);
        if (iter != s_items<T>.cend())
            return s_items<T>[this].size();
        return 0;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the total number of elements inside the container.
    //--------------------------------------------------------------------------
    size_t size() const
    {
        size_t sum = 0u;
        for (auto&& size_func : m_size_functions)
        {
            sum += size_func(*this);
        }
        return sum;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the Stack<T> for the given template T.
    //--------------------------------------------------------------------------
    template <class T>
    Stack<T>& stack()
    {
        return s_items<T>[this];
    }

    //--------------------------------------------------------------------------
    //! \brief Insert the element \c t of type T to the \c key slot.
    //--------------------------------------------------------------------------
    template <class T>
    T& push(const T& t)
    {
        // Don't have it yet ? So create functions for destroying
        if (s_items<T>.find(this) == std::end(s_items<T>))
        {
            m_clear_functions.emplace_back([](Any& a)
            {
                s_items<T>.erase(&a);
            });

            m_copy_functions.emplace_back([](Any const& from, Any& to)
            {
                s_items<T>[&to] = s_items<T>[&from];
            });

            m_size_functions.emplace_back([](Any const& a)
            {
                return s_items<T>[&a].size();
            });
        }

        // Insert
        return s_items<T>[this].push(t);
    }

    //--------------------------------------------------------------------------
    //! \brief Get the element of type T given its key.
    //--------------------------------------------------------------------------
    template <class T>
    T& get(const char* key)
    {
        return s_items<T>[this].at(key);
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Container for any type of elements. It uses variable template so
    //! it will need C++14 to compile.
    //!
    //! Variable templates create separate variables. The static keyword allows
    //! to create a new member Stack<T> across all class
    //! instances. But since we only want to care about the types that are used
    //! for that specific instancen we have to use a map of pointers to Any as
    //! keys.
    //--------------------------------------------------------------------------
    template<class T>
    static std::unordered_map<const Any*, Stack<T>> s_items;

    //! \brief
    std::vector<std::function<void(Any&)>> m_clear_functions;

    //! \brief
    std::vector<std::function<void(Any const&, Any&)>> m_copy_functions;

    //! \brief
    std::vector<std::function<size_t(Any const&)>> m_size_functions;
};

// storage for our static members
template<class T>
std::unordered_map<const Any*, Stack<T>> Any::s_items;

#endif
