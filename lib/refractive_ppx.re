open Migrate_parsetree;
open Ast_409;
open Ast_mapper;
open Ast_helper;
open Ast_convenience_409;
open Location;
open Parsetree;
open Longident;

let nllid = name => mknoloc(Lident(name));
let mkref = name => Exp.mk(Pexp_ident(mknoloc(Lident(name))));
let mklet = (name, expr) => Vb.mk(pvar(name), expr);

let updatedRecord = (~singleField=false, record, field, value) =>
  Exp.mk(
    Pexp_record(
      [(nllid(field), mkref(value))],
      singleField ? None : Some(mkref(record)),
    ),
  );

let findAttr = find_attr;

let hasAttr = (s, attrs) => findAttr(s, attrs) != None;

let declareModule = (loc, moduleName, signatures) => {
  psig_desc:
    Psig_module({
      pmd_name: {
        txt: moduleName,
        loc,
      },
      pmd_type: {
        pmty_desc: Pmty_signature(signatures),
        pmty_loc: loc,
        pmty_attributes: [],
      },
      pmd_loc: loc,
      pmd_attributes: [],
    }),
  psig_loc: loc,
};

let defineModule = (loc, moduleName, expressions) => {
  let expressions =
    List.map(
      x => {pstr_desc: Pstr_value(Nonrecursive, [x]), pstr_loc: loc},
      expressions,
    );
  {
    pstr_desc:
      Pstr_module({
        pmb_name: {
          txt: moduleName,
          loc,
        },
        pmb_expr: {
          pmod_desc: Pmod_structure(expressions),
          pmod_loc: loc,
          pmod_attributes: [],
        },
        pmb_loc: loc,
        pmb_attributes: [],
      }),
    pstr_loc: loc,
  };
};

let lensDefinition = (~singleField=false, name) => {
  let getter = [%expr x => [%e Exp.field(evar("x"), nllid(name))]];
  let setter = [%expr
    (newVal, x) => [%e updatedRecord(~singleField, "x", name, "newVal")]
  ];
  let mklens = [%expr
    Refractive.Lens.make(~get=[%e getter], ~set=[%e setter])
  ];
  mklet(name, mklens);
};

let selectorDefinition = (lensesModule, name) => {
  let mkselector = [%expr
    Refractive.Selector.make(
      ~lens=[%e evar(lensesModule ++ "." ++ name)],
      ~path=[|[%e str(name)]|],
    )
  ];
  mklet(name, mkselector);
};

let typeNamed = name => Typ.mk(Ptyp_constr(nllid(name), []));

let lensType = (record_name, name, typ) => {
  let lensType = [%type:
    Refractive.Lens.t([%t typeNamed(record_name)], [%t typ])
  ];
  Sig.value(Val.mk(mknoloc(name), lensType));
};

let selectorType = (record_name, name, typ) => {
  let selectorType = [%type:
    Refractive.Selector.t([%t typeNamed(record_name)], [%t typ])
  ];
  Sig.value(Val.mk(mknoloc(name), selectorType));
};

let qualifiedModuleName = (typeName, baseName) =>
  switch (typeName) {
  | "t" => baseName
  | other => String.capitalize_ascii(other) ++ baseName
  };

let refractiveAnnotated = ty =>
  hasAttr("refractive.derive", ty.ptype_attributes);

let deriveStrTypeDecl = (typ_decls, pstr_loc, item) => {
  let moduleDecls =
    typ_decls
    |> List.filter(refractiveAnnotated)
    |> List.map(typeDecl =>
         switch (typeDecl.ptype_kind) {
         | Ptype_record(labels) =>
           let typeName = typeDecl.ptype_name.txt;
           let names =
             labels |> List.map(({pld_name: {txt: name, _}}) => name);
           let singleField = List.length(labels) === 1;
           let lensBindings = List.map(lensDefinition(~singleField), names);
           let selectorBindings =
             List.map(
               selectorDefinition(qualifiedModuleName(typeName, "Lenses")),
               names,
             );
           let lensesModule =
             defineModule(
               pstr_loc,
               qualifiedModuleName(typeName, "Lenses"),
               lensBindings,
             );
           let selectorsModule =
             defineModule(
               pstr_loc,
               qualifiedModuleName(typeName, "Selectors"),
               selectorBindings,
             );
           [lensesModule, selectorsModule];
         | _ =>
           raise_errorf(
             ~loc=pstr_loc,
             "refractive can be derived only for record types",
           )
         }
       )
    |> List.flatten;
  [item] @ moduleDecls;
};

let deriveSigTypeDecl = (typ_decls, psig_loc, item) => {
  let moduleSigDecls =
    typ_decls
    |> List.filter(refractiveAnnotated)
    |> List.map(typeDecl =>
         switch (typeDecl.ptype_kind) {
         | Ptype_record(labels) =>
           let typeName = typeDecl.ptype_name.txt;
           let names =
             labels |> List.map(({pld_name: {txt: name, _}}) => name);
           let types = labels |> List.map(l => l.pld_type);
           let lensTypes = List.map2(lensType(typeName), names, types);
           let selectorTypes =
             List.map2(selectorType(typeName), names, types);
           let lensesModuleSig =
             declareModule(
               psig_loc,
               qualifiedModuleName(typeName, "Lenses"),
               lensTypes,
             );
           let selectorsModuleSig =
             declareModule(
               psig_loc,
               qualifiedModuleName(typeName, "Selectors"),
               selectorTypes,
             );
           [lensesModuleSig, selectorsModuleSig];
         | _ =>
           raise_errorf(
             ~loc=psig_loc,
             "refractive can be derived only for record types",
           )
         }
       )
    |> List.flatten;
  [item] @ moduleSigDecls;
};

let anyRefractiveAnnotation = List.exists(refractiveAnnotated);

let mapper = (_, _) => {
  let structure = (mapper, items) =>
    switch (items) {
    | [
        {pstr_desc: Pstr_type(_recFlag, typ_decls), pstr_loc} as item,
        ...rest,
      ]
        when anyRefractiveAnnotation(typ_decls) =>
      let derived =
        Ast_helper.with_default_loc(pstr_loc, () =>
          deriveStrTypeDecl(typ_decls, pstr_loc, item)
        );
      derived @ mapper.structure(mapper, rest);
    | [item, ...rest] =>
      let derived = mapper.structure_item(mapper, item);
      [derived, ...mapper.structure(mapper, rest)];
    | [] => []
    };
  let signature = (mapper, items) =>
    switch (items) {
    | [
        {psig_desc: Psig_type(_recFlag, typ_decls), psig_loc} as item,
        ...rest,
      ]
        when anyRefractiveAnnotation(typ_decls) =>
      let derived =
        Ast_helper.with_default_loc(psig_loc, () =>
          deriveSigTypeDecl(typ_decls, psig_loc, item)
        );
      derived @ mapper.signature(mapper, rest);
    | [item, ...rest] =>
      let derived = mapper.signature_item(mapper, item);
      [derived, ...mapper.signature(mapper, rest)];
    | [] => []
    };
  {
    ...default_mapper,
    structure: (mapper, items) => {
      structure({...mapper, structure, signature}, items);
    },
    signature: (mapper, items) => {
      signature({...mapper, structure, signature}, items);
    },
  };
};

Driver.register(~name="refractive", Versions.ocaml_409, mapper);