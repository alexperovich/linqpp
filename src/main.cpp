// Copyright (c) Alex Perovich. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include <bandit/bandit.h>

int main(int argc, char* argv[])
{
  auto result = bandit::run(argc, argv);
  getc(stdin);
  return result;
}
