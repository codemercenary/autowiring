//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef AUTOBOOST_CONTAINER_CONTAINER_DETAIL_PAIR_HPP
#define AUTOBOOST_CONTAINER_CONTAINER_DETAIL_PAIR_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

#include <autoboost/container/detail/config_begin.hpp>
#include <autoboost/container/detail/workaround.hpp>

#include <autoboost/container/detail/mpl.hpp>
#include <autoboost/container/detail/type_traits.hpp>
#include <autoboost/container/detail/mpl.hpp>
#include <autoboost/container/detail/type_traits.hpp>

#include <utility>   //std::pair
#include <algorithm> //std::swap

#include <autoboost/move/utility_core.hpp>


#ifndef AUTOBOOST_CONTAINER_PERFECT_FORWARDING
#include <autoboost/container/detail/preprocessor.hpp>
#endif

namespace autoboost {
namespace container {
namespace container_detail {

template <class T1, class T2>
struct pair;

template <class T>
struct is_pair
{
   static const bool value = false;
};

template <class T1, class T2>
struct is_pair< pair<T1, T2> >
{
   static const bool value = true;
};

template <class T1, class T2>
struct is_pair< std::pair<T1, T2> >
{
   static const bool value = true;
};

struct pair_nat;

struct piecewise_construct_t { };
static const piecewise_construct_t piecewise_construct = piecewise_construct_t();

/*
template <class T1, class T2>
struct pair
{
    template <class U, class V> pair(pair<U, V>&& p);
    template <class... Args1, class... Args2>
        pair(piecewise_construct_t, tuple<Args1...> first_args,
             tuple<Args2...> second_args);

    template <class U, class V> pair& operator=(const pair<U, V>& p);
    pair& operator=(pair&& p) noexcept(is_nothrow_move_assignable<T1>::value &&
                                       is_nothrow_move_assignable<T2>::value);
    template <class U, class V> pair& operator=(pair<U, V>&& p);

    void swap(pair& p) noexcept(noexcept(swap(first, p.first)) &&
                                noexcept(swap(second, p.second)));
};

template <class T1, class T2> bool operator==(const pair<T1,T2>&, const pair<T1,T2>&);
template <class T1, class T2> bool operator!=(const pair<T1,T2>&, const pair<T1,T2>&);
template <class T1, class T2> bool operator< (const pair<T1,T2>&, const pair<T1,T2>&);
template <class T1, class T2> bool operator> (const pair<T1,T2>&, const pair<T1,T2>&);
template <class T1, class T2> bool operator>=(const pair<T1,T2>&, const pair<T1,T2>&);
template <class T1, class T2> bool operator<=(const pair<T1,T2>&, const pair<T1,T2>&);
*/


template <class T1, class T2>
struct pair
{
   private:
   AUTOBOOST_COPYABLE_AND_MOVABLE(pair)

   public:
   typedef T1 first_type;
   typedef T2 second_type;

   T1 first;
   T2 second;

   //Default constructor
   pair()
      : first(), second()
   {}

   //pair copy assignment
   pair(const pair& x)
      : first(x.first), second(x.second)
   {}

   //pair move constructor
   pair(AUTOBOOST_RV_REF(pair) p)
      : first(::autoboost::move(p.first)), second(::autoboost::move(p.second))
   {}

   template <class D, class S>
   pair(const pair<D, S> &p)
      : first(p.first), second(p.second)
   {}

   template <class D, class S>
   pair(AUTOBOOST_RV_REF_BEG pair<D, S> AUTOBOOST_RV_REF_END p)
      : first(::autoboost::move(p.first)), second(::autoboost::move(p.second))
   {}

   //pair from two values
   pair(const T1 &t1, const T2 &t2)
      : first(t1)
      , second(t2)
   {}

   template<class U, class V>
   pair(AUTOBOOST_FWD_REF(U) u, AUTOBOOST_FWD_REF(V) v)
      : first(::autoboost::forward<U>(u))
      , second(::autoboost::forward<V>(v))
   {}

   //And now compatibility with std::pair
   pair(const std::pair<T1, T2>& x)
      : first(x.first), second(x.second)
   {}

   template <class D, class S>
   pair(const std::pair<D, S>& p)
      : first(p.first), second(p.second)
   {}

   pair(AUTOBOOST_RV_REF_BEG std::pair<T1, T2> AUTOBOOST_RV_REF_END p)
      : first(::autoboost::move(p.first)), second(::autoboost::move(p.second))
   {}

   template <class D, class S>
   pair(AUTOBOOST_RV_REF_BEG std::pair<D, S> AUTOBOOST_RV_REF_END p)
      : first(::autoboost::move(p.first)), second(::autoboost::move(p.second))
   {}

   //piecewise_construct missing
   //template <class U, class V> pair(pair<U, V>&& p);
   //template <class... Args1, class... Args2>
   //   pair(piecewise_construct_t, tuple<Args1...> first_args,
   //        tuple<Args2...> second_args);
/*
   //Variadic versions
   template<class U>
   pair(AUTOBOOST_CONTAINER_PP_PARAM(U, u), typename container_detail::disable_if
         < container_detail::is_pair< typename container_detail::remove_ref_const<U>::type >, pair_nat>::type* = 0)
      : first(::autoboost::forward<U>(u))
      , second()
   {}

   #ifdef AUTOBOOST_CONTAINER_PERFECT_FORWARDING

   template<class U, class V, class ...Args>
   pair(U &&u, V &&v)
      : first(::autoboost::forward<U>(u))
      , second(::autoboost::forward<V>(v), ::autoboost::forward<Args>(args)...)
   {}

   #else

   #define AUTOBOOST_PP_LOCAL_MACRO(n)                                                            \
   template<class U, AUTOBOOST_PP_ENUM_PARAMS(n, class P)>                                        \
   pair(AUTOBOOST_CONTAINER_PP_PARAM(U, u)                                                          \
       ,AUTOBOOST_PP_ENUM(n, AUTOBOOST_CONTAINER_PP_PARAM_LIST, _))                                  \
      : first(::autoboost::forward<U>(u))                             \
      , second(AUTOBOOST_PP_ENUM(n, AUTOBOOST_CONTAINER_PP_PARAM_FORWARD, _))                        \
   {}                                                                                         \
   //!
   #define AUTOBOOST_PP_LOCAL_LIMITS (1, AUTOBOOST_CONTAINER_MAX_CONSTRUCTOR_PARAMETERS)
   #include AUTOBOOST_PP_LOCAL_ITERATE()
   #endif
*/
   //pair copy assignment
   pair& operator=(AUTOBOOST_COPY_ASSIGN_REF(pair) p)
   {
      first  = p.first;
      second = p.second;
      return *this;
   }

   //pair move assignment
   pair& operator=(AUTOBOOST_RV_REF(pair) p)
   {
      first  = ::autoboost::move(p.first);
      second = ::autoboost::move(p.second);
      return *this;
   }

   template <class D, class S>
   typename ::autoboost::container::container_detail::enable_if_c
      < !(::autoboost::container::container_detail::is_same<T1, D>::value &&
          ::autoboost::container::container_detail::is_same<T2, S>::value)
      , pair &>::type
      operator=(const pair<D, S>&p)
   {
      first  = p.first;
      second = p.second;
      return *this;
   }

   template <class D, class S>
   typename ::autoboost::container::container_detail::enable_if_c
      < !(::autoboost::container::container_detail::is_same<T1, D>::value &&
          ::autoboost::container::container_detail::is_same<T2, S>::value)
      , pair &>::type
      operator=(AUTOBOOST_RV_REF_BEG pair<D, S> AUTOBOOST_RV_REF_END p)
   {
      first  = ::autoboost::move(p.first);
      second = ::autoboost::move(p.second);
      return *this;
   }

   //std::pair copy assignment
   pair& operator=(const std::pair<T1, T2> &p)
   {
      first  = p.first;
      second = p.second;
      return *this;
   }

   template <class D, class S>
   pair& operator=(const std::pair<D, S> &p)
   {
      first  = ::autoboost::move(p.first);
      second = ::autoboost::move(p.second);
      return *this;
   }

   //std::pair move assignment
   pair& operator=(AUTOBOOST_RV_REF_BEG std::pair<T1, T2> AUTOBOOST_RV_REF_END p)
   {
      first  = ::autoboost::move(p.first);
      second = ::autoboost::move(p.second);
      return *this;
   }

   template <class D, class S>
   pair& operator=(AUTOBOOST_RV_REF_BEG std::pair<D, S> AUTOBOOST_RV_REF_END p)
   {
      first  = ::autoboost::move(p.first);
      second = ::autoboost::move(p.second);
      return *this;
   }

   //swap
   void swap(pair& p)
   {
      using std::swap;
      swap(this->first, p.first);
      swap(this->second, p.second);
   }
};

template <class T1, class T2>
inline bool operator==(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(x.first == y.first && x.second == y.second);  }

template <class T1, class T2>
inline bool operator< (const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(x.first < y.first ||
                         (!(y.first < x.first) && x.second < y.second)); }

template <class T1, class T2>
inline bool operator!=(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(!(x == y));  }

template <class T1, class T2>
inline bool operator> (const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return y < x;  }

template <class T1, class T2>
inline bool operator>=(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(!(x < y)); }

template <class T1, class T2>
inline bool operator<=(const pair<T1,T2>& x, const pair<T1,T2>& y)
{  return static_cast<bool>(!(y < x)); }

template <class T1, class T2>
inline pair<T1, T2> make_pair(T1 x, T2 y)
{  return pair<T1, T2>(x, y); }

template <class T1, class T2>
inline void swap(pair<T1, T2>& x, pair<T1, T2>& y)
{
   swap(x.first, y.first);
   swap(x.second, y.second);
}

}  //namespace container_detail {
}  //namespace container {


//Without this specialization recursive flat_(multi)map instantiation fails
//because is_enum needs to instantiate the recursive pair, leading to a compilation error).
//This breaks the cycle clearly stating that pair is not an enum avoiding any instantiation.
template<class T>
struct is_enum;

template<class T, class U>
struct is_enum< ::autoboost::container::container_detail::pair<T, U> >
{
   static const bool value = false;
};

template <class T>
struct is_class;

//This specialization is needed to avoid instantiation of pair in
//is_class, and allow recursive maps.
template <class T1, class T2>
struct is_class< ::autoboost::container::container_detail::pair<T1, T2> >
{
   static const bool value = true;
};

#ifdef AUTOBOOST_NO_CXX11_RVALUE_REFERENCES

template<class T1, class T2>
struct has_move_emulation_enabled< ::autoboost::container::container_detail::pair<T1, T2> >
{
   static const bool value = true;
};

#endif

namespace move_detail{

template<class T>
struct is_class_or_union;

template <class T1, class T2>
struct is_class_or_union< ::autoboost::container::container_detail::pair<T1, T2> >
//This specialization is needed to avoid instantiation of pair in
//is_class, and allow recursive maps.
{
   static const bool value = true;
};


}  //namespace move_detail{

}  //namespace autoboost {

#include <autoboost/container/detail/config_end.hpp>

#endif   //#ifndef AUTOBOOST_CONTAINER_DETAIL_PAIR_HPP
