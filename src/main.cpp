// Copyright (c) Alex Perovich. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "enumerable.hpp"
#include <bandit/bandit.h>

using namespace linq;

int main(int argc, char* argv[])
{
  auto result = bandit::run(argc, argv);
  getc(stdin);
  return result;
}
