#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <type_traits>
#include <iostream>
#include <map>
#include <memory>

namespace serialization
{
    using Container = std::vector<uint8_t>;
}

// ============================================================================
//! \brief Class helping to serialize data into a dynamic container of bytes.
//! Supports trivially copyable types, std::string, and std::vector.
// ============================================================================
class Serializer
{
public:

    // ------------------------------------------------------------------------
    //! \brief Serialize: Store trivially copyable data inside the container.
    //! \param p_serializer The serializer object.
    //! \param p_data The data to be stored.
    //! \return The serializer object.
    // ------------------------------------------------------------------------
    template <typename DataType>
    friend Serializer& operator<<(Serializer& p_serializer, DataType const& p_data)
    {
        // Check that the type of the data being pushed is trivially copyable
        static_assert(
            std::is_trivially_copyable<DataType>::value,
            "Type must be trivially copyable for direct serialization");

        size_t container_size = p_serializer.m_container.size();
        p_serializer.m_container.resize(container_size + sizeof(DataType));
        std::memcpy(p_serializer.m_container.data() + container_size, &p_data, sizeof(DataType));
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Serialize: Store std::string inside the container.
    //! \param p_serializer The serializer object.
    //! \param p_string The string to be stored.
    //! \return The serializer object.
    // ------------------------------------------------------------------------
    friend Serializer& operator<<(Serializer& p_serializer, std::string const& p_string)
    {
        // First serialize the size of the string
        size_t string_size = p_string.size();
        p_serializer << string_size;

        // Then serialize the string data
        size_t container_size = p_serializer.m_container.size();
        p_serializer.m_container.resize(container_size + string_size);
        std::memcpy(p_serializer.m_container.data() + container_size, p_string.data(), string_size);
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Serialize: Store std::vector inside the container.
    //! \param p_serializer The serializer object.
    //! \param p_vector The vector to be stored.
    //! \return The serializer object.
    // ------------------------------------------------------------------------
    template <typename T>
    friend Serializer& operator<<(Serializer& p_serializer, std::vector<T> const& p_vector)
    {
        // First serialize the size of the vector
        size_t vector_size = p_vector.size();
        p_serializer << vector_size;

        // Then serialize each element
        for (const auto& element : p_vector)
        {
            p_serializer << element;
        }
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Serialize: Store std::map inside the container.
    //! \param p_serializer The serializer object.
    //! \param p_map The map to be stored.
    //! \return The serializer object.
    // ------------------------------------------------------------------------
    template <typename KeyType, typename ValueType>
    friend Serializer& operator<<(Serializer& p_serializer, std::map<KeyType, ValueType> const& p_map)
    {
        // First serialize the size of the map
        size_t map_size = p_map.size();
        p_serializer << map_size;

        // Then serialize each key-value pair
        for (const auto& pair : p_map)
        {
            p_serializer << pair.first << pair.second;
        }
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Serialize: Store std::unique_ptr inside the container.
    //! \param p_serializer The serializer object.
    //! \param p_unique_ptr The unique_ptr to be stored.
    //! \return The serializer object.
    // ------------------------------------------------------------------------
    template <typename T>
    friend Serializer& operator<<(Serializer& p_serializer, std::unique_ptr<T> const& p_unique_ptr)
    {
        // First serialize whether the pointer is null
        bool is_null = (p_unique_ptr == nullptr);
        p_serializer << is_null;

        // If not null, serialize the pointed object
        if (!is_null)
        {
            p_serializer << *p_unique_ptr;
        }
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Serialize: Store std::shared_ptr inside the container.
    //! \param p_serializer The serializer object.
    //! \param p_shared_ptr The shared_ptr to be stored.
    //! \return The serializer object.
    // ------------------------------------------------------------------------
    template <typename T>
    friend Serializer& operator<<(Serializer& p_serializer, std::shared_ptr<T> const& p_shared_ptr)
    {
        // First serialize whether the pointer is null
        bool is_null = (p_shared_ptr == nullptr);
        p_serializer << is_null;

        // If not null, serialize the pointed object
        if (!is_null)
        {
            p_serializer << *p_shared_ptr;
        }
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Clear the container.
    // ------------------------------------------------------------------------
    inline void clear()
    {
        m_container.clear();
    }

    // ------------------------------------------------------------------------
    //! \brief Get the container.
    // ------------------------------------------------------------------------
    inline serialization::Container const& data() const
    {
        return m_container;
    }

private:
    serialization::Container m_container;
};

// ============================================================================
//! \brief Class helping to deserialize data from a dynamic container of bytes.
//! Supports trivially copyable types, std::string, and std::vector.
// ============================================================================
class Deserializer
{
public:
    // ------------------------------------------------------------------------
    //! \brief Default constructor. Give the container in which this class shall
    //! store bytes.
    //! \param p_container The container in which this class shall store bytes.
    //! \note the container shall not be destroyed while this class is using it.
    // ------------------------------------------------------------------------
    Deserializer(const serialization::Container& p_container)
        : m_container(p_container)
    {}

    // ------------------------------------------------------------------------
    //! \brief Deserialize: Read trivially copyable data from the container.
    //! \param p_deserializer The deserializer object.
    //! \param p_data The data to be read.
    //! \return The deserializer object.
    // ------------------------------------------------------------------------
    template <typename DataType>
    friend Deserializer& operator>>(Deserializer& p_deserializer, DataType& p_data)
    {
        static_assert(
            std::is_trivially_copyable<DataType>::value,
            "Type must be trivially copyable for direct deserialization");

        std::memcpy(&p_data, p_deserializer.m_container.data() +
            p_deserializer.m_offset, sizeof(DataType));
        p_deserializer.m_offset += sizeof(DataType);
        return p_deserializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Deserialize: Read std::string from the container.
    //! \param p_deserializer The deserializer object.
    //! \param p_string The string to be read.
    //! \return The deserializer object.
    // ------------------------------------------------------------------------
    friend Deserializer& operator>>(Deserializer& p_deserializer, std::string& p_string)
    {
        // First deserialize the size of the string
        size_t str_size;
        p_deserializer >> str_size;

        // Then deserialize the string data
        const char* data_ptr = reinterpret_cast<const char*>(
            p_deserializer.m_container.data() + p_deserializer.m_offset);
        p_string.assign(data_ptr, str_size);
        p_deserializer.m_offset += str_size;
        return p_deserializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Deserialize: Read std::vector from the container.
    //! \param p_deserializer The deserializer object.
    //! \param p_vector The vector to be read.
    //! \return The deserializer object.
    // ------------------------------------------------------------------------
    template <typename T>
    friend Deserializer& operator>>(Deserializer& p_deserializer, std::vector<T>& p_vector)
    {
        // First deserialize the size of the vector
        size_t vector_size;
        p_deserializer >> vector_size;

        // Then deserialize each element in order
        p_vector.resize(vector_size);
        for (size_t i = 0; i < vector_size; ++i)
        {
            p_deserializer >> p_vector[i];
        }
        return p_deserializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Deserialize: Read std::map from the container.
    //! \param p_deserializer The deserializer object.
    //! \param p_map The map to be read.
    //! \return The deserializer object.
    // ------------------------------------------------------------------------
    template <typename KeyType, typename ValueType>
    friend Deserializer& operator>>(Deserializer& p_deserializer, std::map<KeyType, ValueType>& p_map)
    {
        // First deserialize the size of the map
        size_t map_size;
        p_deserializer >> map_size;

        // Clear the map and deserialize each key-value pair
        p_map.clear();
        for (size_t i = 0; i < map_size; ++i)
        {
            KeyType key;
            ValueType value;
            p_deserializer >> key >> value;
            p_map[key] = value;
        }
        return p_deserializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Deserialize: Read std::unique_ptr from the container.
    //! \param p_deserializer The deserializer object.
    //! \param p_unique_ptr The unique_ptr to be read.
    //! \return The deserializer object.
    // ------------------------------------------------------------------------
    template <typename T>
    friend Deserializer& operator>>(Deserializer& p_deserializer, std::unique_ptr<T>& p_unique_ptr)
    {
        // First deserialize whether the pointer is null
        bool is_null;
        p_deserializer >> is_null;

        // If null, reset the unique_ptr
        if (is_null)
        {
            p_unique_ptr.reset();
        }
        else
        {
            // Create a new object and deserialize into it
            p_unique_ptr.reset(new T());
            p_deserializer >> *p_unique_ptr;
        }
        return p_deserializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Deserialize: Read std::shared_ptr from the container.
    //! \param p_deserializer The deserializer object.
    //! \param p_shared_ptr The shared_ptr to be read.
    //! \return The deserializer object.
    // ------------------------------------------------------------------------
    template <typename T>
    friend Deserializer& operator>>(Deserializer& p_deserializer, std::shared_ptr<T>& p_shared_ptr)
    {
        // First deserialize whether the pointer is null
        bool is_null;
        p_deserializer >> is_null;

        // If null, reset the shared_ptr
        if (is_null)
        {
            p_shared_ptr.reset();
        }
        else
        {
            // Create a new object and deserialize into it
            p_shared_ptr.reset(new T());
            p_deserializer >> *p_shared_ptr;
        }
        return p_deserializer;
    }

private:
    const serialization::Container& m_container;
    size_t m_offset = 0;
};