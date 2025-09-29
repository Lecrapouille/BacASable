#include <iostream>
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <stdexcept>
#include <any>
#include <type_traits>

// ============================================================================
//! \brief Factory to create objects from a std::string
//! \tparam Base the base class
// ============================================================================
template<typename Base>
class Factory {
public:

    // ------------------------------------------------------------------------
    //! \brief Type of the key
    // ------------------------------------------------------------------------
    using Key = std::string;

    // ------------------------------------------------------------------------
    //! \brief Register a type without constructor parameters
    //! \tparam Derived the derived class
    //! \param key the key of the type
    // ------------------------------------------------------------------------
    template<typename Derived>
    std::enable_if_t<std::is_default_constructible_v<Derived>, void>
    registerType(const Key& key) {
        creators[key] = [key](std::any args) -> std::unique_ptr<Base> {
            try {
                // Pour les types sans paramètres
                if (!args.has_value()) {
                    return std::make_unique<Derived>();
                }

                // Tentative de cast vers un tuple d'arguments
                auto& args_tuple = std::any_cast<std::tuple<>&>(args);
                return std::apply([](auto&&... params) {
                    return std::make_unique<Derived>(std::forward<decltype(params)>(params)...);
                }, args_tuple);
            } catch (const std::bad_any_cast&) {
                throw std::runtime_error("Arguments invalides pour le type : " + key);
            }
        };
    }

    // ------------------------------------------------------------------------
    //! \brief Register a type with specific constructor parameters
    //! \tparam Derived the derived class
    //! \tparam Args the arguments of the constructor
    //! \param key the key of the type
    // ------------------------------------------------------------------------
    template<typename Derived, typename... Args>
    std::enable_if_t<(sizeof...(Args) > 0), void>
    registerType(const Key& key) {
        creators[key] = [](std::any args) -> std::unique_ptr<Base> {
            try {
                auto args_tuple = std::any_cast<std::tuple<Args...>>(args);
                return std::apply([](auto&&... params) {
                    return std::make_unique<Derived>(std::forward<decltype(params)>(params)...);
                }, args_tuple);
            } catch (const std::bad_any_cast&) {
                throw std::runtime_error("Arguments invalides pour le type");
            }
        };
    }

    // ------------------------------------------------------------------------
    //! \brief Create an object without parameters
    //! \param key the key of the type
    // ------------------------------------------------------------------------
    std::unique_ptr<Base> create(const Key& key) const {
        auto it = creators.find(key);
        if (it == creators.end()) {
            throw std::runtime_error("Type non enregistré : " + key);
        }
        return it->second(std::any{});
    }

    // ------------------------------------------------------------------------
    //! \brief Create an object with parameters
    //! \tparam Args the arguments of the constructor
    //! \param key the key of the type
    // ------------------------------------------------------------------------
    template<typename... Args>
    std::unique_ptr<Base> create(const Key& key, Args&&... args) const {
        auto it = creators.find(key);
        if (it == creators.end()) {
            throw std::runtime_error("Type non enregistré : " + key);
        }
        auto args_tuple = std::make_tuple(std::forward<Args>(args)...);
        return it->second(std::make_any<std::tuple<Args...>>(std::move(args_tuple)));
    }

private:
    std::unordered_map<Key, std::function<std::unique_ptr<Base>(std::any)>> creators;
};

// ============================================================================
//! \brief Animal base class
// ============================================================================
struct Animal
{
    virtual ~Animal() = default;
    virtual void speak() = 0;
};

// ============================================================================
//! \brief Dog class
// ============================================================================
struct Dog : public Animal
{
    Dog(std::string name, int age) : name(std::move(name)), age(age) {}
    void speak() override { std::cout << "Woof! I am " << name << " and I have " << age << " years.\n"; }
    std::string name;
    int age;
};

// ============================================================================
//! \brief Cat class
// ============================================================================
struct Cat : public Animal
{
    Cat(std::string color) : color(std::move(color)) {}
    void speak() override { std::cout << "Miaou! I'm a cat " << color << ".\n"; }
    std::string color;
};

// ============================================================================
//! \brief Simple class
// ============================================================================
struct Simple : public Animal
{
    void speak() override { std::cout << "mute\n"; }
};

// ============================================================================
//! \brief g++ -std=c++17 -O2 -I. fabric.cpp -o fabric
// ============================================================================
int main()
{
    Factory<Animal> factory;

    // Register types with their constructor signatures
    factory.registerType<Dog, std::string, int>("dog");
    factory.registerType<Cat, std::string>("cat");
    factory.registerType<Simple>("simple");

    // Create objects
    auto dog = factory.create("dog", std::string("Rex"), 5);
    auto cat = factory.create("cat", std::string("blanc"));
    auto simple = factory.create("simple");

    // Speak
    dog->speak();
    cat->speak();
    simple->speak();

    return 0;
}

