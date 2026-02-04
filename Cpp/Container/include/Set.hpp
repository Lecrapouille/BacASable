#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include <bit>
#include <cassert>
#include <cstddef>
#include <span>
#include <initializer_list>
#include <iterator>
#include <concepts>

namespace container {

// ============================================================================
// Internal implementation details - not part of public API
// ============================================================================
namespace detail {

// ----------------------------------------------------------------------------
// Compile-time constants for block configuration
// ----------------------------------------------------------------------------

/// @brief Number of elements per block: 2^N
template<size_t N>
inline constexpr size_t BlockSize = 1u << N;

/// @brief Number of bits in a size_t (for bitfield storage)
inline constexpr size_t BitsPerWord = sizeof(size_t) * 8u;

/// @brief Number of size_t words needed to store M bits
template<size_t N>
inline constexpr size_t BitfieldWords = (BlockSize<N> + BitsPerWord - 1u) / BitsPerWord;

// ----------------------------------------------------------------------------
// Constexpr utility functions (replacing old macros)
// ----------------------------------------------------------------------------

/// @brief Fast modulo for power-of-two divisors: a % (2^N)
/// @param a The dividend
/// @param N The exponent (divisor is 2^N)
/// @return a mod 2^N
template<size_t N>
[[nodiscard]] constexpr size_t modulo_pow2(size_t a) noexcept
{
    return a & (BlockSize<N> - 1u);
}

/// @brief Compute block index from element index
/// @param index Element index
/// @return Block index (index >> N)
template<size_t N>
[[nodiscard]] constexpr size_t block_index(size_t index) noexcept
{
    return index >> N;
}

/// @brief Compute sub-index within a block
/// @param index Element index
/// @return Index within the block [0, 2^N)
template<size_t N>
[[nodiscard]] constexpr size_t sub_index(size_t index) noexcept
{
    return modulo_pow2<N>(index);
}

// ============================================================================
/// @brief Tracks the smallest contiguous range of modified elements.
///
/// This class is used to know which elements have been modified and may need
/// to be synchronized (e.g., for GPU transfers or database updates).
// ============================================================================
class PendingData
{
public:
    /// @brief Sentinel value indicating no pending data (similar to std::string::npos)
    static constexpr size_t npos = static_cast<size_t>(-1);

    /// @brief Default constructor with no pending data
    PendingData() = default;

    /// @brief Constructor marking first nb_elt elements as dirty
    /// @param nb_elt Number of elements to mark as pending
    explicit PendingData(size_t nb_elt)
    {
        clear_pending(nb_elt);
    }

    /// @brief Check if any elements are marked as modified
    /// @return true if there are pending (dirty) elements
    [[nodiscard]] bool has_pending_data() const noexcept
    {
        return m_pending_start != npos;
    }

    /// @brief Get the range of pending elements
    /// @return Pair of {start, end} indices. Both are npos if no pending data.
    [[nodiscard]] std::pair<size_t, size_t> get_pending_data() const noexcept
    {
        return {m_pending_start, m_pending_end};
    }

    /// @brief Reset pending state (no dirty elements)
    void clear_pending() noexcept
    {
        m_pending_start = npos;
        m_pending_end = npos;
    }

    /// @brief Reset and mark first nb_elt elements as dirty
    /// @param nb_elt Number of elements to mark (0 clears all pending)
    void clear_pending(size_t nb_elt) noexcept
    {
        if (nb_elt == 0u)
        {
            m_pending_start = npos;
            m_pending_end = npos;
        }
        else
        {
            m_pending_start = 0u;
            m_pending_end = nb_elt;
        }
    }

    /// @brief Expand pending range to include a new position
    /// @param pos Position to include in the pending range
    void tag_as_pending(size_t pos) noexcept
    {
        if (has_pending_data())
        {
            m_pending_start = std::min(m_pending_start, pos);
            m_pending_end = std::max(m_pending_end, pos + 1u);
        }
        else
        {
            m_pending_start = pos;
            m_pending_end = pos + 1u;
        }
    }

    /// @brief Expand pending range to include a range
    /// @param pos_start Start of range to include
    /// @param pos_end End of range to include (exclusive)
    void tag_as_pending(size_t pos_start, size_t pos_end) noexcept
    {
        if (has_pending_data())
        {
            m_pending_start = std::min(m_pending_start, pos_start);
            m_pending_end = std::max(m_pending_end, pos_end);
        }
        else
        {
            m_pending_start = pos_start;
            m_pending_end = pos_end;
        }
    }

protected:
    size_t m_pending_start = npos;  ///< Start of dirty range
    size_t m_pending_end = npos;    ///< End of dirty range (exclusive)
};

// ============================================================================
/// @brief A contiguous block of M = 2^N elements with occupancy tracking.
///
/// Each block stores elements in a contiguous array and uses a bitfield to
/// track which slots are occupied. This allows O(1) insertion/removal.
///
/// @tparam T Element type
/// @tparam N Log2 of block size (block holds 2^N elements)
// ============================================================================
template<typename T, size_t N>
class Block : public PendingData
{
public:
    /// @brief Number of elements this block can hold
    static constexpr size_t Capacity = BlockSize<N>;

private:
    /// @brief Number of words in the occupancy bitfield
    static constexpr size_t NumBitfieldWords = BitfieldWords<N>;

public:
    /// @brief Construct a block, optionally deferring memory allocation
    /// @param lazy_allocation If true, defer allocation until first access
    explicit Block(bool lazy_allocation = false)
        : PendingData()
    {
        if (!lazy_allocation)
        {
            m_data = std::make_unique<T[]>(Capacity);
        }
        clear();
    }

    /// @brief Clear occupancy bitfield (mark all slots as empty)
    void clear() noexcept
    {
        for (size_t i = 0u; i < NumBitfieldWords; ++i)
        {
            m_occupied[i] = 0u;
        }
        clear_pending();
    }

    /// @brief Get block capacity
    /// @return Number of elements the block can hold (2^N)
    [[nodiscard]] static constexpr size_t capacity() noexcept
    {
        return Capacity;
    }

    /// @brief Count number of occupied slots using popcount
    /// @return Number of elements currently stored in this block
    [[nodiscard]] size_t occupation() const noexcept
    {
        size_t total = 0u;
        for (size_t i = 0u; i < NumBitfieldWords; ++i)
        {
            total += static_cast<size_t>(std::popcount(m_occupied[i]));
        }
        return total;
    }

    /// @brief Access element at index (allocates if needed)
    /// @param i Index within block [0, Capacity)
    /// @return Reference to element
    [[nodiscard]] T& operator[](size_t i)
    {
        assert(i < Capacity);
        ensure_allocated();
        return m_data[i];
    }

    /// @brief Access element at index (const version)
    /// @param i Index within block [0, Capacity)
    /// @return Const reference to element
    [[nodiscard]] const T& operator[](size_t i) const
    {
        assert(i < Capacity);
        assert(m_data != nullptr);
        return m_data[i];
    }

    /// @brief Check if a slot is occupied
    /// @param i Index within block
    /// @return true if slot contains an element
    [[nodiscard]] bool is_occupied(size_t i) const noexcept
    {
        assert(i < Capacity);
        const size_t word = i / BitsPerWord;
        const size_t bit = i % BitsPerWord;
        return (m_occupied[word] & (1ull << bit)) != 0u;
    }

    /// @brief Mark a slot as occupied
    /// @param i Index within block
    void set_occupied(size_t i) noexcept
    {
        assert(i < Capacity);
        const size_t word = i / BitsPerWord;
        const size_t bit = i % BitsPerWord;
        m_occupied[word] |= (1ull << bit);
    }

    /// @brief Mark a slot as empty
    /// @param i Index within block
    void clear_occupied(size_t i) noexcept
    {
        assert(i < Capacity);
        const size_t word = i / BitsPerWord;
        const size_t bit = i % BitsPerWord;
        m_occupied[word] &= ~(1ull << bit);
    }

    /// @brief Check if data has been allocated
    [[nodiscard]] bool is_allocated() const noexcept
    {
        return m_data != nullptr;
    }

private:
    /// @brief Ensure memory is allocated (for lazy allocation)
    void ensure_allocated()
    {
        if (m_data == nullptr)
        {
            m_data = std::make_unique<T[]>(Capacity);
        }
    }

    std::unique_ptr<T[]> m_data;            ///< Element storage
    size_t m_occupied[NumBitfieldWords]{};  ///< Bitfield tracking occupied slots
};

// ============================================================================
/// @brief Base class for block-allocated containers.
///
/// Manages a vector of blocks and provides common operations like element
/// access, occupancy tracking, and memory management.
///
/// @tparam T Element type
/// @tparam N Log2 of block size
// ============================================================================
template<typename T, size_t N>
    requires std::default_initializable<T> && std::copyable<T>
class ContainerBase
{
public:
    using block_type = Block<T, N>;
    static constexpr size_t BlockCapacity = BlockSize<N>;

protected:
    /// @brief Construct with optional pre-allocation
    /// @param reserve_elements Number of elements to pre-allocate space for
    explicit ContainerBase(size_t reserve_elements = 0u)
    {
        if (reserve_elements > 0u)
        {
            reserve(reserve_elements);
        }
    }

    /// @brief Virtual destructor for proper cleanup
    virtual ~ContainerBase() = default;

public:
    /// @brief Pre-allocate space for at least n elements
    /// @param n Minimum number of elements to reserve space for
    void reserve(size_t n)
    {
        const size_t needed_blocks = (n + BlockCapacity - 1u) >> N;
        allocate_blocks(needed_blocks);
    }

    /// @brief Get number of stored elements
    /// @return Number of elements currently in container
    [[nodiscard]] size_t size() const noexcept
    {
        return m_stored_elements;
    }

    /// @brief Check if container is empty
    /// @return true if no elements are stored
    [[nodiscard]] bool empty() const noexcept
    {
        return m_stored_elements == 0u;
    }

    /// @brief Boolean conversion (true if not empty)
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return !empty();
    }

    /// @brief Get number of allocated blocks
    [[nodiscard]] size_t block_count() const noexcept
    {
        return m_blocks.size();
    }

    /// @brief Get total capacity (allocated slots)
    [[nodiscard]] size_t capacity() const noexcept
    {
        return m_blocks.size() << N;
    }

    /// @brief Get remaining capacity before new allocation needed
    [[nodiscard]] size_t remaining() const noexcept
    {
        return capacity() - m_stored_elements;
    }

    /// @brief Check if container is full (all allocated slots used)
    [[nodiscard]] bool full() const noexcept
    {
        return m_stored_elements == capacity();
    }

    /// @brief Access element without bounds checking
    /// @param index Element index
    /// @return Reference to element
    [[nodiscard]] T& operator[](size_t index)
    {
        const size_t bid = block_index<N>(index);
        const size_t sid = sub_index<N>(index);
        return (*m_blocks[bid])[sid];
    }

    /// @brief Access element without bounds checking (const)
    /// @param index Element index
    /// @return Const reference to element
    [[nodiscard]] const T& operator[](size_t index) const
    {
        const size_t bid = block_index<N>(index);
        const size_t sid = sub_index<N>(index);
        return (*m_blocks[bid])[sid];
    }

    /// @brief Check if index is out of bounds
    /// @param index Index to check
    /// @return true if index is invalid
    [[nodiscard]] virtual bool out_of_bounds(size_t index) const noexcept = 0;

    /// @brief Check if a slot is occupied
    /// @param index Element index
    /// @return true if slot contains an element
    [[nodiscard]] bool occupied(size_t index) const
    {
        if (out_of_bounds(index))
        {
            return false;
        }
        const size_t bid = block_index<N>(index);
        const size_t sid = sub_index<N>(index);
        return m_blocks[bid]->is_occupied(sid);
    }

    /// @brief Clear all elements (does not deallocate blocks)
    virtual void clear()
    {
        for (auto& block : m_blocks)
        {
            block->clear();
        }
        m_stored_elements = 0u;
    }

    /// @brief Release empty blocks at the end
    virtual void shrink_to_fit()
    {
        // Remove empty blocks from the end
        while (!m_blocks.empty())
        {
            if (m_blocks.back()->occupation() == 0u)
            {
                m_blocks.pop_back();
            }
            else
            {
                break;
            }
        }
    }

protected:
    /// @brief Allocate additional blocks
    /// @param num_blocks Number of blocks to add
    void allocate_blocks(size_t num_blocks)
    {
        m_blocks.reserve(m_blocks.size() + num_blocks);
        for (size_t i = 0u; i < num_blocks; ++i)
        {
            m_blocks.push_back(std::make_unique<block_type>(false));
        }
    }

    /// @brief Ensure we have capacity for the given index
    /// @param index Index that must be valid after this call
    void ensure_capacity(size_t index)
    {
        const size_t needed_blocks = block_index<N>(index) + 1u;
        if (needed_blocks > m_blocks.size())
        {
            allocate_blocks(needed_blocks - m_blocks.size());
        }
    }

    /// @brief Mark element as occupied and update count
    void mark_occupied(size_t bid, size_t sid)
    {
        if (!m_blocks[bid]->is_occupied(sid))
        {
            m_blocks[bid]->set_occupied(sid);
            ++m_stored_elements;
        }
    }

    /// @brief Mark element as empty and update count
    void mark_empty(size_t bid, size_t sid)
    {
        if (m_blocks[bid]->is_occupied(sid))
        {
            m_blocks[bid]->clear_occupied(sid);
            --m_stored_elements;
        }
    }

    std::vector<std::unique_ptr<block_type>> m_blocks;  ///< Block storage
    size_t m_stored_elements = 0u;                       ///< Number of stored elements
};

} // namespace detail

// ============================================================================
/// @brief Dense container without holes (block-allocated vector).
///
/// Set is similar to std::vector but uses block allocation to minimize
/// reallocation overhead. When full, a new block is allocated instead of
/// copying all elements to a larger array.
///
/// Key properties:
/// - O(1) append (amortized)
/// - O(1) access by index
/// - O(1) removal (swaps with last element)
/// - No holes: removing an element compacts the container
///
/// @tparam T Element type (must be default_initializable and copyable)
/// @tparam N Log2 of block size (default 4 = 16 elements per block)
///
/// @code
/// Set<int, 4> set;              // Blocks of 16 elements
/// set += 42;                    // Append element
/// set += {1, 2, 3};             // Append multiple
/// int x = set[0];               // Access (no bounds check)
/// int y = set.at(0);            // Access (with bounds check)
/// set.remove(0);                // Remove (swaps with last)
/// for (auto& e : set) { ... }   // Iterate
/// @endcode
// ============================================================================
template<typename T, size_t N = 4>
    requires std::default_initializable<T> && std::copyable<T>
class Set : public detail::ContainerBase<T, N>
{
    using Base = detail::ContainerBase<T, N>;
    using Base::m_blocks;
    using Base::m_stored_elements;
    using Base::BlockCapacity;

public:
    // ========================================================================
    // Forward Iterator
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

        iterator(Set& container, size_t pos)
            : m_container(&container), m_pos(pos)
        {}

        reference operator*() const { return (*m_container)[m_pos]; }
        pointer operator->() const { return &(*m_container)[m_pos]; }

        iterator& operator++()
        {
            ++m_pos;
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const = default;

    private:
        Set* m_container = nullptr;
        size_t m_pos = 0u;
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

        const_iterator(const Set& container, size_t pos)
            : m_container(&container), m_pos(pos)
        {}

        reference operator*() const { return (*m_container)[m_pos]; }
        pointer operator->() const { return &(*m_container)[m_pos]; }

        const_iterator& operator++()
        {
            ++m_pos;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const = default;

    private:
        const Set* m_container = nullptr;
        size_t m_pos = 0u;
    };

    // ========================================================================
    // Constructors
    // ========================================================================

    /// @brief Default constructor (empty container)
    Set() : Base(0u) {}

    /// @brief Construct with pre-allocated capacity
    /// @param reserve_elements Number of elements to reserve space for
    explicit Set(size_t reserve_elements) : Base(reserve_elements) {}

    /// @brief Construct from initializer list
    /// @param init Elements to insert
    Set(std::initializer_list<T> init) : Base(init.size())
    {
        for (const auto& elem : init)
        {
            append_impl(elem);
        }
    }

    // ========================================================================
    // Element Access
    // ========================================================================

    using Base::operator[];

    /// @brief Access element with bounds checking
    /// @param index Element index
    /// @return Reference to element
    /// @throw std::out_of_range if index is invalid
    [[nodiscard]] T& at(size_t index)
    {
        if (out_of_bounds(index))
        {
            throw std::out_of_range("Set::at - index " + std::to_string(index) +
                                    " out of range (size=" + std::to_string(m_stored_elements) + ")");
        }
        return (*this)[index];
    }

    /// @brief Access element with bounds checking (const)
    [[nodiscard]] const T& at(size_t index) const
    {
        if (out_of_bounds(index))
        {
            throw std::out_of_range("Set::at - index " + std::to_string(index) +
                                    " out of range (size=" + std::to_string(m_stored_elements) + ")");
        }
        return (*this)[index];
    }

    // ========================================================================
    // Modifiers
    // ========================================================================

    /// @brief Append a single element
    /// @param elem Element to append
    /// @return Reference to this container
    Set& operator+=(const T& elem)
    {
        append_impl(elem);
        return *this;
    }

    /// @brief Append multiple elements from a span
    /// @param data Elements to append
    /// @return Reference to this container
    Set& operator+=(std::span<const T> data)
    {
        for (const auto& elem : data)
        {
            append_impl(elem);
        }
        return *this;
    }

    /// @brief Append multiple elements from initializer list
    /// @param init Elements to append
    /// @return Reference to this container
    Set& operator+=(std::initializer_list<T> init)
    {
        for (const auto& elem : init)
        {
            append_impl(elem);
        }
        return *this;
    }

    /// @brief Remove element at index (swaps with last element)
    /// @param index Index of element to remove
    /// @note After removal, the element previously at the end is at index
    void remove(size_t index)
    {
        if (out_of_bounds(index))
        {
            return;
        }

        // If not removing the last element, swap with last
        if (index != m_stored_elements - 1u)
        {
            (*this)[index] = (*this)[m_stored_elements - 1u];
            const size_t bid = detail::block_index<N>(index);
            const size_t sid = detail::sub_index<N>(index);
            m_blocks[bid]->tag_as_pending(sid);
        }

        pop_back();
    }

    /// @brief Remove the last element
    void pop_back()
    {
        if (m_stored_elements == 0u)
        {
            return;
        }

        const size_t last = m_stored_elements - 1u;
        const size_t bid = detail::block_index<N>(last);
        const size_t sid = detail::sub_index<N>(last);

        m_blocks[bid]->clear_occupied(sid);
        --m_stored_elements;
    }

    /// @brief Swap two elements
    /// @param i First index
    /// @param j Second index
    void swap(size_t i, size_t j)
    {
        if (i == j || out_of_bounds(i) || out_of_bounds(j))
        {
            return;
        }

        T temp = (*this)[i];
        (*this)[i] = (*this)[j];
        (*this)[j] = temp;

        // Mark as pending
        m_blocks[detail::block_index<N>(i)]->tag_as_pending(detail::sub_index<N>(i));
        m_blocks[detail::block_index<N>(j)]->tag_as_pending(detail::sub_index<N>(j));
    }

    /// @brief Clear all elements
    void clear() override
    {
        Base::clear();
        m_index = 0u;
        m_subindex = 0u;
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
        return index >= m_stored_elements;
    }

    // ========================================================================
    // Pending Data (modification tracking)
    // ========================================================================

    /// @brief Check if any elements have been modified since last clear_pending()
    /// @return true if there are pending (dirty) elements
    ///
    /// Use this to know if data needs to be synchronized (e.g., uploaded to GPU).
    ///
    /// @code
    /// Set<float> vertices;
    /// vertices += {1.0f, 2.0f, 3.0f};
    ///
    /// if (vertices.has_pending_data()) {
    ///     auto [start, end] = vertices.get_pending_range();
    ///     upload_to_gpu(&vertices[start], end - start);
    ///     vertices.clear_pending();
    /// }
    /// @endcode
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
    ///
    /// Returns the smallest contiguous range containing all modified elements.
    /// If no pending data, returns {npos, npos} where npos = size_t(-1).
    ///
    /// @code
    /// Set<int> data = {1, 2, 3, 4, 5};
    /// data.clear_pending();  // Reset tracking
    ///
    /// data[2] = 100;         // Modify element
    /// data[4] = 200;
    ///
    /// auto [start, end] = data.get_pending_range();
    /// // start = 2, end = 5 (range covering indices 2, 3, 4)
    /// @endcode
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

                // Convert to global indices
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
    ///
    /// Call this after you have processed/synchronized the modified data.
    ///
    /// @code
    /// if (set.has_pending_data()) {
    ///     auto [start, end] = set.get_pending_range();
    ///     sync_data(&set[start], end - start);
    ///     set.clear_pending();  // Data is now synchronized
    /// }
    /// @endcode
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

    [[nodiscard]] iterator begin() { return iterator(*this, 0u); }
    [[nodiscard]] iterator end() { return iterator(*this, m_stored_elements); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(*this, 0u); }
    [[nodiscard]] const_iterator end() const { return const_iterator(*this, m_stored_elements); }
    [[nodiscard]] const_iterator cbegin() const { return begin(); }
    [[nodiscard]] const_iterator cend() const { return end(); }

private:
    /// @brief Implementation of append
    void append_impl(const T& elem)
    {
        // Advance position
        if (m_stored_elements > 0u)
        {
            m_subindex = detail::modulo_pow2<N>(m_subindex + 1u);
            if (m_subindex == 0u)
            {
                ++m_index;
            }
        }

        // Ensure we have space
        if (m_index >= m_blocks.size())
        {
            this->allocate_blocks(1u);
        }

        // Insert element
        (*m_blocks[m_index])[m_subindex] = elem;
        m_blocks[m_index]->set_occupied(m_subindex);
        m_blocks[m_index]->tag_as_pending(m_subindex);
        ++m_stored_elements;
    }

    size_t m_index = 0u;     ///< Current block index
    size_t m_subindex = 0u;  ///< Current position within block
};

} // namespace container