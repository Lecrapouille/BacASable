#include "Serialization.h"

// ============================================================================
//! \brief Example structure demonstrating serialization capabilities
// ============================================================================
class Person
{
public:
    // ------------------------------------------------------------------------
    //! \brief Default constructor
    // ------------------------------------------------------------------------
    Person() = default;

    // ------------------------------------------------------------------------
    //! \brief Constructor with parameters
    // ------------------------------------------------------------------------
    Person(const std::string& p_name, int p_age, const std::vector<std::string>& p_hobbies)
        : m_name(p_name), m_age(p_age), m_hobbies(p_hobbies) {}

    // ------------------------------------------------------------------------
    //! \brief Get the name
    // ------------------------------------------------------------------------
    std::string const& name() const { return m_name; }

    // ------------------------------------------------------------------------
    //! \brief Get the age
    // ------------------------------------------------------------------------
    int age() const { return m_age; }

    // ------------------------------------------------------------------------
    //! \brief Get the hobbies
    // ------------------------------------------------------------------------
    std::vector<std::string> const& hobbies() const { return m_hobbies; }

    // ------------------------------------------------------------------------
    //! \brief Serialization operators
    // ------------------------------------------------------------------------
    friend Serializer& operator<<(Serializer& p_serializer, const Person& p_person)
    {
        p_serializer << p_person.m_name << p_person.m_age << p_person.m_hobbies;
        return p_serializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Deserialization operators
    // ------------------------------------------------------------------------
    friend Deserializer& operator>>(Deserializer& p_deserializer, Person& p_person)
    {
        p_deserializer >> p_person.m_name >> p_person.m_age >> p_person.m_hobbies;
        return p_deserializer;
    }

    // ------------------------------------------------------------------------
    //! \brief Utility function to print person info
    // ------------------------------------------------------------------------
    void print() const
    {
        std::cout << "Name: " << m_name << std::endl;
        std::cout << "Age: " << m_age << std::endl;
        std::cout << "Hobbies: ";
        for (size_t i = 0; i < m_hobbies.size(); ++i)
        {
            std::cout << m_hobbies[i];
            if (i < m_hobbies.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

private:
    std::string m_name;
    int m_age;
    std::vector<std::string> m_hobbies;
};

// ============================================================================
//! \brief Example 1: Serialization of a single person
// ============================================================================
inline bool example_single_person()
{
    std::cout << "=== Serialization single person ===" << std::endl;

    // Create a person with data
    Person original_person("Jean Dupont", 30, {"lecture", "musique", "sport"});

    std::cout << "Original person:" << std::endl;
    original_person.print();
    std::cout << std::endl;

    // Serialize the person
    Serializer serializer;
    serializer << original_person;

    std::cout << "Serialized data size: " << serializer.data().size() << " bytes" << std::endl;
    std::cout << std::endl;

    // Deserialize the person
    serialization::Container data = serializer.data();
    Deserializer deserializer(data);

    Person deserialized_person;
    deserializer >> deserialized_person;

    std::cout << "Deserialized person:" << std::endl;
    deserialized_person.print();
    std::cout << std::endl;

    // Verify the data is the same
    bool is_same = (original_person.name() == deserialized_person.name() &&
                   original_person.age() == deserialized_person.age() &&
                   original_person.hobbies() == deserialized_person.hobbies());

    std::cout << "Data integrity check: " << (is_same ? "PASSED" : "FAILED") << std::endl;
    return is_same;
}

// ============================================================================
//! \brief Example 2: Serialization of a vector of people
// ============================================================================
inline bool example_vector_of_people()
{
    std::cout << "\n=== Serialization vector of people ===" << std::endl;

    // Test with multiple persons
    std::vector<Person> people = {
        Person("Alice Martin", 25, {"peinture", "voyage"}),
        Person("Bob Durand", 35, {"cuisine", "jardinage", "photographie"}),
        Person("Claire Dubois", 28, {"danse"})
    };

    std::cout << "Original people:" << std::endl;
    for (size_t i = 0; i < people.size(); ++i)
    {
        std::cout << "Person " << (i + 1) << ":" << std::endl;
        people[i].print();
        std::cout << std::endl;
    }

    // Serialize the vector of people
    Serializer serializer;
    serializer << people;

    std::cout << "Serialized vector size: " << serializer.data().size() << " bytes" << std::endl;
    std::cout << std::endl;

    // Deserialize the vector of people
    serialization::Container data = serializer.data();
    Deserializer deserializer(data);

    std::vector<Person> deserialized_people;
    deserializer >> deserialized_people;

    std::cout << "Deserialized people:" << std::endl;
    for (size_t i = 0; i < deserialized_people.size(); ++i)
    {
        std::cout << "Person " << (i + 1) << ":" << std::endl;
        deserialized_people[i].print();
        std::cout << std::endl;
    }

    // Verify data integrity
    bool all_same = true;
    for (size_t i = 0; i < people.size(); ++i)
    {
        if (people[i].name() != deserialized_people[i].name() ||
            people[i].age() != deserialized_people[i].age() ||
            people[i].hobbies() != deserialized_people[i].hobbies())
        {
            all_same = false;
            break;
        }
    }

    std::cout << "Vector data integrity check: " << (all_same ? "PASSED" : "FAILED") << std::endl;
    return all_same;
}

// ============================================================================
//! \brief Example 3: Serialization of a map
// ============================================================================
inline bool example_map_serialization()
{
    std::cout << "\n=== Serialization map example ===" << std::endl;

    // Create a map with different types
    std::map<std::string, int> scores = {
        {"Alice", 95},
        {"Bob", 87},
        {"Claire", 92},
        {"David", 78}
    };

    std::map<int, std::string> categories = {
        {1, "Débutant"},
        {2, "Intermédiaire"},
        {3, "Avancé"},
        {4, "Expert"}
    };

    std::map<std::string, std::vector<std::string>> teams = {
        {"Équipe A", {"Alice", "Bob", "Claire"}},
        {"Équipe B", {"David", "Eve", "Frank"}},
        {"Équipe C", {"Grace", "Henry"}}
    };

    std::cout << "Original scores:" << std::endl;
    for (const auto& pair : scores)
    {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    std::cout << "\nOriginal categories:" << std::endl;
    for (const auto& pair : categories)
    {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    std::cout << "\nOriginal teams:" << std::endl;
    for (const auto& pair : teams)
    {
        std::cout << "  " << pair.first << ": ";
        for (size_t i = 0; i < pair.second.size(); ++i)
        {
            std::cout << pair.second[i];
            if (i < pair.second.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Serialize all maps
    Serializer serializer;
    serializer << scores << categories << teams;

    std::cout << "\nSerialized maps size: " << serializer.data().size() << " bytes" << std::endl;
    std::cout << std::endl;

    // Deserialize all maps
    serialization::Container data = serializer.data();
    Deserializer deserializer(data);

    std::map<std::string, int> deserialized_scores;
    std::map<int, std::string> deserialized_categories;
    std::map<std::string, std::vector<std::string>> deserialized_teams;

    deserializer >> deserialized_scores >> deserialized_categories >> deserialized_teams;

    std::cout << "Deserialized scores:" << std::endl;
    for (const auto& pair : deserialized_scores)
    {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    std::cout << "\nDeserialized categories:" << std::endl;
    for (const auto& pair : deserialized_categories)
    {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    std::cout << "\nDeserialized teams:" << std::endl;
    for (const auto& pair : deserialized_teams)
    {
        std::cout << "  " << pair.first << ": ";
        for (size_t i = 0; i < pair.second.size(); ++i)
        {
            std::cout << pair.second[i];
            if (i < pair.second.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Verify data integrity
    bool scores_same = (scores == deserialized_scores);
    bool categories_same = (categories == deserialized_categories);
    bool teams_same = (teams == deserialized_teams);

    std::cout << "\nMaps data integrity check:" << std::endl;
    std::cout << "  Scores: " << (scores_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Categories: " << (categories_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Teams: " << (teams_same ? "PASSED" : "FAILED") << std::endl;

    return scores_same && categories_same && teams_same;
}

// ============================================================================
//! \brief Example 4: Serialization of smart pointers
// ============================================================================
inline bool example_smart_pointers()
{
    std::cout << "\n=== Serialization smart pointers example ===" << std::endl;

    // Create smart pointers with different scenarios
    std::unique_ptr<Person> unique_person(new Person("Marie Curie", 45, {"science", "recherche"}));
    std::unique_ptr<Person> unique_null = nullptr;

    std::shared_ptr<Person> shared_person1(new Person("Albert Einstein", 50, {"physique", "musique"}));
    std::shared_ptr<Person> shared_person2 = shared_person1; // Shared ownership
    std::shared_ptr<Person> shared_null = nullptr;

    // Create smart pointers to vectors
    std::unique_ptr<std::vector<int>> unique_vector(new std::vector<int>{1, 2, 3, 4, 5});

    std::shared_ptr<std::map<std::string, int>> shared_map(new std::map<std::string, int>{{"A", 1}, {"B", 2}, {"C", 3}});

    std::cout << "Original unique_ptr person:" << std::endl;
    if (unique_person)
    {
        unique_person->print();
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nOriginal unique_ptr null:" << std::endl;
    if (unique_null)
    {
        unique_null->print();
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nOriginal shared_ptr person:" << std::endl;
    if (shared_person1)
    {
        shared_person1->print();
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nOriginal unique_ptr vector:" << std::endl;
    if (unique_vector)
    {
        std::cout << "  Vector: ";
        for (size_t i = 0; i < unique_vector->size(); ++i)
        {
            std::cout << (*unique_vector)[i];
            if (i < unique_vector->size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nOriginal shared_ptr map:" << std::endl;
    if (shared_map)
    {
        for (const auto& pair : *shared_map)
        {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    // Serialize all smart pointers
    Serializer serializer;
    serializer << unique_person << unique_null << shared_person1 << shared_null << unique_vector << shared_map;

    std::cout << "\nSerialized smart pointers size: " << serializer.data().size() << " bytes" << std::endl;
    std::cout << std::endl;

    // Deserialize all smart pointers
    serialization::Container data = serializer.data();
    Deserializer deserializer(data);

    std::unique_ptr<Person> deserialized_unique_person;
    std::unique_ptr<Person> deserialized_unique_null;
    std::shared_ptr<Person> deserialized_shared_person1;
    std::shared_ptr<Person> deserialized_shared_null;
    std::unique_ptr<std::vector<int>> deserialized_unique_vector;
    std::shared_ptr<std::map<std::string, int>> deserialized_shared_map;

    deserializer >> deserialized_unique_person >> deserialized_unique_null
                >> deserialized_shared_person1 >> deserialized_shared_null
                >> deserialized_unique_vector >> deserialized_shared_map;

    std::cout << "Deserialized unique_ptr person:" << std::endl;
    if (deserialized_unique_person)
    {
        deserialized_unique_person->print();
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nDeserialized unique_ptr null:" << std::endl;
    if (deserialized_unique_null)
    {
        deserialized_unique_null->print();
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nDeserialized shared_ptr person:" << std::endl;
    if (deserialized_shared_person1)
    {
        deserialized_shared_person1->print();
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nDeserialized unique_ptr vector:" << std::endl;
    if (deserialized_unique_vector)
    {
        std::cout << "  Vector: ";
        for (size_t i = 0; i < deserialized_unique_vector->size(); ++i)
        {
            std::cout << (*deserialized_unique_vector)[i];
            if (i < deserialized_unique_vector->size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    std::cout << "\nDeserialized shared_ptr map:" << std::endl;
    if (deserialized_shared_map)
    {
        for (const auto& pair : *deserialized_shared_map)
        {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
    }
    else
    {
        std::cout << "  (null)" << std::endl;
    }

    // Verify data integrity
    bool unique_person_same = (unique_person && deserialized_unique_person &&
                              unique_person->name() == deserialized_unique_person->name() &&
                              unique_person->age() == deserialized_unique_person->age() &&
                              unique_person->hobbies() == deserialized_unique_person->hobbies());
    bool unique_null_same = (!unique_null && !deserialized_unique_null);
    bool shared_person_same = (shared_person1 && deserialized_shared_person1 &&
                              shared_person1->name() == deserialized_shared_person1->name() &&
                              shared_person1->age() == deserialized_shared_person1->age() &&
                              shared_person1->hobbies() == deserialized_shared_person1->hobbies());
    bool shared_null_same = (!shared_null && !deserialized_shared_null);
    bool vector_same = (unique_vector && deserialized_unique_vector && *unique_vector == *deserialized_unique_vector);
    bool map_same = (shared_map && deserialized_shared_map && *shared_map == *deserialized_shared_map);

    std::cout << "\nSmart pointers data integrity check:" << std::endl;
    std::cout << "  Unique_ptr person: " << (unique_person_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Unique_ptr null: " << (unique_null_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Shared_ptr person: " << (shared_person_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Shared_ptr null: " << (shared_null_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Unique_ptr vector: " << (vector_same ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Shared_ptr map: " << (map_same ? "PASSED" : "FAILED") << std::endl;

    return unique_person_same && unique_null_same && shared_person_same &&
           shared_null_same && vector_same && map_same;
}

// ============================================================================
//! \brief Main function
//! g++ -std=c++17 -Wall -Wextra -O2 -o main main.cpp
// ============================================================================
int main()
{
    if (!example_single_person())
    {
        return EXIT_FAILURE;
    }
    if (!example_vector_of_people())
    {
        return EXIT_FAILURE;
    }
    if (!example_map_serialization())
    {
        return EXIT_FAILURE;
    }
    if (!example_smart_pointers())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}