// 2021 Quentin Quadrat lecrapouille@gmail.com
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org>

#ifndef UTILS_HPP
#  define UTILS_HPP

#include <iostream>

//------------------------------------------------------------------------------
static inline float arc_length(float const angle, float const radius)
{
    return angle * radius;
}

//------------------------------------------------------------------------------
static void add_reference(std::vector<Pair>& reference, float const value, float const duration)
{
    assert(duration >= 0.0f);

    float time = reference.empty() ? 0.0f : reference.back().time;
    reference.push_back(Pair(value, time + duration));
}

//------------------------------------------------------------------------------
static float get_reference(std::vector<Pair> const& reference, float const time)
{
    if (reference.size() == 0u)
    {
        return 0.0f;
    }

    const size_t N = reference.size() - 1u;
    if (time <= reference[N].time)
    {
        for (size_t i = 0u; i <= N; ++i)
        {
            if (time < reference[i].time)
            {
                return reference[i].data;
            }
        }
    }

    return 0.0f;
}

//------------------------------------------------------------------------------
static void debug(std::vector<Pair> const& references, const char* time_name, const char* value_name)
{
    std::cout << time_name << " = [";
    std::string separator;
    for (auto const& it: references)
    {
       std::cout << separator << it.time;
       separator = ", ";
    }
    std::cout << "]" << std::endl << value_name << " = [";
    separator.clear();
    for (auto const& it: references)
    {
       std::cout << separator << it.data;
       separator = ", ";
    }
    std::cout << "]" << std::endl;
}

#endif
