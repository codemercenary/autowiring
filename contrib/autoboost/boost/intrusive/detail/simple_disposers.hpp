/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2014-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_DETAIL_SIMPLE_DISPOSERS_HPP
#define BOOST_INTRUSIVE_DETAIL_SIMPLE_DISPOSERS_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

namespace autoboost {
namespace intrusive {
namespace detail {

class null_disposer
{
   public:
   template <class Pointer>
   void operator()(Pointer)
   {}
};

template<class NodeAlgorithms>
class init_disposer
{
   typedef typename NodeAlgorithms::node_ptr node_ptr;

   public:
   void operator()(const node_ptr & p)
   {  NodeAlgorithms::init(p);   }
};

}  //namespace detail{
}  //namespace intrusive{
}  //namespace autoboost{

#endif //BOOST_INTRUSIVE_DETAIL_SIMPLE_DISPOSERS_HPP
