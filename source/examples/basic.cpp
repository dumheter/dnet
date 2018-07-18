#include <iostream>

// ============================================================ //

class Foo
{
public:
  explicit Foo(int val) : m_val(val) {};

  int get() const { return m_val; }

private:
  int m_val;
};

template <typename T>
class Abe
{
public:

  Abe(int val) : m_foo(val)
  {
    //static_assert(std::is_invocable<decltype(m_foo.get()), int>::value, "yes");
  }

private:
  T m_foo;
};

template <typename T>
class has_get
{
  template <typename C>
  static char test(typeof(&C::get));

  template <typename C>
  static long test(...);

public:
  enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// ============================================================ //

int main()
{
  Abe<Foo> abe{4};

  static_assert(has_get<Abe<Foo>>::value, "abe doesnt implement get");

  return 0;
}