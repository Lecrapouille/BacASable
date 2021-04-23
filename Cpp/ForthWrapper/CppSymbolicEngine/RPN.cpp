#include <ginac/ginac.h>
#include <iostream>

// Basic stack
template<typename T>
class Stack
{
public:
    static constexpr size_t size = 16u; // max number of elements
    static constexpr size_t margin = 2u; // canari to detect stack over/under flow

    ~Stack() { /*std::cout << "Destroying Stack " << typeid(T).name() << std::endl;*/ }

    inline void clear() { sp = sp0; }
    inline int32_t depth() const { return int32_t(sp - sp0); }
    inline void push(T const& t) { *(sp++) = t; }
    inline T& pop() { return *(--sp); }
    inline void drop() { --sp; }
    inline T& tos() { return *(sp - 1); }

    inline bool hasOverflowed() const { return sp > spM; }
    inline bool hasUnderflowed() const { return sp < sp0; }

private:

    T  data[size];
    T* const sp0 = data + margin;         // Begin of the stack
    T* const spM = data + size - margin;  // End of the stack

    T* sp = sp0;
};

// Container of heterogeneous stacks
class Stacks
{
public:

    template<class T> void clear() { stack<T>().clear(); }
    template<class T> int32_t depth() { return stack<T>().depth(); }
    template<class T> void push(T const& t) { stack<T>().push(t); }
    template<class T> T& pop() { return stack<T>().pop(); }
    template<class T> void drop() { stack<T>().drop(); }
    template<class T> T& tos() { return stack<T>().tos(); }
    template<class T> bool hasOverflowed() { return stack<T>().hasOverflowed(); }
    template<class T> bool hasUnderflowed() { return stack<T>().hasUnderflowed(); }

    // Destroy all created heterogeneous stacks
    void erase()
    {
        for (auto&& erase_func : m_erase_functions)
        {
            erase_func(*this);
        }
    }

    ~Stacks() { erase(); }

private:

    template<class T>
    Stack<T>& stack()
    {
        auto it = stacks<T>.find(this);
        if (it == std::end(stacks<T>))
        {
            // Hold list of created heterogeneous stacks for their destruction
            m_erase_functions.emplace_back([](Stacks& s)
            {
                //std::cout << "Erasing stack " << typeid(T).name() << std::endl;
                stacks<T>.erase(&s);
            });

            return stacks<T>[this];
        }
        return it->second;
    }

    template<class T>
    static std::unordered_map<const Stacks*, Stack<T>> stacks;

    std::vector<std::function<void(Stacks&)>> m_erase_functions;
};

template<class T>
std::unordered_map<const Stacks*, Stack<T>> Stacks::stacks;


// Forth context
Stacks stacks;
using Dictionary = std::map<std::string, std::function<void()>>;
using Symbols = std::map<std::string, GiNaC::symbol>;
Dictionary dictionary;
Symbols symbols;
int IP = 1; // Interpretation Pointer.
int g_argc = 0;
char** g_argv; // words to excute

// Forth primitives

static void times()
{
    GiNaC::ex e1 = stacks.pop<GiNaC::ex>();
    GiNaC::ex e2 = stacks.pop<GiNaC::ex>();
    std::cout << "  times: " << (e1 * e2) << std::endl;
    stacks.push<GiNaC::ex>(e1 * e2);
}

static void add()
{
    GiNaC::ex e1 = stacks.pop<GiNaC::ex>();
    GiNaC::ex e2 = stacks.pop<GiNaC::ex>();
    std::cout << "  add: " << (e1 + e2) << std::endl;
    stacks.push<GiNaC::ex>(e1 + e2);
}

static void diff()
{
    GiNaC::ex e = stacks.pop<GiNaC::ex>();
    GiNaC::symbol x = symbols[g_argv[++IP]];
    std::cout << "  diff: " << e << " (d" << x << ")" << std::endl;
    e = GiNaC::diff(e, x);
    stacks.push<GiNaC::ex>(e);
}

static void subs()
{
   GiNaC::ex v = stacks.pop<GiNaC::ex>();
   GiNaC::ex e = stacks.pop<GiNaC::ex>();
   GiNaC::symbol x = symbols[g_argv[++IP]];
   std::cout << "  subs: " << e << " (" << x << " == " << v << ")" << std::endl;
   e = e.subs(x == v);
   stacks.push<GiNaC::ex>(e);
}

static void disp()
{
   std::cout << "  disp: " << stacks.tos<GiNaC::ex>() << std::endl;
}

// Forth dictionary
void create_dictionary(Dictionary& dictionary)
{
    dictionary =
    {
        { "=", subs },
        { "*", times },
        { "+", add },
        { "diff", diff },
        { ".", disp },
    };
}

// Forth interpreter util
static bool isInteger(std::string const& word, int& val)
{
    bool isNumber = true;
    for(std::string::const_iterator k = word.begin(); k != word.end(); ++k)
        isNumber = (isNumber && isdigit(*k));

    if (isNumber)
        val = strtol(word.c_str(), nullptr, 10);

    return isNumber;
}

// Interpret Forth script
bool parse(Dictionary& dictionary, int argc, char* argv[])
{
    g_argc = argc;
    g_argv = argv;
    int number;

    std::cout << "Running command line script:" << std::endl;
    for (IP = 1; IP < argc; ++IP)
    {
        const char* word = argv[IP];

        std::cout << "Next token is: " << word << std::endl;
        auto it = dictionary.find(word);
        if (it != dictionary.end())
        {
            it->second();
        }
        else if (isInteger(word, number))
        {
            std::cout << "  " << word << " is an integer !" << std::endl;
            stacks.push<GiNaC::ex>(number);
        }
        else
        {
            std::cout << "  Create symbol " << word << std::endl;
            symbols[word] = GiNaC::symbol(word);

            dictionary[word] = [&]() {
                std::cout << "  Call symbol " << word << std::endl;
                stacks.push<GiNaC::ex>(GiNaC::ex(symbols[word]));
            };
        }
    }

    return true;
}

// Compilation:
// g++ --std=c++14 -W -Wall RPN.cpp -o prog `pkg-config ginac --libs --cflags`
//
// Forth interpreter command line Example:
//   ./prog x y    x x "*" y 2 "*" +    diff x .    4 = x .
// Where:
//   x y create two symbols x and y
//   x x * y 2 + create the expression x^2 + 2 y
//   diff x does the derivation on x
//   . print the expression
//   4 = x affect 4 to x
//   . show the result which shall be 8.
int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cerr << "You did not gave a forth script in the command line" << std::endl;
        std::cerr << "Example:" << std::endl;
        std::cerr << "   ./prog x y    x x \"*\" y 2 \"*\" +    diff x .    4 = x ." << std::endl;
        return EXIT_FAILURE;
    }

    create_dictionary(dictionary);
    if (!parse(dictionary, argc, argv))
        return EXIT_FAILURE;

    std::cout << "Bye" << std::endl;
    return EXIT_SUCCESS;
}
