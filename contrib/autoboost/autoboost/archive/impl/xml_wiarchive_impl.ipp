/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// xml_wiarchive_impl.ipp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <cstring>
#if defined(AUTOBOOST_NO_STDC_NAMESPACE)
namespace std{
    using ::memcpy;
} //std
#endif

#include <autoboost/config.hpp> // msvc 6.0 needs this to suppress warnings
#ifndef AUTOBOOST_NO_STD_WSTREAMBUF

#include <autoboost/assert.hpp>
#include <algorithm> // std::copy

#include <autoboost/detail/workaround.hpp> // Dinkumware and RogueWave
#if AUTOBOOST_WORKAROUND(AUTOBOOST_DINKUMWARE_STDLIB, == 1)
#include <autoboost/archive/dinkumware.hpp>
#endif

#include <autoboost/io/ios_state.hpp>
#include <autoboost/detail/no_exceptions_support.hpp>
#include <autoboost/serialization/pfto.hpp>

#include <autoboost/serialization/string.hpp>
#include <autoboost/archive/add_facet.hpp>
#ifndef AUTOBOOST_NO_CXX11_HDR_CODECVT
    #include <codecvt>
    namespace autoboost { namespace archive { namespace detail {
        typedef std::codecvt_utf8<wchar_t> utf8_codecvt_facet;
    } } }
#else
    #include <autoboost/archive/detail/utf8_codecvt_facet.hpp>
#endif

#include <autoboost/archive/xml_archive_exception.hpp>
#include <autoboost/archive/iterators/mb_from_wchar.hpp>

#include <autoboost/archive/basic_xml_archive.hpp>
#include <autoboost/archive/xml_wiarchive.hpp>

#include "basic_xml_grammar.hpp"

namespace autoboost {
namespace archive {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// implemenations of functions specific to wide char archives

namespace { // anonymous

void copy_to_ptr(char * s, const std::wstring & ws){
    std::copy(
        iterators::mb_from_wchar<std::wstring::const_iterator>(
            AUTOBOOST_MAKE_PFTO_WRAPPER(ws.begin())
        ),
        iterators::mb_from_wchar<std::wstring::const_iterator>(
            AUTOBOOST_MAKE_PFTO_WRAPPER(ws.end())
        ),
        s
    );
    s[ws.size()] = 0;
}

} // anonymous

template<class Archive>
AUTOBOOST_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(std::string & s){
    std::wstring ws;
    bool result = gimpl->parse_string(is, ws);
    if(! result)
        autoboost::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    #if AUTOBOOST_WORKAROUND(_RWSTD_VER, AUTOBOOST_TESTED_AT(20101))
    if(NULL != s.data())
    #endif
        s.resize(0);
    s.reserve(ws.size());
    std::copy(
        iterators::mb_from_wchar<std::wstring::iterator>(
            AUTOBOOST_MAKE_PFTO_WRAPPER(ws.begin())
        ),
        iterators::mb_from_wchar<std::wstring::iterator>(
            AUTOBOOST_MAKE_PFTO_WRAPPER(ws.end())
        ),
        std::back_inserter(s)
    );
}

#ifndef AUTOBOOST_NO_STD_WSTRING
template<class Archive>
AUTOBOOST_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(std::wstring & ws){
    bool result = gimpl->parse_string(is, ws);
    if(! result)
        autoboost::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
}
#endif

template<class Archive>
AUTOBOOST_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(char * s){
    std::wstring ws;
    bool result = gimpl->parse_string(is, ws);
    if(! result)
        autoboost::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    copy_to_ptr(s, ws);
}

#ifndef AUTOBOOST_NO_INTRINSIC_WCHAR_T
template<class Archive>
AUTOBOOST_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load(wchar_t * ws){
    std::wstring twstring;
    bool result = gimpl->parse_string(is, twstring);
    if(! result)
        autoboost::serialization::throw_exception(
            xml_archive_exception(xml_archive_exception::xml_archive_parsing_error)
        );
    std::memcpy(ws, twstring.c_str(), twstring.size());
    ws[twstring.size()] = L'\0';
}
#endif

template<class Archive>
AUTOBOOST_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::load_override(class_name_type & t, int){
    const std::wstring & ws = gimpl->rv.class_name;
    if(ws.size() > AUTOBOOST_SERIALIZATION_MAX_KEY_SIZE - 1)
        autoboost::serialization::throw_exception(
            archive_exception(archive_exception::invalid_class_name)
        );
    copy_to_ptr(t, ws);
}

template<class Archive>
AUTOBOOST_WARCHIVE_DECL(void)
xml_wiarchive_impl<Archive>::init(){
    gimpl->init(is);
    this->set_library_version(
        library_version_type(gimpl->rv.version)
    );
}

template<class Archive>
AUTOBOOST_WARCHIVE_DECL(AUTOBOOST_PP_EMPTY())
xml_wiarchive_impl<Archive>::xml_wiarchive_impl(
    std::wistream &is_,
    unsigned int flags
) :
    basic_text_iprimitive<std::wistream>(
        is_,
        true // don't change the codecvt - use the one below
    ),
    basic_xml_iarchive<Archive>(flags),
    gimpl(new xml_wgrammar())
{
    if(0 == (flags & no_codecvt)){
        archive_locale.reset(
            add_facet(
                is_.getloc(),
                new autoboost::archive::detail::utf8_codecvt_facet
            )
        );
        is.imbue(* archive_locale);
    }
    if(0 == (flags & no_header)){
        AUTOBOOST_TRY{
            this->init();
        }
        AUTOBOOST_CATCH(...){
            delete gimpl;
            #ifndef AUTOBOOST_NO_EXCEPTIONS
                throw; // re-throw
            #endif
        }
        AUTOBOOST_CATCH_END
    }
}

template<class Archive>
AUTOBOOST_WARCHIVE_DECL(AUTOBOOST_PP_EMPTY())
xml_wiarchive_impl<Archive>::~xml_wiarchive_impl(){
    if(0 == (this->get_flags() & no_header)){
        AUTOBOOST_TRY{
            gimpl->windup(is);
        }
        AUTOBOOST_CATCH(...){}
        AUTOBOOST_CATCH_END
    }
    delete gimpl;
}

} // namespace archive
} // namespace autoboost

#endif  // AUTOBOOST_NO_STD_WSTREAMBUF
