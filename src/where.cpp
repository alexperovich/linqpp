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
    describe("where", [] {
      it("should use deferred execution", []()
      {
        bool funcCalled = false;
        auto source = std::vector<std::function<bool()>>{
          [&]() {funcCalled = true; return true;}
        };
        auto result = enumerable(source).where([](std::function<bool()> fn)
        {
          return fn();
        });
        AssertThat(funcCalled, IsFalse());
      });
      it("should return all values if predicate is always true", []()
      {
        auto input = range(0, 10);
        auto result = input.where([](int) {return true;}).to_vector();
        AssertThat(result.size(), Equals(10));
        for (size_t i = 0; i < result.size(); ++i)
        {
          AssertThat(result[i], Equals(i));
        }
      });
      it("should return no values if predicate is always false", []()
      {
        auto input = range(0, 10);
        auto result = input.where([](int) {return false;}).to_vector();
        AssertThat(result.size(), Equals(0));
      });
      it("should return values that satisfy the predicate", []()
      {
        auto input = range(0, 10);
        auto result = input.where([](int value) {return value % 2 == 0;}).to_vector();
        AssertThat(result.size(), Equals(5));
        for (size_t i = 0; i < result.size(); ++i)
        {
          AssertThat(result[i], Equals(i * 2));
        }
      });
      it("should return no items for an empty input", []()
      {
        std::vector<int> input{};
        bool predicateWasCalled = false;
        auto result = enumerable(input).where([&](int)
        {
          predicateWasCalled = true;
          return true;
        });
        bool hasItems = false;
        while (result.move_next()) { hasItems = true; }
        AssertThat(hasItems, IsFalse());
        AssertThat(predicateWasCalled, IsFalse());
      });
      it("should throw after enumerating", []()
      {
        auto value = range(0, 10).where([](int v) {return true;});
        while(value.move_next()){}
        AssertThrows(invalid_operation, value.current());
      });
      it("should propagate exception thrown from predicate to caller of move_next()", []()
      {
        auto input = range(0, 10);
        auto result = input.where([](int value)
        {
          throw std::exception();
          return true;
        });
        AssertThrows(std::exception, result.move_next());
      });
    });
  });
});
