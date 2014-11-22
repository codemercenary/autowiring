// Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "AutoConfig.h"
#include "demangle.h"
#include "SlotInformation.h"
#include <regex>
#include <sstream>

static const std::regex namePattern(".*ConfigTypeExtractor<(?:class |struct )?(\\w*)>");
static const std::regex classPattern(".*TypeUnifierComplex<(?:class |struct )?(\\w*)>");

static std::string ExtractFieldName(const std::type_info& ti) {
  std::smatch sm;
  std::regex_match(autowiring::demangle(ti), sm, namePattern);
  assert(sm.size() == 2 && "Regex couldn't find type name");
  return sm.str(1);
}

static std::string CurrentStumpName(void) {
  const SlotInformationStumpBase* cs = SlotInformationStackLocation::CurrentStump();
  if (!cs)
    return std::string();
  
  std::smatch sm;
  std::regex_match(autowiring::demangle(cs->ti), sm, classPattern);
  assert(sm.size() == 2 && "Regex couldn't find class name");
  return sm.str(1);
}

static std::string FormatFieldName(const std::string& cls, const std::string& name) {
  std::stringstream ss;
  ss << cls << "." << name;
  return ss.str();
}

AutoConfigBase::AutoConfigBase(const std::type_info& tiMemberName):
  Class(CurrentStumpName()),
  Name(ExtractFieldName(tiMemberName)),
  Field(FormatFieldName(Class, Name))
{}
