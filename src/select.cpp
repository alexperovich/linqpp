// Copyright (c) Alex Perovich. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "enumerable.hpp"
#include <bandit/bandit.h>

using namespace linq;
using namespace bandit;

go_bandit([]()
{
  describe("enumerable", []()
  {
    describe("select", [] {
      const size_t sizes[] = {
        0,
        1,
        235725,
        10,
        40
      };
      it("should use deferred execution", []()
      {
        bool funcCalled = false;
        auto source = std::vector<std::function<int()>>{
          [&]() {funcCalled = true; return 1;}
        };
        auto result = enumerable(source).select([](std::function<int()> fn)
        {
          return fn();
        });
        AssertThat(funcCalled, IsFalse());
      });
      it("should return expected values", [&]()
      {
        for (auto size : sizes)
        {
          auto input = range(static_cast<size_t>(0), size);
          auto result = input.select([](int value)
          {
            return value + 1;
          })
            .to_vector();
          AssertThat(result.size(), Equals(size));
          for (size_t i = 0; i < result.size(); ++i)
          {
            AssertThat(result[i], Equals(i + 1));
          }
        }
      });
      it("should return default after enumerating", []()
      {
        auto value = range(0, 10).select([](int v) {return v;});
        while(value.move_next()){}
        AssertThat(value.current(), Equals(int{}));
      });
      it("should return expected values when called twice", [&]()
      {
        for (auto size : sizes)
        {
          auto input = range(static_cast<size_t>(0), size);
          auto result = input.select([](int value)
          {
            return value + 1;
          })
            .select([](int value)
          {
            return value + 1;
          })
            .to_vector();
          AssertThat(result.size(), Equals(size));
          for (size_t i = 0; i < result.size(); ++i)
          {
            AssertThat(result[i], Equals(i + 2));
          }
        }
      });
      it("should return no items for an empty input", []()
      {
        std::vector<int> input{};
        bool selectorWasCalled = false;
        auto result = enumerable(input).select([&](int)
        {
          selectorWasCalled = true;
          return 1;
        });
        bool hasItems = false;
        while (result.move_next()) { hasItems = true; }
        AssertThat(hasItems, IsFalse());
        AssertThat(selectorWasCalled, IsFalse());
      });
      it("should propagate exception thrown from selector to caller of move_next()", []()
      {
        auto input = range(0, 10);
        auto result = input.select([](int value)
        {
          throw std::exception();
          return value;
        });
        AssertThrows(std::exception, result.move_next());
      });
    });
  });
});