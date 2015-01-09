/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Olaf Krzikalla 2004-2006.
// (C) Copyright Ion Gaztanaga  2006-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef AUTOBOOST_INTRUSIVE_LIST_NODE_HPP
#define AUTOBOOST_INTRUSIVE_LIST_NODE_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

#include <autoboost/intrusive/pointer_rebind.hpp>

namespace autoboost {
namespace intrusive {

// list_node_traits can be used with circular_list_algorithms and supplies
// a list_node holding the pointers needed for a double-linked list
// it is used by list_derived_node and list_member_node

template<class VoidPointer>
struct list_node
{
   typedef typename pointer_rebind<VoidPointer, list_node>::type  node_ptr;
   node_ptr next_;
   node_ptr prev_;
};

template<class VoidPointer>
struct list_node_traits
{
   typedef list_node<VoidPointer>      node;
   typedef typename node::node_ptr     node_ptr;
   typedef typename pointer_rebind<VoidPointer, const node>::type   const_node_ptr;

   static node_ptr get_previous(const const_node_ptr & n)
   {  return n->prev_;  }

   static node_ptr get_previous(const node_ptr & n)
   {  return n->prev_;  }

   static void set_previous(const node_ptr & n, const node_ptr & prev)
   {  n->prev_ = prev;  }

   static node_ptr get_next(const const_node_ptr & n)
   {  return n->next_;  }

   static node_ptr get_next(const node_ptr & n)
   {  return n->next_;  }

   static void set_next(const node_ptr & n, const node_ptr & next)
   {  n->next_ = next;  }
};

} //namespace intrusive
} //namespace autoboost

#endif //AUTOBOOST_INTRUSIVE_LIST_NODE_HPP