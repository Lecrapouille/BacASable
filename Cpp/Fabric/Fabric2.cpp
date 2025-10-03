#include <memory>
#include <unordered_map>
#include <any>
#include <functional>
#include <string>
#include <type_traits>
#include <stdexcept>

template <typename Base, typename Key>
class Factory {
public:

    template <typename Derived, typename... Args>
    void registerType(const Key& key) {
        static_assert(std::is_base_of_v<Base, Derived>,
                      "T must inherit from Base");

        // Explicitly convert lambda to std::function before storing in std::any
        std::function<std::unique_ptr<Base>(Args...)> creator =
            [](Args... args) -> std::unique_ptr<Base> {
                return std::make_unique<Derived>(std::forward<Args>(args)...);
            };
        creators_[key] = creator;
    }

    template <typename... Args>
    std::unique_ptr<Base> create(const Key& key, Args&&... args) const {
        auto it = creators_.find(key);
        if (it == creators_.end())
            throw std::runtime_error("Unknown key: " + key);

        auto creator = std::any_cast<std::function<std::unique_ptr<Base>(Args...)>>(
                           it->second);
        return creator(std::forward<Args>(args)...);
    }

private:
    std::unordered_map<Key, std::any> creators_;
};

/* ------------------------------------------------------------
 * Exemple d’utilisation
 * ------------------------------------------------------------
 */
#include <iostream>

struct Widget {
    virtual ~Widget() = default;
    virtual void foo() const = 0;
};

struct A : Widget {
    A() { std::cout << "A()\n"; }
    void foo() const override { std::cout << "A::foo\n"; }
};

struct B : Widget {
    int& ref;
    explicit B(int& r) : ref(r) { std::cout << "B(int&)\n"; }
    B(const B&)            = delete;
    B& operator=(const B&) = delete;
    void foo() const override { std::cout << "B::foo ref=" << ref << '\n'; }
};

// Classe non-copiable utilisée comme paramètre
class NonCopyable {
public:
    explicit NonCopyable(int val) : value_(val) {
        std::cout << "NonCopyable(" << val << ")\n";
    }
    NonCopyable(const NonCopyable&)            = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    int getValue() const { return value_; }
private:
    int value_;
};

// Widget qui prend une référence à NonCopyable
struct C : Widget {
    const NonCopyable& nc_ref;
    explicit C(const NonCopyable& nc) : nc_ref(nc) {
        std::cout << "C(const NonCopyable&)\n";
    }
    C(const C&)            = delete;
    C& operator=(const C&) = delete;
    void foo() const override {
        std::cout << "C::foo nc_ref.value=" << nc_ref.getValue() << '\n';
    }
};

class SingletonFactory : public Factory<Widget, std::string>
{
public:
    static SingletonFactory& instance() {
        static SingletonFactory inst;
        return inst;
    }

    SingletonFactory(const SingletonFactory&)            = delete;
    SingletonFactory& operator=(const SingletonFactory&) = delete;

    private:
    SingletonFactory() = default;
};

int main() {
    auto& factory = SingletonFactory::instance();

    factory.registerType<A>("A");          // constructeur vide
    int x = 42;
    factory.registerType<B, int&>("B");    // constructeur avec référence

    // Test avec une classe non-copiable passée par référence
    NonCopyable nc(99);
    factory.registerType<C, const NonCopyable&>("C");

    auto a = factory.create("A");
    auto b = factory.create("B", x);
    auto c = factory.create<const NonCopyable&>("C", nc);

    a->foo();
    b->foo();
    c->foo();
}