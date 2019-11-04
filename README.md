# Refractive PPX

PPX Rewriter for generating [Refractive](https://github.com/tizoc/refractive) lenses and selectors.

## Example

The following declaration:

```reason
module User = {
  [@refractive.derive]
  type t = {
    id: string,
    email: string,
    username: string,
    score: int,
  };
};
```

will generate:

```reason
module User = {
  [@refractive.derive]
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