
#ifndef AUTOBOOST_MPL_VECTOR_AUX_POP_BACK_HPP_INCLUDED
#define AUTOBOOST_MPL_VECTOR_AUX_POP_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <autoboost/mpl/pop_back_fwd.hpp>
#include <autoboost/mpl/aux_/config/typeof.hpp>

#if defined(AUTOBOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES)

#   include <autoboost/mpl/vector/aux_/item.hpp>
#   include <autoboost/mpl/vector/aux_/tag.hpp>

namespace autoboost { namespace mpl {

template<>
struct pop_back_impl< aux::vector_tag >
{
    template< typename Vector > struct apply
    {
        typedef v_mask<Vector,0> type;
    };
};

}}

#endif // AUTOBOOST_MPL_CFG_TYPEOF_BASED_SEQUENCES

#endif // AUTOBOOST_MPL_VECTOR_AUX_POP_BACK_HPP_INCLUDED
