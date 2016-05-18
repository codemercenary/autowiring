// Copyright (C) 2012-2015 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "thread_specific_ptr.h"

namespace autowiring {

void* thread_specific_ptr_base::get() const {
  return FlsGetValue(m_key);
}

void thread_specific_ptr_base::set(void* value) {
  FlsSetValue(m_key, value);
}

void thread_specific_ptr_base::init(void) {
  m_key = FlsAlloc(m_cleanupFunction);
  set(nullptr);
}

void thread_specific_ptr_base::freeTLS(void) {
  FlsFree(m_key);
}

}
