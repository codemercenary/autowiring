/*=============================================================================
    Copyright (c) 2001-2011 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_ADVANCE_09172005_1149)
#define FUSION_ADVANCE_09172005_1149

#include <autoboost/fusion/support/config.hpp>
#include <autoboost/mpl/int.hpp>
#include <autoboost/mpl/if.hpp>
#include <autoboost/mpl/eval_if.hpp>
#include <autoboost/mpl/identity.hpp>
#include <autoboost/fusion/iterator/next.hpp>
#include <autoboost/fusion/iterator/prior.hpp>

namespace autoboost { namespace fusion { namespace advance_detail
{
    // Default advance implementation, perform next(i)
    // or prior(i) N times.

    template <typename Iterator, int N>
    struct forward;

    template <typename Iterator, int N>
    struct next_forward
    {
        typedef typename
            forward<
                typename result_of::next<Iterator>::type
              , N-1
            >::type
        type;
    };

    template <typename Iterator, int N>
    struct forward
    {
        typedef typename
            mpl::eval_if_c<
                (N == 0)
              , mpl::identity<Iterator>
              , next_forward<Iterator, N>
            >::type
        type;

        AUTOBOOST_FUSION_GPU_ENABLED
        static type const&
        call(type const& i)
        {
            return i;
        }

        template <typename I>
        AUTOBOOST_FUSION_GPU_ENABLED
        static type
        call(I const& i)
        {
            return call(fusion::next(i));
        }
    };

    template <typename Iterator, int N>
    struct backward;

    template <typename Iterator, int N>
    struct next_backward
    {
        typedef typename
            backward<
                typename result_of::prior<Iterator>::type
              , N+1
            >::type
        type;
    };

    template <typename Iterator, int N>
    struct backward
    {
        typedef typename
            mpl::eval_if_c<
                (N == 0)
              , mpl::identity<Iterator>
              , next_backward<Iterator, N>
            >::type
        type;

        AUTOBOOST_FUSION_GPU_ENABLED
        static type const&
        call(type const& i)
        {
            return i;
        }

        template <typename I>
        AUTOBOOST_FUSION_GPU_ENABLED
        static type
        call(I const& i)
        {
            return call(fusion::prior(i));
        }
    };

}}}

#endif