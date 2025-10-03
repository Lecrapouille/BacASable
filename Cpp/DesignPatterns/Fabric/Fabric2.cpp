#include <memory>
#include <unordered_map>
#include <any>
#include <functional>
#include <string>
#include <type_traits>
#include <stdexcept>

#include <iostream>

template <typename Base, typename Key>
class Factory {
public:

    template <typename Derived, typename... Args>
    void registerType(const Key& key) {
        static_assert(std::is_base_of_v<Base, Derived>,
                      "T must inherit from Base");

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
 * Example of usage
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

// Non-copiable class used as parameter
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

// Widget that takes a reference to NonCopyable
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

// Widget that takes a raw pointer
struct D : Widget {
    int* ptr;
    explicit D(int* p) : ptr(p) {
        std::cout << "D(int*)\n";
    }
    D(const D&)            = delete;
    D& operator=(const D&) = delete;
    void foo() const override {
        if (ptr)
            std::cout << "D::foo ptr=" << *ptr << '\n';
        else
            std::cout << "D::foo ptr=nullptr\n";
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

    factory.registerType<A>("A");          // empty constructor
    int x = 42;
    factory.registerType<B, int&>("B");    // constructor with reference

    // Test with a non-copiable class passed by reference
    NonCopyable nc(99);
    factory.registerType<C, const NonCopyable&>("C");

    // Test with a raw pointer
    int y = 123;
    factory.registerType<D, int*>("D");

    auto a = factory.create("A");
    auto b = factory.create("B", x);
    auto c = factory.create<const NonCopyable&>("C", nc);
    auto d = factory.create<int*>("D", &y);

    a->foo();
    b->foo();
    c->foo();
    d->foo();
}