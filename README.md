# records

Records are associative collections of heterogenous elements.
They are a very nifty metaprogramming utility.

You can define your own records wherever you need them, like tuples:

```c++
auto igor = make_record(name="igor"s, age=34, friends={"Frankie"s, "Charles"s});
assert(name(igor) == "igor");
assert(age(igor) == 34);
assert(friends(igor) == array<string, 2>{"Frankie", "Charles"});

// write access too, natch
name(igor) = "Igor";
age(igor)++;
```

You can also adapt other structures as records with minimal boilerplate.
Your structures' properties can be public data members, but that's not required:
Rekt supports get/set functions too.

```c++
#include "person.h"

auto get(rekt::properties, person_t const&)
{
  return make_record(
    name = &person_t::name,
    age = rekt::prop(&person_t::get_age, &person_t::set_age),
    friends = [](person_t const &p) { return poke_server_for_friends(p.name); }
  );
}

person_t igor{ "Igor", 35 }; 
assert(name(igor) == "Igor");
assert(age(igor) == 35);
// friends is a readonly property of person_t, so this would be a compiler error:
//friends(igor) = vector<string>{ "Me" };
```

Since symbols are static information, the compiler is checking every access to a record-
it's impossible to accidentally access an undefined field.

```c++
//email(igor); // compile error; igor doesn't define a field for email
```

... but sometimes the fields you have are not right for the job you need done.
Rekt provides utilities to easily compose and manipulate records, allowing fields to be defined as needed.

```c++
email(igor & (email="igor@svobotnik.net"s)); // augment with a field for email

assert(age(igor ^ (age="thirty five"s)) == "thirty five"); // override age to a string

auto age_only = take(igor, age);
//name(age_only); // compile error, name was explicitly removed
```

Rekt provides the macro `REKT_SYMBOLS` for defining symbols efficiently.
Symbols defined in different namespaces are distinct so
your symbol `name` will not collide with anyone else's, not even within a single record.

```c++
REKT_SYMBOLS(name, email, age,  friends);
namespace gamer { REKT_SYMBOLS(name); }

assert(nameof(name) == "name");
assert(nameof(gamer::name) == "gamer::name");

auto no_conflict = make_record(name="Igor"s, gamer::name="1G0r"s);
```

Symbols can access their own (fully qualified) names.
This allows most record types to be unpacked from associative containers like parsed JSON objects trivially:

```c++
person_t igor = unpack(type_c<person_t>, nlohmann::json{{
  {"name", "Igor"},
  {"age", 35}
});
```
