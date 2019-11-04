module Refractive = {
  module type LENS = {
    type t('parent, 'child);
    let make:
      (~set: ('child, 'parent) => 'parent, ~get: 'parent => 'child) =>
      t('parent, 'child);
    let view: (t('parent, 'child), 'parent) => 'child;
  };

  module Lens: LENS = {
    type t('parent, 'child) = (
      ('child, 'parent) => 'parent,
      'parent => 'child,
    );
    let make = (~set, ~get) => (set, get);
    let view = (lens, parent) => snd(lens, parent);
  };

  module Selector = {
    type t('state, 'value) = {
      lens: Lens.t('state, 'value),
      path: array(string),
    };

    let make = (~lens, ~path) => {lens, path};
  };
};

module Wrapper = {
  module type PERSON = {
    [@refractive.derive]
    type person = {
      age: int,
      name: string,
    };

    [@refractive.derive]
    type t = {
      person: person,
    };
  };

  module Person: PERSON = {
    [@refractive.derive]
    type person = {
      age: int,
      name: string,
    };

    [@refractive.derive]
    type t = {
      person: person,
    };
  };
};

let person = Wrapper.Person.{age: 1, name: "Test success"};

print_endline(Refractive.Lens.view(Wrapper.Person.PersonLenses.name, person));
print_endline(
  String.concat(".", Array.to_list(Wrapper.Person.PersonSelectors.age.path)),
);