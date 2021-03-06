// Copyright (C) 2012-2015 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "AutowiringDebug.h"
#include "Autowired.h"
#include "AutoPacketFactory.h"
#include "demangle.h"
#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <sstream>
#include <thread>

using namespace autowiring;
using namespace autowiring::dbg;

#ifdef _MSC_VER
// On Windows, lambda functions start with the key string "class <"
bool autowiring::dbg::IsLambda(const std::type_info& ti) {
  return demangle(ti)[0] == '<';
}
#else
// On other platforms, try to find lambda functions by the presence of forbidden characters
bool autowiring::dbg::IsLambda(const std::type_info& ti) {
  auto name = demangle(ti);
  for (auto cur : name)
    if (
      !('a' <= cur && cur <= 'z') &&
      !('A' <= cur && cur <= 'Z') &&
      !('0' <= cur && cur <= '9') &&
      cur != '_' &&
      cur != ':' &&
      cur != '<' &&
      cur != '>' &&
      cur != ',' &&
      cur != ' '
    )
      return true;
  return false;
}
#endif

static std::string DemangleWithAutoID(auto_id id) {
  if (id.block->ti)
    return demangle(id.block->ti);

  // Full type information unavailable.  Provide incomplete information instead.
  std::string retVal = demangle(id.block->ti_synth);

  // prefix is at the beginning of the string, skip over it
  static const char prefix[] = "autowiring::s<";

  if (retVal.compare(0, sizeof(prefix) - 1, prefix) == 0) {
    size_t off = sizeof(prefix) - 1;
    size_t end = retVal.find_last_not_of(' ', retVal.find_last_of('>') - 1);
    retVal = retVal.substr(off, end - off + 1);
  }

  // Append an "[i]" to signify that this type is incomplete
  retVal.append(" [i]");
  return retVal;
}

std::string autowiring::dbg::ContextName(void) {
  AutoCurrentContext ctxt;
  return autowiring::demangle(ctxt->GetSigilType());
}

namespace {
  class PrintContext {
  public:
    PrintContext(std::ostream& os, const CoreContext& ctxt) :
      ctxt(ctxt.shared_from_this()),
      os(os)
    {}

    PrintContext(const PrintContext& parent, std::shared_ptr<const CoreContext>&& child) :
      ctxt(std::move(child)),
      pParent(&parent),
      os(parent.os)
    {}

    std::ostream& os;
    const PrintContext* pParent = nullptr;
    const std::shared_ptr<const CoreContext> ctxt;

    void PrintParentIndentation(void) const {
      if (!pParent)
        // Root context, no indentation
        return;

      // Ancestor first, then ourselves
      pParent->PrintParentIndentation();

      if (ctxt->NextSibling())
        os << "|   ";
      else
        os << "    ";
    }

    void PrintAttributeIndentation(void) const {
      if (pParent)
        pParent->PrintAttributeIndentation();

      if (ctxt->FirstChild())
        os << "|   ";
      else
        os << "    ";
    }

    void PrintChildIndentation(void) const {
      if (!pParent)
        // Root requires no indentation
        return;

      // Parent's indentation goes first
      pParent->PrintParentIndentation();

      // Child indentation is a little different, we show a relationship
      // to the item being indented
      if (ctxt->NextSibling())
        os << "|-- ";
      else
        os << "*-- ";
    }
  };
}

static void PrintContextTreeRecursive(PrintContext printCtxt, const std::function<void(PrintContext&)>& prefix) {
  // Show this context's name in the tree
  printCtxt.PrintChildIndentation();
  printCtxt.os << autowiring::demangle(printCtxt.ctxt->GetSigilType());
  if (!printCtxt.pParent)
    printCtxt.os << "(Current Context)";
  printCtxt.os << std::endl;

  // Call prefix routine on the currently enumerated context now that the identifier is printed
  prefix(printCtxt);

  // And now all children:
  for (auto ctxt = printCtxt.ctxt->FirstChild(); ctxt; ctxt = ctxt->NextSibling())
    PrintContextTreeRecursive(
      { printCtxt, ctxt },
      prefix
    );
}

static void PrintContextTreeRecursive(std::ostream& os, const CoreContext& root, std::shared_ptr<CoreContext> ctxt) {
  PrintContextTreeRecursive( { os, root }, [](PrintContext&) {});
}

void autowiring::dbg::PrintContextTree(std::ostream& os) {
  PrintContextTreeRecursive(os, *AutoGlobalContext(), AutoGlobalContext());
}

void autowiring::dbg::PrintContextTree(std::ostream& os, const std::shared_ptr<CoreContext>& ctxt) {
  PrintContextTreeRecursive(os, *ctxt, ctxt);
}

void autowiring::dbg::PrintContextTree(void) {
  PrintContextTreeRecursive(std::cout, *AutoGlobalContext(), AutoGlobalContext());
}

void autowiring::dbg::PrintRunnables(std::ostream& os, CoreContext& ctxt) {
  PrintContextTreeRecursive(
    { os, ctxt },
    [] (PrintContext& printCtxt) {
      std::ostream& os = printCtxt.os;

      // Now all of the runnables in this context
      for (CoreRunnable* runnable : printCtxt.ctxt->GetRunnables()) {
        printCtxt.PrintAttributeIndentation();
        if (runnable->IsRunning())
          os << "[ RUNNING ]";
        else if (runnable->ShouldStop())
          os << "[ STOPPED ]";
        else if (runnable->WasStarted())
          os << "[ STARTED ]";
        else
          os << "[ WAITING ]";

        // If we can get the tid, print that, otherwise just leave it blank
        if (BasicThread* pThread = dynamic_cast<BasicThread*>(runnable)) {
          auto stdThread = std::static_pointer_cast<std::thread>(pThread->GetThread());
          os << "(tid: " << std::setw(5) << std::setfill(' ') << stdThread->get_id() << ")";
        }

        // Type information, in human-readable form
        os << ' ' << autowiring::demangle(typeid(*runnable)) << ' ';
        if (ContextMember* pMember = dynamic_cast<ContextMember*>(runnable))
          os << " (" << pMember->GetName() << ')';
        os << '\n';
      }
    }
  );
}

AutoPacket* autowiring::dbg::CurrentPacket(void) {
  Autowired<AutoPacketFactory> factory;
  if (!factory)
    return nullptr;
  return factory->CurrentPacket().get();
}

static const AutoFilterDescriptor* DescriptorByName(const char* name, const std::vector<AutoFilterDescriptor>& filters) {
  for (const auto& filter : filters) {
    auto type = filter.GetType();
    if (!type)
      continue;

    auto curName = demangle(type);
    if (!curName.compare(name))
      return &filter;
  }
  return nullptr;
}

const AutoFilterDescriptor* autowiring::dbg::DescriptorByName(const char* name) {
  Autowired<AutoPacketFactory> factory;
  if (!factory)
    return nullptr;

  return ::DescriptorByName(name, factory->GetAutoFilters());
}

std::string autowiring::dbg::AutoFilterInfo(const char* name) {
  Autowired<AutoPacketFactory> factory;
  if (!factory)
    return "No factory detected in this context";

  // Obtain all descriptors:
  const std::vector<AutoFilterDescriptor> descs = factory->GetAutoFilters();

  // Need the root descriptor first
  const AutoFilterDescriptor* desc = ::DescriptorByName(name, descs);
  if (!desc)
    return "Filter not found";

  std::ostringstream os;
  std::function<void(const AutoFilterDescriptor&, size_t)> fnCall = [&](const AutoFilterDescriptor& desc, size_t nLevels) {
    auto args = desc.GetAutoFilterArguments();

    for (size_t i = 0; i < desc.GetArity(); i++) {
      // Argument must be an input to the filter we want to know more about
      if (!args[i].is_input)
        continue;

      // Who provides this input?
      for (const auto& providerDesc : descs) {
        auto providerArg = providerDesc.GetArgumentType(args[i].id);
        if (providerArg && providerArg->is_output && !providerArg->is_rvalue) {
          // Need to print information about this provider:
          os << std::string(nLevels, ' ') << demangle(args[i].id) << " " << demangle(providerDesc.GetType()) << std::endl;

          // The current descriptor provides an input to the parent, recurse
          fnCall(providerDesc, nLevels + 1);
        }
      }
    }
  };

  // Root typename print first:
  os << demangle(desc->GetType()) << std::endl;
  fnCall(*desc, 1);

  auto retVal = os.str();
  return retVal;
}

std::vector<std::string> autowiring::dbg::ListRootDecorations(void) {
  Autowired<AutoPacketFactory> factory;
  if (!factory)
    return std::vector<std::string>{};

  // Obtain all descriptors:
  const std::vector<AutoFilterDescriptor> descs = factory->GetAutoFilters();

  // Get all baseline descriptor types, figure out what isn't satisfied:
  std::unordered_set<auto_id> outputs;
  std::unordered_set<auto_id> inputs;

  for (const auto& desc : descs) {
    auto args = desc.GetAutoFilterArguments();
    for (size_t i = 0; i < desc.GetArity(); i++) {
      if (!args[i].is_rvalue)
        (args[i].is_output ? outputs : inputs).insert(args[i].id);
    }
  }

  // Remove all inputs that exist in the outputs set:
  for (auto& output : outputs)
    inputs.erase(output);

  // Any remaining inputs are unsatisfied
  std::vector<std::string> retVal;
  for (auto_id input : inputs)
    if (input)
      retVal.push_back(DemangleWithAutoID(input));
  return retVal;
}

void autowiring::dbg::WriteAutoFilterGraph(std::ostream& os) {
  WriteAutoFilterGraph(os, AutoCurrentContext());
}

std::string autowiring::dbg::AutoFilterGraphStr(void) {
  std::stringstream ss;
  WriteAutoFilterGraph(ss);
  return ss.str();
}

void autowiring::dbg::WriteAutoFilterGraph(std::ostream& os, const std::shared_ptr<CoreContext>& ctxt) {
  return autowiring::dbg::WriteAutoFilterGraph(os, *ctxt);
}

void autowiring::dbg::WriteAutoFilterGraph(std::ostream& os, CoreContext& ctxt) {
  AutowiredFast<AutoPacketFactory> factory(&ctxt);

  // Write opening and closing parts of file. Now, we only need to write edges
  os << "digraph " << autowiring::demangle(ctxt.GetSigilType()) << " {" << std::endl;
  auto exit = MakeAtExit([&os]{
    os << "}" << std::endl;
  });

  // If no factory, this is an empty network
  if (!factory) {
    return;
  }

  // Obtain all descriptors:
  const std::vector<AutoFilterDescriptor> descs = factory->GetAutoFilters();

  std::unordered_map<std::string, int> decorations; // demangled name to ID
  std::unordered_map<std::string, int> filters; // demangled name to ID

  // give each node a unique id
  int idCounter = 0;

  for (const auto& desc : descs) {
    std::string filter = autowiring::demangle(desc.GetType());

    // Assign each filter an ID
    if (filters.find(filter) == filters.end()) {
      filters[filter] = idCounter++;
    }

    auto args = desc.GetAutoFilterArguments();
    for (size_t i = 0; i < desc.GetArity(); i++) {
      const AutoFilterArgument& arg = args[i];

      std::string decoration = DemangleWithAutoID(arg.id);

      // Assign each decoration and ID
      if (decorations.find(decoration) == decorations.end()) {
        decorations[decoration] = idCounter++;
      }

      // Add edge
      if (arg.is_rvalue) {
        os << decorations[decoration] << " -> " << filters[filter] << " [dir=both]";
      } else if (arg.is_input) {
        os << decorations[decoration] << " -> " << filters[filter];
      } else {
        os << filters[filter] << " -> " << decorations[decoration];
      }

      // Label time shifted edges
      if (arg.tshift) {
        os << " [style=dotted label=\"prev=" << arg.tshift << "\"]";
      }

      os << std::endl;
    }
  }

  // Label each filter with its demangled name
  for (const auto& filter : filters) {
    const std::string& name = filter.first;
    int id = filter.second;

    os << id << " [shape=box label=\""<< name << "\"];" << std::endl;
  }

  // Label each decoration with its demangled name
  for (const auto& decoration : decorations) {
    const std::string& name = decoration.first;
    int id = decoration.second;

    os << id << " [shape=oval label=\""<< name << "\"];" << std::endl;
  }
}

void autowiring::dbg::DebugInit(void) {
  static const void* p [] = {
    (void*) AutoFilterGraphStr
  };
  (void)p;
}
