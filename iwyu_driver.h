// Copyright 2011 Google Inc. All Rights Reserved.
// Author: csilvers@google.com (Craig Silverstein)

#ifndef DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
#define DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_

namespace clang {
class CompilerInstance;
}

// This is in the global namespace to make it easier to call from main().

// Creates a CompilerInstance object based on the commandline
// arguments, or NULL if there's an error of some sort.
clang::CompilerInstance* CreateCompilerInstance(int argc, const char **argv);

#endif  // DEVTOOLS_MAINTENANCE_INCLUDE_WHAT_YOU_USE_IWYU_DRIVER_H_
