// Copyright (c) Alex Perovich. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once
#include <iterator>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace linq
{
  class invalid_operation : public std::exception
  {
  public:
    invalid_operation()
    {
    }

    virtual char const* what() const override
    {
      return "The operation is invalid for the current state of the object.";
    }
  };


  namespace detail
  {
    template <typename L, typename R>
    struct same
    {
      static const bool value = false;
    };

    template <typename T>
    struct same<T, T>
    {
      static const bool value = true;
    };

    template <typename TCollection>
  	struct SourceEnumerator
  	{
      using const_iterator = typename TCollection::const_iterator;
      using value_type = typename std::iterator_traits<const_iterator>::value_type;

  	  explicit SourceEnumerator(const TCollection& collection)
  	    : _collection(collection), _started(false)
  	  {
  	  }

  	  explicit SourceEnumerator(TCollection&& collection)
  	    : _collection(collection), _started(false)
  	  {
  	  }

  	  bool move_next()
  	  {
  	    if (!_started)
  	    {
  	      _started = true;
          _current = begin(_collection);
          _end = end(_collection);
  	    }
        else
        {
          if (_current == _end)
            return false;
  	      ++_current;
        }
  	    return _current != _end;
  	  }

  	  const value_type& current() const
  	  {
  	    if (!_started)
  	      throw invalid_operation();
  	    if (_current == _end)
  	      throw invalid_operation();
  	    return *_current;
  	  }

      void reset()
  	  {
        _started = false;
  	  }
  	private:
      TCollection _collection;
  	  bool _started;
  	  const_iterator _current;
  	  const_iterator _end;
  	};

    template <typename TFirst, typename TSecond>
    struct ConcatEnumerator
    {
      using first_type = typename TFirst::value_type;
      using second_type = typename TSecond::value_type;
      using value_type = typename std::enable_if<same<first_type, second_type>::value, first_type>::type;

      ConcatEnumerator(TFirst first, TSecond second)
        : _first(first), _first_has_current(false), _second(second), _second_has_current(false)
      {
      }

  	  bool move_next()
  	  {
        if (_first.move_next())
        {
          _first_has_current = true;
          return true;
        }
	      _first_has_current = false;
	      if (_second.move_next())
        {
          _second_has_current = true;
          return true;
        }
        _second_has_current = false;
        return false;
  	  }

  	  const value_type& current() const
  	  {
        if (_first_has_current)
          return _first.current();
        if (_second_has_current)
          return _second.current();
        throw invalid_operation();
  	  }

      void reset()
  	  {
        _first.reset();
        _second.reset();
        _first_has_current = false;
        _second_has_current = false;
  	  }
    private:
      TFirst _first;
      bool _first_has_current;
      TSecond _second;
      bool _second_has_current;
    };

    template <typename TSelector, typename TEnumerator>
    struct SelectEnumerator
    {
      using TSource = typename TEnumerator::value_type;
      using value_type = typename std::decay<decltype(std::declval<TSelector>()(std::declval<TSource>()))>::type;

      SelectEnumerator(const TEnumerator& enumerator, TSelector resultSelector)
        : _enumerator(enumerator), _resultSelector(resultSelector)
      {
      }

      SelectEnumerator(TEnumerator&& enumerator, TSelector resultSelector)
        : _enumerator(enumerator), _resultSelector(resultSelector)
      {
      }

  	  bool move_next()
  	  {
        auto value = _enumerator.move_next();
        if (value)
        {
          _has_current = true;
          _current = _resultSelector(_enumerator.current());
        }
        else
        {
          _has_current = false;
          _current = value_type{};
        }
        return value;
  	  }

  	  value_type current() const
  	  {
        return _current;
  	  }

      void reset()
  	  {
        _enumerator.reset();
  	  }
    private:
      bool _has_current;
      value_type _current;
      TEnumerator _enumerator;
      TSelector _resultSelector;
    };

    template <typename TSelector, typename TEnumerator>
    struct SelectManyEnumerator
    {
      using source_type = typename TEnumerator::value_type;
      using collection_type = typename std::decay<typename std::result_of<TSelector(source_type)>::type>::type;
      using value_type = typename std::decay<decltype(*begin(std::declval<collection_type>()))>::type;

      SelectManyEnumerator(const TEnumerator& enumerator, TSelector resultSelector)
        : _enumerator(enumerator), _resultSelector(resultSelector), _current(nullptr)
      {
      }

      SelectManyEnumerator(TEnumerator&& enumerator, TSelector resultSelector)
        : _enumerator(enumerator), _resultSelector(resultSelector), _current(nullptr)
      {
      }

      SelectManyEnumerator(const SelectManyEnumerator<TSelector, TEnumerator>& other)
        : _enumerator(other._enumerator), _resultSelector(other._resultSelector), _current(other._current ? std::make_unique<SourceEnumerator<collection_type>>(*other._current) : std::unique_ptr<SourceEnumerator<collection_type>>(nullptr))
      {
      }

  	  bool move_next()
  	  {
        if (_current)
        {
          if (_current->move_next())
            return true;
          _current = nullptr;
        }
        while (_enumerator.move_next())
        {
          auto value = _enumerator.current();
          auto source = _resultSelector(value);
          _current = std::make_unique<SourceEnumerator<collection_type>>(source);
          if (_current->move_next())
          {
            return true;
          }
          _current = nullptr;
        }
        return false;
  	  }

  	  const value_type& current() const
  	  {
        if (_current)
        {
          return _current->current();
        }
        throw invalid_operation();
  	  }

      void reset()
  	  {
        _enumerator.reset();
        _current = nullptr;
  	  }
    private:
      TEnumerator _enumerator;
      TSelector _resultSelector;
      std::unique_ptr<SourceEnumerator<collection_type>> _current;
    };

    template <typename TEnumerator>
    struct TakeEnumerator
    {
      using value_type = typename TEnumerator::value_type;

      TakeEnumerator(const TEnumerator& enumerator, size_t count)
        : _enumerator(enumerator), _count(count)
      {
      }

      TakeEnumerator(TEnumerator&& enumerator, size_t count)
        : _enumerator(enumerator), _count(count)
      {
      }

  	  bool move_next()
  	  {
        if (_i > _count)
          return false;
        ++_i;
        if (_i > _count)
          return false;
        return _enumerator.move_next();
  	  }

  	  const value_type& current() const
  	  {
        if (_i > _count)
          throw invalid_operation();
        return _enumerator.current();
  	  }

      void reset()
  	  {
        _enumerator.reset();
        _i = 0;
  	  }
    private:
      TEnumerator _enumerator;
      size_t _count;
      size_t _i;
    };

    template <typename TEnumerator>
    struct SkipEnumerator
    {
      using value_type = typename TEnumerator::value_type;

      SkipEnumerator(const TEnumerator& enumerator, size_t count)
        : _enumerator(enumerator), _count(count), _started(false)
      {
      }

      SkipEnumerator(TEnumerator&& enumerator, size_t count)
        : _enumerator(enumerator), _count(count), _started(false)
      {
      }

  	  bool move_next()
  	  {
        if (!_started)
        {
          _started = true;
          size_t i = 0;
          bool value;
          while (value = _enumerator.move_next())
          {
            ++i;
            if (i > _count)
              break;
          }
          return value;
        }
        return _enumerator.move_next();
  	  }

  	  const value_type& current() const
  	  {
        if (!_started)
          throw invalid_operation();
        return _enumerator.current();
  	  }

      void reset()
  	  {
        _enumerator.reset();
        _started = false;
  	  }
    private:
      TEnumerator _enumerator;
      size_t _count;
      bool _started;
    };

    template <typename TPredicate, typename TEnumerator>
    struct WhereEnumerator
    {
      using value_type = typename TEnumerator::value_type;

      WhereEnumerator(const TEnumerator& enumerator, TPredicate predicate)
        : _enumerator(enumerator), _predicate(predicate)
      {
      }

      WhereEnumerator(TEnumerator&& enumerator, TPredicate predicate)
        : _enumerator(enumerator), _predicate(predicate)
      {
      }

  	  bool move_next()
  	  {
        while (_enumerator.move_next())
        {
          if (_predicate(_enumerator.current()))
            return true;
        }
        return false;
  	  }

  	  const value_type& current() const
  	  {
        return _enumerator.current();
  	  }

      void reset()
  	  {
        _enumerator.reset();
  	  }
    private:
      TEnumerator _enumerator;
      TPredicate _predicate;
    };

    template <typename TKeySelector, typename TComparer, typename TEnumerator>
    struct OrderByEnumerator
    {
      using value_type = typename TEnumerator::value_type;
      using key_type = typename std::decay<decltype(std::declval<TKeySelector>()(std::declval<value_type>()))>::type;

      OrderByEnumerator(const TEnumerator& enumerator, TKeySelector keySelector, TComparer comp)
        : _enumerator(enumerator), _keySelector(keySelector), _comp(comp), _index(-1)
      {
      }

      OrderByEnumerator(TEnumerator&& enumerator, TKeySelector keySelector, TComparer comp)
        : _enumerator(enumerator), _keySelector(keySelector), _comp(comp), _index(-1)
      {
      }

  	  bool move_next()
  	  {
        if (_index == -1)
        {
          std::map<key_type, value_type, TComparer> sort_map{_comp}; // TODO: add comparrer
          while (_enumerator.move_next())
          {
            auto current = _enumerator.current();
            auto key = _keySelector(current);
            sort_map[key] = current;
          }
          for (auto elem : sort_map)
          {
            _sortedList.emplace_back(elem.second);
          }
          _index = 0;
          return _sortedList.size() != _index;
        }
        else
        {
          auto size = _sortedList.size();
          if (_index < size)
          {
            ++_index;
          }
          return _index < size;
        }
  	  }

  	  const value_type& current() const
  	  {
        return _sortedList[static_cast<unsigned int>(_index)];
  	  }

      void reset()
  	  {
        _index = -1;
        _sortedList.clear();
  	  }
    private:
      TEnumerator _enumerator;
      TKeySelector _keySelector;
      TComparer _comp;
      int64_t _index;
      std::vector<value_type> _sortedList;
    };
  }

  template <typename Enumerator>
  struct Enumerable
  {
    using value_type = typename Enumerator::value_type;

    explicit Enumerable(Enumerator enumerator)
      : _enumerator(enumerator)
    {
    }

    bool move_next()
    {
      return _enumerator.move_next();
    }

    value_type current()
    {
      return _enumerator.current();
    }

    void reset()
    {
      _enumerator.reset();
    }

    std::vector<value_type> to_vector()
    {
      std::vector<value_type> result{};
      _enumerator.reset();
      while (_enumerator.move_next())
      {
        auto current = _enumerator.current();
        result.emplace_back(current);
      }
      return result;
    }

    template <typename TSelector>
    auto select(TSelector resultSelector)
      -> Enumerable<detail::SelectEnumerator<TSelector, Enumerator>>
    {
      auto enumerator = detail::SelectEnumerator<TSelector, Enumerator>(_enumerator, resultSelector);
      return Enumerable<detail::SelectEnumerator<TSelector, Enumerator>>(enumerator);
    }

    template <typename TSelector>
    auto select_many(TSelector resultSelector)
      -> Enumerable<detail::SelectManyEnumerator<TSelector, Enumerator>>
    {
      auto enumerator = detail::SelectManyEnumerator<TSelector, Enumerator>(_enumerator, resultSelector);
      return Enumerable<detail::SelectManyEnumerator<TSelector, Enumerator>>(enumerator);
    }

    template <typename TEnumerator>
    auto concat(Enumerable<TEnumerator> other)
    {
      auto enumerator = detail::ConcatEnumerator<Enumerator, TEnumerator>(_enumerator, other._enumerator);
      return Enumerable<detail::ConcatEnumerator<Enumerator, TEnumerator>>(enumerator);
    }

    template <typename TPredicate>
    auto where(TPredicate predicate)
      -> Enumerable<detail::WhereEnumerator<TPredicate, Enumerator>>
    {
      auto enumerator = detail::WhereEnumerator<TPredicate, Enumerator>(_enumerator, predicate);
      return Enumerable<detail::WhereEnumerator<TPredicate, Enumerator>>(enumerator);
    }

    template <typename TKeySelector, typename TComparer = std::less<typename std::result_of<TKeySelector(value_type)>::type>>
    auto order_by(TKeySelector keySelector, TComparer comp = TComparer{})
      -> Enumerable<detail::OrderByEnumerator<TKeySelector, TComparer, Enumerator>>
    {
      auto enumerator = detail::OrderByEnumerator<TKeySelector, TComparer, Enumerator>(_enumerator, keySelector, comp);
      return Enumerable<detail::OrderByEnumerator<TKeySelector, TComparer, Enumerator>>(enumerator);
    }

    Enumerable<detail::TakeEnumerator<Enumerator>> take(size_t count)
    {
      return Enumerable<detail::TakeEnumerator<Enumerator>>(detail::TakeEnumerator<Enumerator>(_enumerator, count));
    }

    Enumerable<detail::SkipEnumerator<Enumerator>> skip(size_t count)
    {
      return Enumerable<detail::SkipEnumerator<Enumerator>>(detail::SkipEnumerator<Enumerator>(_enumerator, count));
    }
  private:
    Enumerator _enumerator;

    template <typename TEnumerator>
    friend struct Enumerable;
  };

  template <typename Enumerator>
  Enumerable<Enumerator> _enumerable(Enumerator enumerator)
  {
    return Enumerable<Enumerator>(enumerator);
  }

  template <typename T>
  auto enumerable(T&& collection) -> auto
  {
    return _enumerable(detail::SourceEnumerator<typename std::decay<T>::type>(std::forward<T>(collection)));
  }
  
  template <typename T>
  auto range(T start, T end)
  {
    std::vector<T> result{};
    while (start < end)
      result.push_back(start++);
    return enumerable(std::move(result));
  }
}
