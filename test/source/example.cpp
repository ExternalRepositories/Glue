#include <doctest/doctest.h>

#include <vector>

// clang-format off
#include <glue/value.h>
#include <iostream>

void valueExample() {
  // `glue::Value`s hold an `revisited::Any` type that can store any type of value
  glue::Value value = "Hello glue!";

  // access the any type through the `->` or `*` operator
  // `as<T>()` returns an `std::optional<T>` that is defined if the cast is possible
  std::cout << *value->as<std::string>() << std::endl;

  // `glue::MapValue` is a wrapper for a container that maps strings to values
  // `glue::createAnyMap()` creates a map based on `std::unordered_set`
  // Values also accept lambda functions
  glue::MapValue map = glue::createAnyMap();
  map["myNumber"] = 42;
  map["myString"] = value;
  map["myCallback"] = [](bool a, float b){ return a ? b : -b; };
  map["myMap"] = glue::createAnyMap();

  // use helper functions to cast to maps or callbacks.
  // the result will evaluate to `false` if no cast is possible.
  if (auto f = map["myCallback"].asFunction()) {
    // callbacks are stored as a `revisited::AnyFunction` and can accept both values or `Any` arguments
    // (here `map["myNumber"]` is casted to an `Any` through the `*` operator)
    // `get<T>` returns casts the value to `T` or throws an exception if not possible
    std::cout << f(false, *map["myNumber"]).get<int>() << std::endl;
  }

  // inner maps are also `glue::MapValue`s.
  map["myMap"]["inner"] = "inner value";
}
// clang-format on

// clang-format off
#include <glue/class.h>
#include <glue/context.h>
#include <iostream>
#include <glue/declarations.h>

struct A {
  std::string member;
};

struct B: public A {
  B(std::string value) : A{value} {}
  int method(int v) { return int(member.size()) + v; }
};

void classExample() {
  auto map = glue::createAnyMap();

  // `glue::createClass<T>` is a convience function for declaring maps for class APIs
  // `addConstructor<Args...>()` adds a function that constructs `A` given the argument types `Args...`
  // `.addMember("name", &T::member)` adds a setter (`member`) and getter (`setMember`) function
  map["A"] = glue::createClass<A>()
    .addConstructor<>()
    .addMember("member", &A::member)
  ;

  // classes can be made inheritance aware
  // `setExtends(map)` uses the argument map or callback to retrieve undefined keys
  // `glue::WithBases<A>()` adds implicit conversions to the listed base classes
  // `addMethod("name", &T::method)` adds a method that calls a member function or lambda
  map["B"] = glue::createClass<B>(glue::WithBases<A>())
    .setExtends(map["A"])
    .addConstructor<std::string>()
    .addMethod("method", &B::method)
    .addMethod("lambda", [](const B &b, int x){ return b.member.size() + x; })
  ;

  // contexts collect map class data and can be used to test instance creation
  glue::Context context;
  context.addRootMap(map);

  // `glue::Instance` captures a value and behaves as a class instance
  auto b = context.createInstance(map["B"][glue::keys::constructorKey]("arg"));

  // calls will be treated as member functions
  std::cout << b["member"]().get<std::string>() << std::endl;
  b["setMember"]("new value");
  std::cout << b["lambda"](10).get<int>() << std::endl;

  glue::DeclarationPrinter printer;
  printer.init();
  printer.print(std::cout, map, &context);
}
// clang-format on

TEST_CASE("Example") {
  auto orig_buf = std::cout.rdbuf();
  std::cout.rdbuf(NULL);
  CHECK_NOTHROW(valueExample());
  CHECK_NOTHROW(classExample());
  std::cout.rdbuf(orig_buf);
}
