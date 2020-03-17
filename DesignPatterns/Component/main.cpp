// Original code source https://stackoverflow.com/questions/44105058/how-does-unitys-getcomponent-work
// Quote: "I'm wondering how the GetComponent() method that Unity implements works."
//
// Compilation:
// g++ --std=c++11 -W -Wall main.cpp -o prog

#include <string>
#include <functional> // std::hash
#include <vector>
#include <memory>
#include <algorithm>

// -----------------------------------------------------------------------------
// For C++11

namespace std {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
} // namespace std

// -----------------------------------------------------------------------------
// https://gist.github.com/Lee-R/3839813

// FNV-1a 32bit hashing algorithm.
constexpr uint32_t hashing(char const* s, std::size_t count)
{
    return ((count ? hashing(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

constexpr uint32_t operator"" _hash(char const* s, std::size_t count)
{
    return hashing(s, count);
}

// -----------------------------------------------------------------------------
class Component
{
  public:

    Component(std::string && name)
        : m_name(name)
    {}

    virtual ~Component() = default;

    virtual bool isClassType(const std::size_t classType) const
    {
        return classType == Type;
    }

    inline std::string const& name() const
    {
        return m_name;
    }

  public:

    static const uint32_t Type = "Component"_hash;

  private:

    std::string m_name;
};

// -----------------------------------------------------------------------------
class GameObject
{
  public:

    template<class ComponentType, typename... Args>
    void addComponent(Args&&... params)
    {
        m_components.emplace_back(
            std::make_unique<ComponentType>(std::forward<Args>(params)...));
    }

    template<class ComponentType>
    ComponentType& getComponent()
    {
        for (auto && component: m_components)
        {
            if (component->isClassType(ComponentType::Type))
                return *static_cast<ComponentType*>(component.get());
        }

        return *std::unique_ptr<ComponentType>(nullptr);
    }

    template<class ComponentType>
    bool removeComponent()
    {
        if (m_components.empty())
            return false;

        auto index =
                std::find_if(m_components.begin(),
                             m_components.end(),
                             [](std::unique_ptr<Component> & component)
                             {
                                 return component->isClassType(ComponentType::Type);
                             });

        bool success = index != m_components.end();
        if (success)
            m_components.erase(index);

        return success;
    }

    template<class ComponentType>
    std::vector<ComponentType*> getComponents()
    {
        std::vector<ComponentType*> componentsOfType;

        for (auto && component : m_components)
        {
            if (component->isClassType(ComponentType::Type))
                componentsOfType.emplace_back(static_cast<ComponentType*>(component.get()));
        }

        return componentsOfType;
    }

    template<class ComponentType>
    int removeComponents()
    {
        if (m_components.empty())
            return 0;

        int numRemoved = 0;
        bool success = false;

        do
        {
            auto index =
                    std::find_if(m_components.begin(),
                                 m_components.end(),
                                 [](std::unique_ptr<Component> & component)
                                 {
                                     return component->isClassType(ComponentType::Type);
                                 });

            success = index != m_components.end();
            if (success)
            {
                m_components.erase(index);
                ++numRemoved;
            }
        }
        while (success);

        return numRemoved;
    }

  private:

    std::vector<std::unique_ptr<Component>> m_components;
};

// -----------------------------------------------------------------------------
class BoxCollider: public Component
{
  public:

    BoxCollider(std::string && initialValue)
            : Component(std::move(initialValue))
    {}

    virtual bool isClassType(const std::size_t classType) const
    {
        if (classType == BoxCollider::Type)
            return true;
        return Component::isClassType(classType);
    }

  public:

    static const std::size_t Type = "BoxCollider"_hash;
};

// -----------------------------------------------------------------------------
class Collider: public Component
{
  public:

    Collider(std::string && initialValue)
            : Component(std::move(initialValue))
    {}

    virtual bool isClassType(const std::size_t classType) const
    {
        if (classType == Collider::Type)
            return true;
        return Component::isClassType(classType);
    }

  public:

    static const std::size_t Type = "Collider"_hash;
};

// -----------------------------------------------------------------------------
#include <iostream>

int main()
{
    GameObject test;

    test.addComponent<BoxCollider>("BoxCollider_A");
    BoxCollider& boxcollider = test.getComponent<BoxCollider>();
    std::cout << boxcollider.name() << std::endl;

    std::cout << Collider::Type << std::endl;
    std::cout << BoxCollider::Type << std::endl;

    //Collider& collider = test.getComponent<Collider>();
    //std::cout << collider.name() << std::endl;

    return 0;
}
