# records

Records are associative collections of heterogenous elements.
They are a very nifty metaprogramming utility.

You can define your own records wherever you need them, like tuples:

```c++
auto genos = make_record(name="genos"s, age=18, friends={"Saitama"s, "Kuseno"s});
assert(name(genos) == "genos");
assert(age(genos) == 18);
assert(friends(genos) == array<string, 2>{"Saitama", "Kuseno"});

// write access too, natch
name(genos) = "Genos";
age(genos)++;
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
    age = rekt::get_set(&person_t::get_age, &person_t::set_age),
    friends = [](person_t const &p) { return poke_server_for_friends(p.name); }
  );
}

person_t genos{ "Genos", 19 }; 
assert(name(genos) == "Genos");
assert(age(genos) == 19);
// friends is a readonly property of person_t, so this would be a compiler error:
//friends(genos) = vector<string>{ "Me" };
```

Since symbols are static information, the compiler is checking every access to a record-
it's impossible to accidentally access an undefined field.

```c++
//email(genos); // compile error; genos doesn't define a field for email
```

... but sometimes the fields you have are not right for the job you need done.
Rekt provides utilities to easily compose and manipulate records, allowing fields to be defined as needed.

```c++
email(genos & (email="genos@heroassociation.jp"s)); // augment with a field for email

assert(age(genos ^ (age="nineteen"s)) == "nineteen"); // override age to a string

auto age_only = take(genos, age);
//name(age_only); // compile error, name was explicitly removed
```

Rekt provides the macro `REKT_SYMBOLS` for defining symbols efficiently.
Symbols defined in different namespaces are distinct so
your symbol `name` will not collide with anyone else's, not even within a single record.

```c++
REKT_SYMBOLS(name, email, age,  friends);
namespace hero { REKT_SYMBOLS(name); }

assert(nameof(name) == "name");
assert(nameof(hero::name) == "hero::name");

auto no_conflict = make_record(name="Genos"s, hero::name="Blond Cyborg"s);
```

Symbols can access their own (fully qualified) names.
This allows most record types to be unpacked from associative containers like parsed JSON objects trivially:

```c++
nlohmann::json packed{
  {"name", "Genos"},
  {"age", 35}
};

person_t genos;
rekt::unpack(packed, &genos);

nlohmann::json repacked;
rekt::pack(genos, &repacked);
assert(packed == repacked);
```
