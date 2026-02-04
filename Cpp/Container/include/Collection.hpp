#pragma once

#include "Set.hpp"

namespace container {

// ============================================================================
/// @brief Sparse container allowing holes (deleted elements leave gaps).
///
/// Collection is similar to Set but allows holes: when an element is removed,
/// it leaves an empty slot instead of compacting. This is useful when element
/// indices must remain stable (e.g., external references to elements).
///
/// Key properties:
/// - O(1) insertion at any index
/// - O(1) access by index
/// - O(1) removal (leaves a hole)
/// - Holes allowed: indices remain stable after removal
/// - Iterator automatically skips holes
///
/// @tparam T Element type (must be default_initializable and copyable)
/// @tparam N Log2 of block size (default 4 = 16 elements per block)
///
/// @code
/// Collection<int, 4> col;           // Blocks of 16 elements
/// col += 42;                        // Append element
/// col.insert(5, 100);               // Insert at specific index
/// col.remove(0);                    // Remove (leaves hole)
/// if (col.occupied(0)) { ... }      // Check if slot has element
/// for (auto& e : col) { ... }       // Iterate (skips holes)
/// @endcode
// ============================================================================
template<typename T, size_t N = 4>
    requires std::default_initializable<T> && std::copyable<T>
class Collection : public detail::ContainerBase<T, N>
{
    using Base = detail::ContainerBase<T, N>;
    using Base::m_blocks;
    using Base::m_stored_elements;
    using Base::BlockCapacity;

    /// @brief Sentinel value for uninitialized bounds
    static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

public:
    // ========================================================================
    // Forward Iterator (skips holes)
    // ========================================================================
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator() = default;

        iterator(Collection& container, size_t pos, size_t end)
            : m_container(&container), m_pos(pos), m_end(end)
        {
            // Skip to first occupied position
            skip_holes();
        }

        reference operator*() const { return (*m_container)[m_pos]; }
        pointer operator->() const { return &(*m_container)[m_pos]; }

        iterator& operator++()
        {
            ++m_pos;
            skip_holes();
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const
        {
            return m_container == other.m_container && m_pos == other.m_pos;
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        void skip_holes()
        {
            while (m_pos < m_end && !m_container->occupied(m_pos))
            {
                ++m_pos;
            }
        }

        Collection* m_container = nullptr;
        size_t m_pos = 0u;
        size_t m_end = 0u;
    };

    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator() = default;

        const_iterator(const Collection& container, size_t pos, size_t end)
            : m_container(&container), m_pos(pos), m_end(end)
        {
            skip_holes();
        }

        reference operator*() const { return (*m_container)[m_pos]; }
        pointer operator->() const { return &(*m_container)[m_pos]; }

        const_iterator& operator++()
        {
            ++m_pos;
            skip_holes();
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const
        {
            return m_container == other.m_container && m_pos == other.m_pos;
        }

        bool operator!=(const const_iterator& other) const
        {
            return !(*this == other);
        }

    private:
        void skip_holes()
        {
            while (m_pos < m_end && !m_container->occupied(m_pos))
            {
                ++m_pos;
            }
        }

        const Collection* m_container = nullptr;
        size_t m_pos = 0u;
        size_t m_end = 0u;
    };

    // ========================================================================
    // Constructors
    // ========================================================================

    /// @brief Default constructor (empty container)
    Collection() : Base(0u) {}

    /// @brief Construct with pre-allocated capacity
    /// @param reserve_elements Number of elements to reserve space for
    explicit Collection(size_t reserve_elements) : Base(reserve_elements) {}

    /// @brief Construct from initializer list
    /// @param init Elements to insert (sequentially from index 0)
    Collection(std::initializer_list<T> init) : Base(init.size())
    {
        for (const auto& elem : init)
        {
            insert_impl(m_end, elem);
        }
    }

    // ========================================================================
    // Element Access
    // ========================================================================

    using Base::operator[];

    /// @brief Access element with bounds and occupancy checking
    /// @param index Element index
    /// @return Reference to element
    /// @throw std::out_of_range if index is invalid or slot is empty
    [[nodiscard]] T& at(size_t index)
    {
        check_access(index);
        return (*this)[index];
    }

    /// @brief Access element with bounds and occupancy checking (const)
    [[nodiscard]] const T& at(size_t index) const
    {
        check_access(index);
        return (*this)[index];
    }

    using Base::occupied;

    // ========================================================================
    // Modifiers
    // ========================================================================

    /// @brief Append element at the end
    /// @param elem Element to append
    /// @return Reference to this container
    Collection& operator+=(const T& elem)
    {
        insert_impl(m_end, elem);
        return *this;
    }

    /// @brief Append multiple elements from initializer list
    /// @param init Elements to append
    /// @return Reference to this container
    Collection& operator+=(std::initializer_list<T> init)
    {
        for (const auto& elem : init)
        {
            insert_impl(m_end, elem);
        }
        return *this;
    }

    /// @brief Insert element at specific index
    /// @param index Position to insert at
    /// @param elem Element to insert
    void insert(size_t index, const T& elem)
    {
        insert_impl(index, elem);
    }

    /// @brief Remove element at index (creates a hole)
    /// @param index Index of element to remove
    void remove(size_t index)
    {
        if (out_of_bounds(index))
        {
            return;
        }

        const size_t bid = detail::block_index<N>(index);
        const size_t sid = detail::sub_index<N>(index);

        if (!m_blocks[bid]->is_occupied(sid))
        {
            return;  // Already empty
        }

        m_blocks[bid]->clear_occupied(sid);
        --m_stored_elements;

        // Update bounds
        if (index == m_begin)
        {
            // Find next occupied slot
            while (m_begin < m_end && !this->occupied(m_begin))
            {
                ++m_begin;
            }
        }

        if (index + 1u == m_end)
        {
            // Find previous occupied slot
            while (m_end > 0u && !this->occupied(m_end - 1u))
            {
                --m_end;
            }
        }

        // Reset if empty
        if (m_stored_elements == 0u)
        {
            m_begin = INVALID_INDEX;
            m_end = 0u;
        }
    }

    /// @brief Clear all elements
    void clear() override
    {
        Base::clear();
        m_begin = INVALID_INDEX;
        m_end = 0u;
    }

    // ========================================================================
    // Capacity
    // ========================================================================

    using Base::size;
    using Base::empty;
    using Base::capacity;

    /// @brief Check if index is out of bounds
    [[nodiscard]] bool out_of_bounds(size_t index) const noexcept override
    {
        return index >= (m_blocks.size() << N);
    }

    /// @brief Get the index of the last element + 1
    [[nodiscard]] size_t extent() const noexcept
    {
        return m_end;
    }

    // ========================================================================
    // Pending Data (modification tracking)
    // ========================================================================

    /// @brief Check if any elements have been modified since last clear_pending()
    /// @return true if there are pending (dirty) elements
    [[nodiscard]] bool has_pending_data() const noexcept
    {
        for (const auto& block : m_blocks)
        {
            if (block->has_pending_data())
            {
                return true;
            }
        }
        return false;
    }

    /// @brief Get the range of modified elements across all blocks
    /// @return Pair of {start_index, end_index} (end is exclusive)
    [[nodiscard]] std::pair<size_t, size_t> get_pending_range() const noexcept
    {
        constexpr size_t npos = static_cast<size_t>(-1);
        size_t global_start = npos;
        size_t global_end = npos;

        for (size_t bid = 0u; bid < m_blocks.size(); ++bid)
        {
            if (m_blocks[bid]->has_pending_data())
            {
                auto [block_start, block_end] = m_blocks[bid]->get_pending_data();
                size_t start = (bid << N) + block_start;
                size_t end = (bid << N) + block_end;

                if (global_start == npos)
                {
                    global_start = start;
                    global_end = end;
                }
                else
                {
                    global_start = std::min(global_start, start);
                    global_end = std::max(global_end, end);
                }
            }
        }

        return {global_start, global_end};
    }

    /// @brief Clear all pending data flags (mark as synchronized)
    void clear_pending() noexcept
    {
        for (auto& block : m_blocks)
        {
            block->clear_pending();
        }
    }

    // ========================================================================
    // Iterators
    // ========================================================================

    [[nodiscard]] iterator begin()
    {
        size_t start = (m_begin == INVALID_INDEX) ? m_end : m_begin;
        return iterator(*this, start, m_end);
    }

    [[nodiscard]] iterator end()
    {
        return iterator(*this, m_end, m_end);
    }

    [[nodiscard]] const_iterator begin() const
    {
        size_t start = (m_begin == INVALID_INDEX) ? m_end : m_begin;
        return const_iterator(*this, start, m_end);
    }

    [[nodiscard]] const_iterator end() const
    {
        return const_iterator(*this, m_end, m_end);
    }

    [[nodiscard]] const_iterator cbegin() const { return begin(); }
    [[nodiscard]] const_iterator cend() const { return end(); }

private:
    /// @brief Check if access to index is valid
    /// @throw std::out_of_range if invalid
    void check_access(size_t index) const
    {
        if (out_of_bounds(index))
        {
            throw std::out_of_range("Collection::at - index " + std::to_string(index) +
                                    " out of bounds");
        }
        if (!this->occupied(index))
        {
            throw std::out_of_range("Collection::at - index " + std::to_string(index) +
                                    " is empty (hole)");
        }
    }

    /// @brief Implementation of insert
    void insert_impl(size_t index, const T& elem)
    {
        const size_t bid = detail::block_index<N>(index);
        const size_t sid = detail::sub_index<N>(index);

        // Ensure we have space
        if (bid >= m_blocks.size())
        {
            this->allocate_blocks(bid + 1u - m_blocks.size());
        }

        // Insert element
        (*m_blocks[bid])[sid] = elem;

        if (!m_blocks[bid]->is_occupied(sid))
        {
            m_blocks[bid]->set_occupied(sid);
            ++m_stored_elements;

            // Update bounds
            if (index >= m_end)
            {
                m_end = index + 1u;
            }
            if (index < m_begin || m_begin == INVALID_INDEX)
            {
                m_begin = index;
            }
        }

        m_blocks[bid]->tag_as_pending(sid);
    }

    size_t m_begin = INVALID_INDEX;  ///< Index of first occupied element
    size_t m_end = 0u;               ///< Index after last occupied element
};

} // namespace container