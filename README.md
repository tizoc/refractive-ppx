# Refractive PPX

PPX Rewriter for generating [Refractive](https://github.com/tizoc/refractive) lenses and selectors.

## How to use

Add a `[@refractive.derive]` annotation to a type declaration (only record types are supported):

```reason
[@refractive.derive]
type t = { ... };
```

The expansion will define two new modules:

- `Lenses` if the type name is `t`, or `<Titlecased-type-name>Lenses` for any other name.
- `Selectors` if the type name is `t`, or `<Titlecased-type-name>Selectors` for any other name.

The `Lenses` module will include one lense declaration for each record field, with the same name.
The `Selectors` module will include one selector declaration for each record field, with the same name. The path will be the same as the name of the field, and a lense of the same name will be used.

For recursive type definitions, the `[@refractive.derive]` has to be added to each type declaration for which lenses and selectors should be generated.

## Example

The following declaration:

```reason
module User = {
  [@refractive.derive]
  type t = {
    id:       string,
    email:    string,
    username: string,
    score:    int,
  };
};
```

gets expanded to:

```reason
module User = {
  type t = {
    id:       string,
    email:    string,
    username: string,
    score:    int,
  };

  module Lenses = {
    let id =
      Refractive.Lens.make(
        ~get=x => x.id,
        ~set=(newVal, x) => {...x, id: newVal},
      );
    let email =
      Refractive.Lens.make(
        ~get=x => x.email,
        ~set=(newVal, x) => {...x, email: newVal},
      );
    let username =
      Refractive.Lens.make(
        ~get=x => x.username,
        ~set=(newVal, x) => {...x, username: newVal},
      );
    let score =
      Refractive.Lens.make(
        ~get=x => x.score,
        ~set=(newVal, x) => {...x, score: newVal},
      );
  };

  module Selectors = {
    let id       = Refractive.Selector.make(~lens=Lenses.id,       ~path=[|"id"|]);
    let email    = Refractive.Selector.make(~lens=Lenses.email,    ~path=[|"email"|]);
    let username = Refractive.Selector.make(~lens=Lenses.username, ~path=[|"username"|]);
    let score    = Refractive.Selector.make(~lens=Lenses.score,    ~path=[|"score"|]);
  };
};
```

For a type named `t`, no prefix is added to the generated modules. Otherwise, the name of the type is titlecased and used as a prefix. A type named `user` will generate modules named `UserLenses` and `UserSelectors`.
