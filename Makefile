default:
	esy build
	refmt --parse re --print ml tests/ppx_refractive_test.re | ocamlfind ppx_tools/rewriter "_esy/default/build/default/.ppx/refractive_ppx/ppx.exe --as-ppx" | tee tests/tests.ml.stdout
