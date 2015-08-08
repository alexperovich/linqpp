Linq++
==========
Linq for C++

Linq++ is a C++ implementation of the C# Linq functions over stl-like containers. All the functions are implemented using templates with no external dependencies outside the standard libraries. The implementation is header only, and can be easily incorporated into other projects.

## Usage

```C++
#include <enumerable.hpp>
#include <iostream>

using namespace linq;
using namespace std;

int main()
{
  vector<int> input{
    0,1,2,3,4,5
  };

  auto result = enumerable(input)
    .select([](int value)
    {
      return value * 5;
    })
    .where([](int value)
    {
      return value % 2 == 0;
    })
    .select_many([](int value)
    {
      return vector<int>{value, value + 1};
    })
    .to_vector();

  cout << "[";
  bool first = true;
  for (auto element : result)
  {
    if (first)
      first = false;
    else
      cout << ", ";
    cout << element;
  }
  cout << "]" << endl;
  return 0;
}
```
  Output:
  [0, 1, 10, 11, 20, 21]
