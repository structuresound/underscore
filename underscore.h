#ifndef UNDERSCORE_H
#define UNDERSCORE_H

#include <algorithm>

namespace _ {

namespace util {

// simply check if has mapped_type
template<typename T>
class IsMappedContainer {
private:
    template<typename U>
    static typename U::mapped_type check(int);

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(0))>::value;
};

template<typename T>
class HasPushBack {
private:
    template<typename U>
    static auto check(U* p) -> decltype(p->push_back(typename U::value_type()), int());

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(nullptr))>::value;
};

template<typename T>
class HasInsert {
private:
    template<typename U>
    static auto check(U* p) -> decltype(p->insert(typename U::value_type()), int());

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(nullptr))>::value;
};

template<typename T>
class HasInsertAfter {
private:
    template<typename U>
    static auto check(U* p) -> decltype(p->insert_after(p->begin(), typename U::value_type()), int()); // begin is just for checking

    template<typename>
    static void check(...);
public:
    static const bool value = !std::is_void<decltype(check<T>(nullptr))>::value;
};

template<typename T, typename U>
typename std::enable_if<HasPushBack<T>::value, void>::type
add(T& c, U&& v) {
    static_assert(std::is_same<typename T::value_type, 
        typename std::decay<U>::type>::value, "util::add - push_back type inconsistent");
    c.push_back(std::forward<U>(v));
}

template<typename T, typename U>
typename std::enable_if<HasInsert<T>::value, void>::type
add(T& c, U&& v) {
    static_assert(std::is_same<typename T::value_type, 
        typename std::decay<U>::type>::value, "util::add - insert type inconsistent");
    c.insert(std::forward<U>(v));
}

template<typename T, typename U>
typename std::enable_if<HasInsertAfter<T>::value, void>::type
add(T& c, U&& v) {
    static_assert(std::is_same<typename T::value_type, 
        typename std::decay<U>::type>::value, "util::add - insert_after type inconsistent");

    // get to the end of the list, which is O(N) and not fast at all
    auto before_end = c.before_begin();
    for (auto& _ : c) {
        ++before_end;
    }
    c.insert_after(before_end, std::forward<U>(v));
}

} // namespace util


template<typename Collection, typename Function> void
each(Collection& obj, Function iterator) {
    std::for_each(std::begin(obj), std::end(obj), iterator);
}


template<template<class T, class Allocator = std::allocator<T>>
         class RetCollection = std::vector,
         typename Collection,
         typename Function> 
auto
map(const Collection& obj, Function iterator)
    -> RetCollection<decltype(iterator(typename Collection::value_type()))> {

    using R = decltype(iterator(typename Collection::value_type()));
    RetCollection<R> result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        util::add(result, iterator(v));
    });
    return result;
}


template<typename Collection, typename Function, typename Memo> Memo
reduce(const Collection& obj, Function iterator, Memo memo) {
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        memo = iterator(memo, v);
    });
    return memo;
}


template<typename Collection, typename Function, typename Memo>
Memo
reduceRight(const Collection& obj, Function iterator, Memo memo) {
    for (typename Collection::const_reverse_iterator it = obj.rbegin(); it != obj.rend(); ++it) {
        memo = iterator(memo, *it);
    }
    return memo;
}


template<typename Collection, typename Function>
auto
find(const Collection& obj, Function iterator)
    -> decltype(std::begin(obj)) {
    return std::find_if(std::begin(obj), std::end(obj), iterator);
}


template<typename Collection, typename Function>
Collection
filter(const Collection& obj, Function iterator) {
    Collection result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        if (iterator(v)) {
            util::add(result, v);
        }
    });
    return result;
}


template<template<class T, class Allocator = std::allocator<T>>
         class RetCollection = std::vector,
         typename Collection,
         typename Function>
Collection
reject(const Collection& obj, Function iterator) {
    Collection result;
    std::for_each(std::begin(obj), std::end(obj), [&](const typename Collection::value_type& v) {
        if (!iterator(v)) {
            util::add(result, v);
        }
    });
    return result;
}


template<typename Collection, typename Function>
bool
every(const Collection& obj, Function iterator) {
    return std::all_of(std::begin(obj), std::end(obj), iterator);
}


template<typename Collection, typename Function>
bool
some(const Collection& obj, Function iterator) {
    return std::any_of(std::begin(obj), std::end(obj), iterator);
}


template<typename Collection> 
bool
contains(const Collection& obj, const typename Collection::value_type& value) {
    return std::find(std::begin(obj), std::end(obj), value) != std::end(obj);
}


template<template<class T, class Allocator = std::allocator<T>>
         class RetCollection = std::vector,
         typename Collection,
         typename Function> 
auto
invoke(const Collection& obj, Function method)
    -> RetCollection<decltype((typename Collection::value_type()).*method())> {
    return _::map(obj, [&](const typename Collection::value_type& v) {
        return v.*method();
    });
}


} // namespace _

#endif // UNDERSCORE_H

