(lang dune 2.3)

(generate_opam_files true)

(source (github tizoc/refractive-ppx))

(license BSD)

(authors "Bruno Deferrari")
(maintainers "Bruno Deferrari <utizoc@gmail.com>")

(package
 (name refractive_ppx)
 (synopsis "Refractive Lenses PPX rewriter")
 (description "Refractive_ppx generates lense and selector modules for records")
 (depends
  (dune (> 2.0))
  (ocaml (>= 4.2.3))
  (ocaml-migrate-parsetree (>= 1.6.0))
  (ppx_tools_versioned (>= 5.2.3))

  (ocamlfind :with-test)
 ))

