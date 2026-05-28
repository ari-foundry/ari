# Test targets are included by the repository-root Makefile.
.PHONY: check check-debug check-sanitize check-sanitize-run check-language-docs check-std-api check-stdlib-docs check-compiler-artifacts check-source-map-unit check-layout-unit check-compiler-docs check-bootstrap-readiness check-cli check-tools check-prelude check-core-language check-control-flow check-match check-modules check-operators check-generics check-meta check-traits check-structs check-variables check-ownership check-ffi check-functions check-function-errors check-errors

check-debug: TARGET := $(DEBUG_TARGET)
check-debug: $(DEBUG_TARGET) check

TOOL_ENV ?=
TOOL_ARI ?= $(TARGET)

check-sanitize: TARGET := env ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 $(SANITIZE_TARGET)
check-sanitize: TOOL_ENV := env ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
check-sanitize: TOOL_ARI := $(SANITIZE_TARGET)
check-sanitize: $(SANITIZE_TARGET)
	+$(MAKE) check-sanitize-run

check-sanitize-run: TARGET := env ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 $(SANITIZE_TARGET)
check-sanitize-run: TOOL_ENV := env ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
check-sanitize-run: TOOL_ARI := $(SANITIZE_TARGET)
check-sanitize-run: check

check: check-language-docs check-std-api check-stdlib-docs check-compiler-artifacts check-compiler-docs check-bootstrap-readiness $(TARGET) check-cli check-tools check-prelude check-core-language check-control-flow check-match check-modules check-operators check-generics check-meta check-traits check-structs check-variables check-ownership check-ffi check-functions check-errors
	mkdir -p $(BUILD_DIR)/ir
	$(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/ir/count.ll
	grep -q 'define i64 @"_ARNv4main"' $(BUILD_DIR)/ir/count.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-io.ari --emit-llvm $(BUILD_DIR)/ir/prelude-io.ll
	grep -q "declare i32 @printf" $(BUILD_DIR)/ir/prelude-io.ll
	$(TARGET) tests/cases/standard-library/ok/format/format-print.ari --emit-llvm $(BUILD_DIR)/ir/format-print.ll
	grep -q "@ari_builtin_write_i64" $(BUILD_DIR)/ir/format-print.ll
	grep -q "@ari_builtin_newline" $(BUILD_DIR)/ir/format-print.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/format-print.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/format-print.elf; $(BUILD_DIR)/format-print.elf >$(BUILD_DIR)/format-print.out; grep -qx "value=-3 ok=true byte=A" $(BUILD_DIR)/format-print.out; grep -qx "{literal} 12" $(BUILD_DIR)/format-print.out; grep -qx "qualified=2" $(BUILD_DIR)/format-print.out; grep -qx 'text=ari quoted="ari"' $(BUILD_DIR)/format-print.out; grep -qx "owned=owned" $(BUILD_DIR)/format-print.out; grep -qx 'owned-debug="debug-owned"' $(BUILD_DIR)/format-print.out; grep -qx "debug=false" $(BUILD_DIR)/format-print.out; fi
	$(TARGET) tests/cases/standard-library/ok/format/format-eprintln.ari --emit-llvm $(BUILD_DIR)/ir/format-eprintln.ll
	grep -q "@fprintf" $(BUILD_DIR)/ir/format-eprintln.ll
	grep -q "@stderr" $(BUILD_DIR)/ir/format-eprintln.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/format-eprintln.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/format-eprintln.elf; $(BUILD_DIR)/format-eprintln.elf >$(BUILD_DIR)/format-eprintln.out 2>$(BUILD_DIR)/format-eprintln.err; test ! -s $(BUILD_DIR)/format-eprintln.out; grep -qx 'macro=err n=-9 ok=true byte=! pi=2.5 dbg="ari"' $(BUILD_DIR)/format-eprintln.err; grep -qx "std=7" $(BUILD_DIR)/format-eprintln.err; grep -qx 'owned="err-owned"' $(BUILD_DIR)/format-eprintln.err; grep -qx "call=3" $(BUILD_DIR)/format-eprintln.err; fi
	$(TARGET) tests/cases/standard-library/ok/format/format-print-u64.ari --emit-llvm $(BUILD_DIR)/ir/format-print-u64.ll
	grep -q "@ari_builtin_write_u64" $(BUILD_DIR)/ir/format-print-u64.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/format-print-u64.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/format-print-u64.elf; $(BUILD_DIR)/format-print-u64.elf >$(BUILD_DIR)/format-print-u64.out; grep -qx "u64=18446744073709551615 zero=0" $(BUILD_DIR)/format-print-u64.out; grep -qx "small=7 max=18446744073709551615" $(BUILD_DIR)/format-print-u64.out; fi
	$(TARGET) tests/cases/standard-library/ok/format/format-float-precision.ari --emit-llvm $(BUILD_DIR)/ir/format-float-precision.ll
	grep -F -q "\\25\\2E\\32\\66" $(BUILD_DIR)/ir/format-float-precision.ll
	grep -q "fpext float" $(BUILD_DIR)/ir/format-float-precision.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/format-float-precision.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/format-float-precision.elf; $(BUILD_DIR)/format-float-precision.elf >$(BUILD_DIR)/format-float-precision.out; grep -q "pi=3.14 default=1.500000" $(BUILD_DIR)/format-float-precision.out; grep -q "whole=6" $(BUILD_DIR)/format-float-precision.out; fi
	$(TARGET) tests/cases/standard-library/ok/context/context-arg.ari --emit-llvm $(BUILD_DIR)/ir/context-arg.ll
	grep -q "@ari_builtin_context_arg" $(BUILD_DIR)/ir/context-arg.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-puts.ari --emit-llvm $(BUILD_DIR)/ir/ffi-c-puts.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ir/ffi-c-puts.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-module.ari --emit-llvm $(BUILD_DIR)/ir/ffi-c-module.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ir/ffi-c-module.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-alias.ari --emit-llvm $(BUILD_DIR)/ir/ffi-c-alias.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ir/ffi-c-alias.ll
	$(TARGET) tests/cases/ffi/ok/library-export.ari --shared --emit-llvm $(BUILD_DIR)/ir/library-export.ll
	grep -q 'define i64 @"_ARNv3add"' $(BUILD_DIR)/ir/library-export.ll
	if grep -q "__P" $(BUILD_DIR)/ir/library-export.ll; then exit 1; fi
	if grep -q "__R" $(BUILD_DIR)/ir/library-export.ll; then exit 1; fi
	$(TARGET) examples/count.ari -o $(BUILD_DIR)/count.elf
	$(BUILD_DIR)/count.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/functions/ok/own-return.ari -o $(BUILD_DIR)/own-return.elf
	$(BUILD_DIR)/own-return.elf; code=$$?; test $$code -eq 3
	$(TARGET) tests/cases/operators/ok/bitwise.ari -o $(BUILD_DIR)/bitwise.elf
	$(BUILD_DIR)/bitwise.elf; code=$$?; test $$code -eq 2
	$(TARGET) tests/cases/control-flow/ok/control-flow-llvm.ari -o $(BUILD_DIR)/control-flow-llvm.elf
	$(BUILD_DIR)/control-flow-llvm.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/control-flow/ok/for-vector-llvm.ari -o $(BUILD_DIR)/for-vector-llvm.elf
	$(BUILD_DIR)/for-vector-llvm.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/modules/ok/module-llvm.ari -o $(BUILD_DIR)/module-llvm.elf
	$(BUILD_DIR)/module-llvm.elf; code=$$?; test $$code -eq 3

check-language-docs:
	python3 tests/check_language_docs.py

check-std-api:
	python3 tests/check_std_api_manifest.py

check-stdlib-docs:
	python3 tests/check_stdlib_docs.py

check-source-map-unit:
	mkdir -p $(BUILD_DIR)/source-map
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Isrc tests/source_map_unit.cpp src/common.cpp -o $(BUILD_DIR)/source-map/source-map-unit$(EXEEXT)
	$(BUILD_DIR)/source-map/source-map-unit$(EXEEXT)

check-layout-unit:
	mkdir -p $(BUILD_DIR)/layout
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Isrc tests/layout_unit.cpp src/layout.cpp src/aggregate_abi.cpp src/target.cpp src/common.cpp -o $(BUILD_DIR)/layout/layout-unit$(EXEEXT)
	$(BUILD_DIR)/layout/layout-unit$(EXEEXT)

check-compiler-artifacts: $(TARGET) check-source-map-unit
	mkdir -p $(BUILD_DIR)/compiler-development/artifact
	ARI="$(TOOL_ARI)" $(TOOL_ENV) python3 tests/check_compiler_artifact_cli.py
	ARI="$(TOOL_ARI)" $(TOOL_ENV) python3 tests/check_compiler_capability_cli.py
	ARI="$(TOOL_ARI)" $(TOOL_ENV) python3 tests/check_compiler_pass_cli.py
	ARI="$(TOOL_ARI)" $(TOOL_ENV) python3 tests/check_compiler_test_bucket_cli.py
	ARI="$(TOOL_ARI)" $(TOOL_ENV) python3 tests/check_compiler_work_item_cli.py
	python3 tests/check_compiler_artifacts.py
	python3 tests/check_compiler_artifacts.py --list-fixtures >$(BUILD_DIR)/compiler-development/artifact/artifact-fixtures.inventory
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/artifact-fixtures.inventory $(BUILD_DIR)/compiler-development/artifact/artifact-fixtures.inventory
	python3 tests/check_compiler_artifacts.py --list-fixtures ok >$(BUILD_DIR)/compiler-development/artifact/artifact-fixtures-ok.inventory
	grep -q 'CompilerArtifactFixtures version=1 group=ok' $(BUILD_DIR)/compiler-development/artifact/artifact-fixtures-ok.inventory
	if grep -q ' group=errors ' $(BUILD_DIR)/compiler-development/artifact/artifact-fixtures-ok.inventory; then exit 1; fi
	python3 tests/materialize_crlf_fixture.py tests/cases/compiler-development/artifact/ok/source-map-crlf-template.ari $(BUILD_DIR)/compiler-development/artifact/source-map-crlf.ari
	python3 tests/materialize_crlf_fixture.py tests/cases/compiler-development/artifact/ok/token-dump-crlf-string-continuation-template.ari $(BUILD_DIR)/compiler-development/artifact/token-dump-crlf-string-continuation.ari
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --emit-tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-basic.tokens
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/token-dump-basic.tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-basic.tokens
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-lexical-surface.ari --emit-tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-lexical-surface.tokens
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/token-dump-lexical-surface.tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-lexical-surface.tokens
	$(TARGET) $(BUILD_DIR)/compiler-development/artifact/source-map-crlf.ari --emit-tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-crlf.tokens
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/token-dump-crlf.tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-crlf.tokens
	$(TARGET) $(BUILD_DIR)/compiler-development/artifact/token-dump-crlf-string-continuation.ari --emit-tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-crlf-string-continuation.tokens
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/token-dump-crlf-string-continuation.tokens $(BUILD_DIR)/compiler-development/artifact/token-dump-crlf-string-continuation.tokens
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-dump-basic.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/syntax-dump-basic.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-dump-basic.syntax
	$(TARGET) tests/cases/compiler-development/artifact/ok/syntax-declarations.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-declarations.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/syntax-declarations.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-declarations.syntax
	$(TARGET) tests/cases/compiler-development/artifact/ok/syntax-union-by-field.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-union-by-field.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/syntax-union-by-field.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-union-by-field.syntax
	$(TARGET) tests/cases/compiler-development/artifact/ok/syntax-control-flow.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-control-flow.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/syntax-control-flow.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-control-flow.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-top-level.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-top-level.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-top-level.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-top-level.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-inline-module.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-inline-module.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-inline-module.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-inline-module.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-function-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-function-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-function-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-function-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-call-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-call-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-call-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-call-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-array-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-array-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-array-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-array-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-struct-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-struct-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-struct-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-struct-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-enum-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-enum-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-enum-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-enum-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-function-body.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-function-body.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-function-body.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-function-body.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-function-param.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-malformed-function-param.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-malformed-function-param.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-malformed-function-param.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-nested-block-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-nested-block-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-nested-block-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-nested-block-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-match-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-match-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-match-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-match-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-match-expression-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-match-expression-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-match-expression-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-match-expression-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-struct-literal-close.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-struct-literal-close.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-missing-struct-literal-close.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-missing-struct-literal-close.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-enum-payload.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-enum-payload.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-enum-payload.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-enum-payload.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-attribute-token-tree.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-attribute-token-tree.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-attribute-token-tree.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-attribute-token-tree.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/syntax-recovery-item-macro-token-tree.ari --emit-syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-item-macro-token-tree.syntax
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/syntax-recovery-item-macro-token-tree.syntax $(BUILD_DIR)/compiler-development/artifact/syntax-recovery-item-macro-token-tree.syntax
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-unexpected-character.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-unexpected-character.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-unexpected-character.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unterminated-string.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-string.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unterminated-string.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-string.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unterminated-string.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-string.repeat.diagnostic
	cmp -s $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-string.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-string.repeat.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unterminated-block-comment.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-block-comment.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unterminated-block-comment.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-block-comment.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unterminated-block-comment.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-block-comment.repeat.diagnostic
	cmp -s $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-block-comment.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unterminated-block-comment.repeat.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unicode-escape-eof.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unicode-escape-eof.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unicode-escape-eof.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unicode-escape-eof.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-invalid-unicode-scalar.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-invalid-unicode-scalar.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-lexer-invalid-unicode-scalar.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-invalid-unicode-scalar.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unsupported-string-escape.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unsupported-string-escape.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-lexer-unsupported-string-escape.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-unsupported-string-escape.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-lexer-nonascii-byte.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-nonascii-byte.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-lexer-nonascii-byte.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-lexer-nonascii-byte.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-expected.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-expected.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-expected.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-mismatched-token-tree.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-mismatched-token-tree.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-mismatched-token-tree.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-mismatched-token-tree.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-function-body.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-function-body.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-function-body.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-function-body.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-function-param.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-malformed-function-param.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-function-param.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-malformed-function-param.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-expression.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-malformed-expression.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-expression.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-malformed-expression.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-call-close.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-call-close.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-call-close.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-call-close.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-generic-close.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-generic-close.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-generic-close.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-generic-close.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-type-arg-close.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-type-arg-close.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-type-arg-close.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-type-arg-close.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-angle-type-arg-close.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-angle-type-arg-close.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-missing-angle-type-arg-close.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-missing-angle-type-arg-close.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-block-expression-missing-value.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-block-expression-missing-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-block-expression-missing-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-block-expression-missing-value.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-struct-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-malformed-struct-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-malformed-struct-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-malformed-struct-field.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-union-by-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-union-by-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-union-by-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-union-by-field.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-union-by-missing-selector.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-missing-selector.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-union-by-missing-selector.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-missing-selector.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-union-by-missing-segment.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-missing-segment.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-union-by-missing-segment.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-missing-segment.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-union-by-duplicate-arm.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-duplicate-arm.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-union-by-duplicate-arm.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-duplicate-arm.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-union-by-enum-missing-arm.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-enum-missing-arm.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-union-by-enum-missing-arm.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-enum-missing-arm.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-union-by-enum-unknown-arm.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-enum-unknown-arm.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-union-by-enum-unknown-arm.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-union-by-enum-unknown-arm.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-structural-capability-parameter.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-structural-capability-parameter.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-structural-capability-parameter.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-structural-capability-parameter.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-parser-top-level.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-top-level.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-parser-top-level.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-parser-top-level.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-missing-module.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-missing-module.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-missing-module.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ambiguous-module.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ambiguous-module.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ambiguous-module.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ambiguous-module.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-cyclic-module.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-cyclic-module.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-cyclic-module.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-cyclic-module.diagnostic
	$(TARGET) tests/cases/modules/errors/self-import-main.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-self-import-source-id.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-self-import-source-id.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-self-import-source-id.diagnostic
	$(TARGET) tests/cases/modules/errors/imported-parse-error-main.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-imported-parse-source-id.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-imported-parse-source-id.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-imported-parse-source-id.diagnostic
	$(TARGET) tests/cases/modules/errors/imported-semantic-error-main.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-imported-semantic-source-id.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-imported-semantic-source-id.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-imported-semantic-source-id.diagnostic
	$(TARGET) tests/cases/modules/errors/module-glob-duplicate.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-module-glob-duplicate.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-module-glob-duplicate.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-module-glob-duplicate.diagnostic
	$(TARGET) tests/cases/modules/errors/private-nested-module.ari --no-implicit-std --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-nested-module.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-private-nested-module.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-nested-module.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-unknown-trait.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-unknown-trait.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-unknown-trait.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-trait-bound-missing-impl.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-bound-missing-impl.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-bound-missing-impl.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-bound-missing-impl.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-trait-method-ambiguous.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-method-ambiguous.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-method-ambiguous.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-method-ambiguous.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-impl-missing-method.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-missing-method.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-impl-missing-method.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-missing-method.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-impl-extra-method.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-extra-method.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-impl-extra-method.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-extra-method.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-impl-param-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-param-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-impl-param-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-param-mismatch.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-impl-return-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-return-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-impl-return-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-return-mismatch.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-impl-receiver-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-receiver-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-impl-receiver-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-impl-receiver-mismatch.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-qualified-associated-missing-self.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-qualified-associated-missing-self.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-qualified-associated-missing-self.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-qualified-associated-missing-self.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-qualified-associated-missing-impl.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-qualified-associated-missing-impl.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-qualified-associated-missing-impl.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-qualified-associated-missing-impl.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-qualified-associated-expected-missing-impl.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-qualified-associated-expected-missing-impl.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-qualified-associated-expected-missing-impl.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-qualified-associated-expected-missing-impl.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-explicit-arity.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-explicit-arity.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-explicit-arity.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-explicit-arity.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-explicit-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-explicit-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-explicit-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-explicit-mismatch.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-enum-constructor-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-enum-constructor-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-enum-constructor-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-enum-constructor-mismatch.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-inference-conflict.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-inference-conflict.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-inference-conflict.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-inference-conflict.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-uninferred.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-uninferred.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-uninferred.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-uninferred.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-return-context-uninferred.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-return-context-uninferred.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-return-context-uninferred.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-return-context-uninferred.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-signature-unknown-param.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-signature-unknown-param.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-signature-unknown-param.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-signature-unknown-param.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-specialization-name-conflict.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-specialization-name-conflict.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-specialization-name-conflict.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-specialization-name-conflict.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-function-pointer-uninferred.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-function-pointer-uninferred.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-function-pointer-uninferred.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-function-pointer-uninferred.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-function-pointer-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-function-pointer-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-function-pointer-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-function-pointer-mismatch.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-function-unknown-type-arg.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-function-unknown-type-arg.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-function-unknown-type-arg.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-function-unknown-type-arg.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-return-substitution-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-return-substitution-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-return-substitution-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-return-substitution-mismatch.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-duplicate-param.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-duplicate-param.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-duplicate-param.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-duplicate-param.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-nongeneric-type-args.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-nongeneric-type-args.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-nongeneric-type-args.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-nongeneric-type-args.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-private-function.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-private-function.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-private-function.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-private-function.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-aggregate-nested-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-nested-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-aggregate-nested-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-nested-mismatch.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-aggregate-enum-payload-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-enum-payload-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-aggregate-enum-payload-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-enum-payload-mismatch.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-aggregate-recursive-struct.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-recursive-struct.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-aggregate-recursive-struct.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-recursive-struct.diagnostic
	$(TARGET) tests/cases/generics/errors/generic-aggregate-use-after-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-use-after-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-generic-aggregate-use-after-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-generic-aggregate-use-after-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-type-assignment.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-type-assignment.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-type-assignment.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-type-assignment.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-core-use-before-declaration.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-core-use-before-declaration.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-core-use-before-declaration.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-core-use-before-declaration.diagnostic
	$(TARGET) tests/cases/functions/errors/function-missing-main.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-function-missing-main.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-function-missing-main.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-function-missing-main.diagnostic
	$(TARGET) tests/cases/functions/errors/function-main-params.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-function-main-params.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-function-main-params.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-function-main-params.diagnostic
	$(TARGET) tests/cases/functions/errors/function-main-return-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-function-main-return-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-function-main-return-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-function-main-return-type.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-duplicate-name.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-duplicate-name.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-duplicate-name.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-duplicate-name.diagnostic
	$(TARGET) tests/cases/functions/errors/function-overload-params.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-duplicate-function.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-duplicate-function.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-duplicate-function.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-duplicate-decl.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-duplicate-decl.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-duplicate-decl.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-duplicate-decl.diagnostic
	$(TARGET) tests/cases/traits/errors/trait-duplicate-method.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-duplicate-method.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-trait-duplicate-method.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-trait-duplicate-method.diagnostic
	$(TARGET) tests/cases/traits/errors/duplicate-trait-impl.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-duplicate-trait-impl.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-duplicate-trait-impl.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-duplicate-trait-impl.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-wrong-function-arity.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-wrong-function-arity.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-wrong-function-arity.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-wrong-function-arity.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-method-arity.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-method-arity.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-method-arity.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-method-arity.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-macro-arity.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-macro-arity.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-macro-arity.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-macro-arity.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-wrong-argument-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-wrong-argument-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-wrong-argument-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-wrong-argument-type.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-invalid-return.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-invalid-return.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-invalid-return.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-invalid-return.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-invalid-assignment.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-invalid-assignment.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-invalid-assignment.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-invalid-assignment.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-private-access.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-access.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-private-access.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-access.diagnostic
	$(TARGET) tests/cases/modules/errors/private-module-enum-case.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-enum-case.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-private-enum-case.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-enum-case.diagnostic
	$(TARGET) tests/cases/modules/errors/private-module-struct-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-struct-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-private-struct-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-private-struct-type.diagnostic
	$(TARGET) tests/cases/structs/errors/struct-literal-missing-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-missing-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-struct-literal-missing-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-missing-field.diagnostic
	$(TARGET) tests/cases/structs/errors/struct-literal-extra-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-extra-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-struct-literal-extra-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-extra-field.diagnostic
	$(TARGET) tests/cases/structs/errors/struct-literal-duplicate-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-duplicate-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-struct-literal-duplicate-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-duplicate-field.diagnostic
	$(TARGET) tests/cases/structs/errors/struct-literal-field-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-field-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-struct-literal-field-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-literal-field-type.diagnostic
	$(TARGET) tests/cases/structs/errors/struct-field-unknown.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-field-unknown.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-struct-field-unknown.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-field-unknown.diagnostic
	$(TARGET) tests/cases/structs/errors/struct-field-access-non-struct.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-field-access-non-struct.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-struct-field-access-non-struct.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-struct-field-access-non-struct.diagnostic
	$(TARGET) tests/cases/match/errors/enum-duplicate-case.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-duplicate-case.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-enum-duplicate-case.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-duplicate-case.diagnostic
	$(TARGET) tests/cases/match/errors/enum-payload-count.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-payload-count.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-enum-payload-count.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-payload-count.diagnostic
	$(TARGET) tests/cases/match/errors/enum-constructor-payload-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-constructor-payload-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-enum-constructor-payload-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-constructor-payload-type.diagnostic
	$(TARGET) tests/cases/match/errors/match-duplicate-arm.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-duplicate-arm.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-duplicate-arm.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-duplicate-arm.diagnostic
	$(TARGET) tests/cases/match/errors/match-empty.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-empty.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-empty.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-empty.diagnostic
	$(TARGET) tests/cases/match/errors/match-expression-type-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-expression-type-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-expression-type-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-expression-type-mismatch.diagnostic
	$(TARGET) tests/cases/match/errors/match-range-pattern-order.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-range-pattern-order.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-range-pattern-order.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-range-pattern-order.diagnostic
	$(TARGET) tests/cases/control-flow/errors/while-non-bool.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-condition-non-bool.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-condition-non-bool.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-condition-non-bool.diagnostic
	$(TARGET) tests/cases/operators/errors/logical-non-bool.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-logical-operand-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-logical-operand-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-logical-operand-type.diagnostic
	$(TARGET) tests/cases/operators/errors/not-non-bool.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-logical-not-operand-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-logical-not-operand-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-logical-not-operand-type.diagnostic
	$(TARGET) tests/cases/operators/errors/operators-bitwise-not-bool.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-bitwise-not-operand-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-bitwise-not-operand-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-bitwise-not-operand-type.diagnostic
	$(TARGET) tests/cases/control-flow/errors/if-expression-type-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-if-expression-type-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-if-expression-type-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-if-expression-type-mismatch.diagnostic
	$(TARGET) tests/cases/control-flow/errors/if-let-expression-type-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-if-let-expression-type-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-if-let-expression-type-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-if-let-expression-type-mismatch.diagnostic
	$(TARGET) tests/cases/control-flow/errors/labeled-block-break-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-labeled-block-break-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-labeled-block-break-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-labeled-block-break-type.diagnostic
	$(TARGET) tests/cases/match/errors/match-no-payload.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-no-payload.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-no-payload.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-no-payload.diagnostic
	$(TARGET) tests/cases/match/errors/match-non-enum.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-non-enum.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-non-enum.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-non-enum.diagnostic
	$(TARGET) tests/cases/match/errors/match-nonexhaustive.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-nonexhaustive.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-nonexhaustive.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-nonexhaustive.diagnostic
	$(TARGET) tests/cases/match/errors/match-or-pattern-binding-type.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-or-pattern-binding-type.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-or-pattern-binding-type.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-or-pattern-binding-type.diagnostic
	$(TARGET) tests/cases/match/errors/match-payload-required.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-payload-required.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-payload-required.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-payload-required.diagnostic
	$(TARGET) tests/cases/match/errors/match-struct-pattern-duplicate-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-duplicate-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-duplicate-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-duplicate-field.diagnostic
	$(TARGET) tests/cases/match/errors/match-struct-pattern-missing-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-missing-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-missing-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-missing-field.diagnostic
	$(TARGET) tests/cases/match/errors/match-struct-pattern-unknown-field.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-unknown-field.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-unknown-field.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-unknown-field.diagnostic
	$(TARGET) tests/cases/match/errors/match-struct-pattern-tuple-struct.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-tuple-struct.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-struct-pattern-tuple-struct.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-struct-pattern-tuple-struct.diagnostic
	$(TARGET) tests/cases/match/errors/match-tuple-struct-pattern-arity.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-tuple-struct-pattern-arity.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-tuple-struct-pattern-arity.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-tuple-struct-pattern-arity.diagnostic
	$(TARGET) tests/cases/match/errors/match-unreachable-after-wildcard.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-unreachable-after-wildcard.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-unreachable-after-wildcard.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-unreachable-after-wildcard.diagnostic
	$(TARGET) tests/cases/match/errors/match-unknown-enum-case.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-unknown-enum-case.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-unknown-enum-case.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-unknown-enum-case.diagnostic
	$(TARGET) tests/cases/match/errors/match-wrong-enum-case.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-wrong-enum-case.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-wrong-enum-case.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-wrong-enum-case.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-cpp-removed.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-abi.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-abi.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-abi.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-body.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-body.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-body.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-body.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-generic.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-generic.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-generic.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-generic.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-invalid-link-name.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-invalid-link-name.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-invalid-link-name.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-invalid-link-name.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-varargs-aggregate.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-aggregate.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-varargs-aggregate.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-aggregate.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-varargs-empty.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-empty.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-varargs-empty.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-empty.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-varargs-function-pointer.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-function-pointer.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-varargs-function-pointer.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-function-pointer.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-varargs-nonextern.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-nonextern.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-varargs-nonextern.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-varargs-nonextern.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-void-param.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-void-param.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-extern-void-param.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-extern-void-param.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ffi-nonrepr-aggregate-import.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-nonrepr-aggregate-import.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-nonrepr-aggregate-import.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-nonrepr-aggregate-import.diagnostic
	$(TARGET) tests/cases/ffi/errors/extern-c-large-aggregate-import.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-large-aggregate-import.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-large-aggregate-import.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-large-aggregate-import.diagnostic
	$(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/extern-c-target-aggregate-import.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-target-aggregate-import.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ffi-target-aggregate-import.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ffi-target-aggregate-import.diagnostic
	$(TARGET) tests/cases/ffi/errors/c-header-large-aggregate-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-aggregate-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-aggregate-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-large-aggregate-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-aggregate-by-value.diagnostic
	$(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/c-header-target-aggregate-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-aggregate-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-aggregate-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-target-aggregate-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-aggregate-by-value.diagnostic
	$(TARGET) tests/cases/ffi/errors/c-header-large-array-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-array-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-array-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-large-array-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-array-by-value.diagnostic
	$(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/c-header-target-array-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-array-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-array-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-target-array-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-array-by-value.diagnostic
	$(TARGET) tests/cases/ffi/errors/c-header-large-tuple-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-tuple-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-tuple-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-large-tuple-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-tuple-by-value.diagnostic
	$(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/c-header-target-tuple-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-tuple-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-tuple-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-target-tuple-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-target-tuple-by-value.diagnostic
	$(TARGET) tests/cases/ffi/errors/c-header-large-aggregate-enum-by-value.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-aggregate-enum-by-value.h --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-aggregate-enum-by-value.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-c-header-large-aggregate-enum-by-value.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-c-header-large-aggregate-enum-by-value.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-borrow-conflict.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-borrow-conflict.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-borrow-conflict.diagnostic
	$(TARGET) tests/cases/borrowing/errors/field-borrow-conflict.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-field-borrow-conflict.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-field-borrow-conflict.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-field-borrow-conflict.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-assignment-while-borrowed.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-assignment-while-borrowed.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-assignment-while-borrowed.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-assignment-while-borrowed.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-field-assignment-while-borrowed.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-field-assignment-while-borrowed.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-field-assignment-while-borrowed.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-field-assignment-while-borrowed.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-return-live-owner.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-return-live-owner.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-return-live-owner.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-return-live-owner.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-loop-break-live-owner.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-loop-break-live-owner.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-loop-break-live-owner.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-loop-break-live-owner.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-loop-continue-live-owner.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-loop-continue-live-owner.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-loop-continue-live-owner.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-loop-continue-live-owner.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-use-after-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-use-after-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-use-after-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-use-after-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-borrow-after-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-borrow-after-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-borrow-after-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-borrow-after-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-double-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-double-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-double-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-double-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-move-borrowed-owner.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-move-borrowed-owner.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-move-borrowed-owner.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-move-borrowed-owner.diagnostic
	$(TARGET) tests/cases/match/errors/match-branch-state-mismatch.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-branch-state-mismatch.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-branch-state-mismatch.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-branch-state-mismatch.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-enum-payload-invalid-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-payload-invalid-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-enum-payload-invalid-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-enum-payload-invalid-move.diagnostic
	$(TARGET) tests/cases/match/errors/stored-owned-enum-payload-undropped.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-stored-owned-enum-payload-undropped.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-stored-owned-enum-payload-undropped.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-stored-owned-enum-payload-undropped.diagnostic
	$(TARGET) tests/cases/match/errors/match-runtime-owned-enum-payload-undropped.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-runtime-owned-enum-payload-undropped.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-match-runtime-owned-enum-payload-undropped.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-match-runtime-owned-enum-payload-undropped.diagnostic
	$(TARGET) tests/cases/match/errors/runtime-owned-enum-conditional-payload-slot-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-runtime-owned-enum-conditional-payload-slot-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-runtime-owned-enum-conditional-payload-slot-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-runtime-owned-enum-conditional-payload-slot-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-compact-enum-payload-ref.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-compact-enum-payload-ref.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-compact-enum-payload-ref.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-compact-enum-payload-ref.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ownership-aggregate-enum-payload.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-aggregate-enum-payload.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ownership-aggregate-enum-payload.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-aggregate-enum-payload.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-borrow-aggregate-enum-payload.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-borrow-aggregate-enum-payload.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-borrow-aggregate-enum-payload.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-borrow-aggregate-enum-payload.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ownership-partial-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-partial-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ownership-partial-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-partial-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ownership-vector-dynamic-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-vector-dynamic-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ownership-vector-dynamic-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-vector-dynamic-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ownership-temporary-element-move.ari --emit-diagnostics $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-temporary-element-move.diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/errors/diagnostic-ownership-temporary-element-move.diagnostic $(BUILD_DIR)/compiler-development/artifact/diagnostic-ownership-temporary-element-move.diagnostic
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-diagnostic-catalog $(BUILD_DIR)/compiler-development/artifact/diagnostic-catalog.catalog
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/diagnostic-catalog.catalog $(BUILD_DIR)/compiler-development/artifact/diagnostic-catalog.catalog
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-capability-inventory $(BUILD_DIR)/compiler-development/artifact/capability-inventory.inventory
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/capability-inventory.inventory $(BUILD_DIR)/compiler-development/artifact/capability-inventory.inventory
	$(TARGET) tests/cases/modules/ok/file-module-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-source-map $(BUILD_DIR)/compiler-development/artifact/source-map-file-module.map
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/source-map-file-module.map $(BUILD_DIR)/compiler-development/artifact/source-map-file-module.map
	$(TARGET) tests/cases/compiler-development/artifact/ok/source-map-utf8.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-source-map $(BUILD_DIR)/compiler-development/artifact/source-map-utf8.map
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/source-map-utf8.map $(BUILD_DIR)/compiler-development/artifact/source-map-utf8.map
	$(TARGET) tests/cases/compiler-development/artifact/ok/source-map-utf8.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-source-map $(BUILD_DIR)/compiler-development/artifact/source-map-utf8.repeat.map
	cmp -s $(BUILD_DIR)/compiler-development/artifact/source-map-utf8.map $(BUILD_DIR)/compiler-development/artifact/source-map-utf8.repeat.map
	$(TARGET) tests/cases/compiler-development/artifact/ok/source-map-empty.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-source-map $(BUILD_DIR)/compiler-development/artifact/source-map-empty.map
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/source-map-empty.map $(BUILD_DIR)/compiler-development/artifact/source-map-empty.map
	$(TARGET) $(BUILD_DIR)/compiler-development/artifact/source-map-crlf.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-source-map $(BUILD_DIR)/compiler-development/artifact/source-map-crlf.map
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/source-map-crlf.map $(BUILD_DIR)/compiler-development/artifact/source-map-crlf.map
	$(TARGET) tests/cases/modules/ok/diamond-project-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-module-graph $(BUILD_DIR)/compiler-development/artifact/module-graph-diamond-project.graph
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/module-graph-diamond-project.graph $(BUILD_DIR)/compiler-development/artifact/module-graph-diamond-project.graph
	$(TARGET) tests/cases/modules/ok/file-module-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-module-graph $(BUILD_DIR)/compiler-development/artifact/module-graph-file-module.graph
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/module-graph-file-module.graph $(BUILD_DIR)/compiler-development/artifact/module-graph-file-module.graph
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-module-graph $(BUILD_DIR)/compiler-development/artifact/module-graph-project-compiler.graph
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/module-graph-project-compiler.graph $(BUILD_DIR)/compiler-development/artifact/module-graph-project-compiler.graph
	$(TARGET) tests/cases/modules/ok/same-filename-project-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-module-graph $(BUILD_DIR)/compiler-development/artifact/module-graph-same-filename.graph
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/module-graph-same-filename.graph $(BUILD_DIR)/compiler-development/artifact/module-graph-same-filename.graph
	mkdir -p $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-root $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-packages/nested
	cp tests/cases/modules/ok/package-module-main.ari $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-root/app.ari
	cp tests/packages/package_math.ari $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-packages/package_math.ari
	cp tests/packages/package_header.arih $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-packages/package_header.arih
	cp tests/packages/nested/mod.ari $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-packages/nested/mod.ari
	$(TARGET) -I$(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-stale-packages $(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-stale-root/app.ari --emit-module-cache $(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-stale-source.aricache --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-source.ll
	printf '\n// stale\n' >>$(BUILD_DIR)/compiler-development/artifact/module-cache-stale-packages/package_math.ari
	if $(TARGET) -I$(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-stale-packages $(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-stale-root/app.ari --use-module-cache $(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-stale-source.aricache --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-source-use.ll >$(BUILD_DIR)/compiler-development/artifact/module-cache-stale-source.err.txt 2>&1; then exit 1; fi
	python3 tests/check_compiler_artifacts.py --normalize tests/cases/compiler-development/artifact/errors/module-cache-stale-source.err.txt $(BUILD_DIR)/compiler-development/artifact/module-cache-stale-source.err.txt
	$(TARGET) -Itests/packages --target i686-unknown-linux-gnu tests/cases/modules/ok/package-module-main.ari --emit-module-cache $(BUILD_DIR)/compiler-development/artifact/module-cache-target.aricache --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-cache-target.ll
	if $(TARGET) -Itests/packages --target x86_64-pc-linux-gnu tests/cases/modules/ok/package-module-main.ari --use-module-cache $(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-target.aricache --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-cache-target-stale.ll >$(BUILD_DIR)/compiler-development/artifact/module-cache-target-stale.err.txt 2>&1; then exit 1; fi
	python3 tests/check_compiler_artifacts.py --normalize tests/cases/compiler-development/artifact/errors/module-cache-target-stale.err.txt $(BUILD_DIR)/compiler-development/artifact/module-cache-target-stale.err.txt
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --emit-module-cache $(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import.aricache --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import.ll
	perl -0pe 's/\\nimport\\t\\tpackage_math\\tpackage_math\\ttests\/packages\/package_math\.ari\\t0//' $(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import.aricache >$(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(abspath $(BUILD_DIR))/compiler-development/artifact/module-cache-missing-import-tampered.aricache --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import-use.ll >$(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import.err.txt 2>&1; then exit 1; fi
	python3 tests/check_compiler_artifacts.py --normalize tests/cases/compiler-development/artifact/errors/module-cache-missing-import.err.txt $(BUILD_DIR)/compiler-development/artifact/module-cache-missing-import.err.txt
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --emit-module-metadata $(BUILD_DIR)/compiler-development/artifact/module-metadata-stale.arimeta --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-metadata-stale.ll
	if $(TARGET) -Itests/packages --cfg-feature stale tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(abspath $(BUILD_DIR))/compiler-development/artifact/module-metadata-stale.arimeta --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-metadata-stale-cfg.ll >$(BUILD_DIR)/compiler-development/artifact/module-metadata-stale-cfg.err.txt 2>&1; then exit 1; fi
	python3 tests/check_compiler_artifacts.py --normalize tests/cases/compiler-development/artifact/errors/module-metadata-stale-cfg.err.txt $(BUILD_DIR)/compiler-development/artifact/module-metadata-stale-cfg.err.txt
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --emit-module-metadata $(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source.arimeta --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source.ll
	sed '/source.*package_math.*tests\/packages\/package_math.ari/d' $(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source.arimeta >$(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source-tampered.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(abspath $(BUILD_DIR))/compiler-development/artifact/module-metadata-missing-source-tampered.arimeta --emit-llvm $(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source-check.ll >$(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source.err.txt 2>&1; then exit 1; fi
	python3 tests/check_compiler_artifacts.py --normalize tests/cases/compiler-development/artifact/errors/module-metadata-missing-source.err.txt $(BUILD_DIR)/compiler-development/artifact/module-metadata-missing-source.err.txt
	$(TARGET) tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-declaration-index $(BUILD_DIR)/compiler-development/artifact/declaration-index-basic.decls
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/declaration-index-basic.decls $(BUILD_DIR)/compiler-development/artifact/declaration-index-basic.decls
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-declaration-index $(BUILD_DIR)/compiler-development/artifact/declaration-index-project-compiler.decls
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/declaration-index-project-compiler.decls $(BUILD_DIR)/compiler-development/artifact/declaration-index-project-compiler.decls
	$(TARGET) tests/cases/generics/ok/generic-aggregate-monomorphization.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-declaration-index $(BUILD_DIR)/compiler-development/artifact/declaration-index-generic-aggregate.decls
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/declaration-index-generic-aggregate.decls $(BUILD_DIR)/compiler-development/artifact/declaration-index-generic-aggregate.decls
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-resolved-index $(BUILD_DIR)/compiler-development/artifact/resolved-index-project-compiler.resolved
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/resolved-index-project-compiler.resolved $(BUILD_DIR)/compiler-development/artifact/resolved-index-project-compiler.resolved
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-resolved-index $(BUILD_DIR)/compiler-development/artifact/resolved-index-project-compiler.repeat.resolved
	cmp -s $(BUILD_DIR)/compiler-development/artifact/resolved-index-project-compiler.resolved $(BUILD_DIR)/compiler-development/artifact/resolved-index-project-compiler.repeat.resolved
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-stage-plan $(BUILD_DIR)/compiler-development/artifact/stage-plan-basic.plan
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/stage-plan-basic.plan $(BUILD_DIR)/compiler-development/artifact/stage-plan-basic.plan
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/typed-ir-basic.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/typed-ir-basic.ir $(BUILD_DIR)/compiler-development/artifact/typed-ir-basic.ir
	$(TARGET) tests/cases/compiler-development/artifact/ok/core-language-scalar-flow.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/core-language-scalar-flow.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/core-language-scalar-flow.ir $(BUILD_DIR)/compiler-development/artifact/core-language-scalar-flow.ir
	$(TARGET) tests/cases/traits/ok/trait-minimum-compiler-shaped.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/trait-minimum-static-dispatch.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/trait-minimum-static-dispatch.ir $(BUILD_DIR)/compiler-development/artifact/trait-minimum-static-dispatch.ir
	$(TARGET) tests/cases/generics/ok/generic-function-compiler-shaped.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/generic-function-compiler-shaped.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/generic-function-compiler-shaped.ir $(BUILD_DIR)/compiler-development/artifact/generic-function-compiler-shaped.ir
	$(TARGET) tests/cases/generics/ok/generic-aggregate-monomorphization.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/generic-aggregate-monomorphization.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/generic-aggregate-monomorphization.ir $(BUILD_DIR)/compiler-development/artifact/generic-aggregate-monomorphization.ir
	$(TARGET) tests/cases/match/ok/compiler-shaped-aggregates.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/compiler-shaped-aggregates.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/compiler-shaped-aggregates.ir $(BUILD_DIR)/compiler-development/artifact/compiler-shaped-aggregates.ir
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/project-compiler.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/project-compiler.ir $(BUILD_DIR)/compiler-development/artifact/project-compiler.ir
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-move.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-typed-ir $(BUILD_DIR)/compiler-development/artifact/ownership-aggregate-field-move.ir
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/ownership-aggregate-field-move.ir $(BUILD_DIR)/compiler-development/artifact/ownership-aggregate-field-move.ir
	$(TARGET) tests/cases/compiler-development/artifact/ok/token-dump-basic.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-pass-summary $(BUILD_DIR)/compiler-development/artifact/pass-summary-basic.summary
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/pass-summary-basic.summary $(BUILD_DIR)/compiler-development/artifact/pass-summary-basic.summary
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-pass-summary $(BUILD_DIR)/compiler-development/artifact/pass-summary-project-compiler.summary
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/pass-summary-project-compiler.summary $(BUILD_DIR)/compiler-development/artifact/pass-summary-project-compiler.summary
	$(TARGET) tests/cases/compiler-development/artifact/ok/core-language-scalar-flow.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-core.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-core.llvm-frag --llvm-symbol _ARNv3add --llvm-symbol _ARNv7compute --llvm-symbol _ARNv4main
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-core.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-core.llvm-frag
	$(TARGET) tests/cases/generics/ok/generic-function-compiler-shaped.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-generic-function.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-generic-function.llvm-frag --llvm-symbol _ARNv16identity__G_Span --llvm-symbol _ARNv46identity__G_PassResult_Box_Token___Diagnostic_ --llvm-symbol _ARNv15choose__G_Token --llvm-symbol _ARNv17make_box__G_Token --llvm-symbol _ARNv17maybe_or__G_Token --llvm-symbol _ARNv32pass_or__G_Box_Token__Diagnostic
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-generic-function.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-generic-function.llvm-frag
	$(TARGET) tests/cases/generics/ok/generic-aggregate-monomorphization.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-generic-aggregate.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-generic-aggregate.llvm-frag --llvm-symbol _ARNv14make_token_box --llvm-symbol _ARNv11maybe_score --llvm-symbol _ARNv13consume_owned
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-generic-aggregate.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-generic-aggregate.llvm-frag
	$(TARGET) tests/cases/memory/ok/layout-service-aggregate-consistency.ari --target x86_64-pc-linux-gnu --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-layout-aggregate.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-layout-aggregate.llvm-frag --llvm-symbol _ARNv10enum_score --llvm-symbol _ARNv11field_score --llvm-symbol _ARNv4main
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-layout-aggregate.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-layout-aggregate.llvm-frag
	$(TARGET) tests/cases/traits/ok/trait-minimum-compiler-shaped.ari --target x86_64-pc-linux-gnu --no-implicit-std --emit-llvm $(BUILD_DIR)/compiler-development/artifact/trait-minimum-static-dispatch.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-trait-dispatch.llvm-frag --llvm-symbol _ARNv17same__G_TokenKind --llvm-symbol _ARNv4impl9TokenKind9Equatable4same --llvm-symbol _ARNv25bucket_hash__G_SymbolName --llvm-symbol _ARNv4impl10SymbolName8Hashable9hash_code
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-trait-dispatch.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-trait-dispatch.llvm-frag
	$(TARGET) tests/cases/ownership/ok/drop-aggregate-fields.ari --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-ownership-drop-aggregate.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-ownership-drop-aggregate.llvm-frag --llvm-symbol _ARNv4main
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-ownership-drop-aggregate.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-ownership-drop-aggregate.llvm-frag
	$(TARGET) tests/cases/ownership/ok/drop-runtime-owned-enum-payload.ari --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-ownership-drop-runtime-enum.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-ownership-drop-runtime-enum.llvm-frag --llvm-symbol _ARNv4main
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-ownership-drop-runtime-enum.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-ownership-drop-runtime-enum.llvm-frag
	$(TARGET) tests/cases/ownership/ok/ownership-compiler-shaped.ari --emit-llvm $(BUILD_DIR)/compiler-development/artifact/backend-ownership-compiler-shaped.ll --emit-llvm-fragment $(BUILD_DIR)/compiler-development/artifact/backend-ownership-compiler-shaped.llvm-frag --llvm-symbol _ARNv11finish_work --llvm-symbol _ARNv13consume_boxed --llvm-symbol _ARNv13consume_token --llvm-symbol _ARNv7analyze
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/backend-ownership-compiler-shaped.llvm-frag $(BUILD_DIR)/compiler-development/artifact/backend-ownership-compiler-shaped.llvm-frag
	$(TARGET) tests/cases/ffi/ok/c-header-repr-struct.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/c-header-repr-struct.h --emit-llvm $(BUILD_DIR)/compiler-development/artifact/c-header-repr-struct.ll
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/c-header-repr-struct.h $(BUILD_DIR)/compiler-development/artifact/c-header-repr-struct.h
	$(TARGET) tests/cases/ffi/ok/c-header-repr-payload-enum.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/c-header-repr-payload-enum.h --emit-llvm $(BUILD_DIR)/compiler-development/artifact/c-header-repr-payload-enum.ll
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/c-header-repr-payload-enum.h $(BUILD_DIR)/compiler-development/artifact/c-header-repr-payload-enum.h
	$(TARGET) tests/cases/ffi/ok/c-header-generated-aggregates.ari --shared --emit-c-header $(BUILD_DIR)/compiler-development/artifact/c-header-generated-aggregates.h --emit-llvm $(BUILD_DIR)/compiler-development/artifact/c-header-generated-aggregates.ll
	python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/c-header-generated-aggregates.h $(BUILD_DIR)/compiler-development/artifact/c-header-generated-aggregates.h
	if command -v nm >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/library-export.ari --emit-obj $(BUILD_DIR)/compiler-development/artifact/library-export.o --emit-symbols $(BUILD_DIR)/compiler-development/artifact/object-library-export.symbols --symbol _ARNv3add --symbol _ARNv4main; python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/object-library-export.symbols $(BUILD_DIR)/compiler-development/artifact/object-library-export.symbols; fi
	if command -v nm >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/object-aggregate-extern-link.ari --emit-obj $(BUILD_DIR)/compiler-development/artifact/object-aggregate-extern-link.o --emit-symbols $(BUILD_DIR)/compiler-development/artifact/object-aggregate-extern-link.symbols --symbol ari_make_object_pair --symbol ari_take_object_pair --symbol ari_test_add_i64 --symbol _ARNv4main; python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/object-aggregate-extern-link.symbols $(BUILD_DIR)/compiler-development/artifact/object-aggregate-extern-link.symbols; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v nm >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/shared-visibility.ari --shared --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/compiler-development/artifact/shared-visibility.so --emit-symbols $(BUILD_DIR)/compiler-development/artifact/shared-visibility.symbols --symbol _ARNv10public_api --symbol ari_named_private --symbol _ARNv6helper --symbol _ARNv4main --symbol ari_builtin_write_i64; python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/shared-visibility.symbols $(BUILD_DIR)/compiler-development/artifact/shared-visibility.symbols; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/compiler-development/artifact/ok/runtime-output-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/compiler-development/artifact/runtime-output-basic.elf; $(BUILD_DIR)/compiler-development/artifact/runtime-output-basic.elf >$(BUILD_DIR)/compiler-development/artifact/runtime-output-basic.stdout; python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/runtime-output-basic.stdout $(BUILD_DIR)/compiler-development/artifact/runtime-output-basic.stdout; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/compiler-development/artifact/ok/runtime-output-trait.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/compiler-development/artifact/runtime-output-trait.elf; $(BUILD_DIR)/compiler-development/artifact/runtime-output-trait.elf >$(BUILD_DIR)/compiler-development/artifact/runtime-output-trait.stdout; python3 tests/check_compiler_artifacts.py tests/cases/compiler-development/artifact/ok/runtime-output-trait.stdout $(BUILD_DIR)/compiler-development/artifact/runtime-output-trait.stdout; fi

check-ownership: $(TARGET)
	mkdir -p $(BUILD_DIR)/ownership
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-move.ari -o $(BUILD_DIR)/ownership/ownership-aggregate-field-move.elf
	$(BUILD_DIR)/ownership/ownership-aggregate-field-move.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/borrowing/ok/borrow-reborrow-paths.ari -o $(BUILD_DIR)/ownership/borrow-reborrow-paths.elf
	$(BUILD_DIR)/ownership/borrow-reborrow-paths.elf; code=$$?; test $$code -eq 61
	$(TARGET) tests/cases/ownership/ok/ownership-compiler-shaped.ari -o $(BUILD_DIR)/ownership/ownership-compiler-shaped.elf
	$(BUILD_DIR)/ownership/ownership-compiler-shaped.elf; code=$$?; test $$code -eq 72
	$(TARGET) tests/cases/ownership/ok/drop-aggregate-fields.ari --emit-llvm $(BUILD_DIR)/ownership/drop-aggregate-fields.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/ownership/drop-aggregate-fields.ll) -eq 2
	$(TARGET) tests/cases/ownership/ok/drop-runtime-owned-enum-payload.ari --emit-llvm $(BUILD_DIR)/ownership/drop-runtime-owned-enum-payload.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/ownership/drop-runtime-owned-enum-payload.ll) -eq 2
	grep -q "icmp eq i64" $(BUILD_DIR)/ownership/drop-runtime-owned-enum-payload.ll
	$(TARGET) tests/cases/ownership/ok/ownership-compiler-shaped.ari --emit-llvm $(BUILD_DIR)/ownership/ownership-compiler-shaped.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/ownership/ownership-compiler-shaped.ll) -eq 5
	grep -q 'define i64 @"_ARNv7analyze"' $(BUILD_DIR)/ownership/ownership-compiler-shaped.ll
	if $(TARGET) tests/cases/ownership/errors/use-after-move.ari --check >$(BUILD_DIR)/ownership/use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved binding 'token'" $(BUILD_DIR)/ownership/use-after-move.err
	if $(TARGET) tests/cases/ownership/errors/move-borrowed-owner.ari --check >$(BUILD_DIR)/ownership/move-borrowed-owner.err 2>&1; then exit 1; fi
	grep -q "cannot move borrowed binding 'token'" $(BUILD_DIR)/ownership/move-borrowed-owner.err
	if $(TARGET) tests/cases/ownership/errors/ownership-aggregate-partial-move.ari --check >$(BUILD_DIR)/ownership/ownership-aggregate-partial-move.err 2>&1; then exit 1; fi
	grep -q "cannot move partially moved owning binding 'holder'" $(BUILD_DIR)/ownership/ownership-aggregate-partial-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-vector-dynamic-element-move.ari --check >$(BUILD_DIR)/ownership/ownership-vector-dynamic-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements through dynamic indexes is not supported; use a constant index or move the whole aggregate" $(BUILD_DIR)/ownership/ownership-vector-dynamic-element-move.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ownership-temporary-element-move.ari --check >$(BUILD_DIR)/ownership/diagnostic-ownership-temporary-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements out of temporary values is not supported; bind the aggregate first" $(BUILD_DIR)/ownership/diagnostic-ownership-temporary-element-move.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-assignment-while-borrowed.ari --check >$(BUILD_DIR)/ownership/diagnostic-assignment-while-borrowed.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'number'" $(BUILD_DIR)/ownership/diagnostic-assignment-while-borrowed.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-field-assignment-while-borrowed.ari --check >$(BUILD_DIR)/ownership/diagnostic-field-assignment-while-borrowed.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed field 'pair.0'" $(BUILD_DIR)/ownership/diagnostic-field-assignment-while-borrowed.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-return-live-owner.ari --check >$(BUILD_DIR)/ownership/diagnostic-return-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before return" $(BUILD_DIR)/ownership/diagnostic-return-live-owner.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-loop-break-live-owner.ari --check >$(BUILD_DIR)/ownership/diagnostic-loop-break-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before break" $(BUILD_DIR)/ownership/diagnostic-loop-break-live-owner.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-loop-continue-live-owner.ari --check >$(BUILD_DIR)/ownership/diagnostic-loop-continue-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before continue" $(BUILD_DIR)/ownership/diagnostic-loop-continue-live-owner.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-borrow-after-move.ari --check >$(BUILD_DIR)/ownership/diagnostic-borrow-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved binding 'token'" $(BUILD_DIR)/ownership/diagnostic-borrow-after-move.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-double-move.ari --check >$(BUILD_DIR)/ownership/diagnostic-double-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved binding 'token'" $(BUILD_DIR)/ownership/diagnostic-double-move.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-enum-payload-invalid-move.ari --check >$(BUILD_DIR)/ownership/diagnostic-enum-payload-invalid-move.err 2>&1; then exit 1; fi
	grep -q "cannot move runtime-dependent owning enum payload slot 'item.0' outside match" $(BUILD_DIR)/ownership/diagnostic-enum-payload-invalid-move.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-compact-enum-payload-ref.ari --check >$(BUILD_DIR)/ownership/diagnostic-compact-enum-payload-ref.err 2>&1; then exit 1; fi
	grep -q "compact enum payload reference binding for i32 is value-only" $(BUILD_DIR)/ownership/diagnostic-compact-enum-payload-ref.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-ownership-aggregate-enum-payload.ari --check >$(BUILD_DIR)/ownership/diagnostic-ownership-aggregate-enum-payload.err 2>&1; then exit 1; fi
	grep -q "ownership-carrying aggregate enum payloads are not supported yet" $(BUILD_DIR)/ownership/diagnostic-ownership-aggregate-enum-payload.err
	if $(TARGET) tests/cases/compiler-development/artifact/errors/diagnostic-borrow-aggregate-enum-payload.ari --check >$(BUILD_DIR)/ownership/diagnostic-borrow-aggregate-enum-payload.err 2>&1; then exit 1; fi
	grep -q "borrow-carrying aggregate enum payloads are not supported yet" $(BUILD_DIR)/ownership/diagnostic-borrow-aggregate-enum-payload.err

check-compiler-docs:
	python3 tests/check_compiler_docs.py

check-bootstrap-readiness: $(TARGET)
	mkdir -p $(BUILD_DIR)/bootstrap-readiness
	$(TARGET) tests/cases/bootstrap-readiness/ok/model/model-token-span.ari --check
	$(TARGET) tests/cases/bootstrap-readiness/ok/source/source-line-column.ari --check
	$(TARGET) tests/cases/bootstrap-readiness/ok/source/source-line-column.ari --emit-llvm $(BUILD_DIR)/bootstrap-readiness/source-line-column.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/bootstrap-readiness/ok/source/source-line-column.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/bootstrap-readiness/source-line-column.elf; $(BUILD_DIR)/bootstrap-readiness/source-line-column.elf; code=$$?; test $$code -eq 122; fi
	$(TARGET) tests/cases/bootstrap-readiness/ok/errors/errors-result-flow.ari --check
	$(TARGET) tests/cases/bootstrap-readiness/ok/errors/errors-result-flow.ari --emit-llvm $(BUILD_DIR)/bootstrap-readiness/errors-result-flow.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/bootstrap-readiness/ok/errors/errors-result-flow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/bootstrap-readiness/errors-result-flow.elf; $(BUILD_DIR)/bootstrap-readiness/errors-result-flow.elf; code=$$?; test $$code -eq 122; fi
	$(TARGET) tests/cases/bootstrap-readiness/ok/formatting/formatting-artifact-line.ari --check
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/bootstrap-readiness/ok/formatting/formatting-artifact-line.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/bootstrap-readiness/formatting-artifact-line.elf; $(BUILD_DIR)/bootstrap-readiness/formatting-artifact-line.elf >$(BUILD_DIR)/bootstrap-readiness/formatting-artifact-line.out; grep -qx "token=character value=x span=4:5" $(BUILD_DIR)/bootstrap-readiness/formatting-artifact-line.out; fi

check-tools: $(TARGET) $(LINT_TARGET) $(LSP_TARGET)
	mkdir -p $(BUILD_DIR)/tools
	python3 tests/tools/check_vscode_extension.py
	if command -v node >/dev/null 2>&1; then node --check editors/vscode/extension.js; node --check editors/vscode/commands.js; node --check editors/vscode/tasks.js; node --check editors/vscode/lsp.js; node --check editors/vscode/paths.js; node --check editors/vscode/config.js; node --check editors/vscode/scripts/package-vsix.js; fi
	$(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) examples/count.ari >$(BUILD_DIR)/tools/lint-ok.out
	grep -q 'examples/count.ari: ok' $(BUILD_DIR)/tools/lint-ok.out
	$(LINT_TARGET) --list-rules >$(BUILD_DIR)/tools/lint-rules.out
	grep -q 'lint/trailing-whitespace' $(BUILD_DIR)/tools/lint-rules.out
	grep -q 'lint/missing-final-newline' $(BUILD_DIR)/tools/lint-rules.out
	printf 'fn main() -> i64 { \n  return 0;\n}\n' >$(BUILD_DIR)/tools/lint-trailing-whitespace.ari
	printf '# Ari lint config\nlint/trailing-whitespace = error\n' >$(BUILD_DIR)/tools/ari-lint.rules
	if $(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) $(BUILD_DIR)/tools/lint-trailing-whitespace.ari >$(BUILD_DIR)/tools/lint-trailing.out; then exit 1; fi
	grep -q 'lint/trailing-whitespace' $(BUILD_DIR)/tools/lint-trailing.out
	grep -q 'error: \[lint/trailing-whitespace\]' $(BUILD_DIR)/tools/lint-trailing.out
	$(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) --rule lint/trailing-whitespace=off $(BUILD_DIR)/tools/lint-trailing-whitespace.ari >$(BUILD_DIR)/tools/lint-trailing-off.out
	grep -q '$(BUILD_DIR)/tools/lint-trailing-whitespace.ari: ok' $(BUILD_DIR)/tools/lint-trailing-off.out
	if $(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) --config $(BUILD_DIR)/tools/ari-lint.rules $(BUILD_DIR)/tools/lint-trailing-whitespace.ari >$(BUILD_DIR)/tools/lint-trailing-config.out; then exit 1; fi
	grep -q 'error: \[lint/trailing-whitespace\]' $(BUILD_DIR)/tools/lint-trailing-config.out
	if $(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) --json --rule trailing-whitespace=error $(BUILD_DIR)/tools/lint-trailing-whitespace.ari >$(BUILD_DIR)/tools/lint-trailing.json; then exit 1; fi
	grep -q '"severity":"error"' $(BUILD_DIR)/tools/lint-trailing.json
	grep -q '"code":"lint/trailing-whitespace"' $(BUILD_DIR)/tools/lint-trailing.json
	printf 'fn main() -> i64 {\n  return 0;\n}' >$(BUILD_DIR)/tools/lint-missing-final-newline.ari
	if $(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) --json --rule missing-final-newline=error $(BUILD_DIR)/tools/lint-missing-final-newline.ari >$(BUILD_DIR)/tools/lint-missing-final-newline.json; then exit 1; fi
	grep -q '"code":"lint/missing-final-newline"' $(BUILD_DIR)/tools/lint-missing-final-newline.json
	grep -q '"message":"missing final newline"' $(BUILD_DIR)/tools/lint-missing-final-newline.json
	$(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) --rule lint/missing-final-newline=off $(BUILD_DIR)/tools/lint-missing-final-newline.ari >$(BUILD_DIR)/tools/lint-missing-final-newline-off.out
	grep -q '$(BUILD_DIR)/tools/lint-missing-final-newline.ari: ok' $(BUILD_DIR)/tools/lint-missing-final-newline-off.out
	if $(TOOL_ENV) $(LINT_TARGET) --ari $(TOOL_ARI) --json tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari >$(BUILD_DIR)/tools/lint-format.json; then exit 1; fi
	grep -q '"line":2' $(BUILD_DIR)/tools/lint-format.json
	grep -q '"code":"ari/compiler"' $(BUILD_DIR)/tools/lint-format.json
	grep -q "prelude macro 'format!' has no implicit allocation zone; use format_in!(ref mut zone, ...) or call it inside a zone block" $(BUILD_DIR)/tools/lint-format.json
	ARI="$(TOOL_ARI)" LSP="$(LSP_TARGET)" $(TOOL_ENV) sh tests/tools/lsp_smoke.sh

check-cli: $(TARGET)
	mkdir -p $(BUILD_DIR)/cli
	$(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/cli/count.ll
	test -s $(BUILD_DIR)/cli/count.ll
	grep -q 'define i64 @"_ARNv4main"' $(BUILD_DIR)/cli/count.ll
	grep -q 'define i64 @"ari::main"()' $(BUILD_DIR)/cli/count.ll
	grep -q 'define i64 @ari_entry(i32 %argc, ptr %argv)' $(BUILD_DIR)/cli/count.ll
	grep -q 'call i64 @"ari::main"()' $(BUILD_DIR)/cli/count.ll
	grep -q "define i32 @main" $(BUILD_DIR)/cli/count.ll
	$(TARGET) tests/cases/ffi/ok/library-export.ari --check
	$(TARGET) tests/cases/match/ok/match-nested-reference-binding-mode.ari --check
	if $(TARGET) examples/count.ari --check --emit-llvm $(BUILD_DIR)/cli/check-bad.ll >$(BUILD_DIR)/cli/check-output-option.err 2>&1; then exit 1; fi
	grep -q -- "--check cannot be combined with backend output or linking options" $(BUILD_DIR)/cli/check-output-option.err
	$(TARGET) tests/cases/ffi/ok/library-export.ari --emit-obj $(BUILD_DIR)/cli/library-export.o
	test -s $(BUILD_DIR)/cli/library-export.o
	if $(TARGET) examples/count.ari --emit-obj >$(BUILD_DIR)/cli/missing-emit-obj.err 2>&1; then exit 1; fi
	grep -q -- "--emit-obj expects a path" $(BUILD_DIR)/cli/missing-emit-obj.err
	if $(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/cli/count.ll --emit-obj $(BUILD_DIR)/cli/count.o >$(BUILD_DIR)/cli/emit-obj-emit-llvm.err 2>&1; then exit 1; fi
	grep -q -- "--emit-obj cannot be combined with --emit-llvm" $(BUILD_DIR)/cli/emit-obj-emit-llvm.err
	if $(TARGET) examples/count.ari --emit-obj $(BUILD_DIR)/cli/count.o -l c >$(BUILD_DIR)/cli/emit-obj-link.err 2>&1; then exit 1; fi
	grep -q -- "--emit-obj cannot be combined with linker options" $(BUILD_DIR)/cli/emit-obj-link.err
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) examples/count.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/cli/output.elf; test -x $(BUILD_DIR)/cli/output.elf; $(BUILD_DIR)/cli/output.elf; code=$$?; test $$code -eq 5; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/library-export.ari --llvm-cc $(LLVM_CC) --shared -o $(BUILD_DIR)/cli/libari_cli.so; test -s $(BUILD_DIR)/cli/libari_cli.so; fi
	if $(TARGET) >$(BUILD_DIR)/cli/missing-input.err 2>&1; then exit 1; fi
	grep -q "usage: ari" $(BUILD_DIR)/cli/missing-input.err
	if $(TARGET) examples/count.ari -o >$(BUILD_DIR)/cli/missing-output.err 2>&1; then exit 1; fi
	grep -q -- "-o expects a path" $(BUILD_DIR)/cli/missing-output.err
	if $(TARGET) examples/count.ari --emit-cpp $(BUILD_DIR)/cli/count.cpp >$(BUILD_DIR)/cli/emit-cpp-removed.err 2>&1; then exit 1; fi
	grep -q -- "--emit-cpp was removed; use --emit-llvm" $(BUILD_DIR)/cli/emit-cpp-removed.err
	if $(TARGET) examples/count.ari --keep-cpp >$(BUILD_DIR)/cli/keep-cpp-removed.err 2>&1; then exit 1; fi
	grep -q -- "--keep-cpp was removed" $(BUILD_DIR)/cli/keep-cpp-removed.err
	if $(TARGET) examples/count.ari --emit-llvm >$(BUILD_DIR)/cli/missing-emit-llvm.err 2>&1; then exit 1; fi
	grep -q -- "--emit-llvm expects a path" $(BUILD_DIR)/cli/missing-emit-llvm.err
	if $(TARGET) tests/cases/modules/ok/package-module-main.ari --module-path >$(BUILD_DIR)/cli/missing-module-path.err 2>&1; then exit 1; fi
	grep -q -- "--module-path expects a path" $(BUILD_DIR)/cli/missing-module-path.err
	if $(TARGET) tests/cases/modules/ok/package-module-main.ari --emit-module-metadata >$(BUILD_DIR)/cli/missing-emit-module-metadata.err 2>&1; then exit 1; fi
	grep -q -- "--emit-module-metadata expects a path" $(BUILD_DIR)/cli/missing-emit-module-metadata.err
	if $(TARGET) tests/cases/modules/ok/package-module-main.ari --check-module-metadata >$(BUILD_DIR)/cli/missing-check-module-metadata.err 2>&1; then exit 1; fi
	grep -q -- "--check-module-metadata expects a path" $(BUILD_DIR)/cli/missing-check-module-metadata.err
	if $(TARGET) tests/cases/modules/ok/package-module-main.ari --emit-module-cache >$(BUILD_DIR)/cli/missing-emit-module-cache.err 2>&1; then exit 1; fi
	grep -q -- "--emit-module-cache expects a path" $(BUILD_DIR)/cli/missing-emit-module-cache.err
	if $(TARGET) tests/cases/modules/ok/package-module-main.ari --use-module-cache >$(BUILD_DIR)/cli/missing-use-module-cache.err 2>&1; then exit 1; fi
	grep -q -- "--use-module-cache expects a path" $(BUILD_DIR)/cli/missing-use-module-cache.err
	if $(TARGET) tests/cases/modules/ok/package-module-main.ari -I >$(BUILD_DIR)/cli/missing-I.err 2>&1; then exit 1; fi
	grep -q -- "-I expects a path" $(BUILD_DIR)/cli/missing-I.err
	if $(TARGET) examples/count.ari --backend llvm >$(BUILD_DIR)/cli/backend-removed.err 2>&1; then exit 1; fi
	grep -q -- "--backend was removed; LLVM is the only backend" $(BUILD_DIR)/cli/backend-removed.err
	if $(TARGET) examples/count.ari --cc >$(BUILD_DIR)/cli/missing-cc.err 2>&1; then exit 1; fi
	grep -q -- "--cc was removed; use --llvm-cc" $(BUILD_DIR)/cli/missing-cc.err
	if $(TARGET) examples/count.ari --llvm-cc >$(BUILD_DIR)/cli/missing-llvm-cc.err 2>&1; then exit 1; fi
	grep -q -- "--llvm-cc expects a compiler path" $(BUILD_DIR)/cli/missing-llvm-cc.err
	if $(TARGET) examples/count.ari --target >$(BUILD_DIR)/cli/missing-target.err 2>&1; then exit 1; fi
	grep -q -- "--target expects a target triple" $(BUILD_DIR)/cli/missing-target.err
	if $(TARGET) examples/count.ari --target mystery-cpu --emit-llvm $(BUILD_DIR)/cli/bad-target.ll >$(BUILD_DIR)/cli/bad-target.err 2>&1; then exit 1; fi
	grep -q "unsupported target triple 'mystery-cpu'" $(BUILD_DIR)/cli/bad-target.err
	if $(TARGET) examples/count.ari extra.ari >$(BUILD_DIR)/cli/unexpected-arg.err 2>&1; then exit 1; fi
	grep -q "unexpected argument 'extra.ari'" $(BUILD_DIR)/cli/unexpected-arg.err

check-prelude: $(TARGET)
	mkdir -p $(BUILD_DIR)/prelude
	$(TARGET) tests/cases/standard-library/ok/prelude/std-lazy-unused.ari --emit-llvm $(BUILD_DIR)/prelude/std-lazy-unused.ll
	! grep -q '_ARNv3std' $(BUILD_DIR)/prelude/std-lazy-unused.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/std-lazy-used.ari --emit-llvm $(BUILD_DIR)/prelude/std-lazy-used.ll
	grep -q '_ARNv3std5ascii8is_digit' $(BUILD_DIR)/prelude/std-lazy-used.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-builtins.ari -o $(BUILD_DIR)/prelude/prelude-builtins.elf
	$(BUILD_DIR)/prelude/prelude-builtins.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-std-root-builtins.ari -o $(BUILD_DIR)/prelude/prelude-std-root-builtins.elf
	$(BUILD_DIR)/prelude/prelude-std-root-builtins.elf >$(BUILD_DIR)/prelude/prelude-std-root-builtins.out; code=$$?; test $$code -eq 9
	grep -qx "4" $(BUILD_DIR)/prelude/prelude-std-root-builtins.out
	grep -qx "5" $(BUILD_DIR)/prelude/prelude-std-root-builtins.out
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-macros.ari -o $(BUILD_DIR)/prelude/prelude-macros.elf
	$(BUILD_DIR)/prelude/prelude-macros.elf >$(BUILD_DIR)/prelude/prelude-macros.out; code=$$?; test $$code -eq 7
	grep -q "macro=5 ok=true" $(BUILD_DIR)/prelude/prelude-macros.out
	$(TARGET) tests/cases/standard-library/ok/format/format-eprintln.ari --emit-llvm $(BUILD_DIR)/prelude/format-eprintln.ll
	grep -q "@fprintf" $(BUILD_DIR)/prelude/format-eprintln.ll
	grep -q "@stderr" $(BUILD_DIR)/prelude/format-eprintln.ll
	$(TARGET) tests/cases/standard-library/ok/format/format-eprintln.ari -o $(BUILD_DIR)/prelude/format-eprintln.elf
	$(BUILD_DIR)/prelude/format-eprintln.elf >$(BUILD_DIR)/prelude/format-eprintln.out 2>$(BUILD_DIR)/prelude/format-eprintln.err
	test ! -s $(BUILD_DIR)/prelude/format-eprintln.out
	grep -qx 'macro=err n=-9 ok=true byte=! pi=2.5 dbg="ari"' $(BUILD_DIR)/prelude/format-eprintln.err
	grep -qx "std=7" $(BUILD_DIR)/prelude/format-eprintln.err
	grep -qx "call=3" $(BUILD_DIR)/prelude/format-eprintln.err
	$(TARGET) tests/cases/standard-library/ok/format/format-float-llvm.ari -o $(BUILD_DIR)/prelude/format-float-llvm.elf
	$(BUILD_DIR)/prelude/format-float-llvm.elf >$(BUILD_DIR)/prelude/format-float-llvm.out
	grep -qx "value=1.3 default=1.500000 whole=6" $(BUILD_DIR)/prelude/format-float-llvm.out
	grep -qx "round=1.12 carry=10 neg=-2.5" $(BUILD_DIR)/prelude/format-float-llvm.out
	$(TARGET) tests/cases/standard-library/ok/format/format-print-u64.ari -o $(BUILD_DIR)/prelude/format-print-u64.elf
	$(BUILD_DIR)/prelude/format-print-u64.elf >$(BUILD_DIR)/prelude/format-print-u64.out
	grep -qx "u64=18446744073709551615 zero=0" $(BUILD_DIR)/prelude/format-print-u64.out
	grep -qx "small=7 max=18446744073709551615" $(BUILD_DIR)/prelude/format-print-u64.out
	$(TARGET) tests/cases/standard-library/ok/format/format-named-capture.ari --emit-llvm $(BUILD_DIR)/prelude/format-named-capture.ll
	grep -q '@ari_builtin_write_i64' $(BUILD_DIR)/prelude/format-named-capture.ll
	grep -q 'append_string_in' $(BUILD_DIR)/prelude/format-named-capture.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/format-named-capture.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/format-named-capture.elf; $(BUILD_DIR)/prelude/format-named-capture.elf >$(BUILD_DIR)/prelude/format-named-capture.out; code=$$?; test $$code -eq 45; grep -qx 'asdf 42 true Z 3.14 "asdf"' $(BUILD_DIR)/prelude/format-named-capture.out; grep -qx 'point=7,8 pair=5,"six"' $(BUILD_DIR)/prelude/format-named-capture.out; fi
	$(TARGET) tests/cases/standard-library/ok/io/std-io-byte-slice.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-byte-slice.ll
	grep -q '_ARNv3std2io11write_bytes' $(BUILD_DIR)/prelude/std-io-byte-slice.ll
	grep -q "@ari_builtin_write_byte" $(BUILD_DIR)/prelude/std-io-byte-slice.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-byte-slice.ari -o $(BUILD_DIR)/prelude/std-io-byte-slice.elf
	$(BUILD_DIR)/prelude/std-io-byte-slice.elf >$(BUILD_DIR)/prelude/std-io-byte-slice.out; code=$$?; test $$code -eq 6
	grep -qx "AB" $(BUILD_DIR)/prelude/std-io-byte-slice.out
	grep -qx "CD" $(BUILD_DIR)/prelude/std-io-byte-slice.out
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/io/std-io-byte-slice.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-io-byte-slice.elf; $(BUILD_DIR)/prelude/std-io-byte-slice.elf >$(BUILD_DIR)/prelude/std-io-byte-slice.out; code=$$?; test $$code -eq 6; grep -qx "AB" $(BUILD_DIR)/prelude/std-io-byte-slice.out; grep -qx "CD" $(BUILD_DIR)/prelude/std-io-byte-slice.out; fi
	$(TARGET) tests/cases/standard-library/ok/io/std-io-traits-cursor.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-traits-cursor.ll
	grep -q '_ARNv3std2io29read_exact__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-traits-cursor.ll
	grep -q '_ARNv3std2io28write_all__G_std__io__Stdout' $(BUILD_DIR)/prelude/std-io-traits-cursor.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-traits-cursor.ari -o $(BUILD_DIR)/prelude/std-io-traits-cursor.elf
	$(BUILD_DIR)/prelude/std-io-traits-cursor.elf >$(BUILD_DIR)/prelude/std-io-traits-cursor.out; code=$$?; test $$code -eq 8
	grep -qx "OK" $(BUILD_DIR)/prelude/std-io-traits-cursor.out
	$(TARGET) tests/cases/standard-library/ok/io/std-io-read-all.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-read-all.ll
	grep -q '_ARNv3std2io27read_all__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-read-all.ll
	grep -q '_ARNv3std2io47read_all__G_std__io__BufReader_std__io__Cursor_' $(BUILD_DIR)/prelude/std-io-read-all.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-read-all.ari -o $(BUILD_DIR)/prelude/std-io-read-all.elf
	$(BUILD_DIR)/prelude/std-io-read-all.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/standard-library/ok/io/std-io-read-to-string.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	grep -q '_ARNv3std2io33read_to_string__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	grep -q '_ARNv3std2io53read_to_string__G_std__io__BufReader_std__io__Cursor_' $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	grep -q '_ARNv3std2io33read_line_from__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	grep -q '_ARNv3std2io43read_to_string_unchecked__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	grep -q 'std__io__Cursor8inherent9read_line' $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	grep -q 'std__io__Cursor8inherent14read_to_string' $(BUILD_DIR)/prelude/std-io-read-to-string.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-read-to-string.ari -o $(BUILD_DIR)/prelude/std-io-read-to-string.elf
	$(BUILD_DIR)/prelude/std-io-read-to-string.elf; code=$$?; test $$code -eq 6
	$(TARGET) tests/cases/standard-library/ok/io/std-io-copy.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-copy.ll
	grep -q '_ARNv3std2io36copy__G_std__io__Cursor_MemoryWriter' $(BUILD_DIR)/prelude/std-io-copy.ll
	grep -q '_ARNv3std2io40try_copy__G_std__io__Cursor_MemoryWriter' $(BUILD_DIR)/prelude/std-io-copy.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-copy.ari -o $(BUILD_DIR)/prelude/std-io-copy.elf
	$(BUILD_DIR)/prelude/std-io-copy.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/standard-library/ok/io/std-io-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io29read_exact__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io25write_all__G_MemoryWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io24flush__G_FlushFailWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io36copy__G_std__io__Cursor_MemoryWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io25read_one__G_FailingReader' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io27read_exact__G_FailingReader' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io31read_to_string__G_FailingReader' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io31read_line_from__G_FailingReader' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io34copy__G_FailingReader_MemoryWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io39read_exact_unchecked__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io35write_all_unchecked__G_MemoryWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io31flush_unchecked__G_MemoryWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	grep -q '_ARNv3std2io46copy_unchecked__G_std__io__Cursor_MemoryWriter' $(BUILD_DIR)/prelude/std-io-result.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-result.ari -o $(BUILD_DIR)/prelude/std-io-result.elf
	$(BUILD_DIR)/prelude/std-io-result.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/io/std-io-stderr.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-stderr.ll
	grep -q '_ARNv3std2io28write_all__G_std__io__Stderr' $(BUILD_DIR)/prelude/std-io-stderr.ll
	grep -q "@ari_builtin_write_error_byte" $(BUILD_DIR)/prelude/std-io-stderr.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-stderr.ari -o $(BUILD_DIR)/prelude/std-io-stderr.elf
	$(BUILD_DIR)/prelude/std-io-stderr.elf >$(BUILD_DIR)/prelude/std-io-stderr.out 2>$(BUILD_DIR)/prelude/std-io-stderr.err; code=$$?; test $$code -eq 3
	grep -qx "OK" $(BUILD_DIR)/prelude/std-io-stderr.out
	grep -qx "ERR" $(BUILD_DIR)/prelude/std-io-stderr.err
	$(TARGET) tests/cases/standard-library/ok/io/std-io-natural-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io24write__G_std__io__Stdout' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q 'std__io__Stdout15std__io__Writer5write' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q 'std__io__Stdout15std__io__Writer9write_all' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q 'std__io__Stderr15std__io__Writer5write' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q 'std__io__Stderr15std__io__Writer9write_all' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io5print' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io10print_text' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io7println' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io12println_text' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io6eprint' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io11eprint_text' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io8eprintln' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	grep -q '_ARNv3std2io13eprintln_text' $(BUILD_DIR)/prelude/std-io-natural-api.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-natural-api.ari -o $(BUILD_DIR)/prelude/std-io-natural-api.elf
	$(BUILD_DIR)/prelude/std-io-natural-api.elf >$(BUILD_DIR)/prelude/std-io-natural-api.out 2>$(BUILD_DIR)/prelude/std-io-natural-api.err; code=$$?; test $$code -eq 20
	grep -qx "GWMPQL" $(BUILD_DIR)/prelude/std-io-natural-api.out
	grep -qx "N" $(BUILD_DIR)/prelude/std-io-natural-api.out
	grep -qx "ppl" $(BUILD_DIR)/prelude/std-io-natural-api.out
	grep -qx "l" $(BUILD_DIR)/prelude/std-io-natural-api.out
	grep -qx "TU" $(BUILD_DIR)/prelude/std-io-natural-api.out
	grep -qx "EARYS" $(BUILD_DIR)/prelude/std-io-natural-api.err
	grep -qx "Z" $(BUILD_DIR)/prelude/std-io-natural-api.err
	grep -qx "rs" $(BUILD_DIR)/prelude/std-io-natural-api.err
	grep -qx "VX" $(BUILD_DIR)/prelude/std-io-natural-api.err
	$(TARGET) tests/cases/standard-library/ok/io/std-io-pipe.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-pipe.ll
	grep -F -q "std__io__PipeReader" $(BUILD_DIR)/prelude/std-io-pipe.ll
	grep -F -q "std__io__PipeWriter8inherent13close_on_exec" $(BUILD_DIR)/prelude/std-io-pipe.ll
	grep -F -q "std__io__PipeWriter8inherent17set_close_on_exec" $(BUILD_DIR)/prelude/std-io-pipe.ll
	grep -q "@ari_builtin_os_read_byte" $(BUILD_DIR)/prelude/std-io-pipe.ll
	grep -q "@ari_builtin_os_write_byte" $(BUILD_DIR)/prelude/std-io-pipe.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-pipe.ari -o $(BUILD_DIR)/prelude/std-io-pipe.elf
	$(BUILD_DIR)/prelude/std-io-pipe.elf; code=$$?; test $$code -eq 14
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/io/std-io-pipe.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-io-pipe.elf; $(BUILD_DIR)/prelude/std-io-pipe.elf; code=$$?; test $$code -eq 14; fi
	$(TARGET) tests/cases/standard-library/ok/log/std-log-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-log-basic.ll
	grep -q '_ARNv3std3log7message' $(BUILD_DIR)/prelude/std-log-basic.ll
	grep -q '_ARNv3std3log5write' $(BUILD_DIR)/prelude/std-log-basic.ll
	grep -q "@ari_builtin_write_error_byte" $(BUILD_DIR)/prelude/std-log-basic.ll
	$(TARGET) tests/cases/standard-library/ok/log/std-log-basic.ari -o $(BUILD_DIR)/prelude/std-log-basic.elf
	$(BUILD_DIR)/prelude/std-log-basic.elf >$(BUILD_DIR)/prelude/std-log-basic.out 2>$(BUILD_DIR)/prelude/std-log-basic.err; code=$$?; test $$code -eq 9
	test ! -s $(BUILD_DIR)/prelude/std-log-basic.out
	grep -qx "\[info\] OK" $(BUILD_DIR)/prelude/std-log-basic.err
	grep -qx "\[warn\] careful" $(BUILD_DIR)/prelude/std-log-basic.err
	grep -qx "\[debug\] debugging" $(BUILD_DIR)/prelude/std-log-basic.err
	grep -qx "\[error\] broken" $(BUILD_DIR)/prelude/std-log-basic.err
	$(TARGET) tests/cases/standard-library/ok/error/std-error-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-error-basic.ll
	grep -q '_ARNv3std5error10from_errno' $(BUILD_DIR)/prelude/std-error-basic.ll
	grep -q '_ARNv3std5error8from_raw' $(BUILD_DIR)/prelude/std-error-basic.ll
	grep -q '_ARNv3std5error14map_raw__G_i64' $(BUILD_DIR)/prelude/std-error-basic.ll
	grep -q '_ARNv3std5error13to_raw__G_i64' $(BUILD_DIR)/prelude/std-error-basic.ll
	grep -q 'std__error__Error8inherent12is_retryable' $(BUILD_DIR)/prelude/std-error-basic.ll
	$(TARGET) tests/cases/standard-library/ok/error/std-error-basic.ari -o $(BUILD_DIR)/prelude/std-error-basic.elf
	$(BUILD_DIR)/prelude/std-error-basic.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/standard-library/ok/error/std-error-validation.ari --emit-llvm $(BUILD_DIR)/prelude/std-error-validation.ll
	grep -q '_ARNv3std5error13try_with_code' $(BUILD_DIR)/prelude/std-error-validation.ll
	grep -q '_ARNv3std5error12try_from_raw' $(BUILD_DIR)/prelude/std-error-validation.ll
	grep -q '_ARNv3std5error14try_from_errno' $(BUILD_DIR)/prelude/std-error-validation.ll
	$(TARGET) tests/cases/standard-library/ok/error/std-error-validation.ari -o $(BUILD_DIR)/prelude/std-error-validation.elf
	$(BUILD_DIR)/prelude/std-error-validation.elf; code=$$?; test $$code -eq 45
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/error/std-error-validation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-error-validation.elf; $(BUILD_DIR)/prelude/std-error-validation.elf; code=$$?; test $$code -eq 45; fi
	$(TARGET) tests/cases/standard-library/ok/error/std-error-integration.ari --emit-llvm $(BUILD_DIR)/prelude/std-error-integration.ll
	grep -q '_ARNv3std5error12from_os_code' $(BUILD_DIR)/prelude/std-error-integration.ll
	grep -q '_ARNv3std5error16map_errno__G_i64' $(BUILD_DIR)/prelude/std-error-integration.ll
	grep -q '_ARNv3std5error18map_os_code__G_i64' $(BUILD_DIR)/prelude/std-error-integration.ll
	grep -q '_ARNv3std5error21is_connection_refused' $(BUILD_DIR)/prelude/std-error-integration.ll
	$(TARGET) tests/cases/standard-library/ok/error/std-error-integration.ari -o $(BUILD_DIR)/prelude/std-error-integration.elf
	$(BUILD_DIR)/prelude/std-error-integration.elf; code=$$?; test $$code -eq 45
	$(TARGET) tests/cases/standard-library/ok/io/std-io-buffered.ari --emit-llvm $(BUILD_DIR)/prelude/std-io-buffered.ll
	grep -q '_ARNv3std2io29buf_reader__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-buffered.ll
	grep -q '_ARNv3std2io32buf_reader_in__G_std__io__Cursor' $(BUILD_DIR)/prelude/std-io-buffered.ll
	grep -q '_ARNv3std2io29buf_writer__G_std__io__Stdout' $(BUILD_DIR)/prelude/std-io-buffered.ll
	grep -q '_ARNv3std2io33buf_writer_in__G_DropMemoryWriter' $(BUILD_DIR)/prelude/std-io-buffered.ll
	$(TARGET) tests/cases/standard-library/ok/io/std-io-buffered.ari -o $(BUILD_DIR)/prelude/std-io-buffered.elf
	$(BUILD_DIR)/prelude/std-io-buffered.elf >$(BUILD_DIR)/prelude/std-io-buffered.out; code=$$?; test $$code -eq 15
	grep -qx "ABC" $(BUILD_DIR)/prelude/std-io-buffered.out
	$(TARGET) tests/cases/standard-library/ok/format/raw-string-literals.ari -o $(BUILD_DIR)/prelude/raw-string-literals.elf
	$(BUILD_DIR)/prelude/raw-string-literals.elf >$(BUILD_DIR)/prelude/raw-string-literals.out; code=$$?; test $$code -eq 141
	grep -qx "A" $(BUILD_DIR)/prelude/raw-string-literals.out
	grep -qx "B" $(BUILD_DIR)/prelude/raw-string-literals.out
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-macros.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-macros.ll
	grep -q "@ari_builtin_assert_eq_i64" $(BUILD_DIR)/prelude/prelude-macros.ll
	grep -q "@ari_builtin_assert_eq_bool" $(BUILD_DIR)/prelude/prelude-macros.ll
	$(TARGET) tests/cases/standard-library/ok/test/std-test-report.ari --emit-llvm $(BUILD_DIR)/prelude/std-test-report.ll
	grep -q '_ARNv3std4test6report' $(BUILD_DIR)/prelude/std-test-report.ll
	grep -q '_ARNv3std4test12equal__G_i64' $(BUILD_DIR)/prelude/std-test-report.ll
	grep -q '_ARNv3std4test7scratch' $(BUILD_DIR)/prelude/std-test-report.ll
	grep -q '_ARNv3std4test16matches_snapshot' $(BUILD_DIR)/prelude/std-test-report.ll
	grep -q '_ARNv3std4test5bench' $(BUILD_DIR)/prelude/std-test-report.ll
	grep -q '_ARNv3std4test8temp_dir' $(BUILD_DIR)/prelude/std-test-report.ll
	$(TARGET) tests/cases/standard-library/ok/test/std-test-report.ari -o $(BUILD_DIR)/prelude/std-test-report.elf
	$(BUILD_DIR)/prelude/std-test-report.elf; code=$$?; test $$code -eq 16
	$(TARGET) tests/cases/match/ok/rare-payload-packing.ari --emit-llvm $(BUILD_DIR)/prelude/rare-payload-packing.ll
	grep -q "@ari_builtin_assert_eq_i64" $(BUILD_DIR)/prelude/rare-payload-packing.ll
	grep -q "@ari_builtin_assert_ne_bool" $(BUILD_DIR)/prelude/rare-payload-packing.ll
	grep -q "call i64 @ari_builtin_write_i64(i64 4)" $(BUILD_DIR)/prelude/rare-payload-packing.ll
	grep -q "@ari_builtin_write_bool" $(BUILD_DIR)/prelude/rare-payload-packing.ll
	$(TARGET) tests/cases/standard-library/ok/input/prelude-input.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-input.ll
	grep -q "@ari_builtin_read_byte" $(BUILD_DIR)/prelude/prelude-input.ll
	grep -q "@ari_builtin_context_arg" $(BUILD_DIR)/prelude/prelude-input.ll
	grep -q "declare i32 @getchar" $(BUILD_DIR)/prelude/prelude-input.ll
	$(TARGET) tests/cases/standard-library/ok/input/std-input-byte-option.ari --emit-llvm $(BUILD_DIR)/prelude/std-input-byte-option.ll
	grep -q '_ARNv3std5input13try_read_byte' $(BUILD_DIR)/prelude/std-input-byte-option.ll
	grep -q "@ari_builtin_read_byte" $(BUILD_DIR)/prelude/std-input-byte-option.ll
	$(TARGET) tests/cases/standard-library/ok/input/std-input-byte-option.ari -o $(BUILD_DIR)/prelude/std-input-byte-option.elf
	printf 'A' | $(BUILD_DIR)/prelude/std-input-byte-option.elf; code=$$?; test $$code -eq 72
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/input/std-input-byte-option.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-input-byte-option.elf; printf 'A' | $(BUILD_DIR)/prelude/std-input-byte-option.elf; code=$$?; test $$code -eq 72; fi
	$(TARGET) tests/cases/standard-library/ok/context/std-context-args.ari --emit-llvm $(BUILD_DIR)/prelude/std-context-args.ll
	grep -q "@ari_builtin_context_argc" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -q "@ari_builtin_context_thread_id" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -q "@ari_builtin_context_cwd" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -q "@ari_builtin_context_executable_path" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -q "@ari_context_cwd_buffer" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -q "@ari_thread_id" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context7has_arg" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context8has_args" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context14user_arg_count" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context13has_user_args" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context14is_main_thread" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context7has_cwd" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context8cwd_path" $(BUILD_DIR)/prelude/std-context-args.ll
	grep -F -q "std7context19try_executable_path" $(BUILD_DIR)/prelude/std-context-args.ll
	$(TARGET) tests/cases/standard-library/ok/context/std-context-args.ari -o $(BUILD_DIR)/prelude/std-context-args.elf
	$(BUILD_DIR)/prelude/std-context-args.elf; code=$$?; test $$code -eq 123
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/context/std-context-args.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-context-args.elf; $(BUILD_DIR)/prelude/std-context-args.elf; code=$$?; test $$code -eq 123; fi
	$(TARGET) tests/cases/standard-library/ok/env/std-env-args.ari --emit-llvm $(BUILD_DIR)/prelude/std-env-args.ll
	grep -q "@ari_builtin_context_argc" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env9arg_count" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env3arg" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env4args" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env7args_os" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env12arg_optional" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env13arg_unchecked" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env7try_arg" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env6arg_os" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env15arg_os_optional" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env12program_name" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env21program_name_optional" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env15program_name_os" $(BUILD_DIR)/prelude/std-env-args.ll
	grep -F -q "std3env24program_name_os_optional" $(BUILD_DIR)/prelude/std-env-args.ll
	$(TARGET) tests/cases/standard-library/ok/env/std-env-args.ari -o $(BUILD_DIR)/prelude/std-env-args.elf
	$(BUILD_DIR)/prelude/std-env-args.elf; code=$$?; test $$code -eq 98
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/env/std-env-args.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-env-args.elf; $(BUILD_DIR)/prelude/std-env-args.elf; code=$$?; test $$code -eq 98; fi
	$(TARGET) tests/cases/standard-library/ok/env/std-env-vars.ari --emit-llvm $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@ari_builtin_env_get" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@ari_builtin_env_has" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@ari_builtin_env_set" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@ari_builtin_env_remove" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@getenv" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@setenv" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -q "@unsetenv" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env7try_get" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env3var" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env12var_optional" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env6var_os" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env15var_os_optional" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env7set_var" $(BUILD_DIR)/prelude/std-env-vars.ll
	grep -F -q "std3env10remove_var" $(BUILD_DIR)/prelude/std-env-vars.ll
	$(TARGET) tests/cases/standard-library/ok/env/std-env-vars.ari -o $(BUILD_DIR)/prelude/std-env-vars.elf
	$(BUILD_DIR)/prelude/std-env-vars.elf; code=$$?; test $$code -eq 158
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/env/std-env-vars.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-env-vars.elf; $(BUILD_DIR)/prelude/std-env-vars.elf; code=$$?; test $$code -eq 158; fi
	$(TARGET) tests/cases/standard-library/ok/env/std-env-paths.ari --emit-llvm $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -q "@ari_builtin_env_current_dir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -q "@ari_builtin_env_set_current_dir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -q "@ari_builtin_env_executable_path" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -q "@getcwd" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -q "@chdir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -q "@readlink" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env11current_dir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env15executable_path" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env15set_current_dir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env20current_dir_optional" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env15try_current_dir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env8home_dir" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env24executable_path_optional" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env19try_executable_path" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env20executable_path_path" $(BUILD_DIR)/prelude/std-env-paths.ll
	grep -F -q "std3env29executable_path_path_optional" $(BUILD_DIR)/prelude/std-env-paths.ll
	$(TARGET) tests/cases/standard-library/ok/env/std-env-paths.ari -o $(BUILD_DIR)/prelude/std-env-paths.elf
	$(BUILD_DIR)/prelude/std-env-paths.elf; code=$$?; test $$code -eq 56
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/env/std-env-paths.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-env-paths.elf; $(BUILD_DIR)/prelude/std-env-paths.elf; code=$$?; test $$code -eq 56; fi
	$(TARGET) tests/cases/standard-library/ok/env/std-env-os-path-views.ari --emit-llvm $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env6arg_os" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env15arg_os_optional" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env16arg_os_unchecked" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env10try_arg_os" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env15program_name_os" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env24program_name_os_optional" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env6get_os" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env10try_get_os" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env16current_dir_path" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env23current_dir_os_optional" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env25current_dir_os_or_default" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env25current_dir_path_optional" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env27current_dir_path_or_default" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env27executable_path_os_optional" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env29executable_path_os_or_default" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	grep -F -q "std3env22try_executable_path_os" $(BUILD_DIR)/prelude/std-env-os-path-views.ll
	$(TARGET) tests/cases/standard-library/ok/env/std-env-os-path-views.ari -o $(BUILD_DIR)/prelude/std-env-os-path-views.elf
	$(BUILD_DIR)/prelude/std-env-os-path-views.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/env/std-env-os-path-views.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-env-os-path-views.elf; $(BUILD_DIR)/prelude/std-env-os-path-views.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "@ari_builtin_fs_exists" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "@ari_builtin_fs_remove" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "@ari_builtin_fs_open" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "@ari_builtin_fs_close" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "@ari_builtin_fs_read_byte" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "@ari_builtin_fs_write_byte" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "declare i32 @access" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "declare i32 @unlink" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "declare i32 @open" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "declare i64 @read" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "declare i64 @write" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -q "declare i32 @close" $(BUILD_DIR)/prelude/std-fs-basic.ll
	grep -F -q "std2fs8try_open" $(BUILD_DIR)/prelude/std-fs-basic.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-basic.ari -o $(BUILD_DIR)/prelude/std-fs-basic.elf
	$(BUILD_DIR)/prelude/std-fs-basic.elf; code=$$?; test $$code -eq 120
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-basic.elf; $(BUILD_DIR)/prelude/std-fs-basic.elf; code=$$?; test $$code -eq 120; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-try-byte.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-try-byte.ll
	grep -F -q "std2fs13try_read_byte" $(BUILD_DIR)/prelude/std-fs-try-byte.ll
	grep -F -q "std__fs__File8inherent13try_read_byte" $(BUILD_DIR)/prelude/std-fs-try-byte.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-try-byte.ari -o $(BUILD_DIR)/prelude/std-fs-try-byte.elf
	$(BUILD_DIR)/prelude/std-fs-try-byte.elf; code=$$?; test $$code -eq 55
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-try-byte.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-try-byte.elf; $(BUILD_DIR)/prelude/std-fs-try-byte.elf; code=$$?; test $$code -eq 55; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-append.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-append.ll
	grep -q "@ari_builtin_fs_open" $(BUILD_DIR)/prelude/std-fs-append.ll
	grep -q "declare i32 @open" $(BUILD_DIR)/prelude/std-fs-append.ll
	grep -F -q "std2fs8try_open" $(BUILD_DIR)/prelude/std-fs-append.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-append.ari -o $(BUILD_DIR)/prelude/std-fs-append.elf
	$(BUILD_DIR)/prelude/std-fs-append.elf; code=$$?; test $$code -eq 153
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-append.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-append.elf; $(BUILD_DIR)/prelude/std-fs-append.elf; code=$$?; test $$code -eq 153; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-modes.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-open-modes.ll
	grep -q "@ari_builtin_fs_open" $(BUILD_DIR)/prelude/std-fs-open-modes.ll
	grep -q "declare i32 @open" $(BUILD_DIR)/prelude/std-fs-open-modes.ll
	grep -F -q "std2fs8try_open" $(BUILD_DIR)/prelude/std-fs-open-modes.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-modes.ari -o $(BUILD_DIR)/prelude/std-fs-open-modes.elf
	$(BUILD_DIR)/prelude/std-fs-open-modes.elf; code=$$?; test $$code -eq 34
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-modes.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-open-modes.elf; $(BUILD_DIR)/prelude/std-fs-open-modes.elf; code=$$?; test $$code -eq 34; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-options.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-open-options.ll
	grep -q "@ari_builtin_fs_open_options" $(BUILD_DIR)/prelude/std-fs-open-options.ll
	grep -F -q "std2fs12open_options" $(BUILD_DIR)/prelude/std-fs-open-options.ll
	grep -F -q "OpenOptions8inherent10create_new" $(BUILD_DIR)/prelude/std-fs-open-options.ll
	grep -F -q "OpenOptions8inherent8try_open" $(BUILD_DIR)/prelude/std-fs-open-options.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-options.ari -o $(BUILD_DIR)/prelude/std-fs-open-options.elf
	$(BUILD_DIR)/prelude/std-fs-open-options.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs4open" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs13open_optional" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs8open_raw" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs6create" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs15create_optional" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs16create_unchecked" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs10create_raw" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs9open_read" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs18open_read_optional" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs19open_read_unchecked" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs10open_write" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs19open_write_optional" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs20open_write_unchecked" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs11open_append" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs20open_append_optional" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "std2fs21open_append_unchecked" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "OpenOptions8inherent4open" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "OpenOptions8inherent13open_optional" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "OpenOptions8inherent14open_unchecked" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	grep -F -q "OpenOptions8inherent8open_raw" $(BUILD_DIR)/prelude/std-fs-open-result.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-open-result.ari -o $(BUILD_DIR)/prelude/std-fs-open-result.elf
	$(BUILD_DIR)/prelude/std-fs-open-result.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-lock.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-lock.ll
	grep -q 'declare i32 @"flock"' $(BUILD_DIR)/prelude/std-fs-lock.ll
	grep -q 'std2fs11lock_shared' $(BUILD_DIR)/prelude/std-fs-lock.ll
	grep -q 'std2fs14lock_exclusive' $(BUILD_DIR)/prelude/std-fs-lock.ll
	grep -q 'std2fs6unlock' $(BUILD_DIR)/prelude/std-fs-lock.ll
	grep -q 'File8inherent15try_lock_shared' $(BUILD_DIR)/prelude/std-fs-lock.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-lock.ari -o $(BUILD_DIR)/prelude/std-fs-lock.elf
	$(BUILD_DIR)/prelude/std-fs-lock.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-temp.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-temp.ll
	grep -q 'std2fs9temp_file' $(BUILD_DIR)/prelude/std-fs-temp.ll
	grep -q 'std2fs12temp_file_in' $(BUILD_DIR)/prelude/std-fs-temp.ll
	grep -q 'std2fs8temp_dir' $(BUILD_DIR)/prelude/std-fs-temp.ll
	grep -q 'std2fs11temp_dir_in' $(BUILD_DIR)/prelude/std-fs-temp.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-temp.ari -o $(BUILD_DIR)/prelude/std-fs-temp.elf
	$(BUILD_DIR)/prelude/std-fs-temp.elf; code=$$?; test $$code -eq 15
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-write.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs5write" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs6append" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs9try_write" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs10try_append" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs14read_to_string" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs25read_to_string_or_default" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	grep -F -q "std2fs24read_to_string_unchecked" $(BUILD_DIR)/prelude/std-fs-read-write.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-write.ari -o $(BUILD_DIR)/prelude/std-fs-read-write.elf
	$(BUILD_DIR)/prelude/std-fs-read-write.elf; code=$$?; test $$code -eq 136
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-write.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-read-write.elf; $(BUILD_DIR)/prelude/std-fs-read-write.elf; code=$$?; test $$code -eq 136; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-read-result.ll
	grep -F -q "std2fs4read" $(BUILD_DIR)/prelude/std-fs-read-result.ll
	grep -F -q "std2fs13read_optional" $(BUILD_DIR)/prelude/std-fs-read-result.ll
	grep -F -q "std2fs14read_unchecked" $(BUILD_DIR)/prelude/std-fs-read-result.ll
	grep -F -q "std2fs14read_to_string" $(BUILD_DIR)/prelude/std-fs-read-result.ll
	grep -F -q "std2fs24read_to_string_unchecked" $(BUILD_DIR)/prelude/std-fs-read-result.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-result.ari -o $(BUILD_DIR)/prelude/std-fs-read-result.elf
	$(BUILD_DIR)/prelude/std-fs-read-result.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-byte-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs5write" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs9write_raw" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs6append" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs10append_raw" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs4copy" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs8copy_raw" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs10read_bytes" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	grep -F -q "std2fs12write_string" $(BUILD_DIR)/prelude/std-fs-byte-result.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-byte-result.ari -o $(BUILD_DIR)/prelude/std-fs-byte-result.elf
	$(BUILD_DIR)/prelude/std-fs-byte-result.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-try-read.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-try-read.ll
	grep -F -q "std2fs18try_read_to_string" $(BUILD_DIR)/prelude/std-fs-try-read.ll
	grep -F -q "std2fs8try_read" $(BUILD_DIR)/prelude/std-fs-try-read.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-try-read.ari -o $(BUILD_DIR)/prelude/std-fs-try-read.elf
	$(BUILD_DIR)/prelude/std-fs-try-read.elf; code=$$?; test $$code -eq 78
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-try-read.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-try-read.elf; $(BUILD_DIR)/prelude/std-fs-try-read.elf; code=$$?; test $$code -eq 78; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-create-truncate-copy.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	grep -F -q "std2fs10try_create" $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	grep -F -q "std2fs4read" $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	grep -F -q "std2fs8truncate" $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	grep -F -q "std2fs4copy" $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	grep -F -q "std2fs9copy_bool" $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	grep -F -q "std2fs8try_copy" $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-create-truncate-copy.ari -o $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.elf
	$(BUILD_DIR)/prelude/std-fs-create-truncate-copy.elf; code=$$?; test $$code -eq 171
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-create-truncate-copy.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.elf; $(BUILD_DIR)/prelude/std-fs-create-truncate-copy.elf; code=$$?; test $$code -eq 171; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-io-traits.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-io-traits.ll
	grep -F -q "std2io31read_to_string__G_std__fs__File" $(BUILD_DIR)/prelude/std-fs-io-traits.ll
	grep -F -q "std2io39try_copy__G_std__fs__File_std__fs__File" $(BUILD_DIR)/prelude/std-fs-io-traits.ll
	grep -F -q "std2io26write_all__G_std__fs__File" $(BUILD_DIR)/prelude/std-fs-io-traits.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-io-traits.ari -o $(BUILD_DIR)/prelude/std-fs-io-traits.elf
	$(BUILD_DIR)/prelude/std-fs-io-traits.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-seek.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-seek.ll
	grep -q "@ari_builtin_fs_position" $(BUILD_DIR)/prelude/std-fs-seek.ll
	grep -q "@ari_builtin_fs_seek" $(BUILD_DIR)/prelude/std-fs-seek.ll
	grep -q "declare i64 @lseek" $(BUILD_DIR)/prelude/std-fs-seek.ll
	grep -F -q "rewind__G_std__fs__File" $(BUILD_DIR)/prelude/std-fs-seek.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-seek.ari -o $(BUILD_DIR)/prelude/std-fs-seek.elf
	$(BUILD_DIR)/prelude/std-fs-seek.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-rename-dir.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	grep -q "@ari_builtin_fs_rename" $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	grep -q "@ari_builtin_fs_create_dir" $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	grep -q "@ari_builtin_fs_remove_dir" $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	grep -q "declare i32 @rename" $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	grep -q "declare i32 @mkdir" $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	grep -q "declare i32 @rmdir" $(BUILD_DIR)/prelude/std-fs-rename-dir.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-rename-dir.ari -o $(BUILD_DIR)/prelude/std-fs-rename-dir.elf
	$(BUILD_DIR)/prelude/std-fs-rename-dir.elf; code=$$?; test $$code -eq 120
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-rename-dir.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-rename-dir.elf; $(BUILD_DIR)/prelude/std-fs-rename-dir.elf; code=$$?; test $$code -eq 120; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-mutation-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs6remove" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs11remove_file" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs10remove_raw" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs6rename" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs10rename_raw" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs10create_dir" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs14create_dir_raw" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs10remove_dir" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	grep -F -q "std2fs14remove_dir_raw" $(BUILD_DIR)/prelude/std-fs-mutation-result.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-mutation-result.ari -o $(BUILD_DIR)/prelude/std-fs-mutation-result.elf
	$(BUILD_DIR)/prelude/std-fs-mutation-result.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-ensure-dir.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-ensure-dir.ll
	grep -F -q "std2fs10ensure_dir" $(BUILD_DIR)/prelude/std-fs-ensure-dir.ll
	grep -q "@ari_builtin_fs_create_dir" $(BUILD_DIR)/prelude/std-fs-ensure-dir.ll
	grep -q "declare i32 @mkdir" $(BUILD_DIR)/prelude/std-fs-ensure-dir.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-ensure-dir.ari -o $(BUILD_DIR)/prelude/std-fs-ensure-dir.elf
	$(BUILD_DIR)/prelude/std-fs-ensure-dir.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-create-dir-all.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-create-dir-all.ll
	grep -F -q "std2fs14ensure_dir_all" $(BUILD_DIR)/prelude/std-fs-create-dir-all.ll
	grep -F -q "std2fs14create_dir_all" $(BUILD_DIR)/prelude/std-fs-create-dir-all.ll
	grep -F -q "std2fs19create_dir_all_bool" $(BUILD_DIR)/prelude/std-fs-create-dir-all.ll
	grep -q "@ari_builtin_fs_create_dir_all" $(BUILD_DIR)/prelude/std-fs-create-dir-all.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-create-dir-all.ari -o $(BUILD_DIR)/prelude/std-fs-create-dir-all.elf
	$(BUILD_DIR)/prelude/std-fs-create-dir-all.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-remove-dir-all.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-remove-dir-all.ll
	grep -F -q "std2fs14remove_dir_all" $(BUILD_DIR)/prelude/std-fs-remove-dir-all.ll
	grep -F -q "std2fs19remove_dir_all_bool" $(BUILD_DIR)/prelude/std-fs-remove-dir-all.ll
	grep -q "@ari_builtin_fs_remove_bytes" $(BUILD_DIR)/prelude/std-fs-remove-dir-all.ll
	grep -q "@ari_builtin_fs_remove_dir_bytes" $(BUILD_DIR)/prelude/std-fs-remove-dir-all.ll
	grep -q "@ari_builtin_fs_open_dir_bytes" $(BUILD_DIR)/prelude/std-fs-remove-dir-all.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-remove-dir-all.ari -o $(BUILD_DIR)/prelude/std-fs-remove-dir-all.elf
	$(BUILD_DIR)/prelude/std-fs-remove-dir-all.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-ensure-file.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-ensure-file.ll
	grep -F -q "std2fs11ensure_file" $(BUILD_DIR)/prelude/std-fs-ensure-file.ll
	grep -F -q "std2fs10try_create" $(BUILD_DIR)/prelude/std-fs-ensure-file.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-ensure-file.ari -o $(BUILD_DIR)/prelude/std-fs-ensure-file.elf
	$(BUILD_DIR)/prelude/std-fs-ensure-file.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-dir.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -q "@ari_builtin_fs_open_dir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -q "@ari_builtin_fs_read_dir_next" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -q "@ari_builtin_fs_close_dir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -q "declare ptr @opendir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -q "declare ptr @readdir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -q "declare i32 @closedir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs12try_open_dir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs13read_dir_next" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs12try_read_dir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs14read_dir_names" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs24read_dir_names_unchecked" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs8read_dir" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs18read_dir_unchecked" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs20try_read_dir_entries" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	grep -F -q "std2fs16read_dir_entries" $(BUILD_DIR)/prelude/std-fs-read-dir.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-dir.ari -o $(BUILD_DIR)/prelude/std-fs-read-dir.elf
	$(BUILD_DIR)/prelude/std-fs-read-dir.elf; code=$$?; test $$code -eq 253
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-dir.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-read-dir.elf; $(BUILD_DIR)/prelude/std-fs-read-dir.elf; code=$$?; test $$code -eq 253; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-dir-entry-metadata.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -q "@ari_builtin_fs_metadata_size_bytes" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -q "@ari_builtin_fs_symlink_metadata_kind_bytes" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -q "@ari_builtin_fs_can_read_bytes" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent8metadata" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent17metadata_optional" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent18metadata_unchecked" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent12try_metadata" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent16symlink_metadata" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent25symlink_metadata_optional" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent26symlink_metadata_unchecked" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent20try_symlink_metadata" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent9file_type" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent18file_type_optional" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent19file_type_unchecked" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	grep -F -q "DirEntry8inherent13try_file_type" $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-dir-entry-metadata.ari -o $(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.elf
	$(BUILD_DIR)/prelude/std-fs-dir-entry-metadata.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-dir-entry-info.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-dir-entry-info.ll
	grep -F -q "std2fs13read_dir_info" $(BUILD_DIR)/prelude/std-fs-dir-entry-info.ll
	grep -F -q "std2fs22read_dir_info_detailed" $(BUILD_DIR)/prelude/std-fs-dir-entry-info.ll
	grep -F -q "DirEntryInfo8inherent11metadata_ok" $(BUILD_DIR)/prelude/std-fs-dir-entry-info.ll
	grep -F -q "DirEntryInfo8inherent12metadata_err" $(BUILD_DIR)/prelude/std-fs-dir-entry-info.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-dir-entry-info.ari -o $(BUILD_DIR)/prelude/std-fs-dir-entry-info.elf
	$(BUILD_DIR)/prelude/std-fs-dir-entry-info.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-links.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-links.ll
	grep -q "@ari_builtin_fs_hard_link" $(BUILD_DIR)/prelude/std-fs-links.ll
	grep -q "@ari_builtin_fs_symbolic_link" $(BUILD_DIR)/prelude/std-fs-links.ll
	grep -q "declare i32 @link" $(BUILD_DIR)/prelude/std-fs-links.ll
	grep -q "declare i32 @symlink" $(BUILD_DIR)/prelude/std-fs-links.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-links.ari -o $(BUILD_DIR)/prelude/std-fs-links.elf
	$(BUILD_DIR)/prelude/std-fs-links.elf; code=$$?; test $$code -eq 63
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-links.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-links.elf; $(BUILD_DIR)/prelude/std-fs-links.elf; code=$$?; test $$code -eq 63; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-link.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-read-link.ll
	grep -q "@ari_builtin_fs_read_link" $(BUILD_DIR)/prelude/std-fs-read-link.ll
	grep -q "declare i64 @readlink" $(BUILD_DIR)/prelude/std-fs-read-link.ll
	grep -F -q "std2fs13try_read_link" $(BUILD_DIR)/prelude/std-fs-read-link.ll
	grep -F -q "std2fs9read_link" $(BUILD_DIR)/prelude/std-fs-read-link.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-read-link.ari -o $(BUILD_DIR)/prelude/std-fs-read-link.elf
	$(BUILD_DIR)/prelude/std-fs-read-link.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-symlink-metadata.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -q "@ari_builtin_fs_symlink_metadata_size" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -q "@ari_builtin_fs_symlink_metadata_kind" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -q "declare i32 @lstat" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -F -q "std2fs20try_symlink_metadata" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -F -q "std2fs16symlink_metadata" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -F -q "std2fs25symlink_metadata_optional" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -F -q "std2fs26symlink_metadata_unchecked" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	grep -F -q "std2fs10is_symlink" $(BUILD_DIR)/prelude/std-fs-symlink-metadata.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-symlink-metadata.ari -o $(BUILD_DIR)/prelude/std-fs-symlink-metadata.elf
	$(BUILD_DIR)/prelude/std-fs-symlink-metadata.elf; code=$$?; test $$code -eq 127
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-permissions.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-permissions.ll
	grep -q "@ari_builtin_fs_can_read" $(BUILD_DIR)/prelude/std-fs-permissions.ll
	grep -q "@ari_builtin_fs_can_write" $(BUILD_DIR)/prelude/std-fs-permissions.ll
	grep -q "@ari_builtin_fs_can_execute" $(BUILD_DIR)/prelude/std-fs-permissions.ll
	grep -F -q "std2fs11permissions" $(BUILD_DIR)/prelude/std-fs-permissions.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-permissions.ari -o $(BUILD_DIR)/prelude/std-fs-permissions.elf
	$(BUILD_DIR)/prelude/std-fs-permissions.elf; code=$$?; test $$code -eq 171
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-permissions.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-permissions.elf; $(BUILD_DIR)/prelude/std-fs-permissions.elf; code=$$?; test $$code -eq 171; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-mode.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -q "@ari_builtin_fs_metadata_mode" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -q "@ari_builtin_fs_set_mode" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -q "declare i32 @chmod" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -F -q "std2fs8try_mode" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -F -q "std2fs4mode" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -F -q "std2fs13mode_optional" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -F -q "std2fs14mode_unchecked" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -F -q "std2fs8set_mode" $(BUILD_DIR)/prelude/std-fs-mode.ll
	grep -F -q "std2fs15set_permissions" $(BUILD_DIR)/prelude/std-fs-mode.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-mode.ari -o $(BUILD_DIR)/prelude/std-fs-mode.elf
	$(BUILD_DIR)/prelude/std-fs-mode.elf; code=$$?; test $$code -eq 91
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-mode.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-mode.elf; $(BUILD_DIR)/prelude/std-fs-mode.elf; code=$$?; test $$code -eq 91; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -q "@ari_builtin_fs_metadata_size" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -q "@ari_builtin_fs_metadata_kind" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -q "declare i32 @stat" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs12try_metadata" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs13try_file_type" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs7is_file" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs8metadata" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs17metadata_optional" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs18metadata_unchecked" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs9file_type" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	grep -F -q "std2fs18file_type_optional" $(BUILD_DIR)/prelude/std-fs-metadata.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata.ari -o $(BUILD_DIR)/prelude/std-fs-metadata.elf
	$(BUILD_DIR)/prelude/std-fs-metadata.elf; code=$$?; test $$code -eq 129
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-metadata.elf; $(BUILD_DIR)/prelude/std-fs-metadata.elf; code=$$?; test $$code -eq 129; fi
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-query-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs8metadata" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs17metadata_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs16symlink_metadata" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs25symlink_metadata_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs9file_type" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs18file_type_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs4mode" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs13mode_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs8open_dir" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs14read_dir_names" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs23read_dir_names_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs8read_dir" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs17read_dir_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs16read_dir_entries" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs25read_dir_entries_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs12canonicalize" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs21canonicalize_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs9read_link" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	grep -F -q "std2fs18read_link_optional" $(BUILD_DIR)/prelude/std-fs-query-result.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-query-result.ari -o $(BUILD_DIR)/prelude/std-fs-query-result.elf
	$(BUILD_DIR)/prelude/std-fs-query-result.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-detailed-errors.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	grep -F -q "std2fs13open_detailed" $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	grep -F -q "std2fs14write_detailed" $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	grep -F -q "std2fs15rename_detailed" $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	grep -F -q "std2fs17metadata_detailed" $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	grep -F -q "PathError8inherent11path_equals" $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	grep -F -q "TwoPathError8inherent13source_equals" $(BUILD_DIR)/prelude/std-fs-detailed-errors.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-detailed-errors.ari -o $(BUILD_DIR)/prelude/std-fs-detailed-errors.elf
	$(BUILD_DIR)/prelude/std-fs-detailed-errors.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata-times.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -q "@ari_builtin_fs_metadata_accessed_nanos" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -q "@ari_builtin_fs_metadata_modified_nanos" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -q "@ari_builtin_fs_metadata_changed_nanos" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -q "@ari_builtin_fs_symlink_metadata_changed_nanos_bytes" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -F -q "Metadata8inherent8accessed" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -F -q "Metadata8inherent8modified" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	grep -F -q "Metadata8inherent7changed" $(BUILD_DIR)/prelude/std-fs-metadata-times.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata-times.ari -o $(BUILD_DIR)/prelude/std-fs-metadata-times.elf
	$(BUILD_DIR)/prelude/std-fs-metadata-times.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata-owner.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-metadata-owner.ll
	grep -q "@ari_builtin_fs_metadata_uid" $(BUILD_DIR)/prelude/std-fs-metadata-owner.ll
	grep -q "@ari_builtin_fs_metadata_gid" $(BUILD_DIR)/prelude/std-fs-metadata-owner.ll
	grep -F -q "Metadata8inherent5owner" $(BUILD_DIR)/prelude/std-fs-metadata-owner.ll
	grep -F -q "Metadata8inherent7created" $(BUILD_DIR)/prelude/std-fs-metadata-owner.ll
	grep -F -q "Metadata8inherent18created_or_changed" $(BUILD_DIR)/prelude/std-fs-metadata-owner.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-metadata-owner.ari -o $(BUILD_DIR)/prelude/std-fs-metadata-owner.elf
	$(BUILD_DIR)/prelude/std-fs-metadata-owner.elf; code=$$?; test $$code -eq 127
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-canonicalize.ari --emit-llvm $(BUILD_DIR)/prelude/std-fs-canonicalize.ll
	grep -q "@ari_builtin_fs_canonicalize" $(BUILD_DIR)/prelude/std-fs-canonicalize.ll
	grep -q "declare ptr @realpath" $(BUILD_DIR)/prelude/std-fs-canonicalize.ll
	grep -F -q "std2fs16try_canonicalize" $(BUILD_DIR)/prelude/std-fs-canonicalize.ll
	grep -F -q "std2fs12canonicalize" $(BUILD_DIR)/prelude/std-fs-canonicalize.ll
	$(TARGET) tests/cases/standard-library/ok/fs/std-fs-canonicalize.ari -o $(BUILD_DIR)/prelude/std-fs-canonicalize.elf
	$(BUILD_DIR)/prelude/std-fs-canonicalize.elf; code=$$?; test $$code -eq 28
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/fs/std-fs-canonicalize.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fs-canonicalize.elf; $(BUILD_DIR)/prelude/std-fs-canonicalize.elf; code=$$?; test $$code -eq 28; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-fd.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-fd.ll
	grep -F -q "std2os2fd" $(BUILD_DIR)/prelude/std-os-fd.ll
	grep -F -q "std__os__Fd" $(BUILD_DIR)/prelude/std-os-fd.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-fd.ari -o $(BUILD_DIR)/prelude/std-os-fd.elf
	$(BUILD_DIR)/prelude/std-os-fd.elf; code=$$?; test $$code -eq 63
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-fd.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-fd.elf; $(BUILD_DIR)/prelude/std-os-fd.elf; code=$$?; test $$code -eq 63; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-owned-fd.ll
	grep -F -q "std__os__OwnedFd" $(BUILD_DIR)/prelude/std-os-owned-fd.ll
	grep -q "@ari_builtin_os_close" $(BUILD_DIR)/prelude/std-os-owned-fd.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd.ari -o $(BUILD_DIR)/prelude/std-os-owned-fd.elf
	$(BUILD_DIR)/prelude/std-os-owned-fd.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-owned-fd.elf; $(BUILD_DIR)/prelude/std-os-owned-fd.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-duplicate.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.ll
	grep -F -q "std__os__OwnedFd" $(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.ll
	grep -q "@ari_builtin_os_dup" $(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-duplicate.ari -o $(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.elf
	$(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-duplicate.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.elf; $(BUILD_DIR)/prelude/std-os-owned-fd-duplicate.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-flags.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-owned-fd-flags.ll
	grep -q "@ari_builtin_os_close_on_exec" $(BUILD_DIR)/prelude/std-os-owned-fd-flags.ll
	grep -q "@ari_builtin_os_set_close_on_exec" $(BUILD_DIR)/prelude/std-os-owned-fd-flags.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-flags.ari -o $(BUILD_DIR)/prelude/std-os-owned-fd-flags.elf
	$(BUILD_DIR)/prelude/std-os-owned-fd-flags.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-flags.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-owned-fd-flags.elf; $(BUILD_DIR)/prelude/std-os-owned-fd-flags.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-nonblocking.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.ll
	grep -q "@ari_builtin_os_nonblocking" $(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.ll
	grep -q "@ari_builtin_os_set_nonblocking" $(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-nonblocking.ari -o $(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.elf
	$(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-owned-fd-nonblocking.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.elf; $(BUILD_DIR)/prelude/std-os-owned-fd-nonblocking.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-pipe.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-pipe.ll
	grep -F -q "std__os__Pipe" $(BUILD_DIR)/prelude/std-os-pipe.ll
	grep -q "@ari_builtin_os_pipe" $(BUILD_DIR)/prelude/std-os-pipe.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-pipe.ari -o $(BUILD_DIR)/prelude/std-os-pipe.elf
	$(BUILD_DIR)/prelude/std-os-pipe.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-pipe.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-pipe.elf; $(BUILD_DIR)/prelude/std-os-pipe.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/os/std-os-poll.ari --emit-llvm $(BUILD_DIR)/prelude/std-os-poll.ll
	grep -q "@ari_builtin_os_poll_read" $(BUILD_DIR)/prelude/std-os-poll.ll
	grep -q "@ari_builtin_os_poll_write" $(BUILD_DIR)/prelude/std-os-poll.ll
	grep -q "declare i32 @poll" $(BUILD_DIR)/prelude/std-os-poll.ll
	$(TARGET) tests/cases/standard-library/ok/os/std-os-poll.ari -o $(BUILD_DIR)/prelude/std-os-poll.elf
	$(BUILD_DIR)/prelude/std-os-poll.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/os/std-os-poll.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-os-poll.elf; $(BUILD_DIR)/prelude/std-os-poll.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/net/std-net-addresses.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-addresses.ll
	grep -F -q "std3net4ipv4" $(BUILD_DIR)/prelude/std-net-addresses.ll
	grep -F -q "std3net4ipv6" $(BUILD_DIR)/prelude/std-net-addresses.ll
	grep -F -q "std3net11socket_addr" $(BUILD_DIR)/prelude/std-net-addresses.ll
	grep -F -q "std3net9localhost" $(BUILD_DIR)/prelude/std-net-addresses.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-addresses.ari -o $(BUILD_DIR)/prelude/std-net-addresses.elf
	$(BUILD_DIR)/prelude/std-net-addresses.elf; code=$$?; test $$code -eq 120
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/net/std-net-addresses.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-net-addresses.elf; $(BUILD_DIR)/prelude/std-net-addresses.elf; code=$$?; test $$code -eq 120; fi
	$(TARGET) tests/cases/standard-library/ok/net/std-net-address-validation.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-address-validation.ll
	grep -F -q "std__net__Ipv4Addr8inherent9try_octet" $(BUILD_DIR)/prelude/std-net-address-validation.ll
	grep -F -q "std__net__Ipv6Addr8inherent11try_segment" $(BUILD_DIR)/prelude/std-net-address-validation.ll
	grep -F -q "std__net__Ipv4Addr8inherent5octet" $(BUILD_DIR)/prelude/std-net-address-validation.ll
	grep -F -q "std__net__Ipv6Addr8inherent7segment" $(BUILD_DIR)/prelude/std-net-address-validation.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-address-validation.ari -o $(BUILD_DIR)/prelude/std-net-address-validation.elf
	$(BUILD_DIR)/prelude/std-net-address-validation.elf; code=$$?; test $$code -eq 55
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/net/std-net-address-validation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-net-address-validation.elf; $(BUILD_DIR)/prelude/std-net-address-validation.elf; code=$$?; test $$code -eq 55; fi
	$(TARGET) tests/cases/standard-library/ok/net/std-net-error-policy.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-error-policy.ll
	grep -F -q "std3net10is_timeout" $(BUILD_DIR)/prelude/std-net-error-policy.ll
	grep -F -q "std3net14is_would_block" $(BUILD_DIR)/prelude/std-net-error-policy.ll
	grep -F -q "std3net12is_retryable" $(BUILD_DIR)/prelude/std-net-error-policy.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-error-policy.ari -o $(BUILD_DIR)/prelude/std-net-error-policy.elf
	$(BUILD_DIR)/prelude/std-net-error-policy.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/standard-library/ok/net/std-net-tcp-loopback.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_tcp_listen_v4" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_tcp_connect_v4" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_tcp_accept" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_local_port" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_local_addr_v4" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_peer_addr_v4" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_set_read_timeout_millis" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_reuse_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_set_reuse_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_ttl" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_set_ttl" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_linger_enabled" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_linger_seconds" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_set_linger" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_nodelay" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_set_nodelay" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -q "@ari_builtin_net_shutdown" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent10local_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent18set_accept_timeout" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent13bind_optional" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent8bind_raw" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent15accept_optional" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent10accept_raw" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std3net6listen" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std3net10tcp_listen" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std3net11tcp_connect" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std3net7connect" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent10local_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent9peer_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent10read_exact" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent16connect_optional" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent16set_read_timeout" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent17set_write_timeout" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent11connect_raw" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent10reuse_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpListener8inherent14set_reuse_addr" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent7nodelay" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream8inherent11set_nodelay" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	grep -F -q "std__net__TcpStream15std__io__Writer9write_all" $(BUILD_DIR)/prelude/std-net-tcp-loopback.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-tcp-loopback.ari -o $(BUILD_DIR)/prelude/std-net-tcp-loopback.elf
	$(BUILD_DIR)/prelude/std-net-tcp-loopback.elf; code=$$?; test $$code -eq 254
	$(TARGET) tests/cases/standard-library/ok/net/std-net-udp-socket.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_udp_bind_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_local_addr_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_udp_send_byte_to_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_udp_recv_byte" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_reuse_addr" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_set_reuse_addr" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_ttl" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_set_ttl" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_multicast_loop_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_set_multicast_loop_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_multicast_ttl_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_join_multicast_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -q "@ari_builtin_net_leave_multicast_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std3net8udp_bind" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent10local_addr" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent13bind_optional" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent8bind_raw" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent16set_read_timeout" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent17set_write_timeout" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent10reuse_addr" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent14set_reuse_addr" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent17multicast_loop_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent20set_multicast_ttl_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	grep -F -q "std__net__UdpSocket8inherent17join_multicast_v4" $(BUILD_DIR)/prelude/std-net-udp-socket.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-udp-socket.ari -o $(BUILD_DIR)/prelude/std-net-udp-socket.elf
	$(BUILD_DIR)/prelude/std-net-udp-socket.elf; code=$$?; test $$code -eq 254
	$(TARGET) tests/cases/standard-library/ok/net/std-net-ipv6-socket.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_tcp_listen_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_tcp_connect_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_udp_bind_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_udp_send_byte_to_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_local_addr_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_peer_addr_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_lookup_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_lookup_v6_endpoint" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_hop_limit" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -q "@ari_builtin_net_set_hop_limit" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -F -q "std3net13tcp_listen_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -F -q "std3net14tcp_connect_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -F -q "std3net11udp_bind_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -F -q "std__net__TcpStream8inherent12peer_addr_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	grep -F -q "std__net__UdpSocket8inherent13local_addr_v6" $(BUILD_DIR)/prelude/std-net-ipv6-socket.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-ipv6-socket.ari -o $(BUILD_DIR)/prelude/std-net-ipv6-socket.elf
	$(BUILD_DIR)/prelude/std-net-ipv6-socket.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/net/std-net-unix-socket.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -q "@ari_builtin_net_unix_listen" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -q "@ari_builtin_net_unix_connect" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -q "@ari_builtin_net_unix_datagram_bind" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -q "@ari_builtin_net_unix_datagram_unbound" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -q "@ari_builtin_net_unix_datagram_send_to" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -q "@ari_builtin_net_unix_peer_pid" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std3net11unix_listen" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std3net12unix_connect" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std3net13unix_datagram" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixListener8inherent13bind_optional" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixListener8inherent15accept_optional" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixListener8inherent8bind_raw" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixListener8inherent10accept_raw" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream8inherent16connect_optional" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream8inherent11connect_raw" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream8inherent16peer_credentials" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream8inherent10read_exact" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream8inherent16set_read_timeout" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream8inherent17set_write_timeout" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixPeerCredentials8inherent3pid" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixStream15std__io__Writer9write_all" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixDatagram8inherent7unbound" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixDatagram8inherent7send_to" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	grep -F -q "std__net__UnixDatagram8inherent4recv" $(BUILD_DIR)/prelude/std-net-unix-socket.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-unix-socket.ari -o $(BUILD_DIR)/prelude/std-net-unix-socket.elf
	$(BUILD_DIR)/prelude/std-net-unix-socket.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/net/std-net-dns-lookup.ari --emit-llvm $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -q "@ari_builtin_net_lookup_v4" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -q "@ari_builtin_net_lookup_v4_endpoint" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -q "@ari_builtin_net_lookup_v6" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -q "@ari_builtin_net_lookup_v6_endpoint" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -q "@getaddrinfo" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net9lookup_v4" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net9lookup_v6" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net18lookup_v4_optional" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net18lookup_v6_optional" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net13try_lookup_v4" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net13try_lookup_v6" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net13lookup_v4_raw" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net13lookup_v6_raw" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net7resolve" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net16resolve_optional" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net11try_resolve" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net11resolve_raw" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net12service_port" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net15resolve_service" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net15to_socket_addrs" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net23to_socket_addrs_service" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net12connect_host" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	grep -F -q "std3net16tcp_connect_host" $(BUILD_DIR)/prelude/std-net-dns-lookup.ll
	$(TARGET) tests/cases/standard-library/ok/net/std-net-dns-lookup.ari -o $(BUILD_DIR)/prelude/std-net-dns-lookup.elf
	$(BUILD_DIR)/prelude/std-net-dns-lookup.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/target/std-target-basic.ari --target x86_64-pc-linux-gnu --emit-llvm $(BUILD_DIR)/prelude/std-target-basic.ll
	grep -q 'target triple = "x86_64-pc-linux-gnu"' $(BUILD_DIR)/prelude/std-target-basic.ll
	grep -q "@ari_builtin_target_triple" $(BUILD_DIR)/prelude/std-target-basic.ll
	grep -q "@ari_builtin_target_arch" $(BUILD_DIR)/prelude/std-target-basic.ll
	grep -q "_ARNv3std6target11syscall_abi" $(BUILD_DIR)/prelude/std-target-basic.ll
	$(TARGET) tests/cases/standard-library/ok/target/std-target-basic.ari --target x86_64-pc-linux-gnu -o $(BUILD_DIR)/prelude/std-target-basic.elf
	$(BUILD_DIR)/prelude/std-target-basic.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/standard-library/ok/target/std-target-linux64.ari --target x86_64-pc-linux-gnu --emit-llvm $(BUILD_DIR)/prelude/std-target-linux64-x86_64.ll
	grep -q 'target triple = "x86_64-pc-linux-gnu"' $(BUILD_DIR)/prelude/std-target-linux64-x86_64.ll
	$(TARGET) tests/cases/standard-library/ok/target/std-target-linux64.ari --target aarch64-unknown-linux-gnu --emit-llvm $(BUILD_DIR)/prelude/std-target-linux64-aarch64.ll
	grep -q 'target triple = "aarch64-unknown-linux-gnu"' $(BUILD_DIR)/prelude/std-target-linux64-aarch64.ll
	$(TARGET) tests/cases/standard-library/ok/target/std-target-linux64.ari --target riscv64-unknown-linux-gnu --emit-llvm $(BUILD_DIR)/prelude/std-target-linux64-riscv64.ll
	grep -q 'target triple = "riscv64-unknown-linux-gnu"' $(BUILD_DIR)/prelude/std-target-linux64-riscv64.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-basic.ll
	grep -q "@ari_builtin_process_id" $(BUILD_DIR)/prelude/std-process-basic.ll
	grep -q "@getpid" $(BUILD_DIR)/prelude/std-process-basic.ll
	grep -F -q "std7process10is_success" $(BUILD_DIR)/prelude/std-process-basic.ll
	grep -F -q "std7process10is_failure" $(BUILD_DIR)/prelude/std-process-basic.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-basic.ari -o $(BUILD_DIR)/prelude/std-process-basic.elf
	$(BUILD_DIR)/prelude/std-process-basic.elf; code=$$?; test $$code -eq 53
	$(TARGET) tests/cases/standard-library/ok/process/std-process-identity.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-identity.ll
	grep -q "@ari_builtin_process_uid" $(BUILD_DIR)/prelude/std-process-identity.ll
	grep -q "@ari_builtin_process_gid" $(BUILD_DIR)/prelude/std-process-identity.ll
	grep -q "@getuid" $(BUILD_DIR)/prelude/std-process-identity.ll
	grep -q "@getgid" $(BUILD_DIR)/prelude/std-process-identity.ll
	grep -F -q "std7process7is_root" $(BUILD_DIR)/prelude/std-process-identity.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-identity.ari -o $(BUILD_DIR)/prelude/std-process-identity.elf
	$(BUILD_DIR)/prelude/std-process-identity.elf; code=$$?; test $$code -eq 26
	$(TARGET) tests/cases/standard-library/ok/process/std-process-exit.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-exit.ll
	grep -q "@ari_builtin_process_exit" $(BUILD_DIR)/prelude/std-process-exit.ll
	grep -q "call void @exit(i32 %narrow)" $(BUILD_DIR)/prelude/std-process-exit.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-exit.ari -o $(BUILD_DIR)/prelude/std-process-exit.elf
	$(BUILD_DIR)/prelude/std-process-exit.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/process/std-process-abort.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-abort.ll
	grep -q "@ari_builtin_process_abort" $(BUILD_DIR)/prelude/std-process-abort.ll
	grep -q "call void @abort()" $(BUILD_DIR)/prelude/std-process-abort.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-abort.ari -o $(BUILD_DIR)/prelude/std-process-abort.elf
	$(BUILD_DIR)/prelude/std-process-abort.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/process/std-process-fork-wait.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -q "@ari_builtin_process_fork" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -q "@ari_builtin_process_wait" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -q "declare i32 @fork" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -q "declare i32 @waitpid" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -F -q "std7process8is_child" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -F -q "std7process9is_parent" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -F -q "std7process13is_fork_error" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	grep -F -q "std7process13is_wait_error" $(BUILD_DIR)/prelude/std-process-fork-wait.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-fork-wait.ari -o $(BUILD_DIR)/prelude/std-process-fork-wait.elf
	$(BUILD_DIR)/prelude/std-process-fork-wait.elf; code=$$?; test $$code -eq 112
	$(TARGET) tests/cases/standard-library/ok/process/std-process-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-result.ll
	grep -F -q "std7process4fork" $(BUILD_DIR)/prelude/std-process-result.ll
	grep -F -q "std7process4wait" $(BUILD_DIR)/prelude/std-process-result.ll
	grep -q "declare i32 @fork" $(BUILD_DIR)/prelude/std-process-result.ll
	grep -q "declare i32 @waitpid" $(BUILD_DIR)/prelude/std-process-result.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-result.ari -o $(BUILD_DIR)/prelude/std-process-result.elf
	$(BUILD_DIR)/prelude/std-process-result.elf; code=$$?; test $$code -eq 142
	$(TARGET) tests/cases/standard-library/ok/process/std-process-exit-status.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std7process11wait_status" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__Command8inherent11exit_status" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__Child8inherent11wait_status" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__ExitStatus8inherent6signal" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__ExitStatus8inherent7stopped" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__ExitStatus8inherent9continued" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__ExitStatus8inherent11stop_signal" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__ExitStatus8inherent11core_dumped" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	grep -F -q "std__process__ExitStatus8inherent7success" $(BUILD_DIR)/prelude/std-process-exit-status.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-exit-status.ari -o $(BUILD_DIR)/prelude/std-process-exit-status.elf
	$(BUILD_DIR)/prelude/std-process-exit-status.elf; code=$$?; test $$code -eq 83
	$(TARGET) tests/cases/standard-library/ok/process/std-process-command.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process17command_with_args" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process3arg" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process7env_var" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process4kill" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process5spawn" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process6status" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process11status_code" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std7process11exit_status" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std__process__Command8inherent11status_code" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q "std__process__Command8inherent3env" $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q '@"execvp"' $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q '@"kill"' $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q '@"setenv"' $(BUILD_DIR)/prelude/std-process-command.ll
	grep -F -q '@"chdir"' $(BUILD_DIR)/prelude/std-process-command.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-command.ari -o $(BUILD_DIR)/prelude/std-process-command.elf
	$(BUILD_DIR)/prelude/std-process-command.elf; code=$$?; test $$code -eq 237
	$(TARGET) tests/cases/standard-library/ok/process/std-process-output.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std7process9output_in" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std__process__Command8inherent9output_in" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std__process__Output8inherent6stdout" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std__process__Output8inherent6stderr" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std__process__Output8inherent13stdout_string" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std__process__Output8inherent13stderr_string" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q "std7process26redirect_descriptor_result" $(BUILD_DIR)/prelude/std-process-output.ll
	grep -F -q '@"dup2"' $(BUILD_DIR)/prelude/std-process-output.ll
	grep -q "@ari_builtin_os_poll_read" $(BUILD_DIR)/prelude/std-process-output.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-output.ari -o $(BUILD_DIR)/prelude/std-process-output.elf
	$(BUILD_DIR)/prelude/std-process-output.elf; code=$$?; test $$code -eq 221
	$(TARGET) tests/cases/standard-library/ok/process/std-process-high-level.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent3arg" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent9arg_bytes" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent3env" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent9env_bytes" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent9clear_env" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent11inherit_env" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent16current_dir_path" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent8with_arg" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent8with_env" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent14with_clear_env" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent16with_inherit_env" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std__process__Command8inherent16with_current_dir" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std7process9arg_bytes" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std7process13env_var_bytes" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std7process9temp_file" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std7process8temp_dir" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q "std7process11kill_signal" $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q '@"mkstemp"' $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q '@"mkdtemp"' $(BUILD_DIR)/prelude/std-process-high-level.ll
	grep -F -q '@"clearenv"' $(BUILD_DIR)/prelude/std-process-high-level.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-high-level.ari -o $(BUILD_DIR)/prelude/std-process-high-level.elf
	$(BUILD_DIR)/prelude/std-process-high-level.elf; code=$$?; test $$code -eq 183
	$(TARGET) tests/cases/standard-library/ok/process/std-process-stdin.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q "std__process__Command8inherent22status_with_stdin_file" $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q "std__process__Command8inherent28status_with_stdin_file_bytes" $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q "std__process__Command8inherent27status_with_stdin_file_path" $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q "std__process__Command8inherent22status_with_stdin_null" $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q "std__process__Command8inherent17status_with_stdin" $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q "std7process17status_with_stdin" $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q '@"open"' $(BUILD_DIR)/prelude/std-process-stdin.ll
	grep -F -q '@"dup2"' $(BUILD_DIR)/prelude/std-process-stdin.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-stdin.ari -o $(BUILD_DIR)/prelude/std-process-stdin.elf
	$(BUILD_DIR)/prelude/std-process-stdin.elf; code=$$?; test $$code -eq 193
	$(TARGET) tests/cases/standard-library/ok/process/std-process-piped.ari --emit-llvm $(BUILD_DIR)/prelude/std-process-piped.ll
	grep -F -q "spawn_piped" $(BUILD_DIR)/prelude/std-process-piped.ll
	grep -F -q "ChildPipes" $(BUILD_DIR)/prelude/std-process-piped.ll
	grep -F -q '@"dup2"' $(BUILD_DIR)/prelude/std-process-piped.ll
	$(TARGET) tests/cases/standard-library/ok/process/std-process-piped.ari -o $(BUILD_DIR)/prelude/std-process-piped.elf
	$(BUILD_DIR)/prelude/std-process-piped.elf; code=$$?; test $$code -eq 180
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-basic.elf; $(BUILD_DIR)/prelude/std-process-basic.elf; code=$$?; test $$code -eq 53; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-identity.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-identity.elf; $(BUILD_DIR)/prelude/std-process-identity.elf; code=$$?; test $$code -eq 26; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-exit.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-exit.elf; $(BUILD_DIR)/prelude/std-process-exit.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-abort.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-abort.elf; $(BUILD_DIR)/prelude/std-process-abort.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-fork-wait.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-fork-wait.elf; $(BUILD_DIR)/prelude/std-process-fork-wait.elf; code=$$?; test $$code -eq 112; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-result.elf; $(BUILD_DIR)/prelude/std-process-result.elf; code=$$?; test $$code -eq 142; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-exit-status.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-exit-status.elf; $(BUILD_DIR)/prelude/std-process-exit-status.elf; code=$$?; test $$code -eq 83; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-command.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-command.elf; $(BUILD_DIR)/prelude/std-process-command.elf; code=$$?; test $$code -eq 237; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-output.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-output.elf; $(BUILD_DIR)/prelude/std-process-output.elf; code=$$?; test $$code -eq 221; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-high-level.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-high-level.elf; $(BUILD_DIR)/prelude/std-process-high-level.elf; code=$$?; test $$code -eq 183; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-stdin.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-stdin.elf; $(BUILD_DIR)/prelude/std-process-stdin.elf; code=$$?; test $$code -eq 193; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/process/std-process-piped.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-process-piped.elf; $(BUILD_DIR)/prelude/std-process-piped.elf; code=$$?; test $$code -eq 180; fi
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -q "@ari_builtin_thread_spawn" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -q "@ari_builtin_thread_join" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -q "@ari_builtin_thread_yield" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -q "declare i32 @pthread_create" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -q "declare i32 @pthread_join" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -q "declare i32 @sched_yield" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -F -q "std6thread2id" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -F -q "std6thread7is_main" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -F -q "std6thread13is_join_error" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -F -q "std6thread10join_value" $(BUILD_DIR)/prelude/std-thread-basic.ll
	grep -F -q "std__thread__ThreadResult" $(BUILD_DIR)/prelude/std-thread-basic.ll
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-basic.ari -o $(BUILD_DIR)/prelude/std-thread-basic.elf
	$(BUILD_DIR)/prelude/std-thread-basic.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-runtime-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-thread-runtime-helpers.ll
	grep -q "@ari_builtin_thread_available_parallelism" $(BUILD_DIR)/prelude/std-thread-runtime-helpers.ll
	grep -q "@ari_builtin_time_sleep_nanos" $(BUILD_DIR)/prelude/std-thread-runtime-helpers.ll
	grep -q "declare i64 @sysconf" $(BUILD_DIR)/prelude/std-thread-runtime-helpers.ll
	grep -q "declare i32 @nanosleep" $(BUILD_DIR)/prelude/std-thread-runtime-helpers.ll
	grep -F -q "std6thread5sleep" $(BUILD_DIR)/prelude/std-thread-runtime-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-runtime-helpers.ari -o $(BUILD_DIR)/prelude/std-thread-runtime-helpers.elf
	$(BUILD_DIR)/prelude/std-thread-runtime-helpers.elf; code=$$?; test $$code -eq 24
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-builder.ari --emit-llvm $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -q "@ari_builtin_thread_spawn_configured" $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -q "@ari_builtin_thread_is_finished" $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -q "declare i32 @pthread_attr_init" $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -q "declare i32 @pthread_attr_setstacksize" $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -q "declare i32 @pthread_setname_np" $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -q "declare i32 @pthread_kill" $(BUILD_DIR)/prelude/std-thread-builder.ll
	grep -F -q "std__thread__Builder" $(BUILD_DIR)/prelude/std-thread-builder.ll
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-builder.ari -o $(BUILD_DIR)/prelude/std-thread-builder.elf
	$(BUILD_DIR)/prelude/std-thread-builder.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-local.ari --emit-llvm $(BUILD_DIR)/prelude/std-thread-local.ll
	grep -F -q "std__thread__ThreadLocal" $(BUILD_DIR)/prelude/std-thread-local.ll
	grep -F -q "_ARNv3std6thread33thread_local_with_capacity" $(BUILD_DIR)/prelude/std-thread-local.ll
	grep -F -q "ThreadLocal" $(BUILD_DIR)/prelude/std-thread-local.ll
	grep -F -q "get_or_try_init" $(BUILD_DIR)/prelude/std-thread-local.ll
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-local.ari -o $(BUILD_DIR)/prelude/std-thread-local.elf
	$(BUILD_DIR)/prelude/std-thread-local.elf; code=$$?; test $$code -eq 73
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-scope-raw.ari --emit-llvm $(BUILD_DIR)/prelude/std-thread-scope-raw.ll
	grep -q "@ari_builtin_thread_spawn_raw" $(BUILD_DIR)/prelude/std-thread-scope-raw.ll
	grep -q "@ari_thread_trampoline_raw" $(BUILD_DIR)/prelude/std-thread-scope-raw.ll
	grep -F -q "std__thread__ThreadScope" $(BUILD_DIR)/prelude/std-thread-scope-raw.ll
	grep -F -q "try_join" $(BUILD_DIR)/prelude/std-thread-scope-raw.ll
	$(TARGET) tests/cases/standard-library/ok/thread/std-thread-scope-raw.ari -o $(BUILD_DIR)/prelude/std-thread-scope-raw.elf
	$(BUILD_DIR)/prelude/std-thread-scope-raw.elf; code=$$?; test $$code -eq 42
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/thread/std-thread-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-thread-basic.elf; $(BUILD_DIR)/prelude/std-thread-basic.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari --emit-llvm $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "@ari_builtin_sync_atomic_i64_load" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "@ari_builtin_sync_atomic_i64_store" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "@ari_builtin_sync_atomic_i64_swap" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "@ari_builtin_sync_atomic_i64_fetch_add" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "@ari_builtin_sync_atomic_i64_compare_exchange" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "load atomic i64" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "store atomic i64" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "atomicrmw xchg" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "atomicrmw add" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	grep -q "cmpxchg" $(BUILD_DIR)/prelude/std-sync-atomic-i64.ll
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari -o $(BUILD_DIR)/prelude/std-sync-atomic-i64.elf
	$(BUILD_DIR)/prelude/std-sync-atomic-i64.elf; code=$$?; test $$code -eq 42
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/sync/std-sync-atomic-i64.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-sync-atomic-i64.elf; $(BUILD_DIR)/prelude/std-sync-atomic-i64.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-mutex-once.ari --emit-llvm $(BUILD_DIR)/prelude/std-sync-mutex-once.ll
	grep -q '_ARNv3std4sync8try_lock' $(BUILD_DIR)/prelude/std-sync-mutex-once.ll
	grep -q '_ARNv3std4sync9call_once' $(BUILD_DIR)/prelude/std-sync-mutex-once.ll
	grep -q "@ari_builtin_sync_atomic_i64_compare_exchange" $(BUILD_DIR)/prelude/std-sync-mutex-once.ll
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-mutex-once.ari -o $(BUILD_DIR)/prelude/std-sync-mutex-once.elf
	$(BUILD_DIR)/prelude/std-sync-mutex-once.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-rwlock.ari --emit-llvm $(BUILD_DIR)/prelude/std-sync-rwlock.ll
	grep -q '_ARNv3std4sync13try_read_lock' $(BUILD_DIR)/prelude/std-sync-rwlock.ll
	grep -q '_ARNv3std4sync14try_write_lock' $(BUILD_DIR)/prelude/std-sync-rwlock.ll
	grep -q '_ARNv3std4sync12reader_count' $(BUILD_DIR)/prelude/std-sync-rwlock.ll
	grep -q "@ari_builtin_sync_atomic_i64_compare_exchange" $(BUILD_DIR)/prelude/std-sync-rwlock.ll
	grep -q "@ari_builtin_sync_atomic_i64_fetch_add" $(BUILD_DIR)/prelude/std-sync-rwlock.ll
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-rwlock.ari -o $(BUILD_DIR)/prelude/std-sync-rwlock.elf
	$(BUILD_DIR)/prelude/std-sync-rwlock.elf; code=$$?; test $$code -eq 43
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-value-locks.ari --emit-llvm $(BUILD_DIR)/prelude/std-sync-value-locks.ll
	grep -F -q "std__sync__Mutex" $(BUILD_DIR)/prelude/std-sync-value-locks.ll
	grep -F -q "std__sync__RwLock" $(BUILD_DIR)/prelude/std-sync-value-locks.ll
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-value-locks.ari -o $(BUILD_DIR)/prelude/std-sync-value-locks.elf
	$(BUILD_DIR)/prelude/std-sync-value-locks.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-semaphore.ari --emit-llvm $(BUILD_DIR)/prelude/std-sync-semaphore.ll
	grep -F -q "std__sync__Semaphore" $(BUILD_DIR)/prelude/std-sync-semaphore.ll
	grep -F -q "std__sync__SemaphorePermit" $(BUILD_DIR)/prelude/std-sync-semaphore.ll
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-semaphore.ari -o $(BUILD_DIR)/prelude/std-sync-semaphore.elf
	$(BUILD_DIR)/prelude/std-sync-semaphore.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-concurrency-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__AtomicBool" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__AtomicUsize" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__AtomicPtr" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__OnceLock" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__Condvar" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__Barrier" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__Sender" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "std__sync__Receiver" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -F -q "_ARNv3std4sync14channel" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "@ari_builtin_sync_atomic_i64_load_order" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "@ari_builtin_sync_atomic_i64_store_order" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "@ari_builtin_sync_atomic_i64_compare_exchange_order" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "load atomic i64.*acquire" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "store atomic i64.*release" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "atomicrmw add.*acq_rel" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	grep -q "cmpxchg.*seq_cst acquire" $(BUILD_DIR)/prelude/std-sync-concurrency-api.ll
	$(TARGET) tests/cases/standard-library/ok/sync/std-sync-concurrency-api.ari -o $(BUILD_DIR)/prelude/std-sync-concurrency-api.elf
	$(BUILD_DIR)/prelude/std-sync-concurrency-api.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/time/std-time-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -q "@ari_builtin_time_monotonic_nanos" $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -q "@ari_builtin_time_unix_nanos" $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -q "@ari_builtin_time_sleep_nanos" $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -q "@clock_gettime" $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -q "@nanosleep" $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -F -q "std4time3now" $(BUILD_DIR)/prelude/std-time-basic.ll
	grep -F -q "std4time7elapsed" $(BUILD_DIR)/prelude/std-time-basic.ll
	$(TARGET) tests/cases/standard-library/ok/time/std-time-basic.ari -o $(BUILD_DIR)/prelude/std-time-basic.elf
	$(BUILD_DIR)/prelude/std-time-basic.elf; code=$$?; test $$code -eq 105
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/time/std-time-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-time-basic.elf; $(BUILD_DIR)/prelude/std-time-basic.elf; code=$$?; test $$code -eq 105; fi
	$(TARGET) tests/cases/standard-library/ok/time/std-time-try-duration.ari --emit-llvm $(BUILD_DIR)/prelude/std-time-try-duration.ll
	grep -F -q "std4time15try_nanoseconds" $(BUILD_DIR)/prelude/std-time-try-duration.ll
	grep -F -q "std4time16try_milliseconds" $(BUILD_DIR)/prelude/std-time-try-duration.ll
	grep -F -q "std__time__Duration8inherent11try_seconds" $(BUILD_DIR)/prelude/std-time-try-duration.ll
	$(TARGET) tests/cases/standard-library/ok/time/std-time-try-duration.ari -o $(BUILD_DIR)/prelude/std-time-try-duration.elf
	$(BUILD_DIR)/prelude/std-time-try-duration.elf; code=$$?; test $$code -eq 55
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/time/std-time-try-duration.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-time-try-duration.elf; $(BUILD_DIR)/prelude/std-time-try-duration.elf; code=$$?; test $$code -eq 55; fi
	$(TARGET) tests/cases/standard-library/ok/time/std-time-try-unix.ari --emit-llvm $(BUILD_DIR)/prelude/std-time-try-unix.ll
	grep -F -q "std4time20try_system_from_unix" $(BUILD_DIR)/prelude/std-time-try-unix.ll
	grep -F -q "std4time17try_utc_from_unix" $(BUILD_DIR)/prelude/std-time-try-unix.ll
	grep -F -q "std__time__SystemTime8inherent13try_from_unix" $(BUILD_DIR)/prelude/std-time-try-unix.ll
	grep -F -q "std__time__UtcDateTime8inherent13try_from_unix" $(BUILD_DIR)/prelude/std-time-try-unix.ll
	grep -F -q "std__time__UtcDateTime8inherent9from_unix" $(BUILD_DIR)/prelude/std-time-try-unix.ll
	$(TARGET) tests/cases/standard-library/ok/time/std-time-try-unix.ari -o $(BUILD_DIR)/prelude/std-time-try-unix.elf
	$(BUILD_DIR)/prelude/std-time-try-unix.elf; code=$$?; test $$code -eq 66
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/time/std-time-try-unix.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-time-try-unix.elf; $(BUILD_DIR)/prelude/std-time-try-unix.elf; code=$$?; test $$code -eq 66; fi
	$(TARGET) tests/cases/standard-library/ok/time/std-time-calendar-validation.ari --emit-llvm $(BUILD_DIR)/prelude/std-time-calendar-validation.ll
	grep -F -q "std4time17try_days_in_month" $(BUILD_DIR)/prelude/std-time-calendar-validation.ll
	grep -F -q "std4time13days_in_month" $(BUILD_DIR)/prelude/std-time-calendar-validation.ll
	$(TARGET) tests/cases/standard-library/ok/time/std-time-calendar-validation.ari -o $(BUILD_DIR)/prelude/std-time-calendar-validation.elf
	$(BUILD_DIR)/prelude/std-time-calendar-validation.elf; code=$$?; test $$code -eq 21
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/time/std-time-calendar-validation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-time-calendar-validation.elf; $(BUILD_DIR)/prelude/std-time-calendar-validation.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/cases/standard-library/ok/time/std-time-timeout.ari --emit-llvm $(BUILD_DIR)/prelude/std-time-timeout.ll
	grep -F -q "std4time7timeout" $(BUILD_DIR)/prelude/std-time-timeout.ll
	grep -F -q "std4time13timeout_after" $(BUILD_DIR)/prelude/std-time-timeout.ll
	grep -F -q "std4time11deadline_at" $(BUILD_DIR)/prelude/std-time-timeout.ll
	grep -F -q "std__time__Instant8inherent25saturating_duration_since" $(BUILD_DIR)/prelude/std-time-timeout.ll
	grep -F -q "std__time__Deadline8inherent11has_expired" $(BUILD_DIR)/prelude/std-time-timeout.ll
	$(TARGET) tests/cases/standard-library/ok/time/std-time-timeout.ari -o $(BUILD_DIR)/prelude/std-time-timeout.elf
	$(BUILD_DIR)/prelude/std-time-timeout.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/time/std-time-timeout.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-time-timeout.elf; $(BUILD_DIR)/prelude/std-time-timeout.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/time/std-time-utc-calendar.ari --emit-llvm $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	grep -F -q "std4time13utc_from_unix" $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	grep -F -q "std4time16system_from_unix" $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	grep -F -q "std4time13days_in_month" $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	grep -F -q "std4time12is_leap_year" $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	grep -F -q "std__time__SystemTime8inherent6to_utc" $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	grep -F -q "std__time__UtcDateTime8inherent4year" $(BUILD_DIR)/prelude/std-time-utc-calendar.ll
	$(TARGET) tests/cases/standard-library/ok/time/std-time-utc-calendar.ari -o $(BUILD_DIR)/prelude/std-time-utc-calendar.elf
	$(BUILD_DIR)/prelude/std-time-utc-calendar.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/time/std-time-utc-calendar.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-time-utc-calendar.elf; $(BUILD_DIR)/prelude/std-time-utc-calendar.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-std-qualified-print.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	grep -q "call i64 @ari_builtin_write_i64(i64 1)" $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	grep -q "call i64 @ari_builtin_write_i64(i64 2)" $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	grep -q "call i64 @ari_builtin_write_i64(i64 3)" $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-std-qualified-print.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-std-qualified-print.elf; $(BUILD_DIR)/prelude/prelude-std-qualified-print.elf >$(BUILD_DIR)/prelude/prelude-std-qualified-print.out; code=$$?; test $$code -eq 6; grep -qx "std root 1" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; grep -qx "std print 2" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; grep -qx "std io plain" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; grep -qx "std alias 3" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; fi
	$(TARGET) tests/cases/standard-library/ok/input/prelude-read-line.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-read-line.ll
	grep -q "@ari_builtin_read_line" $(BUILD_DIR)/prelude/prelude-read-line.ll
	grep -q "declare ptr @fgets" $(BUILD_DIR)/prelude/prelude-read-line.ll
	grep -q "@ari_line_buffer" $(BUILD_DIR)/prelude/prelude-read-line.ll
	$(TARGET) tests/cases/standard-library/ok/input/prelude-read-line-owned.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-read-line-owned.ll
	grep -q 'ari_builtin_read_line_owned' $(BUILD_DIR)/prelude/prelude-read-line-owned.ll
	grep -q 'ari_builtin_string_from_string' $(BUILD_DIR)/prelude/prelude-read-line-owned.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then \
		$(TARGET) tests/cases/standard-library/ok/input/prelude-read-line.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-read-line.elf; \
		printf 'alpha\nbeta\ngamma\ndelta\nepsilon\nzeta\neta\ntheta\n' | $(BUILD_DIR)/prelude/prelude-read-line.elf >$(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "alpha" $(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "beta" $(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "gamma" $(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "delta" $(BUILD_DIR)/prelude/prelude-read-line.out; \
	fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/input/prelude-read-line-owned.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-read-line-owned.elf; printf 'alpha\nbeta\ngamma\ndelta\n' | $(BUILD_DIR)/prelude/prelude-read-line-owned.elf; code=$$?; test $$code -eq 161; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-trait-surface.ari -o $(BUILD_DIR)/prelude/prelude-trait-surface.elf
	$(BUILD_DIR)/prelude/prelude-trait-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/convert/std-convert-value-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-convert-value-helpers.ll
	grep -F -q 'std7convert15identity__G_i64' $(BUILD_DIR)/prelude/std-convert-value-helpers.ll
	grep -F -q 'std7convert15from__G_i32_i64' $(BUILD_DIR)/prelude/std-convert-value-helpers.ll
	grep -F -q 'std7convert15into__G_i64_i32' $(BUILD_DIR)/prelude/std-convert-value-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/convert/std-convert-value-helpers.ari -o $(BUILD_DIR)/prelude/std-convert-value-helpers.elf
	$(BUILD_DIR)/prelude/std-convert-value-helpers.elf; code=$$?; test $$code -eq 48
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/convert/std-convert-value-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-convert-value-helpers.elf; $(BUILD_DIR)/prelude/std-convert-value-helpers.elf; code=$$?; test $$code -eq 48; fi
	$(TARGET) tests/cases/standard-library/ok/convert/std-convert-try-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-convert-try-helpers.ll
	grep -F -q 'std7convert21try_from__G_i64_Small' $(BUILD_DIR)/prelude/std-convert-try-helpers.ll
	grep -F -q 'std7convert21try_into__G_Small_i64' $(BUILD_DIR)/prelude/std-convert-try-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/convert/std-convert-try-helpers.ari -o $(BUILD_DIR)/prelude/std-convert-try-helpers.elf
	$(BUILD_DIR)/prelude/std-convert-try-helpers.elf; code=$$?; test $$code -eq 93
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/convert/std-convert-try-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-convert-try-helpers.elf; $(BUILD_DIR)/prelude/std-convert-try-helpers.elf; code=$$?; test $$code -eq 93; fi
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-primitive-impls.ari --emit-llvm $(BUILD_DIR)/prelude/std-cmp-primitive-impls.ll
	grep -F -q 'compare__G_i64' $(BUILD_DIR)/prelude/std-cmp-primitive-impls.ll
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-primitive-impls.ari -o $(BUILD_DIR)/prelude/std-cmp-primitive-impls.elf
	$(BUILD_DIR)/prelude/std-cmp-primitive-impls.elf; code=$$?; test $$code -eq 202
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-by-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-cmp-by-helpers.ll
	grep -F -q 'compare_by__G_Score' $(BUILD_DIR)/prelude/std-cmp-by-helpers.ll
	grep -F -q 'is_between_by__G_Score' $(BUILD_DIR)/prelude/std-cmp-by-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-by-helpers.ari -o $(BUILD_DIR)/prelude/std-cmp-by-helpers.elf
	$(BUILD_DIR)/prelude/std-cmp-by-helpers.elf; code=$$?; test $$code -eq 93
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-value-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-cmp-value-helpers.ll
	grep -F -q 'std3cmp10min__G_i64' $(BUILD_DIR)/prelude/std-cmp-value-helpers.ll
	grep -F -q 'std3cmp10max__G_i64' $(BUILD_DIR)/prelude/std-cmp-value-helpers.ll
	grep -F -q 'std3cmp12clamp__G_i64' $(BUILD_DIR)/prelude/std-cmp-value-helpers.ll
	grep -F -q 'std3cmp17is_between__G_i64' $(BUILD_DIR)/prelude/std-cmp-value-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-value-helpers.ari -o $(BUILD_DIR)/prelude/std-cmp-value-helpers.elf
	$(BUILD_DIR)/prelude/std-cmp-value-helpers.elf; code=$$?; test $$code -eq 235
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-value-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-cmp-value-helpers.elf; $(BUILD_DIR)/prelude/std-cmp-value-helpers.elf; code=$$?; test $$code -eq 235; fi
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-equality-operator.ari --emit-llvm $(BUILD_DIR)/prelude/std-cmp-equality-operator.ll
	grep -F -q 'std__cmp__Eq' $(BUILD_DIR)/prelude/std-cmp-equality-operator.ll
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-equality-operator.ari -o $(BUILD_DIR)/prelude/std-cmp-equality-operator.elf
	$(BUILD_DIR)/prelude/std-cmp-equality-operator.elf; code=$$?; test $$code -eq 60
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-order-operators.ari --emit-llvm $(BUILD_DIR)/prelude/std-cmp-order-operators.ll
	grep -F -q 'std__cmp__Ord' $(BUILD_DIR)/prelude/std-cmp-order-operators.ll
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-order-operators.ari -o $(BUILD_DIR)/prelude/std-cmp-order-operators.elf
	$(BUILD_DIR)/prelude/std-cmp-order-operators.elf; code=$$?; test $$code -eq 158
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-ordering.ari --emit-llvm $(BUILD_DIR)/prelude/std-cmp-ordering.ll
	grep -F -q 'compare__G_i64' $(BUILD_DIR)/prelude/std-cmp-ordering.ll
	grep -F -q 'then_compare__G_i64' $(BUILD_DIR)/prelude/std-cmp-ordering.ll
	$(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-ordering.ari -o $(BUILD_DIR)/prelude/std-cmp-ordering.elf
	$(BUILD_DIR)/prelude/std-cmp-ordering.elf; code=$$?; test $$code -eq 233
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/cmp/std-cmp-ordering.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-cmp-ordering.elf; $(BUILD_DIR)/prelude/std-cmp-ordering.elf; code=$$?; test $$code -eq 233; fi
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-slice-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo11sort__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo18stable_sort__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo20binary_search__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo18lower_bound__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo18upper_bound__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'equal_range__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'partition_point__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo16partition__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	grep -F -q 'std4algo12dedup__G_i64' $(BUILD_DIR)/prelude/std-algo-slice-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-slice-helpers.ari -o $(BUILD_DIR)/prelude/std-algo-slice-helpers.elf
	$(BUILD_DIR)/prelude/std-algo-slice-helpers.elf; code=$$?; test $$code -eq 12
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/algo/std-algo-slice-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-algo-slice-helpers.elf; $(BUILD_DIR)/prelude/std-algo-slice-helpers.elf; code=$$?; test $$code -eq 12; fi
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-final-sort.ari --emit-llvm $(BUILD_DIR)/prelude/std-algo-final-sort.ll
	grep -F -q 'introsort_ord_range__G_i64' $(BUILD_DIR)/prelude/std-algo-final-sort.ll
	grep -F -q 'heap_sort_ord_range__G_i64' $(BUILD_DIR)/prelude/std-algo-final-sort.ll
	grep -F -q 'stable_merge_sort_ord_range__G_i64' $(BUILD_DIR)/prelude/std-algo-final-sort.ll
	grep -F -q 'stable_sort_by_in__G_Pair' $(BUILD_DIR)/prelude/std-algo-final-sort.ll
	grep -F -q 'try_stable_sort_by__G_Pair' $(BUILD_DIR)/prelude/std-algo-final-sort.ll
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-final-sort.ari -o $(BUILD_DIR)/prelude/std-algo-final-sort.elf
	$(BUILD_DIR)/prelude/std-algo-final-sort.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-dedup-partition.ari --emit-llvm $(BUILD_DIR)/prelude/std-algo-dedup-partition.ll
	grep -F -q 'std4algo15dedup_by__G_i64' $(BUILD_DIR)/prelude/std-algo-dedup-partition.ll
	grep -F -q 'dedup_by_key__G_User_i64' $(BUILD_DIR)/prelude/std-algo-dedup-partition.ll
	grep -F -q 'std4algo23stable_partition__G_i64' $(BUILD_DIR)/prelude/std-algo-dedup-partition.ll
	grep -F -q 'Slice_i64_8inherent19dedup_by_key__G_i64' $(BUILD_DIR)/prelude/std-algo-dedup-partition.ll
	grep -F -q 'std__vec__Vec_i64_8inherent19dedup_by_key__G_i64' $(BUILD_DIR)/prelude/std-algo-dedup-partition.ll
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-dedup-partition.ari -o $(BUILD_DIR)/prelude/std-algo-dedup-partition.elf
	$(BUILD_DIR)/prelude/std-algo-dedup-partition.elf; code=$$?; test $$code -eq 26
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-by-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-algo-by-helpers.ll
	grep -F -q 'std4algo21is_sorted_by__G_Score' $(BUILD_DIR)/prelude/std-algo-by-helpers.ll
	grep -F -q 'std4algo25binary_search_by__G_Score' $(BUILD_DIR)/prelude/std-algo-by-helpers.ll
	grep -F -q 'equal_range_by__G_Score' $(BUILD_DIR)/prelude/std-algo-by-helpers.ll
	grep -F -q 'partition_point__G_Score' $(BUILD_DIR)/prelude/std-algo-by-helpers.ll
	grep -F -q 'std4algo17clamp_by__G_Score' $(BUILD_DIR)/prelude/std-algo-by-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/algo/std-algo-by-helpers.ari -o $(BUILD_DIR)/prelude/std-algo-by-helpers.elf
	$(BUILD_DIR)/prelude/std-algo-by-helpers.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/hash/std-hash-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-hash-basic.ll
	grep -F -q 'std4hash3new' $(BUILD_DIR)/prelude/std-hash-basic.ll
	grep -F -q 'std4hash12value__G_i64' $(BUILD_DIR)/prelude/std-hash-basic.ll
	grep -F -q 'std4hash5bytes' $(BUILD_DIR)/prelude/std-hash-basic.ll
	grep -F -q 'std4hash6string' $(BUILD_DIR)/prelude/std-hash-basic.ll
	grep -F -q 'std4hash12write__G_i64' $(BUILD_DIR)/prelude/std-hash-basic.ll
	grep -F -q 'std4hash10write_byte' $(BUILD_DIR)/prelude/std-hash-basic.ll
	$(TARGET) tests/cases/standard-library/ok/hash/std-hash-basic.ari -o $(BUILD_DIR)/prelude/std-hash-basic.elf
	$(BUILD_DIR)/prelude/std-hash-basic.elf; code=$$?; test $$code -eq 63
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/hash/std-hash-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-hash-basic.elf; $(BUILD_DIR)/prelude/std-hash-basic.elf; code=$$?; test $$code -eq 63; fi
	$(TARGET) tests/cases/standard-library/ok/hash/std-hash-integer-widths.ari --emit-llvm $(BUILD_DIR)/prelude/std-hash-integer-widths.ll
	grep -F -q 'std4hash8write_i8' $(BUILD_DIR)/prelude/std-hash-integer-widths.ll
	grep -F -q 'std4hash9write_i16' $(BUILD_DIR)/prelude/std-hash-integer-widths.ll
	grep -F -q 'std4hash9write_u32' $(BUILD_DIR)/prelude/std-hash-integer-widths.ll
	grep -F -q 'std4hash12value__G_i16' $(BUILD_DIR)/prelude/std-hash-integer-widths.ll
	$(TARGET) tests/cases/standard-library/ok/hash/std-hash-integer-widths.ari -o $(BUILD_DIR)/prelude/std-hash-integer-widths.elf
	$(BUILD_DIR)/prelude/std-hash-integer-widths.elf; code=$$?; test $$code -eq 255
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/hash/std-hash-integer-widths.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-hash-integer-widths.elf; $(BUILD_DIR)/prelude/std-hash-integer-widths.elf; code=$$?; test $$code -eq 255; fi
	$(TARGET) tests/cases/standard-library/ok/hash/std-hash-combine-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-hash-combine-helpers.ll
	grep -F -q 'std4hash16pair__G_i64_bool' $(BUILD_DIR)/prelude/std-hash-combine-helpers.ll
	grep -F -q 'std4hash7combine' $(BUILD_DIR)/prelude/std-hash-combine-helpers.ll
	grep -F -q 'std4hash23value__G_std__Slice_u8_' $(BUILD_DIR)/prelude/std-hash-combine-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/hash/std-hash-combine-helpers.ari -o $(BUILD_DIR)/prelude/std-hash-combine-helpers.elf
	$(BUILD_DIR)/prelude/std-hash-combine-helpers.elf; code=$$?; test $$code -eq 15
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/hash/std-hash-combine-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-hash-combine-helpers.elf; $(BUILD_DIR)/prelude/std-hash-combine-helpers.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/cases/standard-library/ok/random/std-random-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q '@ari_builtin_random_entropy' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q '@ari_builtin_random_fill' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q 'std6random4seed' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q 'std6random4next' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q 'std6random7boolean' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q 'std6random14shuffle__G_i64' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q 'std__random__Prng8inherent7boolean' $(BUILD_DIR)/prelude/std-random-basic.ll
	grep -F -q 'std__random__Prng8inherent5float' $(BUILD_DIR)/prelude/std-random-basic.ll
	$(TARGET) tests/cases/standard-library/ok/random/std-random-basic.ari -o $(BUILD_DIR)/prelude/std-random-basic.elf
	$(BUILD_DIR)/prelude/std-random-basic.elf; code=$$?; test $$code -eq 11
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/random/std-random-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-random-basic.elf; $(BUILD_DIR)/prelude/std-random-basic.elf; code=$$?; test $$code -eq 11; fi
	$(TARGET) tests/cases/standard-library/ok/random/std-random-result.ari --emit-llvm $(BUILD_DIR)/prelude/std-random-result.ll
	grep -F -q '@ari_builtin_random_fill_result' $(BUILD_DIR)/prelude/std-random-result.ll
	grep -F -q 'std6random4fill' $(BUILD_DIR)/prelude/std-random-result.ll
	grep -F -q 'std6random7entropy' $(BUILD_DIR)/prelude/std-random-result.ll
	grep -F -q 'std__random__Prng8inherent12seed_from_os' $(BUILD_DIR)/prelude/std-random-result.ll
	$(TARGET) tests/cases/standard-library/ok/random/std-random-result.ari -o $(BUILD_DIR)/prelude/std-random-result.elf
	$(BUILD_DIR)/prelude/std-random-result.elf; code=$$?; test $$code -eq 98
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/random/std-random-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-random-result.elf; $(BUILD_DIR)/prelude/std-random-result.elf; code=$$?; test $$code -eq 98; fi
	$(TARGET) tests/cases/standard-library/ok/random/std-random-try-bounds.ari --emit-llvm $(BUILD_DIR)/prelude/std-random-try-bounds.ll
	grep -F -q 'std6random9try_below' $(BUILD_DIR)/prelude/std-random-try-bounds.ll
	grep -F -q 'std6random9try_range' $(BUILD_DIR)/prelude/std-random-try-bounds.ll
	$(TARGET) tests/cases/standard-library/ok/random/std-random-try-bounds.ari -o $(BUILD_DIR)/prelude/std-random-try-bounds.elf
	$(BUILD_DIR)/prelude/std-random-try-bounds.elf; code=$$?; test $$code -eq 5
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/random/std-random-try-bounds.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-random-try-bounds.elf; $(BUILD_DIR)/prelude/std-random-try-bounds.elf; code=$$?; test $$code -eq 5; fi
	$(TARGET) tests/cases/standard-library/ok/math/std-math-integer-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math3abs' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math9min_value' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math9max_value' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math4sign' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math11is_positive' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math11is_negative' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math7is_zero' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math7is_even' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math6is_odd' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math3pow' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math3gcd' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	grep -q '_ARNv3std4math3lcm' $(BUILD_DIR)/prelude/std-math-integer-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/math/std-math-integer-helpers.ari -o $(BUILD_DIR)/prelude/std-math-integer-helpers.elf
	$(BUILD_DIR)/prelude/std-math-integer-helpers.elf; code=$$?; test $$code -eq 139
	$(TARGET) tests/cases/standard-library/ok/math/std-math-division-rounding.ari --emit-llvm $(BUILD_DIR)/prelude/std-math-division-rounding.ll
	grep -q '_ARNv3std4math9div_floor' $(BUILD_DIR)/prelude/std-math-division-rounding.ll
	grep -q '_ARNv3std4math8div_ceil' $(BUILD_DIR)/prelude/std-math-division-rounding.ll
	grep -q '_ARNv3std4math9mod_floor' $(BUILD_DIR)/prelude/std-math-division-rounding.ll
	$(TARGET) tests/cases/standard-library/ok/math/std-math-division-rounding.ari -o $(BUILD_DIR)/prelude/std-math-division-rounding.elf
	$(BUILD_DIR)/prelude/std-math-division-rounding.elf; code=$$?; test $$code -eq 182
	$(TARGET) tests/cases/standard-library/ok/math/std-math-checked-saturating.ari --emit-llvm $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_add' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_sub' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_mul' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_div' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_rem' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_neg' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_abs' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math11checked_pow' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_add' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_sub' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_mul' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_div' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_neg' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_abs' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	grep -q '_ARNv3std4math14saturating_pow' $(BUILD_DIR)/prelude/std-math-checked-saturating.ll
	$(TARGET) tests/cases/standard-library/ok/math/std-math-checked-saturating.ari -o $(BUILD_DIR)/prelude/std-math-checked-saturating.elf
	$(BUILD_DIR)/prelude/std-math-checked-saturating.elf; code=$$?; test $$code -eq 53
	$(TARGET) tests/cases/standard-library/ok/math/std-math-wrapping-overflowing.ari --emit-llvm $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math12wrapping_add' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math12wrapping_sub' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math12wrapping_mul' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math12wrapping_pow' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math15overflowing_add' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math15overflowing_sub' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math15overflowing_mul' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q '_ARNv3std4math15overflowing_pow' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	grep -q 'ret { i64, i1 }' $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.ll
	$(TARGET) tests/cases/standard-library/ok/math/std-math-wrapping-overflowing.ari -o $(BUILD_DIR)/prelude/std-math-wrapping-overflowing.elf
	$(BUILD_DIR)/prelude/std-math-wrapping-overflowing.elf; code=$$?; test $$code -eq 32
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-byte-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-byte-helpers.ll
	grep -q '_ARNv3std5ascii8is_digit' $(BUILD_DIR)/prelude/std-ascii-byte-helpers.ll
	grep -q '_ARNv3std5ascii8to_lower' $(BUILD_DIR)/prelude/std-ascii-byte-helpers.ll
	grep -q '_ARNv3std5ascii11digit_value' $(BUILD_DIR)/prelude/std-ascii-byte-helpers.ll
	grep -q '_ARNv3std5ascii9hex_value' $(BUILD_DIR)/prelude/std-ascii-byte-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-byte-helpers.ari -o $(BUILD_DIR)/prelude/std-ascii-byte-helpers.elf
	$(BUILD_DIR)/prelude/std-ascii-byte-helpers.elf; code=$$?; test $$code -eq 177
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-class-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-class-helpers.ll
	grep -q '_ARNv3std5ascii10is_control' $(BUILD_DIR)/prelude/std-ascii-class-helpers.ll
	grep -q '_ARNv3std5ascii12is_printable' $(BUILD_DIR)/prelude/std-ascii-class-helpers.ll
	grep -q '_ARNv3std5ascii10is_graphic' $(BUILD_DIR)/prelude/std-ascii-class-helpers.ll
	grep -q '_ARNv3std5ascii14is_punctuation' $(BUILD_DIR)/prelude/std-ascii-class-helpers.ll
	grep -q '_ARNv3std5ascii8is_blank' $(BUILD_DIR)/prelude/std-ascii-class-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-class-helpers.ari -o $(BUILD_DIR)/prelude/std-ascii-class-helpers.elf
	$(BUILD_DIR)/prelude/std-ascii-class-helpers.elf; code=$$?; test $$code -eq 171
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-slice-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-slice-helpers.ll
	grep -q '_ARNv3std5ascii15skip_whitespace' $(BUILD_DIR)/prelude/std-ascii-slice-helpers.ll
	grep -q '_ARNv3std5ascii10trim_start' $(BUILD_DIR)/prelude/std-ascii-slice-helpers.ll
	grep -q '_ARNv3std5ascii8trim_end' $(BUILD_DIR)/prelude/std-ascii-slice-helpers.ll
	grep -q '_ARNv3std5ascii13parse_decimal' $(BUILD_DIR)/prelude/std-ascii-slice-helpers.ll
	grep -q '_ARNv3std5ascii9parse_hex' $(BUILD_DIR)/prelude/std-ascii-slice-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-slice-helpers.ari -o $(BUILD_DIR)/prelude/std-ascii-slice-helpers.elf
	$(BUILD_DIR)/prelude/std-ascii-slice-helpers.elf; code=$$?; test $$code -eq 19
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-prefix-parsers.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-prefix-parsers.ll
	grep -q '_ARNv3std5ascii20parse_decimal_prefix' $(BUILD_DIR)/prelude/std-ascii-prefix-parsers.ll
	grep -q '_ARNv3std5ascii16parse_hex_prefix' $(BUILD_DIR)/prelude/std-ascii-prefix-parsers.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-prefix-parsers.ari -o $(BUILD_DIR)/prelude/std-ascii-prefix-parsers.elf
	$(BUILD_DIR)/prelude/std-ascii-prefix-parsers.elf; code=$$?; test $$code -eq 39
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-signed-parsers.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-signed-parsers.ll
	grep -q '_ARNv3std5ascii20parse_signed_decimal' $(BUILD_DIR)/prelude/std-ascii-signed-parsers.ll
	grep -q '_ARNv3std5ascii27parse_signed_decimal_prefix' $(BUILD_DIR)/prelude/std-ascii-signed-parsers.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-signed-parsers.ari -o $(BUILD_DIR)/prelude/std-ascii-signed-parsers.elf
	$(BUILD_DIR)/prelude/std-ascii-signed-parsers.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-overflow-parsers.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-overflow-parsers.ll
	grep -q '_ARNv3std5ascii13parse_decimal' $(BUILD_DIR)/prelude/std-ascii-overflow-parsers.ll
	grep -q '_ARNv3std5ascii20parse_signed_decimal' $(BUILD_DIR)/prelude/std-ascii-overflow-parsers.ll
	grep -q '_ARNv3std5ascii9parse_hex' $(BUILD_DIR)/prelude/std-ascii-overflow-parsers.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-overflow-parsers.ari -o $(BUILD_DIR)/prelude/std-ascii-overflow-parsers.elf
	$(BUILD_DIR)/prelude/std-ascii-overflow-parsers.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-case-compare.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-case-compare.ll
	grep -q '_ARNv3std5ascii18equals_ignore_case' $(BUILD_DIR)/prelude/std-ascii-case-compare.ll
	grep -q '_ARNv3std5ascii23starts_with_ignore_case' $(BUILD_DIR)/prelude/std-ascii-case-compare.ll
	grep -q '_ARNv3std5ascii21ends_with_ignore_case' $(BUILD_DIR)/prelude/std-ascii-case-compare.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-case-compare.ari -o $(BUILD_DIR)/prelude/std-ascii-case-compare.elf
	$(BUILD_DIR)/prelude/std-ascii-case-compare.elf; code=$$?; test $$code -eq 8
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-case-search.ari --emit-llvm $(BUILD_DIR)/prelude/std-ascii-case-search.ll
	grep -q '_ARNv3std5ascii20index_of_ignore_case' $(BUILD_DIR)/prelude/std-ascii-case-search.ll
	grep -q '_ARNv3std5ascii20contains_ignore_case' $(BUILD_DIR)/prelude/std-ascii-case-search.ll
	$(TARGET) tests/cases/standard-library/ok/ascii/std-ascii-case-search.ari -o $(BUILD_DIR)/prelude/std-ascii-case-search.elf
	$(BUILD_DIR)/prelude/std-ascii-case-search.elf; code=$$?; test $$code -eq 167
	$(TARGET) tests/cases/standard-library/ok/parse/std-parse-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse13integer_radix' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse22integer_radix_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16is_integer_radix' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16integer_radix_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse30integer_radix_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse39integer_radix_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse33is_integer_radix_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse33integer_radix_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse19integer_radix_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse36integer_radix_with_underscores_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse7integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16integer_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse13integer_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse10is_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse10integer_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse24integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse33integer_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse30integer_with_underscores_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse27is_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse27integer_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse8unsigned' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse17unsigned_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse14unsigned_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse11is_unsigned' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse11unsigned_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse25unsigned_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse34unsigned_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse31unsigned_with_underscores_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse28is_unsigned_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse28unsigned_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse14unsigned_radix' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse23unsigned_radix_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse17is_unsigned_radix' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse17unsigned_radix_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse20unsigned_radix_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse31unsigned_radix_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse40unsigned_radix_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse37unsigned_radix_with_underscores_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse34is_unsigned_radix_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse34unsigned_radix_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse11hex_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse20hex_integer_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse14is_hex_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse14hex_integer_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse28hex_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse37hex_integer_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse31is_hex_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse31hex_integer_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse14binary_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse23binary_integer_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse17is_binary_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse17binary_integer_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse31binary_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse40binary_integer_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse34is_binary_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse34binary_integer_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse13octal_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse22octal_integer_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16is_octal_integer' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16octal_integer_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse30octal_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse39octal_integer_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse33is_octal_integer_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse33octal_integer_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse7boolean' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16boolean_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse10is_boolean' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse10boolean_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse8is_float' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse8float_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse14float_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse5float' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse11float_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15float_unchecked' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse25is_float_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse25float_with_underscores_or' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse31float_with_underscores_optional' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse22float_with_underscores' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse28float_with_underscores_error' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse12parse__G_i64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse12parse__G_u64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse12parse__G_f64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse13parse__G_bool' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15parse_or__G_i64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15parse_or__G_u64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15parse_or__G_f64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16parse_or__G_bool' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15is_parse__G_i64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15is_parse__G_u64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse15is_parse__G_f64' $(BUILD_DIR)/prelude/std-parse-basic.ll
	grep -q '_ARNv3std5parse16is_parse__G_bool' $(BUILD_DIR)/prelude/std-parse-basic.ll
	$(TARGET) tests/cases/standard-library/ok/parse/std-parse-basic.ari -o $(BUILD_DIR)/prelude/std-parse-basic.elf
	$(BUILD_DIR)/prelude/std-parse-basic.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-text.ari --emit-llvm $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding8is_ascii' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding10utf8_count' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding19utf8_count_optional' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding7is_utf8' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding10utf8_error' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding13validate_utf8' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding22validate_utf8_optional' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding11decode_utf8' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding14decode_utf8_in' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding23decode_utf8_optional_in' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding24decode_utf8_unchecked_in' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding11utf16_count' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding20utf16_count_optional' $(BUILD_DIR)/prelude/std-encoding-text.ll
	grep -q '_ARNv3std8encoding8is_utf16' $(BUILD_DIR)/prelude/std-encoding-text.ll
	$(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-text.ari -o $(BUILD_DIR)/prelude/std-encoding-text.elf
	$(BUILD_DIR)/prelude/std-encoding-text.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-utf8-codepoints.ari --emit-llvm $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding10utf8_width' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding7utf8_at' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding11encode_utf8' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding14encode_utf8_in' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding23encode_utf8_optional_in' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding18try_encode_utf8_in' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q '_ARNv3std8encoding24encode_utf8_unchecked_in' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	grep -q 'std__encoding__Utf8Char' $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.ll
	$(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-utf8-codepoints.ari -o $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.elf
	$(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.elf; code=$$?; test $$code -eq 13
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-utf8-codepoints.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.elf; $(BUILD_DIR)/prelude/std-encoding-utf8-codepoints.elf; code=$$?; test $$code -eq 13; fi
	$(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-codec.ari --emit-llvm $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding13encode_hex_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding15hex_decoded_len' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding24hex_decoded_len_optional' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding9hex_error' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding10decode_hex' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding13decode_hex_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding22decode_hex_optional_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding17try_decode_hex_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding23decode_hex_unchecked_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding16encode_base64_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding18base64_decoded_len' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding27base64_decoded_len_optional' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding12base64_error' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding13decode_base64' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding16decode_base64_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding25decode_base64_optional_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding20try_decode_base64_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding26decode_base64_unchecked_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding23base64_mime_encoded_len' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding21encode_base64_mime_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding23base64_mime_decoded_len' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding32base64_mime_decoded_len_optional' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding17base64_mime_error' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding22can_decode_base64_mime' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding18decode_base64_mime' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding21decode_base64_mime_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding30decode_base64_mime_optional_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding25try_decode_base64_mime_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding31decode_base64_mime_unchecked_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding20encode_base64_url_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding29encode_base64_url_unpadded_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding22base64_url_decoded_len' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding31base64_url_decoded_len_optional' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding16base64_url_error' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding17decode_base64_url' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding20decode_base64_url_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding29decode_base64_url_optional_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding24try_decode_base64_url_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	grep -q '_ARNv3std8encoding30decode_base64_url_unchecked_in' $(BUILD_DIR)/prelude/std-encoding-codec.ll
	$(TARGET) tests/cases/standard-library/ok/encoding/std-encoding-codec.ari -o $(BUILD_DIR)/prelude/std-encoding-codec.elf
	$(BUILD_DIR)/prelude/std-encoding-codec.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/standard-library/ok/path/std-path-basic.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path12is_separator' $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path11is_absolute' $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path9file_name' $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path6parent' $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path9file_stem' $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path7join_in' $(BUILD_DIR)/prelude/std-path-basic.ll
	grep -q '_ARNv3std4path12normalize_in' $(BUILD_DIR)/prelude/std-path-basic.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-basic.ari -o $(BUILD_DIR)/prelude/std-path-basic.elf
	$(BUILD_DIR)/prelude/std-path-basic.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/standard-library/ok/path/std-path-predicates.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-predicates.ll
	grep -q '_ARNv3std4path13has_file_name' $(BUILD_DIR)/prelude/std-path-predicates.ll
	grep -q '_ARNv3std4path13has_extension' $(BUILD_DIR)/prelude/std-path-predicates.ll
	grep -q '_ARNv3std4path8has_stem' $(BUILD_DIR)/prelude/std-path-predicates.ll
	grep -q '_ARNv3std4path13has_file_stem' $(BUILD_DIR)/prelude/std-path-predicates.ll
	grep -q 'std__path__PathBytes8inherent13has_extension' $(BUILD_DIR)/prelude/std-path-predicates.ll
	grep -q 'std__path__PathBytes8inherent13has_file_stem' $(BUILD_DIR)/prelude/std-path-predicates.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-predicates.ari -o $(BUILD_DIR)/prelude/std-path-predicates.elf
	$(BUILD_DIR)/prelude/std-path-predicates.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/path/std-path-affixes.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-affixes.ll
	grep -q '_ARNv3std4path11starts_with' $(BUILD_DIR)/prelude/std-path-affixes.ll
	grep -q '_ARNv3std4path12strip_prefix' $(BUILD_DIR)/prelude/std-path-affixes.ll
	grep -q '_ARNv3std4path9ends_with' $(BUILD_DIR)/prelude/std-path-affixes.ll
	grep -q '_ARNv3std4path12strip_suffix' $(BUILD_DIR)/prelude/std-path-affixes.ll
	grep -q 'std__path__PathBytes8inherent11starts_with' $(BUILD_DIR)/prelude/std-path-affixes.ll
	grep -q 'std__path__PathBytes8inherent12strip_suffix' $(BUILD_DIR)/prelude/std-path-affixes.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-affixes.ari -o $(BUILD_DIR)/prelude/std-path-affixes.elf
	$(BUILD_DIR)/prelude/std-path-affixes.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/standard-library/ok/path/std-path-edit.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-edit.ll
	grep -q '_ARNv3std4path17with_file_name_in' $(BUILD_DIR)/prelude/std-path-edit.ll
	grep -q '_ARNv3std4path17with_extension_in' $(BUILD_DIR)/prelude/std-path-edit.ll
	grep -q 'std__path__PathBytes8inherent17with_file_name_in' $(BUILD_DIR)/prelude/std-path-edit.ll
	grep -q 'std__path__PathBytes8inherent17with_extension_in' $(BUILD_DIR)/prelude/std-path-edit.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-edit.ari -o $(BUILD_DIR)/prelude/std-path-edit.elf
	$(BUILD_DIR)/prelude/std-path-edit.elf; code=$$?; test $$code -eq 27
	$(TARGET) tests/cases/standard-library/ok/path/std-path-components.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-components.ll
	grep -q '_ARNv3std4path10components' $(BUILD_DIR)/prelude/std-path-components.ll
	grep -q '_ARNv3std4path21components_with_kinds' $(BUILD_DIR)/prelude/std-path-components.ll
	grep -q 'std__path__Components13std__Iterator4next' $(BUILD_DIR)/prelude/std-path-components.ll
	grep -q 'std__path__ComponentsWithKinds13std__Iterator4next' $(BUILD_DIR)/prelude/std-path-components.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-components.ari -o $(BUILD_DIR)/prelude/std-path-components.elf
	$(BUILD_DIR)/prelude/std-path-components.elf; code=$$?; test $$code -eq 242
	$(TARGET) tests/cases/standard-library/ok/path/std-path-bytes.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-bytes.ll
	grep -q '_ARNv3std4path5bytes' $(BUILD_DIR)/prelude/std-path-bytes.ll
	grep -q '_ARNv3std4path7from_os' $(BUILD_DIR)/prelude/std-path-bytes.ll
	grep -q 'std__path__PathBytes8inherent11is_absolute' $(BUILD_DIR)/prelude/std-path-bytes.ll
	grep -q 'std__path__PathBytes8inherent12normalize_in' $(BUILD_DIR)/prelude/std-path-bytes.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-bytes.ari -o $(BUILD_DIR)/prelude/std-path-bytes.elf
	$(BUILD_DIR)/prelude/std-path-bytes.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/path/std-path-bytes.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-path-bytes.elf; $(BUILD_DIR)/prelude/std-path-bytes.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/path/std-path-buf.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q '_ARNv3std4path4join' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q '_ARNv3std4path9join_many' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q '_ARNv3std4path10from_bytes' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q '_ARNv3std4path11from_string' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q '_ARNv3std4path9to_string' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q '_ARNv3std4path16current_dir_join' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q 'std__path__PathBuf8inherent7as_path' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q 'std__path__PathBuf8inherent8as_bytes' $(BUILD_DIR)/prelude/std-path-buf.ll
	grep -q 'std__path__PathBuf8inherent9as_string' $(BUILD_DIR)/prelude/std-path-buf.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-buf.ari -o $(BUILD_DIR)/prelude/std-path-buf.elf
	$(BUILD_DIR)/prelude/std-path-buf.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/path/std-path-windows-lexical.ari --emit-llvm $(BUILD_DIR)/prelude/std-path-windows-lexical.ll
	grep -q '_ARNv3std4path20is_windows_separator' $(BUILD_DIR)/prelude/std-path-windows-lexical.ll
	grep -q '_ARNv3std4path24has_windows_drive_prefix' $(BUILD_DIR)/prelude/std-path-windows-lexical.ll
	grep -q '_ARNv3std4path18windows_unc_prefix' $(BUILD_DIR)/prelude/std-path-windows-lexical.ll
	grep -q 'std__path__PathBytes8inherent19is_windows_absolute' $(BUILD_DIR)/prelude/std-path-windows-lexical.ll
	grep -q 'std__path__PathBuf8inherent14is_windows_unc' $(BUILD_DIR)/prelude/std-path-windows-lexical.ll
	$(TARGET) tests/cases/standard-library/ok/path/std-path-windows-lexical.ari -o $(BUILD_DIR)/prelude/std-path-windows-lexical.elf
	$(BUILD_DIR)/prelude/std-path-windows-lexical.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-mask-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-mask-helpers.ll
	grep -q '_ARNv3std4bits6is_set' $(BUILD_DIR)/prelude/std-bits-mask-helpers.ll
	grep -q '_ARNv3std4bits7any_set' $(BUILD_DIR)/prelude/std-bits-mask-helpers.ll
	grep -q '_ARNv3std4bits15is_power_of_two' $(BUILD_DIR)/prelude/std-bits-mask-helpers.ll
	grep -q '_ARNv3std4bits8align_up' $(BUILD_DIR)/prelude/std-bits-mask-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-mask-helpers.ari -o $(BUILD_DIR)/prelude/std-bits-mask-helpers.elf
	$(BUILD_DIR)/prelude/std-bits-mask-helpers.elf; code=$$?; test $$code -eq 199
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-alignment-policy.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-alignment-policy.ll
	grep -q '_ARNv3std4bits16checked_align_up' $(BUILD_DIR)/prelude/std-bits-alignment-policy.ll
	grep -q '_ARNv3std4bits17wrapping_align_up' $(BUILD_DIR)/prelude/std-bits-alignment-policy.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-alignment-policy.ari -o $(BUILD_DIR)/prelude/std-bits-alignment-policy.elf
	$(BUILD_DIR)/prelude/std-bits-alignment-policy.elf; code=$$?; test $$code -eq 59
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-rotate-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-rotate-helpers.ll
	grep -q '_ARNv3std4bits11rotate_left' $(BUILD_DIR)/prelude/std-bits-rotate-helpers.ll
	grep -q '_ARNv3std4bits12rotate_right' $(BUILD_DIR)/prelude/std-bits-rotate-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-rotate-helpers.ari -o $(BUILD_DIR)/prelude/std-bits-rotate-helpers.elf
	$(BUILD_DIR)/prelude/std-bits-rotate-helpers.elf; code=$$?; test $$code -eq 101
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-scan-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-scan-helpers.ll
	grep -q '_ARNv3std4bits10count_ones' $(BUILD_DIR)/prelude/std-bits-scan-helpers.ll
	grep -q '_ARNv3std4bits11count_zeros' $(BUILD_DIR)/prelude/std-bits-scan-helpers.ll
	grep -q '_ARNv3std4bits13leading_zeros' $(BUILD_DIR)/prelude/std-bits-scan-helpers.ll
	grep -q '_ARNv3std4bits14trailing_zeros' $(BUILD_DIR)/prelude/std-bits-scan-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-scan-helpers.ari -o $(BUILD_DIR)/prelude/std-bits-scan-helpers.elf
	$(BUILD_DIR)/prelude/std-bits-scan-helpers.elf; code=$$?; test $$code -eq 187
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-one-run-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-one-run-helpers.ll
	grep -q '_ARNv3std4bits12leading_ones' $(BUILD_DIR)/prelude/std-bits-one-run-helpers.ll
	grep -q '_ARNv3std4bits13trailing_ones' $(BUILD_DIR)/prelude/std-bits-one-run-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-one-run-helpers.ari -o $(BUILD_DIR)/prelude/std-bits-one-run-helpers.elf
	$(BUILD_DIR)/prelude/std-bits-one-run-helpers.elf; code=$$?; test $$code -eq 158
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-byte-population.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-byte-population.ll
	grep -q '_ARNv3std4bits9byte_swap' $(BUILD_DIR)/prelude/std-bits-byte-population.ll
	grep -q '_ARNv3std4bits16population_count' $(BUILD_DIR)/prelude/std-bits-byte-population.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-byte-population.ari -o $(BUILD_DIR)/prelude/std-bits-byte-population.elf
	$(BUILD_DIR)/prelude/std-bits-byte-population.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-width-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-bits-width-helpers.ll
	grep -q '_ARNv3std4bits9bit_width' $(BUILD_DIR)/prelude/std-bits-width-helpers.ll
	grep -q '_ARNv3std4bits18floor_power_of_two' $(BUILD_DIR)/prelude/std-bits-width-helpers.ll
	grep -q '_ARNv3std4bits17ceil_power_of_two' $(BUILD_DIR)/prelude/std-bits-width-helpers.ll
	grep -q '_ARNv3std4bits8low_mask' $(BUILD_DIR)/prelude/std-bits-width-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/bits/std-bits-width-helpers.ari -o $(BUILD_DIR)/prelude/std-bits-width-helpers.elf
	$(BUILD_DIR)/prelude/std-bits-width-helpers.elf; code=$$?; test $$code -eq 239
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-explicit-move.ari -o $(BUILD_DIR)/prelude/prelude-explicit-move.elf
	$(BUILD_DIR)/prelude/prelude-explicit-move.elf; code=$$?; test $$code -eq 32
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-value-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-mem-value-helpers.ll
	grep -q '_mem_replace' $(BUILD_DIR)/prelude/std-mem-value-helpers.ll
	grep -q '_mem_swap' $(BUILD_DIR)/prelude/std-mem-value-helpers.ll
	grep -q 'load { i64, i64 }, ptr' $(BUILD_DIR)/prelude/std-mem-value-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-value-helpers.ari -o $(BUILD_DIR)/prelude/std-mem-value-helpers.elf
	$(BUILD_DIR)/prelude/std-mem-value-helpers.elf; code=$$?; test $$code -eq 46
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-byte-ops.ari --emit-llvm $(BUILD_DIR)/prelude/std-mem-byte-ops.ll
	grep -q '@llvm.memcpy.p0.p0.i64' $(BUILD_DIR)/prelude/std-mem-byte-ops.ll
	grep -q '@llvm.memmove.p0.p0.i64' $(BUILD_DIR)/prelude/std-mem-byte-ops.ll
	grep -q '@llvm.memset.p0.i64' $(BUILD_DIR)/prelude/std-mem-byte-ops.ll
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-byte-ops.ari -o $(BUILD_DIR)/prelude/std-mem-byte-ops.elf
	$(BUILD_DIR)/prelude/std-mem-byte-ops.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-pointer-ops.ari --emit-llvm $(BUILD_DIR)/prelude/std-mem-pointer-ops.ll
	grep -q 'getelementptr i64' $(BUILD_DIR)/prelude/std-mem-pointer-ops.ll
	grep -q 'getelementptr i8' $(BUILD_DIR)/prelude/std-mem-pointer-ops.ll
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-pointer-ops.ari -o $(BUILD_DIR)/prelude/std-mem-pointer-ops.elf
	$(BUILD_DIR)/prelude/std-mem-pointer-ops.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-page-size.ari --emit-llvm $(BUILD_DIR)/prelude/std-mem-page-size.ll
	grep -q '@ari_builtin_mem_page_size' $(BUILD_DIR)/prelude/std-mem-page-size.ll
	grep -q '@getpagesize' $(BUILD_DIR)/prelude/std-mem-page-size.ll
	$(TARGET) tests/cases/standard-library/ok/mem/std-mem-page-size.ari -o $(BUILD_DIR)/prelude/std-mem-page-size.elf
	$(BUILD_DIR)/prelude/std-mem-page-size.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-header-main.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-header-main.ll
	grep -q 'call i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/prelude-header-main.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/prelude-header-main.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/prelude/prelude-header-main.ll
	$(TARGET) tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari --emit-llvm $(BUILD_DIR)/prelude/std-zone-alloc-array.ll
	grep -q '_ARNv3std4zone18alloc_array__G_i64' $(BUILD_DIR)/prelude/std-zone-alloc-array.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-zone-alloc-array.ll
	$(TARGET) tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari -o $(BUILD_DIR)/prelude/std-zone-alloc-array.elf
	$(BUILD_DIR)/prelude/std-zone-alloc-array.elf; code=$$?; test $$code -eq 72
	$(TARGET) tests/cases/standard-library/ok/zone/std-zone-introspection.ari --emit-llvm $(BUILD_DIR)/prelude/std-zone-introspection.ll
	grep -q '@ari_builtin_zone_capacity' $(BUILD_DIR)/prelude/std-zone-introspection.ll
	grep -q '@ari_builtin_zone_remaining_handle' $(BUILD_DIR)/prelude/std-zone-introspection.ll
	$(TARGET) tests/cases/standard-library/ok/zone/std-zone-introspection.ari -o $(BUILD_DIR)/prelude/std-zone-introspection.elf
	$(BUILD_DIR)/prelude/std-zone-introspection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/zone/std-allocator-capability.ari --emit-llvm $(BUILD_DIR)/prelude/std-allocator-capability.ll
	grep -q '_ARNv3std9allocator9from_zone' $(BUILD_DIR)/prelude/std-allocator-capability.ll
	grep -q '_ARNv4impl25std__allocator__Allocator8inherent10new__G_i64' $(BUILD_DIR)/prelude/std-allocator-capability.ll
	grep -q '_ARNv3std9allocator24of__G_std__vec__Vec_i64_' $(BUILD_DIR)/prelude/std-allocator-capability.ll
	grep -q '_ARNv3std9allocator28of_mut__G_std__vec__Vec_i64_' $(BUILD_DIR)/prelude/std-allocator-capability.ll
	$(TARGET) tests/cases/standard-library/ok/zone/std-allocator-capability.ari -o $(BUILD_DIR)/prelude/std-allocator-capability.elf
	$(BUILD_DIR)/prelude/std-allocator-capability.elf; code=$$?; test $$code -eq 92
	$(TARGET) tests/cases/standard-library/ok/zone/std-region-capability.ari --emit-llvm $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std6region6create' $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std6region9allocator' $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std6region6string' $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std6region10vec__G_i64' $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std6region12boxed__G_i64' $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std6region7cstring' $(BUILD_DIR)/prelude/std-region-capability.ll
	grep -q '_ARNv3std9allocator11from_region' $(BUILD_DIR)/prelude/std-region-capability.ll
	$(TARGET) tests/cases/standard-library/ok/zone/std-region-capability.ari -o $(BUILD_DIR)/prelude/std-region-capability.elf
	$(BUILD_DIR)/prelude/std-region-capability.elf; code=$$?; test $$code -eq 119
	$(TARGET) tests/cases/standard-library/ok/zone/std-zone-backed.ari --emit-llvm $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -q '_ARNv3std4zone24of__G_std__vec__Vec_i64_' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -F -q 'define { ptr, i64 } @"_ARNv4impl35std__collections__HashMap_i64__i64_8inherent5entry"' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -F -q 'define { ptr, i64 } @"_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent5entry"' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -q '_ARNv3std4zone46of__G_std__collections__HashMapEntry_i64__i64_' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -q '_ARNv4impl40std__collections__TreeMapEntry_i64__i64_21std__zone__ZoneBacked4zone' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -q '_ARNv3std4zone9from_zone' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -q '_ARNv3std4zone8metadata' $(BUILD_DIR)/prelude/std-zone-backed.ll
	grep -q '@ari_builtin_zone_allocation_zone' $(BUILD_DIR)/prelude/std-zone-backed.ll
	$(TARGET) tests/cases/standard-library/ok/zone/std-zone-backed.ari -o $(BUILD_DIR)/prelude/std-zone-backed.elf
	$(BUILD_DIR)/prelude/std-zone-backed.elf; code=$$?; test $$code -eq 249
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/zone/std-zone-alloc-array.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-zone-alloc-array.elf; $(BUILD_DIR)/prelude/std-zone-alloc-array.elf; code=$$?; test $$code -eq 72; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/zone/std-zone-introspection.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-zone-introspection.elf; $(BUILD_DIR)/prelude/std-zone-introspection.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/zone/std-allocator-capability.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-allocator-capability.elf; $(BUILD_DIR)/prelude/std-allocator-capability.elf; code=$$?; test $$code -eq 92; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/zone/std-region-capability.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-region-capability.elf; $(BUILD_DIR)/prelude/std-region-capability.elf; code=$$?; test $$code -eq 119; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/zone/std-zone-backed.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-zone-backed.elf; $(BUILD_DIR)/prelude/std-zone-backed.elf; code=$$?; test $$code -eq 249; fi
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-alloc-buffer.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-alloc-buffer.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-vec-alloc-buffer.ll
	grep -q '_ARNv3std3vec19alloc_buffer__G_i64' $(BUILD_DIR)/prelude/std-vec-alloc-buffer.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-with-capacity.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-with-capacity.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-vec-with-capacity.ll
	grep -q '_ARNv3std3vec20with_capacity__G_i64' $(BUILD_DIR)/prelude/std-vec-with-capacity.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-new-handle.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-new-handle.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-vec-new-handle.ll
	grep -q '_ARNv3std3vec10new__G_i64' $(BUILD_DIR)/prelude/std-vec-new-handle.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-macro-new.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-macro-new.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-vec-macro-new.ll
	grep -q '_ARNv3std3vec10new__G_i64' $(BUILD_DIR)/prelude/std-vec-macro-new.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-root-alias.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-root-alias.ll
	grep -q '_ARNv3std3vec10new__G_i64' $(BUILD_DIR)/prelude/std-vec-root-alias.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7push_in' $(BUILD_DIR)/prelude/std-vec-root-alias.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-macro-new.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-macro-new.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-boxed-macro-new.ll
	grep -q '_ARNv3std5boxed10new__G_i64' $(BUILD_DIR)/prelude/std-boxed-macro-new.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-box.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-box.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/std-boxed-box.ll
	grep -q '_ARNv3std5boxed10new__G_i64' $(BUILD_DIR)/prelude/std-boxed-box.ll
	grep -F -q 'define i64 @"_ARNv4impl20std__boxed__Box_i64_8inherent3get"(ptr %arg0)' $(BUILD_DIR)/prelude/std-boxed-box.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent3set' $(BUILD_DIR)/prelude/std-boxed-box.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-set-drop-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-set-drop-value.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent3set' $(BUILD_DIR)/prelude/std-boxed-set-drop-value.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/prelude/std-boxed-set-drop-value.ll) -ge 2
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-replace.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-replace.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent7replace' $(BUILD_DIR)/prelude/std-boxed-replace.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-copy-to.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-copy-to.ll
	grep -F -q 'define { ptr } @"_ARNv4impl20std__boxed__Box_i64_8inherent7copy_to"(ptr %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/std-boxed-copy-to.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-swap.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-swap.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent4swap' $(BUILD_DIR)/prelude/std-boxed-swap.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-as-ref-mut.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-as-ref-mut.ll
	grep -F -q 'define ptr @"_ARNv4impl20std__boxed__Box_i64_8inherent6as_ref"(ptr %arg0)' $(BUILD_DIR)/prelude/std-boxed-as-ref-mut.ll
	grep -F -q 'define ptr @"_ARNv4impl20std__boxed__Box_i64_8inherent6as_mut"(ptr %arg0)' $(BUILD_DIR)/prelude/std-boxed-as-ref-mut.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-as-ptr.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-as-ptr.ll
	grep -F -q 'define ptr @"_ARNv4impl20std__boxed__Box_i64_8inherent6as_ptr"(ptr %arg0)' $(BUILD_DIR)/prelude/std-boxed-as-ptr.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-as-mut-ptr.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-as-mut-ptr.ll
	grep -F -q 'define ptr @"_ARNv4impl20std__boxed__Box_i64_8inherent10as_mut_ptr"(ptr %arg0)' $(BUILD_DIR)/prelude/std-boxed-as-mut-ptr.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-take.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-take.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent4take' $(BUILD_DIR)/prelude/std-boxed-take.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent8is_empty' $(BUILD_DIR)/prelude/std-boxed-take.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-try-take.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-try-take.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent8try_take' $(BUILD_DIR)/prelude/std-boxed-try-take.ll
	grep -q '_ARNv4impl16std__Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-boxed-try-take.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-try-get.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-try-get.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent7try_get' $(BUILD_DIR)/prelude/std-boxed-try-get.ll
	grep -q '_ARNv4impl16std__Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-boxed-try-get.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-clear.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-clear.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent5clear' $(BUILD_DIR)/prelude/std-boxed-clear.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/prelude/std-boxed-clear.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-put-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-put-in.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent6put_in' $(BUILD_DIR)/prelude/std-boxed-put-in.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-drop.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-drop.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_9std__Drop4drop' $(BUILD_DIR)/prelude/std-boxed-drop.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-drop-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-boxed-drop-value.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_9std__Drop4drop' $(BUILD_DIR)/prelude/std-boxed-drop-value.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/prelude/std-boxed-drop-value.ll
	$(TARGET) tests/cases/standard-library/ok/boxed/prelude-box-root.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-box-root.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent3new' $(BUILD_DIR)/prelude/prelude-box-root.ll
	grep -q '_ARNv4impl20std__boxed__Box_i64_8inherent7copy_to' $(BUILD_DIR)/prelude/prelude-box-root.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-alloc-buffer.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-alloc-buffer.ll
	grep -q 'call ptr @"ari_builtin_string_alloc_buffer"' $(BUILD_DIR)/prelude/std-string-alloc-buffer.ll
	grep -q 'call ptr @ari_builtin_zone_alloc(ptr %zone.slot, i64 %capacity, i64 1)' $(BUILD_DIR)/prelude/std-string-alloc-buffer.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-handle.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -q 'ari_builtin_string_with_capacity' $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -q 'ari_builtin_string_new' $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent3len"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent8capacity"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -F -q 'define i1 @"_ARNv4impl19std__string__String8inherent8is_empty"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-handle.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-first-last.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-first-last.ll
	grep -F -q 'define i8 @"_ARNv4impl19std__string__String8inherent5first"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-first-last.ll
	grep -F -q 'define i8 @"_ARNv4impl19std__string__String8inherent4last"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-first-last.ll
	grep -F -q 'define i8 @"_ARNv4impl19std__string__String8inherent3get"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-string-handle.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-try-byte-access.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-try-byte-access.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent9try_first"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-try-byte-access.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent8try_last"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-try-byte-access.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent7try_get"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-string-try-byte-access.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent7try_pop"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-try-byte-access.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-try-byte-access.ari -o $(BUILD_DIR)/prelude/std-string-try-byte-access.elf
	$(BUILD_DIR)/prelude/std-string-try-byte-access.elf; code=$$?; test $$code -eq 98
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-try-byte-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-try-byte-access.elf; $(BUILD_DIR)/prelude/std-string-try-byte-access.elf; code=$$?; test $$code -eq 98; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-byte-remove.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-byte-remove.ll
	grep -F -q 'define i8 @"_ARNv4impl19std__string__String8inherent6remove"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-string-byte-remove.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent10try_remove"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-string-byte-remove.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-byte-remove.ari -o $(BUILD_DIR)/prelude/std-string-byte-remove.elf
	$(BUILD_DIR)/prelude/std-string-byte-remove.elf; code=$$?; test $$code -eq 78
	$(TARGET) tests/cases/standard-library/ok/string/std-string-byte-retain.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-byte-retain.ll
	grep -F -q '_ARNv4impl19std__string__String8inherent6retain' $(BUILD_DIR)/prelude/std-string-byte-retain.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-byte-retain.ari -o $(BUILD_DIR)/prelude/std-string-byte-retain.elf
	$(BUILD_DIR)/prelude/std-string-byte-retain.elf; code=$$?; test $$code -eq 54
	grep -F -q 'define ptr @"_ARNv4impl19std__string__String8inherent6as_ptr"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -q '_ARNv4impl19std__string__String8inherent7push_in' $(BUILD_DIR)/prelude/std-string-handle.ll
	grep -q '_ARNv4impl19std__string__String8inherent8as_slice' $(BUILD_DIR)/prelude/std-string-handle.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-fixed-push.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-fixed-push.ll
	grep -q '_ARNv4impl19std__string__String8inherent4push' $(BUILD_DIR)/prelude/std-string-fixed-push.ll
	grep -q '_ARNv4impl19std__string__String8inherent6insert' $(BUILD_DIR)/prelude/std-string-fixed-push.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-search.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-search.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent8index_of"(ptr %arg0, i8 %arg1)' $(BUILD_DIR)/prelude/std-string-search.ll
	grep -F -q 'define i1 @"_ARNv4impl19std__string__String8inherent8contains"(ptr %arg0, i8 %arg1)' $(BUILD_DIR)/prelude/std-string-search.ll
	grep -F -q 'define i64 @"_ARNv4impl19std__string__String8inherent5count"(ptr %arg0, i8 %arg1)' $(BUILD_DIR)/prelude/std-string-search.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-split-join.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-split-join.ll
	grep -F -q 'std6string7join_in' $(BUILD_DIR)/prelude/std-string-split-join.ll
	grep -F -q 'String8inherent5slice' $(BUILD_DIR)/prelude/std-string-split-join.ll
	grep -F -q 'String8inherent5split' $(BUILD_DIR)/prelude/std-string-split-join.ll
	grep -F -q 'String8inherent7windows' $(BUILD_DIR)/prelude/std-string-split-join.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-split-join.ari -o $(BUILD_DIR)/prelude/std-string-split-join.elf
	$(BUILD_DIR)/prelude/std-string-split-join.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/string/std-string-prefix-suffix.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-prefix-suffix.ll
	grep -F -q 'define i1 @"_ARNv4impl19std__string__String8inherent11starts_with"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/std-string-prefix-suffix.ll
	grep -F -q 'define i1 @"_ARNv4impl19std__string__String8inherent9ends_with"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/std-string-prefix-suffix.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-equals.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-equals.ll
	grep -F -q 'define i1 @"_ARNv4impl19std__string__String8inherent6equals"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/std-string-equals.ll
	grep -F -q 'std__cmp__Eq2eq' $(BUILD_DIR)/prelude/std-string-equals.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-ascii-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-ascii-helpers.ll
	grep -F -q 'define { ptr, i64 } @"_ARNv4impl19std__string__String8inherent10trim_start"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-ascii-helpers.ll
	grep -F -q 'define { ptr, i64 } @"_ARNv4impl19std__string__String8inherent8trim_end"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-ascii-helpers.ll
	grep -F -q 'define { ptr, i64 } @"_ARNv4impl19std__string__String8inherent4trim"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-ascii-helpers.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl19std__string__String8inherent13parse_decimal"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-ascii-helpers.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl19std__string__String8inherent9parse_hex"(ptr %arg0)' $(BUILD_DIR)/prelude/std-string-ascii-helpers.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-ascii-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-ascii-helpers.elf; $(BUILD_DIR)/prelude/std-string-ascii-helpers.elf; code=$$?; test $$code -eq 186; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-prefix-parsers.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-prefix-parsers.ll
	grep -F -q 'String8inherent20parse_decimal_prefix' $(BUILD_DIR)/prelude/std-string-prefix-parsers.ll
	grep -F -q 'String8inherent16parse_hex_prefix' $(BUILD_DIR)/prelude/std-string-prefix-parsers.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-prefix-parsers.ari -o $(BUILD_DIR)/prelude/std-string-prefix-parsers.elf
	$(BUILD_DIR)/prelude/std-string-prefix-parsers.elf; code=$$?; test $$code -eq 39
	$(TARGET) tests/cases/standard-library/ok/string/std-string-signed-parsers.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-signed-parsers.ll
	grep -F -q 'String8inherent20parse_signed_decimal' $(BUILD_DIR)/prelude/std-string-signed-parsers.ll
	grep -F -q 'String8inherent27parse_signed_decimal_prefix' $(BUILD_DIR)/prelude/std-string-signed-parsers.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-signed-parsers.ari -o $(BUILD_DIR)/prelude/std-string-signed-parsers.elf
	$(BUILD_DIR)/prelude/std-string-signed-parsers.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/string/std-string-ascii-case-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.ll
	grep -F -q 'String8inherent18equals_ignore_case' $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.ll
	grep -F -q 'String8inherent23starts_with_ignore_case' $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.ll
	grep -F -q 'String8inherent21ends_with_ignore_case' $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.ll
	grep -F -q 'String8inherent20index_of_ignore_case' $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.ll
	grep -F -q 'String8inherent20contains_ignore_case' $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-ascii-case-helpers.ari -o $(BUILD_DIR)/prelude/std-string-ascii-case-helpers.elf
	$(BUILD_DIR)/prelude/std-string-ascii-case-helpers.elf; code=$$?; test $$code -eq 120
	$(TARGET) tests/cases/standard-library/ok/string/std-string-unicode-helpers.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'String8inherent7is_utf8' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'String8inherent15codepoint_count' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'String8inherent12codepoint_at' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'String8inherent20codepoint_next_index' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'std6string10codepoints' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'std__string__String8inherent10codepoints' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'std__string__Codepoints13std__Iterator4next' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'std__string__Utf88inherent10next_index' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'std__string__Utf88inherent10codepoints' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	grep -F -q 'String8inherent17push_codepoint_in' $(BUILD_DIR)/prelude/std-string-unicode-helpers.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-unicode-helpers.ari -o $(BUILD_DIR)/prelude/std-string-unicode-helpers.elf
	$(BUILD_DIR)/prelude/std-string-unicode-helpers.elf; code=$$?; test $$code -eq 15
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-unicode-helpers.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-unicode-helpers.elf; $(BUILD_DIR)/prelude/std-string-unicode-helpers.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-text-kinds.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string4utf8' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string11utf8_string' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string20utf8_string_optional' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string21utf8_string_unchecked' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string6os_str' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string9os_string' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string20os_string_from_slice' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q '_ARNv3std6string5c_str' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q 'std__string__Utf88inherent15codepoint_count' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q 'std__string__Utf8String8inherent15codepoint_count' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q 'std__string__OsStr8inherent8try_utf8' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q 'std__string__OsString8inherent15try_utf8_string' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	grep -q 'std__c__CStr8inherent8as_slice' $(BUILD_DIR)/prelude/std-string-text-kinds.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-text-kinds.ari -o $(BUILD_DIR)/prelude/std-string-text-kinds.elf
	$(BUILD_DIR)/prelude/std-string-text-kinds.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-text-kinds.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-text-kinds.elf; $(BUILD_DIR)/prelude/std-string-text-kinds.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-literal-boundaries.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-literal-boundaries.ll
	grep -q 'std__string__Utf88inherent15codepoint_count' $(BUILD_DIR)/prelude/std-string-literal-boundaries.ll
	grep -q 'std__path__PathBytes8inherent7join_in' $(BUILD_DIR)/prelude/std-string-literal-boundaries.ll
	grep -q 'std__c__CStr8inherent8as_slice' $(BUILD_DIR)/prelude/std-string-literal-boundaries.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-literal-boundaries.ari -o $(BUILD_DIR)/prelude/std-string-literal-boundaries.elf
	$(BUILD_DIR)/prelude/std-string-literal-boundaries.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-literal-boundaries.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-literal-boundaries.elf; $(BUILD_DIR)/prelude/std-string-literal-boundaries.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-byte-literals.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-byte-literals.ll
	grep -q '_ARNv3std6string5bytes' $(BUILD_DIR)/prelude/std-string-byte-literals.ll
	grep -q 'store i8 99' $(BUILD_DIR)/prelude/std-string-byte-literals.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-byte-literals.ari -o $(BUILD_DIR)/prelude/std-string-byte-literals.elf
	$(BUILD_DIR)/prelude/std-string-byte-literals.elf; code=$$?; test $$code -eq 4
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-byte-literals.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-byte-literals.elf; $(BUILD_DIR)/prelude/std-string-byte-literals.elf; code=$$?; test $$code -eq 4; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-literal-slice.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-literal-slice.ll
	! grep -q '_ARNv3std6string5bytes' $(BUILD_DIR)/prelude/std-string-literal-slice.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-literal-slice.ari -o $(BUILD_DIR)/prelude/std-string-literal-slice.elf
	$(BUILD_DIR)/prelude/std-string-literal-slice.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-literal-slice.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-literal-slice.elf; $(BUILD_DIR)/prelude/std-string-literal-slice.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-literal-sequence.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-literal-sequence.ll
	grep -q 'i8 97' $(BUILD_DIR)/prelude/std-string-literal-sequence.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-literal-sequence.ari -o $(BUILD_DIR)/prelude/std-string-literal-sequence.elf
	$(BUILD_DIR)/prelude/std-string-literal-sequence.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-literal-sequence.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-literal-sequence.elf; $(BUILD_DIR)/prelude/std-string-literal-sequence.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-natural-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-natural-api.ll
	grep -q '_ARNv3std6string4from' $(BUILD_DIR)/prelude/std-string-natural-api.ll
	grep -q 'String8inherent6append' $(BUILD_DIR)/prelude/std-string-natural-api.ll
	grep -q 'String8inherent13contains_text' $(BUILD_DIR)/prelude/std-string-natural-api.ll
	grep -q 'String8inherent7trimmed' $(BUILD_DIR)/prelude/std-string-natural-api.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-natural-api.ari -o $(BUILD_DIR)/prelude/std-string-natural-api.elf
	$(BUILD_DIR)/prelude/std-string-natural-api.elf; code=$$?; test $$code -eq 0
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-natural-api.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-natural-api.elf; $(BUILD_DIR)/prelude/std-string-natural-api.elf; code=$$?; test $$code -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-module-views.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q '_ARNv3std6string5lines' $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q '_ARNv3std6string4trim' $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q '_ARNv3std6string10split_once' $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q '_ARNv3std6string12strip_prefix' $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q '_ARNv3std6string12strip_suffix' $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q '_ARNv3std6string7replace' $(BUILD_DIR)/prelude/std-string-module-views.ll
	grep -q 'std__string__String8inherent8push_str' $(BUILD_DIR)/prelude/std-string-module-views.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-module-views.ari -o $(BUILD_DIR)/prelude/std-string-module-views.elf
	$(BUILD_DIR)/prelude/std-string-module-views.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/c/std-c-interop.ari --emit-llvm $(BUILD_DIR)/prelude/std-c-interop.ll
	grep -q 'declare ptr @"dlopen"' $(BUILD_DIR)/prelude/std-c-interop.ll
	grep -q 'declare ptr @"dlsym"' $(BUILD_DIR)/prelude/std-c-interop.ll
	grep -q 'declare ptr @__errno_location' $(BUILD_DIR)/prelude/std-c-interop.ll
	grep -q '_ARNv3std1c13from_slice_in' $(BUILD_DIR)/prelude/std-c-interop.ll
	grep -q 'std__c__CString8inherent17as_bytes_with_nul' $(BUILD_DIR)/prelude/std-c-interop.ll
	$(TARGET) tests/cases/standard-library/ok/c/std-c-interop.ari -o $(BUILD_DIR)/prelude/std-c-interop.elf
	$(BUILD_DIR)/prelude/std-c-interop.elf; code=$$?; test $$code -eq 7
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/c/std-c-interop.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-c-interop.elf; $(BUILD_DIR)/prelude/std-c-interop.elf; code=$$?; test $$code -eq 7; fi
	$(TARGET) tests/cases/standard-library/ok/c/std-c-dynamic-function.ari --emit-llvm $(BUILD_DIR)/prelude/std-c-dynamic-function.ll
	grep -q 'define ptr @"_ARNv3std1c29function' $(BUILD_DIR)/prelude/std-c-dynamic-function.ll
	grep -q 'call i64 %' $(BUILD_DIR)/prelude/std-c-dynamic-function.ll
	$(TARGET) tests/cases/standard-library/ok/c/std-c-dynamic-function.ari -o $(BUILD_DIR)/prelude/std-c-dynamic-function.elf
	$(BUILD_DIR)/prelude/std-c-dynamic-function.elf; code=$$?; test $$code -eq 2
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/c/std-c-dynamic-function.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-c-dynamic-function.elf; $(BUILD_DIR)/prelude/std-c-dynamic-function.elf; code=$$?; test $$code -eq 2; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-trim-copy.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-trim-copy.ll
	grep -F -q 'define { { ptr, i64, i64 } } @"_ARNv4impl19std__string__String8inherent13trim_start_to"(ptr %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/std-string-trim-copy.ll
	grep -F -q 'define { { ptr, i64, i64 } } @"_ARNv4impl19std__string__String8inherent11trim_end_to"(ptr %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/std-string-trim-copy.ll
	grep -F -q 'define { { ptr, i64, i64 } } @"_ARNv4impl19std__string__String8inherent7trim_to"(ptr %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/std-string-trim-copy.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-trim-copy.ari -o $(BUILD_DIR)/prelude/std-string-trim-copy.elf
	$(BUILD_DIR)/prelude/std-string-trim-copy.elf; code=$$?; test $$code -eq 131
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-trim-copy.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-trim-copy.elf; $(BUILD_DIR)/prelude/std-string-trim-copy.elf; code=$$?; test $$code -eq 131; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-from-copy.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-from-copy.ll
	grep -q 'ari_builtin_string_from_string' $(BUILD_DIR)/prelude/std-string-from-copy.ll
	grep -F -q 'define { { ptr, i64, i64 } } @ari_builtin_string_copy_to(ptr %value.slot, ptr %target.slot)' $(BUILD_DIR)/prelude/std-string-from-copy.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-copy-to-method.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-copy-to-method.ll
	grep -F -q 'define { { ptr, i64, i64 } } @"_ARNv4impl19std__string__String8inherent7copy_to"(ptr %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/std-string-copy-to-method.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-from-slice-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-from-slice-in.ll
	grep -q '_ARNv3std6string13from_slice_in' $(BUILD_DIR)/prelude/std-string-from-slice-in.ll
	grep -q '_ARNv4impl19std__string__String8inherent20extend_from_slice_in' $(BUILD_DIR)/prelude/std-string-from-slice-in.ll
	$(TARGET) tests/cases/standard-library/ok/string/prelude-string-root.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-string-root.ll
	grep -q 'ari_builtin_string_from_string' $(BUILD_DIR)/prelude/prelude-string-root.ll
	grep -q 'ari_builtin_string_copy_to' $(BUILD_DIR)/prelude/prelude-string-root.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-grow.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv4impl19std__string__String8inherent7reserve' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv4impl19std__string__String8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv4impl19std__string__String8inherent7push_in' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv4impl19std__string__String8inherent9insert_in' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv4impl19std__string__String8inherent20extend_from_slice_in' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv4impl19std__string__String8inherent9resize_in' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv3std6string16grow_to_capacity' $(BUILD_DIR)/prelude/std-string-grow.ll
	grep -q '_ARNv3std6string21ensure_spare_capacity' $(BUILD_DIR)/prelude/std-string-grow.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-implicit-zone.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent7reserve' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent7push_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent9insert_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent20extend_from_slice_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent9resize_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent16append_string_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent13append_i64_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent13append_u64_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent14append_bool_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent13append_f32_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	grep -q '_ARNv4impl19std__string__String8inherent13append_f64_in' $(BUILD_DIR)/prelude/std-string-implicit-zone.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-append-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-append-value.ll
	grep -q 'append_value_in' $(BUILD_DIR)/prelude/std-string-append-value.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-append-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-append-value.elf; $(BUILD_DIR)/prelude/std-string-append-value.elf; code=$$?; test $$code -eq 196; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-append-debug.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-append-debug.ll
	grep -q 'append_debug_in' $(BUILD_DIR)/prelude/std-string-append-debug.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-append-debug.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-append-debug.elf; $(BUILD_DIR)/prelude/std-string-append-debug.elf; code=$$?; test $$code -eq 193; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-append.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-append.ll
	grep -q 'append_string_in' $(BUILD_DIR)/prelude/std-string-append.ll
	grep -q 'append_i64_in' $(BUILD_DIR)/prelude/std-string-append.ll
	grep -q 'append_bool_in' $(BUILD_DIR)/prelude/std-string-append.ll
	$(TARGET) tests/cases/standard-library/ok/string/std-string-append-f32.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-append-f32.ll
	grep -F -q 'define void @"_ARNv4impl19std__string__String8inherent13append_f32_in"(ptr %arg0, ptr %arg1, float %arg2, i64 %arg3)' $(BUILD_DIR)/prelude/std-string-append-f32.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-append-f32.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-append-f32.elf; $(BUILD_DIR)/prelude/std-string-append-f32.elf; code=$$?; test $$code -eq 8; fi
	$(TARGET) tests/cases/standard-library/ok/string/std-string-append-u64.ari --emit-llvm $(BUILD_DIR)/prelude/std-string-append-u64.ll
	grep -F -q 'define void @"_ARNv4impl19std__string__String8inherent13append_u64_in"(ptr %arg0, ptr %arg1, i64 %arg2)' $(BUILD_DIR)/prelude/std-string-append-u64.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-append-u64.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-append-u64.elf; $(BUILD_DIR)/prelude/std-string-append-u64.elf; code=$$?; test $$code -eq 24; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-format-in.ll
	grep -q 'append_string_in' $(BUILD_DIR)/prelude/prelude-format-in.ll
	grep -q 'append_i64_in' $(BUILD_DIR)/prelude/prelude-format-in.ll
	grep -q 'append_bool_in' $(BUILD_DIR)/prelude/prelude-format-in.ll
	grep -q 'append_f32_in' $(BUILD_DIR)/prelude/prelude-format-in.ll
	grep -q 'append_f64_in' $(BUILD_DIR)/prelude/prelude-format-in.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-format-in.elf; $(BUILD_DIR)/prelude/prelude-format-in.elf; code=$$?; test $$code -eq 155; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in-u64.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-format-in-u64.ll
	grep -q 'append_u64_in' $(BUILD_DIR)/prelude/prelude-format-in-u64.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in-u64.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-format-in-u64.elf; $(BUILD_DIR)/prelude/prelude-format-in-u64.elf; code=$$?; test $$code -eq 26; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in-display.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-format-in-display.ll
	grep -F -q 'std__fmt__Display9format_in"(ptr' $(BUILD_DIR)/prelude/prelude-format-in-display.ll
	grep -E -q 'getelementptr inbounds \{ i64, i64 \}, ptr %t[0-9]+, i32 0, i32 0' $(BUILD_DIR)/prelude/prelude-format-in-display.ll
	grep -E -q 'getelementptr inbounds \{ i64, i64 \}, ptr %t[0-9]+, i32 0, i32 1' $(BUILD_DIR)/prelude/prelude-format-in-display.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in-display.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-format-in-display.elf; $(BUILD_DIR)/prelude/prelude-format-in-display.elf; code=$$?; test $$code -eq 150; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in-debug.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-format-in-debug.ll
	grep -F -q 'std__fmt__Debug8debug_in"(ptr' $(BUILD_DIR)/prelude/prelude-format-in-debug.ll
	grep -q 'debug_in' $(BUILD_DIR)/prelude/prelude-format-in-debug.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-format-in-debug.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-format-in-debug.elf; $(BUILD_DIR)/prelude/prelude-format-in-debug.elf; code=$$?; test $$code -eq 188; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-format-spec.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q '_ARNv3std3fmt11unsigned_in' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q '_ARNv3std3fmt13debug_text_in' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_unsigned' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_unsigned_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_integer' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_integer_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_boolean' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_boolean_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_text' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_text_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_value' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_value_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_line_text' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_line_text_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_line_value' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_line_value_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_line_debug' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_line_debug_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_concat2' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_concat2_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_concat3' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	grep -q 'write_concat3_bool' $(BUILD_DIR)/prelude/std-fmt-format-spec.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-format-spec.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-format-spec.elf; $(BUILD_DIR)/prelude/std-fmt-format-spec.elf; code=$$?; test $$code -eq 29; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-concat-format-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'format_value' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'std3fmt13format' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'format2' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'format3' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'format4' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'format_texts' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'format_values' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'write_format' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'write_format4' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'write_format_texts' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'write_format_values' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'concat2' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'concat3' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'concat_all' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'concat_strings' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	grep -q 'write_concat_all' $(BUILD_DIR)/prelude/std-fmt-concat-format-value.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-concat-format-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-concat-format-value.elf; $(BUILD_DIR)/prelude/std-fmt-concat-format-value.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-format-validation.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-format-validation.ll
	grep -q '_ARNv3std3fmt14try_with_width' $(BUILD_DIR)/prelude/std-fmt-format-validation.ll
	grep -q '_ARNv3std3fmt18try_with_precision' $(BUILD_DIR)/prelude/std-fmt-format-validation.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-format-validation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-format-validation.elf; $(BUILD_DIR)/prelude/std-fmt-format-validation.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-debug-values.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-debug-values.ll
	grep -q 'debug_value' $(BUILD_DIR)/prelude/std-fmt-debug-values.ll
	grep -q 'write_debug' $(BUILD_DIR)/prelude/std-fmt-debug-values.ll
	grep -q 'write_debug_bool' $(BUILD_DIR)/prelude/std-fmt-debug-values.ll
	grep -q 'print_debug' $(BUILD_DIR)/prelude/std-fmt-debug-values.ll
	grep -q 'println_debug' $(BUILD_DIR)/prelude/std-fmt-debug-values.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-debug-values.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-debug-values.elf; $(BUILD_DIR)/prelude/std-fmt-debug-values.elf > $(BUILD_DIR)/prelude/std-fmt-debug-values.out; code=$$?; grep -qx '"out"false' $(BUILD_DIR)/prelude/std-fmt-debug-values.out; out_ok=$$?; test $$code -eq 6; test $$out_ok -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-char-values.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-char-values.ll
	grep -q '_ARNv3std3fmt7char_in' $(BUILD_DIR)/prelude/std-fmt-char-values.ll
	grep -q '_ARNv3std3fmt13debug_char_in' $(BUILD_DIR)/prelude/std-fmt-char-values.ll
	grep -q 'append_byte' $(BUILD_DIR)/prelude/std-fmt-char-values.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-char-values.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-char-values.elf; $(BUILD_DIR)/prelude/std-fmt-char-values.elf; code=$$?; test $$code -eq 27; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-print-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-print-value.ll
	grep -q 'print_value' $(BUILD_DIR)/prelude/std-fmt-print-value.ll
	grep -q 'println_value' $(BUILD_DIR)/prelude/std-fmt-print-value.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-print-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-print-value.elf; $(BUILD_DIR)/prelude/std-fmt-print-value.elf > $(BUILD_DIR)/prelude/std-fmt-print-value.out; code=$$?; grep -qx 'pv=-12true' $(BUILD_DIR)/prelude/std-fmt-print-value.out; out_ok=$$?; test $$code -eq 11; test $$out_ok -eq 0; fi
	$(TARGET) tests/cases/standard-library/ok/format/std-fmt-display-builtins.ari --emit-llvm $(BUILD_DIR)/prelude/std-fmt-display-builtins.ll
	grep -q '_ARNv3std3fmt8float_in' $(BUILD_DIR)/prelude/std-fmt-display-builtins.ll
	grep -q 'std__fmt__Display9format_in' $(BUILD_DIR)/prelude/std-fmt-display-builtins.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/format/std-fmt-display-builtins.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-fmt-display-builtins.elf; $(BUILD_DIR)/prelude/std-fmt-display-builtins.elf; code=$$?; test $$code -eq 66; fi
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-metadata-methods.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-metadata-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl18std__vec__Vec_i64_8inherent3len"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-metadata-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl18std__vec__Vec_i64_8inherent8capacity"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-metadata-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl18std__vec__Vec_i64_8inherent8is_empty"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-metadata-methods.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-fixed-ops.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent4push' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-replace.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-replace.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7replace' $(BUILD_DIR)/prelude/std-vec-replace.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent4swap' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6insert' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6remove' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -F -q 'define i64 @"_ARNv4impl18std__vec__Vec_i64_8inherent3get"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -F -q 'define i64 @"_ARNv4impl18std__vec__Vec_i64_8inherent8index_of"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -F -q 'define i1 @"_ARNv4impl18std__vec__Vec_i64_8inherent8contains"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	grep -F -q 'define i64 @"_ARNv4impl18std__vec__Vec_i64_8inherent5count"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-fixed-ops.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-get-ref-mut.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-get-ref-mut.ll
	grep -F -q 'define ptr @"_ARNv4impl18std__vec__Vec_i64_8inherent7get_ref"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-get-ref-mut.ll
	grep -F -q 'define ptr @"_ARNv4impl18std__vec__Vec_i64_8inherent7get_mut"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-get-ref-mut.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-slice-compare.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-slice-compare.ll
	grep -F -q 'define i1 @"_ARNv4impl18std__vec__Vec_i64_8inherent11starts_with"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/std-vec-slice-compare.ll
	grep -F -q 'define i1 @"_ARNv4impl18std__vec__Vec_i64_8inherent9ends_with"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/std-vec-slice-compare.ll
	grep -F -q 'define i1 @"_ARNv4impl18std__vec__Vec_i64_8inherent6equals"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/std-vec-slice-compare.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-sequence.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent5slice' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent8split_at' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent4find' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent7compare' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent8ordering' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent6chunks' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent7windows' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent7reverse' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent11rotate_left' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent12rotate_right' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent5dedup' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent4fill' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent9copy_from' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent9partition' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent4sort' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent11stable_sort' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent9is_sorted' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent13binary_search' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent11lower_bound' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent11upper_bound' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent11equal_range' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent15partition_point' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent3min' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	grep -F -q 'std__vec__Vec_i64_8inherent3max' $(BUILD_DIR)/prelude/std-vec-sequence.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-as-slice.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-as-slice.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent8as_slice' $(BUILD_DIR)/prelude/std-vec-as-slice.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-reserve.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-reserve.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7reserve' $(BUILD_DIR)/prelude/std-vec-reserve.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-reserve-extra.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-reserve-extra.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-vec-reserve-extra.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-copy-to.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-copy-to.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7copy_to' $(BUILD_DIR)/prelude/std-vec-copy-to.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-as-ptr.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-as-ptr.ll
	grep -F -q 'define ptr @"_ARNv4impl18std__vec__Vec_i64_8inherent6as_ptr"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-as-ptr.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-as-mut-ptr.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-as-mut-ptr.ll
	grep -F -q 'define ptr @"_ARNv4impl18std__vec__Vec_i64_8inherent10as_mut_ptr"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-as-mut-ptr.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-iter.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-iter.ll
	grep -F -q 'define { ptr, i64, i64 } @"_ARNv4impl18std__vec__Vec_i64_8inherent4iter"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-iter.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl19std__vec__Iter_i64_13std__Iterator4next"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-iter.ll
	grep -F -q 'define { ptr, i64, i64 } @"_ARNv4impl18std__vec__Vec_i64_17std__IntoIterator9into_iter"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-iter.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-adapters.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter34map__G_i64_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter33filter__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter31take__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter31skip__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter36enumerate__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter54zip__G_i64_i64_std__vec__Iter_i64__std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter35fold__G_i64_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std4iter33reduce__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	grep -q '_ARNv3std3vec34collect__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-adapters.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-consumers.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter32count__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter35count_if__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter30nth__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter31last__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter34find_if__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter35position__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter30any__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	grep -q '_ARNv3std4iter30all__G_i64_std__vec__Iter_i64_' $(BUILD_DIR)/prelude/std-iter-consumers.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-consumers.ari -o $(BUILD_DIR)/prelude/std-iter-consumers.elf
	$(BUILD_DIR)/prelude/std-iter-consumers.elf
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-repeat-with.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-repeat-with.ll
	grep -q '_ARNv3std4iter18repeat_with__G_i64' $(BUILD_DIR)/prelude/std-iter-repeat-with.ll
	grep -q '_ARNv4impl26std__iter__RepeatWith_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-repeat-with.ll
	grep -q '_ARNv4impl19std__vec__Vec_Pair_8inherent65extend_iter__G_std__iter__Take_std__iter__RepeatWith_Pair___Pair_' $(BUILD_DIR)/prelude/std-iter-repeat-with.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-repeat-with.ari -o $(BUILD_DIR)/prelude/std-iter-repeat-with.elf
	$(BUILD_DIR)/prelude/std-iter-repeat-with.elf
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-once-empty.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-once-empty.ll
	grep -q '_ARNv3std4iter11once__G_i64' $(BUILD_DIR)/prelude/std-iter-once-empty.ll
	grep -q '_ARNv3std4iter12empty__G_i64' $(BUILD_DIR)/prelude/std-iter-once-empty.ll
	grep -q '_ARNv4impl24std__iter__OnceIter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-once-empty.ll
	grep -q '_ARNv4impl25std__iter__EmptyIter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-once-empty.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-once-empty.ari -o $(BUILD_DIR)/prelude/std-iter-once-empty.elf
	$(BUILD_DIR)/prelude/std-iter-once-empty.elf
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-slice-vec.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-slice-vec.ll
	grep -q '_ARNv4impl19std__SliceIter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-slice-vec.ll
	grep -q '_ARNv4impl22std__SliceIterMut_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-slice-vec.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent8iter_mut' $(BUILD_DIR)/prelude/std-iter-slice-vec.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent34extend_iter__G_std__SliceIter_i64_' $(BUILD_DIR)/prelude/std-iter-slice-vec.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-slice-vec.ari -o $(BUILD_DIR)/prelude/std-iter-slice-vec.elf
	$(BUILD_DIR)/prelude/std-iter-slice-vec.elf; code=$$?; test $$code -eq 83
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-exact-size.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-exact-size.ll
	grep -q 'std__ExactSizeIterator3len' $(BUILD_DIR)/prelude/std-iter-exact-size.ll
	grep -q 'std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-exact-size.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-exact-size.ari -o $(BUILD_DIR)/prelude/std-iter-exact-size.elf
	$(BUILD_DIR)/prelude/std-iter-exact-size.elf; code=$$?; test $$code -eq 210
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-double-ended.ari --emit-llvm $(BUILD_DIR)/prelude/std-iter-double-ended.ll
	grep -q 'std__DoubleEndedIterator9next_back' $(BUILD_DIR)/prelude/std-iter-double-ended.ll
	grep -q 'std__Iterator4next' $(BUILD_DIR)/prelude/std-iter-double-ended.ll
	$(TARGET) tests/cases/standard-library/ok/iter/std-iter-double-ended.ari -o $(BUILD_DIR)/prelude/std-iter-double-ended.elf
	$(BUILD_DIR)/prelude/std-iter-double-ended.elf; code=$$?; test $$code -eq 219
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-push-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-push-in.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7push_in' $(BUILD_DIR)/prelude/std-vec-push-in.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-insert-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-insert-in.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent9insert_in' $(BUILD_DIR)/prelude/std-vec-insert-in.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-extend-from-slice-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-extend-from-slice-in.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent20extend_from_slice_in' $(BUILD_DIR)/prelude/std-vec-extend-from-slice-in.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-from-slice-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-from-slice-in.ll
	grep -q '_ARNv3std3vec20from_slice_in__G_i64' $(BUILD_DIR)/prelude/std-vec-from-slice-in.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent17extend_from_slice' $(BUILD_DIR)/prelude/std-vec-from-slice-in.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-resize-in.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-resize-in.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent9resize_in' $(BUILD_DIR)/prelude/std-vec-resize-in.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-resize-with.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-resize-with.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent11resize_with' $(BUILD_DIR)/prelude/std-vec-resize-with.ll
	grep -q '_ARNv4impl19std__vec__Vec_Pair_8inherent11resize_with' $(BUILD_DIR)/prelude/std-vec-resize-with.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-resize-with.ari -o $(BUILD_DIR)/prelude/std-vec-resize-with.elf
	$(BUILD_DIR)/prelude/std-vec-resize-with.elf
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-growth-paths.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-growth-paths.ll
	grep -q '_ARNv3std3vec27grow_to_own_capacity__G_i64' $(BUILD_DIR)/prelude/std-vec-growth-paths.ll
	grep -q '_ARNv3std3vec28ensure_spare_capacity__G_i64' $(BUILD_DIR)/prelude/std-vec-growth-paths.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-convenience-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent13with_capacity' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent11try_reserve' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent13shrink_to_fit' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6extend' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6append' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent5drain' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6splice' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent11insert_many' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent12remove_range' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl20std__vec__Drain_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	grep -q '_ARNv4impl20std__vec__Drain_i64_24std__DoubleEndedIterator9next_back' $(BUILD_DIR)/prelude/std-vec-convenience-api.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-convenience-api.ari -o $(BUILD_DIR)/prelude/std-vec-convenience-api.elf
	$(BUILD_DIR)/prelude/std-vec-convenience-api.elf; code=$$?; test $$code -eq 123
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-complete-convenience-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent11swap_remove' $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent9split_off' $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.ll
	grep -q 'extend_iter__G_std__SliceIter_i64_' $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent8dedup_by' $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.ll
	grep -q 'dedup_by_key__G_i64' $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-complete-convenience-api.ari -o $(BUILD_DIR)/prelude/std-vec-complete-convenience-api.elf
	$(BUILD_DIR)/prelude/std-vec-complete-convenience-api.elf
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-range-mutation.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-range-mutation.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent11copy_within' $(BUILD_DIR)/prelude/std-vec-range-mutation.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent10fill_range' $(BUILD_DIR)/prelude/std-vec-range-mutation.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent13reverse_range' $(BUILD_DIR)/prelude/std-vec-range-mutation.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent12rotate_range' $(BUILD_DIR)/prelude/std-vec-range-mutation.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent11drain_range' $(BUILD_DIR)/prelude/std-vec-range-mutation.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-range-mutation.ari -o $(BUILD_DIR)/prelude/std-vec-range-mutation.elf
	$(BUILD_DIR)/prelude/std-vec-range-mutation.elf; code=$$?; test $$code -eq 143
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-implicit-zone.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7reserve' $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent4push' $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6insert' $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent17extend_from_slice' $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6resize' $(BUILD_DIR)/prelude/std-vec-implicit-zone.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-pop.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-try-pop.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent7try_pop' $(BUILD_DIR)/prelude/std-vec-try-pop.ll
	grep -q '_ARNv4impl16std__Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-vec-try-pop.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-remove.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-try-remove.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent10try_remove' $(BUILD_DIR)/prelude/std-vec-try-remove.ll
	grep -q '_ARNv4impl16std__Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-vec-try-remove.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-remove.ari -o $(BUILD_DIR)/prelude/std-vec-try-remove.elf
	$(BUILD_DIR)/prelude/std-vec-try-remove.elf; code=$$?; test $$code -eq 128
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-retain.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-retain.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent6retain' $(BUILD_DIR)/prelude/std-vec-retain.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-retain.ari -o $(BUILD_DIR)/prelude/std-vec-retain.elf
	$(BUILD_DIR)/prelude/std-vec-retain.elf; code=$$?; test $$code -eq 54
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-access.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-try-access.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl18std__vec__Vec_i64_8inherent9try_first"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-try-access.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl18std__vec__Vec_i64_8inherent8try_last"(ptr %arg0)' $(BUILD_DIR)/prelude/std-vec-try-access.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl18std__vec__Vec_i64_8inherent7try_get"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/std-vec-try-access.ll
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-access.ari -o $(BUILD_DIR)/prelude/std-vec-try-access.elf
	$(BUILD_DIR)/prelude/std-vec-try-access.elf; code=$$?; test $$code -eq 120
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-shrink-drop-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll
	grep -q '_ARNv3std3vec20shrink_to_len__G_i64' $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent3set' $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent5clear' $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent8truncate' $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_8inherent9resize_in' $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.ll) -ge 3
	$(TARGET) tests/cases/standard-library/ok/vec/std-vec-drop-value.ari --emit-llvm $(BUILD_DIR)/prelude/std-vec-drop-value.ll
	grep -q '_ARNv4impl18std__vec__Vec_i64_9std__Drop4drop' $(BUILD_DIR)/prelude/std-vec-drop-value.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/prelude/std-vec-drop-value.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set.ll
	grep -q '_ARNv3std11collections10new__G_i64' $(BUILD_DIR)/prelude/std-collections-set.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent6insert' $(BUILD_DIR)/prelude/std-collections-set.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent4take' $(BUILD_DIR)/prelude/std-collections-set.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set.ari -o $(BUILD_DIR)/prelude/std-collections-set.elf
	$(BUILD_DIR)/prelude/std-collections-set.elf; code=$$?; test $$code -eq 250
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-access.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set-access.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent5first' $(BUILD_DIR)/prelude/std-collections-set-access.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent9try_first' $(BUILD_DIR)/prelude/std-collections-set-access.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent7try_get' $(BUILD_DIR)/prelude/std-collections-set-access.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent7reserve' $(BUILD_DIR)/prelude/std-collections-set-access.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-collections-set-access.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent7try_pop' $(BUILD_DIR)/prelude/std-collections-set-access.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-access.ari -o $(BUILD_DIR)/prelude/std-collections-set-access.elf
	$(BUILD_DIR)/prelude/std-collections-set-access.elf; code=$$?; test $$code -eq 120
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-replace.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set-replace.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent7replace' $(BUILD_DIR)/prelude/std-collections-set-replace.ll
	grep -q '_ARNv4impl16std__Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-collections-set-replace.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-replace.ari -o $(BUILD_DIR)/prelude/std-collections-set-replace.elf
	$(BUILD_DIR)/prelude/std-collections-set-replace.elf; code=$$?; test $$code -eq 127
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-implicit-zone.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent6insert' $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent7replace' $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent7reserve' $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-implicit-zone.ari -o $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.elf
	$(BUILD_DIR)/prelude/std-collections-set-implicit-zone.elf; code=$$?; test $$code -eq 119
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-implicit-zone.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__HashMap_i64__i64_8inherent7reserve' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__HashMap_i64__i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__HashSet_i64_8inherent7replace' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__HashSet_i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__TreeMap_i64__i64_8inherent6insert' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__TreeMap_i64__i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__TreeSet_i64_8inherent13reserve_extra' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__Deque_i64_8inherent9push_back' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__LinkedList_i64_8inherent10push_front' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__BinaryHeap_i64_8inherent4push' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	grep -q 'std__collections__PriorityQueue_i64_8inherent4push' $(BUILD_DIR)/prelude/std-collections-implicit-zone.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-implicit-zone.ari -o $(BUILD_DIR)/prelude/std-collections-implicit-zone.elf
	$(BUILD_DIR)/prelude/std-collections-implicit-zone.elf; code=$$?; test $$code -eq 190
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-relations.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent6equals' $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent9is_subset' $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent11is_superset' $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent11is_disjoint' $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	grep -q 'intersection' $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	grep -q 'symmetric_difference' $(BUILD_DIR)/prelude/std-collections-set-relations.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-relations.ari -o $(BUILD_DIR)/prelude/std-collections-set-relations.elf
	$(BUILD_DIR)/prelude/std-collections-set-relations.elf; code=$$?; test $$code -eq 195
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-iter.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set-iter.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent4iter' $(BUILD_DIR)/prelude/std-collections-set-iter.ll
	grep -q 'std__collections__Iter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-set-iter.ll
	grep -q 'std__collections__Set_i64_17std__IntoIterator9into_iter' $(BUILD_DIR)/prelude/std-collections-set-iter.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-iter.ari -o $(BUILD_DIR)/prelude/std-collections-set-iter.elf
	$(BUILD_DIR)/prelude/std-collections-set-iter.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-hash.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent6insert' $(BUILD_DIR)/prelude/std-collections-hash.ll
	grep -q '_ARNv3std11collections36hash_map_grow_to_capacity__G_i64_i64' $(BUILD_DIR)/prelude/std-collections-hash.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent4take' $(BUILD_DIR)/prelude/std-collections-hash.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash.ari -o $(BUILD_DIR)/prelude/std-collections-hash.elf
	$(BUILD_DIR)/prelude/std-collections-hash.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-with-hash.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-hash-with-hash.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent9with_hash' $(BUILD_DIR)/prelude/std-collections-hash-with-hash.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent9with_hash' $(BUILD_DIR)/prelude/std-collections-hash-with-hash.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-with-hash.ari -o $(BUILD_DIR)/prelude/std-collections-hash-with-hash.elf
	$(BUILD_DIR)/prelude/std-collections-hash-with-hash.elf; code=$$?; test $$code -eq 63
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-set-relations.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent6equals' $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent9is_subset' $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent11is_superset' $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent11is_disjoint' $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	grep -q 'HashSetIntersection' $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	grep -q 'HashSetSymmetricDifference' $(BUILD_DIR)/prelude/std-collections-hash-set-relations.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-set-relations.ari -o $(BUILD_DIR)/prelude/std-collections-hash-set-relations.elf
	$(BUILD_DIR)/prelude/std-collections-hash-set-relations.elf; code=$$?; test $$code -eq 195
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-hash-iter.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent4keys' $(BUILD_DIR)/prelude/std-collections-hash-iter.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent6values' $(BUILD_DIR)/prelude/std-collections-hash-iter.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent4iter' $(BUILD_DIR)/prelude/std-collections-hash-iter.ll
	grep -q 'std__collections__HashSet_i64_17std__IntoIterator9into_iter' $(BUILD_DIR)/prelude/std-collections-hash-iter.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari -o $(BUILD_DIR)/prelude/std-collections-hash-iter.elf
	$(BUILD_DIR)/prelude/std-collections-hash-iter.elf; code=$$?; test $$code -eq 164
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-string-map.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q '_ARNv3std11collections11hash_string' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q '_ARNv3std11collections38string_hash_map__G_std__string__String' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q '_ARNv3std11collections15string_hash_set' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent12contains_key' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent6get_or' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent18contains_key_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent9get_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent12get_or_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent13get_mut_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent13try_get_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent17try_get_mut_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent12remove_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashMap_std__string__String__std__string__String_8inherent18remove_entry_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashSet_std__string__String_8inherent14contains_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashSet_std__string__String_8inherent9get_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashSet_std__string__String_8inherent13try_get_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashSet_std__string__String_8inherent10take_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	grep -q 'HashSet_std__string__String_8inherent12remove_bytes' $(BUILD_DIR)/prelude/std-collections-string-map.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-string-map.ari -o $(BUILD_DIR)/prelude/std-collections-string-map.elf
	$(BUILD_DIR)/prelude/std-collections-string-map.elf; code=$$?; test $$code -eq 255
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-representatives.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-set-representatives.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent7try_get' $(BUILD_DIR)/prelude/std-collections-set-representatives.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent3get' $(BUILD_DIR)/prelude/std-collections-set-representatives.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-representatives.ari -o $(BUILD_DIR)/prelude/std-collections-set-representatives.elf
	$(BUILD_DIR)/prelude/std-collections-set-representatives.elf; code=$$?; test $$code -eq 236
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entries.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-entries.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent7entries' $(BUILD_DIR)/prelude/std-collections-map-entries.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent7entries' $(BUILD_DIR)/prelude/std-collections-map-entries.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entries.ari -o $(BUILD_DIR)/prelude/std-collections-map-entries.elf
	$(BUILD_DIR)/prelude/std-collections-map-entries.elf; code=$$?; test $$code -eq 71
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-view-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent10values_mut' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl39std__collections__HashMapValuesMut_i64_8inherent4next' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent4iter' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent8iter_mut' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_17std__IntoIterator9into_iter' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent5drain' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl45std__collections__HashMapEntriesMut_i64__i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl40std__collections__HashMapDrain_i64__i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent10values_mut' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl39std__collections__TreeMapValuesMut_i64_8inherent4next' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent4iter' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent8iter_mut' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_17std__IntoIterator9into_iter' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent5drain' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl45std__collections__TreeMapEntriesMut_i64__i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl40std__collections__TreeMapDrain_i64__i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent5drain' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent5drain' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent5drain' $(BUILD_DIR)/prelude/std-collections-view-api.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-view-api.ari -o $(BUILD_DIR)/prelude/std-collections-view-api.elf
	$(BUILD_DIR)/prelude/std-collections-view-api.elf; code=$$?; test $$code -eq 110
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-natural-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-natural-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent12contains_key' $(BUILD_DIR)/prelude/std-collections-map-natural-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent6get_or' $(BUILD_DIR)/prelude/std-collections-map-natural-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent12contains_key' $(BUILD_DIR)/prelude/std-collections-map-natural-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent6get_or' $(BUILD_DIR)/prelude/std-collections-map-natural-api.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-natural-api.ari -o $(BUILD_DIR)/prelude/std-collections-map-natural-api.elf
	$(BUILD_DIR)/prelude/std-collections-map-natural-api.elf; code=$$?; test $$code -eq 145
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-mut-access.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent11try_get_mut' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent7get_mut' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent7replace' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent11try_get_mut' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent7get_mut' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent7replace' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	grep -q '_ARNv4impl34std__collections__MapValueMut_i64_8inherent9value_mut' $(BUILD_DIR)/prelude/std-collections-map-mut-access.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-mut-access.ari -o $(BUILD_DIR)/prelude/std-collections-map-mut-access.elf
	$(BUILD_DIR)/prelude/std-collections-map-mut-access.elf; code=$$?; test $$code -eq 236
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-api.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent5entry' $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent12remove_entry' $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	grep -q '_ARNv4impl40std__collections__HashMapEntry_i64__i64_8inherent9or_insert' $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	grep -q '_ARNv4impl40std__collections__HashMapEntry_i64__i64_8inherent14or_insert_with' $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	grep -q '_ARNv4impl40std__collections__TreeMapEntry_i64__i64_8inherent10and_modify' $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent12remove_entry' $(BUILD_DIR)/prelude/std-collections-map-entry-api.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-api.ari -o $(BUILD_DIR)/prelude/std-collections-map-entry-api.elf
	$(BUILD_DIR)/prelude/std-collections-map-entry-api.elf; code=$$?; test $$code -eq 136
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-accessors.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.ll
	grep -q '_ARNv4impl36std__collections__MapEntry_i64__i64_8inherent3key' $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.ll
	grep -q '_ARNv4impl40std__collections__HashMapEntry_i64__i64_8inherent6insert' $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.ll
	grep -q '_ARNv4impl40std__collections__HashMapEntry_i64__i64_8inherent9value_mut' $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.ll
	grep -q '_ARNv4impl40std__collections__TreeMapEntry_i64__i64_8inherent6remove' $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-accessors.ari -o $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.elf
	$(BUILD_DIR)/prelude/std-collections-map-entry-accessors.elf; code=$$?; test $$code -eq 131
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-defaults.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-entry-defaults.ll
	grep -q '_ARNv4impl40std__collections__HashMapEntry_i64__i64_8inherent10or_default' $(BUILD_DIR)/prelude/std-collections-map-entry-defaults.ll
	grep -q '_ARNv4impl40std__collections__HashMapEntry_i64__i64_8inherent12insert_entry' $(BUILD_DIR)/prelude/std-collections-map-entry-defaults.ll
	grep -q '_ARNv4impl40std__collections__TreeMapEntry_i64__i64_8inherent10or_default' $(BUILD_DIR)/prelude/std-collections-map-entry-defaults.ll
	grep -q '_ARNv4impl40std__collections__TreeMapEntry_i64__i64_8inherent12insert_entry' $(BUILD_DIR)/prelude/std-collections-map-entry-defaults.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-defaults.ari -o $(BUILD_DIR)/prelude/std-collections-map-entry-defaults.elf
	$(BUILD_DIR)/prelude/std-collections-map-entry-defaults.elf
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-retain.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-retain.ll
	grep -q '_ARNv4impl26std__collections__Set_i64_8inherent6retain' $(BUILD_DIR)/prelude/std-collections-retain.ll
	grep -q '_ARNv4impl30std__collections__HashSet_i64_8inherent6retain' $(BUILD_DIR)/prelude/std-collections-retain.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent6retain' $(BUILD_DIR)/prelude/std-collections-retain.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent6retain' $(BUILD_DIR)/prelude/std-collections-retain.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent6retain' $(BUILD_DIR)/prelude/std-collections-retain.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-retain.ari -o $(BUILD_DIR)/prelude/std-collections-retain.elf
	$(BUILD_DIR)/prelude/std-collections-retain.elf
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-value-predicates.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-map-value-predicates.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent14contains_value' $(BUILD_DIR)/prelude/std-collections-map-value-predicates.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent14contains_value' $(BUILD_DIR)/prelude/std-collections-map-value-predicates.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-value-predicates.ari -o $(BUILD_DIR)/prelude/std-collections-map-value-predicates.elf
	$(BUILD_DIR)/prelude/std-collections-map-value-predicates.elf; code=$$?; test $$code -eq 90
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent6insert' $(BUILD_DIR)/prelude/std-collections-tree.ll
	grep -q '_ARNv3std11collections30tree_map_fix_insert__G_i64_i64' $(BUILD_DIR)/prelude/std-collections-tree.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent7replace' $(BUILD_DIR)/prelude/std-collections-tree.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree.ari -o $(BUILD_DIR)/prelude/std-collections-tree.elf
	$(BUILD_DIR)/prelude/std-collections-tree.elf; code=$$?; test $$code -eq 159
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-boundaries.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree-boundaries.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent9first_key' $(BUILD_DIR)/prelude/std-collections-tree-boundaries.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent14try_last_value' $(BUILD_DIR)/prelude/std-collections-tree-boundaries.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent5first' $(BUILD_DIR)/prelude/std-collections-tree-boundaries.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent8try_last' $(BUILD_DIR)/prelude/std-collections-tree-boundaries.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-boundaries.ari -o $(BUILD_DIR)/prelude/std-collections-tree-boundaries.elf
	$(BUILD_DIR)/prelude/std-collections-tree-boundaries.elf; code=$$?; test $$code -eq 238
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-entry-boundaries.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent11first_entry' $(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent14try_last_entry' $(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-entry-boundaries.ari -o $(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.elf
	$(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.elf; code=$$?; test $$code -eq 77
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-bounds.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree-bounds.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent11lower_bound' $(BUILD_DIR)/prelude/std-collections-tree-bounds.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent11upper_bound' $(BUILD_DIR)/prelude/std-collections-tree-bounds.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-bounds.ari -o $(BUILD_DIR)/prelude/std-collections-tree-bounds.elf
	$(BUILD_DIR)/prelude/std-collections-tree-bounds.elf; code=$$?; test $$code -eq 197
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-copy-to.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-copy-to.ll
	grep -q '_ARNv4impl35std__collections__HashMap_i64__i64_8inherent7copy_to' $(BUILD_DIR)/prelude/std-collections-copy-to.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent7copy_to' $(BUILD_DIR)/prelude/std-collections-copy-to.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-copy-to.ari -o $(BUILD_DIR)/prelude/std-collections-copy-to.elf
	$(BUILD_DIR)/prelude/std-collections-copy-to.elf; code=$$?; test $$code -eq 48
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-structure-copy-to.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-structure-copy-to.ll
	grep -q '_ARNv4impl28std__collections__Deque_i64_8inherent7copy_to' $(BUILD_DIR)/prelude/std-collections-structure-copy-to.ll
	grep -q '_ARNv4impl36std__collections__PriorityQueue_i64_8inherent7copy_to' $(BUILD_DIR)/prelude/std-collections-structure-copy-to.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-structure-copy-to.ari -o $(BUILD_DIR)/prelude/std-collections-structure-copy-to.elf
	$(BUILD_DIR)/prelude/std-collections-structure-copy-to.elf; code=$$?; test $$code -eq 53
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-remove.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree-remove.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent6remove' $(BUILD_DIR)/prelude/std-collections-tree-remove.ll
	grep -q '_ARNv3std11collections31tree_map_delete_slot__G_i64_i64' $(BUILD_DIR)/prelude/std-collections-tree-remove.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent4take' $(BUILD_DIR)/prelude/std-collections-tree-remove.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent6remove' $(BUILD_DIR)/prelude/std-collections-tree-remove.ll
	grep -q '_ARNv3std11collections27tree_set_delete_slot__G_i64' $(BUILD_DIR)/prelude/std-collections-tree-remove.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-remove.ari -o $(BUILD_DIR)/prelude/std-collections-tree-remove.elf
	$(BUILD_DIR)/prelude/std-collections-tree-remove.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-set-relations.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent6equals' $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent9is_subset' $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent11is_superset' $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent11is_disjoint' $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	grep -q 'TreeSetUnion' $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	grep -q 'TreeSetSymmetricDifference' $(BUILD_DIR)/prelude/std-collections-tree-set-relations.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-set-relations.ari -o $(BUILD_DIR)/prelude/std-collections-tree-set-relations.elf
	$(BUILD_DIR)/prelude/std-collections-tree-set-relations.elf; code=$$?; test $$code -eq 195
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-tree-iter.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent4keys' $(BUILD_DIR)/prelude/std-collections-tree-iter.ll
	grep -q '_ARNv4impl35std__collections__TreeMap_i64__i64_8inherent6values' $(BUILD_DIR)/prelude/std-collections-tree-iter.ll
	grep -q '_ARNv4impl30std__collections__TreeSet_i64_8inherent4iter' $(BUILD_DIR)/prelude/std-collections-tree-iter.ll
	grep -q 'std__collections__TreeSet_i64_17std__IntoIterator9into_iter' $(BUILD_DIR)/prelude/std-collections-tree-iter.ll
	$(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari -o $(BUILD_DIR)/prelude/std-collections-tree-iter.elf
	$(BUILD_DIR)/prelude/std-collections-tree-iter.elf; code=$$?; test $$code -eq 98
	$(TARGET) tests/cases/standard-library/ok/collections/deque/std-collections-deque.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-deque.ll
	grep -q '_ARNv3std11collections12deque__G_i64' $(BUILD_DIR)/prelude/std-collections-deque.ll
	grep -q '_ARNv4impl28std__collections__Deque_i64_8inherent10push_front' $(BUILD_DIR)/prelude/std-collections-deque.ll
	grep -q '_ARNv4impl28std__collections__Deque_i64_8inherent9push_back' $(BUILD_DIR)/prelude/std-collections-deque.ll
	grep -q 'std__collections__DequeIter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-deque.ll
	$(TARGET) tests/cases/standard-library/ok/collections/deque/std-collections-deque.ari -o $(BUILD_DIR)/prelude/std-collections-deque.elf
	$(BUILD_DIR)/prelude/std-collections-deque.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/collections/ring-buffer/std-collections-ring-buffer.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-ring-buffer.ll
	grep -q '_ARNv3std11collections18ring_buffer__G_i64' $(BUILD_DIR)/prelude/std-collections-ring-buffer.ll
	grep -q '_ARNv4impl33std__collections__RingBuffer_i64_8inherent4push' $(BUILD_DIR)/prelude/std-collections-ring-buffer.ll
	grep -q '_ARNv4impl33std__collections__RingBuffer_i64_8inherent14push_overwrite' $(BUILD_DIR)/prelude/std-collections-ring-buffer.ll
	grep -q 'std__collections__RingBufferIter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-ring-buffer.ll
	$(TARGET) tests/cases/standard-library/ok/collections/ring-buffer/std-collections-ring-buffer.ari -o $(BUILD_DIR)/prelude/std-collections-ring-buffer.elf
	$(BUILD_DIR)/prelude/std-collections-ring-buffer.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/collections/linked-list/std-collections-linked-list.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-linked-list.ll
	grep -q '_ARNv3std11collections18linked_list__G_i64' $(BUILD_DIR)/prelude/std-collections-linked-list.ll
	grep -q '_ARNv3std11collections29linked_list_alloc_node__G_i64' $(BUILD_DIR)/prelude/std-collections-linked-list.ll
	grep -q '_ARNv4impl33std__collections__LinkedList_i64_8inherent13try_remove_at' $(BUILD_DIR)/prelude/std-collections-linked-list.ll
	grep -q 'std__collections__LinkedListIter_i64_13std__Iterator4next' $(BUILD_DIR)/prelude/std-collections-linked-list.ll
	$(TARGET) tests/cases/standard-library/ok/collections/linked-list/std-collections-linked-list.ari -o $(BUILD_DIR)/prelude/std-collections-linked-list.elf
	$(BUILD_DIR)/prelude/std-collections-linked-list.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/standard-library/ok/collections/heap/std-collections-heap.ari --emit-llvm $(BUILD_DIR)/prelude/std-collections-heap.ll
	grep -q '_ARNv3std11collections18binary_heap__G_i64' $(BUILD_DIR)/prelude/std-collections-heap.ll
	grep -q '_ARNv3std11collections21priority_queue__G_i64' $(BUILD_DIR)/prelude/std-collections-heap.ll
	grep -q '_ARNv3std11collections28binary_heap_sift_down__G_i64' $(BUILD_DIR)/prelude/std-collections-heap.ll
	grep -q '_ARNv4impl33std__collections__BinaryHeap_i64_8inherent13with_capacity' $(BUILD_DIR)/prelude/std-collections-heap.ll
	grep -q '_ARNv4impl36std__collections__PriorityQueue_i64_8inherent13with_capacity' $(BUILD_DIR)/prelude/std-collections-heap.ll
	grep -q '_ARNv4impl36std__collections__PriorityQueue_i64_8inherent4push' $(BUILD_DIR)/prelude/std-collections-heap.ll
	$(TARGET) tests/cases/standard-library/ok/collections/heap/std-collections-heap.ari -o $(BUILD_DIR)/prelude/std-collections-heap.elf
	$(BUILD_DIR)/prelude/std-collections-heap.elf; code=$$?; test $$code -eq 42
	if grep -q 'extern "C".*ari_builtin' lib/std.arih; then exit 1; fi
	$(TARGET) tests/cases/standard-library/ok/runtime/ari-builtin-extern.ari --emit-llvm $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	grep -q 'call i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	grep -q 'call i64 @"ari_builtin_write_u64"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	grep -q 'call i64 @"ari_builtin_newline"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	if grep -q 'declare i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll; then exit 1; fi
	if grep -q 'declare i64 @"ari_builtin_write_u64"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll; then exit 1; fi
	if grep -q 'declare i64 @"ari_builtin_newline"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll; then exit 1; fi
	$(TARGET) tests/cases/standard-library/ok/prelude/std-prelude-unqualified.ari --emit-llvm $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -q 'call i64 @"_ARNv10span_score"' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -q 'call ptr @"ari_builtin_zone_create"' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -F -q 'getelementptr inbounds { i64, [3 x i64] }' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/std-prelude-local-shadow.ari --emit-llvm $(BUILD_DIR)/prelude/std-prelude-local-shadow.ll
	grep -q 'call i64 @"_ARNv5Debug"' $(BUILD_DIR)/prelude/std-prelude-local-shadow.ll
	$(TARGET) tests/cases/standard-library/ok/smoke/std-library-smoke.ari --emit-llvm $(BUILD_DIR)/prelude/std-library-smoke.ll
	grep -q '_ARNv3std3vec10new__G_i64' $(BUILD_DIR)/prelude/std-library-smoke.ll
	grep -q 'ari_builtin_string_new' $(BUILD_DIR)/prelude/std-library-smoke.ll
	grep -q '_ARNv3std5boxed10new__G_i64' $(BUILD_DIR)/prelude/std-library-smoke.ll
	grep -F -q 'Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-library-smoke.ll
	grep -F -q 'Result_i64__i32_8inherent9unwrap_or' $(BUILD_DIR)/prelude/std-library-smoke.ll
	$(TARGET) tests/cases/standard-library/ok/smoke/std-library-smoke.ari -o $(BUILD_DIR)/prelude/std-library-smoke.elf
	$(BUILD_DIR)/prelude/std-library-smoke.elf; code=$$?; test $$code -eq 232
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result.ll
	grep -q "try.ok" $(BUILD_DIR)/prelude/prelude-option-result.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/prelude/prelude-option-result.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-view.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q '_ARNv3std12slice__G_i64' $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "slice.bounds.fail" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "slice.range.bounds.fail" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "store i64 8" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "store i64 7" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "store i64 5" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-methods.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl15std__Slice_i64_8inherent5first"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl15std__Slice_i64_8inherent4last"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl15std__Slice_i64_8inherent3get"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl15std__Slice_i64_8inherent8contains"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl15std__Slice_i64_8inherent8index_of"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i64 @"_ARNv4impl15std__Slice_i64_8inherent5count"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl15std__Slice_i64_8inherent6equals"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl15std__Slice_i64_8inherent11starts_with"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl15std__Slice_i64_8inherent9ends_with"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/prelude/prelude-slice-methods.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-convenience.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent12strip_prefix' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent12strip_suffix' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent11split_first' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent10split_last' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent9first_mut' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent8last_mut' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent7get_mut' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent7find_if' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent8position' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent9rposition' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent3any' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent3all' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	grep -F -q 'Slice_i64_8inherent8count_if' $(BUILD_DIR)/prelude/prelude-slice-convenience.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent5slice' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent8split_at' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent4find' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent7compare' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent8ordering' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent6chunks' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent7windows' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent7reverse' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent11rotate_left' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent12rotate_right' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent4fill' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent9copy_from' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent9partition' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent5dedup' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent4sort' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent11stable_sort' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent9is_sorted' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent13binary_search' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent11lower_bound' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent11upper_bound' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent11equal_range' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent15partition_point' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent3min' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	grep -F -q 'Slice_i64_8inherent3max' $(BUILD_DIR)/prelude/prelude-slice-sequence.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-metadata.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-metadata.ll
	grep -F -q 'define i1 @"_ARNv4impl15std__Slice_i64_8inherent8is_empty"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-slice-metadata.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-option-access.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-option-access.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl15std__Slice_i64_8inherent9try_first"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-slice-option-access.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl15std__Slice_i64_8inherent8try_last"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-slice-option-access.ll
	grep -F -q 'define { i32, i64 } @"_ARNv4impl15std__Slice_i64_8inherent7try_get"(ptr %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-slice-option-access.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-as-ptr.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-as-ptr.ll
	grep -F -q 'define ptr @"_ARNv4impl15std__Slice_i64_8inherent6as_ptr"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-slice-as-ptr.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-copy-to.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-copy-to.ll
	grep -q '_ARNv4impl15std__Slice_i64_8inherent7copy_to' $(BUILD_DIR)/prelude/prelude-slice-copy-to.ll
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-view.ari -o $(BUILD_DIR)/prelude/prelude-slice-view.elf
	$(BUILD_DIR)/prelude/prelude-slice-view.elf; code=$$?; test $$code -eq 101
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-methods.ari -o $(BUILD_DIR)/prelude/prelude-slice-methods.elf
	$(BUILD_DIR)/prelude/prelude-slice-methods.elf; code=$$?; test $$code -eq 105
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-convenience.ari -o $(BUILD_DIR)/prelude/prelude-slice-convenience.elf
	$(BUILD_DIR)/prelude/prelude-slice-convenience.elf; code=$$?; test $$code -eq 44
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari -o $(BUILD_DIR)/prelude/prelude-slice-sequence.elf
	$(BUILD_DIR)/prelude/prelude-slice-sequence.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-metadata.ari -o $(BUILD_DIR)/prelude/prelude-slice-metadata.elf
	$(BUILD_DIR)/prelude/prelude-slice-metadata.elf; code=$$?; test $$code -eq 72
	$(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-option-access.ari -o $(BUILD_DIR)/prelude/prelude-slice-option-access.elf
	$(BUILD_DIR)/prelude/prelude-slice-option-access.elf; code=$$?; test $$code -eq 77
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-methods.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl16std__Option_i64_8inherent7is_some"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-option-result-methods.ll
	grep -F -q 'Option_i64_8inherent9unwrap_or' $(BUILD_DIR)/prelude/prelude-option-result-methods.ll
	grep -F -q 'define i1 @"_ARNv4impl21std__Result_i64__i32_8inherent5is_ok"(ptr %arg0)' $(BUILD_DIR)/prelude/prelude-option-result-methods.ll
	grep -F -q 'Result_i64__i32_8inherent9unwrap_or' $(BUILD_DIR)/prelude/prelude-option-result-methods.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-methods.ari -o $(BUILD_DIR)/prelude/prelude-option-result-methods.elf
	$(BUILD_DIR)/prelude/prelude-option-result-methods.elf; code=$$?; test $$code -eq 91
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-ref-access.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'Option_i64_8inherent6as_ref' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'Option_i64_8inherent6as_mut' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'Option_i64_8inherent4take' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'Option_i64_8inherent7replace' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'Result_i64__i64_8inherent6as_ref' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'Result_i64__i64_8inherent6as_mut' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'std__option__OptionRef_i64_8inherent6unwrap' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	grep -F -q 'std__result__ResultMut_i64__i64_8inherent10unwrap_err' $(BUILD_DIR)/prelude/prelude-option-result-ref-access.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-ref-access.ari -o $(BUILD_DIR)/prelude/prelude-option-result-ref-access.elf
	$(BUILD_DIR)/prelude/prelude-option-result-ref-access.elf
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-predicates.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl16std__Option_i64_8inherent11is_some_and"({ i32, i64 } %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl16std__Option_i64_8inherent10is_none_or"({ i32, i64 } %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl16std__Option_i64_8inherent8contains"({ i32, i64 } %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl21std__Result_i64__i32_8inherent9is_ok_and"({ i32, i64 } %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl21std__Result_i64__i32_8inherent10is_err_and"({ i32, i64 } %arg0, ptr %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl21std__Result_i64__i32_8inherent8contains"({ i32, i64 } %arg0, i64 %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	grep -F -q 'define i1 @"_ARNv4impl21std__Result_i64__i32_8inherent12contains_err"({ i32, i64 } %arg0, i32 %arg1)' $(BUILD_DIR)/prelude/prelude-option-result-predicates.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-predicates.ari -o $(BUILD_DIR)/prelude/prelude-option-result-predicates.elf
	$(BUILD_DIR)/prelude/prelude-option-result-predicates.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-filter.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-filter.ll
	grep -F -q 'Option_i64_8inherent6filter' $(BUILD_DIR)/prelude/prelude-option-filter.ll
	grep -F -q 'Option_bool_8inherent6filter' $(BUILD_DIR)/prelude/prelude-option-filter.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-filter.ari -o $(BUILD_DIR)/prelude/prelude-option-filter.elf
	$(BUILD_DIR)/prelude/prelude-option-filter.elf; code=$$?; test $$code -eq 59
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-flatten.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-flatten.ll
	grep -F -q 'std__Option_i64__8inherent7flatten' $(BUILD_DIR)/prelude/prelude-option-flatten.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-flatten.ari -o $(BUILD_DIR)/prelude/prelude-option-flatten.elf
	$(BUILD_DIR)/prelude/prelude-option-flatten.elf; code=$$?; test $$code -eq 61
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-transpose.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-transpose.ll
	grep -F -q 'std__Option_std__Result_i64__i32__8inherent9transpose' $(BUILD_DIR)/prelude/prelude-option-transpose.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-transpose.ari -o $(BUILD_DIR)/prelude/prelude-option-transpose.elf
	$(BUILD_DIR)/prelude/prelude-option-transpose.elf; code=$$?; test $$code -eq 71
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-result-transpose.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-result-transpose.ll
	grep -F -q 'std__Result_std__Option_i64___i32_8inherent9transpose' $(BUILD_DIR)/prelude/prelude-result-transpose.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-result-transpose.ari -o $(BUILD_DIR)/prelude/prelude-result-transpose.elf
	$(BUILD_DIR)/prelude/prelude-result-transpose.elf; code=$$?; test $$code -eq 67
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-combinators.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent10map__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent13map_or__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent18map_or_else__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent7inspect' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent10and__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent15and_then__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent2or' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Option_i64_8inherent7or_else' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent10map__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent14map_err__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent13map_or__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent18map_or_else__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent7inspect' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent11inspect_err' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent10and__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent15and_then__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent9or__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	grep -F -q 'Result_i64__i32_8inherent14or_else__G_i64' $(BUILD_DIR)/prelude/prelude-option-result-combinators.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-combinators.ari -o $(BUILD_DIR)/prelude/prelude-option-result-combinators.elf
	$(BUILD_DIR)/prelude/prelude-option-result-combinators.elf; code=$$?; test $$code -eq 225
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-conversions.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Option_i64_8inherent14unwrap_or_else' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Option_i64_8inherent12ok_or__G_i32' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Option_i64_8inherent17ok_or_else__G_i32' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Option_i64_8inherent3xor' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Result_i64__i32_8inherent2ok' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Result_i64__i32_8inherent3err' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	grep -F -q 'Result_i64__i32_8inherent14unwrap_or_else' $(BUILD_DIR)/prelude/prelude-option-result-conversions.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-conversions.ari -o $(BUILD_DIR)/prelude/prelude-option-result-conversions.elf
	$(BUILD_DIR)/prelude/prelude-option-result-conversions.elf; code=$$?; test $$code -eq 95
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-unwrap.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	grep -F -q 'Option_i64_8inherent6unwrap' $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	grep -F -q 'Option_i64_8inherent6expect' $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	grep -F -q 'Result_i64__i32_8inherent6unwrap' $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	grep -F -q 'Result_i64__i32_8inherent10unwrap_err' $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	grep -F -q 'call void @"ari_builtin_panic"()' $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	grep -F -q 'unreachable' $(BUILD_DIR)/prelude/prelude-option-result-unwrap.ll
	$(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-unwrap-llvm.ari -o $(BUILD_DIR)/prelude/prelude-option-result-unwrap-llvm.elf
	$(BUILD_DIR)/prelude/prelude-option-result-unwrap-llvm.elf; code=$$?; test $$code -eq 52
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result.elf; $(BUILD_DIR)/prelude/prelude-option-result.elf; code=$$?; test $$code -eq 67; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-methods.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result-methods.elf; $(BUILD_DIR)/prelude/prelude-option-result-methods.elf; code=$$?; test $$code -eq 91; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-ref-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result-ref-access.elf; $(BUILD_DIR)/prelude/prelude-option-result-ref-access.elf; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-predicates.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result-predicates.elf; $(BUILD_DIR)/prelude/prelude-option-result-predicates.elf; code=$$?; test $$code -eq 21; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-filter.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-filter.elf; $(BUILD_DIR)/prelude/prelude-option-filter.elf; code=$$?; test $$code -eq 59; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-flatten.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-flatten.elf; $(BUILD_DIR)/prelude/prelude-option-flatten.elf; code=$$?; test $$code -eq 61; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-transpose.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-transpose.elf; $(BUILD_DIR)/prelude/prelude-option-transpose.elf; code=$$?; test $$code -eq 71; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-result-transpose.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-result-transpose.elf; $(BUILD_DIR)/prelude/prelude-result-transpose.elf; code=$$?; test $$code -eq 67; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-combinators.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result-combinators.elf; $(BUILD_DIR)/prelude/prelude-option-result-combinators.elf; code=$$?; test $$code -eq 225; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-conversions.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result-conversions.elf; $(BUILD_DIR)/prelude/prelude-option-result-conversions.elf; code=$$?; test $$code -eq 95; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/prelude-option-result-unwrap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result-unwrap.elf; $(BUILD_DIR)/prelude/prelude-option-result-unwrap.elf; code=$$?; test $$code -eq 91; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-view.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-view.elf; $(BUILD_DIR)/prelude/prelude-slice-view.elf; code=$$?; test $$code -eq 101; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-methods.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-methods.elf; $(BUILD_DIR)/prelude/prelude-slice-methods.elf; code=$$?; test $$code -eq 105; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-convenience.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-convenience.elf; $(BUILD_DIR)/prelude/prelude-slice-convenience.elf; code=$$?; test $$code -eq 44; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-sequence.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-sequence.elf; $(BUILD_DIR)/prelude/prelude-slice-sequence.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-as-ptr.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-as-ptr.elf; $(BUILD_DIR)/prelude/prelude-slice-as-ptr.elf; code=$$?; test $$code -eq 47; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/prelude-slice-copy-to.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-copy-to.elf; $(BUILD_DIR)/prelude/prelude-slice-copy-to.elf; code=$$?; test $$code -eq 32; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then \
		$(TARGET) tests/cases/standard-library/ok/prelude/prelude-header-main.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-header-main.elf; \
		$(BUILD_DIR)/prelude/prelude-header-main.elf >$(BUILD_DIR)/prelude/prelude-header-main.out; \
		code=$$?; test $$code -eq 21; \
		grep -qx "1" $(BUILD_DIR)/prelude/prelude-header-main.out; \
	fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-alloc-buffer.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-alloc-buffer.elf; $(BUILD_DIR)/prelude/std-vec-alloc-buffer.elf; code=$$?; test $$code -eq 41; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-with-capacity.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-with-capacity.elf; $(BUILD_DIR)/prelude/std-vec-with-capacity.elf; code=$$?; test $$code -eq 46; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-new-handle.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-new-handle.elf; $(BUILD_DIR)/prelude/std-vec-new-handle.elf; code=$$?; test $$code -eq 40; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-macro-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-macro-new.elf; $(BUILD_DIR)/prelude/std-vec-macro-new.elf; code=$$?; test $$code -eq 29; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-root-alias.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-root-alias.elf; $(BUILD_DIR)/prelude/std-vec-root-alias.elf; code=$$?; test $$code -eq 14; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-macro-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-macro-new.elf; $(BUILD_DIR)/prelude/std-boxed-macro-new.elf; code=$$?; test $$code -eq 50; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-box.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-box.elf; $(BUILD_DIR)/prelude/std-boxed-box.elf; code=$$?; test $$code -eq 30; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-set-drop-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-set-drop-value.elf; $(BUILD_DIR)/prelude/std-boxed-set-drop-value.elf; code=$$?; test $$code -eq 34; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-replace.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-replace.elf; $(BUILD_DIR)/prelude/std-boxed-replace.elf; code=$$?; test $$code -eq 29; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-copy-to.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-copy-to.elf; $(BUILD_DIR)/prelude/std-boxed-copy-to.elf; code=$$?; test $$code -eq 51; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-swap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-swap.elf; $(BUILD_DIR)/prelude/std-boxed-swap.elf; code=$$?; test $$code -eq 85; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-as-ref-mut.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-as-ref-mut.elf; $(BUILD_DIR)/prelude/std-boxed-as-ref-mut.elf; code=$$?; test $$code -eq 45; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-as-ptr.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-as-ptr.elf; $(BUILD_DIR)/prelude/std-boxed-as-ptr.elf; code=$$?; test $$code -eq 30; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-as-mut-ptr.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-as-mut-ptr.elf; $(BUILD_DIR)/prelude/std-boxed-as-mut-ptr.elf; code=$$?; test $$code -eq 38; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-take.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-take.elf; $(BUILD_DIR)/prelude/std-boxed-take.elf; code=$$?; test $$code -eq 24; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-try-take.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-try-take.elf; $(BUILD_DIR)/prelude/std-boxed-try-take.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-try-get.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-try-get.elf; $(BUILD_DIR)/prelude/std-boxed-try-get.elf; code=$$?; test $$code -eq 60; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-clear.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-clear.elf; $(BUILD_DIR)/prelude/std-boxed-clear.elf; code=$$?; test $$code -eq 30; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-put-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-put-in.elf; $(BUILD_DIR)/prelude/std-boxed-put-in.elf; code=$$?; test $$code -eq 110; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/std-boxed-drop.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-boxed-drop.elf; $(BUILD_DIR)/prelude/std-boxed-drop.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/boxed/prelude-box-root.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-box-root.elf; $(BUILD_DIR)/prelude/prelude-box-root.elf; code=$$?; test $$code -eq 43; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-alloc-buffer.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-alloc-buffer.elf; $(BUILD_DIR)/prelude/std-string-alloc-buffer.elf; code=$$?; test $$code -eq 6; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-handle.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-handle.elf; $(BUILD_DIR)/prelude/std-string-handle.elf; code=$$?; test $$code -eq 153; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-byte-remove.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-byte-remove.elf; $(BUILD_DIR)/prelude/std-string-byte-remove.elf; code=$$?; test $$code -eq 78; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-byte-retain.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-byte-retain.elf; $(BUILD_DIR)/prelude/std-string-byte-retain.elf; code=$$?; test $$code -eq 54; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-fixed-push.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-fixed-push.elf; $(BUILD_DIR)/prelude/std-string-fixed-push.elf; code=$$?; test $$code -eq 179; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-first-last.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-first-last.elf; $(BUILD_DIR)/prelude/std-string-first-last.elf; code=$$?; test $$code -eq 239; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-search.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-search.elf; $(BUILD_DIR)/prelude/std-string-search.elf; code=$$?; test $$code -eq 77; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-split-join.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-split-join.elf; $(BUILD_DIR)/prelude/std-string-split-join.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-prefix-suffix.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-prefix-suffix.elf; $(BUILD_DIR)/prelude/std-string-prefix-suffix.elf; code=$$?; test $$code -eq 143; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-equals.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-equals.elf; $(BUILD_DIR)/prelude/std-string-equals.elf; code=$$?; test $$code -eq 143; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-from-copy.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-from-copy.elf; $(BUILD_DIR)/prelude/std-string-from-copy.elf; code=$$?; test $$code -eq 163; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-copy-to-method.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-copy-to-method.elf; $(BUILD_DIR)/prelude/std-string-copy-to-method.elf; code=$$?; test $$code -eq 104; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-from-slice-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-from-slice-in.elf; $(BUILD_DIR)/prelude/std-string-from-slice-in.elf; code=$$?; test $$code -eq 103; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-grow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-grow.elf; $(BUILD_DIR)/prelude/std-string-grow.elf; code=$$?; test $$code -eq 177; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/string/std-string-implicit-zone.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-string-implicit-zone.elf; $(BUILD_DIR)/prelude/std-string-implicit-zone.elf; code=$$?; test $$code -eq 142; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-metadata-methods.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-metadata-methods.elf; $(BUILD_DIR)/prelude/std-vec-metadata-methods.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-fixed-ops.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-fixed-ops.elf; $(BUILD_DIR)/prelude/std-vec-fixed-ops.elf; code=$$?; test $$code -eq 118; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-get-ref-mut.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-get-ref-mut.elf; $(BUILD_DIR)/prelude/std-vec-get-ref-mut.elf; code=$$?; test $$code -eq 30; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-slice-compare.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-slice-compare.elf; $(BUILD_DIR)/prelude/std-vec-slice-compare.elf; code=$$?; test $$code -eq 221; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-sequence.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-sequence.elf; $(BUILD_DIR)/prelude/std-vec-sequence.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-replace.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-replace.elf; $(BUILD_DIR)/prelude/std-vec-replace.elf; code=$$?; test $$code -eq 22; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-as-slice.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-as-slice.elf; $(BUILD_DIR)/prelude/std-vec-as-slice.elf; code=$$?; test $$code -eq 99; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-reserve.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-reserve.elf; $(BUILD_DIR)/prelude/std-vec-reserve.elf; code=$$?; test $$code -eq 43; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-reserve-extra.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-reserve-extra.elf; $(BUILD_DIR)/prelude/std-vec-reserve-extra.elf; code=$$?; test $$code -eq 41; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-copy-to.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-copy-to.elf; $(BUILD_DIR)/prelude/std-vec-copy-to.elf; code=$$?; test $$code -eq 19; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-as-ptr.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-as-ptr.elf; $(BUILD_DIR)/prelude/std-vec-as-ptr.elf; code=$$?; test $$code -eq 17; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-as-mut-ptr.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-as-mut-ptr.elf; $(BUILD_DIR)/prelude/std-vec-as-mut-ptr.elf; code=$$?; test $$code -eq 30; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-iter.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-iter.elf; $(BUILD_DIR)/prelude/std-vec-iter.elf; code=$$?; test $$code -eq 31; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/iter/std-iter-adapters.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-iter-adapters.elf; $(BUILD_DIR)/prelude/std-iter-adapters.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/iter/std-iter-exact-size.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-iter-exact-size.elf; $(BUILD_DIR)/prelude/std-iter-exact-size.elf; code=$$?; test $$code -eq 210; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/iter/std-iter-double-ended.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-iter-double-ended.elf; $(BUILD_DIR)/prelude/std-iter-double-ended.elf; code=$$?; test $$code -eq 219; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-push-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-push-in.elf; $(BUILD_DIR)/prelude/std-vec-push-in.elf; code=$$?; test $$code -eq 25; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-insert-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-insert-in.elf; $(BUILD_DIR)/prelude/std-vec-insert-in.elf; code=$$?; test $$code -eq 23; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-extend-from-slice-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-extend-from-slice-in.elf; $(BUILD_DIR)/prelude/std-vec-extend-from-slice-in.elf; code=$$?; test $$code -eq 11; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-from-slice-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-from-slice-in.elf; $(BUILD_DIR)/prelude/std-vec-from-slice-in.elf; code=$$?; test $$code -eq 40; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-resize-in.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-resize-in.elf; $(BUILD_DIR)/prelude/std-vec-resize-in.elf; code=$$?; test $$code -eq 37; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-growth-paths.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-growth-paths.elf; $(BUILD_DIR)/prelude/std-vec-growth-paths.elf; code=$$?; test $$code -eq 135; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-convenience-api.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-convenience-api.elf; $(BUILD_DIR)/prelude/std-vec-convenience-api.elf; code=$$?; test $$code -eq 123; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-implicit-zone.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-implicit-zone.elf; $(BUILD_DIR)/prelude/std-vec-implicit-zone.elf; code=$$?; test $$code -eq 36; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-pop.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-try-pop.elf; $(BUILD_DIR)/prelude/std-vec-try-pop.elf; code=$$?; test $$code -eq 25; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-try-remove.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-try-remove.elf; $(BUILD_DIR)/prelude/std-vec-try-remove.elf; code=$$?; test $$code -eq 128; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-retain.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-retain.elf; $(BUILD_DIR)/prelude/std-vec-retain.elf; code=$$?; test $$code -eq 54; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-shrink-drop-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.elf; $(BUILD_DIR)/prelude/std-vec-shrink-drop-value.elf; code=$$?; test $$code -eq 13; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/vec/std-vec-drop-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-vec-drop-value.elf; $(BUILD_DIR)/prelude/std-vec-drop-value.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set.elf; $(BUILD_DIR)/prelude/std-collections-set.elf; code=$$?; test $$code -eq 250; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set-access.elf; $(BUILD_DIR)/prelude/std-collections-set-access.elf; code=$$?; test $$code -eq 120; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-replace.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set-replace.elf; $(BUILD_DIR)/prelude/std-collections-set-replace.elf; code=$$?; test $$code -eq 127; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-implicit-zone.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.elf; $(BUILD_DIR)/prelude/std-collections-set-implicit-zone.elf; code=$$?; test $$code -eq 119; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-implicit-zone.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-implicit-zone.elf; $(BUILD_DIR)/prelude/std-collections-implicit-zone.elf; code=$$?; test $$code -eq 190; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-relations.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set-relations.elf; $(BUILD_DIR)/prelude/std-collections-set-relations.elf; code=$$?; test $$code -eq 195; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-iter.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set-iter.elf; $(BUILD_DIR)/prelude/std-collections-set-iter.elf; code=$$?; test $$code -eq 37; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-hash.elf; $(BUILD_DIR)/prelude/std-collections-hash.elf; code=$$?; test $$code -eq 255; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-set-relations.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-hash-set-relations.elf; $(BUILD_DIR)/prelude/std-collections-hash-set-relations.elf; code=$$?; test $$code -eq 195; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-hash-iter.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-hash-iter.elf; $(BUILD_DIR)/prelude/std-collections-hash-iter.elf; code=$$?; test $$code -eq 164; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-string-map.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-string-map.elf; $(BUILD_DIR)/prelude/std-collections-string-map.elf; code=$$?; test $$code -eq 255; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-set-representatives.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-set-representatives.elf; $(BUILD_DIR)/prelude/std-collections-set-representatives.elf; code=$$?; test $$code -eq 236; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entries.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-map-entries.elf; $(BUILD_DIR)/prelude/std-collections-map-entries.elf; code=$$?; test $$code -eq 71; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-natural-api.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-map-natural-api.elf; $(BUILD_DIR)/prelude/std-collections-map-natural-api.elf; code=$$?; test $$code -eq 145; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-mut-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-map-mut-access.elf; $(BUILD_DIR)/prelude/std-collections-map-mut-access.elf; code=$$?; test $$code -eq 236; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-entry-accessors.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.elf; $(BUILD_DIR)/prelude/std-collections-map-entry-accessors.elf; code=$$?; test $$code -eq 131; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-map-value-predicates.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-map-value-predicates.elf; $(BUILD_DIR)/prelude/std-collections-map-value-predicates.elf; code=$$?; test $$code -eq 90; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree.elf; $(BUILD_DIR)/prelude/std-collections-tree.elf; code=$$?; test $$code -eq 159; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-boundaries.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree-boundaries.elf; $(BUILD_DIR)/prelude/std-collections-tree-boundaries.elf; code=$$?; test $$code -eq 238; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-entry-boundaries.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.elf; $(BUILD_DIR)/prelude/std-collections-tree-entry-boundaries.elf; code=$$?; test $$code -eq 77; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-bounds.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree-bounds.elf; $(BUILD_DIR)/prelude/std-collections-tree-bounds.elf; code=$$?; test $$code -eq 197; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-copy-to.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-copy-to.elf; $(BUILD_DIR)/prelude/std-collections-copy-to.elf; code=$$?; test $$code -eq 48; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-structure-copy-to.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-structure-copy-to.elf; $(BUILD_DIR)/prelude/std-collections-structure-copy-to.elf; code=$$?; test $$code -eq 53; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-remove.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree-remove.elf; $(BUILD_DIR)/prelude/std-collections-tree-remove.elf; code=$$?; test $$code -eq 23; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-set-relations.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree-set-relations.elf; $(BUILD_DIR)/prelude/std-collections-tree-set-relations.elf; code=$$?; test $$code -eq 195; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/std-collections-tree-iter.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-tree-iter.elf; $(BUILD_DIR)/prelude/std-collections-tree-iter.elf; code=$$?; test $$code -eq 98; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/deque/std-collections-deque.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-deque.elf; $(BUILD_DIR)/prelude/std-collections-deque.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/ring-buffer/std-collections-ring-buffer.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-ring-buffer.elf; $(BUILD_DIR)/prelude/std-collections-ring-buffer.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/linked-list/std-collections-linked-list.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-linked-list.elf; $(BUILD_DIR)/prelude/std-collections-linked-list.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/collections/heap/std-collections-heap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-collections-heap.elf; $(BUILD_DIR)/prelude/std-collections-heap.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/std-prelude-unqualified.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-prelude-unqualified.elf; $(BUILD_DIR)/prelude/std-prelude-unqualified.elf; code=$$?; test $$code -eq 39; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/standard-library/ok/prelude/std-prelude-local-shadow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-prelude-local-shadow.elf; $(BUILD_DIR)/prelude/std-prelude-local-shadow.elf; code=$$?; test $$code -eq 8; fi
	$(TARGET) --no-implicit-std --module-path lib tests/cases/standard-library/ok/prelude/prelude-header-explicit-std.ari --emit-module-metadata $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta --emit-llvm $(BUILD_DIR)/prelude/prelude-header-explicit-std.ll
	grep -q 'option	implicit_std	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std	lib/std.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::io	lib/std/io.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::io	io	lib/std/io.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::input	lib/std/input.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::input	input	lib/std/input.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::context	lib/std/context.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::context	context	lib/std/context.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::env	lib/std/env.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::env	env	lib/std/env.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::process	lib/std/process.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::process	process	lib/std/process.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::time	lib/std/time.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::time	time	lib/std/time.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::fs	lib/std/fs.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::fs	fs	lib/std/fs.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::mem	lib/std/mem.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::mem	mem	lib/std/mem.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::zone	lib/std/zone.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::zone	zone	lib/std/zone.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::boxed	lib/std/boxed.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::boxed	boxed	lib/std/boxed.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::string	lib/std/string.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::string	string	lib/std/string.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::c	lib/std/c.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::c	c	lib/std/c.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::vec	lib/std/vec.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::vec	vec	lib/std/vec.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::collections	lib/std/collections.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::collections	collections	lib/std/collections.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::iter	lib/std/iter.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::iter	iter	lib/std/iter.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::fmt	lib/std/fmt.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::fmt	fmt	lib/std/fmt.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::cmp	lib/std/cmp.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::cmp	cmp	lib/std/cmp.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::convert	lib/std/convert.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::convert	convert	lib/std/convert.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::math	lib/std/math.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::math	math	lib/std/math.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::ascii	lib/std/ascii.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::ascii	ascii	lib/std/ascii.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std::bits	lib/std/bits.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'import	std	std::bits	bits	lib/std/bits.arih	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'item	std::io	extern-ari-fn	std::io::write_i64	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'item	std::io	extern-ari-fn	std::io::write_u64	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'call i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/prelude-header-explicit-std.ll
	grep -q 'call i64 @"ari_builtin_write_u64"' $(BUILD_DIR)/prelude/prelude-header-explicit-std.ll
	if $(TARGET) --no-implicit-std tests/cases/standard-library/errors/prelude/std-use-without-implicit.ari -o $(BUILD_DIR)/prelude/std-use-without-implicit.elf >$(BUILD_DIR)/prelude/std-use-without-implicit.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'write_i64'" $(BUILD_DIR)/prelude/std-use-without-implicit.err
	if $(TARGET) --no-implicit-std tests/cases/standard-library/errors/prelude/prelude-bare-without-std.ari -o $(BUILD_DIR)/prelude/prelude-bare-without-std.elf >$(BUILD_DIR)/prelude/prelude-bare-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'write_i64'" $(BUILD_DIR)/prelude/prelude-bare-without-std.err
	if $(TARGET) --no-implicit-std tests/cases/standard-library/errors/prelude/prelude-path-without-std.ari -o $(BUILD_DIR)/prelude/prelude-path-without-std.elf >$(BUILD_DIR)/prelude/prelude-path-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'io::write_i64'" $(BUILD_DIR)/prelude/prelude-path-without-std.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-maybe-removed.ari -o $(BUILD_DIR)/prelude/prelude-maybe-removed.elf >$(BUILD_DIR)/prelude/prelude-maybe-removed.err 2>&1; then exit 1; fi
	grep -q "unknown type 'Maybe'" $(BUILD_DIR)/prelude/prelude-maybe-removed.err
	if $(TARGET) tests/cases/standard-library/errors/fs/std-fs-dir-entry-different-zone.ari -o $(BUILD_DIR)/prelude/std-fs-dir-entry-different-zone.elf >$(BUILD_DIR)/prelude/std-fs-dir-entry-different-zone.err 2>&1; then exit 1; fi
	grep -q "zone-backed struct literal fields must come from the same zone" $(BUILD_DIR)/prelude/std-fs-dir-entry-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-alloc-buffer-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-alloc-buffer-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-alloc-buffer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'data' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-alloc-buffer-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-with-capacity-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-with-capacity-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-with-capacity-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-with-capacity-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-new-handle-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-new-handle-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-new-handle-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'vec' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-new-handle-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-macro-new-shape.ari -o $(BUILD_DIR)/prelude/std-vec-macro-new-shape.elf >$(BUILD_DIR)/prelude/std-vec-macro-new-shape.err 2>&1; then exit 1; fi
	grep -q "Vec! expects Vec!(T, ref mut zone, capacity)" $(BUILD_DIR)/prelude/std-vec-macro-new-shape.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-macro-new-shape.ari -o $(BUILD_DIR)/prelude/std-boxed-macro-new-shape.elf >$(BUILD_DIR)/prelude/std-boxed-macro-new-shape.err 2>&1; then exit 1; fi
	grep -q "Box! expects Box!(T, ref mut zone, value)" $(BUILD_DIR)/prelude/std-boxed-macro-new-shape.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-metadata-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-metadata-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-metadata-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'vec' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-metadata-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-get-ref-mut-conflict.ari -o $(BUILD_DIR)/prelude/std-vec-get-ref-mut-conflict.elf >$(BUILD_DIR)/prelude/std-vec-get-ref-mut-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow already borrowed binding 'vec'" $(BUILD_DIR)/prelude/std-vec-get-ref-mut-conflict.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-if-handle-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-if-handle-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-if-handle-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'chosen' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-if-handle-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-push-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-push-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-push-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'vec' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-push-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-slice-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-slice-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-slice-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'tail' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-slice-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-reserve-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-reserve-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-reserve-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.reserve_in zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-reserve-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-reserve-untracked-receiver.ari -o $(BUILD_DIR)/prelude/std-vec-reserve-untracked-receiver.elf >$(BUILD_DIR)/prelude/std-vec-reserve-untracked-receiver.err 2>&1; then exit 1; fi
	grep -q "zone-backed struct literal fields must come from one zone" $(BUILD_DIR)/prelude/std-vec-reserve-untracked-receiver.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-implicit-zone-untracked.ari -o $(BUILD_DIR)/prelude/std-vec-implicit-zone-untracked.elf >$(BUILD_DIR)/prelude/std-vec-implicit-zone-untracked.err 2>&1; then exit 1; fi
	grep -q "zone-backed struct literal fields must come from one zone" $(BUILD_DIR)/prelude/std-vec-implicit-zone-untracked.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-reserve-extra-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-reserve-extra-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-reserve-extra-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.reserve_extra_in zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-reserve-extra-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-push-in-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-push-in-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-push-in-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.push_in zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-push-in-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-insert-in-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-insert-in-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-insert-in-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.insert_in zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-insert-in-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-extend-from-slice-in-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-extend-from-slice-in-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-extend-from-slice-in-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.extend_from_slice_in zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-extend-from-slice-in-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-resize-in-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-resize-in-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-resize-in-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.resize_in zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-resize-in-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-split-off-different-zone.ari -o $(BUILD_DIR)/prelude/std-vec-split-off-different-zone.elf >$(BUILD_DIR)/prelude/std-vec-split-off-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::vec::Vec.split_off zone argument must match the vector allocation zone" $(BUILD_DIR)/prelude/std-vec-split-off-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-copy-to-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-vec-copy-to-after-target-reset.elf >$(BUILD_DIR)/prelude/std-vec-copy-to-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-copy-to-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-from-slice-in-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-vec-from-slice-in-after-target-reset.elf >$(BUILD_DIR)/prelude/std-vec-from-slice-in-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-from-slice-in-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-set-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-set-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'set' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-set-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-iter-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-set-iter-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-set-iter-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-set-iter-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-insert-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-set-insert-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-set-insert-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::Set.insert zone argument must match the set allocation zone" $(BUILD_DIR)/prelude/std-collections-set-insert-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-replace-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-set-replace-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-set-replace-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::Set.replace zone argument must match the set allocation zone" $(BUILD_DIR)/prelude/std-collections-set-replace-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-reserve-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-set-reserve-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-set-reserve-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::Set.reserve zone argument must match the set allocation zone" $(BUILD_DIR)/prelude/std-collections-set-reserve-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-reserve-extra-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-set-reserve-extra-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-set-reserve-extra-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::Set.reserve_extra zone argument must match the set allocation zone" $(BUILD_DIR)/prelude/std-collections-set-reserve-extra-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-set-implicit-zone-untracked.ari -o $(BUILD_DIR)/prelude/std-collections-set-implicit-zone-untracked.elf >$(BUILD_DIR)/prelude/std-collections-set-implicit-zone-untracked.err 2>&1; then exit 1; fi
	grep -q "std::collections::Set.reserve receiver must come from a tracked zone allocation to infer its zone" $(BUILD_DIR)/prelude/std-collections-set-implicit-zone-untracked.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-implicit-zone-untracked.ari -o $(BUILD_DIR)/prelude/std-collections-implicit-zone-untracked.elf >$(BUILD_DIR)/prelude/std-collections-implicit-zone-untracked.err 2>&1; then exit 1; fi
	grep -q "std::collections::HashMap.reserve receiver must come from a tracked zone allocation to infer its zone" $(BUILD_DIR)/prelude/std-collections-implicit-zone-untracked.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-hash-map-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-hash-map-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-hash-map-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'map' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-hash-map-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-hash-map-entry-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-hash-map-entry-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-hash-map-entry-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'entry' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-hash-map-entry-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-hash-map-keys-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-hash-map-keys-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-hash-map-keys-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-hash-map-keys-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-hash-map-values-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-hash-map-values-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-hash-map-values-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-hash-map-values-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-hash-set-iter-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-hash-set-iter-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-hash-set-iter-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-hash-set-iter-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-hash-map-insert-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-hash-map-insert-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-hash-map-insert-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::HashMap.insert zone argument must match the hash map allocation zone" $(BUILD_DIR)/prelude/std-collections-hash-map-insert-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-tree-map-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-tree-map-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-tree-map-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'map' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-tree-map-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-tree-map-entry-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-tree-map-entry-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-tree-map-entry-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'entry' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-tree-map-entry-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-tree-map-keys-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-tree-map-keys-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-tree-map-keys-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-tree-map-keys-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-tree-map-values-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-tree-map-values-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-tree-map-values-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-tree-map-values-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-tree-set-iter-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-tree-set-iter-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-tree-set-iter-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-tree-set-iter-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-tree-set-insert-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-tree-set-insert-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-tree-set-insert-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::TreeSet.insert zone argument must match the tree set allocation zone" $(BUILD_DIR)/prelude/std-collections-tree-set-insert-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-deque-iter-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-deque-iter-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-deque-iter-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-deque-iter-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-ring-buffer-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-ring-buffer-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-ring-buffer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'buffer' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-ring-buffer-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-linked-list-iter-after-reset.ari -o $(BUILD_DIR)/prelude/std-collections-linked-list-iter-after-reset.elf >$(BUILD_DIR)/prelude/std-collections-linked-list-iter-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-collections-linked-list-iter-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-deque-push-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-deque-push-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-deque-push-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::Deque.push_back zone argument must match the deque allocation zone" $(BUILD_DIR)/prelude/std-collections-deque-push-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-linked-list-push-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-linked-list-push-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-linked-list-push-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::LinkedList.push_front zone argument must match the linked list allocation zone" $(BUILD_DIR)/prelude/std-collections-linked-list-push-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-binary-heap-push-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-binary-heap-push-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-binary-heap-push-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::BinaryHeap.push zone argument must match the binary heap allocation zone" $(BUILD_DIR)/prelude/std-collections-binary-heap-push-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/collections/std-collections-priority-queue-push-different-zone.ari -o $(BUILD_DIR)/prelude/std-collections-priority-queue-push-different-zone.elf >$(BUILD_DIR)/prelude/std-collections-priority-queue-push-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::collections::PriorityQueue.push zone argument must match the priority queue allocation zone" $(BUILD_DIR)/prelude/std-collections-priority-queue-push-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/vec/prelude-slice-copy-to-after-target-reset.ari -o $(BUILD_DIR)/prelude/prelude-slice-copy-to-after-target-reset.elf >$(BUILD_DIR)/prelude/prelude-slice-copy-to-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/prelude-slice-copy-to-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/prelude-slice-as-ptr-after-reset.ari -o $(BUILD_DIR)/prelude/prelude-slice-as-ptr-after-reset.elf >$(BUILD_DIR)/prelude/prelude-slice-as-ptr-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/prelude-slice-as-ptr-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-as-ptr-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-as-ptr-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-as-ptr-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-as-ptr-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-as-mut-ptr-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-as-mut-ptr-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-as-mut-ptr-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-as-mut-ptr-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/vec/std-vec-iter-after-reset.ari -o $(BUILD_DIR)/prelude/std-vec-iter-after-reset.elf >$(BUILD_DIR)/prelude/std-vec-iter-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'cursor' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-vec-iter-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-box-after-reset.ari -o $(BUILD_DIR)/prelude/std-boxed-box-after-reset.elf >$(BUILD_DIR)/prelude/std-boxed-box-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'boxed' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-boxed-box-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-as-ptr-after-reset.ari -o $(BUILD_DIR)/prelude/std-boxed-as-ptr-after-reset.elf >$(BUILD_DIR)/prelude/std-boxed-as-ptr-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-boxed-as-ptr-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-as-mut-ptr-after-reset.ari -o $(BUILD_DIR)/prelude/std-boxed-as-mut-ptr-after-reset.elf >$(BUILD_DIR)/prelude/std-boxed-as-mut-ptr-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-boxed-as-mut-ptr-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-as-ref-mut-conflict.ari -o $(BUILD_DIR)/prelude/std-boxed-as-ref-mut-conflict.elf >$(BUILD_DIR)/prelude/std-boxed-as-ref-mut-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow already borrowed binding 'boxed'" $(BUILD_DIR)/prelude/std-boxed-as-ref-mut-conflict.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-copy-to-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-boxed-copy-to-after-target-reset.elf >$(BUILD_DIR)/prelude/std-boxed-copy-to-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-boxed-copy-to-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-drop-use-after.ari -o $(BUILD_DIR)/prelude/std-boxed-drop-use-after.elf >$(BUILD_DIR)/prelude/std-boxed-drop-use-after.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'boxed'" $(BUILD_DIR)/prelude/std-boxed-drop-use-after.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-put-in-different-zone.ari -o $(BUILD_DIR)/prelude/std-boxed-put-in-different-zone.elf >$(BUILD_DIR)/prelude/std-boxed-put-in-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::boxed::Box.put_in zone argument must match the box allocation zone" $(BUILD_DIR)/prelude/std-boxed-put-in-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/std-boxed-put-in-untracked.ari -o $(BUILD_DIR)/prelude/std-boxed-put-in-untracked.elf >$(BUILD_DIR)/prelude/std-boxed-put-in-untracked.err 2>&1; then exit 1; fi
	grep -q "std::boxed::Box.put_in receiver must come from a tracked zone allocation" $(BUILD_DIR)/prelude/std-boxed-put-in-untracked.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-alloc-buffer-after-reset.ari -o $(BUILD_DIR)/prelude/std-string-alloc-buffer-after-reset.elf >$(BUILD_DIR)/prelude/std-string-alloc-buffer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'data' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-alloc-buffer-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-handle-after-reset.ari -o $(BUILD_DIR)/prelude/std-string-handle-after-reset.elf >$(BUILD_DIR)/prelude/std-string-handle-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-handle-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-as-ptr-after-reset.ari -o $(BUILD_DIR)/prelude/std-string-as-ptr-after-reset.elf >$(BUILD_DIR)/prelude/std-string-as-ptr-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'raw' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-as-ptr-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-copy-to-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-string-copy-to-after-target-reset.elf >$(BUILD_DIR)/prelude/std-string-copy-to-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-copy-to-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-copy-to-method-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-string-copy-to-method-after-target-reset.elf >$(BUILD_DIR)/prelude/std-string-copy-to-method-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-copy-to-method-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-from-slice-in-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-string-from-slice-in-after-target-reset.elf >$(BUILD_DIR)/prelude/std-string-from-slice-in-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-from-slice-in-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-trim-to-after-target-reset.ari -o $(BUILD_DIR)/prelude/std-string-trim-to-after-target-reset.elf >$(BUILD_DIR)/prelude/std-string-trim-to-after-target-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'copied' after zone 'target' was reset or destroyed" $(BUILD_DIR)/prelude/std-string-trim-to-after-target-reset.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-push-in-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-push-in-different-zone.elf >$(BUILD_DIR)/prelude/std-string-push-in-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.push_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-push-in-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-implicit-zone-untracked.ari -o $(BUILD_DIR)/prelude/std-string-implicit-zone-untracked.elf >$(BUILD_DIR)/prelude/std-string-implicit-zone-untracked.err 2>&1; then exit 1; fi
	grep -q "std::string::String.reserve receiver must come from a tracked zone allocation to infer its zone" $(BUILD_DIR)/prelude/std-string-implicit-zone-untracked.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-append-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-append-different-zone.elf >$(BUILD_DIR)/prelude/std-string-append-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.append_i64_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-append-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-append-value-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-append-value-different-zone.elf >$(BUILD_DIR)/prelude/std-string-append-value-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.append_value_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-append-value-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-append-debug-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-append-debug-different-zone.elf >$(BUILD_DIR)/prelude/std-string-append-debug-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.append_debug_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-append-debug-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-append-f64-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-append-f64-different-zone.elf >$(BUILD_DIR)/prelude/std-string-append-f64-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.append_f64_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-append-f64-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-append-f32-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-append-f32-different-zone.elf >$(BUILD_DIR)/prelude/std-string-append-f32-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.append_f32_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-append-f32-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-append-u64-different-zone.ari -o $(BUILD_DIR)/prelude/std-string-append-u64-different-zone.elf >$(BUILD_DIR)/prelude/std-string-append-u64-different-zone.err 2>&1; then exit 1; fi
	grep -q "std::string::String.append_u64_in zone argument must match the string allocation zone" $(BUILD_DIR)/prelude/std-string-append-u64-different-zone.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-literal-array-length.ari -o $(BUILD_DIR)/prelude/std-string-literal-array-length.elf >$(BUILD_DIR)/prelude/std-string-literal-array-length.err 2>&1; then exit 1; fi
	grep -q "string literal has 3 bytes but array type expects 2" $(BUILD_DIR)/prelude/std-string-literal-array-length.err
	if $(TARGET) tests/cases/standard-library/errors/string/std-string-literal-utf8-invalid.ari -o $(BUILD_DIR)/prelude/std-string-literal-utf8-invalid.elf >$(BUILD_DIR)/prelude/std-string-literal-utf8-invalid.err 2>&1; then exit 1; fi
	grep -q "string literal is not valid UTF-8 for std::string::Utf8" $(BUILD_DIR)/prelude/std-string-literal-utf8-invalid.err
	if $(TARGET) tests/cases/standard-library/errors/string/prelude-string-type-args.ari -o $(BUILD_DIR)/prelude/prelude-string-type-args.elf >$(BUILD_DIR)/prelude/prelude-string-type-args.err 2>&1; then exit 1; fi
	grep -q "struct 'std::string::String' expects 0 type arguments" $(BUILD_DIR)/prelude/prelude-string-type-args.err
	if $(TARGET) tests/cases/standard-library/errors/string/prelude-string-root-after-reset.ari -o $(BUILD_DIR)/prelude/prelude-string-root-after-reset.elf >$(BUILD_DIR)/prelude/prelude-string-root-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'text' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/prelude-string-root-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/boxed/prelude-box-root-after-reset.ari -o $(BUILD_DIR)/prelude/prelude-box-root-after-reset.elf >$(BUILD_DIR)/prelude/prelude-box-root-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'boxed' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/prelude-box-root-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-unique-reserved.ari -o $(BUILD_DIR)/prelude/prelude-unique-reserved.elf >$(BUILD_DIR)/prelude/prelude-unique-reserved.err 2>&1; then exit 1; fi
	grep -q "prelude type 'Unique' is reserved for smart-pointer policy; use Box\\[T\\] for today's explicit-zone handle" $(BUILD_DIR)/prelude/prelude-unique-reserved.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-shared-reserved.ari -o $(BUILD_DIR)/prelude/prelude-shared-reserved.elf >$(BUILD_DIR)/prelude/prelude-shared-reserved.err 2>&1; then exit 1; fi
	grep -q "prelude type 'Shared' is reserved for future reference-counted ownership" $(BUILD_DIR)/prelude/prelude-shared-reserved.err
	if $(TARGET) tests/cases/standard-library/errors/cmp/std-cmp-equality-operator-missing-eq.ari -o $(BUILD_DIR)/prelude/std-cmp-equality-operator-missing-eq.elf >$(BUILD_DIR)/prelude/std-cmp-equality-operator-missing-eq.err 2>&1; then exit 1; fi
	grep -q "unknown method 'eq' for type Score" $(BUILD_DIR)/prelude/std-cmp-equality-operator-missing-eq.err
	if $(TARGET) tests/cases/standard-library/errors/cmp/std-cmp-order-operator-missing-ord.ari -o $(BUILD_DIR)/prelude/std-cmp-order-operator-missing-ord.elf >$(BUILD_DIR)/prelude/std-cmp-order-operator-missing-ord.err 2>&1; then exit 1; fi
	grep -q "unknown method 'lt' for type Score" $(BUILD_DIR)/prelude/std-cmp-order-operator-missing-ord.err
	if $(TARGET) --no-implicit-std tests/cases/standard-library/errors/prelude/prelude-special-without-std.ari -o $(BUILD_DIR)/prelude/prelude-special-without-std.elf >$(BUILD_DIR)/prelude/prelude-special-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'size_of'" $(BUILD_DIR)/prelude/prelude-special-without-std.err
	if $(TARGET) --no-implicit-std --module-path tests/fixtures/std-missing-helper tests/cases/standard-library/errors/prelude/prelude-special-missing-decl.ari -o $(BUILD_DIR)/prelude/prelude-special-missing-decl.elf >$(BUILD_DIR)/prelude/prelude-special-missing-decl.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'std::mem::size_of'" $(BUILD_DIR)/prelude/prelude-special-missing-decl.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-take-non-place.ari -o $(BUILD_DIR)/prelude/prelude-take-non-place.elf >$(BUILD_DIR)/prelude/prelude-take-non-place.err 2>&1; then exit 1; fi
	grep -q "take expects a local binding, field, tuple index, or index expression" $(BUILD_DIR)/prelude/prelude-take-non-place.err
	if $(TARGET) tests/cases/standard-library/errors/algo/std-algo-copy-own.ari -o $(BUILD_DIR)/prelude/std-algo-copy-own.elf >$(BUILD_DIR)/prelude/std-algo-copy-own.err 2>&1; then exit 1; fi
	grep -q "as_slice currently supports copyable element arrays and vectors only" $(BUILD_DIR)/prelude/std-algo-copy-own.err
	if $(TARGET) tests/cases/standard-library/errors/mem/std-mem-replace-own.ari -o $(BUILD_DIR)/prelude/std-mem-replace-own.elf >$(BUILD_DIR)/prelude/std-mem-replace-own.err 2>&1; then exit 1; fi
	grep -q "std::mem::replace cannot copy ownership- or borrow-valued values through raw pointers yet" $(BUILD_DIR)/prelude/std-mem-replace-own.err
	if $(TARGET) tests/cases/standard-library/errors/mem/std-mem-swap-own.ari -o $(BUILD_DIR)/prelude/std-mem-swap-own.elf >$(BUILD_DIR)/prelude/std-mem-swap-own.err 2>&1; then exit 1; fi
	grep -q "std::mem::swap cannot copy ownership- or borrow-valued values through raw pointers yet" $(BUILD_DIR)/prelude/std-mem-swap-own.err
	if $(TARGET) tests/cases/standard-library/errors/mem/std-mem-pointer-op-after-reset.ari -o $(BUILD_DIR)/prelude/std-mem-pointer-op-after-reset.elf >$(BUILD_DIR)/prelude/std-mem-pointer-op-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'shifted' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/std-mem-pointer-op-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-move-borrow-result.ari -o $(BUILD_DIR)/prelude/prelude-move-borrow-result.elf >$(BUILD_DIR)/prelude/prelude-move-borrow-result.err 2>&1; then exit 1; fi
	grep -q "move cannot return borrow-valued expressions" $(BUILD_DIR)/prelude/prelude-move-borrow-result.err
	if $(TARGET) --no-implicit-std tests/cases/standard-library/errors/prelude/prelude-trait-without-std.ari -o $(BUILD_DIR)/prelude/prelude-trait-without-std.elf >$(BUILD_DIR)/prelude/prelude-trait-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'Debug'" $(BUILD_DIR)/prelude/prelude-trait-without-std.err
	if $(TARGET) tests/cases/standard-library/errors/runtime/ari-builtin-unknown.ari -o $(BUILD_DIR)/prelude/ari-builtin-unknown.elf >$(BUILD_DIR)/prelude/ari-builtin-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown Ari builtin symbol 'puts'" $(BUILD_DIR)/prelude/ari-builtin-unknown.err
	if $(TARGET) tests/cases/standard-library/errors/runtime/ari-builtin-missing-link.ari -o $(BUILD_DIR)/prelude/ari-builtin-missing-link.elf >$(BUILD_DIR)/prelude/ari-builtin-missing-link.err 2>&1; then exit 1; fi
	grep -q 'extern "ari" builtin declarations require an explicit builtin symbol' $(BUILD_DIR)/prelude/ari-builtin-missing-link.err
	if $(TARGET) tests/cases/standard-library/errors/runtime/ari-builtin-signature-arity.ari -o $(BUILD_DIR)/prelude/ari-builtin-signature-arity.elf >$(BUILD_DIR)/prelude/ari-builtin-signature-arity.err 2>&1; then exit 1; fi
	grep -q "Ari builtin 'ari_builtin_write_i64' expects 1 parameter, got 0" $(BUILD_DIR)/prelude/ari-builtin-signature-arity.err
	if $(TARGET) tests/cases/standard-library/errors/runtime/ari-builtin-signature-param.ari -o $(BUILD_DIR)/prelude/ari-builtin-signature-param.elf >$(BUILD_DIR)/prelude/ari-builtin-signature-param.err 2>&1; then exit 1; fi
	grep -q "Ari builtin 'ari_builtin_write_i64' parameter 1 must be i64, got u64" $(BUILD_DIR)/prelude/ari-builtin-signature-param.err
	if $(TARGET) tests/cases/standard-library/errors/runtime/ari-builtin-signature-result.ari -o $(BUILD_DIR)/prelude/ari-builtin-signature-result.elf >$(BUILD_DIR)/prelude/ari-builtin-signature-result.err 2>&1; then exit 1; fi
	grep -q "Ari builtin 'ari_builtin_write_i64' return type must be i64, got u64" $(BUILD_DIR)/prelude/ari-builtin-signature-result.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-result-arity.ari -o $(BUILD_DIR)/prelude/prelude-result-arity.elf >$(BUILD_DIR)/prelude/prelude-result-arity.err 2>&1; then exit 1; fi
	grep -q "enum 'std::Result' expects 2 type arguments" $(BUILD_DIR)/prelude/prelude-result-arity.err
	if $(TARGET) tests/cases/standard-library/errors/input/prelude-read-line-owned-after-reset.ari -o $(BUILD_DIR)/prelude/prelude-read-line-owned-after-reset.elf >$(BUILD_DIR)/prelude/prelude-read-line-owned-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'text' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/prelude/prelude-read-line-owned-after-reset.err
	if $(TARGET) tests/cases/standard-library/errors/format/prelude-macro-format-no-default-zone.ari -o $(BUILD_DIR)/prelude/prelude-macro-format-no-default-zone.elf >$(BUILD_DIR)/prelude/prelude-macro-format-no-default-zone.err 2>&1; then exit 1; fi
	grep -q "prelude macro 'format!' has no implicit allocation zone; use format_in!(ref mut zone, ...) or call it inside a zone block" $(BUILD_DIR)/prelude/prelude-macro-format-no-default-zone.err
	if $(TARGET) tests/cases/standard-library/errors/format/prelude-macro-format-qualified-no-default-zone.ari -o $(BUILD_DIR)/prelude/prelude-macro-format-qualified-no-default-zone.elf >$(BUILD_DIR)/prelude/prelude-macro-format-qualified-no-default-zone.err 2>&1; then exit 1; fi
	grep -q "prelude macro 'format!' has no implicit allocation zone; use format_in!(ref mut zone, ...) or call it inside a zone block" $(BUILD_DIR)/prelude/prelude-macro-format-qualified-no-default-zone.err
	if $(TARGET) tests/cases/standard-library/errors/format/prelude-format-in-f128.ari -o $(BUILD_DIR)/prelude/prelude-format-in-f128.elf >$(BUILD_DIR)/prelude/prelude-format-in-f128.err 2>&1; then exit 1; fi
	grep -q "format_in! currently supports string, char, integer, bool, f32, f64, and Display values, got f128" $(BUILD_DIR)/prelude/prelude-format-in-f128.err
	if $(TARGET) tests/cases/standard-library/errors/format/prelude-format-in-no-display.ari -o $(BUILD_DIR)/prelude/prelude-format-in-no-display.elf >$(BUILD_DIR)/prelude/prelude-format-in-no-display.err 2>&1; then exit 1; fi
	grep -q "format_in! currently supports string, char, integer, bool, f32, f64, and Display values, got Point" $(BUILD_DIR)/prelude/prelude-format-in-no-display.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-macro-nonprelude-qualified.ari -o $(BUILD_DIR)/prelude/prelude-macro-nonprelude-qualified.elf >$(BUILD_DIR)/prelude/prelude-macro-nonprelude-qualified.err 2>&1; then exit 1; fi
	grep -q "unknown macro 'other::format!'" $(BUILD_DIR)/prelude/prelude-macro-nonprelude-qualified.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-macro-nonprelude-matches.ari -o $(BUILD_DIR)/prelude/prelude-macro-nonprelude-matches.elf >$(BUILD_DIR)/prelude/prelude-macro-nonprelude-matches.err 2>&1; then exit 1; fi
	grep -q "unknown macro 'other::matches!'" $(BUILD_DIR)/prelude/prelude-macro-nonprelude-matches.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-macro-assert-eq-type.ari -o $(BUILD_DIR)/prelude/prelude-macro-assert-eq-type.elf >$(BUILD_DIR)/prelude/prelude-macro-assert-eq-type.err 2>&1; then exit 1; fi
	grep -q "assert_eq! arguments must both be integer values or both be bool values" $(BUILD_DIR)/prelude/prelude-macro-assert-eq-type.err
	if $(TARGET) tests/cases/standard-library/errors/prelude/prelude-assert-type.ari -o $(BUILD_DIR)/prelude/prelude-assert-type.elf >$(BUILD_DIR)/prelude/prelude-assert-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i64" $(BUILD_DIR)/prelude/prelude-assert-type.err

check-core-language: $(TARGET)
	mkdir -p $(BUILD_DIR)/core-language
	$(TARGET) tests/cases/core-language/ok/core-language-readiness.ari --check
	$(TARGET) tests/cases/core-language/ok/core-language-readiness.ari -o $(BUILD_DIR)/core-language/core-language-readiness.elf
	$(BUILD_DIR)/core-language/core-language-readiness.elf; code=$$?; test $$code -eq 115
	$(TARGET) tests/cases/core-language/ok/core-language-readiness.ari --emit-llvm $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q 'define i64 @"_ARNv3add"' $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q 'define i64 @"_ARNv7combine"' $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q 'call i64 @"_ARNv4fact"' $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q "while.cond" $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q "for.cond" $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q "srem i64" $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q "shl i64" $(BUILD_DIR)/core-language/core-language-readiness.ll
	grep -q "icmp" $(BUILD_DIR)/core-language/core-language-readiness.ll
	if $(TARGET) tests/cases/functions/errors/function-wrong-arg-count.ari -o $(BUILD_DIR)/core-language/function-wrong-arg-count.elf >$(BUILD_DIR)/core-language/function-wrong-arg-count.err 2>&1; then exit 1; fi
	grep -q "wrong argument count for 'add'" $(BUILD_DIR)/core-language/function-wrong-arg-count.err
	if $(TARGET) tests/cases/functions/errors/function-arg-type-mismatch.ari -o $(BUILD_DIR)/core-language/function-arg-type-mismatch.elf >$(BUILD_DIR)/core-language/function-arg-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/core-language/function-arg-type-mismatch.err
	if $(TARGET) tests/cases/functions/errors/function-missing-return.ari -o $(BUILD_DIR)/core-language/function-missing-return.elf >$(BUILD_DIR)/core-language/function-missing-return.err 2>&1; then exit 1; fi
	grep -q "function 'main' must return a value on all paths" $(BUILD_DIR)/core-language/function-missing-return.err
	if $(TARGET) tests/cases/functions/errors/function-return-type-mismatch.ari -o $(BUILD_DIR)/core-language/function-return-type-mismatch.elf >$(BUILD_DIR)/core-language/function-return-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/core-language/function-return-type-mismatch.err
	if $(TARGET) tests/cases/variables/errors/variables-assign-immutable.ari -o $(BUILD_DIR)/core-language/variables-assign-immutable.elf >$(BUILD_DIR)/core-language/variables-assign-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'value'" $(BUILD_DIR)/core-language/variables-assign-immutable.err
	if $(TARGET) tests/cases/variables/errors/variables-assign-type-mismatch.ari -o $(BUILD_DIR)/core-language/variables-assign-type-mismatch.elf >$(BUILD_DIR)/core-language/variables-assign-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i32, got bool" $(BUILD_DIR)/core-language/variables-assign-type-mismatch.err
	if $(TARGET) tests/cases/core-language/errors/core-use-before-declaration.ari -o $(BUILD_DIR)/core-language/core-use-before-declaration.elf >$(BUILD_DIR)/core-language/core-use-before-declaration.err 2>&1; then exit 1; fi
	grep -q "unknown name 'later'" $(BUILD_DIR)/core-language/core-use-before-declaration.err
	if $(TARGET) tests/cases/core-language/errors/core-invalid-binary-operator.ari -o $(BUILD_DIR)/core-language/core-invalid-binary-operator.elf >$(BUILD_DIR)/core-language/core-invalid-binary-operator.err 2>&1; then exit 1; fi
	grep -q "numeric operands must have the same numeric type, got bool and bool" $(BUILD_DIR)/core-language/core-invalid-binary-operator.err
	if $(TARGET) tests/cases/core-language/errors/core-invalid-unary-minus.ari -o $(BUILD_DIR)/core-language/core-invalid-unary-minus.elf >$(BUILD_DIR)/core-language/core-invalid-unary-minus.err 2>&1; then exit 1; fi
	grep -q "numeric operands must have the same numeric type, got i64 and bool" $(BUILD_DIR)/core-language/core-invalid-unary-minus.err
	if $(TARGET) tests/cases/operators/errors/invalid-cast.ari -o $(BUILD_DIR)/core-language/invalid-cast.elf >$(BUILD_DIR)/core-language/invalid-cast.err 2>&1; then exit 1; fi
	grep -q "explicit casts currently require integer types, float types, integer/float casts, or raw pointer casts, got bool as i64" $(BUILD_DIR)/core-language/invalid-cast.err
	if $(TARGET) tests/cases/control-flow/errors/break-outside-loop.ari -o $(BUILD_DIR)/core-language/break-outside-loop.elf >$(BUILD_DIR)/core-language/break-outside-loop.err 2>&1; then exit 1; fi
	grep -q "break used outside a loop" $(BUILD_DIR)/core-language/break-outside-loop.err
	if $(TARGET) tests/cases/control-flow/errors/continue-outside-loop.ari -o $(BUILD_DIR)/core-language/continue-outside-loop.elf >$(BUILD_DIR)/core-language/continue-outside-loop.err 2>&1; then exit 1; fi
	grep -q "continue used outside a loop" $(BUILD_DIR)/core-language/continue-outside-loop.err
	if $(TARGET) tests/cases/control-flow/errors/if-expression-type-mismatch.ari -o $(BUILD_DIR)/core-language/if-expression-type-mismatch.elf >$(BUILD_DIR)/core-language/if-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/core-language/if-expression-type-mismatch.err

check-control-flow: $(TARGET)
	mkdir -p $(BUILD_DIR)/control-flow
	$(TARGET) tests/cases/control-flow/ok/control-flow-if-else-if.ari -o $(BUILD_DIR)/control-flow/control-flow-if-else-if.elf
	$(BUILD_DIR)/control-flow/control-flow-if-else-if.elf; code=$$?; test $$code -eq 3
	$(TARGET) tests/cases/control-flow/ok/control-flow-boolish.ari -o $(BUILD_DIR)/control-flow/control-flow-boolish.elf
	$(BUILD_DIR)/control-flow/control-flow-boolish.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/control-flow/ok/if-expression.ari -o $(BUILD_DIR)/control-flow/if-expression.elf
	$(BUILD_DIR)/control-flow/if-expression.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/control-flow/ok/noreturn-expression-arms.ari -o $(BUILD_DIR)/control-flow/noreturn-expression-arms.elf
	$(BUILD_DIR)/control-flow/noreturn-expression-arms.elf; code=$$?; test $$code -eq 84
	$(TARGET) tests/cases/control-flow/ok/if-expression-payloads.ari -o $(BUILD_DIR)/control-flow/if-expression-payloads.elf
	$(BUILD_DIR)/control-flow/if-expression-payloads.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/control-flow/ok/if-let-expression.ari -o $(BUILD_DIR)/control-flow/if-let-expression.elf
	$(BUILD_DIR)/control-flow/if-let-expression.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/control-flow/ok/if-let-or-pattern.ari -o $(BUILD_DIR)/control-flow/if-let-or-pattern.elf
	$(BUILD_DIR)/control-flow/if-let-or-pattern.elf; code=$$?; test $$code -eq 44
	$(TARGET) tests/cases/control-flow/ok/if-let-aggregate-pattern.ari -o $(BUILD_DIR)/control-flow/if-let-aggregate-pattern.elf
	$(BUILD_DIR)/control-flow/if-let-aggregate-pattern.elf; code=$$?; test $$code -eq 17
	$(TARGET) tests/cases/control-flow/ok/if-let-aggregate-or-pattern.ari -o $(BUILD_DIR)/control-flow/if-let-aggregate-or-pattern.elf
	$(BUILD_DIR)/control-flow/if-let-aggregate-or-pattern.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/control-flow/ok/block-expression.ari -o $(BUILD_DIR)/control-flow/block-expression.elf
	$(BUILD_DIR)/control-flow/block-expression.elf; code=$$?; test $$code -eq 27
	$(TARGET) tests/cases/control-flow/ok/block-expression-payloads.ari -o $(BUILD_DIR)/control-flow/block-expression-payloads.elf
	$(BUILD_DIR)/control-flow/block-expression-payloads.elf; code=$$?; test $$code -eq 26
	$(TARGET) tests/cases/control-flow/ok/statement-body-payloads.ari -o $(BUILD_DIR)/control-flow/statement-body-payloads.elf
	$(BUILD_DIR)/control-flow/statement-body-payloads.elf; code=$$?; test $$code -eq 19
	$(TARGET) tests/cases/control-flow/ok/labeled-block-expression.ari -o $(BUILD_DIR)/control-flow/labeled-block-expression.elf
	$(BUILD_DIR)/control-flow/labeled-block-expression.elf; code=$$?; test $$code -eq 17
	$(TARGET) tests/cases/control-flow/ok/labeled-block-aggregate.ari -o $(BUILD_DIR)/control-flow/labeled-block-aggregate.elf
	$(BUILD_DIR)/control-flow/labeled-block-aggregate.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/control-flow/ok/labeled-block-own-break.ari -o $(BUILD_DIR)/control-flow/labeled-block-own-break.elf
	$(BUILD_DIR)/control-flow/labeled-block-own-break.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/control-flow/ok/control-flow-aggregate-expr-llvm.ari -o $(BUILD_DIR)/control-flow/control-flow-aggregate-expr-llvm.elf
	$(BUILD_DIR)/control-flow/control-flow-aggregate-expr-llvm.elf; code=$$?; test $$code -eq 82
	$(TARGET) tests/cases/control-flow/ok/for-range-operator.ari -o $(BUILD_DIR)/control-flow/for-range-operator.elf
	$(BUILD_DIR)/control-flow/for-range-operator.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/control-flow/ok/for-range-inclusive.ari -o $(BUILD_DIR)/control-flow/for-range-inclusive.elf
	$(BUILD_DIR)/control-flow/for-range-inclusive.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/control-flow/ok/for-range-exact-once-owner-state.ari -o $(BUILD_DIR)/control-flow/for-range-exact-once-owner-state.elf
	$(BUILD_DIR)/control-flow/for-range-exact-once-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/for-range-known-nonempty-break-owner-state.ari -o $(BUILD_DIR)/control-flow/for-range-known-nonempty-break-owner-state.elf
	$(BUILD_DIR)/control-flow/for-range-known-nonempty-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/range-values.ari -o $(BUILD_DIR)/control-flow/range-values.elf
	$(BUILD_DIR)/control-flow/range-values.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/control-flow/ok/for-range-alias-pattern.ari -o $(BUILD_DIR)/control-flow/for-range-alias-pattern.elf
	$(BUILD_DIR)/control-flow/for-range-alias-pattern.elf; code=$$?; test $$code -eq 28
	$(TARGET) tests/cases/generics/ok/range-generic-bounds.ari -o $(BUILD_DIR)/control-flow/range-generic-bounds.elf
	$(BUILD_DIR)/control-flow/range-generic-bounds.elf; code=$$?; test $$code -eq 32
	$(TARGET) tests/cases/control-flow/ok/for-vector-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-pattern.elf
	$(BUILD_DIR)/control-flow/for-vector-pattern.elf; code=$$?; test $$code -eq 50
	$(TARGET) tests/cases/control-flow/ok/for-vector-alias-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-alias-pattern.elf
	$(BUILD_DIR)/control-flow/for-vector-alias-pattern.elf; code=$$?; test $$code -eq 75
	$(TARGET) tests/cases/control-flow/ok/for-vector-exact-once-owner-state.ari -o $(BUILD_DIR)/control-flow/for-vector-exact-once-owner-state.elf
	$(BUILD_DIR)/control-flow/for-vector-exact-once-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/for-vector-known-nonempty-break-owner-state.ari -o $(BUILD_DIR)/control-flow/for-vector-known-nonempty-break-owner-state.elf
	$(BUILD_DIR)/control-flow/for-vector-known-nonempty-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/ir/ok/ir-for-loop-payloads.ari -o $(BUILD_DIR)/control-flow/ir-for-loop-payloads.elf
	$(BUILD_DIR)/control-flow/ir-for-loop-payloads.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/control-flow/ok/if-while-let.ari -o $(BUILD_DIR)/control-flow/if-while-let.elf
	$(BUILD_DIR)/control-flow/if-while-let.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/control-flow/ok/while-let-or-pattern.ari -o $(BUILD_DIR)/control-flow/while-let-or-pattern.elf
	$(BUILD_DIR)/control-flow/while-let-or-pattern.elf; code=$$?; test $$code -eq 55
	$(TARGET) tests/cases/control-flow/ok/while-let-aggregate-or-pattern.ari -o $(BUILD_DIR)/control-flow/while-let-aggregate-or-pattern.elf
	$(BUILD_DIR)/control-flow/while-let-aggregate-or-pattern.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/control-flow/ok/while-let-aggregate-break-owner-state.ari -o $(BUILD_DIR)/control-flow/while-let-aggregate-break-owner-state.elf
	$(BUILD_DIR)/control-flow/while-let-aggregate-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-let-direct-enum-owner-state.ari -o $(BUILD_DIR)/control-flow/while-let-direct-enum-owner-state.elf
	$(BUILD_DIR)/control-flow/while-let-direct-enum-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-let-local-enum-owner-state.ari -o $(BUILD_DIR)/control-flow/while-let-local-enum-owner-state.elf
	$(BUILD_DIR)/control-flow/while-let-local-enum-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-let-local-alias-enum-owner-state.ari -o $(BUILD_DIR)/control-flow/while-let-local-alias-enum-owner-state.elf
	$(BUILD_DIR)/control-flow/while-let-local-alias-enum-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/init-while-owning.ari -o $(BUILD_DIR)/control-flow/init-while-owning.elf
	$(BUILD_DIR)/control-flow/init-while-owning.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/control-flow/ok/init-while-false-owner-state.ari -o $(BUILD_DIR)/control-flow/init-while-false-owner-state.elf
	$(BUILD_DIR)/control-flow/init-while-false-owner-state.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/control-flow/ok/init-while-true-return.ari -o $(BUILD_DIR)/control-flow/init-while-true-return.elf
	$(BUILD_DIR)/control-flow/init-while-true-return.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/control-flow/ok/init-while-true-continue-break-owner-state.ari -o $(BUILD_DIR)/control-flow/init-while-true-continue-break-owner-state.elf
	$(BUILD_DIR)/control-flow/init-while-true-continue-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/init-while-known-true-continue-break-owner-state.ari -o $(BUILD_DIR)/control-flow/init-while-known-true-continue-break-owner-state.elf
	$(BUILD_DIR)/control-flow/init-while-known-true-continue-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/init-while-known-bool-alias-owner-state.ari -o $(BUILD_DIR)/control-flow/init-while-known-bool-alias-owner-state.elf
	$(BUILD_DIR)/control-flow/init-while-known-bool-alias-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/labeled-break.ari -o $(BUILD_DIR)/control-flow/labeled-break.elf
	$(BUILD_DIR)/control-flow/labeled-break.elf; code=$$?; test $$code -eq 2
	$(TARGET) tests/cases/control-flow/ok/break-statement-payloads.ari -o $(BUILD_DIR)/control-flow/break-statement-payloads.elf
	$(BUILD_DIR)/control-flow/break-statement-payloads.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/control-flow/ok/while-true-break-owner-state.ari -o $(BUILD_DIR)/control-flow/while-true-break-owner-state.elf
	$(BUILD_DIR)/control-flow/while-true-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-known-true-break-owner-state.ari -o $(BUILD_DIR)/control-flow/while-known-true-break-owner-state.elf
	$(BUILD_DIR)/control-flow/while-known-true-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-known-bool-alias-owner-state.ari -o $(BUILD_DIR)/control-flow/while-known-bool-alias-owner-state.elf
	$(BUILD_DIR)/control-flow/while-known-bool-alias-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/loop-break-owner-unavailable-merge.ari -o $(BUILD_DIR)/control-flow/loop-break-owner-unavailable-merge.elf
	$(BUILD_DIR)/control-flow/loop-break-owner-unavailable-merge.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/control-flow/ok/loop-break-borrow-release-merge.ari -o $(BUILD_DIR)/control-flow/loop-break-borrow-release-merge.elf
	$(BUILD_DIR)/control-flow/loop-break-borrow-release-merge.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/control-flow/ok/loop-fallthrough-borrow-release-merge.ari -o $(BUILD_DIR)/control-flow/loop-fallthrough-borrow-release-merge.elf
	$(BUILD_DIR)/control-flow/loop-fallthrough-borrow-release-merge.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/control-flow/ok/loop-continue-borrow-release-merge.ari -o $(BUILD_DIR)/control-flow/loop-continue-borrow-release-merge.elf
	$(BUILD_DIR)/control-flow/loop-continue-borrow-release-merge.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/control-flow/ok/while-true-continue-break-owner-state.ari -o $(BUILD_DIR)/control-flow/while-true-continue-break-owner-state.elf
	$(BUILD_DIR)/control-flow/while-true-continue-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-true-return-owner-state.ari -o $(BUILD_DIR)/control-flow/while-true-return-owner-state.elf
	$(BUILD_DIR)/control-flow/while-true-return-owner-state.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/control-flow/ok/while-true-literal-if-break-owner-state.ari -o $(BUILD_DIR)/control-flow/while-true-literal-if-break-owner-state.elf
	$(BUILD_DIR)/control-flow/while-true-literal-if-break-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-false-owner-state.ari -o $(BUILD_DIR)/control-flow/while-false-owner-state.elf
	$(BUILD_DIR)/control-flow/while-false-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-known-false-owner-state.ari -o $(BUILD_DIR)/control-flow/while-known-false-owner-state.elf
	$(BUILD_DIR)/control-flow/while-known-false-owner-state.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/control-flow/ok/while-false-borrow-state.ari -o $(BUILD_DIR)/control-flow/while-false-borrow-state.elf
	$(BUILD_DIR)/control-flow/while-false-borrow-state.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/control-flow/ok/while-true-continue-noreturn.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-continue-noreturn.ll
	grep -q "while.cond" $(BUILD_DIR)/control-flow/while-true-continue-noreturn.ll
	$(TARGET) tests/cases/control-flow/ok/while-true-empty-noreturn.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-empty-noreturn.ll
	grep -q "while.cond" $(BUILD_DIR)/control-flow/while-true-empty-noreturn.ll
	$(TARGET) tests/cases/control-flow/ok/while-true-if-false-break-noreturn.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-if-false-break-noreturn.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/while-true-if-false-break-noreturn.ll
	$(TARGET) tests/cases/control-flow/ok/statement-label-payloads.ari -o $(BUILD_DIR)/control-flow/statement-label-payloads.elf
	$(BUILD_DIR)/control-flow/statement-label-payloads.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/control-flow/ok/labeled-block.ari -o $(BUILD_DIR)/control-flow/labeled-block.elf
	$(BUILD_DIR)/control-flow/labeled-block.elf; code=$$?; test $$code -eq 2
	$(TARGET) tests/cases/control-flow/ok/if-while-let.ari --emit-llvm $(BUILD_DIR)/control-flow/if-while-let.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/if-while-let.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-while-let.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-or-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-or-pattern.ll
	grep -q "whilelet.arm" $(BUILD_DIR)/control-flow/while-let-or-pattern.ll
	grep -q "whilelet.test" $(BUILD_DIR)/control-flow/while-let-or-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-aggregate-or-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-aggregate-or-pattern.ll
	grep -q "while.cond" $(BUILD_DIR)/control-flow/while-let-aggregate-or-pattern.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/while-let-aggregate-or-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-aggregate-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-aggregate-break-owner-state.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/while-let-aggregate-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-direct-enum-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-direct-enum-owner-state.ll
	grep -q "whilelet.end" $(BUILD_DIR)/control-flow/while-let-direct-enum-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-local-enum-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-local-enum-owner-state.ll
	grep -q "whilelet.end" $(BUILD_DIR)/control-flow/while-let-local-enum-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-local-alias-enum-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-local-alias-enum-owner-state.ll
	grep -q "whilelet.end" $(BUILD_DIR)/control-flow/while-let-local-alias-enum-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-static-payload-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-static-payload-owner-state.ll
	grep -q "whilelet.end" $(BUILD_DIR)/control-flow/while-let-static-payload-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-static-nested-payload-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-static-nested-payload-owner-state.ll
	grep -q "whilelet.end" $(BUILD_DIR)/control-flow/while-let-static-nested-payload-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-let-enum-ref-binding-mode.ari -o $(BUILD_DIR)/control-flow/while-let-enum-ref-binding-mode.elf
	$(BUILD_DIR)/control-flow/while-let-enum-ref-binding-mode.elf; code=$$?; test $$code -eq 28
	$(TARGET) tests/cases/control-flow/ok/while-let-enum-ref-or-binding-mode.ari -o $(BUILD_DIR)/control-flow/while-let-enum-ref-or-binding-mode.elf
	$(BUILD_DIR)/control-flow/while-let-enum-ref-or-binding-mode.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/control-flow/ok/while-let-enum-ref-mut-binding-mode.ari -o $(BUILD_DIR)/control-flow/while-let-enum-ref-mut-binding-mode.elf
	$(BUILD_DIR)/control-flow/while-let-enum-ref-mut-binding-mode.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/control-flow/ok/control-flow-ref-mut-enum-payload.ari -o $(BUILD_DIR)/control-flow/control-flow-ref-mut-enum-payload.elf
	$(BUILD_DIR)/control-flow/control-flow-ref-mut-enum-payload.elf; code=$$?; test $$code -eq 190
	$(TARGET) tests/cases/control-flow/ok/control-flow-ref-mut-aggregate-binding-mode.ari -o $(BUILD_DIR)/control-flow/control-flow-ref-mut-aggregate-binding-mode.elf
	$(BUILD_DIR)/control-flow/control-flow-ref-mut-aggregate-binding-mode.elf; code=$$?; test $$code -eq 132
	$(TARGET) tests/cases/control-flow/ok/control-flow-ref-mut-runtime-sequence-binding-mode.ari -o $(BUILD_DIR)/control-flow/control-flow-ref-mut-runtime-sequence-binding-mode.elf
	$(BUILD_DIR)/control-flow/control-flow-ref-mut-runtime-sequence-binding-mode.elf; code=$$?; test $$code -eq 241
	$(TARGET) tests/cases/control-flow/ok/condition-pattern-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/condition-pattern-payloads.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/condition-pattern-payloads.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/condition-pattern-payloads.ll
	grep -q "while.cond" $(BUILD_DIR)/control-flow/condition-pattern-payloads.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/condition-pattern-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/for-range-inclusive.ari --emit-llvm $(BUILD_DIR)/control-flow/for-range-inclusive.ll
	grep -q "icmp sle" $(BUILD_DIR)/control-flow/for-range-inclusive.ll
	$(TARGET) tests/cases/control-flow/ok/for-range-exact-once-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/for-range-exact-once-owner-state.ll
	grep -q "for.end" $(BUILD_DIR)/control-flow/for-range-exact-once-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/for-range-known-nonempty-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/for-range-known-nonempty-break-owner-state.ll
	grep -q "for.end" $(BUILD_DIR)/control-flow/for-range-known-nonempty-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/range-values.ari --emit-llvm $(BUILD_DIR)/control-flow/range-values.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/range-values.ll
	$(TARGET) tests/cases/control-flow/ok/for-range-alias-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-range-alias-pattern.ll
	grep -q "store i64 .* ptr %item" $(BUILD_DIR)/control-flow/for-range-alias-pattern.ll
	grep -q "store i64 .* ptr %value" $(BUILD_DIR)/control-flow/for-range-alias-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-pattern-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/for-pattern-payloads.ll
	grep -q "store i64 .* ptr %item" $(BUILD_DIR)/control-flow/for-pattern-payloads.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/for-pattern-payloads.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-pattern-payloads.ll
	$(TARGET) tests/cases/generics/ok/range-generic-bounds.ari --emit-llvm $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ult i8" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ule i8" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ult i16" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ult i64" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	$(TARGET) tests/cases/ir/ok/ir-for-loop-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/ir-for-loop-payloads.ll
	grep -q "for.cond" $(BUILD_DIR)/control-flow/ir-for-loop-payloads.ll
	grep -q "forvec.next" $(BUILD_DIR)/control-flow/ir-for-loop-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-pattern.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/for-vector-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-alias-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-alias-pattern.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/for-vector-alias-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-exact-once-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-exact-once-owner-state.ll
	grep -q "for.end" $(BUILD_DIR)/control-flow/for-vector-exact-once-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-known-nonempty-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-known-nonempty-break-owner-state.ll
	grep -q "for.end" $(BUILD_DIR)/control-flow/for-vector-known-nonempty-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-stored.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-stored.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/control-flow/for-vector-stored.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/control-flow/for-vector-stored.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-stored-alias-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-vector-stored-current-len.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-stored-current-len.ll
	test $$(grep -c "store i64 1, ptr %.*for_end" $(BUILD_DIR)/control-flow/for-vector-stored-current-len.ll) -ge 2
	grep -q "store i64 0, ptr %.*for_end" $(BUILD_DIR)/control-flow/for-vector-stored-current-len.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-vector-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-pattern.elf; $(BUILD_DIR)/control-flow/for-vector-pattern.elf; code=$$?; test $$code -eq 50; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-vector-alias-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-alias-pattern.elf; $(BUILD_DIR)/control-flow/for-vector-alias-pattern.elf; code=$$?; test $$code -eq 75; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-vector-stored.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-stored.elf; $(BUILD_DIR)/control-flow/for-vector-stored.elf; code=$$?; test $$code -eq 20; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-vector-stored-alias-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.elf; $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.elf; code=$$?; test $$code -eq 28; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-pattern-payloads.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-pattern-payloads.elf; $(BUILD_DIR)/control-flow/for-pattern-payloads.elf; code=$$?; test $$code -eq 67; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/while-let-or-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/while-let-or-pattern.elf; $(BUILD_DIR)/control-flow/while-let-or-pattern.elf; code=$$?; test $$code -eq 55; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/if-let-or-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/if-let-or-pattern.elf; $(BUILD_DIR)/control-flow/if-let-or-pattern.elf; code=$$?; test $$code -eq 44; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/condition-pattern-payloads.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/condition-pattern-payloads.elf; $(BUILD_DIR)/control-flow/condition-pattern-payloads.elf; code=$$?; test $$code -eq 35; fi
	$(TARGET) tests/cases/control-flow/ok/labeled-block.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block.ll
	grep -q "block.end" $(BUILD_DIR)/control-flow/labeled-block.ll
	$(TARGET) tests/cases/control-flow/ok/if-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/if-expression.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-expression.ll
	grep -q "if.expr.end" $(BUILD_DIR)/control-flow/if-expression.ll
	$(TARGET) tests/cases/control-flow/ok/noreturn-expression-arms.ari --emit-llvm $(BUILD_DIR)/control-flow/noreturn-expression-arms.ll
	grep -q "call void @\"ari_builtin_panic\"()" $(BUILD_DIR)/control-flow/noreturn-expression-arms.ll
	grep -q "unreachable" $(BUILD_DIR)/control-flow/noreturn-expression-arms.ll
	grep -q "match.expr.end" $(BUILD_DIR)/control-flow/noreturn-expression-arms.ll
	$(TARGET) tests/cases/control-flow/ok/if-expression-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/if-expression-payloads.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-expression-payloads.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-expression-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/if-let-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/if-let-expression.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-let-expression.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-let-expression.ll
	grep -q "lshr" $(BUILD_DIR)/control-flow/if-let-expression.ll
	$(TARGET) tests/cases/control-flow/ok/if-let-or-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/if-let-or-pattern.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-let-or-pattern.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-let-or-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/if-let-aggregate-or-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/if-let-aggregate-or-pattern.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-let-aggregate-or-pattern.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/if-let-aggregate-or-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/block-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/block-expression.ll
	grep -q "_ARNv4main" $(BUILD_DIR)/control-flow/block-expression.ll
	$(TARGET) tests/cases/control-flow/ok/block-expression-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/block-expression-payloads.ll
	grep -q "block.expr.end" $(BUILD_DIR)/control-flow/block-expression-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/statement-body-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/statement-body-payloads.ll
	grep -q "if.then" $(BUILD_DIR)/control-flow/statement-body-payloads.ll
	grep -q "while.cond" $(BUILD_DIR)/control-flow/statement-body-payloads.ll
	grep -q "for.cond" $(BUILD_DIR)/control-flow/statement-body-payloads.ll
	grep -q "initwhile.cond" $(BUILD_DIR)/control-flow/statement-body-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-owning.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-owning.ll
	grep -q "initwhile.update" $(BUILD_DIR)/control-flow/init-while-owning.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-false-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-false-owner-state.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/init-while-false-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-true-noreturn.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-true-noreturn.ll
	grep -q "br i1 1" $(BUILD_DIR)/control-flow/init-while-true-noreturn.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-true-return.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-true-return.ll
	grep -q "add i64 .* 9" $(BUILD_DIR)/control-flow/init-while-true-return.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-true-continue-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-true-continue-break-owner-state.ll
	grep -q "initwhile.end" $(BUILD_DIR)/control-flow/init-while-true-continue-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-known-true-continue-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-known-true-continue-break-owner-state.ll
	grep -q "br i1 1" $(BUILD_DIR)/control-flow/init-while-known-true-continue-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/init-while-known-bool-alias-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/init-while-known-bool-alias-owner-state.ll
	grep -q "br i1 1" $(BUILD_DIR)/control-flow/init-while-known-bool-alias-owner-state.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/init-while-known-bool-alias-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/labeled-block-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block-expression.ll
	grep -q "block.expr.end" $(BUILD_DIR)/control-flow/labeled-block-expression.ll
	$(TARGET) tests/cases/control-flow/ok/labeled-block-aggregate.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block-aggregate.ll
	grep -q "store { i64, i64 }" $(BUILD_DIR)/control-flow/labeled-block-aggregate.ll
	$(TARGET) tests/cases/control-flow/ok/labeled-block-own-break.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block-own-break.ll
	grep -q "block.expr.end" $(BUILD_DIR)/control-flow/labeled-block-own-break.ll
	$(TARGET) tests/cases/control-flow/ok/break-statement-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/break-statement-payloads.ll
	grep -q "block.expr.end" $(BUILD_DIR)/control-flow/break-statement-payloads.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/break-statement-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/while-true-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-break-owner-state.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/while-true-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-known-true-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-known-true-break-owner-state.ll
	grep -q "br i1 1" $(BUILD_DIR)/control-flow/while-known-true-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-known-bool-alias-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-known-bool-alias-owner-state.ll
	grep -q "br i1 1" $(BUILD_DIR)/control-flow/while-known-bool-alias-owner-state.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/while-known-bool-alias-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/loop-break-owner-unavailable-merge.ari --emit-llvm $(BUILD_DIR)/control-flow/loop-break-owner-unavailable-merge.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/loop-break-owner-unavailable-merge.ll
	$(TARGET) tests/cases/control-flow/ok/loop-break-borrow-release-merge.ari --emit-llvm $(BUILD_DIR)/control-flow/loop-break-borrow-release-merge.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/loop-break-borrow-release-merge.ll
	$(TARGET) tests/cases/control-flow/ok/loop-fallthrough-borrow-release-merge.ari --emit-llvm $(BUILD_DIR)/control-flow/loop-fallthrough-borrow-release-merge.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/loop-fallthrough-borrow-release-merge.ll
	$(TARGET) tests/cases/control-flow/ok/loop-continue-borrow-release-merge.ari --emit-llvm $(BUILD_DIR)/control-flow/loop-continue-borrow-release-merge.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/loop-continue-borrow-release-merge.ll
	$(TARGET) tests/cases/control-flow/ok/while-true-continue-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-continue-break-owner-state.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/while-true-continue-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-true-return-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-return-owner-state.ll
	grep -q "ret i64 12" $(BUILD_DIR)/control-flow/while-true-return-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-true-literal-if-break-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-true-literal-if-break-owner-state.ll
	grep -q "br i1 1" $(BUILD_DIR)/control-flow/while-true-literal-if-break-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-false-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-false-owner-state.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/while-false-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-known-false-owner-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-known-false-owner-state.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/while-known-false-owner-state.ll
	$(TARGET) tests/cases/control-flow/ok/while-false-borrow-state.ari --emit-llvm $(BUILD_DIR)/control-flow/while-false-borrow-state.ll
	grep -q "br i1 0" $(BUILD_DIR)/control-flow/while-false-borrow-state.ll
	$(TARGET) tests/cases/control-flow/ok/statement-label-payloads.ari --emit-llvm $(BUILD_DIR)/control-flow/statement-label-payloads.ll
	grep -q "block.end" $(BUILD_DIR)/control-flow/statement-label-payloads.ll
	grep -q "while.end" $(BUILD_DIR)/control-flow/statement-label-payloads.ll
	grep -q "for.end" $(BUILD_DIR)/control-flow/statement-label-payloads.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-direct.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-direct.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-direct.ll
	$(TARGET) tests/cases/control-flow/ok/for-generic-iterator-direct.ari --emit-llvm $(BUILD_DIR)/control-flow/for-generic-iterator-direct.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-generic-iterator-direct.ll
	$(TARGET) tests/cases/control-flow/ok/for-into-iterator-direct.ari --emit-llvm $(BUILD_DIR)/control-flow/for-into-iterator-direct.ll
	grep -q "into_iter" $(BUILD_DIR)/control-flow/for-into-iterator-direct.ll
	$(TARGET) tests/cases/control-flow/ok/for-into-iterator-result.ari --emit-llvm $(BUILD_DIR)/control-flow/for-into-iterator-result.ll
	grep -q "BagIter.*next" $(BUILD_DIR)/control-flow/for-into-iterator-result.ll
	$(TARGET) tests/cases/control-flow/ok/for-generic-into-iterator-result.ari --emit-llvm $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.ll
	grep -q "BagIter_i64.*next" $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.ll
	$(TARGET) tests/cases/control-flow/ok/for-into-iterator-mut-self.ari --emit-llvm $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.ll
	grep -q "into_iter" $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-refutable-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-filter-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-payload-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-aggregate-payload-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.ll
	grep -F -q "extractvalue { i32, { i32, i64 } }" $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-nested-multi-payload-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.ll
	grep -F -q "extractvalue { i32, { i32, i64, i64 } }" $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-mut-self.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-mut-self.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-mut-self.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-ref-mut.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-ref-mut.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-ref-mut.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-owning-value.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-owning-value.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-owning-value.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-owning-return.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-owning-return.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/control-flow/for-iterator-owning-return.ll
	$(TARGET) tests/cases/control-flow/ok/for-iterator-owning-try.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-owning-try.ll
	grep -q "try.return" $(BUILD_DIR)/control-flow/for-iterator-owning-try.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/control-flow/for-iterator-owning-try.ll) -ge 2
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-direct.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-direct.elf; $(BUILD_DIR)/control-flow/for-iterator-direct.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-generic-iterator-direct.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-generic-iterator-direct.elf; $(BUILD_DIR)/control-flow/for-generic-iterator-direct.elf; code=$$?; test $$code -eq 11; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-into-iterator-direct.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-into-iterator-direct.elf; $(BUILD_DIR)/control-flow/for-into-iterator-direct.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-into-iterator-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-into-iterator-result.elf; $(BUILD_DIR)/control-flow/for-into-iterator-result.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-generic-into-iterator-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.elf; $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-into-iterator-mut-self.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.elf; $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.elf; code=$$?; test $$code -eq 9; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-refutable-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.elf; code=$$?; test $$code -eq 52; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-filter-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.elf; code=$$?; test $$code -eq 20; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-payload-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-aggregate-payload-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.elf; code=$$?; test $$code -eq 21; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-nested-multi-payload-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.elf; code=$$?; test $$code -eq 55; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-mut-self.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-mut-self.elf; $(BUILD_DIR)/control-flow/for-iterator-mut-self.elf; code=$$?; test $$code -eq 15; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-ref-mut.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-ref-mut.elf; $(BUILD_DIR)/control-flow/for-iterator-ref-mut.elf; code=$$?; test $$code -eq 64; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-owning-value.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-owning-value.elf; $(BUILD_DIR)/control-flow/for-iterator-owning-value.elf; code=$$?; test $$code -eq 6; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-owning-return.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-owning-return.elf; $(BUILD_DIR)/control-flow/for-iterator-owning-return.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/control-flow/ok/for-iterator-owning-try.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-owning-try.elf; $(BUILD_DIR)/control-flow/for-iterator-owning-try.elf; code=$$?; test $$code -eq 87; fi
	if $(TARGET) tests/cases/control-flow/errors/for-iterator-planned.ari -o $(BUILD_DIR)/control-flow/for-iterator-planned.elf >$(BUILD_DIR)/control-flow/for-iterator-planned.err 2>&1; then exit 1; fi
	grep -q "copyable IntoIterator\\[T\\] values" $(BUILD_DIR)/control-flow/for-iterator-planned.err
	if $(TARGET) tests/cases/control-flow/errors/for-into-iterator-result-not-iterator.ari -o $(BUILD_DIR)/control-flow/for-into-iterator-result-not-iterator.elf >$(BUILD_DIR)/control-flow/for-into-iterator-result-not-iterator.err 2>&1; then exit 1; fi
	grep -q "method 'into_iter' for IntoIterator\\[i64\\] must return a type that implements Iterator\\[i64\\], got i64" $(BUILD_DIR)/control-flow/for-into-iterator-result-not-iterator.err
	if $(TARGET) tests/cases/control-flow/errors/for-generic-into-iterator-result-not-iterator.ari -o $(BUILD_DIR)/control-flow/for-generic-into-iterator-result-not-iterator.elf >$(BUILD_DIR)/control-flow/for-generic-into-iterator-result-not-iterator.err 2>&1; then exit 1; fi
	grep -q "method 'into_iter' for IntoIterator\\[T\\] must return a type that implements Iterator\\[T\\], got T" $(BUILD_DIR)/control-flow/for-generic-into-iterator-result-not-iterator.err
	if $(TARGET) tests/cases/control-flow/errors/for-pattern-planned.ari -o $(BUILD_DIR)/control-flow/for-pattern-planned.elf >$(BUILD_DIR)/control-flow/for-pattern-planned.err 2>&1; then exit 1; fi
	grep -q "for enum-case patterns are planned for Iterator\\[T\\] values" $(BUILD_DIR)/control-flow/for-pattern-planned.err
	if $(TARGET) tests/cases/control-flow/errors/for-vector-refutable-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-refutable-pattern.elf >$(BUILD_DIR)/control-flow/for-vector-refutable-pattern.err 2>&1; then exit 1; fi
	grep -q "refutable for-loop patterns require for-let filters on Iterator\\[T\\]" $(BUILD_DIR)/control-flow/for-vector-refutable-pattern.err
	if $(TARGET) tests/cases/control-flow/errors/for-let-filter-range.ari -o $(BUILD_DIR)/control-flow/for-let-filter-range.elf >$(BUILD_DIR)/control-flow/for-let-filter-range.err 2>&1; then exit 1; fi
	grep -q "for-let filters currently require an Iterator\\[T\\] or IntoIterator\\[T\\] value" $(BUILD_DIR)/control-flow/for-let-filter-range.err
	if $(TARGET) tests/cases/control-flow/errors/for-vector-alias-refutable-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-alias-refutable-pattern.elf >$(BUILD_DIR)/control-flow/for-vector-alias-refutable-pattern.err 2>&1; then exit 1; fi
	grep -q "refutable for-loop patterns require for-let filters on Iterator\\[T\\]" $(BUILD_DIR)/control-flow/for-vector-alias-refutable-pattern.err
	if $(TARGET) tests/cases/control-flow/errors/for-range-alias-refutable-pattern.ari -o $(BUILD_DIR)/control-flow/for-range-alias-refutable-pattern.elf >$(BUILD_DIR)/control-flow/for-range-alias-refutable-pattern.err 2>&1; then exit 1; fi
	grep -q "refutable for-loop patterns require for-let filters on Iterator\\[T\\]" $(BUILD_DIR)/control-flow/for-range-alias-refutable-pattern.err
	if $(TARGET) tests/cases/control-flow/errors/range-type-non-integer.ari -o $(BUILD_DIR)/control-flow/range-type-non-integer.elf >$(BUILD_DIR)/control-flow/range-type-non-integer.err 2>&1; then exit 1; fi
	grep -q "prelude range types require integer bounds" $(BUILD_DIR)/control-flow/range-type-non-integer.err
	if $(TARGET) tests/cases/control-flow/errors/break-unknown-label.ari -o $(BUILD_DIR)/control-flow/break-unknown-label.elf >$(BUILD_DIR)/control-flow/break-unknown-label.err 2>&1; then exit 1; fi
	grep -q "unknown loop label 'missing'" $(BUILD_DIR)/control-flow/break-unknown-label.err
	if $(TARGET) tests/cases/control-flow/errors/duplicate-loop-label.ari -o $(BUILD_DIR)/control-flow/duplicate-loop-label.elf >$(BUILD_DIR)/control-flow/duplicate-loop-label.err 2>&1; then exit 1; fi
	grep -q "duplicate active loop label 'loop'" $(BUILD_DIR)/control-flow/duplicate-loop-label.err
	if $(TARGET) tests/cases/control-flow/errors/if-expression-type-mismatch.ari -o $(BUILD_DIR)/control-flow/if-expression-type-mismatch.elf >$(BUILD_DIR)/control-flow/if-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/control-flow/if-expression-type-mismatch.err
	if $(TARGET) tests/cases/control-flow/errors/if-let-expression-type-mismatch.ari -o $(BUILD_DIR)/control-flow/if-let-expression-type-mismatch.elf >$(BUILD_DIR)/control-flow/if-let-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/control-flow/if-let-expression-type-mismatch.err
	if $(TARGET) tests/cases/control-flow/errors/if-let-aggregate-irrefutable-else.ari -o $(BUILD_DIR)/control-flow/if-let-aggregate-irrefutable-else.elf >$(BUILD_DIR)/control-flow/if-let-aggregate-irrefutable-else.err 2>&1; then exit 1; fi
	grep -q "irrefutable if-let aggregate pattern cannot have else" $(BUILD_DIR)/control-flow/if-let-aggregate-irrefutable-else.err
	if $(TARGET) tests/cases/control-flow/errors/labeled-block-break-type.ari -o $(BUILD_DIR)/control-flow/labeled-block-break-type.elf >$(BUILD_DIR)/control-flow/labeled-block-break-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i64" $(BUILD_DIR)/control-flow/labeled-block-break-type.err
	if $(TARGET) tests/cases/control-flow/errors/labeled-block-ownership-state.ari -o $(BUILD_DIR)/control-flow/labeled-block-ownership-state.elf >$(BUILD_DIR)/control-flow/labeled-block-ownership-state.err 2>&1; then exit 1; fi
	grep -q "binding 'token' has incompatible ownership states after labeled block exits" $(BUILD_DIR)/control-flow/labeled-block-ownership-state.err
	if $(TARGET) tests/cases/control-flow/errors/block-expression-final-semicolon.ari -o $(BUILD_DIR)/control-flow/block-expression-final-semicolon.elf >$(BUILD_DIR)/control-flow/block-expression-final-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected value before } in block expression" $(BUILD_DIR)/control-flow/block-expression-final-semicolon.err
	if $(TARGET) tests/cases/control-flow/errors/block-expression-nonfinal-missing-semicolon.ari -o $(BUILD_DIR)/control-flow/block-expression-nonfinal-missing-semicolon.elf >$(BUILD_DIR)/control-flow/block-expression-nonfinal-missing-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected ; after non-final expression statement" $(BUILD_DIR)/control-flow/block-expression-nonfinal-missing-semicolon.err
	if $(TARGET) tests/cases/control-flow/errors/nonblock-statement-missing-semicolon.ari -o $(BUILD_DIR)/control-flow/nonblock-statement-missing-semicolon.elf >$(BUILD_DIR)/control-flow/nonblock-statement-missing-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected ; after binding" $(BUILD_DIR)/control-flow/nonblock-statement-missing-semicolon.err
	if $(TARGET) tests/cases/control-flow/errors/init-while-own-overwrite.ari -o $(BUILD_DIR)/control-flow/init-while-own-overwrite.elf >$(BUILD_DIR)/control-flow/init-while-own-overwrite.err 2>&1; then exit 1; fi
	grep -q "cannot overwrite owning init-while binding 'token' before it is moved or dropped" $(BUILD_DIR)/control-flow/init-while-own-overwrite.err
	if $(TARGET) tests/cases/control-flow/errors/init-while-own-plain-continue.ari -o $(BUILD_DIR)/control-flow/init-while-own-plain-continue.elf >$(BUILD_DIR)/control-flow/init-while-own-plain-continue.err 2>&1; then exit 1; fi
	grep -q "plain continue in owning init-while loops must provide update values" $(BUILD_DIR)/control-flow/init-while-own-plain-continue.err

check-match: $(TARGET)
	mkdir -p $(BUILD_DIR)/match
	$(TARGET) tests/cases/match/ok/enum-match.ari -o $(BUILD_DIR)/match/enum-match.elf
	$(BUILD_DIR)/match/enum-match.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/match/ok/match-patterns.ari -o $(BUILD_DIR)/match/match-patterns.elf
	$(BUILD_DIR)/match/match-patterns.elf; code=$$?; test $$code -eq 34
	$(TARGET) tests/cases/match/ok/match-wildcard.ari -o $(BUILD_DIR)/match/match-wildcard.elf
	$(BUILD_DIR)/match/match-wildcard.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/match/ok/match-return-flow.ari -o $(BUILD_DIR)/match/match-return-flow.elf
	$(BUILD_DIR)/match/match-return-flow.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/match/ok/match-llvm.ari -o $(BUILD_DIR)/match/match-llvm.elf
	$(BUILD_DIR)/match/match-llvm.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/match/ok/match-expression.ari -o $(BUILD_DIR)/match/match-expression.elf
	$(BUILD_DIR)/match/match-expression.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/match/ok/match-expression-payloads.ari -o $(BUILD_DIR)/match/match-expression-payloads.elf
	$(BUILD_DIR)/match/match-expression-payloads.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/match/ok/match-literals.ari -o $(BUILD_DIR)/match/match-literals.elf
	$(BUILD_DIR)/match/match-literals.elf; code=$$?; test $$code -eq 128
	$(TARGET) tests/cases/match/ok/match-statement-payloads.ari -o $(BUILD_DIR)/match/match-statement-payloads.elf
	$(BUILD_DIR)/match/match-statement-payloads.elf; code=$$?; test $$code -eq 30
	$(TARGET) tests/cases/match/ok/match-owned-enum-payload.ari -o $(BUILD_DIR)/match/match-owned-enum-payload.elf
	$(BUILD_DIR)/match/match-owned-enum-payload.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/patterns/ok/pattern-payload-unions.ari -o $(BUILD_DIR)/match/pattern-payload-unions.elf
	$(BUILD_DIR)/match/pattern-payload-unions.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/match/ok/match-tuple-pattern.ari -o $(BUILD_DIR)/match/match-tuple-pattern.elf
	$(BUILD_DIR)/match/match-tuple-pattern.elf; code=$$?; test $$code -eq 32
	$(TARGET) tests/cases/match/ok/match-struct-pattern.ari -o $(BUILD_DIR)/match/match-struct-pattern.elf
	$(BUILD_DIR)/match/match-struct-pattern.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/match/ok/match-product-exhaustive.ari -o $(BUILD_DIR)/match/match-product-exhaustive.elf
	$(BUILD_DIR)/match/match-product-exhaustive.elf; code=$$?; test $$code -eq 25
	$(TARGET) tests/cases/match/ok/match-finite-product-domain.ari -o $(BUILD_DIR)/match/match-finite-product-domain.elf
	$(BUILD_DIR)/match/match-finite-product-domain.elf; code=$$?; test $$code -eq 6
	$(TARGET) tests/cases/match/ok/match-symbolic-product.ari -o $(BUILD_DIR)/match/match-symbolic-product.elf
	$(BUILD_DIR)/match/match-symbolic-product.elf; code=$$?; test $$code -eq 9
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/match-aggregate-enum-payload-literals.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.elf; $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.elf; code=$$?; test $$code -eq 64; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/match-aggregate-enum-nested-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.elf; $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.elf; code=$$?; test $$code -eq 99; fi
	$(TARGET) tests/cases/match/ok/match-or-binding.ari -o $(BUILD_DIR)/match/match-or-binding.elf
	$(BUILD_DIR)/match/match-or-binding.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/match/ok/match-range-overlap.ari -o $(BUILD_DIR)/match/match-range-overlap.elf
	$(BUILD_DIR)/match/match-range-overlap.elf; code=$$?; test $$code -eq 1
	$(TARGET) tests/cases/match/ok/match-signed-range-coverage.ari -o $(BUILD_DIR)/match/match-signed-range-coverage.elf >$(BUILD_DIR)/match/match-signed-range-coverage.warn 2>&1
	grep -q "warning: scalar match pattern is fully shadowed by earlier match arms" $(BUILD_DIR)/match/match-signed-range-coverage.warn
	$(BUILD_DIR)/match/match-signed-range-coverage.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/match/ok/match-shadow-warning.ari -o $(BUILD_DIR)/match/match-shadow-warning.elf >$(BUILD_DIR)/match/match-shadow-warning.warn 2>&1
	grep -q "warning: scalar match pattern is fully shadowed by earlier match arms" $(BUILD_DIR)/match/match-shadow-warning.warn
	$(BUILD_DIR)/match/match-shadow-warning.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/match/ok/match-product-shadow-warning.ari -o $(BUILD_DIR)/match/match-product-shadow-warning.elf >$(BUILD_DIR)/match/match-product-shadow-warning.warn 2>&1
	grep -q "warning: aggregate match pattern is fully shadowed by earlier match arms" $(BUILD_DIR)/match/match-product-shadow-warning.warn
	$(BUILD_DIR)/match/match-product-shadow-warning.elf; code=$$?; test $$code -eq 1
	$(TARGET) tests/cases/match/ok/match-symbolic-product-shadow-warning.ari -o $(BUILD_DIR)/match/match-symbolic-product-shadow-warning.elf >$(BUILD_DIR)/match/match-symbolic-product-shadow-warning.warn 2>&1
	grep -q "warning: aggregate match pattern is fully shadowed by earlier match arms" $(BUILD_DIR)/match/match-symbolic-product-shadow-warning.warn
	$(BUILD_DIR)/match/match-symbolic-product-shadow-warning.elf; code=$$?; test $$code -eq 1
	$(TARGET) tests/cases/match/ok/match-alias-pattern.ari -o $(BUILD_DIR)/match/match-alias-pattern.elf
	$(BUILD_DIR)/match/match-alias-pattern.elf; code=$$?; test $$code -eq 28
	$(TARGET) tests/cases/match/ok/match-alias-or-pattern.ari -o $(BUILD_DIR)/match/match-alias-or-pattern.elf
	$(BUILD_DIR)/match/match-alias-or-pattern.elf; code=$$?; test $$code -eq 66
	$(TARGET) tests/cases/match/ok/match-enum-payload-literals.ari -o $(BUILD_DIR)/match/match-enum-payload-literals.elf
	$(BUILD_DIR)/match/match-enum-payload-literals.elf; code=$$?; test $$code -eq 66
	$(TARGET) tests/cases/constants/ok/constant-patterns.ari -o $(BUILD_DIR)/match/constant-patterns.elf
	$(BUILD_DIR)/match/constant-patterns.elf; code=$$?; test $$code -eq 61
	$(TARGET) tests/cases/constants/ok/constant-enum-payload-patterns.ari -o $(BUILD_DIR)/match/constant-enum-payload-patterns.elf
	$(BUILD_DIR)/match/constant-enum-payload-patterns.elf; code=$$?; test $$code -eq 65
	$(TARGET) tests/cases/constants/ok/constant-folding.ari -o $(BUILD_DIR)/match/constant-folding.elf
	$(BUILD_DIR)/match/constant-folding.elf; code=$$?; test $$code -eq 32
	$(TARGET) tests/cases/constants/ok/constant-bitwise-folding.ari -o $(BUILD_DIR)/match/constant-bitwise-folding.elf
	$(BUILD_DIR)/match/constant-bitwise-folding.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/constants/ok/constant-aggregates.ari -o $(BUILD_DIR)/match/constant-aggregates.elf
	$(BUILD_DIR)/match/constant-aggregates.elf; code=$$?; test $$code -eq 34
	$(TARGET) tests/cases/constants/ok/constant-direct-composite.ari -o $(BUILD_DIR)/match/constant-direct-composite.elf
	$(BUILD_DIR)/match/constant-direct-composite.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/variables/ok/array-patterns.ari -o $(BUILD_DIR)/match/array-patterns.elf
	$(BUILD_DIR)/match/array-patterns.elf; code=$$?; test $$code -eq 25
	$(TARGET) tests/cases/match/ok/runtime-sequence-patterns.ari -o $(BUILD_DIR)/match/runtime-sequence-patterns.elf
	$(BUILD_DIR)/match/runtime-sequence-patterns.elf; code=$$?; test $$code -eq 166
	$(TARGET) tests/cases/match/ok/match-patterns.ari --emit-llvm $(BUILD_DIR)/match/match-patterns.ll
	grep -q "match.arm" $(BUILD_DIR)/match/match-patterns.ll
	grep -q "lshr" $(BUILD_DIR)/match/match-patterns.ll
	$(TARGET) tests/cases/match/ok/match-expression.ari --emit-llvm $(BUILD_DIR)/match/match-expression.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/match-expression.ll
	grep -q "phi i64" $(BUILD_DIR)/match/match-expression.ll
	$(TARGET) tests/cases/match/ok/match-expression-payloads.ari --emit-llvm $(BUILD_DIR)/match/match-expression-payloads.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/match-expression-payloads.ll
	grep -q "phi i64" $(BUILD_DIR)/match/match-expression-payloads.ll
	$(TARGET) tests/cases/match/ok/match-literals.ari --emit-llvm $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp eq i1" $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp sge i64" $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp slt i64" $(BUILD_DIR)/match/match-literals.ll
	$(TARGET) tests/cases/match/ok/match-statement-payloads.ari --emit-llvm $(BUILD_DIR)/match/match-statement-payloads.ll
	grep -q "match.arm" $(BUILD_DIR)/match/match-statement-payloads.ll
	grep -q "icmp sle i64" $(BUILD_DIR)/match/match-statement-payloads.ll
	grep -q "extractvalue" $(BUILD_DIR)/match/match-statement-payloads.ll
	$(TARGET) tests/cases/match/ok/match-owned-enum-payload.ari --emit-llvm $(BUILD_DIR)/match/match-owned-enum-payload.ll
	grep -q "store i64 .*%token.addr" $(BUILD_DIR)/match/match-owned-enum-payload.ll
	grep -q "match.arm" $(BUILD_DIR)/match/match-owned-enum-payload.ll
	$(TARGET) tests/cases/patterns/ok/pattern-payload-unions.ari --emit-llvm $(BUILD_DIR)/match/pattern-payload-unions.ll
	grep -q "icmp eq i1" $(BUILD_DIR)/match/pattern-payload-unions.ll
	grep -q "icmp sge i64" $(BUILD_DIR)/match/pattern-payload-unions.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/pattern-payload-unions.ll
	grep -q "store { i64, i1 }" $(BUILD_DIR)/match/pattern-payload-unions.ll
	$(TARGET) tests/cases/match/ok/match-alias-pattern.ari --emit-llvm $(BUILD_DIR)/match/match-alias-pattern.ll
	grep -q "store i64" $(BUILD_DIR)/match/match-alias-pattern.ll
	grep -q "store i8" $(BUILD_DIR)/match/match-alias-pattern.ll
	$(TARGET) tests/cases/match/ok/match-alias-or-pattern.ari --emit-llvm $(BUILD_DIR)/match/match-alias-or-pattern.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/match-alias-or-pattern.ll
	grep -q "store { i64, i1 }" $(BUILD_DIR)/match/match-alias-or-pattern.ll
	$(TARGET) tests/cases/match/ok/match-enum-payload-literals.ari --emit-llvm $(BUILD_DIR)/match/match-enum-payload-literals.ll
	grep -q "4294967297" $(BUILD_DIR)/match/match-enum-payload-literals.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/match/match-enum-payload-literals.ll
	grep -q "icmp sle i32" $(BUILD_DIR)/match/match-enum-payload-literals.ll
	$(TARGET) tests/cases/match/ok/enum-multi-payload.ari --emit-llvm $(BUILD_DIR)/match/enum-multi-payload.ll
	grep -q "insertvalue { i32, i64, i64, i64 }" $(BUILD_DIR)/match/enum-multi-payload.ll
	grep -q "extractvalue { i32, i64, i64, i64 }" $(BUILD_DIR)/match/enum-multi-payload.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/enum-multi-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-multi-payload.elf; $(BUILD_DIR)/match/enum-multi-payload.elf; code=$$?; test $$code -eq 33; fi
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-local.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-local.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-local.elf; code=$$?; test $$code -eq 30
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-payload-tests.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-payload-tests.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-payload-tests.elf; code=$$?; test $$code -eq 73
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-nested-payload.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-nested-payload.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-nested-payload.elf; code=$$?; test $$code -eq 99
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-nested-aggregate-payload.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-nested-aggregate-payload.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-nested-aggregate-payload.elf; code=$$?; test $$code -eq 49
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-pointer-copy.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-pointer-copy.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-pointer-copy.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-direct-store.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-direct-store.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-direct-store.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-enum-materialization.ari -o $(BUILD_DIR)/match/llvm-aggregate-enum-materialization.elf
	$(BUILD_DIR)/match/llvm-aggregate-enum-materialization.elf; code=$$?; test $$code -eq 65
	$(TARGET) tests/cases/match/ok/enum-plain-aggregate-payloads.ari -o $(BUILD_DIR)/match/enum-plain-aggregate-payloads.elf
	$(BUILD_DIR)/match/enum-plain-aggregate-payloads.elf; code=$$?; test $$code -eq 63
	$(TARGET) tests/cases/match/ok/enum-fixed-vector-payload.ari -o $(BUILD_DIR)/match/enum-fixed-vector-payload.elf
	$(BUILD_DIR)/match/enum-fixed-vector-payload.elf; code=$$?; test $$code -eq 96
	$(TARGET) tests/cases/match/ok/enum-payload-pointer-access.ari --emit-llvm $(BUILD_DIR)/match/enum-payload-pointer-access.ll
	grep -q "getelementptr inbounds { i32, i64 }" $(BUILD_DIR)/match/enum-payload-pointer-access.ll
	grep -q "getelementptr inbounds { i32, { i32, i64, i64 } }" $(BUILD_DIR)/match/enum-payload-pointer-access.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/enum-payload-pointer-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-payload-pointer-access.elf; $(BUILD_DIR)/match/enum-payload-pointer-access.elf; code=$$?; test $$code -eq 65; fi
	$(TARGET) tests/cases/match/ok/enum-payload-pointer-access.ari -o $(BUILD_DIR)/match/enum-payload-pointer-access.elf
	$(BUILD_DIR)/match/enum-payload-pointer-access.elf; code=$$?; test $$code -eq 65
	$(TARGET) tests/cases/match/ok/enum-mixed-nested-scalar-payload-slot.ari --emit-llvm $(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.ll
	grep -q "insertvalue { i32, i64, i64 } zeroinitializer" $(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.ll
	grep -q "extractvalue { i32, i64, i64 }" $(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/enum-mixed-nested-scalar-payload-slot.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.elf; $(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/match/ok/enum-mixed-nested-scalar-payload-slot.ari -o $(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.elf
	$(BUILD_DIR)/match/enum-mixed-nested-scalar-payload-slot.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/match/ok/enum-pointer-shaped-payloads.ari --emit-llvm $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	grep -q "ptrtoint ptr" $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	grep -q "inttoptr i64" $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	grep -q "call i64 %" $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/enum-pointer-shaped-payloads.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-pointer-shaped-payloads.elf; $(BUILD_DIR)/match/enum-pointer-shaped-payloads.elf; code=$$?; test $$code -eq 25; fi
	$(TARGET) tests/cases/match/ok/enum-mixed-nested-pointer-payload-slot.ari --emit-llvm $(BUILD_DIR)/match/enum-mixed-nested-pointer-payload-slot.ll
	grep -q "insertvalue { i32, i64, i64 } zeroinitializer" $(BUILD_DIR)/match/enum-mixed-nested-pointer-payload-slot.ll
	grep -q "ptrtoint ptr" $(BUILD_DIR)/match/enum-mixed-nested-pointer-payload-slot.ll
	grep -q "inttoptr i64" $(BUILD_DIR)/match/enum-mixed-nested-pointer-payload-slot.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/enum-mixed-nested-pointer-payload-slot.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-mixed-nested-pointer-payload-slot.elf; $(BUILD_DIR)/match/enum-mixed-nested-pointer-payload-slot.elf; code=$$?; test $$code -eq 61; fi
	$(TARGET) tests/cases/match/ok/enum-multi-payload.ari -o $(BUILD_DIR)/match/enum-multi-payload.elf
	$(BUILD_DIR)/match/enum-multi-payload.elf; code=$$?; test $$code -eq 33
	$(TARGET) tests/cases/constants/ok/constant-patterns.ari --emit-llvm $(BUILD_DIR)/match/constant-patterns.ll
	grep -q "30064771072" $(BUILD_DIR)/match/constant-patterns.ll
	grep -q "icmp eq i1" $(BUILD_DIR)/match/constant-patterns.ll
	$(TARGET) tests/cases/constants/ok/constant-folding.ari --emit-llvm $(BUILD_DIR)/match/constant-folding.ll
	grep -q "br i1 1" $(BUILD_DIR)/match/constant-folding.ll
	$(TARGET) tests/cases/constants/ok/constant-aggregates.ari --emit-llvm $(BUILD_DIR)/match/constant-aggregates.ll
	grep -q "insertvalue { i64, i64 }" $(BUILD_DIR)/match/constant-aggregates.ll
	grep -q "insertvalue \\[3 x i64\\]" $(BUILD_DIR)/match/constant-aggregates.ll
	$(TARGET) tests/cases/constants/ok/constant-direct-composite.ari --emit-llvm $(BUILD_DIR)/match/constant-direct-composite.ll
	grep -q "insertvalue { i64, i1 }" $(BUILD_DIR)/match/constant-direct-composite.ll
	grep -q "insertvalue \\[2 x i64\\]" $(BUILD_DIR)/match/constant-direct-composite.ll
	$(TARGET) tests/cases/variables/ok/array-patterns.ari --emit-llvm $(BUILD_DIR)/match/array-patterns.ll
	grep -q "extractvalue \\[4 x i64\\]" $(BUILD_DIR)/match/array-patterns.ll
	$(TARGET) tests/cases/match/ok/runtime-sequence-patterns.ari --emit-llvm $(BUILD_DIR)/match/runtime-sequence-patterns.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/match/runtime-sequence-patterns.ll
	grep -q "slice.bounds.fail" $(BUILD_DIR)/match/runtime-sequence-patterns.ll
	grep -q "icmp sge i64" $(BUILD_DIR)/match/runtime-sequence-patterns.ll
	$(TARGET) tests/cases/match/ok/match-symbolic-product.ari --emit-llvm $(BUILD_DIR)/match/match-symbolic-product.ll
	grep -q "icmp ule i16" $(BUILD_DIR)/match/match-symbolic-product.ll
	$(TARGET) tests/cases/match/ok/match-aggregate-enum-payload-literals.ari --emit-llvm $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	grep -q "icmp sle i64" $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	$(TARGET) tests/cases/match/ok/match-aggregate-enum-nested-payload.ari --emit-llvm $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	grep -q "lshr i64" $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	grep -q "icmp sle i32" $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	$(TARGET) tests/cases/match/ok/match-aggregate-enum-nested-aggregate-payload.ari --emit-llvm $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.ll
	grep -F -q "insertvalue { i32, { i32, i64 } }" $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.ll
	grep -F -q "extractvalue { i32, { i32, i64 } }" $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/match/ok/match-aggregate-enum-nested-aggregate-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.elf; $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.elf; code=$$?; test $$code -eq 31; fi
	$(TARGET) tests/cases/match/ok/compiler-shaped-aggregates.ari -o $(BUILD_DIR)/match/compiler-shaped-aggregates.elf
	$(BUILD_DIR)/match/compiler-shaped-aggregates.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/match/ok/compiler-shaped-aggregates.ari --emit-llvm $(BUILD_DIR)/match/compiler-shaped-aggregates.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/compiler-shaped-aggregates.ll
	grep -q "extractvalue" $(BUILD_DIR)/match/compiler-shaped-aggregates.ll
	if $(TARGET) tests/cases/match/errors/match-expression-type-mismatch.ari -o $(BUILD_DIR)/match/match-expression-type-mismatch.elf >$(BUILD_DIR)/match/match-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/match/match-expression-type-mismatch.err
	if $(TARGET) tests/cases/match/errors/match-literal-nonexhaustive.ari -o $(BUILD_DIR)/match/match-literal-nonexhaustive.elf >$(BUILD_DIR)/match/match-literal-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "integer match must include a wildcard arm" $(BUILD_DIR)/match/match-literal-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-bool-nonexhaustive.ari -o $(BUILD_DIR)/match/match-bool-nonexhaustive.elf >$(BUILD_DIR)/match/match-bool-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "bool match must cover true and false or include a wildcard arm" $(BUILD_DIR)/match/match-bool-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/runtime-sequence-match-nonexhaustive.ari -o $(BUILD_DIR)/match/runtime-sequence-match-nonexhaustive.elf >$(BUILD_DIR)/match/runtime-sequence-match-nonexhaustive.err 2>&1; then exit 1; fi
	grep -Fq "Vec[T] match must include _ or [..] fallback" $(BUILD_DIR)/match/runtime-sequence-match-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-literal-type.ari -o $(BUILD_DIR)/match/match-literal-type.elf >$(BUILD_DIR)/match/match-literal-type.err 2>&1; then exit 1; fi
	grep -q "integer match patterns must be integer literals or _" $(BUILD_DIR)/match/match-literal-type.err
	if $(TARGET) tests/cases/match/errors/match-range-pattern-order.ari -o $(BUILD_DIR)/match/match-range-pattern-order.elf >$(BUILD_DIR)/match/match-range-pattern-order.err 2>&1; then exit 1; fi
	grep -q "range pattern start must be <= end" $(BUILD_DIR)/match/match-range-pattern-order.err
	if $(TARGET) tests/cases/match/errors/match-range-pattern-nonexhaustive.ari -o $(BUILD_DIR)/match/match-range-pattern-nonexhaustive.elf >$(BUILD_DIR)/match/match-range-pattern-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "integer match must include a wildcard arm" $(BUILD_DIR)/match/match-range-pattern-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-or-pattern-binding.ari -o $(BUILD_DIR)/match/match-or-pattern-binding.elf >$(BUILD_DIR)/match/match-or-pattern-binding.err 2>&1; then exit 1; fi
	grep -q "or-pattern alternatives must bind the same names" $(BUILD_DIR)/match/match-or-pattern-binding.err
	if $(TARGET) tests/cases/match/errors/match-or-pattern-binding-type.ari -o $(BUILD_DIR)/match/match-or-pattern-binding-type.elf >$(BUILD_DIR)/match/match-or-pattern-binding-type.err 2>&1; then exit 1; fi
	grep -q "or-pattern binding 'value' has inconsistent types: i32 and u32" $(BUILD_DIR)/match/match-or-pattern-binding-type.err
	$(TARGET) tests/cases/match/ok/match-nested-reference-binding-mode.ari -o $(BUILD_DIR)/match/match-nested-reference-binding-mode.elf
	$(BUILD_DIR)/match/match-nested-reference-binding-mode.elf; code=$$?; test $$code -eq 40
	$(TARGET) tests/cases/match/ok/match-nested-amp-binding-mode.ari -o $(BUILD_DIR)/match/match-nested-amp-binding-mode.elf
	$(BUILD_DIR)/match/match-nested-amp-binding-mode.elf; code=$$?; test $$code -eq 40
	if $(TARGET) tests/cases/match/errors/match-tuple-nonexhaustive.ari -o $(BUILD_DIR)/match/match-tuple-nonexhaustive.elf >$(BUILD_DIR)/match/match-tuple-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "tuple match must cover every supported product case" $(BUILD_DIR)/match/match-tuple-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-product-nonexhaustive-bool.ari -o $(BUILD_DIR)/match/match-product-nonexhaustive-bool.elf >$(BUILD_DIR)/match/match-product-nonexhaustive-bool.err 2>&1; then exit 1; fi
	grep -q "tuple match must cover every supported product case" $(BUILD_DIR)/match/match-product-nonexhaustive-bool.err
	if $(TARGET) tests/cases/match/errors/match-symbolic-product-missing.ari -o $(BUILD_DIR)/match/match-symbolic-product-missing.elf >$(BUILD_DIR)/match/match-symbolic-product-missing.err 2>&1; then exit 1; fi
	grep -q "missing case such as (_, true)" $(BUILD_DIR)/match/match-symbolic-product-missing.err
	if $(TARGET) tests/cases/match/errors/match-tuple-struct-missing-hint.ari -o $(BUILD_DIR)/match/match-tuple-struct-missing-hint.elf >$(BUILD_DIR)/match/match-tuple-struct-missing-hint.err 2>&1; then exit 1; fi
	grep -q "missing case such as Bits(true, true)" $(BUILD_DIR)/match/match-tuple-struct-missing-hint.err
	if $(TARGET) tests/cases/match/errors/match-struct-pattern-missing-field.ari -o $(BUILD_DIR)/match/match-struct-pattern-missing-field.elf >$(BUILD_DIR)/match/match-struct-pattern-missing-field.err 2>&1; then exit 1; fi
	grep -q "struct match pattern must mention all fields or use '..'" $(BUILD_DIR)/match/match-struct-pattern-missing-field.err
	if $(TARGET) tests/cases/match/errors/match-struct-pattern-duplicate-field.ari -o $(BUILD_DIR)/match/match-struct-pattern-duplicate-field.elf >$(BUILD_DIR)/match/match-struct-pattern-duplicate-field.err 2>&1; then exit 1; fi
	grep -q "duplicate field 'x' in struct match pattern" $(BUILD_DIR)/match/match-struct-pattern-duplicate-field.err
	if $(TARGET) tests/cases/match/errors/match-struct-pattern-unknown-field.ari -o $(BUILD_DIR)/match/match-struct-pattern-unknown-field.elf >$(BUILD_DIR)/match/match-struct-pattern-unknown-field.err 2>&1; then exit 1; fi
	grep -q "struct 'Point' has no field 'z'" $(BUILD_DIR)/match/match-struct-pattern-unknown-field.err
	if $(TARGET) tests/cases/match/errors/match-struct-pattern-tuple-struct.ari -o $(BUILD_DIR)/match/match-struct-pattern-tuple-struct.elf >$(BUILD_DIR)/match/match-struct-pattern-tuple-struct.err 2>&1; then exit 1; fi
	grep -q "tuple structs must use positional match patterns" $(BUILD_DIR)/match/match-struct-pattern-tuple-struct.err
	if $(TARGET) tests/cases/match/errors/match-tuple-struct-pattern-arity.ari -o $(BUILD_DIR)/match/match-tuple-struct-pattern-arity.elf >$(BUILD_DIR)/match/match-tuple-struct-pattern-arity.err 2>&1; then exit 1; fi
	grep -q "tuple-struct pattern for 'Rgb' has 1 field but value has 2" $(BUILD_DIR)/match/match-tuple-struct-pattern-arity.err
	if $(TARGET) tests/cases/match/errors/enum-duplicate-case.ari -o $(BUILD_DIR)/match/enum-duplicate-case.elf >$(BUILD_DIR)/match/enum-duplicate-case.err 2>&1; then exit 1; fi
	grep -q "duplicate enum case constructor 'A'" $(BUILD_DIR)/match/enum-duplicate-case.err
	if $(TARGET) tests/cases/match/errors/enum-constructor-payload-type.ari -o $(BUILD_DIR)/match/enum-constructor-payload-type.elf >$(BUILD_DIR)/match/enum-constructor-payload-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i32, got bool" $(BUILD_DIR)/match/enum-constructor-payload-type.err
	if $(TARGET) tests/cases/match/errors/match-unknown-enum-case.ari -o $(BUILD_DIR)/match/match-unknown-enum-case.elf >$(BUILD_DIR)/match/match-unknown-enum-case.err 2>&1; then exit 1; fi
	grep -q "unknown enum case 'Missing'" $(BUILD_DIR)/match/match-unknown-enum-case.err
	if $(TARGET) tests/cases/match/errors/match-enum-payload-literal-nonexhaustive.ari -o $(BUILD_DIR)/match/match-enum-payload-literal-nonexhaustive.elf >$(BUILD_DIR)/match/match-enum-payload-literal-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "match must cover all cases of enum 'OptionI32'" $(BUILD_DIR)/match/match-enum-payload-literal-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-enum-bool-payload-literal-nonexhaustive.ari -o $(BUILD_DIR)/match/match-enum-bool-payload-literal-nonexhaustive.elf >$(BUILD_DIR)/match/match-enum-bool-payload-literal-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "match must cover all cases of enum 'MaybeBool'" $(BUILD_DIR)/match/match-enum-bool-payload-literal-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-enum-payload-literal-type.ari -o $(BUILD_DIR)/match/match-enum-payload-literal-type.elf >$(BUILD_DIR)/match/match-enum-payload-literal-type.err 2>&1; then exit 1; fi
	grep -q "bool payload patterns require a bool enum payload" $(BUILD_DIR)/match/match-enum-payload-literal-type.err
	if $(TARGET) tests/cases/match/errors/match-enum-payload-duplicate-literal.ari -o $(BUILD_DIR)/match/match-enum-payload-duplicate-literal.elf >$(BUILD_DIR)/match/match-enum-payload-duplicate-literal.err 2>&1; then exit 1; fi
	grep -q "duplicate match arm for enum payload pattern 'Some'" $(BUILD_DIR)/match/match-enum-payload-duplicate-literal.err
	if $(TARGET) tests/cases/match/errors/match-duplicate-arm.ari -o $(BUILD_DIR)/match/match-duplicate-arm.elf >$(BUILD_DIR)/match/match-duplicate-arm.err 2>&1; then exit 1; fi
	grep -q "duplicate match arm for enum case 'A'" $(BUILD_DIR)/match/match-duplicate-arm.err
	if $(TARGET) tests/cases/constants/errors/constant-pattern-type.ari -o $(BUILD_DIR)/match/constant-pattern-type.elf >$(BUILD_DIR)/match/constant-pattern-type.err 2>&1; then exit 1; fi
	grep -q "integer match constant pattern must have an integer type" $(BUILD_DIR)/match/constant-pattern-type.err
	if $(TARGET) tests/cases/constants/errors/constant-folding-div-zero.ari -o $(BUILD_DIR)/match/constant-folding-div-zero.elf >$(BUILD_DIR)/match/constant-folding-div-zero.err 2>&1; then exit 1; fi
	grep -q "constant expression divides by zero" $(BUILD_DIR)/match/constant-folding-div-zero.err
	if $(TARGET) tests/cases/constants/errors/constant-shift-too-wide.ari -o $(BUILD_DIR)/match/constant-shift-too-wide.elf >$(BUILD_DIR)/match/constant-shift-too-wide.err 2>&1; then exit 1; fi
	grep -q "constant shift amount must be less than 8" $(BUILD_DIR)/match/constant-shift-too-wide.err
	if $(TARGET) tests/cases/constants/errors/constant-cycle-path.ari -o $(BUILD_DIR)/match/constant-cycle-path.elf >$(BUILD_DIR)/match/constant-cycle-path.err 2>&1; then exit 1; fi
	grep -q "constant cycle: A -> B -> C -> A" $(BUILD_DIR)/match/constant-cycle-path.err
	if $(TARGET) tests/cases/constants/errors/constant-aggregate-array-arity.ari -o $(BUILD_DIR)/match/constant-aggregate-array-arity.elf >$(BUILD_DIR)/match/constant-aggregate-array-arity.err 2>&1; then exit 1; fi
	grep -q "array constant has 1 element but type expects 2" $(BUILD_DIR)/match/constant-aggregate-array-arity.err
	if $(TARGET) tests/cases/variables/errors/array-pattern-arity.ari -o $(BUILD_DIR)/match/array-pattern-arity.elf >$(BUILD_DIR)/match/array-pattern-arity.err 2>&1; then exit 1; fi
	grep -q "array match pattern has 2 elements but value has 3" $(BUILD_DIR)/match/array-pattern-arity.err

check-modules: $(TARGET)
	mkdir -p $(BUILD_DIR)/modules
	$(TARGET) tests/cases/modules/ok/modules.ari -o $(BUILD_DIR)/modules/modules.elf
	$(BUILD_DIR)/modules/modules.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/modules/ok/module-nested-alias.ari -o $(BUILD_DIR)/modules/module-nested-alias.elf
	$(BUILD_DIR)/modules/module-nested-alias.elf; code=$$?; test $$code -eq 29
	$(TARGET) tests/cases/modules/ok/module-use-list.ari -o $(BUILD_DIR)/modules/module-use-list.elf
	$(BUILD_DIR)/modules/module-use-list.elf; code=$$?; test $$code -eq 15
	$(TARGET) tests/cases/modules/ok/module-use-glob.ari -o $(BUILD_DIR)/modules/module-use-glob.elf
	$(BUILD_DIR)/modules/module-use-glob.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/modules/ok/module-use-alias-prefix.ari -o $(BUILD_DIR)/modules/module-use-alias-prefix.elf
	$(BUILD_DIR)/modules/module-use-alias-prefix.elf; code=$$?; test $$code -eq 8
	$(TARGET) tests/cases/modules/ok/module-relative-paths.ari -o $(BUILD_DIR)/modules/module-relative-paths.elf
	$(BUILD_DIR)/modules/module-relative-paths.elf; code=$$?; test $$code -eq 16
	$(TARGET) tests/cases/modules/ok/module-pub-reexport.ari -o $(BUILD_DIR)/modules/module-pub-reexport.elf
	$(BUILD_DIR)/modules/module-pub-reexport.elf; code=$$?; test $$code -eq 16
	$(TARGET) tests/cases/modules/ok/module-pub-glob-reexport.ari -o $(BUILD_DIR)/modules/module-pub-glob-reexport.elf
	$(BUILD_DIR)/modules/module-pub-glob-reexport.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/modules/ok/file-module-alias-main.ari -o $(BUILD_DIR)/modules/file-module-alias-main.elf
	$(BUILD_DIR)/modules/file-module-alias-main.elf; code=$$?; test $$code -eq 6
	$(TARGET) tests/cases/modules/ok/inline-file-module-main.ari -o $(BUILD_DIR)/modules/inline-file-module-main.elf
	$(BUILD_DIR)/modules/inline-file-module-main.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/modules/ok/file-inline-bridge-main.ari -o $(BUILD_DIR)/modules/file-inline-bridge-main.elf
	$(BUILD_DIR)/modules/file-inline-bridge-main.elf; code=$$?; test $$code -eq 14
	$(TARGET) tests/cases/modules/ok/diamond-project-main.ari -o $(BUILD_DIR)/modules/diamond-project-main.elf
	$(BUILD_DIR)/modules/diamond-project-main.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/modules/ok/trait-project-main.ari -o $(BUILD_DIR)/modules/trait-project-main.elf
	$(BUILD_DIR)/modules/trait-project-main.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari -o $(BUILD_DIR)/modules/project-compiler-main.elf
	$(BUILD_DIR)/modules/project-compiler-main.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/modules/ok/same-filename-project-main.ari -o $(BUILD_DIR)/modules/same-filename-project-main.elf
	$(BUILD_DIR)/modules/same-filename-project-main.elf; code=$$?; test $$code -eq 33
	$(TARGET) tests/cases/modules/ok/file-relative-child-main.ari -o $(BUILD_DIR)/modules/file-relative-child-main.elf
	$(BUILD_DIR)/modules/file-relative-child-main.elf; code=$$?; test $$code -eq 38
	$(TARGET) tests/cases/modules/ok/reimport-same-source-main.ari -o $(BUILD_DIR)/modules/reimport-same-source-main.elf
	$(BUILD_DIR)/modules/reimport-same-source-main.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/modules/ok/project-compiler-main.ari --emit-module-graph $(BUILD_DIR)/modules/project-compiler-main.graph
	grep -E -q "Source ordinal=3 source_id=[0-9]+ module=project_compiler::parser .*path=tests/cases/modules/ok/project_compiler/parse.ari" $(BUILD_DIR)/modules/project-compiler-main.graph
	grep -q "Import owner=project_compiler module=project_compiler::parser local=parse visibility=pub" $(BUILD_DIR)/modules/project-compiler-main.graph
	grep -q "Item module=project_compiler::symbols kind=fn name=project_compiler::symbols::private_bias visibility=private" $(BUILD_DIR)/modules/project-compiler-main.graph
	$(TARGET) tests/cases/modules/ok/same-filename-project-main.ari --emit-module-graph $(BUILD_DIR)/modules/same-filename-project-main.graph
	$(TARGET) tests/cases/modules/ok/same-filename-project-main.ari --emit-module-graph $(BUILD_DIR)/modules/same-filename-project-main.repeat.graph
	cmp -s $(BUILD_DIR)/modules/same-filename-project-main.graph $(BUILD_DIR)/modules/same-filename-project-main.repeat.graph
	grep -E -q "Source ordinal=3 source_id=[0-9]+ module=same_filename_project::left::util .*path=tests/cases/modules/ok/same_filename_project/left/util.ari" $(BUILD_DIR)/modules/same-filename-project-main.graph
	grep -E -q "Source ordinal=5 source_id=[0-9]+ module=same_filename_project::right::util .*path=tests/cases/modules/ok/same_filename_project/right/util.ari" $(BUILD_DIR)/modules/same-filename-project-main.graph
	$(TARGET) --module-path tests/packages tests/cases/modules/ok/package-module-main.ari -o $(BUILD_DIR)/modules/package-module-main.elf
	$(BUILD_DIR)/modules/package-module-main.elf; code=$$?; test $$code -eq 31
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --emit-llvm $(BUILD_DIR)/modules/package-module-main.ll
	grep -q '_ARNv12package_math5value' $(BUILD_DIR)/modules/package-module-main.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --emit-module-metadata $(BUILD_DIR)/modules/package-module-main.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-metadata.ll
	grep -q "ari-module-metadata-v0" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -q "source.*package_math.*tests/packages/package_math.ari" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -Eq "source.*package_math.*tests/packages/package_math.ari.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -q "import.*package_math::nested.*tests/packages/nested/mod.ari" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -q "item.*fn.*package_math::value" $(BUILD_DIR)/modules/package-module-main.arimeta
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-metadata-check.ll
	$(TARGET) tests/cases/modules/ok/module-metadata-impl-generics.ari --emit-module-metadata $(BUILD_DIR)/modules/module-metadata-impl-generics.arimeta --emit-llvm $(BUILD_DIR)/modules/module-metadata-impl-generics.ll
	grep -q "item.*impl.*\\[T\\] Boxed\\[T\\]" $(BUILD_DIR)/modules/module-metadata-impl-generics.arimeta
	grep -q "item.*impl.*\\[T: Ordered\\[T\\]\\] Boxed\\[T\\]" $(BUILD_DIR)/modules/module-metadata-impl-generics.arimeta
	$(TARGET) tests/cases/modules/ok/module-metadata-impl-generics.ari --check-module-metadata $(BUILD_DIR)/modules/module-metadata-impl-generics.arimeta --check
	$(TARGET) tests/cases/modules/ok/module-metadata-impl-generics.ari -o $(BUILD_DIR)/modules/module-metadata-impl-generics.elf
	$(BUILD_DIR)/modules/module-metadata-impl-generics.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/modules/ok/module-metadata-split-impl.ari --emit-module-metadata $(BUILD_DIR)/modules/module-metadata-split-impl.arimeta --emit-llvm $(BUILD_DIR)/modules/module-metadata-split-impl.ll
	test $$(grep -c "item.*impl.*Boxed" $(BUILD_DIR)/modules/module-metadata-split-impl.arimeta) -eq 2
	awk -F '\t' '$$1 == "item" && $$3 == "impl" && $$4 == "Boxed" { print $$5 }' $(BUILD_DIR)/modules/module-metadata-split-impl.arimeta | sort -u >$(BUILD_DIR)/modules/module-metadata-split-impl.origins
	test $$(wc -l <$(BUILD_DIR)/modules/module-metadata-split-impl.origins) -eq 2
	$(TARGET) tests/cases/modules/ok/module-metadata-split-impl.ari --check-module-metadata $(BUILD_DIR)/modules/module-metadata-split-impl.arimeta --check
	$(TARGET) tests/cases/modules/ok/module-metadata-split-impl.ari -o $(BUILD_DIR)/modules/module-metadata-split-impl.elf
	$(BUILD_DIR)/modules/module-metadata-split-impl.elf; code=$$?; test $$code -eq 15
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "import") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-duplicate-import.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-duplicate-import.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-duplicate-import.ll >$(BUILD_DIR)/modules/package-module-main-duplicate-import.err 2>&1; then exit 1; fi
	grep -q "duplicate import record" $(BUILD_DIR)/modules/package-module-main-duplicate-import.err
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-module-main.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache.ll
	grep -q "ari-module-cache-v0" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -q "metadata.*ari-module-metadata-v0" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -q "source.*package_math.*tests/packages/package_math.ari" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -Eq "ast-summary.*package_math.*tests/packages/package_math.ari.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -Eq "ir-summary.*package_math.*tests/packages/package_math.ari.*ari-ir-summary-v0" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -Eq "ir-summary.*package_math.*B;" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -Eq "ir-summary.*package_math.*T;" $(BUILD_DIR)/modules/package-module-main.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-module-main-cache.ll $(BUILD_DIR)/modules/package-module-main-cache-use.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-body.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-body-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "3:var" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:block" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "6:assign" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "6:return" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "9:expr-stmt" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "2:if" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:break" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:while" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "3:for" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "10:init-while" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "8:continue" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:match" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "11:method-call" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "7:if-expr" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "10:block-expr" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "10:match-expr" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:float" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "4:null" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "6:borrow" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:unary" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "3:try" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "13:null-coalesce" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "10:macro-call" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "14:qualified-call" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:tuple" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "6:vector" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "5:array" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "31:cache_bodies::empty_unit_cached;0;0;1;0;1:C;0:;0;1;0;0;0;1;0;" $(BUILD_DIR)/modules/package-cache-body.aricache
	grep -q "4:drop" $(BUILD_DIR)/modules/package-cache-body.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-body.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-body-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-body-cache.ll $(BUILD_DIR)/modules/package-cache-body-cache-use.ll
	python3 tests/tamper_module_cache_ir_summary.py $(BUILD_DIR)/modules/package-cache-body.aricache $(BUILD_DIR)/modules/package-cache-body-ir-replay-bad-type.aricache cache_bodies '26:cache_bodies::return_value;12:cache_bodies;0:;3:i64;' '26:cache_bodies::return_value;12:cache_bodies;0:;7:Vec[i64;'
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-body-ir-replay-bad-type.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-body-ir-replay-bad-type.ll >$(BUILD_DIR)/modules/package-cache-body-ir-replay-bad-type.err 2>&1; then exit 1; fi
	grep -q "has lowered function 'cache_bodies::return_value' return type 'Vec\\[i64' but declaration summary expects 'i64'" $(BUILD_DIR)/modules/package-cache-body-ir-replay-bad-type.err
	python3 tests/tamper_module_cache_ir_summary.py $(BUILD_DIR)/modules/package-cache-body.aricache $(BUILD_DIR)/modules/package-cache-body-trait-origin-tampered.aricache cache_bodies 'G;11:impl-method;68:impl::cache_bodies__CacheBox_i64_::cache_bodies__CacheSelect::select;1;1:T;3:i64;' 'G;11:impl-method;68:impl::cache_bodies__CacheBox_i64_::cache_bodies__MissingAbcd::select;1;1:T;3:i64;'
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-body-trait-origin-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-body-trait-origin-tampered.ll >$(BUILD_DIR)/modules/package-cache-body-trait-origin-tampered.err 2>&1; then exit 1; fi
	grep -q "unknown impl method origin" $(BUILD_DIR)/modules/package-cache-body-trait-origin-tampered.err
	python3 tests/tamper_module_cache_ir_summary.py $(BUILD_DIR)/modules/package-cache-body.aricache $(BUILD_DIR)/modules/package-cache-body-call-target-tampered.aricache cache_bodies '68:impl::cache_bodies__CacheBox_i64_::cache_bodies__CacheSelect::select' '68:impl::cache_bodies__CacheBox_i64_::cache_bodies__MissingAbcd::select'
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-body-call-target-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-body-call-target-tampered.ll >$(BUILD_DIR)/modules/package-cache-body-call-target-tampered.err 2>&1; then exit 1; fi
	grep -q "calling unknown cached impl function" $(BUILD_DIR)/modules/package-cache-body-call-target-tampered.err
	sed '0,/ari-ast-decls-v0/s/ari-ast-decls-v0/ari-ast-decls-v1/' $(BUILD_DIR)/modules/package-cache-body.aricache >$(BUILD_DIR)/modules/package-cache-body-ast-non-v0.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-body-ast-non-v0.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-body-ast-non-v0.ll >$(BUILD_DIR)/modules/package-cache-body-ast-non-v0.err 2>&1; then exit 1; fi
	grep -q "expected 'ari-ast-decls-v0;'" $(BUILD_DIR)/modules/package-cache-body-ast-non-v0.err
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-layout-types-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-layout-types.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-layout-types-cache.ll
	grep -Fq "11:Vec[i64; 5]" $(BUILD_DIR)/modules/package-cache-layout-types.aricache
	grep -Fq "D;14:vector-storage;11:Vec[i64; 5];3:i64;5;" $(BUILD_DIR)/modules/package-cache-layout-types.aricache
	grep -q "%values.addr.* = alloca { i64, \\[5 x i64\\] }" $(BUILD_DIR)/modules/package-cache-layout-types-cache.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-layout-types-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-layout-types.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-layout-types-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-layout-types-cache.ll $(BUILD_DIR)/modules/package-cache-layout-types-cache-use.ll
	grep -q "%values.addr.* = alloca { i64, \\[5 x i64\\] }" $(BUILD_DIR)/modules/package-cache-layout-types-cache-use.ll
	python3 tests/tamper_module_cache_ir_summary.py $(BUILD_DIR)/modules/package-cache-layout-types.aricache $(BUILD_DIR)/modules/package-cache-layout-types-layout-tampered.aricache cache_layout_types 'D;14:vector-storage;11:Vec[i64; 5];3:i64;5;' 'D;14:vector-storage;11:Vec[i64; 5];3:i64;4;'
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-layout-types-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-layout-types-layout-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-layout-types-layout-tampered.ll >$(BUILD_DIR)/modules/package-cache-layout-types-layout-tampered.err 2>&1; then exit 1; fi
	grep -Fq "IR layout descriptor mismatch for Vec[i64; 5]" $(BUILD_DIR)/modules/package-cache-layout-types-layout-tampered.err
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-body-main.ari -o $(BUILD_DIR)/modules/package-cache-body.elf
	$(BUILD_DIR)/modules/package-cache-body.elf; code=$$?; test $$code -eq 31
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-indirect-call-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-indirect-call.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-indirect-call-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/modules/package-cache-indirect-call.aricache
	grep -q "13:indirect-call" $(BUILD_DIR)/modules/package-cache-indirect-call.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-indirect-call-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-indirect-call.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-indirect-call-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-indirect-call-cache.ll $(BUILD_DIR)/modules/package-cache-indirect-call-cache-use.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-generics-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-generics.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-generics-cache.ll
	grep -q "16:generic-function" $(BUILD_DIR)/modules/package-cache-generics.aricache
	grep -q "11:impl-method" $(BUILD_DIR)/modules/package-cache-generics.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-generics-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-generics.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-generics-cache-use.ll
	test $$(grep -c 'define i64 @"_ARNv14cache_generics15identity__G_i64"' $(BUILD_DIR)/modules/package-cache-generics-cache-use.ll) -eq 1
	test $$(grep -c 'define { i64 } @"_ARNv4impl29cache_generics__CacheBox_i64_8inherent3new"' $(BUILD_DIR)/modules/package-cache-generics-cache-use.ll) -eq 1
	test $$(grep -c 'define { i64 } @"_ARNv4impl29cache_generics__CacheBox_i64_8inherent14replace__G_i64"' $(BUILD_DIR)/modules/package-cache-generics-cache-use.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv4impl29cache_generics__CacheBox_i64_8inherent4take"' $(BUILD_DIR)/modules/package-cache-generics-cache-use.ll) -eq 1
	python3 tests/tamper_module_cache_ir_summary.py $(BUILD_DIR)/modules/package-cache-generics.aricache $(BUILD_DIR)/modules/package-cache-generics-impl-origin-tampered.aricache cache_generics '54:impl::cache_generics__CacheBox_i64_::inherent::replace' '54:impl::cache_generics__CacheBox_i64_::inherent::fakerep'
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-generics-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-generics-impl-origin-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-generics-impl-origin-tampered.ll >$(BUILD_DIR)/modules/package-cache-generics-impl-origin-tampered.err 2>&1; then exit 1; fi
	grep -q "has impl-method specialization" $(BUILD_DIR)/modules/package-cache-generics-impl-origin-tampered.err
	grep -q "unknown impl method origin" $(BUILD_DIR)/modules/package-cache-generics-impl-origin-tampered.err
	python3 tests/tamper_module_cache_ir_summary.py $(BUILD_DIR)/modules/package-cache-generics.aricache $(BUILD_DIR)/modules/package-cache-generics-arg-tampered.aricache cache_generics 'G;16:generic-function;24:cache_generics::identity;1;1:T;3:i64;' 'G;16:generic-function;24:cache_generics::identity;1;1:Z;3:i64;'
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-generics-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-generics-arg-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-generics-arg-tampered.ll >$(BUILD_DIR)/modules/package-cache-generics-arg-tampered.err 2>&1; then exit 1; fi
	grep -q "with unknown generic argument 'Z'" $(BUILD_DIR)/modules/package-cache-generics-arg-tampered.err
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-generics-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-generics.aricache -o $(BUILD_DIR)/modules/package-cache-generics.elf
	$(BUILD_DIR)/modules/package-cache-generics.elf; code=$$?; test $$code -eq 16
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-generics-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-generics.aricache --emit-obj $(BUILD_DIR)/modules/package-cache-generics.o
	readelf -h $(BUILD_DIR)/modules/package-cache-generics.o >$(BUILD_DIR)/modules/package-cache-generics-object.header
	grep -q 'Type:[[:space:]]*REL' $(BUILD_DIR)/modules/package-cache-generics-object.header
	readelf -sW $(BUILD_DIR)/modules/package-cache-generics.o >$(BUILD_DIR)/modules/package-cache-generics-object.sym
	test $$(grep -c '_ARNv14cache_generics15identity__G_i64' $(BUILD_DIR)/modules/package-cache-generics-object.sym) -eq 1
	test $$(grep -c '_ARNv4impl29cache_generics__CacheBox_i64_8inherent3new' $(BUILD_DIR)/modules/package-cache-generics-object.sym) -eq 1
	test $$(grep -c '_ARNv4impl29cache_generics__CacheBox_i64_8inherent14replace__G_i64' $(BUILD_DIR)/modules/package-cache-generics-object.sym) -eq 1
	test $$(grep -c '_ARNv4impl29cache_generics__CacheBox_i64_8inherent4take' $(BUILD_DIR)/modules/package-cache-generics-object.sym) -eq 1
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-host-literals-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-host-literals.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-host-literals-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/modules/package-cache-host-literals.aricache
	grep -q "6:string;5:cache;" $(BUILD_DIR)/modules/package-cache-host-literals.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-host-literals-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-host-literals.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-host-literals-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-host-literals-cache.ll $(BUILD_DIR)/modules/package-cache-host-literals-cache-use.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-decls-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-decls.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-decls-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/modules/package-cache-decls.aricache
	grep -Eq "ast-summary.*cache_decls.*tests/packages/cache_decls.ari.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-decls.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-decls-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-decls.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-decls-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-decls-cache.ll $(BUILD_DIR)/modules/package-cache-decls-cache-use.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-const-header-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-const-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-header-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/modules/package-cache-const-header.aricache
	grep -Eq "ast-summary.*cache_consts.*tests/packages/cache_consts.arih.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-const-header.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-const-header-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-const-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-header-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-const-header-cache.ll $(BUILD_DIR)/modules/package-cache-const-header-cache-use.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-const-aggregate-header-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -Eq "ast-summary.*cache_const_aggregates.*tests/packages/cache_const_aggregates.arih.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "5:tuple" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "6:struct" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "6:vector" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "4:call" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "4:cast" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "6:binary" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "5:unary" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "5:field" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "11:tuple-index" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "5:index" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-const-aggregate-header-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache.ll $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache-use.ll
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-const-aggregate-header-main.ari -o $(BUILD_DIR)/modules/package-cache-const-aggregate-header.elf
	$(BUILD_DIR)/modules/package-cache-const-aggregate-header.elf; code=$$?; test $$code -eq 64
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-extern-header-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-extern-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-extern-header-cache.ll
	grep -Eq "ast-summary.*extern_header.*tests/packages/extern_header.arih.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-extern-header.aricache
	$(TARGET) -Itests/packages tests/cases/modules/ok/package-cache-extern-header-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-extern-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-extern-header-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-extern-header-cache.ll $(BUILD_DIR)/modules/package-cache-extern-header-cache-use.ll
	awk 'NR == 1 { print "ari-module-cache-vnext"; next } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-non-v0.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-non-v0.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-non-v0.ll >$(BUILD_DIR)/modules/package-module-main-cache-non-v0.err 2>&1; then exit 1; fi
	grep -q "expected ari-module-cache-v0 header" $(BUILD_DIR)/modules/package-module-main-cache-non-v0.err
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "source") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.ll >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.err 2>&1; then exit 1; fi
	grep -q "duplicate source record" $(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.err
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "ast-summary") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.ll >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.err 2>&1; then exit 1; fi
	grep -q "duplicate ast-summary record" $(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.err
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "ir-summary") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-ir-summary.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-duplicate-ir-summary.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-duplicate-ir-summary.ll >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-ir-summary.err 2>&1; then exit 1; fi
	grep -q "duplicate ir-summary record" $(BUILD_DIR)/modules/package-module-main-cache-duplicate-ir-summary.err
	sed '/ast-summary.*package_math.*tests\/packages\/package_math.ari/d' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-missing-summary.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-missing-summary.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-missing-summary.ll >$(BUILD_DIR)/modules/package-module-main-cache-missing-summary.err 2>&1; then exit 1; fi
	grep -q "missing AST summary" $(BUILD_DIR)/modules/package-module-main-cache-missing-summary.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ast-summary" && $$2 == "package_math" { $$12 = "99" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.err 2>&1; then exit 1; fi
	grep -q "has function count 99 but declaration summary contains" $(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ast-summary" && $$2 == "package_math" { $$6 = "0000000000000000" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.err 2>&1; then exit 1; fi
	grep -q "declaration summary hashes to" $(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ast-summary" && $$2 == "package_math" { $$7 = "tampered" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.err 2>&1; then exit 1; fi
	grep -q "declaration summary expected 'ari-ast-decls-v0;' header" $(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ir-summary" && $$2 == "package_math" { $$8 = "99" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-tampered.err 2>&1; then exit 1; fi
	grep -q "has function count 99 but IR summary contains" $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-tampered.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ir-summary" && $$2 == "package_math" { $$6 = "0000000000000000" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-hash-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-hash-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-hash-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-hash-tampered.err 2>&1; then exit 1; fi
	grep -q "IR summary for 'package_math'.*hashes to" $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-hash-tampered.err
	sed '0,/ari-ir-summary-v0/s/ari-ir-summary-v0/ari-ir-summary-v1/' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-non-v0.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-non-v0.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-non-v0.ll >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-non-v0.err 2>&1; then exit 1; fi
	grep -q "expected 'ari-ir-summary-v0;' header" $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-non-v0.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ir-summary" && $$2 == "package_math" { pm_path = $$3; pm_root = $$4; pm_hash = $$5; next } $$1 == "ir-summary" && $$2 == "package_math::nested" { $$2 = "package_math"; $$3 = pm_path; $$4 = pm_root; $$5 = pm_hash } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-wrong-surface.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-wrong-surface.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-wrong-surface.ll >$(BUILD_DIR)/modules/package-module-main-cache-ir-summary-wrong-surface.err 2>&1; then exit 1; fi
	grep -q "contains lowered function 'package_math::nested::bonus' from module 'package_math::nested'" $(BUILD_DIR)/modules/package-module-main-cache-ir-summary-wrong-surface.err
	sed '0,/package_math::value/s/package_math::value/package_math::wrong/' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-tampered.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-tampered.err 2>&1; then exit 1; fi
	grep -q "does not match the current source graph" $(BUILD_DIR)/modules/package-module-main-cache-tampered.err
	grep -q "new item fn 'package_math::value'" $(BUILD_DIR)/modules/package-module-main-cache-tampered.err
	sed 's/\\nimport\\t\\tpackage_math\\tpackage_math\\ttests\/packages\/package_math.ari\\t0//' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-missing-import.aricache
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-missing-import.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-missing-import.ll >$(BUILD_DIR)/modules/package-module-main-cache-missing-import.err 2>&1; then exit 1; fi
	grep -q "module cache is missing validated import 'package_math'" $(BUILD_DIR)/modules/package-module-main-cache-missing-import.err
	if $(TARGET) -Itests/packages --cfg-feature stale tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-stale.ll >$(BUILD_DIR)/modules/package-module-main-cache-stale.err 2>&1; then exit 1; fi
	grep -q "current source graph has new cfg feature 'stale'" $(BUILD_DIR)/modules/package-module-main-cache-stale.err
	$(TARGET) -Itests/packages --target i686-unknown-linux-gnu tests/cases/modules/ok/package-module-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-module-main-target.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-target.ll
	if $(TARGET) -Itests/packages --target x86_64-pc-linux-gnu tests/cases/modules/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-target.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-target-stale.ll >$(BUILD_DIR)/modules/package-module-main-target-stale.err 2>&1; then exit 1; fi
	grep -q "target option changed from 'i686-unknown-linux-gnu' to 'x86_64-pc-linux-gnu'" $(BUILD_DIR)/modules/package-module-main-target-stale.err
	rm -rf $(BUILD_DIR)/modules/cache-root $(BUILD_DIR)/modules/cache-packages
	mkdir -p $(BUILD_DIR)/modules/cache-root $(BUILD_DIR)/modules/cache-packages/nested
	cp tests/cases/modules/ok/package-module-main.ari $(BUILD_DIR)/modules/cache-root/app.ari
	cp tests/packages/package_math.ari $(BUILD_DIR)/modules/cache-packages/package_math.ari
	cp tests/packages/package_header.arih $(BUILD_DIR)/modules/cache-packages/package_header.arih
	cp tests/packages/nested/mod.ari $(BUILD_DIR)/modules/cache-packages/nested/mod.ari
	$(TARGET) -I$(BUILD_DIR)/modules/cache-packages $(BUILD_DIR)/modules/cache-root/app.ari --emit-module-cache $(BUILD_DIR)/modules/cache-source.aricache --emit-llvm $(BUILD_DIR)/modules/cache-source.ll
	printf '\n// stale\n' >>$(BUILD_DIR)/modules/cache-packages/package_math.ari
	if $(TARGET) -I$(BUILD_DIR)/modules/cache-packages $(BUILD_DIR)/modules/cache-root/app.ari --use-module-cache $(BUILD_DIR)/modules/cache-source.aricache --emit-llvm $(BUILD_DIR)/modules/cache-source-stale.ll >$(BUILD_DIR)/modules/cache-source-stale.err 2>&1; then exit 1; fi
	grep -q "content hash changed" $(BUILD_DIR)/modules/cache-source-stale.err
	rm -rf $(BUILD_DIR)/modules/cache-shadow-root $(BUILD_DIR)/modules/cache-shadow-packages
	mkdir -p $(BUILD_DIR)/modules/cache-shadow-root $(BUILD_DIR)/modules/cache-shadow-packages/nested
	cp tests/cases/modules/ok/package-module-main.ari $(BUILD_DIR)/modules/cache-shadow-root/app.ari
	cp tests/packages/package_math.ari $(BUILD_DIR)/modules/cache-shadow-packages/package_math.ari
	cp tests/packages/package_header.arih $(BUILD_DIR)/modules/cache-shadow-packages/package_header.arih
	cp tests/packages/nested/mod.ari $(BUILD_DIR)/modules/cache-shadow-packages/nested/mod.ari
	$(TARGET) -I$(BUILD_DIR)/modules/cache-shadow-packages $(BUILD_DIR)/modules/cache-shadow-root/app.ari --emit-module-cache $(BUILD_DIR)/modules/cache-shadow.aricache --emit-llvm $(BUILD_DIR)/modules/cache-shadow.ll
	printf 'pub fn value() -> i64 {\n  99\n}\n' >$(BUILD_DIR)/modules/cache-shadow-root/package_math.ari
	if $(TARGET) -I$(BUILD_DIR)/modules/cache-shadow-packages $(BUILD_DIR)/modules/cache-shadow-root/app.ari --use-module-cache $(BUILD_DIR)/modules/cache-shadow.aricache --emit-llvm $(BUILD_DIR)/modules/cache-shadow-stale.ll >$(BUILD_DIR)/modules/cache-shadow-stale.err 2>&1; then exit 1; fi
	grep -q "resolved to" $(BUILD_DIR)/modules/cache-shadow-stale.err
	awk 'BEGIN { FS=OFS="\t" } NR == 1 { print "ari-module-metadata-vnext"; next } $$1 == "source" { print $$1, $$2, $$3, $$4; next } { print }' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-non-v0.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-non-v0.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-non-v0-check.ll >$(BUILD_DIR)/modules/package-module-main-non-v0-check.err 2>&1; then exit 1; fi
	grep -q "expected ari-module-metadata-v0 header" $(BUILD_DIR)/modules/package-module-main-non-v0-check.err
	if $(TARGET) -Itests/packages --cfg-feature stale tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-stale.ll >$(BUILD_DIR)/modules/package-module-main-stale.err 2>&1; then exit 1; fi
	grep -q "does not match the current source graph" $(BUILD_DIR)/modules/package-module-main-stale.err
	grep -q "new cfg feature 'stale'" $(BUILD_DIR)/modules/package-module-main-stale.err
	sed '/source.*package_math.*tests\/packages\/package_math.ari/s/[0-9a-f][0-9a-f]*$$/deadbeefdeadbeef/' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-stale-source-hash.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-stale-source-hash.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-stale-source-hash.ll >$(BUILD_DIR)/modules/package-module-main-stale-source-hash.err 2>&1; then exit 1; fi
	grep -q "content hash changed" $(BUILD_DIR)/modules/package-module-main-stale-source-hash.err
	sed '/source.*package_math.*tests\/packages\/package_math.ari/d' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-missing-source.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-missing-source.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-missing-source.ll >$(BUILD_DIR)/modules/package-module-main-missing-source.err 2>&1; then exit 1; fi
	grep -q "new source 'package_math' at 'tests/packages/package_math.ari'" $(BUILD_DIR)/modules/package-module-main-missing-source.err
	sed '/import.*package_math::nested.*tests\/packages\/nested\/mod.ari/d' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-missing-import.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-missing-import.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-missing-import.ll >$(BUILD_DIR)/modules/package-module-main-missing-import.err 2>&1; then exit 1; fi
	grep -q "new import 'package_math::nested as nested'" $(BUILD_DIR)/modules/package-module-main-missing-import.err
	sed '/item.*fn.*package_math::value/d' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-missing-item.arimeta
	if $(TARGET) -Itests/packages tests/cases/modules/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-missing-item.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-missing-item.ll >$(BUILD_DIR)/modules/package-module-main-missing-item.err 2>&1; then exit 1; fi
	grep -q "new item fn 'package_math::value'" $(BUILD_DIR)/modules/package-module-main-missing-item.err
	if $(TARGET) tests/cases/modules/errors/module-private-reexport.ari -o $(BUILD_DIR)/modules/module-private-reexport.elf >$(BUILD_DIR)/modules/module-private-reexport.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'Api::answer'" $(BUILD_DIR)/modules/module-private-reexport.err
	if $(TARGET) tests/cases/modules/errors/module-glob-private.ari -o $(BUILD_DIR)/modules/module-glob-private.elf >$(BUILD_DIR)/modules/module-glob-private.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'hidden'" $(BUILD_DIR)/modules/module-glob-private.err
	if $(TARGET) tests/cases/modules/errors/private-module-constant.ari -o $(BUILD_DIR)/modules/private-module-constant.elf >$(BUILD_DIR)/modules/private-module-constant.err 2>&1; then exit 1; fi
	grep -q "constant 'Hidden::SECRET' is not public" $(BUILD_DIR)/modules/private-module-constant.err
	if $(TARGET) tests/cases/modules/errors/module-glob-duplicate.ari -o $(BUILD_DIR)/modules/module-glob-duplicate.elf >$(BUILD_DIR)/modules/module-glob-duplicate.err 2>&1; then exit 1; fi
	grep -q "duplicate use alias 'value'" $(BUILD_DIR)/modules/module-glob-duplicate.err
	if $(TARGET) tests/cases/modules/errors/module-super-root.ari -o $(BUILD_DIR)/modules/module-super-root.elf >$(BUILD_DIR)/modules/module-super-root.err 2>&1; then exit 1; fi
	grep -q "super:: path cannot escape the root module" $(BUILD_DIR)/modules/module-super-root.err
	if $(TARGET) tests/cases/modules/errors/private-nested-module.ari -o $(BUILD_DIR)/modules/private-nested-module.elf >$(BUILD_DIR)/modules/private-nested-module.err 2>&1; then exit 1; fi
	grep -q "module 'Outer::Inner' is not public" $(BUILD_DIR)/modules/private-nested-module.err
	if $(TARGET) tests/cases/modules/errors/cyclic-import-main.ari --check >$(BUILD_DIR)/modules/cyclic-import.err 2>&1; then exit 1; fi
	grep -q "cyclic module import 'cycle_a::cycle_b::cycle_a' through source 'tests/cases/modules/errors/cycle_a.ari'" $(BUILD_DIR)/modules/cyclic-import.err
	if $(TARGET) tests/cases/modules/errors/self-import-main.ari --check >$(BUILD_DIR)/modules/self-import.err 2>&1; then exit 1; fi
	grep -q "cyclic module import 'self_import::self_import' through source 'tests/cases/modules/errors/self_import.ari'" $(BUILD_DIR)/modules/self-import.err
	if $(TARGET) tests/cases/modules/errors/ambiguous-module-main.ari --check >$(BUILD_DIR)/modules/ambiguous-module.err 2>&1; then exit 1; fi
	grep -q "ambiguous module file for 'ambiguous_module'" $(BUILD_DIR)/modules/ambiguous-module.err
	if $(TARGET) tests/cases/modules/errors/duplicate-module-decl.ari --check >$(BUILD_DIR)/modules/duplicate-module-decl.err 2>&1; then exit 1; fi
	grep -q "duplicate module 'Repeated'" $(BUILD_DIR)/modules/duplicate-module-decl.err
	if $(TARGET) tests/cases/modules/errors/unknown-imported-symbol.ari --check >$(BUILD_DIR)/modules/unknown-imported-symbol.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'missing'" $(BUILD_DIR)/modules/unknown-imported-symbol.err
	if $(TARGET) tests/cases/modules/errors/type-value-namespace-confusion.ari --check >$(BUILD_DIR)/modules/type-value-namespace-confusion.err 2>&1; then exit 1; fi
	grep -q "unknown type 'Token'" $(BUILD_DIR)/modules/type-value-namespace-confusion.err
	if $(TARGET) tests/cases/modules/errors/invalid-module-path-syntax.ari --check >$(BUILD_DIR)/modules/invalid-module-path-syntax.err 2>&1; then exit 1; fi
	grep -q "expected { after module name" $(BUILD_DIR)/modules/invalid-module-path-syntax.err
	if $(TARGET) tests/cases/modules/errors/imported-parse-error-main.ari --check >$(BUILD_DIR)/modules/imported-parse-error.err 2>&1; then exit 1; fi
	grep -q "tests/cases/modules/errors/imported_parse_error.ari:3:1: expected ; after return" $(BUILD_DIR)/modules/imported-parse-error.err
	if $(TARGET) tests/cases/modules/errors/imported-semantic-error-main.ari --check >$(BUILD_DIR)/modules/imported-semantic-error.err 2>&1; then exit 1; fi
	grep -q "tests/cases/modules/errors/imported_semantic_error.ari:2:10: unknown name 'missing_value'" $(BUILD_DIR)/modules/imported-semantic-error.err

check-operators: $(TARGET)
	mkdir -p $(BUILD_DIR)/operators
	$(TARGET) tests/cases/operators/ok/operators-numeric-suffixes.ari -o $(BUILD_DIR)/operators/operators-numeric-suffixes.elf
	$(BUILD_DIR)/operators/operators-numeric-suffixes.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/operators/ok/operators-numeric-bases.ari -o $(BUILD_DIR)/operators/operators-numeric-bases.elf
	$(BUILD_DIR)/operators/operators-numeric-bases.elf; code=$$?; test $$code -eq 33
	$(TARGET) tests/cases/operators/ok/operators-float-suffixes.ari -o $(BUILD_DIR)/operators/operators-float-suffixes.elf
	$(BUILD_DIR)/operators/operators-float-suffixes.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/operators/ok/operators-bitwise-not.ari -o $(BUILD_DIR)/operators/operators-bitwise-not.elf
	$(BUILD_DIR)/operators/operators-bitwise-not.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/operators/ok/operators-compound-assignment.ari -o $(BUILD_DIR)/operators/operators-compound-assignment.elf
	$(BUILD_DIR)/operators/operators-compound-assignment.elf; code=$$?; test $$code -eq 8
	$(TARGET) tests/cases/ir/ok/ast-builder-scalar-paths.ari -o $(BUILD_DIR)/operators/ast-builder-scalar-paths.elf
	$(BUILD_DIR)/operators/ast-builder-scalar-paths.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/ir/ok/ast-operand-helper-paths.ari -o $(BUILD_DIR)/operators/ast-operand-helper-paths.elf
	$(BUILD_DIR)/operators/ast-operand-helper-paths.elf; code=$$?; test $$code -eq 34
	$(TARGET) tests/cases/ir/ok/ast-argument-payloads.ari -o $(BUILD_DIR)/operators/ast-argument-payloads.elf
	$(BUILD_DIR)/operators/ast-argument-payloads.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/operators/ok/operators-try.ari -o $(BUILD_DIR)/operators/operators-try.elf
	$(BUILD_DIR)/operators/operators-try.elf; code=$$?; test $$code -eq 52
	$(TARGET) tests/cases/operators/ok/operators-try-residual-conversion.ari -o $(BUILD_DIR)/operators/operators-try-residual-conversion.elf
	$(BUILD_DIR)/operators/operators-try-residual-conversion.elf; code=$$?; test $$code -eq 65
	$(TARGET) tests/cases/ir/ok/ir-try-payloads.ari -o $(BUILD_DIR)/operators/ir-try-payloads.elf
	$(BUILD_DIR)/operators/ir-try-payloads.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/ir/ok/ir-enum-result-payloads.ari -o $(BUILD_DIR)/operators/ir-enum-result-payloads.elf
	$(BUILD_DIR)/operators/ir-enum-result-payloads.elf; code=$$?; test $$code -eq 35
	$(TARGET) tests/cases/operators/ok/operators-null-coalescing.ari -o $(BUILD_DIR)/operators/operators-null-coalescing.elf
	$(BUILD_DIR)/operators/operators-null-coalescing.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/operators/ok/operators-numeric-suffixes.ari --emit-llvm $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i8 -8" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i16 2" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i32 3" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i64 4" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	$(TARGET) tests/cases/operators/ok/operators-numeric-bases.ari --emit-llvm $(BUILD_DIR)/operators/operators-numeric-bases.ll
	grep -q "store i8 255" $(BUILD_DIR)/operators/operators-numeric-bases.ll
	grep -q "store i16 8" $(BUILD_DIR)/operators/operators-numeric-bases.ll
	$(TARGET) tests/cases/literals/ok/string-escapes.ari --emit-llvm $(BUILD_DIR)/operators/string-escapes.ll
	grep -Fq '\0A\09\0D\07\08\0C\0B\5C\22\27\3F\21\41\42' $(BUILD_DIR)/operators/string-escapes.ll
	grep -Fq '\F0\9F\98\80\53\00' $(BUILD_DIR)/operators/string-escapes.ll
	$(TARGET) tests/cases/operators/ok/operators-float-suffixes.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "alloca float" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "alloca double" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "store float 1.00000000000000000e+00" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "store double 1.00000000000000000e+01" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	$(TARGET) tests/cases/operators/ok/operators-float-llvm.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fadd float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fsub float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fmul float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fdiv float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fadd double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fsub double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fmul double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fdiv double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fcmp ogt double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	$(TARGET) tests/cases/operators/ok/operators-float-llvm.ari -o $(BUILD_DIR)/operators/operators-float-llvm.elf
	$(BUILD_DIR)/operators/operators-float-llvm.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/operators/ok/operators-float-casts.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-casts.ll
	grep -q "fpext float" $(BUILD_DIR)/operators/operators-float-casts.ll
	grep -q "fptrunc double" $(BUILD_DIR)/operators/operators-float-casts.ll
	$(TARGET) tests/cases/operators/ok/operators-float-casts.ari -o $(BUILD_DIR)/operators/operators-float-casts.elf
	$(BUILD_DIR)/operators/operators-float-casts.elf; code=$$?; test $$code -eq 15
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/operators/ok/operators-float-casts.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-float-casts.elf; $(BUILD_DIR)/operators/operators-float-casts.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/cases/operators/ok/operators-float-int-casts.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "sitofp i64" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "uitofp i8" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "fptosi double" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "fptoui float" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	$(TARGET) tests/cases/operators/ok/operators-float-int-casts.ari -o $(BUILD_DIR)/operators/operators-float-int-casts.elf
	$(BUILD_DIR)/operators/operators-float-int-casts.elf; code=$$?; test $$code -eq 15
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/operators/ok/operators-float-int-casts.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-float-int-casts.elf; $(BUILD_DIR)/operators/operators-float-int-casts.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/cases/operators/ok/operators-f128-llvm.ari --emit-llvm $(BUILD_DIR)/operators/operators-f128-llvm.ll
	grep -q "alloca fp128" $(BUILD_DIR)/operators/operators-f128-llvm.ll
	grep -q "fadd fp128" $(BUILD_DIR)/operators/operators-f128-llvm.ll
	grep -q "fcmp ogt fp128" $(BUILD_DIR)/operators/operators-f128-llvm.ll
	$(TARGET) tests/cases/operators/ok/operators-bitwise-not.ari --emit-llvm $(BUILD_DIR)/operators/operators-bitwise-not.ll
	grep -q "xor i64 .* -1" $(BUILD_DIR)/operators/operators-bitwise-not.ll
	grep -q "xor i8 .* -1" $(BUILD_DIR)/operators/operators-bitwise-not.ll
	$(TARGET) tests/cases/operators/ok/operators-compound-assignment.ari --emit-llvm $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "add i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "sub i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "mul i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "sdiv i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "srem i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "shl i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "ashr i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "and i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "or i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	grep -q "xor i64" $(BUILD_DIR)/operators/operators-compound-assignment.ll
	$(TARGET) tests/cases/ir/ok/ast-builder-scalar-paths.ari --emit-llvm $(BUILD_DIR)/operators/ast-builder-scalar-paths.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/operators/ast-builder-scalar-paths.ll
	grep -q "getelementptr inbounds { i64, i1 }" $(BUILD_DIR)/operators/ast-builder-scalar-paths.ll
	grep -q "call i64 @\"_ARNv7inspect\"" $(BUILD_DIR)/operators/ast-builder-scalar-paths.ll
	$(TARGET) tests/cases/ir/ok/ast-operand-helper-paths.ari --emit-llvm $(BUILD_DIR)/operators/ast-operand-helper-paths.ll
	grep -q "try.ok" $(BUILD_DIR)/operators/ast-operand-helper-paths.ll
	grep -q "getelementptr inbounds" $(BUILD_DIR)/operators/ast-operand-helper-paths.ll
	grep -q "call i64 @\"_ARNv7inspect\"" $(BUILD_DIR)/operators/ast-operand-helper-paths.ll
	grep -q "bonus" $(BUILD_DIR)/operators/ast-operand-helper-paths.ll
	$(TARGET) tests/cases/ir/ok/ast-builder-composite-paths.ari --emit-llvm $(BUILD_DIR)/operators/ast-builder-composite-paths.ll
	grep -q "@.ari.str" $(BUILD_DIR)/operators/ast-builder-composite-paths.ll
	grep -q "ptr null" $(BUILD_DIR)/operators/ast-builder-composite-paths.ll
	grep -q "insertvalue { i64, i1 }" $(BUILD_DIR)/operators/ast-builder-composite-paths.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/operators/ast-builder-composite-paths.ll
	$(TARGET) tests/cases/ir/ok/ast-argument-payloads.ari --emit-llvm $(BUILD_DIR)/operators/ast-argument-payloads.ll
	grep -q 'call i64 @"_ARNv3add"' $(BUILD_DIR)/operators/ast-argument-payloads.ll
	grep -q "insertvalue { i64, i64 }" $(BUILD_DIR)/operators/ast-argument-payloads.ll
	$(TARGET) tests/cases/operators/ok/operators-try.ari --emit-llvm $(BUILD_DIR)/operators/operators-try.ll
	grep -q "try.ok" $(BUILD_DIR)/operators/operators-try.ll
	grep -q "try.return" $(BUILD_DIR)/operators/operators-try.ll
	$(TARGET) tests/cases/operators/ok/operators-try-residual-conversion.ari --emit-llvm $(BUILD_DIR)/operators/operators-try-residual-conversion.ll
	grep -q "try.return" $(BUILD_DIR)/operators/operators-try-residual-conversion.ll
	grep -q "shl i64" $(BUILD_DIR)/operators/operators-try-residual-conversion.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/operators/ok/operators-try-residual-conversion.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-try-residual-conversion.elf; $(BUILD_DIR)/operators/operators-try-residual-conversion.elf; code=$$?; test $$code -eq 65; fi
	$(TARGET) tests/cases/ir/ok/ir-try-payloads.ari --emit-llvm $(BUILD_DIR)/operators/ir-try-payloads.ll
	grep -q "try.ok" $(BUILD_DIR)/operators/ir-try-payloads.ll
	grep -q "try.return" $(BUILD_DIR)/operators/ir-try-payloads.ll
	$(TARGET) tests/cases/ir/ok/ir-enum-result-payloads.ari --emit-llvm $(BUILD_DIR)/operators/ir-enum-result-payloads.ll
	grep -q "try.ok" $(BUILD_DIR)/operators/ir-enum-result-payloads.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/operators/ir-enum-result-payloads.ll
	$(TARGET) tests/cases/operators/ok/operators-try-aggregate.ari --emit-llvm $(BUILD_DIR)/operators/operators-try-aggregate.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/operators/operators-try-aggregate.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/operators/ok/operators-try-aggregate.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-try-aggregate.elf; $(BUILD_DIR)/operators/operators-try-aggregate.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/operators/ok/operators-null-coalescing.ari --emit-llvm $(BUILD_DIR)/operators/operators-null-coalescing.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/operators/operators-null-coalescing.ll
	grep -q "coalesce.fallback" $(BUILD_DIR)/operators/operators-null-coalescing.ll
	if $(TARGET) tests/cases/operators/errors/operators-float-int-suffix.ari -o $(BUILD_DIR)/operators/operators-float-int-suffix.elf >$(BUILD_DIR)/operators/operators-float-int-suffix.err 2>&1; then exit 1; fi
	grep -q "float literal suffix must be f32, f64, or f128" $(BUILD_DIR)/operators/operators-float-int-suffix.err
	if $(TARGET) tests/cases/operators/errors/operators-int-suffix-range.ari -o $(BUILD_DIR)/operators/operators-int-suffix-range.elf >$(BUILD_DIR)/operators/operators-int-suffix-range.err 2>&1; then exit 1; fi
	grep -q "integer literal 256 is out of range for u8" $(BUILD_DIR)/operators/operators-int-suffix-range.err
	if $(TARGET) tests/cases/operators/errors/operators-invalid-suffix.ari -o $(BUILD_DIR)/operators/operators-invalid-suffix.elf >$(BUILD_DIR)/operators/operators-invalid-suffix.err 2>&1; then exit 1; fi
	grep -q "unsupported numeric literal suffix 'usize'" $(BUILD_DIR)/operators/operators-invalid-suffix.err
	if $(TARGET) tests/cases/operators/errors/operators-invalid-binary-literal.ari -o $(BUILD_DIR)/operators/operators-invalid-binary-literal.elf >$(BUILD_DIR)/operators/operators-invalid-binary-literal.err 2>&1; then exit 1; fi
	grep -q "invalid digit '2' in binary literal" $(BUILD_DIR)/operators/operators-invalid-binary-literal.err
	if $(TARGET) tests/cases/literals/errors/string-invalid-hex-escape.ari -o $(BUILD_DIR)/operators/string-invalid-hex-escape.elf >$(BUILD_DIR)/operators/string-invalid-hex-escape.err 2>&1; then exit 1; fi
	grep -q "\\\\x escape requires at least one digit" $(BUILD_DIR)/operators/string-invalid-hex-escape.err
	if $(TARGET) tests/cases/literals/errors/string-invalid-unicode-escape.ari -o $(BUILD_DIR)/operators/string-invalid-unicode-escape.elf >$(BUILD_DIR)/operators/string-invalid-unicode-escape.err 2>&1; then exit 1; fi
	grep -q "invalid Unicode scalar value in string escape" $(BUILD_DIR)/operators/string-invalid-unicode-escape.err
	if $(TARGET) tests/cases/operators/errors/operators-question.ari -o $(BUILD_DIR)/operators/operators-question.elf >$(BUILD_DIR)/operators/operators-question.err 2>&1; then exit 1; fi
	grep -q "postfix ? can only early-return from functions returning MaybeI64, got i32" $(BUILD_DIR)/operators/operators-question.err
	if $(TARGET) tests/cases/operators/errors/operators-try-residual-mismatch.ari -o $(BUILD_DIR)/operators/operators-try-residual-mismatch.elf >$(BUILD_DIR)/operators/operators-try-residual-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i32" $(BUILD_DIR)/operators/operators-try-residual-mismatch.err
	if $(TARGET) tests/cases/operators/errors/operators-question-non-enum.ari -o $(BUILD_DIR)/operators/operators-question-non-enum.elf >$(BUILD_DIR)/operators/operators-question-non-enum.err 2>&1; then exit 1; fi
	grep -q "postfix ? requires an enum value, got i64" $(BUILD_DIR)/operators/operators-question-non-enum.err
	if $(TARGET) tests/cases/operators/errors/operator-null-coalescing-non-enum.ari -o $(BUILD_DIR)/operators/operator-null-coalescing-non-enum.elf >$(BUILD_DIR)/operators/operator-null-coalescing-non-enum.err 2>&1; then exit 1; fi
	grep -q "operator ?? requires an Option/Result-style enum value, got i64" $(BUILD_DIR)/operators/operator-null-coalescing-non-enum.err
	if $(TARGET) tests/cases/operators/errors/operators-bitwise-not-bool.ari -o $(BUILD_DIR)/operators/operators-bitwise-not-bool.elf >$(BUILD_DIR)/operators/operators-bitwise-not-bool.err 2>&1; then exit 1; fi
	grep -q "bitwise-not operand must be integer, got bool" $(BUILD_DIR)/operators/operators-bitwise-not-bool.err
	if $(TARGET) tests/cases/operators/errors/operators-compound-immutable.ari -o $(BUILD_DIR)/operators/operators-compound-immutable.elf >$(BUILD_DIR)/operators/operators-compound-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'value'" $(BUILD_DIR)/operators/operators-compound-immutable.err

check-generics: $(TARGET)
	mkdir -p $(BUILD_DIR)/generics
	$(TARGET) tests/cases/functions/ok/function-generic-call.ari -o $(BUILD_DIR)/generics/function-generic-call.elf
	$(BUILD_DIR)/generics/function-generic-call.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/vectors/ok/vector-generic-function-param.ari --emit-llvm $(BUILD_DIR)/generics/vector-generic-function-param.ll
	grep -q "identity__G_Vec" $(BUILD_DIR)/generics/vector-generic-function-param.ll
	$(TARGET) tests/cases/vectors/ok/vector-generic-root-param.ari --emit-llvm $(BUILD_DIR)/generics/vector-generic-root-param.ll
	grep -q "length_of__G_i64" $(BUILD_DIR)/generics/vector-generic-root-param.ll
	grep -Fq 'define i64 @"_ARNv16length_of__G_i64"({ ptr, i64 } %arg0)' $(BUILD_DIR)/generics/vector-generic-root-param.ll
	! grep -q "length_of__G_.*cap" $(BUILD_DIR)/generics/vector-generic-root-param.ll
	$(TARGET) tests/cases/vectors/ok/vector-generic-root-param.ari -o $(BUILD_DIR)/generics/vector-generic-root-param.elf
	$(BUILD_DIR)/generics/vector-generic-root-param.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/generics/ok/generic-function-specializations.ari -o $(BUILD_DIR)/generics/generic-function-specializations.elf
	$(BUILD_DIR)/generics/generic-function-specializations.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/generics/ok/generic-function-declaration-policy.ari -o $(BUILD_DIR)/generics/generic-function-declaration-policy.elf
	$(BUILD_DIR)/generics/generic-function-declaration-policy.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/generics/ok/generic-function-declaration-policy.ari --emit-llvm $(BUILD_DIR)/generics/generic-function-declaration-policy.ll
	test $$(grep -c 'define i64 @"_ARNv19marker_value__G_i64"' $(BUILD_DIR)/generics/generic-function-declaration-policy.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv20marker_value__G_bool"' $(BUILD_DIR)/generics/generic-function-declaration-policy.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv17make_value__G_i64"' $(BUILD_DIR)/generics/generic-function-declaration-policy.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv28keep_with_marker__G_i64_bool"' $(BUILD_DIR)/generics/generic-function-declaration-policy.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv24choose_first__G_i64_bool"' $(BUILD_DIR)/generics/generic-function-declaration-policy.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv22shadow_concrete__G_i64"' $(BUILD_DIR)/generics/generic-function-declaration-policy.ll) -eq 1
	$(TARGET) tests/cases/generics/ok/generic-function-modules.ari -o $(BUILD_DIR)/generics/generic-function-modules.elf
	$(BUILD_DIR)/generics/generic-function-modules.elf; code=$$?; test $$code -eq 15
	$(TARGET) tests/cases/generics/ok/generic-function-modules.ari --emit-llvm $(BUILD_DIR)/generics/generic-function-modules.ll
	test $$(grep -c 'define i64 @"_ARNv13ParserHelpers15identity__G_i64"' $(BUILD_DIR)/generics/generic-function-modules.ll) -eq 1
	test $$(grep -c 'define i1 @"_ARNv13ParserHelpers16identity__G_bool"' $(BUILD_DIR)/generics/generic-function-modules.ll) -eq 1
	test $$(grep -c 'define i64 @"_ARNv12OtherHelpers15identity__G_i64"' $(BUILD_DIR)/generics/generic-function-modules.ll) -eq 1
	$(TARGET) tests/cases/generics/ok/generic-function-compiler-shaped.ari -o $(BUILD_DIR)/generics/generic-function-compiler-shaped.elf
	$(BUILD_DIR)/generics/generic-function-compiler-shaped.elf; code=$$?; test $$code -eq 50
	$(TARGET) tests/cases/generics/ok/generic-function-compiler-shaped.ari --emit-llvm $(BUILD_DIR)/generics/generic-function-compiler-shaped.ll
	test $$(grep -c 'define .*"_ARNv17make_box__G_Token"' $(BUILD_DIR)/generics/generic-function-compiler-shaped.ll) -eq 1
	test $$(grep -c 'define .*"_ARNv32pass_or__G_Box_Token__Diagnostic"' $(BUILD_DIR)/generics/generic-function-compiler-shaped.ll) -eq 1
	grep -q "_ARNv46identity__G_PassResult_Box_Token___Diagnostic_" $(BUILD_DIR)/generics/generic-function-compiler-shaped.ll
	grep -q "_ARNv17maybe_or__G_Token" $(BUILD_DIR)/generics/generic-function-compiler-shaped.ll
	$(TARGET) tests/cases/generics/ok/generic-function-pointer.ari -o $(BUILD_DIR)/generics/generic-function-pointer.elf
	$(BUILD_DIR)/generics/generic-function-pointer.elf; code=$$?; test $$code -eq 54
	$(TARGET) tests/cases/generics/ok/generic-function-pointer.ari --emit-llvm $(BUILD_DIR)/generics/generic-function-pointer.ll
	grep -q "identity__G_i64" $(BUILD_DIR)/generics/generic-function-pointer.ll
	grep -q "identity__G_bool" $(BUILD_DIR)/generics/generic-function-pointer.ll
	grep -q 'call i64 %' $(BUILD_DIR)/generics/generic-function-pointer.ll
	$(TARGET) tests/cases/generics/ok/generic-any-name.ari -o $(BUILD_DIR)/generics/generic-any-name.elf
	$(BUILD_DIR)/generics/generic-any-name.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/generics/ok/generic-explicit-type-args.ari -o $(BUILD_DIR)/generics/generic-explicit-type-args.elf
	$(BUILD_DIR)/generics/generic-explicit-type-args.elf; code=$$?; test $$code -eq 27
	$(TARGET) tests/cases/generics/ok/generic-explicit-type-args.ari --emit-llvm $(BUILD_DIR)/generics/generic-explicit-type-args.ll
	grep -q "identity__G_i64" $(BUILD_DIR)/generics/generic-explicit-type-args.ll
	grep -q "identity__G_bool" $(BUILD_DIR)/generics/generic-explicit-type-args.ll
	$(TARGET) tests/cases/generics/ok/generic-angle-type-annotations.ari -o $(BUILD_DIR)/generics/generic-angle-type-annotations.elf
	$(BUILD_DIR)/generics/generic-angle-type-annotations.elf; code=$$?; test $$code -eq 5
	$(TARGET) tests/cases/generics/ok/type-alias-char.ari -o $(BUILD_DIR)/generics/type-alias-char.elf
	$(BUILD_DIR)/generics/type-alias-char.elf; code=$$?; test $$code -eq 55
	$(TARGET) --no-implicit-std tests/cases/generics/ok/type-alias-no-implicit-std.ari -o $(BUILD_DIR)/generics/type-alias-no-implicit-std.elf
	$(BUILD_DIR)/generics/type-alias-no-implicit-std.elf; code=$$?; test $$code -eq 90
	if $(TARGET) tests/cases/generics/errors/type-alias-arity.ari -o $(BUILD_DIR)/generics/type-alias-arity.elf >$(BUILD_DIR)/generics/type-alias-arity.err 2>&1; then exit 1; fi
	grep -q "type alias 'Boxed' expects 1 type argument" $(BUILD_DIR)/generics/type-alias-arity.err
	$(TARGET) tests/cases/ir/ok/ast-type-argument-payloads.ari -o $(BUILD_DIR)/generics/ast-type-argument-payloads.elf
	$(BUILD_DIR)/generics/ast-type-argument-payloads.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/ir/ok/ast-type-argument-payloads.ari --emit-llvm $(BUILD_DIR)/generics/ast-type-argument-payloads.ll
	grep -q "identity__G_i64" $(BUILD_DIR)/generics/ast-type-argument-payloads.ll
	grep -q "insertvalue { i32, i64 }" $(BUILD_DIR)/generics/ast-type-argument-payloads.ll
	$(TARGET) tests/cases/generics/ok/generic-angle-call-index.ari -o $(BUILD_DIR)/generics/generic-angle-call-index.elf
	$(BUILD_DIR)/generics/generic-angle-call-index.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/structs/ok/struct-surface.ari -o $(BUILD_DIR)/generics/struct-surface.elf
	$(BUILD_DIR)/generics/struct-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/traits/ok/trait-generic-impl.ari -o $(BUILD_DIR)/generics/trait-generic-impl.elf
	$(BUILD_DIR)/generics/trait-generic-impl.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/generics/ok/generic-adt-surface.ari -o $(BUILD_DIR)/generics/generic-adt-surface.elf
	$(BUILD_DIR)/generics/generic-adt-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/generics/ok/generic-aggregate-monomorphization.ari -o $(BUILD_DIR)/generics/generic-aggregate-monomorphization.elf
	$(BUILD_DIR)/generics/generic-aggregate-monomorphization.elf; code=$$?; test $$code -eq 34
	$(TARGET) tests/cases/generics/ok/generic-aggregate-monomorphization.ari --emit-llvm $(BUILD_DIR)/generics/generic-aggregate-monomorphization.ll
	grep -q "impl10Box_Token_8inherent6unwrap" $(BUILD_DIR)/generics/generic-aggregate-monomorphization.ll
	grep -Fq "[128 x i8]" $(BUILD_DIR)/generics/generic-aggregate-monomorphization.ll
	$(TARGET) tests/cases/generics/ok/generic-aggregate-stdlib-stress.ari -o $(BUILD_DIR)/generics/generic-aggregate-stdlib-stress.elf
	$(BUILD_DIR)/generics/generic-aggregate-stdlib-stress.elf; code=$$?; test $$code -eq 94
	$(TARGET) tests/cases/generics/ok/generic-aggregate-recursive-pointer.ari --check
	$(TARGET) tests/cases/generics/ok/generic-enum-constructors.ari --emit-llvm $(BUILD_DIR)/generics/generic-enum-constructors.ll
	grep -q "insertvalue { i32, i64 }" $(BUILD_DIR)/generics/generic-enum-constructors.ll
	$(TARGET) tests/cases/generics/ok/generic-enum-try.ari --emit-llvm $(BUILD_DIR)/generics/generic-enum-try.ll
	grep -q "try.ok" $(BUILD_DIR)/generics/generic-enum-try.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/generics/generic-enum-try.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/generics/generic-enum-try.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-enum-try.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/generics/generic-enum-try.elf; $(BUILD_DIR)/generics/generic-enum-try.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/generics/ok/generic-enum-patterns.ari -o $(BUILD_DIR)/generics/generic-enum-patterns.elf
	$(BUILD_DIR)/generics/generic-enum-patterns.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/generics/ok/generic-enum-patterns.ari --emit-llvm $(BUILD_DIR)/generics/generic-enum-patterns.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/generics/generic-enum-patterns.ll
	grep -q "lshr" $(BUILD_DIR)/generics/generic-enum-patterns.ll
	$(TARGET) tests/cases/generics/ok/generic-enum-constants.ari -o $(BUILD_DIR)/generics/generic-enum-constants.elf
	$(BUILD_DIR)/generics/generic-enum-constants.elf; code=$$?; test $$code -eq 7
	if $(TARGET) tests/cases/generics/errors/generic-inference-conflict.ari -o $(BUILD_DIR)/generics/generic-inference-conflict.elf >$(BUILD_DIR)/generics/generic-inference-conflict.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' cannot be both i64 and bool" $(BUILD_DIR)/generics/generic-inference-conflict.err
	grep -q "secondary label: .*previous binding for 'T' was i64" $(BUILD_DIR)/generics/generic-inference-conflict.err
	grep -q "help: make every use of 'T' agree; if explicit type arguments are present, choose one that matches the values" $(BUILD_DIR)/generics/generic-inference-conflict.err
	if $(TARGET) tests/cases/generics/errors/generic-uninferred.ari -o $(BUILD_DIR)/generics/generic-uninferred.elf >$(BUILD_DIR)/generics/generic-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' could not be inferred for 'make'" $(BUILD_DIR)/generics/generic-uninferred.err
	if $(TARGET) tests/cases/generics/errors/generic-return-context-uninferred.ari -o $(BUILD_DIR)/generics/generic-return-context-uninferred.elf >$(BUILD_DIR)/generics/generic-return-context-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' could not be inferred for 'make_value'" $(BUILD_DIR)/generics/generic-return-context-uninferred.err
	if $(TARGET) tests/cases/generics/errors/generic-signature-unknown-param.ari -o $(BUILD_DIR)/generics/generic-signature-unknown-param.elf >$(BUILD_DIR)/generics/generic-signature-unknown-param.err 2>&1; then exit 1; fi
	grep -q "unknown type 'U'" $(BUILD_DIR)/generics/generic-signature-unknown-param.err
	if $(TARGET) tests/cases/generics/errors/generic-signature-unknown-return.ari -o $(BUILD_DIR)/generics/generic-signature-unknown-return.elf >$(BUILD_DIR)/generics/generic-signature-unknown-return.err 2>&1; then exit 1; fi
	grep -q "unknown type 'U'" $(BUILD_DIR)/generics/generic-signature-unknown-return.err
	if $(TARGET) tests/cases/generics/errors/generic-enum-constructor-uninferred.ari -o $(BUILD_DIR)/generics/generic-enum-constructor-uninferred.elf >$(BUILD_DIR)/generics/generic-enum-constructor-uninferred.err 2>&1; then exit 1; fi
	grep -q "cannot infer type argument 'T' for enum case 'std::None'" $(BUILD_DIR)/generics/generic-enum-constructor-uninferred.err
	if $(TARGET) tests/cases/generics/errors/generic-enum-constructor-arity.ari -o $(BUILD_DIR)/generics/generic-enum-constructor-arity.elf >$(BUILD_DIR)/generics/generic-enum-constructor-arity.err 2>&1; then exit 1; fi
	grep -q "enum 'std::Option' expects 1 type argument" $(BUILD_DIR)/generics/generic-enum-constructor-arity.err
	if $(TARGET) tests/cases/generics/errors/generic-enum-constructor-mismatch.ari -o $(BUILD_DIR)/generics/generic-enum-constructor-mismatch.elf >$(BUILD_DIR)/generics/generic-enum-constructor-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected std::Option\\[i64\\], got std::Option\\[bool\\]" $(BUILD_DIR)/generics/generic-enum-constructor-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-unknown-type-argument.ari --check >$(BUILD_DIR)/generics/generic-aggregate-unknown-type-argument.err 2>&1; then exit 1; fi
	grep -q "unknown type 'Missing'" $(BUILD_DIR)/generics/generic-aggregate-unknown-type-argument.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-nongeneric-type-args.ari --check >$(BUILD_DIR)/generics/generic-aggregate-nongeneric-type-args.err 2>&1; then exit 1; fi
	grep -q "struct 'Span' expects 0 type arguments" $(BUILD_DIR)/generics/generic-aggregate-nongeneric-type-args.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-enum-arity.ari --check >$(BUILD_DIR)/generics/generic-aggregate-enum-arity.err 2>&1; then exit 1; fi
	grep -q "enum 'Either' expects 2 type arguments" $(BUILD_DIR)/generics/generic-aggregate-enum-arity.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-alias-nongeneric-type-args.ari --check >$(BUILD_DIR)/generics/generic-aggregate-alias-nongeneric-type-args.err 2>&1; then exit 1; fi
	grep -q "type alias 'TokenAlias' expects 0 type arguments" $(BUILD_DIR)/generics/generic-aggregate-alias-nongeneric-type-args.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-field-mismatch.ari --check >$(BUILD_DIR)/generics/generic-aggregate-field-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected Token, got i64" $(BUILD_DIR)/generics/generic-aggregate-field-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-nested-mismatch.ari --check >$(BUILD_DIR)/generics/generic-aggregate-nested-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected Box\\[Box\\[Token\\]\\], got Box\\[Box\\[Span\\]\\]" $(BUILD_DIR)/generics/generic-aggregate-nested-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-enum-payload-mismatch.ari --check >$(BUILD_DIR)/generics/generic-aggregate-enum-payload-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected Token, got LexError" $(BUILD_DIR)/generics/generic-aggregate-enum-payload-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-field-access.ari --check >$(BUILD_DIR)/generics/generic-aggregate-field-access.err 2>&1; then exit 1; fi
	grep -q "struct 'Box' has no field 'missing'" $(BUILD_DIR)/generics/generic-aggregate-field-access.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-method-receiver.ari --check >$(BUILD_DIR)/generics/generic-aggregate-method-receiver.err 2>&1; then exit 1; fi
	grep -q "unknown method 'id' for type Box\\[bool\\]" $(BUILD_DIR)/generics/generic-aggregate-method-receiver.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-use-after-move.ari --check >$(BUILD_DIR)/generics/generic-aggregate-use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'value.value'" $(BUILD_DIR)/generics/generic-aggregate-use-after-move.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-recursive-struct.ari --check >$(BUILD_DIR)/generics/generic-aggregate-recursive-struct.err 2>&1; then exit 1; fi
	grep -q "recursive aggregate value type 'Node\\[T\\]' requires indirection" $(BUILD_DIR)/generics/generic-aggregate-recursive-struct.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-recursive-enum.ari --check >$(BUILD_DIR)/generics/generic-aggregate-recursive-enum.err 2>&1; then exit 1; fi
	grep -q "recursive aggregate value type 'Tree\\[i64\\]' requires indirection" $(BUILD_DIR)/generics/generic-aggregate-recursive-enum.err
	if $(TARGET) tests/cases/generics/errors/generic-aggregate-recursive-growth.ari --check >$(BUILD_DIR)/generics/generic-aggregate-recursive-growth.err 2>&1; then exit 1; fi
	grep -q "recursive aggregate value type 'Node\\[Wrapper\\[Wrapper\\[T\\]\\]\\]' requires indirection" $(BUILD_DIR)/generics/generic-aggregate-recursive-growth.err
	if $(TARGET) tests/cases/generics/errors/generic-enum-null-coalescing-type.ari -o $(BUILD_DIR)/generics/generic-enum-null-coalescing-type.elf >$(BUILD_DIR)/generics/generic-enum-null-coalescing-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/generics/generic-enum-null-coalescing-type.err
	if $(TARGET) tests/cases/generics/errors/generic-enum-pattern-type.ari -o $(BUILD_DIR)/generics/generic-enum-pattern-type.elf >$(BUILD_DIR)/generics/generic-enum-pattern-type.err 2>&1; then exit 1; fi
	grep -q "bool payload patterns require a bool enum payload" $(BUILD_DIR)/generics/generic-enum-pattern-type.err
	if $(TARGET) tests/cases/generics/errors/generic-enum-constant-mismatch.ari -o $(BUILD_DIR)/generics/generic-enum-constant-mismatch.elf >$(BUILD_DIR)/generics/generic-enum-constant-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected std::Option\\[i32\\], got std::Option\\[bool\\]" $(BUILD_DIR)/generics/generic-enum-constant-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-function-pointer-uninferred.ari -o $(BUILD_DIR)/generics/generic-function-pointer-uninferred.elf >$(BUILD_DIR)/generics/generic-function-pointer-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' could not be inferred for function pointer 'keep'" $(BUILD_DIR)/generics/generic-function-pointer-uninferred.err
	if $(TARGET) tests/cases/generics/errors/generic-function-pointer-mismatch.ari -o $(BUILD_DIR)/generics/generic-function-pointer-mismatch.elf >$(BUILD_DIR)/generics/generic-function-pointer-mismatch.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' cannot be both i64 and bool" $(BUILD_DIR)/generics/generic-function-pointer-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-function-value-unexpected.ari -o $(BUILD_DIR)/generics/generic-function-value-unexpected.elf >$(BUILD_DIR)/generics/generic-function-value-unexpected.err 2>&1; then exit 1; fi
	grep -q "generic function 'identity' requires an expected function pointer type or a call" $(BUILD_DIR)/generics/generic-function-value-unexpected.err
	if $(TARGET) tests/cases/generics/errors/generic-function-unknown-type-arg.ari -o $(BUILD_DIR)/generics/generic-function-unknown-type-arg.elf >$(BUILD_DIR)/generics/generic-function-unknown-type-arg.err 2>&1; then exit 1; fi
	grep -q "unknown type 'Missing'" $(BUILD_DIR)/generics/generic-function-unknown-type-arg.err
	if $(TARGET) tests/cases/generics/errors/generic-return-substitution-mismatch.ari -o $(BUILD_DIR)/generics/generic-return-substitution-mismatch.elf >$(BUILD_DIR)/generics/generic-return-substitution-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i64" $(BUILD_DIR)/generics/generic-return-substitution-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-duplicate-param.ari -o $(BUILD_DIR)/generics/generic-duplicate-param.elf >$(BUILD_DIR)/generics/generic-duplicate-param.err 2>&1; then exit 1; fi
	grep -q "duplicate generic parameter 'T' in function 'same'" $(BUILD_DIR)/generics/generic-duplicate-param.err
	if $(TARGET) tests/cases/generics/errors/generic-explicit-arity.ari -o $(BUILD_DIR)/generics/generic-explicit-arity.elf >$(BUILD_DIR)/generics/generic-explicit-arity.err 2>&1; then exit 1; fi
	grep -q "generic function 'identity' expects 1 type argument" $(BUILD_DIR)/generics/generic-explicit-arity.err
	if $(TARGET) tests/cases/generics/errors/generic-explicit-mismatch.ari -o $(BUILD_DIR)/generics/generic-explicit-mismatch.elf >$(BUILD_DIR)/generics/generic-explicit-mismatch.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' cannot be both i64 and bool" $(BUILD_DIR)/generics/generic-explicit-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-nongeneric-type-args.ari -o $(BUILD_DIR)/generics/generic-nongeneric-type-args.elf >$(BUILD_DIR)/generics/generic-nongeneric-type-args.err 2>&1; then exit 1; fi
	grep -q "function 'add' does not take type arguments" $(BUILD_DIR)/generics/generic-nongeneric-type-args.err
	if $(TARGET) tests/cases/generics/errors/generic-specialization-name-conflict.ari -o $(BUILD_DIR)/generics/generic-specialization-name-conflict.elf >$(BUILD_DIR)/generics/generic-specialization-name-conflict.err 2>&1; then exit 1; fi
	grep -q "generic specialization 'identity__G_i64' conflicts with an existing function" $(BUILD_DIR)/generics/generic-specialization-name-conflict.err
	if $(TARGET) tests/cases/generics/errors/generic-private-function.ari -o $(BUILD_DIR)/generics/generic-private-function.elf >$(BUILD_DIR)/generics/generic-private-function.err 2>&1; then exit 1; fi
	grep -q "function 'Hidden::identity' is not public" $(BUILD_DIR)/generics/generic-private-function.err
	if $(TARGET) tests/cases/variables/errors/array-literal-index-out-of-range.ari -o $(BUILD_DIR)/generics/array-literal-index-out-of-range.elf >$(BUILD_DIR)/generics/array-literal-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "array literal index 2 is out of range for 2 elements" $(BUILD_DIR)/generics/array-literal-index-out-of-range.err

check-meta: $(TARGET)
	mkdir -p $(BUILD_DIR)/meta
	$(TARGET) tests/cases/attributes/ok/attributes-builtin.ari -o $(BUILD_DIR)/meta/attributes-builtin.elf
	$(BUILD_DIR)/meta/attributes-builtin.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/attributes/ok/attributes-builtin.ari --emit-llvm $(BUILD_DIR)/meta/attributes-builtin.ll >$(BUILD_DIR)/meta/attribute-deprecated.warn 2>&1
	grep -q "warning: use of deprecated function 'old_value': use next_value" $(BUILD_DIR)/meta/attribute-deprecated.warn
	$(TARGET) tests/cases/attributes/ok/attribute-derive-debug.ari -o $(BUILD_DIR)/meta/attribute-derive-debug.elf
	$(BUILD_DIR)/meta/attribute-derive-debug.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/attributes/ok/attribute-derive-copy.ari -o $(BUILD_DIR)/meta/attribute-derive-copy.elf
	$(BUILD_DIR)/meta/attribute-derive-copy.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/attributes/ok/attribute-derive-clone.ari -o $(BUILD_DIR)/meta/attribute-derive-clone.elf
	$(BUILD_DIR)/meta/attribute-derive-clone.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/attributes/ok/attribute-derive-default.ari -o $(BUILD_DIR)/meta/attribute-derive-default.elf
	$(BUILD_DIR)/meta/attribute-derive-default.elf; code=$$?; test $$code -eq 28
	$(TARGET) tests/cases/attributes/ok/attribute-derive-enum-default.ari -o $(BUILD_DIR)/meta/attribute-derive-enum-default.elf
	$(BUILD_DIR)/meta/attribute-derive-enum-default.elf; code=$$?; test $$code -eq 37
	$(TARGET) tests/cases/attributes/ok/attribute-derive-eq.ari -o $(BUILD_DIR)/meta/attribute-derive-eq.elf
	$(BUILD_DIR)/meta/attribute-derive-eq.elf; code=$$?; test $$code -eq 50
	$(TARGET) tests/cases/attributes/ok/attribute-derive-eq-enum-payload.ari -o $(BUILD_DIR)/meta/attribute-derive-eq-enum-payload.elf
	$(BUILD_DIR)/meta/attribute-derive-eq-enum-payload.elf; code=$$?; test $$code -eq 56
	$(TARGET) tests/cases/attributes/ok/attribute-derive-partial-eq.ari -o $(BUILD_DIR)/meta/attribute-derive-partial-eq.elf
	$(BUILD_DIR)/meta/attribute-derive-partial-eq.elf; code=$$?; test $$code -eq 50
	$(TARGET) tests/cases/attributes/ok/attribute-derive-partial-eq-enum-payload.ari -o $(BUILD_DIR)/meta/attribute-derive-partial-eq-enum-payload.elf
	$(BUILD_DIR)/meta/attribute-derive-partial-eq-enum-payload.elf; code=$$?; test $$code -eq 56
	$(TARGET) tests/cases/attributes/ok/attribute-derive-ord.ari -o $(BUILD_DIR)/meta/attribute-derive-ord.elf
	$(BUILD_DIR)/meta/attribute-derive-ord.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/attributes/ok/attribute-derive-ord-enum.ari -o $(BUILD_DIR)/meta/attribute-derive-ord-enum.elf
	$(BUILD_DIR)/meta/attribute-derive-ord-enum.elf; code=$$?; test $$code -eq 78
	$(TARGET) tests/cases/attributes/ok/attribute-derive-partial-ord.ari -o $(BUILD_DIR)/meta/attribute-derive-partial-ord.elf
	$(BUILD_DIR)/meta/attribute-derive-partial-ord.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/attributes/ok/attribute-derive-partial-ord-enum.ari -o $(BUILD_DIR)/meta/attribute-derive-partial-ord-enum.elf
	$(BUILD_DIR)/meta/attribute-derive-partial-ord-enum.elf; code=$$?; test $$code -eq 78
	$(TARGET) tests/cases/attributes/ok/attribute-repr-borrow-fields.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-borrow-fields.ll
	grep -q "{ ptr, ptr, ptr }" $(BUILD_DIR)/meta/attribute-repr-borrow-fields.ll
	grep -q "extractvalue { ptr, ptr, ptr }" $(BUILD_DIR)/meta/attribute-repr-borrow-fields.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/attributes/ok/attribute-repr-borrow-fields.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/attribute-repr-borrow-fields.elf; $(BUILD_DIR)/meta/attribute-repr-borrow-fields.elf; code=$$?; test $$code -eq 38; fi
	$(TARGET) tests/cases/attributes/ok/attribute-repr-generic-pointer-fields.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.ll
	grep -q "{ ptr, ptr, ptr, i64 }" $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/attributes/ok/attribute-repr-generic-pointer-fields.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.elf; $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.elf; code=$$?; test $$code -eq 26; fi
	$(TARGET) tests/cases/attributes/ok/attribute-repr-generic-value-fields.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.ll
	grep -q "{ i64, i8 }" $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/attributes/ok/attribute-repr-generic-value-fields.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.elf; $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/attributes/ok/attribute-repr-generic-fieldless-enum.ari -o $(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.elf
	$(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/attributes/ok/attribute-repr-generic-fieldless-enum.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.ll
	$(TARGET) tests/cases/attributes/ok/attributes-user-meta.ari -o $(BUILD_DIR)/meta/attributes-user-meta.elf
	$(BUILD_DIR)/meta/attributes-user-meta.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/attributes/ok/attributes-user-meta.ari --emit-module-cache $(BUILD_DIR)/meta/attributes-user-meta.aricache --emit-llvm $(BUILD_DIR)/meta/attributes-user-meta-cache.ll
	grep -q "ari-module-cache-v0" $(BUILD_DIR)/meta/attributes-user-meta.aricache
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/meta/attributes-user-meta.aricache
	grep -q "3:u32;" $(BUILD_DIR)/meta/attributes-user-meta.aricache
	$(TARGET) tests/cases/attributes/ok/attributes-user-meta.ari --use-module-cache $(BUILD_DIR)/meta/attributes-user-meta.aricache --emit-llvm $(BUILD_DIR)/meta/attributes-user-meta-cache-use.ll
	cmp -s $(BUILD_DIR)/meta/attributes-user-meta-cache.ll $(BUILD_DIR)/meta/attributes-user-meta-cache-use.ll
	$(TARGET) tests/cases/attributes/ok/attribute-cfg.ari -o $(BUILD_DIR)/meta/attribute-cfg.elf
	$(BUILD_DIR)/meta/attribute-cfg.elf; code=$$?; test $$code -eq 10
	$(TARGET) --cfg-feature fast tests/cases/attributes/ok/attribute-cfg-feature.ari -o $(BUILD_DIR)/meta/attribute-cfg-feature.elf
	$(BUILD_DIR)/meta/attribute-cfg-feature.elf; code=$$?; test $$code -eq 13
	$(TARGET) --target x86_64-pc-windows-msvc tests/cases/attributes/ok/attribute-cfg-target.ari --emit-llvm $(BUILD_DIR)/meta/attribute-cfg-target.ll
	grep -q 'target triple = "x86_64-pc-windows-msvc"' $(BUILD_DIR)/meta/attribute-cfg-target.ll
	$(TARGET) --target x86_64-pc-linux-gnu tests/cases/attributes/ok/attribute-cfg-target-env.ari -o $(BUILD_DIR)/meta/attribute-cfg-target-env.elf
	$(BUILD_DIR)/meta/attribute-cfg-target-env.elf; code=$$?; test $$code -eq 17
	$(TARGET) --target x86_64-unknown-linux-musl tests/cases/attributes/ok/attribute-cfg-target-env.ari --emit-llvm $(BUILD_DIR)/meta/attribute-cfg-target-env-musl.ll
	grep -q 'target triple = "x86_64-unknown-linux-musl"' $(BUILD_DIR)/meta/attribute-cfg-target-env-musl.ll
	$(TARGET) --test tests/cases/attributes/ok/attribute-test-runner.ari -o $(BUILD_DIR)/meta/attribute-test-runner.elf
	$(BUILD_DIR)/meta/attribute-test-runner.elf >$(BUILD_DIR)/meta/attribute-test-runner.out 2>$(BUILD_DIR)/meta/attribute-test-runner.err; code=$$?; test $$code -eq 0
	grep -q "test smoke ..." $(BUILD_DIR)/meta/attribute-test-runner.err
	grep -q "ok returns_zero" $(BUILD_DIR)/meta/attribute-test-runner.err
	$(TARGET) test tests/cases/attributes/ok/attribute-test-filter.ari --filter selected -o $(BUILD_DIR)/meta/attribute-test-filter.elf
	$(BUILD_DIR)/meta/attribute-test-filter.elf >$(BUILD_DIR)/meta/attribute-test-filter.out 2>$(BUILD_DIR)/meta/attribute-test-filter.err; code=$$?; test $$code -eq 0
	grep -q "test selected_smoke ..." $(BUILD_DIR)/meta/attribute-test-filter.err
	if grep -q "ignored_failure" $(BUILD_DIR)/meta/attribute-test-filter.err; then exit 1; fi
	$(TARGET) --test tests/cases/attributes/ok/attribute-test-status.ari -o $(BUILD_DIR)/meta/attribute-test-status.elf
	$(BUILD_DIR)/meta/attribute-test-status.elf >$(BUILD_DIR)/meta/attribute-test-status.out 2>$(BUILD_DIR)/meta/attribute-test-status.err; code=$$?; test $$code -eq 5
	grep -q "failed second_fails" $(BUILD_DIR)/meta/attribute-test-status.err
	$(TARGET) --test tests/cases/attributes/ok/attribute-test-panic-marker.ari -o $(BUILD_DIR)/meta/attribute-test-panic-marker.elf
	$(BUILD_DIR)/meta/attribute-test-panic-marker.elf >$(BUILD_DIR)/meta/attribute-test-panic-marker.out 2>$(BUILD_DIR)/meta/attribute-test-panic-marker.err; code=$$?; test $$code -eq 1
	grep -q "test panic_marker ..." $(BUILD_DIR)/meta/attribute-test-panic-marker.err
	$(TARGET) tests/cases/attributes/ok/attribute-export-symbols.ari --shared --emit-llvm $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'define i64 @"ari_public_add"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'define i64 @"increment"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'define i64 @"reset"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'call i64 @"ari_public_add"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	$(TARGET) tests/cases/ffi/ok/c-header-export.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-export.h --emit-llvm $(BUILD_DIR)/meta/c-header-export.ll
	grep -q '#pragma once' $(BUILD_DIR)/meta/c-header-export.h
	grep -q 'int64_t ari_add_i64(int64_t arg0, int64_t arg1);' $(BUILD_DIR)/meta/c-header-export.h
	grep -q 'void ari_touch(void\* arg0, bool arg1);' $(BUILD_DIR)/meta/c-header-export.h
	if grep -q 'hidden' $(BUILD_DIR)/meta/c-header-export.h; then exit 1; fi
	$(TARGET) tests/cases/ffi/ok/c-header-repr-struct.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-repr-struct.h --emit-llvm $(BUILD_DIR)/meta/c-header-repr-struct.ll
	grep -q 'typedef int64_t ApiStatus;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'ApiStatus_Ready = 0,' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'ApiStatus_Busy = 1,' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'ApiStatus_Done = 2,' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'typedef int64_t WireStatus;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'WireStatus_Offline = 0,' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'WireStatus_Online = 1,' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'typedef struct ApiSlots ApiSlots;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'typedef struct ApiPair ApiPair;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'typedef struct GenericHandle GenericHandle;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'typedef struct GenericHandle_i64 GenericHandle_i64;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'struct ApiSlots {' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t value;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'bool flag;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'ApiStatus status;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'void\* raw;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'const int64_t\* shared;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t\* unique;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'struct ApiPair {' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t left;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t right;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'struct GenericHandle_i64 {' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t\* raw;' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_ping(int64_t arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_observe(ApiSlots\* arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_observe_ref(const int64_t\* arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_observe_generic(GenericHandle\* arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_take_generic(GenericHandle_i64 arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_sum_pair(ApiPair arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'ApiPair ari_make_pair(int64_t arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_status_code(ApiStatus arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'int64_t ari_wire_status_code(WireStatus arg0);' $(BUILD_DIR)/meta/c-header-repr-struct.h
	grep -q 'define i64 @"ari_take_generic"({ ptr } %arg0)' $(BUILD_DIR)/meta/c-header-repr-struct.ll
	grep -q 'define i64 @"ari_sum_pair"({ i64, i64 } %arg0)' $(BUILD_DIR)/meta/c-header-repr-struct.ll
	grep -q 'define { i64, i64 } @"ari_make_pair"(i64 %arg0)' $(BUILD_DIR)/meta/c-header-repr-struct.ll
	if grep -q 'HiddenSlots' $(BUILD_DIR)/meta/c-header-repr-struct.h; then exit 1; fi
	if grep -q 'HiddenStatus' $(BUILD_DIR)/meta/c-header-repr-struct.h; then exit 1; fi
	if grep -q 'struct GenericHandle {' $(BUILD_DIR)/meta/c-header-repr-struct.h; then exit 1; fi
	$(TARGET) tests/cases/ffi/ok/c-header-repr-array-fields.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-repr-array-fields.h --emit-llvm $(BUILD_DIR)/meta/c-header-repr-array-fields.ll
	grep -q 'typedef struct ApiArrayFields ApiArrayFields;' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'typedef struct ApiPairArray ApiPairArray;' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'typedef struct AriArray_i64_2 AriArray_i64_2;' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'uint8_t bytes\[4\];' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t words\[2\];' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t (\*word_ptr)\[2\];' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'const int64_t (\*word_ref)\[2\];' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t (\*word_mut)\[2\];' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'struct AriArray_i64_2 {' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t elements\[2\];' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t ari_array_fields(ApiArrayFields\* arg0);' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t ari_array_pair_sum(ApiPairArray arg0);' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t ari_array_ptr(int64_t (\*arg0)\[2\]);' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t ari_array_ref(const int64_t (\*arg0)\[2\]);' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'int64_t ari_take_array(AriArray_i64_2 arg0);' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'AriArray_i64_2 ari_make_array(int64_t arg0);' $(BUILD_DIR)/meta/c-header-repr-array-fields.h
	grep -q 'define i64 @"ari_take_array"(\[2 x i64\] %arg0)' $(BUILD_DIR)/meta/c-header-repr-array-fields.ll
	grep -q 'define \[2 x i64\] @"ari_make_array"(i64 %arg0)' $(BUILD_DIR)/meta/c-header-repr-array-fields.ll
	$(TARGET) tests/cases/ffi/ok/c-header-repr-payload-enum.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-repr-payload-enum.h --emit-llvm $(BUILD_DIR)/meta/c-header-repr-payload-enum.ll
	grep -q 'typedef struct WireValue WireValue;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'typedef struct WirePair WirePair;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'typedef struct WireSmall WireSmall;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'typedef struct WireHolder WireHolder;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'struct WireValue {' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'int32_t tag;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'uint64_t payload0;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'struct WirePair {' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'uint64_t payload1;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'struct WireSmall {' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'struct WireHolder {' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireValue value;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WirePair\* pair;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireSmall small;' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireValue_ValueEmpty = 0,' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireValue_Number = 1,' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WirePair_Pair = 1,' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireSmall_Small = 1,' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'int64_t ari_wire_value_tag(WireValue arg0);' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireValue ari_make_wire_value(int64_t arg0);' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'int64_t ari_wire_pair_ptr(WirePair\* arg0);' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'int64_t ari_wire_holder_ptr(WireHolder\* arg0);' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'WireSmall ari_make_wire_small(int32_t arg0);' $(BUILD_DIR)/meta/c-header-repr-payload-enum.h
	grep -q 'define i64 @"ari_wire_value_tag"({ i32, i64 } %arg0)' $(BUILD_DIR)/meta/c-header-repr-payload-enum.ll
	grep -q 'define { i32, i64 } @"ari_make_wire_value"(i64 %arg0)' $(BUILD_DIR)/meta/c-header-repr-payload-enum.ll
	grep -q 'define { i32, i64 } @"ari_make_wire_small"(i32 %arg0)' $(BUILD_DIR)/meta/c-header-repr-payload-enum.ll
	$(TARGET) tests/cases/ffi/ok/c-header-generated-aggregates.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-generated-aggregates.h --emit-llvm $(BUILD_DIR)/meta/c-header-generated-aggregates.ll
	grep -q 'typedef struct AriTuple_i64_i64 AriTuple_i64_i64;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'typedef struct AriEnum_MaybeWide AriEnum_MaybeWide;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'typedef struct AriVec_Vec_i64_1 AriVec_Vec_i64_1;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'struct AriTuple_i64_i64 {' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t field0;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t field1;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'struct AriEnum_MaybeWide {' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int32_t tag;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'uint64_t payload0;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'struct AriVec_Vec_i64_1 {' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t len;' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t data\[1\];' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t ari_take_tuple(AriTuple_i64_i64 arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'AriTuple_i64_i64 ari_make_tuple(int64_t arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t ari_tuple_ptr(AriTuple_i64_i64\* arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'int64_t ari_take_maybe(AriEnum_MaybeWide arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'AriEnum_MaybeWide ari_make_maybe(int64_t arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'AriVec_Vec_i64_1 _ARNv23echo__G_Vec_i64__cap_1_(AriVec_Vec_i64_1 arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates.h
	grep -q 'define i64 @"ari_take_tuple"({ i64, i64 } %arg0)' $(BUILD_DIR)/meta/c-header-generated-aggregates.ll
	grep -q 'define { i64, i64 } @"ari_make_tuple"(i64 %arg0)' $(BUILD_DIR)/meta/c-header-generated-aggregates.ll
	grep -q 'define i64 @"ari_take_maybe"({ i32, i64 } %arg0)' $(BUILD_DIR)/meta/c-header-generated-aggregates.ll
	grep -q 'define { i32, i64 } @"ari_make_maybe"(i64 %arg0)' $(BUILD_DIR)/meta/c-header-generated-aggregates.ll
	grep -q 'define { i64, \[1 x i64\] } @"_ARNv23echo__G_Vec_i64__cap_1_"({ i64, \[1 x i64\] } %arg0)' $(BUILD_DIR)/meta/c-header-generated-aggregates.ll
	$(TARGET) tests/cases/ffi/ok/c-header-generated-aggregates.ari --emit-c-header $(BUILD_DIR)/meta/c-header-generated-aggregates-object.h --emit-obj $(BUILD_DIR)/meta/c-header-generated-aggregates.o
	grep -q 'int64_t ari_take_tuple(AriTuple_i64_i64 arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates-object.h
	grep -q 'AriEnum_MaybeWide ari_make_maybe(int64_t arg0);' $(BUILD_DIR)/meta/c-header-generated-aggregates-object.h
	readelf -h $(BUILD_DIR)/meta/c-header-generated-aggregates.o >$(BUILD_DIR)/meta/c-header-generated-aggregates-object.header
	grep -q 'Type:[[:space:]]*REL' $(BUILD_DIR)/meta/c-header-generated-aggregates-object.header
	readelf -sW $(BUILD_DIR)/meta/c-header-generated-aggregates.o >$(BUILD_DIR)/meta/c-header-generated-aggregates-object.sym
	grep -q 'ari_take_tuple' $(BUILD_DIR)/meta/c-header-generated-aggregates-object.sym
	grep -q 'ari_make_maybe' $(BUILD_DIR)/meta/c-header-generated-aggregates-object.sym
	grep -q '_ARNv23echo__G_Vec_i64__cap_1_' $(BUILD_DIR)/meta/c-header-generated-aggregates-object.sym
	$(TARGET) tests/cases/ffi/ok/library-export.ari --emit-obj $(BUILD_DIR)/meta/library-export.o
	readelf -h $(BUILD_DIR)/meta/library-export.o >$(BUILD_DIR)/meta/library-export-object.header
	grep -q 'Type:[[:space:]]*REL' $(BUILD_DIR)/meta/library-export-object.header
	readelf -SW $(BUILD_DIR)/meta/library-export.o >$(BUILD_DIR)/meta/library-export-object.sections
	grep -q '\.text' $(BUILD_DIR)/meta/library-export-object.sections
	grep -q '\.symtab' $(BUILD_DIR)/meta/library-export-object.sections
	readelf -sW $(BUILD_DIR)/meta/library-export.o >$(BUILD_DIR)/meta/library-export-object.sym
	grep -q '_ARNv3add' $(BUILD_DIR)/meta/library-export-object.sym
	if grep -q '_ARNv4main' $(BUILD_DIR)/meta/library-export-object.sym; then exit 1; fi
	$(TARGET) tests/cases/ffi/ok/object-aggregate-extern-link.ari --emit-obj $(BUILD_DIR)/meta/object-aggregate-extern-link.o
	readelf -h $(BUILD_DIR)/meta/object-aggregate-extern-link.o >$(BUILD_DIR)/meta/object-aggregate-extern-link.header
	grep -q 'Type:[[:space:]]*REL' $(BUILD_DIR)/meta/object-aggregate-extern-link.header
	readelf -sW $(BUILD_DIR)/meta/object-aggregate-extern-link.o >$(BUILD_DIR)/meta/object-aggregate-extern-link.sym
	grep -q 'ari_make_object_pair' $(BUILD_DIR)/meta/object-aggregate-extern-link.sym
	grep -q 'ari_take_object_pair' $(BUILD_DIR)/meta/object-aggregate-extern-link.sym
	grep -q 'ari_test_add_i64' $(BUILD_DIR)/meta/object-aggregate-extern-link.sym
	readelf -rW $(BUILD_DIR)/meta/object-aggregate-extern-link.o >$(BUILD_DIR)/meta/object-aggregate-extern-link.relocs
	grep -q 'ari_test_add_i64' $(BUILD_DIR)/meta/object-aggregate-extern-link.relocs
	if grep -q '_ARNv4main' $(BUILD_DIR)/meta/object-aggregate-extern-link.sym; then exit 1; fi
	$(TARGET) tests/cases/ffi/ok/shared-visibility.ari --shared --emit-llvm $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define hidden i64 @"_ARNv6helper"' $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define i64 @"_ARNv10public_api"' $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define i64 @"ari_named_private"' $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define hidden i64 @ari_builtin_write_i64' $(BUILD_DIR)/meta/shared-visibility.ll
	$(TARGET) tests/cases/ffi/ok/shared-visibility.ari --emit-obj $(BUILD_DIR)/meta/shared-visibility.o
	readelf -sW $(BUILD_DIR)/meta/shared-visibility.o >$(BUILD_DIR)/meta/shared-visibility-object.sym
	grep -Eq 'HIDDEN[[:space:]]+[0-9]+[[:space:]]+_ARNv6helper' $(BUILD_DIR)/meta/shared-visibility-object.sym
	grep -Eq 'DEFAULT[[:space:]]+[0-9]+[[:space:]]+_ARNv10public_api' $(BUILD_DIR)/meta/shared-visibility-object.sym
	grep -Eq 'DEFAULT[[:space:]]+[0-9]+[[:space:]]+ari_named_private' $(BUILD_DIR)/meta/shared-visibility-object.sym
	grep -Eq 'HIDDEN[[:space:]]+[0-9]+[[:space:]]+ari_builtin_write_i64' $(BUILD_DIR)/meta/shared-visibility-object.sym
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v nm >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/shared-visibility.ari --shared --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/shared-visibility.so; nm -D $(BUILD_DIR)/meta/shared-visibility.so >$(BUILD_DIR)/meta/shared-visibility.sym; grep -q '_ARNv10public_api' $(BUILD_DIR)/meta/shared-visibility.sym; grep -q 'ari_named_private' $(BUILD_DIR)/meta/shared-visibility.sym; if grep -Eq '_ARNv6helper|_ARNv4main|ari_builtin_' $(BUILD_DIR)/meta/shared-visibility.sym; then exit 1; fi; fi
	$(TARGET) tests/cases/ffi/ok/shared-visibility.ari --emit-llvm $(BUILD_DIR)/meta/shared-visibility-exe.ll
	grep -q 'define i64 @"_ARNv6helper"' $(BUILD_DIR)/meta/shared-visibility-exe.ll
	$(TARGET) tests/cases/meta/ok/meta-function-surface.ari -o $(BUILD_DIR)/meta/meta-function-surface.elf
	$(BUILD_DIR)/meta/meta-function-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/meta/ok/meta-function-identity-body.ari -o $(BUILD_DIR)/meta/meta-function-identity-body.elf
	$(BUILD_DIR)/meta/meta-function-identity-body.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/meta/ok/macro-token-tree-syntax.ari -o $(BUILD_DIR)/meta/macro-token-tree-syntax.elf
	$(BUILD_DIR)/meta/macro-token-tree-syntax.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/meta/ok/macro-call-identity.ari -o $(BUILD_DIR)/meta/macro-call-identity.elf
	$(BUILD_DIR)/meta/macro-call-identity.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/meta/ok/meta-function-token-stream-return.ari -o $(BUILD_DIR)/meta/meta-function-token-stream-return.elf
	$(BUILD_DIR)/meta/meta-function-token-stream-return.elf; code=$$?; test $$code -eq 40
	$(TARGET) tests/cases/meta/ok/meta-function-token-branch.ari -o $(BUILD_DIR)/meta/meta-function-token-branch.elf
	$(BUILD_DIR)/meta/meta-function-token-branch.elf; code=$$?; test $$code -eq 43
	$(TARGET) tests/cases/meta/ok/meta-function-token-count.ari -o $(BUILD_DIR)/meta/meta-function-token-count.elf
	$(BUILD_DIR)/meta/meta-function-token-count.elf; code=$$?; test $$code -eq 60
	$(TARGET) tests/cases/meta/ok/meta-function-token-match.ari -o $(BUILD_DIR)/meta/meta-function-token-match.elf
	$(BUILD_DIR)/meta/meta-function-token-match.elf; code=$$?; test $$code -eq 70
	$(TARGET) tests/cases/meta/ok/meta-function-token-prefix.ari -o $(BUILD_DIR)/meta/meta-function-token-prefix.elf
	$(BUILD_DIR)/meta/meta-function-token-prefix.elf; code=$$?; test $$code -eq 58
	$(TARGET) tests/cases/meta/ok/meta-function-token-nth.ari -o $(BUILD_DIR)/meta/meta-function-token-nth.elf
	$(BUILD_DIR)/meta/meta-function-token-nth.elf; code=$$?; test $$code -eq 49
	$(TARGET) tests/cases/meta/ok/meta-function-token-suffix.ari -o $(BUILD_DIR)/meta/meta-function-token-suffix.elf
	$(BUILD_DIR)/meta/meta-function-token-suffix.elf; code=$$?; test $$code -eq 61
	$(TARGET) tests/cases/meta/ok/meta-function-token-wrapped.ari -o $(BUILD_DIR)/meta/meta-function-token-wrapped.elf
	$(BUILD_DIR)/meta/meta-function-token-wrapped.elf; code=$$?; test $$code -eq 69
	$(TARGET) tests/cases/meta/ok/meta-function-token-slice.ari -o $(BUILD_DIR)/meta/meta-function-token-slice.elf
	$(BUILD_DIR)/meta/meta-function-token-slice.elf; code=$$?; test $$code -eq 44
	$(TARGET) tests/cases/meta/ok/meta-function-token-pattern.ari -o $(BUILD_DIR)/meta/meta-function-token-pattern.elf
	$(BUILD_DIR)/meta/meta-function-token-pattern.elf; code=$$?; test $$code -eq 61
	$(TARGET) tests/cases/meta/ok/meta-function-token-capture.ari -o $(BUILD_DIR)/meta/meta-function-token-capture.elf
	$(BUILD_DIR)/meta/meta-function-token-capture.elf; code=$$?; test $$code -eq 38
	$(TARGET) tests/cases/meta/ok/meta-function-token-span-capture.ari -o $(BUILD_DIR)/meta/meta-function-token-span-capture.elf
	$(BUILD_DIR)/meta/meta-function-token-span-capture.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/meta/ok/item-macro-token-capture-splice.ari -o $(BUILD_DIR)/meta/item-macro-token-capture-splice.elf
	$(BUILD_DIR)/meta/item-macro-token-capture-splice.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-return.ari -o $(BUILD_DIR)/meta/meta-function-ast-return.elf
	$(BUILD_DIR)/meta/meta-function-ast-return.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-input-substitution.ari -o $(BUILD_DIR)/meta/meta-function-ast-input-substitution.elf
	$(BUILD_DIR)/meta/meta-function-ast-input-substitution.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-call-access.ari -o $(BUILD_DIR)/meta/meta-function-ast-call-access.elf
	$(BUILD_DIR)/meta/meta-function-ast-call-access.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-method-call.ari -o $(BUILD_DIR)/meta/meta-function-ast-method-call.elf
	$(BUILD_DIR)/meta/meta-function-ast-method-call.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-struct-literal.ari -o $(BUILD_DIR)/meta/meta-function-ast-struct-literal.elf
	$(BUILD_DIR)/meta/meta-function-ast-struct-literal.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-borrow.ari -o $(BUILD_DIR)/meta/meta-function-ast-borrow.elf
	$(BUILD_DIR)/meta/meta-function-ast-borrow.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/meta/ok/meta-function-ast-try-null-coalescing.ari -o $(BUILD_DIR)/meta/meta-function-ast-try-null-coalescing.elf
	$(BUILD_DIR)/meta/meta-function-ast-try-null-coalescing.elf; code=$$?; test $$code -eq 46
	$(TARGET) tests/cases/meta/ok/meta-function-ast-control-flow.ari -o $(BUILD_DIR)/meta/meta-function-ast-control-flow.elf
	$(BUILD_DIR)/meta/meta-function-ast-control-flow.elf; code=$$?; test $$code -eq 53
	$(TARGET) tests/cases/meta/ok/meta-function-ast-expression-kind-branch.ari -o $(BUILD_DIR)/meta/meta-function-ast-expression-kind-branch.elf
	$(BUILD_DIR)/meta/meta-function-ast-expression-kind-branch.elf; code=$$?; test $$code -eq 56
	$(TARGET) tests/cases/meta/ok/meta-function-ast-statement-body.ari -o $(BUILD_DIR)/meta/meta-function-ast-statement-body.elf
	$(BUILD_DIR)/meta/meta-function-ast-statement-body.elf; code=$$?; test $$code -eq 62
	$(TARGET) tests/cases/meta/ok/item-macro-function-identity.ari -o $(BUILD_DIR)/meta/item-macro-function-identity.elf
	$(BUILD_DIR)/meta/item-macro-function-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-ast-decl-return.ari -o $(BUILD_DIR)/meta/item-macro-ast-decl-return.elf
	$(BUILD_DIR)/meta/item-macro-ast-decl-return.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-ast-decl-input-substitution.ari -o $(BUILD_DIR)/meta/item-macro-ast-decl-input-substitution.elf
	$(BUILD_DIR)/meta/item-macro-ast-decl-input-substitution.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-decl-input-inspection.ari -o $(BUILD_DIR)/meta/meta-function-ast-decl-input-inspection.elf
	$(BUILD_DIR)/meta/meta-function-ast-decl-input-inspection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/meta-function-ast-decl-shape-inspection.ari -o $(BUILD_DIR)/meta/meta-function-ast-decl-shape-inspection.elf
	$(BUILD_DIR)/meta/meta-function-ast-decl-shape-inspection.elf; code=$$?; test $$code -eq 59
	$(TARGET) tests/cases/meta/ok/meta-function-ast-decl-member-inspection.ari -o $(BUILD_DIR)/meta/meta-function-ast-decl-member-inspection.elf
	$(BUILD_DIR)/meta/meta-function-ast-decl-member-inspection.elf; code=$$?; test $$code -eq 96
	$(TARGET) tests/cases/meta/ok/meta-function-ast-dynamic-ident.ari -o $(BUILD_DIR)/meta/meta-function-ast-dynamic-ident.elf
	$(BUILD_DIR)/meta/meta-function-ast-dynamic-ident.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/attributes/ok/attribute-macro-function-decl.ari -o $(BUILD_DIR)/meta/attribute-macro-function-decl.elf
	$(BUILD_DIR)/meta/attribute-macro-function-decl.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/attributes/ok/attribute-macro-args-token.ari -o $(BUILD_DIR)/meta/attribute-macro-args-token.elf
	$(BUILD_DIR)/meta/attribute-macro-args-token.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/attributes/ok/attribute-macro-generated-attribute.ari -o $(BUILD_DIR)/meta/attribute-macro-generated-attribute.elf
	$(BUILD_DIR)/meta/attribute-macro-generated-attribute.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-const-identity.ari -o $(BUILD_DIR)/meta/item-macro-const-identity.elf
	$(BUILD_DIR)/meta/item-macro-const-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-struct-identity.ari -o $(BUILD_DIR)/meta/item-macro-struct-identity.elf
	$(BUILD_DIR)/meta/item-macro-struct-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-enum-identity.ari -o $(BUILD_DIR)/meta/item-macro-enum-identity.elf
	$(BUILD_DIR)/meta/item-macro-enum-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-trait-impl-identity.ari -o $(BUILD_DIR)/meta/item-macro-trait-impl-identity.elf
	$(BUILD_DIR)/meta/item-macro-trait-impl-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/item-macro-module-use-identity.ari -o $(BUILD_DIR)/meta/item-macro-module-use-identity.elf
	$(BUILD_DIR)/meta/item-macro-module-use-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/type-macro-identity.ari -o $(BUILD_DIR)/meta/type-macro-identity.elf
	$(BUILD_DIR)/meta/type-macro-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/type-macro-type-return.ari -o $(BUILD_DIR)/meta/type-macro-type-return.elf
	$(BUILD_DIR)/meta/type-macro-type-return.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/pattern-macro-identity.ari -o $(BUILD_DIR)/meta/pattern-macro-identity.elf
	$(BUILD_DIR)/meta/pattern-macro-identity.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/pattern-macro-ast-return.ari -o $(BUILD_DIR)/meta/pattern-macro-ast-return.elf
	$(BUILD_DIR)/meta/pattern-macro-ast-return.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/meta/ok/pattern-macro-or-expanded.ari -o $(BUILD_DIR)/meta/pattern-macro-or-expanded.elf
	$(BUILD_DIR)/meta/pattern-macro-or-expanded.elf; code=$$?; test $$code -eq 33
	$(TARGET) tests/cases/meta/ok/pattern-macro-ref-binding-expanded.ari -o $(BUILD_DIR)/meta/pattern-macro-ref-binding-expanded.elf
	$(BUILD_DIR)/meta/pattern-macro-ref-binding-expanded.elf; code=$$?; test $$code -eq 27
	$(TARGET) tests/cases/meta/ok/pattern-macro-identity.ari --emit-module-cache $(BUILD_DIR)/meta/pattern-macro-identity.aricache --emit-llvm $(BUILD_DIR)/meta/pattern-macro-identity-cache.ll
	grep -q "ari-ast-decls-v0" $(BUILD_DIR)/meta/pattern-macro-identity.aricache
	$(TARGET) tests/cases/meta/ok/pattern-macro-identity.ari --use-module-cache $(BUILD_DIR)/meta/pattern-macro-identity.aricache --emit-llvm $(BUILD_DIR)/meta/pattern-macro-identity-cache-use.ll
	cmp -s $(BUILD_DIR)/meta/pattern-macro-identity-cache.ll $(BUILD_DIR)/meta/pattern-macro-identity-cache-use.ll
	$(TARGET) tests/cases/meta/ok/matches-macro.ari -o $(BUILD_DIR)/meta/matches-macro.elf
	$(BUILD_DIR)/meta/matches-macro.elf; code=$$?; test $$code -eq 15
	if $(TARGET) tests/cases/attributes/errors/attribute-unknown.ari -o $(BUILD_DIR)/meta/attribute-unknown.elf >$(BUILD_DIR)/meta/attribute-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown attribute '@unknown'" $(BUILD_DIR)/meta/attribute-unknown.err
	if $(TARGET) tests/cases/attributes/errors/attribute-token-tree-mismatch.ari -o $(BUILD_DIR)/meta/attribute-token-tree-mismatch.elf >$(BUILD_DIR)/meta/attribute-token-tree-mismatch.err 2>&1; then exit 1; fi
	grep -q "mismatched delimiter in attribute arguments" $(BUILD_DIR)/meta/attribute-token-tree-mismatch.err
	if $(TARGET) tests/cases/attributes/errors/attribute-placement.ari -o $(BUILD_DIR)/meta/attribute-placement.elf >$(BUILD_DIR)/meta/attribute-placement.err 2>&1; then exit 1; fi
	grep -q "attribute '@derive' is only supported on structs and enums" $(BUILD_DIR)/meta/attribute-placement.err
	if $(TARGET) tests/cases/attributes/errors/attribute-derive-unsupported.ari -o $(BUILD_DIR)/meta/attribute-derive-unsupported.elf >$(BUILD_DIR)/meta/attribute-derive-unsupported.err 2>&1; then exit 1; fi
	grep -q "unsupported derive 'Display'; supported derives: Debug, Copy, Clone, Default, Eq, PartialEq, Ord, PartialOrd" $(BUILD_DIR)/meta/attribute-derive-unsupported.err
	if $(TARGET) tests/cases/attributes/errors/attribute-derive-duplicate.ari -o $(BUILD_DIR)/meta/attribute-derive-duplicate.elf >$(BUILD_DIR)/meta/attribute-derive-duplicate.err 2>&1; then exit 1; fi
	grep -q "duplicate derive 'Debug'" $(BUILD_DIR)/meta/attribute-derive-duplicate.err
	if $(TARGET) tests/cases/attributes/errors/attribute-derive-default-enum.ari -o $(BUILD_DIR)/meta/attribute-derive-default-enum.elf >$(BUILD_DIR)/meta/attribute-derive-default-enum.err 2>&1; then exit 1; fi
	grep -q "Default derive for enums requires a case marker such as @derive(Default(CaseName))" $(BUILD_DIR)/meta/attribute-derive-default-enum.err
	if $(TARGET) tests/cases/attributes/errors/attribute-derive-default-enum-missing-case.ari -o $(BUILD_DIR)/meta/attribute-derive-default-enum-missing-case.elf >$(BUILD_DIR)/meta/attribute-derive-default-enum-missing-case.err 2>&1; then exit 1; fi
	grep -q "Default derive case 'Missing' does not name a case of enum 'State'" $(BUILD_DIR)/meta/attribute-derive-default-enum-missing-case.err
	if $(TARGET) tests/cases/attributes/errors/attribute-derive-default-struct-case.ari -o $(BUILD_DIR)/meta/attribute-derive-default-struct-case.elf >$(BUILD_DIR)/meta/attribute-derive-default-struct-case.err 2>&1; then exit 1; fi
	grep -q "Default derive case marker is only valid for enum derives" $(BUILD_DIR)/meta/attribute-derive-default-struct-case.err
	if $(TARGET) tests/cases/attributes/errors/attribute-repr-unsupported.ari -o $(BUILD_DIR)/meta/attribute-repr-unsupported.elf >$(BUILD_DIR)/meta/attribute-repr-unsupported.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr' currently supports only C layout" $(BUILD_DIR)/meta/attribute-repr-unsupported.err
	if $(TARGET) tests/cases/attributes/errors/attribute-repr-enum-payload.ari -o $(BUILD_DIR)/meta/attribute-repr-enum-payload.elf >$(BUILD_DIR)/meta/attribute-repr-enum-payload.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr(C)' generic payload enums are not supported yet" $(BUILD_DIR)/meta/attribute-repr-enum-payload.err
	if $(TARGET) tests/cases/attributes/errors/attribute-repr-enum-own-payload.ari -o $(BUILD_DIR)/meta/attribute-repr-enum-own-payload.elf >$(BUILD_DIR)/meta/attribute-repr-enum-own-payload.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr(C)' enum payloads cannot use own; expose ownership through an explicit ptr/ref ABI" $(BUILD_DIR)/meta/attribute-repr-enum-own-payload.err
	if $(TARGET) tests/cases/attributes/errors/attribute-repr-own-field.ari -o $(BUILD_DIR)/meta/attribute-repr-own-field.elf >$(BUILD_DIR)/meta/attribute-repr-own-field.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr(C)' fields cannot use own; expose ownership through an explicit ptr/ref ABI" $(BUILD_DIR)/meta/attribute-repr-own-field.err
	if $(TARGET) tests/cases/attributes/errors/attribute-repr-own-tuple-field.ari -o $(BUILD_DIR)/meta/attribute-repr-own-tuple-field.elf >$(BUILD_DIR)/meta/attribute-repr-own-tuple-field.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr(C)' fields cannot use own; expose ownership through an explicit ptr/ref ABI" $(BUILD_DIR)/meta/attribute-repr-own-tuple-field.err
	if $(TARGET) tests/cases/ffi/errors/c-header-large-aggregate-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-large-aggregate-by-value.h >$(BUILD_DIR)/meta/c-header-large-aggregate-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter Wide is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment" $(BUILD_DIR)/meta/c-header-large-aggregate-by-value.err
	if $(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/c-header-target-aggregate-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-target-aggregate-by-value.h >$(BUILD_DIR)/meta/c-header-target-aggregate-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter ApiPair is currently supported only on 64-bit Unix targets" $(BUILD_DIR)/meta/c-header-target-aggregate-by-value.err
	if $(TARGET) tests/cases/ffi/errors/c-header-large-array-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-large-array-by-value.h >$(BUILD_DIR)/meta/c-header-large-array-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter \\[i64, 3\\] is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment" $(BUILD_DIR)/meta/c-header-large-array-by-value.err
	if $(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/c-header-target-array-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-target-array-by-value.h >$(BUILD_DIR)/meta/c-header-target-array-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter \\[i64, 2\\] is currently supported only on 64-bit Unix targets" $(BUILD_DIR)/meta/c-header-target-array-by-value.err
	if $(TARGET) tests/cases/ffi/errors/c-header-large-tuple-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-large-tuple-by-value.h >$(BUILD_DIR)/meta/c-header-large-tuple-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter (i64, i64, i64) is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment" $(BUILD_DIR)/meta/c-header-large-tuple-by-value.err
	if $(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/c-header-target-tuple-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-target-tuple-by-value.h >$(BUILD_DIR)/meta/c-header-target-tuple-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter (i64, i64) is currently supported only on 64-bit Unix targets" $(BUILD_DIR)/meta/c-header-target-tuple-by-value.err
	if $(TARGET) tests/cases/ffi/errors/c-header-large-aggregate-enum-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-large-aggregate-enum-by-value.h >$(BUILD_DIR)/meta/c-header-large-aggregate-enum-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter Packet is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment" $(BUILD_DIR)/meta/c-header-large-aggregate-enum-by-value.err
	$(TARGET) tests/cases/ffi/ok/c-header-repr-enum-aggregate-payload.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h --emit-llvm $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.ll
	grep -q 'typedef struct WireTuple WireTuple;' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'typedef struct AriTuple_i64_i64 AriTuple_i64_i64;' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'struct AriTuple_i64_i64 {' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'int64_t field0;' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'int64_t field1;' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'struct WireTuple {' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'int32_t tag;' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'AriTuple_i64_i64 payload0;' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'WireTuple_Empty = 0,' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'WireTuple_Pair = 1,' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'int64_t ari_wire_tuple_ptr(WireTuple\* arg0);' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h
	grep -q 'define i64 @"ari_wire_tuple_ptr"(ptr %arg0)' $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.ll
	if command -v cc >/dev/null 2>&1; then cc -x c -fsyntax-only -include $(BUILD_DIR)/meta/c-header-repr-enum-aggregate-payload.h /dev/null; fi
	if $(TARGET) tests/cases/attributes/errors/attribute-cfg-argument.ari -o $(BUILD_DIR)/meta/attribute-cfg-argument.elf >$(BUILD_DIR)/meta/attribute-cfg-argument.err 2>&1; then exit 1; fi
	grep -q "unknown cfg predicate 'enabled'" $(BUILD_DIR)/meta/attribute-cfg-argument.err
	if $(TARGET) tests/cases/attributes/errors/attribute-deprecated-argument.ari -o $(BUILD_DIR)/meta/attribute-deprecated-argument.elf >$(BUILD_DIR)/meta/attribute-deprecated-argument.err 2>&1; then exit 1; fi
	grep -q "attribute '@deprecated' expects no arguments or one string message" $(BUILD_DIR)/meta/attribute-deprecated-argument.err
	if $(TARGET) tests/cases/attributes/errors/attribute-export-argument.ari -o $(BUILD_DIR)/meta/attribute-export-argument.elf >$(BUILD_DIR)/meta/attribute-export-argument.err 2>&1; then exit 1; fi
	grep -q "attribute '@export' expects no arguments or one string symbol name" $(BUILD_DIR)/meta/attribute-export-argument.err
	if $(TARGET) tests/cases/attributes/errors/attribute-export-symbol-invalid.ari -o $(BUILD_DIR)/meta/attribute-export-symbol-invalid.elf >$(BUILD_DIR)/meta/attribute-export-symbol-invalid.err 2>&1; then exit 1; fi
	grep -q "invalid exported symbol 'bad-symbol'" $(BUILD_DIR)/meta/attribute-export-symbol-invalid.err
	if $(TARGET) tests/cases/attributes/errors/attribute-export-mangle-collision.ari -o $(BUILD_DIR)/meta/attribute-export-mangle-collision.elf >$(BUILD_DIR)/meta/attribute-export-mangle-collision.err 2>&1; then exit 1; fi
	grep -q "emitted symbol '_ARNv6helper' for function 'collides' conflicts with function 'helper'" $(BUILD_DIR)/meta/attribute-export-mangle-collision.err
	if $(TARGET) tests/cases/attributes/errors/attribute-no-mangle-argument.ari -o $(BUILD_DIR)/meta/attribute-no-mangle-argument.elf >$(BUILD_DIR)/meta/attribute-no-mangle-argument.err 2>&1; then exit 1; fi
	grep -q "attribute '@no_mangle' does not take arguments" $(BUILD_DIR)/meta/attribute-no-mangle-argument.err
	if $(TARGET) --test tests/cases/attributes/errors/attribute-test-param.ari -o $(BUILD_DIR)/meta/attribute-test-param.elf >$(BUILD_DIR)/meta/attribute-test-param.err 2>&1; then exit 1; fi
	grep -q "@test functions cannot take parameters" $(BUILD_DIR)/meta/attribute-test-param.err
	if $(TARGET) --test --test-filter missing tests/cases/attributes/ok/attribute-test-filter.ari -o $(BUILD_DIR)/meta/attribute-test-filter-missing.elf >$(BUILD_DIR)/meta/attribute-test-filter-missing.err 2>&1; then exit 1; fi
	grep -q "test filter matched no @test functions" $(BUILD_DIR)/meta/attribute-test-filter-missing.err
	if $(TARGET) tests/cases/meta/errors/meta-function-bad-param.ari -o $(BUILD_DIR)/meta/meta-function-bad-param.elf >$(BUILD_DIR)/meta/meta-function-bad-param.err 2>&1; then exit 1; fi
	grep -q "meta function parameters must use token_stream, ast, or type, got i64" $(BUILD_DIR)/meta/meta-function-bad-param.err
	if $(TARGET) tests/cases/meta/errors/meta-function-generic.ari -o $(BUILD_DIR)/meta/meta-function-generic.elf >$(BUILD_DIR)/meta/meta-function-generic.err 2>&1; then exit 1; fi
	grep -q "meta functions cannot be generic; define one concrete meta entry point per token_stream, ast, or type transform" $(BUILD_DIR)/meta/meta-function-generic.err
	if $(TARGET) tests/cases/meta/errors/meta-function-missing-param.ari -o $(BUILD_DIR)/meta/meta-function-missing-param.elf >$(BUILD_DIR)/meta/meta-function-missing-param.err 2>&1; then exit 1; fi
	grep -q "meta functions must take exactly one token_stream, ast, or type parameter" $(BUILD_DIR)/meta/meta-function-missing-param.err
	if $(TARGET) tests/cases/meta/errors/meta-function-domain-mismatch.ari -o $(BUILD_DIR)/meta/meta-function-domain-mismatch.elf >$(BUILD_DIR)/meta/meta-function-domain-mismatch.err 2>&1; then exit 1; fi
	grep -q "meta function return type must match its input meta type" $(BUILD_DIR)/meta/meta-function-domain-mismatch.err
	if $(TARGET) tests/cases/meta/errors/meta-function-body-planned.ari -o $(BUILD_DIR)/meta/meta-function-body-planned.elf >$(BUILD_DIR)/meta/meta-function-body-planned.err 2>&1; then exit 1; fi
	grep -q 'token_stream meta function bodies currently allow only an empty body, `return input;` identity body, tokens!(...) token output, token slice/capture extraction, or expression-only `if` token branching with supported token_stream conditions' $(BUILD_DIR)/meta/meta-function-body-planned.err
	if $(TARGET) tests/cases/meta/errors/meta-function-ast-name-reference.ari -o $(BUILD_DIR)/meta/meta-function-ast-name-reference.elf >$(BUILD_DIR)/meta/meta-function-ast-name-reference.err 2>&1; then exit 1; fi
	grep -q "ast meta expression returns cannot reference names other than the meta input, generated local bindings, or local control-flow pattern bindings yet" $(BUILD_DIR)/meta/meta-function-ast-name-reference.err
	if $(TARGET) tests/cases/meta/errors/meta-function-ast-kind-value-output.ari -o $(BUILD_DIR)/meta/meta-function-ast-kind-value-output.elf >$(BUILD_DIR)/meta/meta-function-ast-kind-value-output.err 2>&1; then exit 1; fi
	grep -q "ast expression value helpers input.kind() / ast_kind(input)" $(BUILD_DIR)/meta/meta-function-ast-kind-value-output.err
	if $(TARGET) tests/cases/meta/errors/meta-function-decl-expression-site.ari -o $(BUILD_DIR)/meta/meta-function-decl-expression-site.elf >$(BUILD_DIR)/meta/meta-function-decl-expression-site.err 2>&1; then exit 1; fi
	grep -q "declaration-returning ast meta function 'make_decl' can currently be used only at item macro or attribute sites" $(BUILD_DIR)/meta/meta-function-decl-expression-site.err
	if $(TARGET) tests/cases/meta/errors/meta-function-pattern-expression-site.ari -o $(BUILD_DIR)/meta/meta-function-pattern-expression-site.elf >$(BUILD_DIR)/meta/meta-function-pattern-expression-site.err 2>&1; then exit 1; fi
	grep -q "pattern-returning ast meta function 'wildcard' can currently be used only at pattern macro sites" $(BUILD_DIR)/meta/meta-function-pattern-expression-site.err
	if $(TARGET) tests/cases/attributes/errors/attribute-meta-type-domain.ari -o $(BUILD_DIR)/meta/attribute-meta-type-domain.elf >$(BUILD_DIR)/meta/attribute-meta-type-domain.err 2>&1; then exit 1; fi
	grep -q "attributes require token_stream -> token_stream or ast -> ast" $(BUILD_DIR)/meta/attribute-meta-type-domain.err
	if $(TARGET) tests/cases/meta/errors/macro-call-type-domain.ari -o $(BUILD_DIR)/meta/macro-call-type-domain.elf >$(BUILD_DIR)/meta/macro-call-type-domain.err 2>&1; then exit 1; fi
	grep -q "expression macros require token_stream -> token_stream or ast -> ast" $(BUILD_DIR)/meta/macro-call-type-domain.err
	if $(TARGET) tests/cases/meta/errors/type-macro-invalid-input.ari -o $(BUILD_DIR)/meta/type-macro-invalid-input.elf >$(BUILD_DIR)/meta/type-macro-invalid-input.err 2>&1; then exit 1; fi
	grep -q "expected end of type macro input" $(BUILD_DIR)/meta/type-macro-invalid-input.err
	if $(TARGET) tests/cases/meta/errors/meta-function-type-constructor-empty.ari -o $(BUILD_DIR)/meta/meta-function-type-constructor-empty.elf >$(BUILD_DIR)/meta/meta-function-type-constructor-empty.err 2>&1; then exit 1; fi
	grep -q "type! type constructor requires one or more type tokens" $(BUILD_DIR)/meta/meta-function-type-constructor-empty.err
	if $(TARGET) tests/cases/meta/errors/type-macro-token-domain.ari -o $(BUILD_DIR)/meta/type-macro-token-domain.elf >$(BUILD_DIR)/meta/type-macro-token-domain.err 2>&1; then exit 1; fi
	grep -q "type macros require type -> type" $(BUILD_DIR)/meta/type-macro-token-domain.err
	if $(TARGET) tests/cases/meta/errors/type-macro-unknown.ari -o $(BUILD_DIR)/meta/type-macro-unknown.elf >$(BUILD_DIR)/meta/type-macro-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown type macro 'missing_type!'" $(BUILD_DIR)/meta/type-macro-unknown.err
	if $(TARGET) tests/cases/meta/errors/macro-call-invalid-input.ari -o $(BUILD_DIR)/meta/macro-call-invalid-input.elf >$(BUILD_DIR)/meta/macro-call-invalid-input.err 2>&1; then exit 1; fi
	grep -q "expected end of macro invocation input" $(BUILD_DIR)/meta/macro-call-invalid-input.err
	if $(TARGET) tests/cases/meta/errors/matches-macro-arity.ari -o $(BUILD_DIR)/meta/matches-macro-arity.elf >$(BUILD_DIR)/meta/matches-macro-arity.err 2>&1; then exit 1; fi
	grep -q "expected , after matches! value" $(BUILD_DIR)/meta/matches-macro-arity.err
	if $(TARGET) tests/cases/meta/errors/matches-macro-pattern-type.ari -o $(BUILD_DIR)/meta/matches-macro-pattern-type.elf >$(BUILD_DIR)/meta/matches-macro-pattern-type.err 2>&1; then exit 1; fi
	grep -q "integer match patterns must be integer literals or _" $(BUILD_DIR)/meta/matches-macro-pattern-type.err
	if $(TARGET) tests/cases/meta/errors/pattern-macro-invalid-input.ari -o $(BUILD_DIR)/meta/pattern-macro-invalid-input.elf >$(BUILD_DIR)/meta/pattern-macro-invalid-input.err 2>&1; then exit 1; fi
	grep -q "expected end of pattern macro input" $(BUILD_DIR)/meta/pattern-macro-invalid-input.err
	if $(TARGET) tests/cases/meta/errors/pattern-macro-type-domain.ari -o $(BUILD_DIR)/meta/pattern-macro-type-domain.elf >$(BUILD_DIR)/meta/pattern-macro-type-domain.err 2>&1; then exit 1; fi
	grep -q "pattern macros require token_stream -> token_stream or ast -> ast" $(BUILD_DIR)/meta/pattern-macro-type-domain.err
	if $(TARGET) tests/cases/meta/errors/pattern-macro-unknown.ari -o $(BUILD_DIR)/meta/pattern-macro-unknown.elf >$(BUILD_DIR)/meta/pattern-macro-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown pattern macro 'missing_pattern!'" $(BUILD_DIR)/meta/pattern-macro-unknown.err
	if $(TARGET) tests/cases/meta/errors/macro-call-unknown.ari -o $(BUILD_DIR)/meta/macro-call-unknown.elf >$(BUILD_DIR)/meta/macro-call-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown macro 'missing!'" $(BUILD_DIR)/meta/macro-call-unknown.err
	if $(TARGET) tests/cases/meta/errors/item-macro-unsupported-item.ari -o $(BUILD_DIR)/meta/item-macro-unsupported-item.elf >$(BUILD_DIR)/meta/item-macro-unsupported-item.err 2>&1; then exit 1; fi
	grep -q "item macro identity expansion cannot generate file-backed module imports; use an inline module output or a source-level mod declaration" $(BUILD_DIR)/meta/item-macro-unsupported-item.err
	if $(TARGET) tests/cases/meta/errors/item-macro-type-domain.ari -o $(BUILD_DIR)/meta/item-macro-type-domain.elf >$(BUILD_DIR)/meta/item-macro-type-domain.err 2>&1; then exit 1; fi
	grep -q "item macros require token_stream -> token_stream or ast -> ast" $(BUILD_DIR)/meta/item-macro-type-domain.err
	if $(TARGET) tests/cases/meta/errors/item-macro-unknown.ari -o $(BUILD_DIR)/meta/item-macro-unknown.elf >$(BUILD_DIR)/meta/item-macro-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown item macro 'missing_item!'" $(BUILD_DIR)/meta/item-macro-unknown.err

check-traits: $(TARGET)
	mkdir -p $(BUILD_DIR)/traits
	$(TARGET) tests/cases/traits/ok/trait-surface.ari -o $(BUILD_DIR)/traits/trait-surface.elf
	$(BUILD_DIR)/traits/trait-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/traits/ok/trait-object-type-surface.ari -o $(BUILD_DIR)/traits/trait-object-type-surface.elf
	$(BUILD_DIR)/traits/trait-object-type-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/traits/ok/trait-generic-impl.ari -o $(BUILD_DIR)/traits/trait-generic-impl.elf
	$(BUILD_DIR)/traits/trait-generic-impl.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/traits/ok/trait-multi-generic-impl.ari -o $(BUILD_DIR)/traits/trait-multi-generic-impl.elf
	$(BUILD_DIR)/traits/trait-multi-generic-impl.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/traits/ok/iterable-trait-surface.ari -o $(BUILD_DIR)/traits/iterable-trait-surface.elf
	$(BUILD_DIR)/traits/iterable-trait-surface.elf; code=$$?; test $$code -eq 6
	$(TARGET) tests/cases/traits/ok/trait-method-call.ari -o $(BUILD_DIR)/traits/trait-method-call.elf
	$(BUILD_DIR)/traits/trait-method-call.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/traits/ok/trait-method-call.ari --emit-llvm $(BUILD_DIR)/traits/trait-method-call.ll
	grep -q "_ARNv4impl3i647std__Eq2eq" $(BUILD_DIR)/traits/trait-method-call.ll
	$(TARGET) tests/cases/traits/ok/trait-object-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-dispatch.ll
	grep -q "dyn::vtable::i64::dyn_AddTo_i64_" $(BUILD_DIR)/traits/trait-object-dispatch.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-dispatch.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-dispatch.elf; $(BUILD_DIR)/traits/trait-object-dispatch.elf; code=$$?; test $$code -eq 11; fi
	$(TARGET) tests/cases/traits/ok/trait-object-dispatch.ari -o $(BUILD_DIR)/traits/trait-object-dispatch.elf
	$(BUILD_DIR)/traits/trait-object-dispatch.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/traits/ok/trait-object-ref-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-ref-dispatch.ll
	grep -q "dyn::vtable::i64::dyn_Score" $(BUILD_DIR)/traits/trait-object-ref-dispatch.ll
	$(TARGET) tests/cases/traits/ok/trait-object-ref-dispatch.ari -o $(BUILD_DIR)/traits/trait-object-ref-dispatch.elf
	$(BUILD_DIR)/traits/trait-object-ref-dispatch.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/trait-object-ref-mut-dispatch.ari -o $(BUILD_DIR)/traits/trait-object-ref-mut-dispatch.elf
	$(BUILD_DIR)/traits/trait-object-ref-mut-dispatch.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/trait-object-multi-arg-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.ll
	grep -q "dyn::vtable::i64::dyn_Mix_i64_" $(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.ll
	grep -q "call i64 .*ptr .* i64 3, i64 4" $(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-multi-arg-dispatch.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.elf; $(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.elf; code=$$?; test $$code -eq 36; fi
	$(TARGET) tests/cases/traits/ok/trait-object-multi-arg-dispatch.ari -o $(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.elf
	$(BUILD_DIR)/traits/trait-object-multi-arg-dispatch.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/traits/ok/trait-object-generic-impl-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.ll
	grep -q "dyn::vtable::Box_i64_::dyn_Score_i64_" $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.ll
	grep -q "_ARNv4impl8Box_i64_5Score5score" $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-generic-impl-dispatch.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.elf; $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.elf; code=$$?; test $$code -eq 11; fi
	$(TARGET) tests/cases/traits/ok/trait-object-generic-impl-dispatch.ari -o $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.elf
	$(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/traits/ok/trait-object-supertrait-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-supertrait-dispatch.ll
	grep -q "dyn::vtable::i64::dyn_ChildScore" $(BUILD_DIR)/traits/trait-object-supertrait-dispatch.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-supertrait-dispatch.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-supertrait-dispatch.elf; $(BUILD_DIR)/traits/trait-object-supertrait-dispatch.elf; code=$$?; test $$code -eq 26; fi
	$(TARGET) tests/cases/traits/ok/trait-object-supertrait-dispatch.ari -o $(BUILD_DIR)/traits/trait-object-supertrait-dispatch.elf
	$(BUILD_DIR)/traits/trait-object-supertrait-dispatch.elf; code=$$?; test $$code -eq 26
	$(TARGET) tests/cases/traits/ok/trait-object-supertrait-upcast.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-supertrait-upcast.ll
	grep -q "getelementptr ptr, ptr .* i64 1" $(BUILD_DIR)/traits/trait-object-supertrait-upcast.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-supertrait-upcast.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-supertrait-upcast.elf; $(BUILD_DIR)/traits/trait-object-supertrait-upcast.elf; code=$$?; test $$code -eq 12; fi
	$(TARGET) tests/cases/traits/ok/trait-object-supertrait-upcast.ari -o $(BUILD_DIR)/traits/trait-object-supertrait-upcast.elf
	$(BUILD_DIR)/traits/trait-object-supertrait-upcast.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/traits/ok/trait-object-ref-supertrait-upcast.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-ref-supertrait-upcast.ll
	grep -q "getelementptr ptr, ptr .* i64 1" $(BUILD_DIR)/traits/trait-object-ref-supertrait-upcast.ll
	$(TARGET) tests/cases/traits/ok/trait-object-ref-supertrait-upcast.ari -o $(BUILD_DIR)/traits/trait-object-ref-supertrait-upcast.elf
	$(BUILD_DIR)/traits/trait-object-ref-supertrait-upcast.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/traits/ok/trait-bound-static-dispatch.ari -o $(BUILD_DIR)/traits/trait-bound-static-dispatch.elf
	$(BUILD_DIR)/traits/trait-bound-static-dispatch.elf; code=$$?; test $$code -eq 6
	$(TARGET) tests/cases/traits/ok/structural-capability-parameter.ari -o $(BUILD_DIR)/traits/structural-capability-parameter.elf
	$(BUILD_DIR)/traits/structural-capability-parameter.elf; code=$$?; test $$code -eq 43
	$(TARGET) tests/cases/traits/ok/structural-capability-multi-method.ari -o $(BUILD_DIR)/traits/structural-capability-multi-method.elf
	$(BUILD_DIR)/traits/structural-capability-multi-method.elf; code=$$?; test $$code -eq 43
	$(TARGET) tests/cases/traits/ok/structural-capability-impl-method.ari -o $(BUILD_DIR)/traits/structural-capability-impl-method.elf
	$(BUILD_DIR)/traits/structural-capability-impl-method.elf; code=$$?; test $$code -eq 57
	$(TARGET) tests/cases/traits/ok/structural-capability-generic-bound.ari -o $(BUILD_DIR)/traits/structural-capability-generic-bound.elf
	$(BUILD_DIR)/traits/structural-capability-generic-bound.elf; code=$$?; test $$code -eq 62
	$(TARGET) tests/cases/traits/ok/structural-capability-alias-bound.ari -o $(BUILD_DIR)/traits/structural-capability-alias-bound.elf
	$(BUILD_DIR)/traits/structural-capability-alias-bound.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/structural-capability-generic-alias-bound.ari -o $(BUILD_DIR)/traits/structural-capability-generic-alias-bound.elf
	$(BUILD_DIR)/traits/structural-capability-generic-alias-bound.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/structural-capability-struct-bound.ari -o $(BUILD_DIR)/traits/structural-capability-struct-bound.elf
	$(BUILD_DIR)/traits/structural-capability-struct-bound.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/structural-capability-enum-bound.ari -o $(BUILD_DIR)/traits/structural-capability-enum-bound.elf
	$(BUILD_DIR)/traits/structural-capability-enum-bound.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/structural-capability-forwarded-aggregate-bound.ari -o $(BUILD_DIR)/traits/structural-capability-forwarded-aggregate-bound.elf
	$(BUILD_DIR)/traits/structural-capability-forwarded-aggregate-bound.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/traits/ok/structural-capability-forwarded-impl-body.ari -o $(BUILD_DIR)/traits/structural-capability-forwarded-impl-body.elf
	$(BUILD_DIR)/traits/structural-capability-forwarded-impl-body.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/traits/ok/structural-capability-string-result.ari -o $(BUILD_DIR)/traits/structural-capability-string-result.elf
	$(BUILD_DIR)/traits/structural-capability-string-result.elf; code=$$?; test $$code -eq 44
	$(TARGET) tests/cases/traits/ok/structural-capability-trait-method.ari -o $(BUILD_DIR)/traits/structural-capability-trait-method.elf
	$(BUILD_DIR)/traits/structural-capability-trait-method.elf; code=$$?; test $$code -eq 40
	$(TARGET) tests/cases/traits/ok/trait-minimum-compiler-shaped.ari -o $(BUILD_DIR)/traits/trait-minimum-compiler-shaped.elf
	$(BUILD_DIR)/traits/trait-minimum-compiler-shaped.elf; code=$$?; test $$code -eq 74
	$(TARGET) tests/cases/traits/ok/trait-minimum-compiler-shaped.ari --emit-llvm $(BUILD_DIR)/traits/trait-minimum-compiler-shaped.ll
	grep -q "_ARNv17same__G_TokenKind" $(BUILD_DIR)/traits/trait-minimum-compiler-shaped.ll
	grep -q "_ARNv4impl10SymbolName8Hashable9hash_code" $(BUILD_DIR)/traits/trait-minimum-compiler-shaped.ll
	grep -q "_ARNv4impl15DebugBox_Token_10Debuggable11debug_score" $(BUILD_DIR)/traits/trait-minimum-compiler-shaped.ll
	$(TARGET) tests/cases/traits/ok/trait-compiler-symbol-diagnostic-dispatch.ari -o $(BUILD_DIR)/traits/trait-compiler-symbol-diagnostic-dispatch.elf
	$(BUILD_DIR)/traits/trait-compiler-symbol-diagnostic-dispatch.elf; code=$$?; test $$code -eq 101
	$(TARGET) tests/cases/traits/ok/trait-compiler-symbol-diagnostic-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-compiler-symbol-diagnostic-dispatch.ll
	grep -q "_ARNv4impl6Symbol10CompilerEq4same" $(BUILD_DIR)/traits/trait-compiler-symbol-diagnostic-dispatch.ll
	grep -q "_ARNv4impl10Diagnostic13CompilerDebug11debug_score" $(BUILD_DIR)/traits/trait-compiler-symbol-diagnostic-dispatch.ll
	grep -q "_ARNv4impl7TypeRef12CompilerHash10hash_score" $(BUILD_DIR)/traits/trait-compiler-symbol-diagnostic-dispatch.ll
	$(TARGET) tests/cases/traits/ok/trait-compiler-generic-enum-payload.ari -o $(BUILD_DIR)/traits/trait-compiler-generic-enum-payload.elf
	$(BUILD_DIR)/traits/trait-compiler-generic-enum-payload.elf; code=$$?; test $$code -eq 32
	$(TARGET) tests/cases/traits/ok/trait-compiler-generic-enum-payload.ari --emit-llvm $(BUILD_DIR)/traits/trait-compiler-generic-enum-payload.ll
	grep -q "_ARNv40pass_score__G_Envelope_Token__Diagnostic" $(BUILD_DIR)/traits/trait-compiler-generic-enum-payload.ll
	grep -q "_ARNv4impl38PassValue_Envelope_Token___Diagnostic_13CompilerScore5score" $(BUILD_DIR)/traits/trait-compiler-generic-enum-payload.ll
	$(TARGET) tests/cases/traits/ok/trait-compiler-qualified-methods.ari -o $(BUILD_DIR)/traits/trait-compiler-qualified-methods.elf
	$(BUILD_DIR)/traits/trait-compiler-qualified-methods.elf; code=$$?; test $$code -eq 72
	$(TARGET) tests/cases/traits/ok/trait-compiler-qualified-methods.ari --emit-llvm $(BUILD_DIR)/traits/trait-compiler-qualified-methods.ll
	grep -q "_ARNv4impl6Symbol11BuildSymbol4make" $(BUILD_DIR)/traits/trait-compiler-qualified-methods.ll
	grep -q "_ARNv30generic_symbol_score__G_Symbol" $(BUILD_DIR)/traits/trait-compiler-qualified-methods.ll
	$(TARGET) tests/cases/traits/ok/trait-minimum-stdlib-like.ari -o $(BUILD_DIR)/traits/trait-minimum-stdlib-like.elf
	$(BUILD_DIR)/traits/trait-minimum-stdlib-like.elf; code=$$?; test $$code -eq 31
	$(TARGET) tests/cases/traits/ok/trait-supertrait-static.ari -o $(BUILD_DIR)/traits/trait-supertrait-static.elf
	$(BUILD_DIR)/traits/trait-supertrait-static.elf; code=$$?; test $$code -eq 56
	$(TARGET) tests/cases/traits/ok/trait-qualified-call.ari -o $(BUILD_DIR)/traits/trait-qualified-call.elf
	$(BUILD_DIR)/traits/trait-qualified-call.elf; code=$$?; test $$code -eq 150
	$(TARGET) tests/cases/vectors/ok/vector-trait-method-param-view.ari -o $(BUILD_DIR)/traits/vector-trait-method-param-view.elf
	$(BUILD_DIR)/traits/vector-trait-method-param-view.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/vectors/ok/vector-trait-method-param-view.ari --emit-llvm $(BUILD_DIR)/traits/vector-trait-method-param-view.ll
	grep -Fq 'define i64 @"_ARNv4impl3i647UsesVec4take"(i64 %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/traits/vector-trait-method-param-view.ll
	grep -Fq 'define i64 @"_ARNv4impl3i647UsesVec3sum"({ ptr, i64 } %arg0)' $(BUILD_DIR)/traits/vector-trait-method-param-view.ll
	$(TARGET) tests/cases/vectors/ok/vector-impl-receiver-view.ari --emit-llvm $(BUILD_DIR)/traits/vector-impl-receiver-view.ll
	grep -Fq 'define i64 @"_ARNv4impl15Vec_i64__cap_0_8inherent8edge_sum"({ ptr, i64 } %arg0)' $(BUILD_DIR)/traits/vector-impl-receiver-view.ll
	grep -Fq 'define i64 @"_ARNv4impl15Vec_i64__cap_0_8VecScore5score"({ ptr, i64 } %arg0)' $(BUILD_DIR)/traits/vector-impl-receiver-view.ll
	$(TARGET) tests/cases/vectors/ok/vector-impl-receiver-view.ari -o $(BUILD_DIR)/traits/vector-impl-receiver-view.elf
	$(BUILD_DIR)/traits/vector-impl-receiver-view.elf; code=$$?; test $$code -eq 33
	$(TARGET) tests/cases/vectors/ok/vector-trait-object-param-view.ari --emit-llvm $(BUILD_DIR)/traits/vector-trait-object-param-view.ll
	grep -Fq 'define private i64 @"dyn::vtable::i64::dyn_UsesVecObject::UsesVecObject::take"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/traits/vector-trait-object-param-view.ll
	$(TARGET) tests/cases/vectors/ok/vector-trait-object-param-view.ari -o $(BUILD_DIR)/traits/vector-trait-object-param-view.elf
	$(BUILD_DIR)/traits/vector-trait-object-param-view.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/traits/ok/trait-qualified-associated-call.ari -o $(BUILD_DIR)/traits/trait-qualified-associated-call.elf
	$(BUILD_DIR)/traits/trait-qualified-associated-call.elf; code=$$?; test $$code -eq 96
	$(TARGET) tests/cases/traits/ok/trait-qualified-receiver-type-args.ari -o $(BUILD_DIR)/traits/trait-qualified-receiver-type-args.elf
	$(BUILD_DIR)/traits/trait-qualified-receiver-type-args.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/trait-qualified-receiver-type-args.ari --emit-llvm $(BUILD_DIR)/traits/trait-qualified-receiver-type-args.ll
	grep -q "Select" $(BUILD_DIR)/traits/trait-qualified-receiver-type-args.ll
	$(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-control-flow.ari -o $(BUILD_DIR)/traits/trait-qualified-associated-expected-control-flow.elf
	$(BUILD_DIR)/traits/trait-qualified-associated-expected-control-flow.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-aggregates.ari --emit-llvm $(BUILD_DIR)/traits/trait-qualified-associated-expected-aggregates.ll
	grep -q "Boxed" $(BUILD_DIR)/traits/trait-qualified-associated-expected-aggregates.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-aggregates.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-qualified-associated-expected-aggregates.elf; $(BUILD_DIR)/traits/trait-qualified-associated-expected-aggregates.elf; code=$$?; test $$code -eq 36; fi
	$(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-enum-payload.ari --emit-llvm $(BUILD_DIR)/traits/trait-qualified-associated-expected-enum-payload.ll
	grep -q "Build" $(BUILD_DIR)/traits/trait-qualified-associated-expected-enum-payload.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-enum-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-qualified-associated-expected-enum-payload.elf; $(BUILD_DIR)/traits/trait-qualified-associated-expected-enum-payload.elf; code=$$?; test $$code -eq 61; fi
	$(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-primitive.ari --emit-llvm $(BUILD_DIR)/traits/trait-qualified-associated-expected-primitive.ll
	grep -q "Build" $(BUILD_DIR)/traits/trait-qualified-associated-expected-primitive.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-qualified-associated-expected-primitive.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-qualified-associated-expected-primitive.elf; $(BUILD_DIR)/traits/trait-qualified-associated-expected-primitive.elf; code=$$?; test $$code -eq 40; fi
	$(TARGET) tests/cases/traits/ok/trait-bound-generic-args.ari -o $(BUILD_DIR)/traits/trait-bound-generic-args.elf
	$(BUILD_DIR)/traits/trait-bound-generic-args.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/traits/ok/trait-bound-static-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-bound-static-dispatch.ll
	grep -q "LeftScore" $(BUILD_DIR)/traits/trait-bound-static-dispatch.ll
	$(TARGET) tests/cases/generics/ok/generic-trait-impl.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-impl.ll
	grep -q "Box_i64_" $(BUILD_DIR)/traits/generic-trait-impl.ll
	grep -q "Box_bool_" $(BUILD_DIR)/traits/generic-trait-impl.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-trait-impl.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-impl.elf; $(BUILD_DIR)/traits/generic-trait-impl.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/cases/generics/ok/generic-impl-bounds.ari --emit-llvm $(BUILD_DIR)/traits/generic-impl-bounds.ll
	grep -q "WrappedScore" $(BUILD_DIR)/traits/generic-impl-bounds.ll
	grep -q "Box_i64_" $(BUILD_DIR)/traits/generic-impl-bounds.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-impl-bounds.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-impl-bounds.elf; $(BUILD_DIR)/traits/generic-impl-bounds.elf; code=$$?; test $$code -eq 23; fi
	$(TARGET) tests/cases/traits/ok/trait-associated-call.ari --emit-llvm $(BUILD_DIR)/traits/trait-associated-call.ll
	grep -q "Build" $(BUILD_DIR)/traits/trait-associated-call.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-associated-call.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-associated-call.elf; $(BUILD_DIR)/traits/trait-associated-call.elf; code=$$?; test $$code -eq 9; fi
	$(TARGET) tests/cases/generics/ok/generic-trait-associated.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-associated.ll
	grep -q "Box_i64_" $(BUILD_DIR)/traits/generic-trait-associated.ll
	grep -q "Box_bool_" $(BUILD_DIR)/traits/generic-trait-associated.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-trait-associated.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-associated.elf; $(BUILD_DIR)/traits/generic-trait-associated.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/cases/traits/ok/associated-type-projection.ari -o $(BUILD_DIR)/traits/associated-type-projection.elf
	$(BUILD_DIR)/traits/associated-type-projection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/traits/ok/associated-type-supertrait-projection.ari -o $(BUILD_DIR)/traits/associated-type-supertrait-projection.elf
	$(BUILD_DIR)/traits/associated-type-supertrait-projection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/generics/ok/generic-trait-method.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-method.ll
	grep -q "keep__G_i64" $(BUILD_DIR)/traits/generic-trait-method.ll
	grep -q "keep__G_bool" $(BUILD_DIR)/traits/generic-trait-method.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-trait-method.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-method.elf; $(BUILD_DIR)/traits/generic-trait-method.elf; code=$$?; test $$code -eq 30; fi
	$(TARGET) tests/cases/generics/ok/generic-trait-method-bound.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-method-bound.ll
	grep -q "Score" $(BUILD_DIR)/traits/generic-trait-method-bound.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-trait-method-bound.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-method-bound.elf; $(BUILD_DIR)/traits/generic-trait-method-bound.elf; code=$$?; test $$code -eq 45; fi
	$(TARGET) tests/cases/generics/ok/generic-trait-method-ref-bound-inherent.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-method-ref-bound-inherent.ll
	grep -q "Readerish" $(BUILD_DIR)/traits/generic-trait-method-ref-bound-inherent.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-trait-method-ref-bound-inherent.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-method-ref-bound-inherent.elf; $(BUILD_DIR)/traits/generic-trait-method-ref-bound-inherent.elf; code=$$?; test $$code -eq 105; fi
	$(TARGET) tests/cases/traits/ok/iterator-next-surface.ari --emit-llvm $(BUILD_DIR)/traits/iterator-next-surface.ll
	grep -q "Cursor.*next" $(BUILD_DIR)/traits/iterator-next-surface.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/iterator-next-surface.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/iterator-next-surface.elf; $(BUILD_DIR)/traits/iterator-next-surface.elf; code=$$?; test $$code -eq 37; fi
	if $(TARGET) tests/cases/traits/errors/trait-drop-missing-method.ari -o $(BUILD_DIR)/traits/trait-drop-missing-method.elf >$(BUILD_DIR)/traits/trait-drop-missing-method.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'std::Drop' for i64 is missing method 'drop'" $(BUILD_DIR)/traits/trait-drop-missing-method.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-missing.ari -o $(BUILD_DIR)/traits/structural-capability-missing.elf >$(BUILD_DIR)/traits/structural-capability-missing.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-missing.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-multi-missing.ari -o $(BUILD_DIR)/traits/structural-capability-multi-missing.elf >$(BUILD_DIR)/traits/structural-capability-multi-missing.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has add(i64) -> i64'" $(BUILD_DIR)/traits/structural-capability-multi-missing.err
	grep -q "full structural capability set: has { serialize() -> i64, add(i64) -> i64 }" $(BUILD_DIR)/traits/structural-capability-multi-missing.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-duplicate-method.ari -o $(BUILD_DIR)/traits/structural-capability-duplicate-method.elf >$(BUILD_DIR)/traits/structural-capability-duplicate-method.err 2>&1; then exit 1; fi
	grep -q "duplicate structural capability method requirement 'serialize'" $(BUILD_DIR)/traits/structural-capability-duplicate-method.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-field-requirement.ari -o $(BUILD_DIR)/traits/structural-capability-field-requirement.elf >$(BUILD_DIR)/traits/structural-capability-field-requirement.err 2>&1; then exit 1; fi
	grep -q "structural capability requirements are method-only" $(BUILD_DIR)/traits/structural-capability-field-requirement.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-impl-method-missing.ari -o $(BUILD_DIR)/traits/structural-capability-impl-method-missing.elf >$(BUILD_DIR)/traits/structural-capability-impl-method-missing.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-impl-method-missing.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-trait-method-mismatch.ari -o $(BUILD_DIR)/traits/structural-capability-trait-method-mismatch.elf >$(BUILD_DIR)/traits/structural-capability-trait-method-mismatch.err 2>&1; then exit 1; fi
	grep -q "structural capability bound mismatch" $(BUILD_DIR)/traits/structural-capability-trait-method-mismatch.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-return-mismatch.ari -o $(BUILD_DIR)/traits/structural-capability-return-mismatch.elf >$(BUILD_DIR)/traits/structural-capability-return-mismatch.err 2>&1; then exit 1; fi
	grep -q "return type expects i64, found bool" $(BUILD_DIR)/traits/structural-capability-return-mismatch.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-generic-bound-missing.ari -o $(BUILD_DIR)/traits/structural-capability-generic-bound-missing.elf >$(BUILD_DIR)/traits/structural-capability-generic-bound-missing.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-generic-bound-missing.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-alias-as-type.ari -o $(BUILD_DIR)/traits/structural-capability-alias-as-type.elf >$(BUILD_DIR)/traits/structural-capability-alias-as-type.err 2>&1; then exit 1; fi
	grep -q "structural capability alias 'Serializable' can only be used as a generic bound" $(BUILD_DIR)/traits/structural-capability-alias-as-type.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-alias-struct-bound.ari -o $(BUILD_DIR)/traits/structural-capability-alias-struct-bound.elf >$(BUILD_DIR)/traits/structural-capability-alias-struct-bound.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-alias-struct-bound.err
	grep -q "while checking structural capability alias 'Serializable' as has serialize() -> i64" $(BUILD_DIR)/traits/structural-capability-alias-struct-bound.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-generic-alias-missing.ari -o $(BUILD_DIR)/traits/structural-capability-generic-alias-missing.elf >$(BUILD_DIR)/traits/structural-capability-generic-alias-missing.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has add(i64) -> i64'" $(BUILD_DIR)/traits/structural-capability-generic-alias-missing.err
	grep -q "while checking structural capability alias 'Combiner' as has add(i64) -> i64" $(BUILD_DIR)/traits/structural-capability-generic-alias-missing.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-generic-alias-arity.ari -o $(BUILD_DIR)/traits/structural-capability-generic-alias-arity.elf >$(BUILD_DIR)/traits/structural-capability-generic-alias-arity.err 2>&1; then exit 1; fi
	grep -q "type alias 'Combiner' expects 1 type argument" $(BUILD_DIR)/traits/structural-capability-generic-alias-arity.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-struct-generic-bound.ari -o $(BUILD_DIR)/traits/structural-capability-struct-generic-bound.elf >$(BUILD_DIR)/traits/structural-capability-struct-generic-bound.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-struct-generic-bound.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-enum-generic-bound.ari -o $(BUILD_DIR)/traits/structural-capability-enum-generic-bound.elf >$(BUILD_DIR)/traits/structural-capability-enum-generic-bound.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-enum-generic-bound.err
	if $(TARGET) tests/cases/traits/errors/structural-capability-forwarded-aggregate-bound-missing.ari -o $(BUILD_DIR)/traits/structural-capability-forwarded-aggregate-bound-missing.elf >$(BUILD_DIR)/traits/structural-capability-forwarded-aggregate-bound-missing.err 2>&1; then exit 1; fi
	grep -q "does not satisfy structural capability 'has serialize() -> i64'" $(BUILD_DIR)/traits/structural-capability-forwarded-aggregate-bound-missing.err
	if $(TARGET) tests/cases/traits/errors/unknown-trait-impl.ari -o $(BUILD_DIR)/traits/unknown-trait-impl.elf >$(BUILD_DIR)/traits/unknown-trait-impl.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'MissingTrait'" $(BUILD_DIR)/traits/unknown-trait-impl.err
	if $(TARGET) tests/cases/traits/errors/trait-object-unknown.ari -o $(BUILD_DIR)/traits/trait-object-unknown.elf >$(BUILD_DIR)/traits/trait-object-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'Missing' in trait object type" $(BUILD_DIR)/traits/trait-object-unknown.err
	if $(TARGET) tests/cases/traits/errors/trait-object-arity.ari -o $(BUILD_DIR)/traits/trait-object-arity.elf >$(BUILD_DIR)/traits/trait-object-arity.err 2>&1; then exit 1; fi
	grep -q "trait 'Compare' expects 1 type argument" $(BUILD_DIR)/traits/trait-object-arity.err
	if $(TARGET) tests/cases/traits/errors/trait-object-qualifier.ari -o $(BUILD_DIR)/traits/trait-object-qualifier.elf >$(BUILD_DIR)/traits/trait-object-qualifier.err 2>&1; then exit 1; fi
	grep -q "ptr dyn Trait is not supported; use dyn Trait, ref dyn Trait, ref mut dyn Trait, or own dyn Trait" $(BUILD_DIR)/traits/trait-object-qualifier.err
	$(TARGET) tests/cases/traits/ok/trait-object-own-zone-dispatch-drop.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-own-zone-dispatch-drop.ll
	grep -q '::drop"' $(BUILD_DIR)/traits/trait-object-own-zone-dispatch-drop.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/traits/trait-object-own-zone-dispatch-drop.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-own-zone-dispatch-drop.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-own-zone-dispatch-drop.elf; $(BUILD_DIR)/traits/trait-object-own-zone-dispatch-drop.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/trait-object-own-supertrait-upcast.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-own-supertrait-upcast.elf; $(BUILD_DIR)/traits/trait-object-own-supertrait-upcast.elf; code=$$?; test $$code -eq 47; fi
	if $(TARGET) tests/cases/traits/errors/trait-object-own-non-zone.ari -o $(BUILD_DIR)/traits/trait-object-own-non-zone.elf >$(BUILD_DIR)/traits/trait-object-own-non-zone.err 2>&1; then exit 1; fi
	grep -q "own dyn Trait conversions require a zone-tracked ptr operand, got i64" $(BUILD_DIR)/traits/trait-object-own-non-zone.err
	if $(TARGET) tests/cases/traits/errors/trait-object-own-after-reset.ari -o $(BUILD_DIR)/traits/trait-object-own-after-reset.elf >$(BUILD_DIR)/traits/trait-object-own-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'object' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/traits/trait-object-own-after-reset.err
	if $(TARGET) tests/cases/traits/errors/trait-object-ref-mut-source.ari -o $(BUILD_DIR)/traits/trait-object-ref-mut-source.elf >$(BUILD_DIR)/traits/trait-object-ref-mut-source.err 2>&1; then exit 1; fi
	grep -q "borrowed trait object conversion requires ref mut dyn Score source mode, got ref i64" $(BUILD_DIR)/traits/trait-object-ref-mut-source.err
	if $(TARGET) tests/cases/traits/errors/trait-object-value-planned.ari -o $(BUILD_DIR)/traits/trait-object-value-planned.elf >$(BUILD_DIR)/traits/trait-object-value-planned.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected dyn Score, got i64" $(BUILD_DIR)/traits/trait-object-value-planned.err
	if $(TARGET) tests/cases/traits/errors/trait-object-coerce-missing-impl.ari -o $(BUILD_DIR)/traits/trait-object-coerce-missing-impl.elf >$(BUILD_DIR)/traits/trait-object-coerce-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type i64 does not implement trait 'Score' for dyn conversion" $(BUILD_DIR)/traits/trait-object-coerce-missing-impl.err
	if $(TARGET) tests/cases/traits/errors/trait-object-upcast.ari -o $(BUILD_DIR)/traits/trait-object-upcast.elf >$(BUILD_DIR)/traits/trait-object-upcast.err 2>&1; then exit 1; fi
	grep -q "trait object upcast from dyn Score to dyn Show requires the target trait to be the same trait or a supertrait" $(BUILD_DIR)/traits/trait-object-upcast.err
	if $(TARGET) tests/cases/traits/errors/trait-object-generic-method-conversion.ari -o $(BUILD_DIR)/traits/trait-object-generic-method-conversion.elf >$(BUILD_DIR)/traits/trait-object-generic-method-conversion.err 2>&1; then exit 1; fi
	grep -q "trait object method 'keep' is generic and is not object-safe" $(BUILD_DIR)/traits/trait-object-generic-method-conversion.err
	if $(TARGET) tests/cases/traits/errors/trait-object-generic-method-call.ari -o $(BUILD_DIR)/traits/trait-object-generic-method-call.elf >$(BUILD_DIR)/traits/trait-object-generic-method-call.err 2>&1; then exit 1; fi
	grep -q "trait object method 'keep' is generic and is not object-safe" $(BUILD_DIR)/traits/trait-object-generic-method-call.err
	if $(TARGET) tests/cases/traits/errors/trait-object-supertrait-ambiguous.ari -o $(BUILD_DIR)/traits/trait-object-supertrait-ambiguous.elf >$(BUILD_DIR)/traits/trait-object-supertrait-ambiguous.err 2>&1; then exit 1; fi
	grep -q "trait object method 'score' for dyn BothScores is ambiguous across supertraits" $(BUILD_DIR)/traits/trait-object-supertrait-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/iterable-trait-arity.ari -o $(BUILD_DIR)/traits/iterable-trait-arity.elf >$(BUILD_DIR)/traits/iterable-trait-arity.err 2>&1; then exit 1; fi
	grep -q "trait 'std::Iterable' expects 1 type argument" $(BUILD_DIR)/traits/iterable-trait-arity.err
	if $(TARGET) tests/cases/traits/errors/duplicate-trait-impl.ari -o $(BUILD_DIR)/traits/duplicate-trait-impl.elf >$(BUILD_DIR)/traits/duplicate-trait-impl.err 2>&1; then exit 1; fi
	grep -q "duplicate impl of trait 'std::Iterable' for Bag" $(BUILD_DIR)/traits/duplicate-trait-impl.err
	if $(TARGET) tests/cases/vectors/errors/vector-trait-method-return-runtime-abi.ari -o $(BUILD_DIR)/traits/vector-trait-method-return-runtime-abi.elf >$(BUILD_DIR)/traits/vector-trait-method-return-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as a trait method return type; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/traits/vector-trait-method-return-runtime-abi.err
	if $(TARGET) tests/cases/traits/errors/trait-duplicate-decl.ari -o $(BUILD_DIR)/traits/trait-duplicate-decl.elf >$(BUILD_DIR)/traits/trait-duplicate-decl.err 2>&1; then exit 1; fi
	grep -q "duplicate trait 'Show'" $(BUILD_DIR)/traits/trait-duplicate-decl.err
	if $(TARGET) tests/cases/traits/errors/trait-duplicate-method.ari -o $(BUILD_DIR)/traits/trait-duplicate-method.elf >$(BUILD_DIR)/traits/trait-duplicate-method.err 2>&1; then exit 1; fi
	grep -q "duplicate method 'show' in trait 'Show'" $(BUILD_DIR)/traits/trait-duplicate-method.err
	if $(TARGET) tests/cases/traits/errors/trait-impl-missing-method.ari -o $(BUILD_DIR)/traits/trait-impl-missing-method.elf >$(BUILD_DIR)/traits/trait-impl-missing-method.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'Show' for i64 is missing method 'show'" $(BUILD_DIR)/traits/trait-impl-missing-method.err
	if $(TARGET) tests/cases/traits/errors/trait-impl-extra-method.ari -o $(BUILD_DIR)/traits/trait-impl-extra-method.elf >$(BUILD_DIR)/traits/trait-impl-extra-method.err 2>&1; then exit 1; fi
	grep -q "method 'debug' is not declared by trait 'Show'" $(BUILD_DIR)/traits/trait-impl-extra-method.err
	if $(TARGET) tests/cases/traits/errors/trait-impl-param-mismatch.ari -o $(BUILD_DIR)/traits/trait-impl-param-mismatch.elf >$(BUILD_DIR)/traits/trait-impl-param-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'eq' parameter 2 type mismatch: expected i64, got bool" $(BUILD_DIR)/traits/trait-impl-param-mismatch.err
	if $(TARGET) tests/cases/traits/errors/trait-impl-return-mismatch.ari -o $(BUILD_DIR)/traits/trait-impl-return-mismatch.elf >$(BUILD_DIR)/traits/trait-impl-return-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'show' return type mismatch: expected i64, got bool" $(BUILD_DIR)/traits/trait-impl-return-mismatch.err
	if $(TARGET) tests/cases/traits/errors/trait-impl-receiver-mismatch.ari -o $(BUILD_DIR)/traits/trait-impl-receiver-mismatch.elf >$(BUILD_DIR)/traits/trait-impl-receiver-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'score' parameter 1 type mismatch: expected ref i64, got i64" $(BUILD_DIR)/traits/trait-impl-receiver-mismatch.err
	if $(TARGET) tests/cases/generics/errors/generic-trait-method-count.ari -o $(BUILD_DIR)/traits/generic-trait-method-count.elf >$(BUILD_DIR)/traits/generic-trait-method-count.err 2>&1; then exit 1; fi
	grep -q "method 'keep' generic parameter count mismatch: trait expects 1, impl has 0" $(BUILD_DIR)/traits/generic-trait-method-count.err
	if $(TARGET) tests/cases/generics/errors/generic-trait-method-bound-missing.ari -o $(BUILD_DIR)/traits/generic-trait-method-bound-missing.elf >$(BUILD_DIR)/traits/generic-trait-method-bound-missing.err 2>&1; then exit 1; fi
	grep -q "method 'keep' generic parameter 1 bound mismatch: expected Score, got none" $(BUILD_DIR)/traits/generic-trait-method-bound-missing.err
	if $(TARGET) tests/cases/generics/errors/generic-trait-method-bound-mismatch.ari -o $(BUILD_DIR)/traits/generic-trait-method-bound-mismatch.elf >$(BUILD_DIR)/traits/generic-trait-method-bound-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'keep' generic parameter 1 bound mismatch: expected Score, got Other" $(BUILD_DIR)/traits/generic-trait-method-bound-mismatch.err
	if $(TARGET) tests/cases/traits/errors/trait-method-unknown.ari -o $(BUILD_DIR)/traits/trait-method-unknown.elf >$(BUILD_DIR)/traits/trait-method-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown method 'missing' for type i64" $(BUILD_DIR)/traits/trait-method-unknown.err
	if $(TARGET) tests/cases/traits/errors/trait-method-ambiguous.ari -o $(BUILD_DIR)/traits/trait-method-ambiguous.elf >$(BUILD_DIR)/traits/trait-method-ambiguous.err 2>&1; then exit 1; fi
	grep -q "method call 'score' for type i64 is ambiguous" $(BUILD_DIR)/traits/trait-method-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/trait-compiler-method-ambiguous.ari -o $(BUILD_DIR)/traits/trait-compiler-method-ambiguous.elf >$(BUILD_DIR)/traits/trait-compiler-method-ambiguous.err 2>&1; then exit 1; fi
	grep -q "method call 'score' for type Symbol is ambiguous" $(BUILD_DIR)/traits/trait-compiler-method-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/trait-qualified-supertrait-ambiguous.ari -o $(BUILD_DIR)/traits/trait-qualified-supertrait-ambiguous.elf >$(BUILD_DIR)/traits/trait-qualified-supertrait-ambiguous.err 2>&1; then exit 1; fi
	grep -q "trait-qualified method call 'BothScores::score' for type i64 is ambiguous" $(BUILD_DIR)/traits/trait-qualified-supertrait-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/trait-associated-ambiguous.ari -o $(BUILD_DIR)/traits/trait-associated-ambiguous.elf >$(BUILD_DIR)/traits/trait-associated-ambiguous.err 2>&1; then exit 1; fi
	grep -q "associated function 'pick' for type Box is ambiguous" $(BUILD_DIR)/traits/trait-associated-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/associated-type-missing-witness.ari -o $(BUILD_DIR)/traits/associated-type-missing-witness.elf >$(BUILD_DIR)/traits/associated-type-missing-witness.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'Source' for i64 is missing associated type 'Item'" $(BUILD_DIR)/traits/associated-type-missing-witness.err
	if $(TARGET) tests/cases/traits/errors/associated-type-projection-ambiguous.ari -o $(BUILD_DIR)/traits/associated-type-projection-ambiguous.elf >$(BUILD_DIR)/traits/associated-type-projection-ambiguous.err 2>&1; then exit 1; fi
	grep -q "associated type projection 'Source\\[i64\\]::Item' is ambiguous across impl witnesses" $(BUILD_DIR)/traits/associated-type-projection-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/associated-type-supertrait-ambiguous.ari -o $(BUILD_DIR)/traits/associated-type-supertrait-ambiguous.elf >$(BUILD_DIR)/traits/associated-type-supertrait-ambiguous.err 2>&1; then exit 1; fi
	grep -q "associated type projection 'Both\\[i64\\]::Item' is ambiguous across supertraits" $(BUILD_DIR)/traits/associated-type-supertrait-ambiguous.err
	if $(TARGET) tests/cases/traits/errors/trait-qualified-associated-missing-self.ari -o $(BUILD_DIR)/traits/trait-qualified-associated-missing-self.elf >$(BUILD_DIR)/traits/trait-qualified-associated-missing-self.err 2>&1; then exit 1; fi
	grep -q "trait-qualified associated function call 'Build::build' requires an explicit implementing type argument" $(BUILD_DIR)/traits/trait-qualified-associated-missing-self.err
	if $(TARGET) tests/cases/traits/errors/trait-qualified-associated-missing-impl.ari -o $(BUILD_DIR)/traits/trait-qualified-associated-missing-impl.elf >$(BUILD_DIR)/traits/trait-qualified-associated-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type Box does not implement trait 'Build' for trait-qualified associated function call" $(BUILD_DIR)/traits/trait-qualified-associated-missing-impl.err
	if $(TARGET) tests/cases/traits/errors/trait-qualified-associated-expected-missing-impl.ari -o $(BUILD_DIR)/traits/trait-qualified-associated-expected-missing-impl.elf >$(BUILD_DIR)/traits/trait-qualified-associated-expected-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type Other does not implement trait 'Build' for trait-qualified associated function call" $(BUILD_DIR)/traits/trait-qualified-associated-expected-missing-impl.err
	if $(TARGET) tests/cases/traits/errors/trait-bound-missing-impl.ari -o $(BUILD_DIR)/traits/trait-bound-missing-impl.elf >$(BUILD_DIR)/traits/trait-bound-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type i64 does not implement trait 'NeedsScore' required by generic parameter 'T'" $(BUILD_DIR)/traits/trait-bound-missing-impl.err
	if $(TARGET) tests/cases/traits/errors/trait-compiler-bound-missing-impl.ari -o $(BUILD_DIR)/traits/trait-compiler-bound-missing-impl.elf >$(BUILD_DIR)/traits/trait-compiler-bound-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type Diagnostic does not implement trait 'Renderable' required by generic parameter 'T'" $(BUILD_DIR)/traits/trait-compiler-bound-missing-impl.err
	if $(TARGET) tests/cases/traits/errors/trait-supertrait-missing-impl.ari -o $(BUILD_DIR)/traits/trait-supertrait-missing-impl.elf >$(BUILD_DIR)/traits/trait-supertrait-missing-impl.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'Child' for i64 requires supertrait 'Base'" $(BUILD_DIR)/traits/trait-supertrait-missing-impl.err
	if $(TARGET) tests/cases/generics/errors/generic-impl-bound-missing.ari -o $(BUILD_DIR)/traits/generic-impl-bound-missing.elf >$(BUILD_DIR)/traits/generic-impl-bound-missing.err 2>&1; then exit 1; fi
	grep -q "type i64 does not implement trait 'Score' required by impl generic parameter 'T'" $(BUILD_DIR)/traits/generic-impl-bound-missing.err
	if $(TARGET) tests/cases/generics/errors/generic-impl-coherence-concrete.ari -o $(BUILD_DIR)/traits/generic-impl-coherence-concrete.elf >$(BUILD_DIR)/traits/generic-impl-coherence-concrete.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'Score' for Box\\[i64\\] overlaps generic impl of trait 'Score' for Box\\[T\\]" $(BUILD_DIR)/traits/generic-impl-coherence-concrete.err
	if $(TARGET) tests/cases/generics/errors/generic-impl-coherence-generic.ari -o $(BUILD_DIR)/traits/generic-impl-coherence-generic.elf >$(BUILD_DIR)/traits/generic-impl-coherence-generic.err 2>&1; then exit 1; fi
	grep -q "generic impl of trait 'Pick' for Pair\\[First, Second\\] overlaps generic impl of trait 'Pick' for Pair\\[T, T\\]" $(BUILD_DIR)/traits/generic-impl-coherence-generic.err
	if $(TARGET) tests/cases/traits/errors/trait-bound-unknown.ari -o $(BUILD_DIR)/traits/trait-bound-unknown.elf >$(BUILD_DIR)/traits/trait-bound-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown trait bound 'MissingTrait'" $(BUILD_DIR)/traits/trait-bound-unknown.err

check-structs: $(TARGET)
	mkdir -p $(BUILD_DIR)/structs
	$(TARGET) tests/cases/structs/ok/struct-surface.ari -o $(BUILD_DIR)/structs/struct-surface.elf
	$(BUILD_DIR)/structs/struct-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/structs/ok/struct-module-surface.ari -o $(BUILD_DIR)/structs/struct-module-surface.elf
	$(BUILD_DIR)/structs/struct-module-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/structs/ok/block-expression-field-assignment.ari -o $(BUILD_DIR)/structs/block-expression-field-assignment.elf
	$(BUILD_DIR)/structs/block-expression-field-assignment.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/match/ok/adt-enum-patterns.ari -o $(BUILD_DIR)/structs/adt-enum-patterns.elf
	$(BUILD_DIR)/structs/adt-enum-patterns.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/generics/ok/generic-adt-surface.ari -o $(BUILD_DIR)/structs/generic-adt-surface.elf
	$(BUILD_DIR)/structs/generic-adt-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/generics/ok/generic-struct-infer.ari -o $(BUILD_DIR)/structs/generic-struct-infer.elf
	$(BUILD_DIR)/structs/generic-struct-infer.elf; code=$$?; test $$code -eq 40
	$(TARGET) tests/cases/generics/ok/generic-struct-infer.ari --emit-llvm $(BUILD_DIR)/structs/generic-struct-infer.ll
	grep -q "insertvalue { i64 }" $(BUILD_DIR)/structs/generic-struct-infer.ll
	grep -q "insertvalue { i64, i1 }" $(BUILD_DIR)/structs/generic-struct-infer.ll
	$(TARGET) tests/cases/structs/ok/struct-literal-field-name-payloads.ari -o $(BUILD_DIR)/structs/struct-literal-field-name-payloads.elf
	$(BUILD_DIR)/structs/struct-literal-field-name-payloads.elf; code=$$?; test $$code -eq 84
	$(TARGET) tests/cases/structs/ok/struct-literal-field-name-payloads.ari --emit-llvm $(BUILD_DIR)/structs/struct-literal-field-name-payloads.ll
	grep -q "insertvalue { i64, i64 }" $(BUILD_DIR)/structs/struct-literal-field-name-payloads.ll
	$(TARGET) tests/cases/generics/ok/generic-impl-method.ari --emit-llvm $(BUILD_DIR)/structs/generic-impl-method.ll
	grep -q "Box_i64_" $(BUILD_DIR)/structs/generic-impl-method.ll
	grep -q "Box_bool_" $(BUILD_DIR)/structs/generic-impl-method.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-impl-method.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-impl-method.elf; $(BUILD_DIR)/structs/generic-impl-method.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/cases/generics/ok/generic-method-type-args.ari --emit-llvm $(BUILD_DIR)/structs/generic-method-type-args.ll
	grep -q "replace__G_i64" $(BUILD_DIR)/structs/generic-method-type-args.ll
	grep -q "score_with__G_i64" $(BUILD_DIR)/structs/generic-method-type-args.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-method-type-args.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-method-type-args.elf; $(BUILD_DIR)/structs/generic-method-type-args.elf; code=$$?; test $$code -eq 22; fi
	$(TARGET) tests/cases/generics/ok/generic-method-infer.ari --emit-llvm $(BUILD_DIR)/structs/generic-method-infer.ll
	grep -q "replace__G_i64" $(BUILD_DIR)/structs/generic-method-infer.ll
	grep -q "score_with__G_i64" $(BUILD_DIR)/structs/generic-method-infer.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-method-infer.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-method-infer.elf; $(BUILD_DIR)/structs/generic-method-infer.elf; code=$$?; test $$code -eq 22; fi
	$(TARGET) tests/cases/structs/ok/tuple-layout.ari -o $(BUILD_DIR)/structs/tuple-layout.elf
	$(BUILD_DIR)/structs/tuple-layout.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/structs/ok/tuple-layout.ari --emit-llvm $(BUILD_DIR)/structs/tuple-layout.ll
	grep -q "insertvalue { i64, i1 }" $(BUILD_DIR)/structs/tuple-layout.ll
	grep -q "extractvalue { i64, i1 }" $(BUILD_DIR)/structs/tuple-layout.ll
	$(TARGET) tests/cases/structs/ok/struct-lowering.ari -o $(BUILD_DIR)/structs/struct-lowering.elf
	$(BUILD_DIR)/structs/struct-lowering.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/structs/ok/struct-lowering.ari --emit-llvm $(BUILD_DIR)/structs/struct-lowering.ll
	grep -q "insertvalue { i64, i64, i1 }" $(BUILD_DIR)/structs/struct-lowering.ll
	grep -q "extractvalue { i64, i64, i1 }" $(BUILD_DIR)/structs/struct-lowering.ll
	$(TARGET) tests/cases/structs/ok/union-by-constructor.ari -o $(BUILD_DIR)/structs/union-by-constructor.elf
	$(BUILD_DIR)/structs/union-by-constructor.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/structs/ok/union-by-constructor.ari --emit-llvm $(BUILD_DIR)/structs/union-by-constructor.ll
	grep -q "insertvalue" $(BUILD_DIR)/structs/union-by-constructor.ll
	$(TARGET) tests/cases/structs/ok/union-by-match.ari -o $(BUILD_DIR)/structs/union-by-match.elf
	$(BUILD_DIR)/structs/union-by-match.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/structs/ok/union-by-match.ari --emit-llvm $(BUILD_DIR)/structs/union-by-match.ll
	grep -q "switch i64" $(BUILD_DIR)/structs/union-by-match.ll
	$(TARGET) tests/cases/structs/ok/union-by-bool-selector.ari -o $(BUILD_DIR)/structs/union-by-bool-selector.elf
	$(BUILD_DIR)/structs/union-by-bool-selector.elf; code=$$?; test $$code -eq 46
	$(TARGET) tests/cases/structs/ok/union-by-selector-match-projection.ari -o $(BUILD_DIR)/structs/union-by-selector-match-projection.elf
	$(BUILD_DIR)/structs/union-by-selector-match-projection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/structs/ok/union-by-wrapper-selector-match-projection.ari -o $(BUILD_DIR)/structs/union-by-wrapper-selector-match-projection.elf
	$(BUILD_DIR)/structs/union-by-wrapper-selector-match-projection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/structs/ok/union-by-alias-match-projection.ari -o $(BUILD_DIR)/structs/union-by-alias-match-projection.elf
	$(BUILD_DIR)/structs/union-by-alias-match-projection.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/structs/ok/union-by-bool-selector-match-projection.ari -o $(BUILD_DIR)/structs/union-by-bool-selector-match-projection.elf
	$(BUILD_DIR)/structs/union-by-bool-selector-match-projection.elf; code=$$?; test $$code -eq 46
	$(TARGET) tests/cases/structs/ok/union-by-bool-call-constructor.ari -o $(BUILD_DIR)/structs/union-by-bool-call-constructor.elf
	$(BUILD_DIR)/structs/union-by-bool-call-constructor.elf; code=$$?; test $$code -eq 44
	$(TARGET) tests/cases/structs/ok/union-by-shorthand-constructor.ari -o $(BUILD_DIR)/structs/union-by-shorthand-constructor.elf
	$(BUILD_DIR)/structs/union-by-shorthand-constructor.elf; code=$$?; test $$code -eq 47
	$(TARGET) tests/cases/structs/ok/union-by-owned-payload-drop.ari -o $(BUILD_DIR)/structs/union-by-owned-payload-drop.elf
	$(BUILD_DIR)/structs/union-by-owned-payload-drop.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/structs/ok/union-by-owned-payload-drop.ari --emit-llvm $(BUILD_DIR)/structs/union-by-owned-payload-drop.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/structs/union-by-owned-payload-drop.ll) -eq 2
	$(TARGET) tests/cases/traits/ok/impl-associated-new.ari --emit-llvm $(BUILD_DIR)/structs/impl-associated-new.ll
	grep -q "_ARNv4impl5Point8inherent3new" $(BUILD_DIR)/structs/impl-associated-new.ll
	grep -q "call { i64, i64 }" $(BUILD_DIR)/structs/impl-associated-new.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/traits/ok/impl-associated-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/impl-associated-new.elf; $(BUILD_DIR)/structs/impl-associated-new.elf; code=$$?; test $$code -eq 9; fi
	$(TARGET) tests/cases/generics/ok/generic-associated-new.ari --emit-llvm $(BUILD_DIR)/structs/generic-associated-new.ll
	grep -q "Box_i64_" $(BUILD_DIR)/structs/generic-associated-new.ll
	grep -q "Box_bool_" $(BUILD_DIR)/structs/generic-associated-new.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-associated-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-associated-new.elf; $(BUILD_DIR)/structs/generic-associated-new.elf; code=$$?; test $$code -eq 22; fi
	$(TARGET) tests/cases/generics/ok/generic-associated-method-type-args.ari --emit-llvm $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	grep -q "make__G_i64" $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	grep -q "make__G_bool" $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	grep -q "scored__G_i64" $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/generics/ok/generic-associated-method-type-args.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-associated-method-type-args.elf; $(BUILD_DIR)/structs/generic-associated-method-type-args.elf; code=$$?; test $$code -eq 22; fi
	if $(TARGET) tests/cases/structs/errors/struct-duplicate-decl.ari -o $(BUILD_DIR)/structs/struct-duplicate-decl.elf >$(BUILD_DIR)/structs/struct-duplicate-decl.err 2>&1; then exit 1; fi
	grep -q "duplicate struct 'Point'" $(BUILD_DIR)/structs/struct-duplicate-decl.err
	if $(TARGET) tests/cases/structs/errors/struct-field-semicolon.ari -o $(BUILD_DIR)/structs/struct-field-semicolon.elf >$(BUILD_DIR)/structs/struct-field-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected , after struct field" $(BUILD_DIR)/structs/struct-field-semicolon.err
	if $(TARGET) tests/cases/structs/errors/tuple-struct-semicolon.ari -o $(BUILD_DIR)/structs/tuple-struct-semicolon.elf >$(BUILD_DIR)/structs/tuple-struct-semicolon.err 2>&1; then exit 1; fi
	grep -q "tuple struct declarations do not end with ;" $(BUILD_DIR)/structs/tuple-struct-semicolon.err
	if $(TARGET) tests/cases/match/errors/enum-case-semicolon.ari -o $(BUILD_DIR)/structs/enum-case-semicolon.elf >$(BUILD_DIR)/structs/enum-case-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected , after enum case" $(BUILD_DIR)/structs/enum-case-semicolon.err
	if $(TARGET) tests/cases/structs/errors/struct-duplicate-field.ari -o $(BUILD_DIR)/structs/struct-duplicate-field.elf >$(BUILD_DIR)/structs/struct-duplicate-field.err 2>&1; then exit 1; fi
	grep -q "duplicate field 'x' in struct 'Point'" $(BUILD_DIR)/structs/struct-duplicate-field.err
	if $(TARGET) tests/cases/structs/errors/struct-unknown-field-type.ari -o $(BUILD_DIR)/structs/struct-unknown-field-type.elf >$(BUILD_DIR)/structs/struct-unknown-field-type.err 2>&1; then exit 1; fi
	grep -q "unknown type 'Missing'" $(BUILD_DIR)/structs/struct-unknown-field-type.err
	if $(TARGET) tests/cases/vectors/errors/vector-struct-field-runtime-abi.ari -o $(BUILD_DIR)/structs/vector-struct-field-runtime-abi.elf >$(BUILD_DIR)/structs/vector-struct-field-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as a struct field; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/structs/vector-struct-field-runtime-abi.err
	if $(TARGET) tests/cases/vectors/errors/vector-generic-struct-field-runtime-abi.ari -o $(BUILD_DIR)/structs/vector-generic-struct-field-runtime-abi.elf >$(BUILD_DIR)/structs/vector-generic-struct-field-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as a struct field; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/structs/vector-generic-struct-field-runtime-abi.err
	if $(TARGET) tests/cases/structs/errors/struct-generic-arity.ari -o $(BUILD_DIR)/structs/struct-generic-arity.elf >$(BUILD_DIR)/structs/struct-generic-arity.err 2>&1; then exit 1; fi
	grep -q "struct 'Pair' expects 1 type argument" $(BUILD_DIR)/structs/struct-generic-arity.err
	if $(TARGET) tests/cases/generics/errors/generic-struct-infer-conflict.ari -o $(BUILD_DIR)/structs/generic-struct-infer-conflict.elf >$(BUILD_DIR)/structs/generic-struct-infer-conflict.err 2>&1; then exit 1; fi
	grep -q "conflicting inferred type for generic parameter 'T': i64 and bool" $(BUILD_DIR)/structs/generic-struct-infer-conflict.err
	if $(TARGET) tests/cases/generics/errors/generic-struct-infer-missing.ari -o $(BUILD_DIR)/structs/generic-struct-infer-missing.elf >$(BUILD_DIR)/structs/generic-struct-infer-missing.err 2>&1; then exit 1; fi
	grep -q "cannot infer type argument 'T' for struct 'Phantom'" $(BUILD_DIR)/structs/generic-struct-infer-missing.err
	if $(TARGET) tests/cases/generics/errors/generic-associated-arity.ari -o $(BUILD_DIR)/structs/generic-associated-arity.elf >$(BUILD_DIR)/structs/generic-associated-arity.err 2>&1; then exit 1; fi
	grep -q "generic associated function expects 1 type argument" $(BUILD_DIR)/structs/generic-associated-arity.err
	if $(TARGET) tests/cases/generics/errors/generic-method-arity.ari -o $(BUILD_DIR)/structs/generic-method-arity.elf >$(BUILD_DIR)/structs/generic-method-arity.err 2>&1; then exit 1; fi
	grep -q "generic method 'replace' expects 1 type argument" $(BUILD_DIR)/structs/generic-method-arity.err
	if $(TARGET) tests/cases/generics/errors/generic-method-uninferred.ari -o $(BUILD_DIR)/structs/generic-method-uninferred.elf >$(BUILD_DIR)/structs/generic-method-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'U' could not be inferred for method 'make'" $(BUILD_DIR)/structs/generic-method-uninferred.err
	if $(TARGET) tests/cases/generics/errors/generic-associated-method-uninferred.ari -o $(BUILD_DIR)/structs/generic-associated-method-uninferred.elf >$(BUILD_DIR)/structs/generic-associated-method-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'U' could not be inferred for associated function 'make'" $(BUILD_DIR)/structs/generic-associated-method-uninferred.err
	if $(TARGET) tests/cases/modules/errors/private-module-struct-type.ari -o $(BUILD_DIR)/structs/private-module-struct-type.elf >$(BUILD_DIR)/structs/private-module-struct-type.err 2>&1; then exit 1; fi
	grep -q "struct 'Hidden::Secret' is not public" $(BUILD_DIR)/structs/private-module-struct-type.err
	if $(TARGET) tests/cases/structs/errors/struct-field-immutable.ari -o $(BUILD_DIR)/structs/struct-field-immutable.elf >$(BUILD_DIR)/structs/struct-field-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable field 'x' of struct 'Point'" $(BUILD_DIR)/structs/struct-field-immutable.err
	if $(TARGET) tests/cases/structs/errors/struct-field-immutable-binding.ari -o $(BUILD_DIR)/structs/struct-field-immutable-binding.elf >$(BUILD_DIR)/structs/struct-field-immutable-binding.err 2>&1; then exit 1; fi
	grep -q "cannot assign to field of immutable binding 'meter'" $(BUILD_DIR)/structs/struct-field-immutable-binding.err
	if $(TARGET) tests/cases/structs/errors/struct-field-unknown.ari -o $(BUILD_DIR)/structs/struct-field-unknown.elf >$(BUILD_DIR)/structs/struct-field-unknown.err 2>&1; then exit 1; fi
	grep -q "struct 'Point' has no field 'y'" $(BUILD_DIR)/structs/struct-field-unknown.err
	if $(TARGET) tests/cases/structs/errors/struct-literal-missing-field.ari -o $(BUILD_DIR)/structs/struct-literal-missing-field.elf >$(BUILD_DIR)/structs/struct-literal-missing-field.err 2>&1; then exit 1; fi
	grep -q "missing field 'y' in struct literal for 'Point'" $(BUILD_DIR)/structs/struct-literal-missing-field.err
	if $(TARGET) tests/cases/structs/errors/struct-literal-extra-field.ari -o $(BUILD_DIR)/structs/struct-literal-extra-field.elf >$(BUILD_DIR)/structs/struct-literal-extra-field.err 2>&1; then exit 1; fi
	grep -q "extra field 'y' in struct literal for 'Point'" $(BUILD_DIR)/structs/struct-literal-extra-field.err
	if $(TARGET) tests/cases/structs/errors/struct-constant-literal-missing-field.ari -o $(BUILD_DIR)/structs/struct-constant-literal-missing-field.elf >$(BUILD_DIR)/structs/struct-constant-literal-missing-field.err 2>&1; then exit 1; fi
	grep -q "missing field 'y' in struct literal for 'Point'" $(BUILD_DIR)/structs/struct-constant-literal-missing-field.err
	if $(TARGET) tests/cases/structs/errors/struct-constant-literal-extra-field.ari -o $(BUILD_DIR)/structs/struct-constant-literal-extra-field.elf >$(BUILD_DIR)/structs/struct-constant-literal-extra-field.err 2>&1; then exit 1; fi
	grep -q "extra field 'y' in struct literal for 'Point'" $(BUILD_DIR)/structs/struct-constant-literal-extra-field.err
	if $(TARGET) tests/cases/structs/errors/struct-literal-duplicate-field.ari -o $(BUILD_DIR)/structs/struct-literal-duplicate-field.elf >$(BUILD_DIR)/structs/struct-literal-duplicate-field.err 2>&1; then exit 1; fi
	grep -q "duplicate field 'x' in struct literal" $(BUILD_DIR)/structs/struct-literal-duplicate-field.err
	if $(TARGET) tests/cases/structs/errors/struct-literal-field-type.ari -o $(BUILD_DIR)/structs/struct-literal-field-type.elf >$(BUILD_DIR)/structs/struct-literal-field-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/structs/struct-literal-field-type.err
	$(TARGET) tests/cases/structs/ok/union-by-selector-inferred.ari -o $(BUILD_DIR)/structs/union-by-selector-inferred.elf
	$(BUILD_DIR)/structs/union-by-selector-inferred.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/structs/ok/union-by-nested-selector-inferred.ari -o $(BUILD_DIR)/structs/union-by-nested-selector-inferred.elf
	$(BUILD_DIR)/structs/union-by-nested-selector-inferred.elf; code=$$?; test $$code -eq 84
	if $(TARGET) tests/cases/structs/errors/union-by-constructor-unknown-arm.ari -o $(BUILD_DIR)/structs/union-by-constructor-unknown-arm.elf >$(BUILD_DIR)/structs/union-by-constructor-unknown-arm.err 2>&1; then exit 1; fi
	grep -q "union by constructor arm 'record' is not declared for field 'fragment'" $(BUILD_DIR)/structs/union-by-constructor-unknown-arm.err
	if $(TARGET) tests/cases/structs/errors/union-by-constructor-selector-mismatch.ari -o $(BUILD_DIR)/structs/union-by-constructor-selector-mismatch.elf >$(BUILD_DIR)/structs/union-by-constructor-selector-mismatch.err 2>&1; then exit 1; fi
	grep -q "union by constructor arm 'block' does not match selector 'security.cipher_type' value 'stream'" $(BUILD_DIR)/structs/union-by-constructor-selector-mismatch.err
	if $(TARGET) tests/cases/structs/errors/union-by-constructor-dynamic-selector.ari -o $(BUILD_DIR)/structs/union-by-constructor-dynamic-selector.elf >$(BUILD_DIR)/structs/union-by-constructor-dynamic-selector.err 2>&1; then exit 1; fi
	grep -q "union by constructor arm 'stream' cannot be checked against non-static selector 'kind'" $(BUILD_DIR)/structs/union-by-constructor-dynamic-selector.err
	if $(TARGET) tests/cases/structs/errors/union-by-nested-constructor-dynamic-selector.ari -o $(BUILD_DIR)/structs/union-by-nested-constructor-dynamic-selector.elf >$(BUILD_DIR)/structs/union-by-nested-constructor-dynamic-selector.err 2>&1; then exit 1; fi
	grep -q "union by constructor arm 'stream' cannot be checked against non-static selector 'security.cipher_type'" $(BUILD_DIR)/structs/union-by-nested-constructor-dynamic-selector.err
	if $(TARGET) tests/cases/structs/errors/union-by-constructor-payload-count.ari -o $(BUILD_DIR)/structs/union-by-constructor-payload-count.elf >$(BUILD_DIR)/structs/union-by-constructor-payload-count.err 2>&1; then exit 1; fi
	grep -q "union by constructor arm 'stream' for field 'fragment' in struct 'TLSCiphertext' expects exactly one payload expression" $(BUILD_DIR)/structs/union-by-constructor-payload-count.err
	if $(TARGET) tests/cases/structs/errors/union-by-bool-selector-missing-arm.ari -o $(BUILD_DIR)/structs/union-by-bool-selector-missing-arm.elf >$(BUILD_DIR)/structs/union-by-bool-selector-missing-arm.err 2>&1; then exit 1; fi
	grep -q "union by bool selector is missing arm 'true'" $(BUILD_DIR)/structs/union-by-bool-selector-missing-arm.err
	if $(TARGET) tests/cases/structs/errors/union-by-bool-selector-unknown-arm.ari -o $(BUILD_DIR)/structs/union-by-bool-selector-unknown-arm.elf >$(BUILD_DIR)/structs/union-by-bool-selector-unknown-arm.err 2>&1; then exit 1; fi
	grep -q "union by bool selector arm must be 'false' or 'true', got 'maybe'" $(BUILD_DIR)/structs/union-by-bool-selector-unknown-arm.err
	if $(TARGET) tests/cases/structs/errors/union-by-shorthand-non-struct-payload.ari -o $(BUILD_DIR)/structs/union-by-shorthand-non-struct-payload.elf >$(BUILD_DIR)/structs/union-by-shorthand-non-struct-payload.err 2>&1; then exit 1; fi
	grep -q "cannot use struct payload shorthand" $(BUILD_DIR)/structs/union-by-shorthand-non-struct-payload.err
	if $(TARGET) tests/cases/structs/errors/union-by-shorthand-selector-mismatch.ari -o $(BUILD_DIR)/structs/union-by-shorthand-selector-mismatch.elf >$(BUILD_DIR)/structs/union-by-shorthand-selector-mismatch.err 2>&1; then exit 1; fi
	grep -q "union by constructor arm 'stream' does not match selector 'security.cipher_type' value 'block'" $(BUILD_DIR)/structs/union-by-shorthand-selector-mismatch.err
	if $(TARGET) tests/cases/structs/errors/union-by-owned-payload-aggregate.ari -o $(BUILD_DIR)/structs/union-by-owned-payload-aggregate.elf >$(BUILD_DIR)/structs/union-by-owned-payload-aggregate.err 2>&1; then exit 1; fi
	grep -q "ownership-carrying aggregate union by payloads are not supported yet" $(BUILD_DIR)/structs/union-by-owned-payload-aggregate.err
	if $(TARGET) tests/cases/structs/errors/union-by-owned-payload-undropped.ari -o $(BUILD_DIR)/structs/union-by-owned-payload-undropped.elf >$(BUILD_DIR)/structs/union-by-owned-payload-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before return" $(BUILD_DIR)/structs/union-by-owned-payload-undropped.err
	if $(TARGET) tests/cases/structs/errors/union-by-owned-payload-binding-undropped.ari -o $(BUILD_DIR)/structs/union-by-owned-payload-binding-undropped.elf >$(BUILD_DIR)/structs/union-by-owned-payload-binding-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'packet' must be moved or dropped before return" $(BUILD_DIR)/structs/union-by-owned-payload-binding-undropped.err
	if $(TARGET) tests/cases/structs/errors/union-by-selector-inference-conflict.ari -o $(BUILD_DIR)/structs/union-by-selector-inference-conflict.elf >$(BUILD_DIR)/structs/union-by-selector-inference-conflict.err 2>&1; then exit 1; fi
	grep -q "union by constructors infer conflicting values for selector 'kind'" $(BUILD_DIR)/structs/union-by-selector-inference-conflict.err
	if $(TARGET) tests/cases/structs/errors/union-by-nested-selector-inference-conflict.ari -o $(BUILD_DIR)/structs/union-by-nested-selector-inference-conflict.elf >$(BUILD_DIR)/structs/union-by-nested-selector-inference-conflict.err 2>&1; then exit 1; fi
	grep -q "union by constructors infer conflicting values for selector 'security.cipher_type'" $(BUILD_DIR)/structs/union-by-nested-selector-inference-conflict.err
	if $(TARGET) tests/cases/structs/errors/union-by-selector-assignment.ari -o $(BUILD_DIR)/structs/union-by-selector-assignment.elf >$(BUILD_DIR)/structs/union-by-selector-assignment.err 2>&1; then exit 1; fi
	grep -q "cannot assign to union by selector path 'security.cipher_type' through 'packet.security.cipher_type'" $(BUILD_DIR)/structs/union-by-selector-assignment.err
	if $(TARGET) tests/cases/structs/errors/union-by-selector-root-assignment.ari -o $(BUILD_DIR)/structs/union-by-selector-root-assignment.elf >$(BUILD_DIR)/structs/union-by-selector-root-assignment.err 2>&1; then exit 1; fi
	grep -q "cannot assign to union by selector path 'security.cipher_type' through 'packet.security'" $(BUILD_DIR)/structs/union-by-selector-root-assignment.err
	if $(TARGET) tests/cases/structs/errors/union-by-field-assignment.ari -o $(BUILD_DIR)/structs/union-by-field-assignment.elf >$(BUILD_DIR)/structs/union-by-field-assignment.err 2>&1; then exit 1; fi
	grep -q "cannot assign to union by field 'fragment' through 'packet.fragment'" $(BUILD_DIR)/structs/union-by-field-assignment.err
	if $(TARGET) tests/cases/structs/errors/union-by-direct-payload-access.ari -o $(BUILD_DIR)/structs/union-by-direct-payload-access.elf >$(BUILD_DIR)/structs/union-by-direct-payload-access.err 2>&1; then exit 1; fi
	grep -q "cannot project union by payload slot 'fragment.0' without matching the active arm" $(BUILD_DIR)/structs/union-by-direct-payload-access.err
	if $(TARGET) tests/cases/structs/errors/union-by-local-payload-access.ari -o $(BUILD_DIR)/structs/union-by-local-payload-access.elf >$(BUILD_DIR)/structs/union-by-local-payload-access.err 2>&1; then exit 1; fi
	grep -q "cannot project union by payload slot 'fragment.0' without matching the active arm" $(BUILD_DIR)/structs/union-by-local-payload-access.err
	if $(TARGET) tests/cases/structs/errors/union-by-pattern-alias-payload-access.ari -o $(BUILD_DIR)/structs/union-by-pattern-alias-payload-access.elf >$(BUILD_DIR)/structs/union-by-pattern-alias-payload-access.err 2>&1; then exit 1; fi
	grep -q "cannot project union by payload slot 'fragment.0' without matching the active arm" $(BUILD_DIR)/structs/union-by-pattern-alias-payload-access.err
	if $(TARGET) tests/cases/structs/errors/union-by-local-alias-assignment.ari -o $(BUILD_DIR)/structs/union-by-local-alias-assignment.elf >$(BUILD_DIR)/structs/union-by-local-alias-assignment.err 2>&1; then exit 1; fi
	grep -q "cannot assign to union by alias 'fragment'" $(BUILD_DIR)/structs/union-by-local-alias-assignment.err
	if $(TARGET) tests/cases/structs/errors/union-by-pattern-alias-assignment.ari -o $(BUILD_DIR)/structs/union-by-pattern-alias-assignment.elf >$(BUILD_DIR)/structs/union-by-pattern-alias-assignment.err 2>&1; then exit 1; fi
	grep -q "cannot assign to union by alias 'fragment'" $(BUILD_DIR)/structs/union-by-pattern-alias-assignment.err
	if $(TARGET) tests/cases/structs/errors/struct-field-access-non-struct.ari -o $(BUILD_DIR)/structs/struct-field-access-non-struct.elf >$(BUILD_DIR)/structs/struct-field-access-non-struct.err 2>&1; then exit 1; fi
	grep -q "field access requires a struct value, got i64" $(BUILD_DIR)/structs/struct-field-access-non-struct.err
	if $(TARGET) tests/cases/traits/errors/impl-associated-private.ari -o $(BUILD_DIR)/structs/impl-associated-private.elf >$(BUILD_DIR)/structs/impl-associated-private.err 2>&1; then exit 1; fi
	grep -q "method 'new' for type Factory::Point is not public" $(BUILD_DIR)/structs/impl-associated-private.err
	if $(TARGET) tests/cases/traits/errors/impl-associated-unknown.ari -o $(BUILD_DIR)/structs/impl-associated-unknown.elf >$(BUILD_DIR)/structs/impl-associated-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown associated function 'missing' for type Point" $(BUILD_DIR)/structs/impl-associated-unknown.err
	if $(TARGET) tests/cases/structs/errors/tuple-index-out-of-range.ari -o $(BUILD_DIR)/structs/tuple-index-out-of-range.elf >$(BUILD_DIR)/structs/tuple-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "tuple index 2 is out of range for (i64, bool)" $(BUILD_DIR)/structs/tuple-index-out-of-range.err

check-variables: $(TARGET)
	mkdir -p $(BUILD_DIR)/variables
	$(TARGET) tests/cases/variables/ok/variables-basic.ari -o $(BUILD_DIR)/variables/variables-basic.elf
	$(BUILD_DIR)/variables/variables-basic.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/variables/ok/variables-widths.ari -o $(BUILD_DIR)/variables/variables-widths.elf
	$(BUILD_DIR)/variables/variables-widths.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/ir/ok/llvm-scalar-width-local.ari -o $(BUILD_DIR)/variables/llvm-scalar-width-local.elf
	$(BUILD_DIR)/variables/llvm-scalar-width-local.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/ir/ok/llvm-scalar-stack-pack.ari -o $(BUILD_DIR)/variables/llvm-scalar-stack-pack.elf
	$(BUILD_DIR)/variables/llvm-scalar-stack-pack.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/variables/ok/variables-match-binding.ari -o $(BUILD_DIR)/variables/variables-match-binding.elf
	$(BUILD_DIR)/variables/variables-match-binding.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/variables/ok/variables-tuple-pattern.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern.elf
	$(BUILD_DIR)/variables/variables-tuple-pattern.elf; code=$$?; test $$code -eq 38
	$(TARGET) tests/cases/patterns/ok/pattern-mut-binding-mode.ari -o $(BUILD_DIR)/variables/pattern-mut-binding-mode.elf
	$(BUILD_DIR)/variables/pattern-mut-binding-mode.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/structs/ok/unit-empty-tuple.ari -o $(BUILD_DIR)/variables/unit-empty-tuple.elf
	$(BUILD_DIR)/variables/unit-empty-tuple.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/variables/ok/variables-struct-pattern.ari -o $(BUILD_DIR)/variables/variables-struct-pattern.elf
	$(BUILD_DIR)/variables/variables-struct-pattern.elf; code=$$?; test $$code -eq 25
	$(TARGET) tests/cases/variables/ok/variables-enum-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-enum-pattern-binding.elf
	$(BUILD_DIR)/variables/variables-enum-pattern-binding.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/variables/ok/variables-enum-or-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.elf
	$(BUILD_DIR)/variables/variables-enum-or-pattern-binding.elf; code=$$?; test $$code -eq 33
	$(TARGET) tests/cases/variables/ok/variables-aggregate-or-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-aggregate-or-pattern-binding.elf
	$(BUILD_DIR)/variables/variables-aggregate-or-pattern-binding.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-access.ari -o $(BUILD_DIR)/variables/ownership-aggregate-field-access.elf
	$(BUILD_DIR)/variables/ownership-aggregate-field-access.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-field-move.elf
	$(BUILD_DIR)/variables/ownership-aggregate-field-move.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-nested-field-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.elf
	$(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.elf; code=$$?; test $$code -eq 13
	$(TARGET) tests/cases/ownership/ok/ownership-array-element-move.ari -o $(BUILD_DIR)/variables/ownership-array-element-move.elf
	$(BUILD_DIR)/variables/ownership-array-element-move.elf; code=$$?; test $$code -eq 6
	$(TARGET) tests/cases/ownership/ok/drop-statement-payloads.ari -o $(BUILD_DIR)/variables/drop-statement-payloads.elf
	$(BUILD_DIR)/variables/drop-statement-payloads.elf; code=$$?; test $$code -eq 9
	$(TARGET) tests/cases/variables/ok/assignment-statement-payloads.ari -o $(BUILD_DIR)/variables/assignment-statement-payloads.elf
	$(BUILD_DIR)/variables/assignment-statement-payloads.elf; code=$$?; test $$code -eq 44
	$(TARGET) tests/cases/variables/ok/array-fixed.ari -o $(BUILD_DIR)/variables/array-fixed.elf
	$(BUILD_DIR)/variables/array-fixed.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/match/ok/scalar-payload-unions.ari -o $(BUILD_DIR)/variables/scalar-payload-unions.elf
	$(BUILD_DIR)/variables/scalar-payload-unions.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/variables/ok/array-literal-default.ari -o $(BUILD_DIR)/variables/array-literal-default.elf
	$(BUILD_DIR)/variables/array-literal-default.elf; code=$$?; test $$code -eq 14
	$(TARGET) tests/cases/variables/ok/array-dynamic-index.ari -o $(BUILD_DIR)/variables/array-dynamic-index.elf
	$(BUILD_DIR)/variables/array-dynamic-index.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/vectors/ok/vector-local-llvm.ari -o $(BUILD_DIR)/variables/vector-local-llvm.elf
	$(BUILD_DIR)/variables/vector-local-llvm.elf; code=$$?; test $$code -eq 152
	$(TARGET) tests/cases/variables/ok/array-len-literal.ari -o $(BUILD_DIR)/variables/array-len-literal.elf
	$(BUILD_DIR)/variables/array-len-literal.elf; code=$$?; test $$code -eq 4
	$(TARGET) tests/cases/variables/ok/variables-basic.ari --emit-llvm $(BUILD_DIR)/variables/variables-basic.ll
	grep -q "alloca i64" $(BUILD_DIR)/variables/variables-basic.ll
	grep -q "alloca i1" $(BUILD_DIR)/variables/variables-basic.ll
	$(TARGET) tests/cases/variables/ok/variables-widths.ari --emit-llvm $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i8" $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i16" $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i32" $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i64" $(BUILD_DIR)/variables/variables-widths.ll
	$(TARGET) tests/cases/variables/ok/variables-tuple-pattern.ari --emit-llvm $(BUILD_DIR)/variables/variables-tuple-pattern.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/variables-tuple-pattern.ll
	grep -q "extractvalue { { i64, i1 }, i64 }" $(BUILD_DIR)/variables/variables-tuple-pattern.ll
	$(TARGET) tests/cases/ir/ok/ir-builder-smoke.ari --emit-llvm $(BUILD_DIR)/variables/ir-builder-smoke.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/ir-builder-smoke.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/ir-builder-smoke.ll
	$(TARGET) tests/cases/ir/ok/ir-builder-aggregate-paths.ari --emit-llvm $(BUILD_DIR)/variables/ir-builder-aggregate-paths.ll
	grep -q "insertvalue { i64, i64 }" $(BUILD_DIR)/variables/ir-builder-aggregate-paths.ll
	grep -q "insertvalue { { i64, i64 }, i64 }" $(BUILD_DIR)/variables/ir-builder-aggregate-paths.ll
	grep -q "for.body" $(BUILD_DIR)/variables/ir-builder-aggregate-paths.ll
	$(TARGET) tests/cases/ir/ok/ir-argument-payloads.ari -o $(BUILD_DIR)/variables/ir-argument-payloads.elf
	$(BUILD_DIR)/variables/ir-argument-payloads.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/ir/ok/ir-argument-payloads.ari --emit-llvm $(BUILD_DIR)/variables/ir-argument-payloads.ll
	grep -q 'call i64 @"_ARNv3inc"' $(BUILD_DIR)/variables/ir-argument-payloads.ll
	grep -q "insertvalue { i64, i64 }" $(BUILD_DIR)/variables/ir-argument-payloads.ll
	$(TARGET) tests/cases/ir/ok/ir-builder-access-paths.ari --emit-llvm $(BUILD_DIR)/variables/ir-builder-access-paths.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/ir-builder-access-paths.ll
	grep -q "getelementptr inbounds \\[3 x i64\\]" $(BUILD_DIR)/variables/ir-builder-access-paths.ll
	grep -q "getelementptr inbounds { i64, i64 }" $(BUILD_DIR)/variables/ir-builder-access-paths.ll
	$(TARGET) tests/cases/ir/ok/ir-builder-literal-paths.ari --emit-llvm $(BUILD_DIR)/variables/ir-builder-literal-paths.ll
	grep -q "double 1.25000000000000000e+00" $(BUILD_DIR)/variables/ir-builder-literal-paths.ll
	grep -q "@.ari.str" $(BUILD_DIR)/variables/ir-builder-literal-paths.ll
	grep -q "ptr null" $(BUILD_DIR)/variables/ir-builder-literal-paths.ll
	grep -q "insertvalue { double, ptr }" $(BUILD_DIR)/variables/ir-builder-literal-paths.ll
	$(TARGET) tests/cases/ir/ok/ir-string-payloads.ari --emit-llvm $(BUILD_DIR)/variables/ir-string-payloads.ll
	grep -q "@.ari.str" $(BUILD_DIR)/variables/ir-string-payloads.ll
	grep -q 'call i64 @"_ARNv7add_one"' $(BUILD_DIR)/variables/ir-string-payloads.ll
	grep -q 'call i64 %' $(BUILD_DIR)/variables/ir-string-payloads.ll
	grep -q "getelementptr inbounds { i64 }" $(BUILD_DIR)/variables/ir-string-payloads.ll
	$(TARGET) tests/cases/ir/ok/ir-builder-call-paths.ari --emit-llvm $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	grep -q 'call i64 @"_ARNv3add"' $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	grep -q "_ARNv15identity__G_i64" $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	grep -q "_ARNv4impl5Point8inherent3new" $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	grep -q "_ARNv4impl5Point5Build5build" $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	grep -q "_ARNv4impl3i649LeftScore5score" $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/variables/ir-builder-call-paths.ll
	$(TARGET) tests/cases/match/ok/scalar-payload-unions.ari --emit-llvm $(BUILD_DIR)/variables/scalar-payload-unions.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/scalar-payload-unions.ll
	grep -q "getelementptr inbounds \\[3 x i64\\]" $(BUILD_DIR)/variables/scalar-payload-unions.ll
	grep -q "fadd double" $(BUILD_DIR)/variables/scalar-payload-unions.ll
	grep -q "fcmp" $(BUILD_DIR)/variables/scalar-payload-unions.ll
	$(TARGET) tests/cases/structs/ok/unit-empty-tuple.ari --emit-llvm $(BUILD_DIR)/variables/unit-empty-tuple.ll
	grep -q "define {}" $(BUILD_DIR)/variables/unit-empty-tuple.ll
	grep -q "store {} zeroinitializer" $(BUILD_DIR)/variables/unit-empty-tuple.ll
	$(TARGET) tests/cases/variables/ok/variables-struct-pattern.ari --emit-llvm $(BUILD_DIR)/variables/variables-struct-pattern.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/variables-struct-pattern.ll
	grep -q "extractvalue { { i64, i64 }, { i64, i64 } }" $(BUILD_DIR)/variables/variables-struct-pattern.ll
	$(TARGET) tests/cases/variables/ok/variables-enum-pattern-binding.ari --emit-llvm $(BUILD_DIR)/variables/variables-enum-pattern-binding.ll
	grep -q "match.arm" $(BUILD_DIR)/variables/variables-enum-pattern-binding.ll
	$(TARGET) tests/cases/variables/ok/variables-enum-or-pattern-binding.ari --emit-llvm $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.ll
	grep -q "match.arm" $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.ll
	grep -q "store i64" $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.ll
	$(TARGET) tests/cases/variables/ok/variables-aggregate-or-pattern-binding.ari --emit-llvm $(BUILD_DIR)/variables/variables-aggregate-or-pattern-binding.ll
	grep -q "@ari_builtin_panic" $(BUILD_DIR)/variables/variables-aggregate-or-pattern-binding.ll
	grep -q "store i64" $(BUILD_DIR)/variables/variables-aggregate-or-pattern-binding.ll
	$(TARGET) tests/cases/variables/ok/variables-enum-pattern-binding-trap.ari --emit-llvm $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.ll
	grep -q "@ari_builtin_panic" $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.ll
	$(TARGET) tests/cases/variables/ok/variables-host-scalars.ari --emit-llvm $(BUILD_DIR)/variables/variables-host-scalars.ll
	grep -q "alloca ptr" $(BUILD_DIR)/variables/variables-host-scalars.ll
	grep -q "alloca double" $(BUILD_DIR)/variables/variables-host-scalars.ll
	grep -q "store double 1.00000000000000000e+01" $(BUILD_DIR)/variables/variables-host-scalars.ll
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-storage.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-storage.ll
	grep -q "alloca { i64, i64 }" $(BUILD_DIR)/variables/ownership-aggregate-storage.ll
	grep -q "alloca { ptr, ptr }" $(BUILD_DIR)/variables/ownership-aggregate-storage.ll
	$(TARGET) tests/cases/ownership/ok/drop-aggregate-fields.ari --emit-llvm $(BUILD_DIR)/variables/drop-aggregate-fields.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/drop-aggregate-fields.ll) -eq 2
	$(TARGET) tests/cases/ownership/ok/drop-aggregate-moved-field.ari --emit-llvm $(BUILD_DIR)/variables/drop-aggregate-moved-field.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/drop-aggregate-moved-field.ll) -eq 2
	$(TARGET) tests/cases/match/ok/stored-owned-enum-payload-drop.ari --emit-llvm $(BUILD_DIR)/variables/stored-owned-enum-payload-drop.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/stored-owned-enum-payload-drop.ll) -eq 3
	$(TARGET) tests/cases/match/ok/stored-owned-enum-payload-drop.ari -o $(BUILD_DIR)/variables/stored-owned-enum-payload-drop.elf
	$(BUILD_DIR)/variables/stored-owned-enum-payload-drop.elf; code=$$?; test $$code -eq 44
	if $(TARGET) tests/cases/match/errors/stored-owned-enum-payload-undropped.ari -o $(BUILD_DIR)/variables/stored-owned-enum-payload-undropped.elf >$(BUILD_DIR)/variables/stored-owned-enum-payload-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'item' must be moved or dropped before return" $(BUILD_DIR)/variables/stored-owned-enum-payload-undropped.err
	$(TARGET) tests/cases/ownership/ok/drop-runtime-owned-enum-payload.ari --emit-llvm $(BUILD_DIR)/variables/drop-runtime-owned-enum-payload.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/drop-runtime-owned-enum-payload.ll) -eq 2
	grep -q "icmp eq i64" $(BUILD_DIR)/variables/drop-runtime-owned-enum-payload.ll
	$(TARGET) tests/cases/ownership/ok/drop-runtime-owned-enum-payload.ari -o $(BUILD_DIR)/variables/drop-runtime-owned-enum-payload.elf
	$(BUILD_DIR)/variables/drop-runtime-owned-enum-payload.elf; code=$$?; test $$code -eq 42
	if $(TARGET) tests/cases/ownership/errors/drop-runtime-owned-enum-payload-undropped.ari -o $(BUILD_DIR)/variables/drop-runtime-owned-enum-payload-undropped.elf >$(BUILD_DIR)/variables/drop-runtime-owned-enum-payload-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'item' must be moved or dropped before return" $(BUILD_DIR)/variables/drop-runtime-owned-enum-payload-undropped.err
	$(TARGET) tests/cases/match/ok/match-runtime-owned-enum-payload.ari -o $(BUILD_DIR)/variables/match-runtime-owned-enum-payload.elf
	$(BUILD_DIR)/variables/match-runtime-owned-enum-payload.elf; code=$$?; test $$code -eq 42
	if $(TARGET) tests/cases/match/errors/match-runtime-owned-enum-payload-undropped.ari -o $(BUILD_DIR)/variables/match-runtime-owned-enum-payload-undropped.elf >$(BUILD_DIR)/variables/match-runtime-owned-enum-payload-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before return" $(BUILD_DIR)/variables/match-runtime-owned-enum-payload-undropped.err
	$(TARGET) tests/cases/match/ok/runtime-owned-enum-uniform-payload-slot-move.ari -o $(BUILD_DIR)/variables/runtime-owned-enum-uniform-payload-slot-move.elf
	$(BUILD_DIR)/variables/runtime-owned-enum-uniform-payload-slot-move.elf; code=$$?; test $$code -eq 42
	if $(TARGET) tests/cases/match/errors/runtime-owned-enum-conditional-payload-slot-move.ari -o $(BUILD_DIR)/variables/runtime-owned-enum-conditional-payload-slot-move.elf >$(BUILD_DIR)/variables/runtime-owned-enum-conditional-payload-slot-move.err 2>&1; then exit 1; fi
	grep -q "cannot move runtime-dependent owning enum payload slot 'item.0' outside match because enum cases do not share the same owner payload layout; use match" $(BUILD_DIR)/variables/runtime-owned-enum-conditional-payload-slot-move.err
	$(TARGET) tests/cases/match/ok/runtime-owned-enum-tagged-payload-paths.ari -o $(BUILD_DIR)/variables/runtime-owned-enum-tagged-payload-paths.elf
	$(BUILD_DIR)/variables/runtime-owned-enum-tagged-payload-paths.elf; code=$$?; test $$code -eq 45
	$(TARGET) tests/cases/ownership/ok/drop-statement-payloads.ari --emit-llvm $(BUILD_DIR)/variables/drop-statement-payloads.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/drop-statement-payloads.ll) -eq 3
	$(TARGET) tests/cases/ownership/ok/forget-maybe-unavailable-owner.ari -o $(BUILD_DIR)/variables/forget-maybe-unavailable-owner.elf
	$(BUILD_DIR)/variables/forget-maybe-unavailable-owner.elf; code=$$?; test $$code -eq 14
	$(TARGET) tests/cases/ownership/ok/forget-skips-drop-lowering.ari --emit-llvm $(BUILD_DIR)/variables/forget-skips-drop-lowering.ll
	if grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/forget-skips-drop-lowering.ll; then exit 1; fi
	$(TARGET) tests/cases/ir/ok/local-state-mutation-api.ari --emit-llvm $(BUILD_DIR)/variables/local-state-mutation-api.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/local-state-mutation-api.ll) -eq 3
	grep -q "store i64 1, ptr %current_len" $(BUILD_DIR)/variables/local-state-mutation-api.ll
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-access.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-field-access.ll
	grep -q "store i64" $(BUILD_DIR)/variables/ownership-aggregate-field-access.ll
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-field-move.ll
	grep -q "extractvalue" $(BUILD_DIR)/variables/ownership-aggregate-field-move.ll
	$(TARGET) tests/cases/ownership/ok/ownership-aggregate-nested-field-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.ll
	grep -q "extractvalue" $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.ll
	$(TARGET) tests/cases/ownership/ok/ownership-array-element-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-array-element-move.ll
	grep -q "\\[2 x i64\\]" $(BUILD_DIR)/variables/ownership-array-element-move.ll
	$(TARGET) tests/cases/ownership/ok/ownership-vector-element-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-vector-element-move.ll
	grep -q "getelementptr inbounds { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/ownership-vector-element-move.ll
	$(TARGET) tests/cases/variables/ok/array-fixed.ari --emit-llvm $(BUILD_DIR)/variables/array-fixed.ll
	grep -q "\\[3 x i64\\]" $(BUILD_DIR)/variables/array-fixed.ll
	$(TARGET) tests/cases/variables/ok/array-literal-default.ari --emit-llvm $(BUILD_DIR)/variables/array-literal-default.ll
	grep -q "alloca \\[3 x i64\\]" $(BUILD_DIR)/variables/array-literal-default.ll
	$(TARGET) tests/cases/variables/ok/array-dynamic-index.ari --emit-llvm $(BUILD_DIR)/variables/array-dynamic-index.ll
	grep -q "getelementptr inbounds \\[3 x i64\\]" $(BUILD_DIR)/variables/array-dynamic-index.ll
	grep -q "array.bounds.fail" $(BUILD_DIR)/variables/array-dynamic-index.ll
	$(TARGET) tests/cases/vectors/ok/vector-storage.ari --emit-llvm $(BUILD_DIR)/variables/vector-storage.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-storage.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-storage.ll
	$(TARGET) tests/cases/vectors/ok/vector-storage-reassign.ari --emit-llvm $(BUILD_DIR)/variables/vector-storage-reassign.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-storage-reassign.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-storage-reassign.ll
	$(TARGET) tests/cases/vectors/ok/vector-empty-typed.ari --emit-llvm $(BUILD_DIR)/variables/vector-empty-typed.ll
	grep -q "store { i64, \\[0 x i64\\] }" $(BUILD_DIR)/variables/vector-empty-typed.ll
	grep -q "store { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-empty-typed.ll
	$(TARGET) tests/cases/vectors/ok/vector-len.ari --emit-llvm $(BUILD_DIR)/variables/vector-len.ll
	grep -q "store i64 2, ptr %before" $(BUILD_DIR)/variables/vector-len.ll
	grep -q "store i64 3, ptr %after" $(BUILD_DIR)/variables/vector-len.ll
	if grep -q "extractvalue { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-len.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-known-length-copy.ari --emit-llvm $(BUILD_DIR)/variables/vector-known-length-copy.ll
	grep -q "store i64 1, ptr %copied_len" $(BUILD_DIR)/variables/vector-known-length-copy.ll
	grep -q "store i64 1, ptr %assigned_len" $(BUILD_DIR)/variables/vector-known-length-copy.ll
	$(TARGET) tests/cases/vectors/ok/vector-copy-capacity.ari --emit-llvm $(BUILD_DIR)/variables/vector-copy-capacity.ll
	grep -q "%assigned.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-copy-capacity.ll
	grep -q "store i64 3, ptr %assigned_len" $(BUILD_DIR)/variables/vector-copy-capacity.ll
	$(TARGET) tests/cases/vectors/ok/vector-control-flow-capacity.ari --emit-llvm $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	grep -q "%assigned.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	grep -q "%from_block.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	grep -q "%from_locals.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	grep -q "store i64 3, ptr %assigned_len" $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	grep -q "store i64 3, ptr %block_len" $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	grep -q "store i64 3, ptr %locals_len" $(BUILD_DIR)/variables/vector-control-flow-capacity.ll
	$(TARGET) tests/cases/vectors/ok/vector-labeled-block-result.ari --emit-llvm $(BUILD_DIR)/variables/vector-labeled-block-result.ll
	grep -q "%from_match.addr.* = alloca { i64, \\[4 x i64\\] }" $(BUILD_DIR)/variables/vector-labeled-block-result.ll
	grep -q "%from_break.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-labeled-block-result.ll
	grep -q "%same_length.addr.* = alloca { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-labeled-block-result.ll
	grep -q "store i64 2, ptr %same_len" $(BUILD_DIR)/variables/vector-labeled-block-result.ll
	$(TARGET) tests/cases/vectors/ok/vector-control-flow-known-local-length.ari --emit-llvm $(BUILD_DIR)/variables/vector-control-flow-known-local-length.ll
	grep -q "store i64 1, ptr %if_len" $(BUILD_DIR)/variables/vector-control-flow-known-local-length.ll
	grep -q "store i64 1, ptr %match_len" $(BUILD_DIR)/variables/vector-control-flow-known-local-length.ll
	grep -q "store i64 1, ptr %direct_len" $(BUILD_DIR)/variables/vector-control-flow-known-local-length.ll
	grep -q "store i64 1, ptr %method_len" $(BUILD_DIR)/variables/vector-control-flow-known-local-length.ll
	grep -q "store i1 0, ptr %direct_empty" $(BUILD_DIR)/variables/vector-control-flow-known-local-length.ll
	$(TARGET) tests/cases/vectors/ok/vector-match-capacity.ari --emit-llvm $(BUILD_DIR)/variables/vector-match-capacity.ll
	grep -q "%from_match.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-match-capacity.ll
	grep -q "%from_iflet.addr.* = alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-match-capacity.ll
	grep -q "store i64 3, ptr %match_len" $(BUILD_DIR)/variables/vector-match-capacity.ll
	grep -q "store i64 3, ptr %iflet_len" $(BUILD_DIR)/variables/vector-match-capacity.ll
	$(TARGET) tests/cases/vectors/ok/vector-as-slice-known-len.ari --emit-llvm $(BUILD_DIR)/variables/vector-as-slice-known-len.ll
	grep -Eq "insertvalue \\{ ptr, i64 \\} .* i64 2, 1" $(BUILD_DIR)/variables/vector-as-slice-known-len.ll
	if grep -q "extractvalue { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-as-slice-known-len.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-push-reserve.ari --emit-llvm $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "vector.push.full" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i64 0, ptr" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i1 1, ptr %empty_before" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i1 0, ptr %empty_after" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i1 1, ptr %empty_after_clear" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "vector.truncate.negative" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i64 9, ptr" $(BUILD_DIR)/variables/vector-push-reserve.ll
	$(TARGET) tests/cases/vectors/ok/vector-reserve-runtime.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-runtime.ll
	grep -q "vector.reserve.fail" $(BUILD_DIR)/variables/vector-reserve-runtime.ll
	$(TARGET) tests/cases/vectors/ok/vector-reserve-runtime-trap.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-runtime-trap.ll
	grep -q "vector.reserve.fail" $(BUILD_DIR)/variables/vector-reserve-runtime-trap.ll
	$(TARGET) tests/cases/vectors/ok/vector-reserve-known-local.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-known-local.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-reserve-known-local.ll
	if grep -q "vector.reserve.fail" $(BUILD_DIR)/variables/vector-reserve-known-local.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-reserve-known-static-local.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-known-static-local.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-reserve-known-static-local.ll
	if grep -q "vector.reserve.fail" $(BUILD_DIR)/variables/vector-reserve-known-static-local.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-truncate-known-static-local.ari --emit-llvm $(BUILD_DIR)/variables/vector-truncate-known-static-local.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-truncate-known-static-local.ll
	if grep -q "alloca { i64, \\[4 x i64\\] }" $(BUILD_DIR)/variables/vector-truncate-known-static-local.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-static-folding-shared.ari --emit-llvm $(BUILD_DIR)/variables/vector-static-folding-shared.ll
	grep -q "alloca { i64, \\[4 x i64\\] }" $(BUILD_DIR)/variables/vector-static-folding-shared.ll
	if grep -q "alloca { i64, \\[5 x i64\\] }" $(BUILD_DIR)/variables/vector-static-folding-shared.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-reserve-bitwise-static.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-bitwise-static.ll
	grep -q "alloca { i64, \\[5 x i64\\] }" $(BUILD_DIR)/variables/vector-reserve-bitwise-static.ll
	if grep -q "alloca { i64, \\[6 x i64\\] }" $(BUILD_DIR)/variables/vector-reserve-bitwise-static.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-reserve-const.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-const.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-reserve-const.ll
	if grep -q "vector.reserve.fail" $(BUILD_DIR)/variables/vector-reserve-const.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-reserve-static-expr.ari --emit-llvm $(BUILD_DIR)/variables/vector-reserve-static-expr.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-reserve-static-expr.ll
	if grep -q "vector.reserve.fail" $(BUILD_DIR)/variables/vector-reserve-static-expr.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-push-autogrow.ari --emit-llvm $(BUILD_DIR)/variables/vector-push-autogrow.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-push-autogrow.ll
	grep -q "alloca { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-push-autogrow.ll
	$(TARGET) tests/cases/vectors/ok/vector-pop.ari --emit-llvm $(BUILD_DIR)/variables/vector-pop.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-pop.ll
	grep -q "vector.pop.empty" $(BUILD_DIR)/variables/vector-pop.ll
	$(TARGET) tests/cases/vectors/ok/vector-pop-push-capacity.ari --emit-llvm $(BUILD_DIR)/variables/vector-pop-push-capacity.ll
	grep -q "alloca { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-pop-push-capacity.ll
	if grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-pop-push-capacity.ll; then exit 1; fi
	$(TARGET) tests/cases/vectors/ok/vector-first-last.ari --emit-llvm $(BUILD_DIR)/variables/vector-first-last.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-first-last.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-first-last.ll
	$(TARGET) tests/cases/vectors/ok/vector-get.ari --emit-llvm $(BUILD_DIR)/variables/vector-get.ll
	grep -q "alloca { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-get.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-get.ll
	$(TARGET) tests/cases/vectors/ok/vector-swap.ari --emit-llvm $(BUILD_DIR)/variables/vector-swap.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-swap.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-swap.ll
	$(TARGET) tests/cases/vectors/ok/vector-remove.ari --emit-llvm $(BUILD_DIR)/variables/vector-remove.ll
	grep -q "alloca { i64, \\[4 x i64\\] }" $(BUILD_DIR)/variables/vector-remove.ll
	grep -q "vector.remove.body" $(BUILD_DIR)/variables/vector-remove.ll
	$(TARGET) tests/cases/vectors/ok/vector-insert.ari --emit-llvm $(BUILD_DIR)/variables/vector-insert.ll
	grep -q "alloca { i64, \\[5 x i64\\] }" $(BUILD_DIR)/variables/vector-insert.ll
	grep -q "vector.insert.body" $(BUILD_DIR)/variables/vector-insert.ll
	$(TARGET) tests/cases/vectors/ok/vector-search.ari --emit-llvm $(BUILD_DIR)/variables/vector-search.ll
	grep -q "vector.contains.body" $(BUILD_DIR)/variables/vector-search.ll
	grep -q "vector.index_of.body" $(BUILD_DIR)/variables/vector-search.ll
	grep -q "vector.count.body" $(BUILD_DIR)/variables/vector-search.ll
	$(TARGET) tests/cases/variables/ok/array-len-literal.ari --emit-llvm $(BUILD_DIR)/variables/array-len-literal.ll
	grep -q "ret i64 4" $(BUILD_DIR)/variables/array-len-literal.ll
	$(TARGET) tests/cases/borrowing/ok/field-borrow.ari --emit-llvm $(BUILD_DIR)/variables/field-borrow.ll
	grep -q "getelementptr inbounds { i64, i64 }" $(BUILD_DIR)/variables/field-borrow.ll
	$(TARGET) tests/cases/variables/ok/assignment-statement-payloads.ari --emit-llvm $(BUILD_DIR)/variables/assignment-statement-payloads.ll
	grep -q "store i64" $(BUILD_DIR)/variables/assignment-statement-payloads.ll
	grep -q "add i64" $(BUILD_DIR)/variables/assignment-statement-payloads.ll
	grep -q "getelementptr inbounds" $(BUILD_DIR)/variables/assignment-statement-payloads.ll
	$(TARGET) tests/cases/variables/ok/array-dynamic-index-trap.ari --emit-llvm $(BUILD_DIR)/variables/array-dynamic-index-trap.ll
	grep -q "array.bounds.fail" $(BUILD_DIR)/variables/array-dynamic-index-trap.ll
	$(TARGET) tests/cases/variables/ok/array-dynamic-index-trap.ari -o $(BUILD_DIR)/variables/array-dynamic-index-trap.elf
	if $(BUILD_DIR)/variables/array-dynamic-index-trap.elf; then exit 1; fi
	$(TARGET) tests/cases/variables/ok/variables-enum-pattern-binding-trap.ari -o $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.elf
	if $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.elf; then exit 1; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/variables-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-basic.elf; $(BUILD_DIR)/variables/variables-basic.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ir/ok/ir-builder-smoke.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ir-builder-smoke.elf; $(BUILD_DIR)/variables/ir-builder-smoke.elf; code=$$?; test $$code -eq 16; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ir/ok/ir-builder-call-paths.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ir-builder-call-paths.elf; $(BUILD_DIR)/variables/ir-builder-call-paths.elf; code=$$?; test $$code -eq 90; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/structs/ok/unit-empty-tuple.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/unit-empty-tuple.elf; $(BUILD_DIR)/variables/unit-empty-tuple.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/variables-host-scalars.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-host-scalars.elf; $(BUILD_DIR)/variables/variables-host-scalars.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ownership/ok/ownership-aggregate-storage.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-storage.elf; $(BUILD_DIR)/variables/ownership-aggregate-storage.elf; code=$$?; test $$code -eq 47; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-field-access.elf; $(BUILD_DIR)/variables/ownership-aggregate-field-access.elf; code=$$?; test $$code -eq 7; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ownership/ok/ownership-aggregate-field-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-field-move.elf; $(BUILD_DIR)/variables/ownership-aggregate-field-move.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ownership/ok/ownership-aggregate-nested-field-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.elf; $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.elf; code=$$?; test $$code -eq 13; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ownership/ok/ownership-array-element-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-array-element-move.elf; $(BUILD_DIR)/variables/ownership-array-element-move.elf; code=$$?; test $$code -eq 6; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ownership/ok/ownership-vector-element-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-vector-element-move.elf; $(BUILD_DIR)/variables/ownership-vector-element-move.elf; code=$$?; test $$code -eq 8; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/array-fixed.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-fixed.elf; $(BUILD_DIR)/variables/array-fixed.elf; code=$$?; test $$code -eq 4; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/array-literal-default.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-literal-default.elf; $(BUILD_DIR)/variables/array-literal-default.elf; code=$$?; test $$code -eq 14; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/array-dynamic-index.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-dynamic-index.elf; $(BUILD_DIR)/variables/array-dynamic-index.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-storage.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-storage.elf; $(BUILD_DIR)/variables/vector-storage.elf; code=$$?; test $$code -eq 16; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-storage-reassign.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-storage-reassign.elf; $(BUILD_DIR)/variables/vector-storage-reassign.elf; code=$$?; test $$code -eq 14; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-empty-typed.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-empty-typed.elf; $(BUILD_DIR)/variables/vector-empty-typed.elf; code=$$?; test $$code -eq 3; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-len.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-len.elf; $(BUILD_DIR)/variables/vector-len.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-push-reserve.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-push-reserve.elf; $(BUILD_DIR)/variables/vector-push-reserve.elf; code=$$?; test $$code -eq 24; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-runtime.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-runtime.elf; $(BUILD_DIR)/variables/vector-reserve-runtime.elf; code=$$?; test $$code -eq 6; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-runtime-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-runtime-trap.elf; if $(BUILD_DIR)/variables/vector-reserve-runtime-trap.elf; then exit 1; fi; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-known-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-known-local.elf; $(BUILD_DIR)/variables/vector-reserve-known-local.elf; code=$$?; test $$code -eq 18; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-known-static-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-known-static-local.elf; $(BUILD_DIR)/variables/vector-reserve-known-static-local.elf; code=$$?; test $$code -eq 18; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-truncate-known-static-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-truncate-known-static-local.elf; $(BUILD_DIR)/variables/vector-truncate-known-static-local.elf; code=$$?; test $$code -eq 14; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-static-folding-shared.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-static-folding-shared.elf; $(BUILD_DIR)/variables/vector-static-folding-shared.elf; code=$$?; test $$code -eq 17; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-bitwise-static.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-bitwise-static.elf; $(BUILD_DIR)/variables/vector-reserve-bitwise-static.elf; code=$$?; test $$code -eq 17; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-const.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-const.elf; $(BUILD_DIR)/variables/vector-reserve-const.elf; code=$$?; test $$code -eq 18; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-reserve-static-expr.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-reserve-static-expr.elf; $(BUILD_DIR)/variables/vector-reserve-static-expr.elf; code=$$?; test $$code -eq 18; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-push-autogrow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-push-autogrow.elf; $(BUILD_DIR)/variables/vector-push-autogrow.elf; code=$$?; test $$code -eq 23; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-pop.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-pop.elf; $(BUILD_DIR)/variables/vector-pop.elf; code=$$?; test $$code -eq 26; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-pop-push-capacity.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-pop-push-capacity.elf; $(BUILD_DIR)/variables/vector-pop-push-capacity.elf; code=$$?; test $$code -eq 9; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-first-last.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-first-last.elf; $(BUILD_DIR)/variables/vector-first-last.elf; code=$$?; test $$code -eq 20; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-get.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-get.elf; $(BUILD_DIR)/variables/vector-get.elf; code=$$?; test $$code -eq 26; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-swap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-swap.elf; $(BUILD_DIR)/variables/vector-swap.elf; code=$$?; test $$code -eq 24; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-remove.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-remove.elf; $(BUILD_DIR)/variables/vector-remove.elf; code=$$?; test $$code -eq 32; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-insert.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-insert.elf; $(BUILD_DIR)/variables/vector-insert.elf; code=$$?; test $$code -eq 29; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-search.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-search.elf; $(BUILD_DIR)/variables/vector-search.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/field-borrow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/field-borrow.elf; $(BUILD_DIR)/variables/field-borrow.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/cases/borrowing/ok/aggregate-borrow-field-assignment.ari -o $(BUILD_DIR)/variables/aggregate-borrow-field-assignment.elf
	$(BUILD_DIR)/variables/aggregate-borrow-field-assignment.elf; code=$$?; test $$code -eq 32
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/array-dynamic-index-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-dynamic-index-trap.elf; if $(BUILD_DIR)/variables/array-dynamic-index-trap.elf; then exit 1; fi; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/vectors/ok/vector-storage-shrink-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-storage-shrink-trap.elf; if $(BUILD_DIR)/variables/vector-storage-shrink-trap.elf; then exit 1; fi; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/variables-enum-pattern-binding.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-enum-pattern-binding.elf; $(BUILD_DIR)/variables/variables-enum-pattern-binding.elf; code=$$?; test $$code -eq 13; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/variables-enum-or-pattern-binding.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.elf; $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.elf; code=$$?; test $$code -eq 33; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/variables/ok/variables-enum-pattern-binding-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.elf; if $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.elf; then exit 1; fi; fi
	if $(TARGET) tests/cases/variables/errors/variables-assign-immutable.ari -o $(BUILD_DIR)/variables/variables-assign-immutable.elf >$(BUILD_DIR)/variables/variables-assign-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'value'" $(BUILD_DIR)/variables/variables-assign-immutable.err
	if $(TARGET) tests/cases/variables/errors/variables-local-shadowing.ari -o $(BUILD_DIR)/variables/variables-local-shadowing.elf >$(BUILD_DIR)/variables/variables-local-shadowing.err 2>&1; then exit 1; fi
	grep -q "local name 'value' shadows or redeclares an existing local" $(BUILD_DIR)/variables/variables-local-shadowing.err
	if $(TARGET) tests/cases/variables/errors/variables-local-shadowing-block.ari -o $(BUILD_DIR)/variables/variables-local-shadowing-block.elf >$(BUILD_DIR)/variables/variables-local-shadowing-block.err 2>&1; then exit 1; fi
	grep -q "local name 'value' shadows or redeclares an existing local" $(BUILD_DIR)/variables/variables-local-shadowing-block.err
	if $(TARGET) tests/cases/variables/errors/variables-assign-type-mismatch.ari -o $(BUILD_DIR)/variables/variables-assign-type-mismatch.elf >$(BUILD_DIR)/variables/variables-assign-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i32, got bool" $(BUILD_DIR)/variables/variables-assign-type-mismatch.err
	if $(TARGET) tests/cases/borrowing/errors/aggregate-borrow-source-assign.ari -o $(BUILD_DIR)/variables/aggregate-borrow-source-assign.elf >$(BUILD_DIR)/variables/aggregate-borrow-source-assign.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'number'" $(BUILD_DIR)/variables/aggregate-borrow-source-assign.err
	if $(TARGET) tests/cases/borrowing/errors/aggregate-borrow-reassigned-source.ari -o $(BUILD_DIR)/variables/aggregate-borrow-reassigned-source.elf >$(BUILD_DIR)/variables/aggregate-borrow-reassigned-source.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'second'" $(BUILD_DIR)/variables/aggregate-borrow-reassigned-source.err
	$(TARGET) tests/cases/borrowing/ok/aggregate-borrow-source-release.ari -o $(BUILD_DIR)/variables/aggregate-borrow-source-release.elf
	$(BUILD_DIR)/variables/aggregate-borrow-source-release.elf; code=$$?; test $$code -eq 3
	if $(TARGET) tests/cases/borrowing/errors/field-borrow-assign.ari -o $(BUILD_DIR)/variables/field-borrow-assign.elf >$(BUILD_DIR)/variables/field-borrow-assign.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed field 'pair.0'" $(BUILD_DIR)/variables/field-borrow-assign.err
	if $(TARGET) tests/cases/borrowing/errors/field-borrow-conflict.ari -o $(BUILD_DIR)/variables/field-borrow-conflict.elf >$(BUILD_DIR)/variables/field-borrow-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow already borrowed field 'pair.0'" $(BUILD_DIR)/variables/field-borrow-conflict.err
	if $(TARGET) tests/cases/ownership/errors/aggregate-owner-undropped.ari -o $(BUILD_DIR)/variables/aggregate-owner-undropped.elf >$(BUILD_DIR)/variables/aggregate-owner-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'pair' must be moved or dropped before return" $(BUILD_DIR)/variables/aggregate-owner-undropped.err
	if $(TARGET) tests/cases/ownership/errors/ownership-aggregate-field-use-after-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-field-use-after-move.elf >$(BUILD_DIR)/variables/ownership-aggregate-field-use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'holder.token'" $(BUILD_DIR)/variables/ownership-aggregate-field-use-after-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-aggregate-partial-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-partial-move.elf >$(BUILD_DIR)/variables/ownership-aggregate-partial-move.err 2>&1; then exit 1; fi
	grep -q "cannot move partially moved owning binding 'holder'" $(BUILD_DIR)/variables/ownership-aggregate-partial-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-temporary-field-move.ari -o $(BUILD_DIR)/variables/ownership-temporary-field-move.elf >$(BUILD_DIR)/variables/ownership-temporary-field-move.err 2>&1; then exit 1; fi
	grep -q "moving owned fields out of temporary aggregate values is not supported; bind the aggregate first" $(BUILD_DIR)/variables/ownership-temporary-field-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-temporary-tuple-field-move.ari -o $(BUILD_DIR)/variables/ownership-temporary-tuple-field-move.elf >$(BUILD_DIR)/variables/ownership-temporary-tuple-field-move.err 2>&1; then exit 1; fi
	grep -q "moving owned fields out of temporary aggregate values is not supported; bind the aggregate first" $(BUILD_DIR)/variables/ownership-temporary-tuple-field-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-temporary-array-element-move.ari -o $(BUILD_DIR)/variables/ownership-temporary-array-element-move.elf >$(BUILD_DIR)/variables/ownership-temporary-array-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements out of temporary values is not supported; bind the aggregate first" $(BUILD_DIR)/variables/ownership-temporary-array-element-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-array-element-use-after-move.ari -o $(BUILD_DIR)/variables/ownership-array-element-use-after-move.elf >$(BUILD_DIR)/variables/ownership-array-element-use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'values.0'" $(BUILD_DIR)/variables/ownership-array-element-use-after-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-array-dynamic-element-move.ari -o $(BUILD_DIR)/variables/ownership-array-dynamic-element-move.elf >$(BUILD_DIR)/variables/ownership-array-dynamic-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements through dynamic indexes is not supported; use a constant index or move the whole aggregate" $(BUILD_DIR)/variables/ownership-array-dynamic-element-move.err
	if $(TARGET) tests/cases/ownership/errors/ownership-vector-dynamic-element-move.ari -o $(BUILD_DIR)/variables/ownership-vector-dynamic-element-move.elf >$(BUILD_DIR)/variables/ownership-vector-dynamic-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements through dynamic indexes is not supported; use a constant index or move the whole aggregate" $(BUILD_DIR)/variables/ownership-vector-dynamic-element-move.err
	if $(TARGET) tests/cases/variables/errors/array-size-mismatch.ari -o $(BUILD_DIR)/variables/array-size-mismatch.elf >$(BUILD_DIR)/variables/array-size-mismatch.err 2>&1; then exit 1; fi
	grep -q "array literal has 3 elements but type expects 2" $(BUILD_DIR)/variables/array-size-mismatch.err
	if $(TARGET) tests/cases/variables/errors/array-index-out-of-range.ari -o $(BUILD_DIR)/variables/array-index-out-of-range.elf >$(BUILD_DIR)/variables/array-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "array index 2 is out of range for 2 elements" $(BUILD_DIR)/variables/array-index-out-of-range.err
	if $(TARGET) tests/cases/vectors/errors/vector-len-non-vec.ari -o $(BUILD_DIR)/variables/vector-len-non-vec.elf >$(BUILD_DIR)/variables/vector-len-non-vec.err 2>&1; then exit 1; fi
	grep -q "len expects an array, Vec, or Slice value, got i64" $(BUILD_DIR)/variables/vector-len-non-vec.err
	if $(TARGET) tests/cases/vectors/errors/collection-len-method-shape.ari -o $(BUILD_DIR)/variables/collection-len-method-shape.elf >$(BUILD_DIR)/variables/collection-len-method-shape.err 2>&1; then exit 1; fi
	grep -q "len expects no method arguments" $(BUILD_DIR)/variables/collection-len-method-shape.err
	if $(TARGET) tests/cases/vectors/errors/vector-empty-untyped.ari -o $(BUILD_DIR)/variables/vector-empty-untyped.elf >$(BUILD_DIR)/variables/vector-empty-untyped.err 2>&1; then exit 1; fi
	grep -Fq "empty [] literals need an explicit Vec[T] type or non-empty array elements" $(BUILD_DIR)/variables/vector-empty-untyped.err
	if $(TARGET) tests/cases/vectors/errors/vector-empty-non-vec-expected.ari -o $(BUILD_DIR)/variables/vector-empty-non-vec-expected.elf >$(BUILD_DIR)/variables/vector-empty-non-vec-expected.err 2>&1; then exit 1; fi
	grep -Fq "empty [] literals need an explicit Vec[T] type or non-empty array elements" $(BUILD_DIR)/variables/vector-empty-non-vec-expected.err
	$(TARGET) tests/cases/vectors/ok/vector-as-ptr.ari -o $(BUILD_DIR)/variables/vector-as-ptr.elf
	$(BUILD_DIR)/variables/vector-as-ptr.elf; code=$$?; test $$code -eq 65
	$(TARGET) tests/cases/vectors/ok/vector-as-ptr.ari --emit-llvm $(BUILD_DIR)/variables/vector-as-ptr.ll
	grep -F -q "getelementptr inbounds { i64, [3 x i64] }, ptr" $(BUILD_DIR)/variables/vector-as-ptr.ll
	if $(TARGET) tests/cases/vectors/errors/vector-api-freeze.ari -o $(BUILD_DIR)/variables/vector-api-freeze.elf >$(BUILD_DIR)/variables/vector-api-freeze.err 2>&1; then exit 1; fi
	grep -Fq "local Vec method 'map' is reserved for allocator-backed std collection APIs" $(BUILD_DIR)/variables/vector-api-freeze.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-shape.ari -o $(BUILD_DIR)/variables/vector-method-shape.elf >$(BUILD_DIR)/variables/vector-method-shape.err 2>&1; then exit 1; fi
	grep -q "Vec.push expects one value argument" $(BUILD_DIR)/variables/vector-method-shape.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-immutable.ari -o $(BUILD_DIR)/variables/vector-method-immutable.elf >$(BUILD_DIR)/variables/vector-method-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot call Vec.push on immutable binding 'values'" $(BUILD_DIR)/variables/vector-method-immutable.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-index-type.ari -o $(BUILD_DIR)/variables/vector-method-index-type.elf >$(BUILD_DIR)/variables/vector-method-index-type.err 2>&1; then exit 1; fi
	grep -q "Vec.get index must be an integer, got bool" $(BUILD_DIR)/variables/vector-method-index-type.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-static-negative-index.ari -o $(BUILD_DIR)/variables/vector-method-static-negative-index.elf >$(BUILD_DIR)/variables/vector-method-static-negative-index.err 2>&1; then exit 1; fi
	grep -q "Vec.get index must be non-negative" $(BUILD_DIR)/variables/vector-method-static-negative-index.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-static-index-out-of-range.ari -o $(BUILD_DIR)/variables/vector-method-static-index-out-of-range.elf >$(BUILD_DIR)/variables/vector-method-static-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "Vec.get index 2 is out of range for 2 elements" $(BUILD_DIR)/variables/vector-method-static-index-out-of-range.err
	if $(TARGET) tests/cases/vectors/errors/vector-index-known-length-out-of-range.ari -o $(BUILD_DIR)/variables/vector-index-known-length-out-of-range.elf >$(BUILD_DIR)/variables/vector-index-known-length-out-of-range.err 2>&1; then exit 1; fi
	grep -q "vector index 1 is out of range for 1 elements" $(BUILD_DIR)/variables/vector-index-known-length-out-of-range.err
	if $(TARGET) tests/cases/vectors/errors/vector-index-known-length-copy-out-of-range.ari -o $(BUILD_DIR)/variables/vector-index-known-length-copy-out-of-range.elf >$(BUILD_DIR)/variables/vector-index-known-length-copy-out-of-range.err 2>&1; then exit 1; fi
	grep -q "vector index 1 is out of range for 1 elements" $(BUILD_DIR)/variables/vector-index-known-length-copy-out-of-range.err
	if $(TARGET) tests/cases/vectors/errors/vector-control-flow-known-local-index.ari -o $(BUILD_DIR)/variables/vector-control-flow-known-local-index.elf >$(BUILD_DIR)/variables/vector-control-flow-known-local-index.err 2>&1; then exit 1; fi
	grep -q "vector index 1 is out of range for 1 elements" $(BUILD_DIR)/variables/vector-control-flow-known-local-index.err
	if $(TARGET) tests/cases/vectors/errors/vector-index-known-empty-dynamic.ari -o $(BUILD_DIR)/variables/vector-index-known-empty-dynamic.elf >$(BUILD_DIR)/variables/vector-index-known-empty-dynamic.err 2>&1; then exit 1; fi
	grep -q "vector index requires a non-empty Vec" $(BUILD_DIR)/variables/vector-index-known-empty-dynamic.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-first.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-first.elf >$(BUILD_DIR)/variables/vector-method-known-empty-first.err 2>&1; then exit 1; fi
	grep -q "Vec.first requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-first.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-last-after-clear.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-last-after-clear.elf >$(BUILD_DIR)/variables/vector-method-known-empty-last-after-clear.err 2>&1; then exit 1; fi
	grep -q "Vec.last requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-last-after-clear.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-pop.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-pop.elf >$(BUILD_DIR)/variables/vector-method-known-empty-pop.err 2>&1; then exit 1; fi
	grep -q "Vec.pop requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-pop.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-get-dynamic.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-get-dynamic.elf >$(BUILD_DIR)/variables/vector-method-known-empty-get-dynamic.err 2>&1; then exit 1; fi
	grep -q "Vec.get requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-get-dynamic.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-remove-dynamic.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-remove-dynamic.elf >$(BUILD_DIR)/variables/vector-method-known-empty-remove-dynamic.err 2>&1; then exit 1; fi
	grep -q "Vec.remove requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-remove-dynamic.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-set-dynamic.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-set-dynamic.elf >$(BUILD_DIR)/variables/vector-method-known-empty-set-dynamic.err 2>&1; then exit 1; fi
	grep -q "Vec.set requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-set-dynamic.err
	if $(TARGET) tests/cases/vectors/errors/vector-method-known-empty-swap-after-clear.ari -o $(BUILD_DIR)/variables/vector-method-known-empty-swap-after-clear.elf >$(BUILD_DIR)/variables/vector-method-known-empty-swap-after-clear.err 2>&1; then exit 1; fi
	grep -q "Vec.swap requires a non-empty Vec" $(BUILD_DIR)/variables/vector-method-known-empty-swap-after-clear.err
	if $(TARGET) tests/cases/vectors/errors/slice-view-method-shape.ari -o $(BUILD_DIR)/variables/slice-view-method-shape.elf >$(BUILD_DIR)/variables/slice-view-method-shape.err 2>&1; then exit 1; fi
	grep -q "as_slice expects no arguments" $(BUILD_DIR)/variables/slice-view-method-shape.err
	if $(TARGET) tests/cases/vectors/errors/vector-reserve-known-negative.ari -o $(BUILD_DIR)/variables/vector-reserve-known-negative.elf >$(BUILD_DIR)/variables/vector-reserve-known-negative.err 2>&1; then exit 1; fi
	grep -q "Vec.reserve capacity must be non-negative" $(BUILD_DIR)/variables/vector-reserve-known-negative.err
	if $(TARGET) tests/cases/vectors/errors/vector-reserve-known-static-negative.ari -o $(BUILD_DIR)/variables/vector-reserve-known-static-negative.elf >$(BUILD_DIR)/variables/vector-reserve-known-static-negative.err 2>&1; then exit 1; fi
	grep -q "Vec.reserve capacity must be non-negative" $(BUILD_DIR)/variables/vector-reserve-known-static-negative.err
	if $(TARGET) tests/cases/vectors/errors/vector-reserve-const-negative.ari -o $(BUILD_DIR)/variables/vector-reserve-const-negative.elf >$(BUILD_DIR)/variables/vector-reserve-const-negative.err 2>&1; then exit 1; fi
	grep -q "Vec.reserve capacity must be non-negative" $(BUILD_DIR)/variables/vector-reserve-const-negative.err
	if $(TARGET) tests/cases/vectors/errors/vector-reserve-static-negative.ari -o $(BUILD_DIR)/variables/vector-reserve-static-negative.elf >$(BUILD_DIR)/variables/vector-reserve-static-negative.err 2>&1; then exit 1; fi
	grep -q "Vec.reserve capacity must be non-negative" $(BUILD_DIR)/variables/vector-reserve-static-negative.err
	if $(TARGET) tests/cases/vectors/errors/vector-truncate-known-static-negative.ari -o $(BUILD_DIR)/variables/vector-truncate-known-static-negative.elf >$(BUILD_DIR)/variables/vector-truncate-known-static-negative.err 2>&1; then exit 1; fi
	grep -q "Vec.truncate length must be non-negative" $(BUILD_DIR)/variables/vector-truncate-known-static-negative.err
	if $(TARGET) tests/cases/variables/errors/variables-match-binding-immutable.ari -o $(BUILD_DIR)/variables/variables-match-binding-immutable.elf >$(BUILD_DIR)/variables/variables-match-binding-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'inner'" $(BUILD_DIR)/variables/variables-match-binding-immutable.err
	if $(TARGET) tests/cases/variables/errors/variables-let-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-let-pattern-binding.elf >$(BUILD_DIR)/variables/variables-let-pattern-binding.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'inner'" $(BUILD_DIR)/variables/variables-let-pattern-binding.err
	if $(TARGET) tests/cases/variables/errors/variables-tuple-pattern-non-tuple.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern-non-tuple.elf >$(BUILD_DIR)/variables/variables-tuple-pattern-non-tuple.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern requires a tuple value, got i64" $(BUILD_DIR)/variables/variables-tuple-pattern-non-tuple.err
	if $(TARGET) tests/cases/variables/errors/variables-tuple-pattern-arity.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern-arity.elf >$(BUILD_DIR)/variables/variables-tuple-pattern-arity.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern has 3 elements but value has 2" $(BUILD_DIR)/variables/variables-tuple-pattern-arity.err
	if $(TARGET) tests/cases/variables/errors/variables-tuple-pattern-rest-arity.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern-rest-arity.elf >$(BUILD_DIR)/variables/variables-tuple-pattern-rest-arity.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern has 3 non-rest elements but value has 2" $(BUILD_DIR)/variables/variables-tuple-pattern-rest-arity.err
	if $(TARGET) tests/cases/variables/errors/variables-struct-pattern-non-struct.ari -o $(BUILD_DIR)/variables/variables-struct-pattern-non-struct.elf >$(BUILD_DIR)/variables/variables-struct-pattern-non-struct.err 2>&1; then exit 1; fi
	grep -q "struct binding pattern requires a struct value, got i64" $(BUILD_DIR)/variables/variables-struct-pattern-non-struct.err
	if $(TARGET) tests/cases/variables/errors/variables-struct-pattern-field.ari -o $(BUILD_DIR)/variables/variables-struct-pattern-field.elf >$(BUILD_DIR)/variables/variables-struct-pattern-field.err 2>&1; then exit 1; fi
	grep -q "struct 'Point' has no field 'y'" $(BUILD_DIR)/variables/variables-struct-pattern-field.err
check-ffi: $(TARGET) check-layout-unit
	mkdir -p $(BUILD_DIR)/ffi
	$(TARGET) tests/cases/ffi/ok/ffi-c-puts.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-puts.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ffi/ffi-c-puts.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-module.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-module.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ffi/ffi-c-module.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-alias.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-alias.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ffi/ffi-c-alias.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-libc-types.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-libc-types.ll
	grep -q 'declare i32 @"abs"(i32)' $(BUILD_DIR)/ffi/ffi-c-libc-types.ll
	grep -Eq 'declare i64 @"?strlen"?\(ptr\)' $(BUILD_DIR)/ffi/ffi-c-libc-types.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-types.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-types.ll
	grep -q 'declare i64 @"ari_test_add_i64"(i64, i64)' $(BUILD_DIR)/ffi/ffi-c-types.ll
	grep -q 'declare i64 @"ari_test_strlen"(ptr)' $(BUILD_DIR)/ffi/ffi-c-types.ll
	grep -q 'declare void @"ari_test_inc_i64"(ptr)' $(BUILD_DIR)/ffi/ffi-c-types.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-aggregate-import.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-aggregate-import.ll
	grep -q 'declare { i64, i64 } @"ari_test_make_pair"(i64)' $(BUILD_DIR)/ffi/ffi-c-aggregate-import.ll
	grep -q 'declare i64 @"ari_test_sum_pair"({ i64, i64 })' $(BUILD_DIR)/ffi/ffi-c-aggregate-import.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-unsigned-aliases.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i1' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i8' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i16' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i32' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i64' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	$(TARGET) --target i686-unknown-linux-gnu tests/cases/ffi/ok/ffi-c-target-ilp32.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-target-ilp32.ll
	grep -q 'target triple = "i686-unknown-linux-gnu"' $(BUILD_DIR)/ffi/ffi-c-target-ilp32.ll
	grep -q 'alloca i32' $(BUILD_DIR)/ffi/ffi-c-target-ilp32.ll
	$(TARGET) --target x86_64-pc-linux-gnu tests/cases/ffi/ok/ffi-c-target-lp64.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-target-lp64.ll
	grep -q 'target triple = "x86_64-pc-linux-gnu"' $(BUILD_DIR)/ffi/ffi-c-target-lp64.ll
	grep -q 'alloca i64' $(BUILD_DIR)/ffi/ffi-c-target-lp64.ll
	$(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/ok/ffi-c-target-llp64.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	grep -q 'target triple = "x86_64-pc-windows-msvc"' $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	grep -q 'alloca i32' $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	grep -q 'alloca i64' $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-null-pointer.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-null-pointer.ll
	grep -q 'declare i64 @"ari_test_is_null"(ptr)' $(BUILD_DIR)/ffi/ffi-c-null-pointer.ll
	grep -q 'store ptr null' $(BUILD_DIR)/ffi/ffi-c-null-pointer.ll
	$(TARGET) tests/cases/memory/ok/nullable-type-pointer.ari --emit-llvm $(BUILD_DIR)/ffi/nullable-type-pointer.ll
	grep -q 'load i64, ptr' $(BUILD_DIR)/ffi/nullable-type-pointer.ll
	grep -q 'store ptr null' $(BUILD_DIR)/ffi/nullable-type-pointer.ll
	$(TARGET) tests/cases/memory/ok/nullable-type-pointer.ari -o $(BUILD_DIR)/ffi/nullable-type-pointer.elf
	$(BUILD_DIR)/ffi/nullable-type-pointer.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/memory/ok/pointer-casts.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-casts.ll
	grep -q 'ptrtoint ptr' $(BUILD_DIR)/ffi/pointer-casts.ll
	grep -q 'inttoptr i64' $(BUILD_DIR)/ffi/pointer-casts.ll
	$(TARGET) tests/cases/memory/ok/pointer-offset.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-offset.ll
	grep -q 'getelementptr i8, ptr' $(BUILD_DIR)/ffi/pointer-offset.ll
	grep -q 'ptrtoint ptr' $(BUILD_DIR)/ffi/pointer-offset.ll
	$(TARGET) tests/cases/memory/ok/pointer-offset.ari -o $(BUILD_DIR)/ffi/pointer-offset.elf
	$(BUILD_DIR)/ffi/pointer-offset.elf; code=$$?; test $$code -eq 8
	$(TARGET) tests/cases/memory/ok/layout-query.ari --emit-llvm $(BUILD_DIR)/ffi/layout-query.ll
	grep -q 'getelementptr i8, ptr' $(BUILD_DIR)/ffi/layout-query.ll
	$(TARGET) tests/cases/memory/ok/layout-query.ari -o $(BUILD_DIR)/ffi/layout-query.elf
	$(BUILD_DIR)/ffi/layout-query.elf; code=$$?; test $$code -eq 35
	$(TARGET) tests/cases/memory/ok/layout-service-aggregate-consistency.ari --emit-llvm $(BUILD_DIR)/ffi/layout-service-aggregate-consistency.ll
	grep -q 'getelementptr inbounds { i8, i64, i16 }' $(BUILD_DIR)/ffi/layout-service-aggregate-consistency.ll
	grep -q 'extractvalue { i32, i64, i64 }' $(BUILD_DIR)/ffi/layout-service-aggregate-consistency.ll
	$(TARGET) tests/cases/memory/ok/layout-service-aggregate-consistency.ari -o $(BUILD_DIR)/ffi/layout-service-aggregate-consistency.elf
	$(BUILD_DIR)/ffi/layout-service-aggregate-consistency.elf; code=$$?; test $$code -eq 152
	$(TARGET) tests/cases/memory/ok/zone-allocation.ari --emit-llvm $(BUILD_DIR)/ffi/zone-allocation.ll
	grep -q '@ari_builtin_zone_create' $(BUILD_DIR)/ffi/zone-allocation.ll
	grep -q 'declare ptr @malloc' $(BUILD_DIR)/ffi/zone-allocation.ll
	grep -q 'declare void @free' $(BUILD_DIR)/ffi/zone-allocation.ll
	$(TARGET) tests/cases/memory/ok/zone-header-metadata.ari --emit-llvm $(BUILD_DIR)/ffi/zone-header-metadata.ll
	grep -q '@ari_builtin_zone_allocation_zone' $(BUILD_DIR)/ffi/zone-header-metadata.ll
	grep -q 'getelementptr i8, ptr %data, i64 -8' $(BUILD_DIR)/ffi/zone-header-metadata.ll
	grep -q 'call ptr @"ari_builtin_zone_allocation_zone"' $(BUILD_DIR)/ffi/zone-header-metadata.ll
	! grep -q 'ari_builtin_zone_allocation_size' $(BUILD_DIR)/ffi/zone-header-metadata.ll
	! grep -q 'ari_builtin_zone_allocation_align' $(BUILD_DIR)/ffi/zone-header-metadata.ll
	$(TARGET) tests/cases/memory/ok/zone-typed-allocation.ari --emit-llvm $(BUILD_DIR)/ffi/zone-typed-allocation.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-typed-allocation.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/zone-typed-allocation.ll
	$(TARGET) tests/cases/memory/ok/zone-placement-new.ari --emit-llvm $(BUILD_DIR)/ffi/zone-placement-new.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-placement-new.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/zone-placement-new.ll
	$(TARGET) tests/cases/memory/ok/zone-associated-new.ari --emit-llvm $(BUILD_DIR)/ffi/zone-associated-new.ll
	grep -q "_ARNv4impl5Point8inherent3new" $(BUILD_DIR)/ffi/zone-associated-new.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-associated-new.ll
	$(TARGET) tests/cases/memory/ok/zone-temp-function-local.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-function-local.ll
	grep -q 'call ptr @"ari_builtin_zone_create"' $(BUILD_DIR)/ffi/zone-temp-function-local.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-function-local.ll
	$(TARGET) tests/cases/memory/ok/zone-temp-nested.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-nested.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-nested.ll
	$(TARGET) tests/cases/memory/ok/zone-temp-control-flow.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-control-flow.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-control-flow.ll
	$(TARGET) tests/cases/memory/ok/zone-temp-init-while-continue.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-init-while-continue.ll
	grep -q '%_continue' $(BUILD_DIR)/ffi/zone-temp-init-while-continue.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-init-while-continue.ll
	$(TARGET) tests/cases/memory/ok/zone-scratch-local.ari --emit-llvm $(BUILD_DIR)/ffi/zone-scratch-local.ll
	grep -q 'call ptr @"ari_builtin_zone_create"' $(BUILD_DIR)/ffi/zone-scratch-local.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-scratch-local.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-scratch-local.ll
	$(TARGET) tests/cases/memory/ok/zone-promote-return.ari --emit-llvm $(BUILD_DIR)/ffi/zone-promote-return.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-promote-return.ll
	grep -q 'load { i64, i64 }, ptr' $(BUILD_DIR)/ffi/zone-promote-return.ll
	$(TARGET) tests/cases/memory/ok/zone-current-trait-calls.ari --emit-llvm $(BUILD_DIR)/ffi/zone-current-trait-calls.ll
	grep -q "Render" $(BUILD_DIR)/ffi/zone-current-trait-calls.ll
	$(TARGET) tests/cases/memory/ok/zone-current-callable.ari --emit-llvm $(BUILD_DIR)/ffi/zone-current-callable.ll
	grep -q "render" $(BUILD_DIR)/ffi/zone-current-callable.ll
	$(TARGET) tests/cases/memory/ok/zone-current-capacity.ari --emit-llvm $(BUILD_DIR)/ffi/zone-current-capacity.ll
	grep -q "fail.bad.capacity" $(BUILD_DIR)/ffi/zone-current-capacity.ll
	grep -q "fail.capacity" $(BUILD_DIR)/ffi/zone-current-capacity.ll
	$(TARGET) tests/cases/memory/ok/pointer-add.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-add.ll
	grep -q 'getelementptr i64, ptr' $(BUILD_DIR)/ffi/pointer-add.ll
	grep -q 'getelementptr i8, ptr' $(BUILD_DIR)/ffi/pointer-add.ll
	$(TARGET) tests/cases/memory/ok/pointer-add.ari -o $(BUILD_DIR)/ffi/pointer-add.elf
	$(BUILD_DIR)/ffi/pointer-add.elf; code=$$?; test $$code -eq 19
	$(TARGET) tests/cases/memory/ok/pointer-add-aggregate.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-add-aggregate.ll
	grep -q 'getelementptr { i64, i64 }, ptr' $(BUILD_DIR)/ffi/pointer-add-aggregate.ll
	grep -q 'getelementptr \[3 x i64\], ptr' $(BUILD_DIR)/ffi/pointer-add-aggregate.ll
	$(TARGET) tests/cases/memory/ok/pointer-add-aggregate.ari -o $(BUILD_DIR)/ffi/pointer-add-aggregate.elf
	$(BUILD_DIR)/ffi/pointer-add-aggregate.elf; code=$$?; test $$code -eq 56
	$(TARGET) tests/cases/memory/ok/pointer-explicit-type-args.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-explicit-type-args.ll
	grep -q 'getelementptr i64, ptr' $(BUILD_DIR)/ffi/pointer-explicit-type-args.ll
	grep -q 'load { i64, i64 }, ptr' $(BUILD_DIR)/ffi/pointer-explicit-type-args.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/pointer-explicit-type-args.ll
	$(TARGET) tests/cases/memory/ok/pointer-explicit-type-args.ari -o $(BUILD_DIR)/ffi/pointer-explicit-type-args.elf
	$(BUILD_DIR)/ffi/pointer-explicit-type-args.elf; code=$$?; test $$code -eq 41
	$(TARGET) tests/cases/memory/ok/pointer-load-store.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-load-store.ll
	grep -q 'load i64, ptr' $(BUILD_DIR)/ffi/pointer-load-store.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/pointer-load-store.ll
	$(TARGET) tests/cases/memory/ok/pointer-load-store.ari -o $(BUILD_DIR)/ffi/pointer-load-store.elf
	$(BUILD_DIR)/ffi/pointer-load-store.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/ir/ok/ir-operand-helper-paths.ari --emit-llvm $(BUILD_DIR)/ffi/ir-operand-helper-paths.ll
	grep -q 'call i64 %' $(BUILD_DIR)/ffi/ir-operand-helper-paths.ll
	grep -q 'getelementptr i64, ptr' $(BUILD_DIR)/ffi/ir-operand-helper-paths.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/ir-operand-helper-paths.ll
	$(TARGET) tests/cases/ir/ok/ir-operand-helper-paths.ari -o $(BUILD_DIR)/ffi/ir-operand-helper-paths.elf
	$(BUILD_DIR)/ffi/ir-operand-helper-paths.elf; code=$$?; test $$code -eq 16
	$(TARGET) tests/cases/memory/ok/pointer-float-load-store.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'load float, ptr' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'store float' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'load double, ptr' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'store double' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	$(TARGET) tests/cases/memory/ok/pointer-float-load-store.ari -o $(BUILD_DIR)/ffi/pointer-float-load-store.elf
	$(BUILD_DIR)/ffi/pointer-float-load-store.elf; code=$$?; test $$code -eq 2
	$(TARGET) tests/cases/memory/ok/pointer-deref.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-deref.ll
	grep -q 'load i64, ptr' $(BUILD_DIR)/ffi/pointer-deref.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/pointer-deref.ll
	grep -q 'store i8' $(BUILD_DIR)/ffi/pointer-deref.ll
	$(TARGET) tests/cases/memory/ok/pointer-deref.ari -o $(BUILD_DIR)/ffi/pointer-deref.elf
	$(BUILD_DIR)/ffi/pointer-deref.elf; code=$$?; test $$code -eq 15
	$(TARGET) tests/cases/memory/ok/pointer-aggregate-access.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	grep -q 'getelementptr inbounds { i64, i64 }' $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	grep -q 'getelementptr inbounds \[3 x i64\]' $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	$(TARGET) tests/cases/memory/ok/pointer-aggregate-access.ari -o $(BUILD_DIR)/ffi/pointer-aggregate-access.elf
	$(BUILD_DIR)/ffi/pointer-aggregate-access.elf; code=$$?; test $$code -eq 45
	$(TARGET) tests/cases/ir/ok/llvm-aggregate-byte-layout.ari -o $(BUILD_DIR)/ffi/llvm-aggregate-byte-layout.elf
	$(BUILD_DIR)/ffi/llvm-aggregate-byte-layout.elf; code=$$?; test $$code -eq 45
	$(TARGET) tests/cases/memory/ok/pointer-aggregate-load-store.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	grep -q 'load { i64, i64 }, ptr' $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	grep -q 'store \[2 x i64\]' $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	$(TARGET) tests/cases/memory/ok/pointer-aggregate-load-store.ari -o $(BUILD_DIR)/ffi/pointer-aggregate-load-store.elf
	$(BUILD_DIR)/ffi/pointer-aggregate-load-store.elf; code=$$?; test $$code -eq 48
	$(TARGET) tests/cases/ffi/ok/ffi-c-varargs.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-varargs.ll
	grep -q 'declare i32 @printf(ptr, ...)' $(BUILD_DIR)/ffi/ffi-c-varargs.ll
	grep -q 'call i32 @"printf"(ptr' $(BUILD_DIR)/ffi/ffi-c-varargs.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-varargs-promotions.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'sext i8' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'zext i16' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'zext i1' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'fpext float' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	$(TARGET) tests/cases/ffi/ok/ffi-c-callback.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-callback.ll
	grep -q 'declare i64 @"ari_test_apply_i64"(ptr, i64)' $(BUILD_DIR)/ffi/ffi-c-callback.ll
	grep -q 'call i64 @"ari_test_apply_i64"(ptr @"_ARNv7add_ten", i64 5)' $(BUILD_DIR)/ffi/ffi-c-callback.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(LLVM_CC) -c tests/ffi/c_helpers.c -o $(BUILD_DIR)/ffi/c_helpers.o; ar rcs $(BUILD_DIR)/ffi/libari_testffi.a $(BUILD_DIR)/ffi/c_helpers.o; $(TARGET) tests/cases/ffi/ok/ffi-c-types.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-types.elf; $(BUILD_DIR)/ffi/ffi-c-types.elf; code=$$?; test $$code -eq 36; $(TARGET) tests/cases/ffi/ok/ffi-c-aggregate-import.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-aggregate-import.elf; $(BUILD_DIR)/ffi/ffi-c-aggregate-import.elf; code=$$?; test $$code -eq 36; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/ffi-c-null-pointer.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-null-pointer.elf; $(BUILD_DIR)/ffi/ffi-c-null-pointer.elf; code=$$?; test $$code -eq 3; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/ffi-c-borrow-return.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-borrow-return.elf; $(BUILD_DIR)/ffi/ffi-c-borrow-return.elf; code=$$?; test $$code -eq 33; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-casts.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/pointer-casts.elf; $(BUILD_DIR)/ffi/pointer-casts.elf; code=$$?; test $$code -eq 1; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-offset.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-offset.elf; $(BUILD_DIR)/ffi/pointer-offset.elf; code=$$?; test $$code -eq 8; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/layout-query.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/layout-query.elf; $(BUILD_DIR)/ffi/layout-query.elf; code=$$?; test $$code -eq 35; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/layout-service-aggregate-consistency.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/layout-service-aggregate-consistency.elf; $(BUILD_DIR)/ffi/layout-service-aggregate-consistency.elf; code=$$?; test $$code -eq 152; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-allocation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-allocation.elf; $(BUILD_DIR)/ffi/zone-allocation.elf; code=$$?; test $$code -eq 54; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-header-metadata.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-header-metadata.elf; $(BUILD_DIR)/ffi/zone-header-metadata.elf; code=$$?; test $$code -eq 41; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-typed-allocation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-typed-allocation.elf; $(BUILD_DIR)/ffi/zone-typed-allocation.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-placement-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-placement-new.elf; $(BUILD_DIR)/ffi/zone-placement-new.elf; code=$$?; test $$code -eq 46; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-associated-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-associated-new.elf; $(BUILD_DIR)/ffi/zone-associated-new.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-temp-function-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-function-local.elf; $(BUILD_DIR)/ffi/zone-temp-function-local.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-temp-nested.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-nested.elf; $(BUILD_DIR)/ffi/zone-temp-nested.elf; code=$$?; test $$code -eq 22; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-temp-control-flow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-control-flow.elf; $(BUILD_DIR)/ffi/zone-temp-control-flow.elf; code=$$?; test $$code -eq 37; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-temp-init-while-continue.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-init-while-continue.elf; $(BUILD_DIR)/ffi/zone-temp-init-while-continue.elf; code=$$?; test $$code -eq 6; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-scratch-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-scratch-local.elf; $(BUILD_DIR)/ffi/zone-scratch-local.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-promote-return.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-promote-return.elf; $(BUILD_DIR)/ffi/zone-promote-return.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-current-trait-calls.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-current-trait-calls.elf; $(BUILD_DIR)/ffi/zone-current-trait-calls.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-current-callable.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-current-callable.elf; $(BUILD_DIR)/ffi/zone-current-callable.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/zone-current-capacity.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-current-capacity.elf; $(BUILD_DIR)/ffi/zone-current-capacity.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-add.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-add.elf; $(BUILD_DIR)/ffi/pointer-add.elf; code=$$?; test $$code -eq 19; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-add-aggregate.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-add-aggregate.elf; $(BUILD_DIR)/ffi/pointer-add-aggregate.elf; code=$$?; test $$code -eq 56; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-load-store.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-load-store.elf; $(BUILD_DIR)/ffi/pointer-load-store.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-deref.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-deref.elf; $(BUILD_DIR)/ffi/pointer-deref.elf; code=$$?; test $$code -eq 15; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-aggregate-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-aggregate-access.elf; $(BUILD_DIR)/ffi/pointer-aggregate-access.elf; code=$$?; test $$code -eq 45; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/memory/ok/pointer-aggregate-load-store.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-aggregate-load-store.elf; $(BUILD_DIR)/ffi/pointer-aggregate-load-store.elf; code=$$?; test $$code -eq 48; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/ffi-c-libc-types.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/ffi-c-libc-types.elf; $(BUILD_DIR)/ffi/ffi-c-libc-types.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/ffi-c-varargs.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/ffi-c-varargs.elf; $(BUILD_DIR)/ffi/ffi-c-varargs.elf >$(BUILD_DIR)/ffi/ffi-c-varargs.out; code=$$?; test $$code -eq 0; grep -q "ari varargs: 7 ok" $(BUILD_DIR)/ffi/ffi-c-varargs.out; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/ffi-c-varargs-promotions.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.elf; $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.elf >$(BUILD_DIR)/ffi/ffi-c-varargs-promotions.out; code=$$?; test $$code -eq 0; grep -q "ari promotions: 7 9 1 1.5" $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.out; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/cases/ffi/ok/ffi-c-callback.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-callback.elf; $(BUILD_DIR)/ffi/ffi-c-callback.elf; code=$$?; test $$code -eq 15; fi
	if $(TARGET) tests/cases/ffi/errors/extern-void-param.ari -o $(BUILD_DIR)/ffi/extern-void-param.elf >$(BUILD_DIR)/ffi/extern-void-param.err 2>&1; then exit 1; fi
	grep -q "extern parameter cannot have void type; use ptr c_void for void\\*" $(BUILD_DIR)/ffi/extern-void-param.err
	if $(TARGET) tests/cases/vectors/errors/vector-extern-param-runtime-abi.ari -o $(BUILD_DIR)/ffi/vector-extern-param-runtime-abi.elf >$(BUILD_DIR)/ffi/vector-extern-param-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as an extern function parameter; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/ffi/vector-extern-param-runtime-abi.err
	if $(TARGET) tests/cases/vectors/errors/vector-extern-return-runtime-abi.ari -o $(BUILD_DIR)/ffi/vector-extern-return-runtime-abi.elf >$(BUILD_DIR)/ffi/vector-extern-return-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as an extern function return type; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/ffi/vector-extern-return-runtime-abi.err
	if $(TARGET) tests/cases/ffi/errors/extern-c-large-aggregate-import.ari --emit-llvm $(BUILD_DIR)/ffi/extern-c-large-aggregate-import.ll >$(BUILD_DIR)/ffi/extern-c-large-aggregate-import.err 2>&1; then exit 1; fi
	grep -q "extern C import for by-value parameter Wide is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment" $(BUILD_DIR)/ffi/extern-c-large-aggregate-import.err
	if $(TARGET) --target x86_64-pc-windows-msvc tests/cases/ffi/errors/extern-c-target-aggregate-import.ari --emit-llvm $(BUILD_DIR)/ffi/extern-c-target-aggregate-import.ll >$(BUILD_DIR)/ffi/extern-c-target-aggregate-import.err 2>&1; then exit 1; fi
	grep -q "extern C import for by-value parameter Pair is currently supported only on 64-bit Unix targets" $(BUILD_DIR)/ffi/extern-c-target-aggregate-import.err
	if $(TARGET) tests/cases/ffi/errors/extern-c-nonrepr-aggregate-import.ari --emit-llvm $(BUILD_DIR)/ffi/extern-c-nonrepr-aggregate-import.ll >$(BUILD_DIR)/ffi/extern-c-nonrepr-aggregate-import.err 2>&1; then exit 1; fi
	grep -q "extern C import for by-value aggregate parameter Pair requires a @repr(C) struct" $(BUILD_DIR)/ffi/extern-c-nonrepr-aggregate-import.err
	if $(TARGET) tests/cases/ffi/errors/extern-varargs-nonextern.ari -o $(BUILD_DIR)/ffi/extern-varargs-nonextern.elf >$(BUILD_DIR)/ffi/extern-varargs-nonextern.err 2>&1; then exit 1; fi
	grep -q "variadic parameters are only supported on extern \"C\" functions" $(BUILD_DIR)/ffi/extern-varargs-nonextern.err
	if $(TARGET) tests/cases/ffi/errors/extern-varargs-empty.ari -o $(BUILD_DIR)/ffi/extern-varargs-empty.elf >$(BUILD_DIR)/ffi/extern-varargs-empty.err 2>&1; then exit 1; fi
	grep -q "C variadic extern functions require at least one fixed parameter" $(BUILD_DIR)/ffi/extern-varargs-empty.err
	if $(TARGET) tests/cases/ffi/errors/extern-varargs-aggregate.ari -o $(BUILD_DIR)/ffi/extern-varargs-aggregate.elf >$(BUILD_DIR)/ffi/extern-varargs-aggregate.err 2>&1; then exit 1; fi
	grep -q "C variadic argument type is not supported: (i64, i64)" $(BUILD_DIR)/ffi/extern-varargs-aggregate.err
	if $(TARGET) tests/cases/ffi/errors/extern-varargs-function-pointer.ari -o $(BUILD_DIR)/ffi/extern-varargs-function-pointer.elf >$(BUILD_DIR)/ffi/extern-varargs-function-pointer.err 2>&1; then exit 1; fi
	grep -q "variadic extern function 'printf' cannot be used as a function pointer value" $(BUILD_DIR)/ffi/extern-varargs-function-pointer.err
	if $(TARGET) tests/cases/ffi/errors/extern-generic.ari -o $(BUILD_DIR)/ffi/extern-generic.elf >$(BUILD_DIR)/ffi/extern-generic.err 2>&1; then exit 1; fi
	grep -q "extern C declarations cannot be generic; declare concrete C symbols or wrap generic foreign APIs in C" $(BUILD_DIR)/ffi/extern-generic.err
	if $(TARGET) tests/cases/ffi/errors/ffi-c-borrow-return-missing-contract.ari -o $(BUILD_DIR)/ffi/ffi-c-borrow-return-missing-contract.elf >$(BUILD_DIR)/ffi/ffi-c-borrow-return-missing-contract.err 2>&1; then exit 1; fi
	grep -q "extern borrow-returning function 'ari_test_identity_i64_ref' needs an explicit @borrow_return(source) contract" $(BUILD_DIR)/ffi/ffi-c-borrow-return-missing-contract.err
	if $(TARGET) tests/cases/memory/errors/null-non-pointer.ari -o $(BUILD_DIR)/ffi/null-non-pointer.elf >$(BUILD_DIR)/ffi/null-non-pointer.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got ptr void" $(BUILD_DIR)/ffi/null-non-pointer.err
	if $(TARGET) tests/cases/memory/errors/nullable-qualified.ari -o $(BUILD_DIR)/ffi/nullable-qualified.elf >$(BUILD_DIR)/ffi/nullable-qualified.err 2>&1; then exit 1; fi
	grep -q "nullable type suffix ? cannot be combined with own, ref, or ptr qualifiers" $(BUILD_DIR)/ffi/nullable-qualified.err
	if $(TARGET) tests/cases/memory/errors/nullable-value-initializer.ari -o $(BUILD_DIR)/ffi/nullable-value-initializer.elf >$(BUILD_DIR)/ffi/nullable-value-initializer.err 2>&1; then exit 1; fi
	grep -q "nullable type 'i64?' is a raw pointer spelling; initialize it with null, a raw pointer, or a compatible string pointer value" $(BUILD_DIR)/ffi/nullable-value-initializer.err
	if $(TARGET) tests/cases/memory/errors/pointer-deref-non-pointer.ari -o $(BUILD_DIR)/ffi/pointer-deref-non-pointer.elf >$(BUILD_DIR)/ffi/pointer-deref-non-pointer.err 2>&1; then exit 1; fi
	grep -q "pointer dereference expects a raw pointer, got i64" $(BUILD_DIR)/ffi/pointer-deref-non-pointer.err
	if $(TARGET) tests/cases/memory/errors/layout-query-type-args.ari -o $(BUILD_DIR)/ffi/layout-query-type-args.elf >$(BUILD_DIR)/ffi/layout-query-type-args.err 2>&1; then exit 1; fi
	grep -q "size_of expects exactly one type argument" $(BUILD_DIR)/ffi/layout-query-type-args.err
	if $(TARGET) tests/cases/memory/errors/zone-destroy-required.ari -o $(BUILD_DIR)/ffi/zone-destroy-required.elf >$(BUILD_DIR)/ffi/zone-destroy-required.err 2>&1; then exit 1; fi
	grep -q "owning binding 'zone' must be moved or dropped before return" $(BUILD_DIR)/ffi/zone-destroy-required.err
	if $(TARGET) tests/cases/memory/errors/zone-drop.ari -o $(BUILD_DIR)/ffi/zone-drop.elf >$(BUILD_DIR)/ffi/zone-drop.err 2>&1; then exit 1; fi
	grep -q "use zone::destroy(zone) to release a Zone" $(BUILD_DIR)/ffi/zone-drop.err
	if $(TARGET) tests/cases/memory/errors/zone-temp-move.ari -o $(BUILD_DIR)/ffi/zone-temp-move.elf >$(BUILD_DIR)/ffi/zone-temp-move.err 2>&1; then exit 1; fi
	grep -q "temporary zone 'scratch' cannot be moved" $(BUILD_DIR)/ffi/zone-temp-move.err
	if $(TARGET) tests/cases/memory/errors/zone-temp-return-escape.ari -o $(BUILD_DIR)/ffi/zone-temp-return-escape.elf >$(BUILD_DIR)/ffi/zone-temp-return-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'item' from temporary zone 'scratch' cannot escape through function return" $(BUILD_DIR)/ffi/zone-temp-return-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-temp-outer-binding-escape.ari -o $(BUILD_DIR)/ffi/zone-temp-outer-binding-escape.elf >$(BUILD_DIR)/ffi/zone-temp-outer-binding-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'leaked' from temporary zone 'scratch' cannot outlive the temporary zone scope" $(BUILD_DIR)/ffi/zone-temp-outer-binding-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-temp-struct-escape.ari -o $(BUILD_DIR)/ffi/zone-temp-struct-escape.elf >$(BUILD_DIR)/ffi/zone-temp-struct-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'item' from temporary zone 'scratch' cannot escape through struct literal" $(BUILD_DIR)/ffi/zone-temp-struct-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-scratch-return-escape.ari -o $(BUILD_DIR)/ffi/zone-scratch-return-escape.elf >$(BUILD_DIR)/ffi/zone-scratch-return-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'item' from temporary zone" $(BUILD_DIR)/ffi/zone-scratch-return-escape.err
	grep -q "cannot escape through function return" $(BUILD_DIR)/ffi/zone-scratch-return-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-scratch-outer-binding-escape.ari -o $(BUILD_DIR)/ffi/zone-scratch-outer-binding-escape.elf >$(BUILD_DIR)/ffi/zone-scratch-outer-binding-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'leaked' from temporary zone" $(BUILD_DIR)/ffi/zone-scratch-outer-binding-escape.err
	grep -q "cannot outlive the temporary zone scope" $(BUILD_DIR)/ffi/zone-scratch-outer-binding-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-scratch-expression.ari -o $(BUILD_DIR)/ffi/zone-scratch-expression.elf >$(BUILD_DIR)/ffi/zone-scratch-expression.err 2>&1; then exit 1; fi
	grep -q "zone::scratch<T> can only initialize a local pointer binding" $(BUILD_DIR)/ffi/zone-scratch-expression.err
	if $(TARGET) tests/cases/memory/errors/zone-current-missing-trait-qualified.ari -o $(BUILD_DIR)/ffi/zone-current-missing-trait-qualified.elf >$(BUILD_DIR)/ffi/zone-current-missing-trait-qualified.err 2>&1; then exit 1; fi
	grep -q "call to trait-qualified method 'Render::render' needs an allocation zone" $(BUILD_DIR)/ffi/zone-current-missing-trait-qualified.err
	if $(TARGET) tests/cases/memory/errors/zone-current-missing-trait-object.ari -o $(BUILD_DIR)/ffi/zone-current-missing-trait-object.elf >$(BUILD_DIR)/ffi/zone-current-missing-trait-object.err 2>&1; then exit 1; fi
	grep -q "call to trait object method 'render' needs an allocation zone" $(BUILD_DIR)/ffi/zone-current-missing-trait-object.err
	if $(TARGET) tests/cases/memory/errors/zone-current-missing-callable.ari -o $(BUILD_DIR)/ffi/zone-current-missing-callable.elf >$(BUILD_DIR)/ffi/zone-current-missing-callable.err 2>&1; then exit 1; fi
	grep -q "call to callable value needs an allocation zone" $(BUILD_DIR)/ffi/zone-current-missing-callable.err
	if $(TARGET) tests/cases/memory/errors/zone-promote-source-type.ari -o $(BUILD_DIR)/ffi/zone-promote-source-type.elf >$(BUILD_DIR)/ffi/zone-promote-source-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected ptr i64, got i64" $(BUILD_DIR)/ffi/zone-promote-source-type.err
	if $(TARGET) tests/cases/memory/errors/zone-alloc-type-args.ari -o $(BUILD_DIR)/ffi/zone-alloc-type-args.elf >$(BUILD_DIR)/ffi/zone-alloc-type-args.err 2>&1; then exit 1; fi
	grep -q "zone::alloc<T> expects exactly one type argument" $(BUILD_DIR)/ffi/zone-alloc-type-args.err
	if $(TARGET) tests/cases/memory/errors/zone-alloc-zero-sized.ari -o $(BUILD_DIR)/ffi/zone-alloc-zero-sized.elf >$(BUILD_DIR)/ffi/zone-alloc-zero-sized.err 2>&1; then exit 1; fi
	grep -q "zone::alloc<T> requires a non-zero-sized type" $(BUILD_DIR)/ffi/zone-alloc-zero-sized.err
	if $(TARGET) tests/cases/memory/errors/zone-new-type-args.ari -o $(BUILD_DIR)/ffi/zone-new-type-args.elf >$(BUILD_DIR)/ffi/zone-new-type-args.err 2>&1; then exit 1; fi
	grep -q "zone::new<T> expects exactly one type argument" $(BUILD_DIR)/ffi/zone-new-type-args.err
	if $(TARGET) tests/cases/memory/errors/zone-new-own-aggregate.ari -o $(BUILD_DIR)/ffi/zone-new-own-aggregate.elf >$(BUILD_DIR)/ffi/zone-new-own-aggregate.err 2>&1; then exit 1; fi
	grep -q "zone::new<T> cannot place ownership- or borrow-valued types yet" $(BUILD_DIR)/ffi/zone-new-own-aggregate.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-after-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-reset.err
	if $(TARGET) tests/cases/memory/errors/region-string-after-reset.ari -o $(BUILD_DIR)/ffi/region-string-after-reset.elf >$(BUILD_DIR)/ffi/region-string-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'text' after zone 'region' was reset or destroyed" $(BUILD_DIR)/ffi/region-string-after-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-enum-payload-after-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-enum-payload-after-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-enum-payload-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'maybe' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-enum-payload-after-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-after-borrowed-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-borrowed-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-borrowed-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-borrowed-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-after-destroy.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-destroy.elf >$(BUILD_DIR)/ffi/zone-pointer-after-destroy.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-destroy.err
	if $(TARGET) tests/cases/memory/errors/zone-wrapper-pointer-after-reset.ari -o $(BUILD_DIR)/ffi/zone-wrapper-pointer-after-reset.elf >$(BUILD_DIR)/ffi/zone-wrapper-pointer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-wrapper-pointer-after-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-associated-pointer-after-reset.ari -o $(BUILD_DIR)/ffi/zone-associated-pointer-after-reset.elf >$(BUILD_DIR)/ffi/zone-associated-pointer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'point' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-associated-pointer-after-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-after-if-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-if-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-if-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-if-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-after-match-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-match-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-match-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-match-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-after-loop-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-loop-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-loop-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-loop-reset.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-struct-escape.ari -o $(BUILD_DIR)/ffi/zone-pointer-struct-escape.elf >$(BUILD_DIR)/ffi/zone-pointer-struct-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer cannot escape into struct literal" $(BUILD_DIR)/ffi/zone-pointer-struct-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-ffi-escape.ari -o $(BUILD_DIR)/ffi/zone-pointer-ffi-escape.elf >$(BUILD_DIR)/ffi/zone-pointer-ffi-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer cannot escape into extern C call argument" $(BUILD_DIR)/ffi/zone-pointer-ffi-escape.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-multizone-return.ari -o $(BUILD_DIR)/ffi/zone-pointer-multizone-return.elf >$(BUILD_DIR)/ffi/zone-pointer-multizone-return.err 2>&1; then exit 1; fi
	grep -q "zone pointer returns must come from the function's single zone borrow parameter" $(BUILD_DIR)/ffi/zone-pointer-multizone-return.err
	if $(TARGET) tests/cases/memory/errors/zone-pointer-no-zone-return.ari -o $(BUILD_DIR)/ffi/zone-pointer-no-zone-return.elf >$(BUILD_DIR)/ffi/zone-pointer-no-zone-return.err 2>&1; then exit 1; fi
	grep -q "zone pointer returns must come from the function's single zone borrow parameter" $(BUILD_DIR)/ffi/zone-pointer-no-zone-return.err
	if $(TARGET) tests/cases/memory/errors/pointer-deref-aggregate.ari -o $(BUILD_DIR)/ffi/pointer-deref-aggregate.elf >$(BUILD_DIR)/ffi/pointer-deref-aggregate.err 2>&1; then exit 1; fi
	grep -q "pointer dereference cannot copy ownership- or borrow-valued values through raw pointers yet" $(BUILD_DIR)/ffi/pointer-deref-aggregate.err
check-functions: $(TARGET) check-function-errors
	mkdir -p $(BUILD_DIR)/functions
	$(TARGET) tests/cases/functions/ok/function-recursion.ari -o $(BUILD_DIR)/functions/function-recursion.elf
	$(BUILD_DIR)/functions/function-recursion.elf; code=$$?; test $$code -eq 120
	readelf -sW $(BUILD_DIR)/functions/function-recursion.elf > $(BUILD_DIR)/functions/function-recursion.sym
	grep -q "_ARNv4fact" $(BUILD_DIR)/functions/function-recursion.sym
	grep -q "_ARNv4main" $(BUILD_DIR)/functions/function-recursion.sym
	if grep -q "__P" $(BUILD_DIR)/functions/function-recursion.sym; then exit 1; fi
	if grep -q "__R" $(BUILD_DIR)/functions/function-recursion.sym; then exit 1; fi
	$(TARGET) tests/cases/functions/ok/function-recursion.ari --emit-llvm $(BUILD_DIR)/functions/function-recursion.ll
	grep -q "_ARNv4fact" $(BUILD_DIR)/functions/function-recursion.ll
	grep -q "_ARNv4main" $(BUILD_DIR)/functions/function-recursion.ll
	if grep -q "__P" $(BUILD_DIR)/functions/function-recursion.ll; then exit 1; fi
	if grep -q "__R" $(BUILD_DIR)/functions/function-recursion.ll; then exit 1; fi
	$(TARGET) tests/cases/functions/ok/function-recursion-llvm.ari -o $(BUILD_DIR)/functions/function-recursion-llvm.elf
	$(BUILD_DIR)/functions/function-recursion-llvm.elf; code=$$?; test $$code -eq 120
	readelf -sW $(BUILD_DIR)/functions/function-recursion-llvm.elf > $(BUILD_DIR)/functions/function-recursion-llvm.sym
	grep -q "_ARNv4fact" $(BUILD_DIR)/functions/function-recursion-llvm.sym
	grep -q "_ARNv4main" $(BUILD_DIR)/functions/function-recursion-llvm.sym
	$(TARGET) tests/cases/functions/ok/function-void-return.ari -o $(BUILD_DIR)/functions/function-void-return.elf
	$(BUILD_DIR)/functions/function-void-return.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/functions/ok/function-param-patterns.ari --emit-llvm $(BUILD_DIR)/functions/function-param-patterns.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/functions/function-param-patterns.ll
	grep -q "extractvalue \\[4 x i64\\]" $(BUILD_DIR)/functions/function-param-patterns.ll
	grep -q "slice.bounds.fail" $(BUILD_DIR)/functions/function-param-patterns.ll
	grep -q "match.arm" $(BUILD_DIR)/functions/function-param-patterns.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/functions/ok/function-param-patterns.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/functions/function-param-patterns.elf; $(BUILD_DIR)/functions/function-param-patterns.elf; code=$$?; test $$code -eq 87; fi
	$(TARGET) tests/cases/functions/ok/function-param-patterns.ari -o $(BUILD_DIR)/functions/function-param-patterns.elf
	$(BUILD_DIR)/functions/function-param-patterns.elf; code=$$?; test $$code -eq 87
	$(TARGET) tests/cases/functions/ok/function-param-pattern-ownership.ari -o $(BUILD_DIR)/functions/function-param-pattern-ownership.elf
	$(BUILD_DIR)/functions/function-param-pattern-ownership.elf; code=$$?; test $$code -eq 80
	$(TARGET) tests/cases/functions/ok/function-reference-param-patterns.ari --emit-llvm $(BUILD_DIR)/functions/function-reference-param-patterns.ll
	grep -q "getelementptr" $(BUILD_DIR)/functions/function-reference-param-patterns.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/functions/ok/function-reference-param-patterns.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/functions/function-reference-param-patterns.elf; $(BUILD_DIR)/functions/function-reference-param-patterns.elf; code=$$?; test $$code -eq 70; fi
	$(TARGET) tests/cases/functions/ok/function-reference-param-patterns.ari -o $(BUILD_DIR)/functions/function-reference-param-patterns.elf
	$(BUILD_DIR)/functions/function-reference-param-patterns.elf; code=$$?; test $$code -eq 70
	$(TARGET) tests/cases/functions/ok/function-six-args-llvm.ari -o $(BUILD_DIR)/functions/function-six-args-llvm.elf
	$(BUILD_DIR)/functions/function-six-args-llvm.elf; code=$$?; test $$code -eq 21
	$(TARGET) tests/cases/functions/ok/function-eight-args-llvm.ari -o $(BUILD_DIR)/functions/function-eight-args-llvm.elf
	$(BUILD_DIR)/functions/function-eight-args-llvm.elf; code=$$?; test $$code -eq 36
	$(TARGET) tests/cases/functions/ok/function-seven-args-host.ari -o $(BUILD_DIR)/functions/function-seven-args-host.elf
	$(BUILD_DIR)/functions/function-seven-args-host.elf; code=$$?; test $$code -eq 28
	$(TARGET) tests/cases/functions/ok/function-narrow-return-llvm.ari -o $(BUILD_DIR)/functions/function-narrow-return-llvm.elf
	$(BUILD_DIR)/functions/function-narrow-return-llvm.elf; code=$$?; test $$code -eq 23
	$(TARGET) tests/cases/functions/ok/function-float-llvm.ari -o $(BUILD_DIR)/functions/function-float-llvm.elf
	$(BUILD_DIR)/functions/function-float-llvm.elf; code=$$?; test $$code -eq 3
	$(TARGET) tests/cases/functions/ok/function-float-llvm.ari --emit-llvm $(BUILD_DIR)/functions/function-float-llvm.ll
	grep -q 'define double @"_ARNv5widen"' $(BUILD_DIR)/functions/function-float-llvm.ll
	grep -q 'define float @"_ARNv6narrow"' $(BUILD_DIR)/functions/function-float-llvm.ll
	$(TARGET) tests/cases/functions/ok/function-aggregate-return-llvm.ari -o $(BUILD_DIR)/functions/function-aggregate-return-llvm.elf
	$(BUILD_DIR)/functions/function-aggregate-return-llvm.elf; code=$$?; test $$code -eq 69
	$(TARGET) tests/cases/functions/ok/function-aggregate-params-llvm.ari -o $(BUILD_DIR)/functions/function-aggregate-params-llvm.elf
	$(BUILD_DIR)/functions/function-aggregate-params-llvm.elf; code=$$?; test $$code -eq 149
	$(TARGET) tests/cases/functions/ok/function-aggregate-indirect-return-llvm.ari -o $(BUILD_DIR)/functions/function-aggregate-indirect-return-llvm.elf
	$(BUILD_DIR)/functions/function-aggregate-indirect-return-llvm.elf; code=$$?; test $$code -eq 76
	$(TARGET) tests/cases/functions/ok/function-generic-surface.ari -o $(BUILD_DIR)/functions/function-generic-surface.elf
	$(BUILD_DIR)/functions/function-generic-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/functions/ok/function-generic-call.ari -o $(BUILD_DIR)/functions/function-generic-call.elf
	$(BUILD_DIR)/functions/function-generic-call.elf; code=$$?; test $$code -eq 22
	$(TARGET) tests/cases/functions/ok/function-generic-companion.ari -o $(BUILD_DIR)/functions/function-generic-companion.elf
	$(BUILD_DIR)/functions/function-generic-companion.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/vectors/ok/vector-function-param.ari --emit-llvm $(BUILD_DIR)/functions/vector-function-param.ll
	grep -Fq 'define i64 @"_ARNv8edge_sum"({ ptr, i64 } %arg0)' $(BUILD_DIR)/functions/vector-function-param.ll
	! grep -q "edge_sum__V" $(BUILD_DIR)/functions/vector-function-param.ll
	$(TARGET) tests/cases/vectors/ok/vector-function-param.ari -o $(BUILD_DIR)/functions/vector-function-param.elf
	$(BUILD_DIR)/functions/vector-function-param.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/vectors/ok/vector-param-temporary-view.ari --emit-llvm $(BUILD_DIR)/functions/vector-param-temporary-view.ll
	grep -Fq 'define i64 @"_ARNv3sum"({ ptr, i64 } %arg0)' $(BUILD_DIR)/functions/vector-param-temporary-view.ll
	$(TARGET) tests/cases/vectors/ok/vector-param-temporary-view.ari -o $(BUILD_DIR)/functions/vector-param-temporary-view.elf
	$(BUILD_DIR)/functions/vector-param-temporary-view.elf; code=$$?; test $$code -eq 42
	$(TARGET) tests/cases/vectors/ok/vector-function-pointer-param.ari --emit-llvm $(BUILD_DIR)/functions/vector-function-pointer-param.ll
	grep -Fq 'define i64 @"_ARNv7call_it"(ptr %arg0, { ptr, i64 } %arg1)' $(BUILD_DIR)/functions/vector-function-pointer-param.ll
	grep -q 'call i64 %' $(BUILD_DIR)/functions/vector-function-pointer-param.ll
	$(TARGET) tests/cases/vectors/ok/vector-function-pointer-param.ari -o $(BUILD_DIR)/functions/vector-function-pointer-param.elf
	$(BUILD_DIR)/functions/vector-function-pointer-param.elf; code=$$?; test $$code -eq 34
	$(TARGET) tests/cases/functions/ok/function-pointer-call.ari -o $(BUILD_DIR)/functions/function-pointer-call.elf
	$(BUILD_DIR)/functions/function-pointer-call.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/cases/functions/ok/function-pointer-call.ari --emit-llvm $(BUILD_DIR)/functions/function-pointer-call.ll
	grep -q 'call i64 %' $(BUILD_DIR)/functions/function-pointer-call.ll
	$(TARGET) tests/cases/functions/ok/lambda-function-pointer.ari -o $(BUILD_DIR)/functions/lambda-function-pointer.elf
	$(BUILD_DIR)/functions/lambda-function-pointer.elf; code=$$?; test $$code -eq 50
	$(TARGET) tests/cases/functions/ok/lambda-function-pointer.ari --emit-llvm $(BUILD_DIR)/functions/lambda-function-pointer.ll
	grep -q '_u24lambda' $(BUILD_DIR)/functions/lambda-function-pointer.ll
	grep -q 'call i64 %' $(BUILD_DIR)/functions/lambda-function-pointer.ll
	$(TARGET) tests/cases/functions/ok/lambda-capture.ari -o $(BUILD_DIR)/functions/lambda-capture.elf
	$(BUILD_DIR)/functions/lambda-capture.elf; code=$$?; test $$code -eq 64
	$(TARGET) tests/cases/functions/ok/lambda-capture.ari --emit-llvm $(BUILD_DIR)/functions/lambda-capture.ll
	grep -q '_u24lambda' $(BUILD_DIR)/functions/lambda-capture.ll
	grep -q 'extractvalue' $(BUILD_DIR)/functions/lambda-capture.ll
	grep -q 'call i64 %' $(BUILD_DIR)/functions/lambda-capture.ll
	$(TARGET) tests/cases/functions/ok/function-extern-c.ari --emit-llvm $(BUILD_DIR)/functions/function-extern-c.ll
	grep -q 'declare i64 @"puts"(ptr)' $(BUILD_DIR)/functions/function-extern-c.ll
	grep -q 'call i64 @"puts"' $(BUILD_DIR)/functions/function-extern-c.ll
	$(TARGET) tests/cases/functions/ok/function-module-pub.ari -o $(BUILD_DIR)/functions/function-module-pub.elf
	$(BUILD_DIR)/functions/function-module-pub.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/cases/functions/ok/function-module-pub.ari --emit-llvm $(BUILD_DIR)/functions/function-module-pub.ll
	grep -q 'define i64 @"_ARNv4Math5value"' $(BUILD_DIR)/functions/function-module-pub.ll
	grep -q 'define i64 @"_ARNv4Math11call_hidden"' $(BUILD_DIR)/functions/function-module-pub.ll

check-function-errors: $(TARGET)
	mkdir -p $(BUILD_DIR)/functions/errors
	if $(TARGET) tests/cases/functions/errors/function-missing-main.ari -o $(BUILD_DIR)/functions/errors/function-missing-main.elf >$(BUILD_DIR)/functions/errors/function-missing-main.err 2>&1; then exit 1; fi
	grep -q "missing executable function 'main'" $(BUILD_DIR)/functions/errors/function-missing-main.err
	if $(TARGET) tests/cases/functions/errors/function-main-params.ari -o $(BUILD_DIR)/functions/errors/function-main-params.elf >$(BUILD_DIR)/functions/errors/function-main-params.err 2>&1; then exit 1; fi
	grep -q "main cannot take parameters" $(BUILD_DIR)/functions/errors/function-main-params.err
	if $(TARGET) tests/cases/functions/errors/function-main-return-type.ari -o $(BUILD_DIR)/functions/errors/function-main-return-type.elf >$(BUILD_DIR)/functions/errors/function-main-return-type.err 2>&1; then exit 1; fi
	grep -q "main must return i64 in the executable subset" $(BUILD_DIR)/functions/errors/function-main-return-type.err
	if $(TARGET) tests/cases/functions/errors/function-missing-return.ari -o $(BUILD_DIR)/functions/errors/function-missing-return.elf >$(BUILD_DIR)/functions/errors/function-missing-return.err 2>&1; then exit 1; fi
	grep -q "function 'main' must return a value on all paths" $(BUILD_DIR)/functions/errors/function-missing-return.err
	if $(TARGET) tests/cases/functions/errors/function-branch-missing-return.ari -o $(BUILD_DIR)/functions/errors/function-branch-missing-return.elf >$(BUILD_DIR)/functions/errors/function-branch-missing-return.err 2>&1; then exit 1; fi
	grep -q "function 'choose' must return a value on all paths" $(BUILD_DIR)/functions/errors/function-branch-missing-return.err
	if $(TARGET) tests/cases/functions/errors/function-return-type-mismatch.ari -o $(BUILD_DIR)/functions/errors/function-return-type-mismatch.elf >$(BUILD_DIR)/functions/errors/function-return-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/functions/errors/function-return-type-mismatch.err
	if $(TARGET) tests/cases/functions/errors/function-wrong-arg-count.ari -o $(BUILD_DIR)/functions/errors/function-wrong-arg-count.elf >$(BUILD_DIR)/functions/errors/function-wrong-arg-count.err 2>&1; then exit 1; fi
	grep -q "wrong argument count for 'add'" $(BUILD_DIR)/functions/errors/function-wrong-arg-count.err
	if $(TARGET) tests/cases/functions/errors/function-arg-type-mismatch.ari -o $(BUILD_DIR)/functions/errors/function-arg-type-mismatch.elf >$(BUILD_DIR)/functions/errors/function-arg-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/functions/errors/function-arg-type-mismatch.err
	if $(TARGET) tests/cases/vectors/errors/vector-function-return-runtime-abi.ari -o $(BUILD_DIR)/functions/errors/vector-function-return-runtime-abi.elf >$(BUILD_DIR)/functions/errors/vector-function-return-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as a function return type; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/functions/errors/vector-function-return-runtime-abi.err
	if $(TARGET) tests/cases/vectors/errors/vector-function-pointer-return-runtime-abi.ari -o $(BUILD_DIR)/functions/errors/vector-function-pointer-return-runtime-abi.elf >$(BUILD_DIR)/functions/errors/vector-function-pointer-return-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as a function pointer return type; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/functions/errors/vector-function-pointer-return-runtime-abi.err
	if $(TARGET) tests/cases/vectors/errors/vector-generic-function-return-runtime-abi.ari -o $(BUILD_DIR)/functions/errors/vector-generic-function-return-runtime-abi.elf >$(BUILD_DIR)/functions/errors/vector-generic-function-return-runtime-abi.err 2>&1; then exit 1; fi
	grep -Fq "bare root Vec[T] cannot be used as a function return type; it is limited to local storage and Slice-shaped parameter views" $(BUILD_DIR)/functions/errors/vector-generic-function-return-runtime-abi.err
	if $(TARGET) tests/cases/functions/errors/function-param-pattern.ari -o $(BUILD_DIR)/functions/errors/function-param-pattern.elf >$(BUILD_DIR)/functions/errors/function-param-pattern.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern requires a tuple value, got i64" $(BUILD_DIR)/functions/errors/function-param-pattern.err
	if $(TARGET) tests/cases/functions/errors/function-overload-params.ari -o $(BUILD_DIR)/functions/errors/function-overload-params.elf >$(BUILD_DIR)/functions/errors/function-overload-params.err 2>&1; then exit 1; fi
	grep -q "duplicate executable function 'pick'" $(BUILD_DIR)/functions/errors/function-overload-params.err
	if $(TARGET) tests/cases/functions/errors/function-void-return-value.ari -o $(BUILD_DIR)/functions/errors/function-void-return-value.elf >$(BUILD_DIR)/functions/errors/function-void-return-value.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected void, got i64" $(BUILD_DIR)/functions/errors/function-void-return-value.err
	if $(TARGET) tests/cases/functions/errors/function-module-private.ari -o $(BUILD_DIR)/functions/errors/function-module-private.elf >$(BUILD_DIR)/functions/errors/function-module-private.err 2>&1; then exit 1; fi
	grep -q "function 'Secret::hidden' is not public" $(BUILD_DIR)/functions/errors/function-module-private.err
	if $(TARGET) tests/cases/functions/errors/lambda-missing-expected-type.ari -o $(BUILD_DIR)/functions/errors/lambda-missing-expected-type.elf >$(BUILD_DIR)/functions/errors/lambda-missing-expected-type.err 2>&1; then exit 1; fi
	grep -q "lambda parameter 'value' needs an explicit type when no expected function type is available" $(BUILD_DIR)/functions/errors/lambda-missing-expected-type.err
	if $(TARGET) tests/cases/functions/errors/lambda-param-count.ari -o $(BUILD_DIR)/functions/errors/lambda-param-count.elf >$(BUILD_DIR)/functions/errors/lambda-param-count.err 2>&1; then exit 1; fi
	grep -q "lambda parameter count does not match expected fn(i64) -> i64" $(BUILD_DIR)/functions/errors/lambda-param-count.err
	if $(TARGET) tests/cases/functions/errors/lambda-capture-local.ari -o $(BUILD_DIR)/functions/errors/lambda-capture-local.elf >$(BUILD_DIR)/functions/errors/lambda-capture-local.err 2>&1; then exit 1; fi
	grep -q "capturing lambda cannot be used as a plain function pointer" $(BUILD_DIR)/functions/errors/lambda-capture-local.err
	if $(TARGET) tests/cases/functions/errors/lambda-capture-borrow.ari -o $(BUILD_DIR)/functions/errors/lambda-capture-borrow.elf >$(BUILD_DIR)/functions/errors/lambda-capture-borrow.err 2>&1; then exit 1; fi
	grep -q "lambda capture of borrow-carrying binding 'borrowed'" $(BUILD_DIR)/functions/errors/lambda-capture-borrow.err
	if $(TARGET) tests/cases/functions/errors/lambda-capture-raw-pointer.ari -o $(BUILD_DIR)/functions/errors/lambda-capture-raw-pointer.elf >$(BUILD_DIR)/functions/errors/lambda-capture-raw-pointer.err 2>&1; then exit 1; fi
	grep -q "lambda capture of binding 'raw' has unsupported type ptr void" $(BUILD_DIR)/functions/errors/lambda-capture-raw-pointer.err

check-errors: $(TARGET) check-function-errors
	mkdir -p $(BUILD_DIR)/errors
	if $(TARGET) tests/cases/ownership/errors/use-after-move.ari -o $(BUILD_DIR)/errors/use-after-move.elf >$(BUILD_DIR)/errors/use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved binding 'token'" $(BUILD_DIR)/errors/use-after-move.err
	if $(TARGET) tests/cases/ownership/errors/undropped-own.ari -o $(BUILD_DIR)/errors/undropped-own.elf >$(BUILD_DIR)/errors/undropped-own.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before scope exit" $(BUILD_DIR)/errors/undropped-own.err
	if $(TARGET) tests/cases/ownership/errors/return-live-owner.ari -o $(BUILD_DIR)/errors/return-live-owner.elf >$(BUILD_DIR)/errors/return-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before return" $(BUILD_DIR)/errors/return-live-owner.err
	if $(TARGET) tests/cases/ownership/errors/overwrite-own.ari -o $(BUILD_DIR)/errors/overwrite-own.elf >$(BUILD_DIR)/errors/overwrite-own.err 2>&1; then exit 1; fi
	grep -q "cannot overwrite owning binding 'token'" $(BUILD_DIR)/errors/overwrite-own.err
	if $(TARGET) tests/cases/ownership/errors/discard-own-result.ari -o $(BUILD_DIR)/errors/discard-own-result.elf >$(BUILD_DIR)/errors/discard-own-result.err 2>&1; then exit 1; fi
	grep -q "owning expression result must be bound, returned, passed, or dropped" $(BUILD_DIR)/errors/discard-own-result.err
	if $(TARGET) tests/cases/ownership/errors/own-bool-condition.ari -o $(BUILD_DIR)/errors/own-bool-condition.elf >$(BUILD_DIR)/errors/own-bool-condition.err 2>&1; then exit 1; fi
	grep -q "condition must be bool or integer-convertible, got own bool" $(BUILD_DIR)/errors/own-bool-condition.err
	$(TARGET) tests/cases/ownership/ok/drop-destructor-lowering.ari --emit-llvm $(BUILD_DIR)/errors/drop-destructor-lowering.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/errors/drop-destructor-lowering.ll
	$(TARGET) tests/cases/ownership/ok/drop-destructor-lowering.ari -o $(BUILD_DIR)/errors/drop-destructor-lowering.elf
	$(BUILD_DIR)/errors/drop-destructor-lowering.elf; code=$$?; test $$code -eq 0
	if $(TARGET) tests/cases/control-flow/errors/if-drop-one-branch.ari -o $(BUILD_DIR)/errors/if-drop-one-branch.elf >$(BUILD_DIR)/errors/if-drop-one-branch.err 2>&1; then exit 1; fi
	grep -q "changes ownership state in if without else" $(BUILD_DIR)/errors/if-drop-one-branch.err
	if $(TARGET) tests/cases/control-flow/errors/if-branch-state-mismatch.ari -o $(BUILD_DIR)/errors/if-branch-state-mismatch.elf >$(BUILD_DIR)/errors/if-branch-state-mismatch.err 2>&1; then exit 1; fi
	grep -q "has incompatible ownership states after if branches" $(BUILD_DIR)/errors/if-branch-state-mismatch.err
	if $(TARGET) tests/cases/control-flow/errors/loop-drop-owner.ari -o $(BUILD_DIR)/errors/loop-drop-owner.elf >$(BUILD_DIR)/errors/loop-drop-owner.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/loop-drop-owner.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-owner-state.ari -o $(BUILD_DIR)/errors/loop-break-owner-state.elf >$(BUILD_DIR)/errors/loop-break-owner-state.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/loop-break-owner-state.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-owner-maybe-unavailable-use.ari -o $(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-use.elf >$(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-use.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' may be unavailable before drop" $(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-use.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-owner-maybe-unavailable-return.ari -o $(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-return.elf >$(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-return.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' may be unavailable before return" $(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-return.err
	if $(TARGET) tests/cases/ownership/errors/drop-maybe-unavailable-owner.ari -o $(BUILD_DIR)/errors/drop-maybe-unavailable-owner.elf >$(BUILD_DIR)/errors/drop-maybe-unavailable-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' may be unavailable before drop" $(BUILD_DIR)/errors/drop-maybe-unavailable-owner.err
	if $(TARGET) tests/cases/ownership/errors/forget-non-owner.ari -o $(BUILD_DIR)/errors/forget-non-owner.elf >$(BUILD_DIR)/errors/forget-non-owner.err 2>&1; then exit 1; fi
	grep -q "forget requires a live or maybe-unavailable owning binding 'value'" $(BUILD_DIR)/errors/forget-non-owner.err
	if $(TARGET) tests/cases/ownership/errors/use-after-forget.ari -o $(BUILD_DIR)/errors/use-after-forget.elf >$(BUILD_DIR)/errors/use-after-forget.err 2>&1; then exit 1; fi
	grep -q "cannot use moved binding 'token'" $(BUILD_DIR)/errors/use-after-forget.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-owner-maybe-unavailable-overwrite.ari -o $(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-overwrite.elf >$(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-overwrite.err 2>&1; then exit 1; fi
	grep -q "cannot overwrite owning binding 'token' while it may be unavailable" $(BUILD_DIR)/errors/loop-break-owner-maybe-unavailable-overwrite.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-owned-field-maybe-unavailable-overwrite.ari -o $(BUILD_DIR)/errors/loop-break-owned-field-maybe-unavailable-overwrite.elf >$(BUILD_DIR)/errors/loop-break-owned-field-maybe-unavailable-overwrite.err 2>&1; then exit 1; fi
	grep -q "cannot overwrite owning field 'holder.0' while it may be unavailable" $(BUILD_DIR)/errors/loop-break-owned-field-maybe-unavailable-overwrite.err
	if $(TARGET) tests/cases/control-flow/errors/loop-continue-owner-state.ari -o $(BUILD_DIR)/errors/loop-continue-owner-state.elf >$(BUILD_DIR)/errors/loop-continue-owner-state.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/loop-continue-owner-state.err
	if $(TARGET) tests/cases/control-flow/errors/init-while-known-true-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/init-while-known-true-owner-widen-recheck.elf >$(BUILD_DIR)/errors/init-while-known-true-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/init-while-known-true-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/init-while-known-bool-alias-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/init-while-known-bool-alias-owner-widen-recheck.elf >$(BUILD_DIR)/errors/init-while-known-bool-alias-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/init-while-known-bool-alias-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-aggregate-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-aggregate-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-aggregate-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-aggregate-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-direct-enum-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-direct-enum-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-direct-enum-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-direct-enum-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-runtime-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-runtime-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-runtime-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-runtime-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-local-enum-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-local-enum-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-local-enum-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-local-enum-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-local-alias-enum-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-local-alias-enum-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-local-alias-enum-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-local-alias-enum-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-static-payload-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-static-payload-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-static-payload-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-static-payload-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-static-nested-payload-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/while-let-static-nested-payload-owner-widen-recheck.elf >$(BUILD_DIR)/errors/while-let-static-nested-payload-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/while-let-static-nested-payload-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-let-enum-ref-mut-temporary.ari -o $(BUILD_DIR)/errors/while-let-enum-ref-mut-temporary.elf >$(BUILD_DIR)/errors/while-let-enum-ref-mut-temporary.err 2>&1; then exit 1; fi
	grep -q "ref mut while-let binding requires an addressable local, field, or indexed element" $(BUILD_DIR)/errors/while-let-enum-ref-mut-temporary.err
	if $(TARGET) tests/cases/control-flow/errors/control-flow-ref-mut-enum-temporary.ari -o $(BUILD_DIR)/errors/control-flow-ref-mut-enum-temporary.elf >$(BUILD_DIR)/errors/control-flow-ref-mut-enum-temporary.err 2>&1; then exit 1; fi
	grep -q "ref mut match binding requires an addressable local, field, or indexed element" $(BUILD_DIR)/errors/control-flow-ref-mut-enum-temporary.err
	if $(TARGET) tests/cases/control-flow/errors/control-flow-ref-mut-aggregate-temporary.ari -o $(BUILD_DIR)/errors/control-flow-ref-mut-aggregate-temporary.elf >$(BUILD_DIR)/errors/control-flow-ref-mut-aggregate-temporary.err 2>&1; then exit 1; fi
	grep -q "ref mut match binding requires an addressable local, field, or indexed element" $(BUILD_DIR)/errors/control-flow-ref-mut-aggregate-temporary.err
	if $(TARGET) tests/cases/control-flow/errors/control-flow-ref-mut-runtime-sequence-temporary.ari -o $(BUILD_DIR)/errors/control-flow-ref-mut-runtime-sequence-temporary.elf >$(BUILD_DIR)/errors/control-flow-ref-mut-runtime-sequence-temporary.err 2>&1; then exit 1; fi
	grep -q "ref mut if-let binding requires an addressable local, field, or indexed element" $(BUILD_DIR)/errors/control-flow-ref-mut-runtime-sequence-temporary.err
	if $(TARGET) tests/cases/control-flow/errors/for-iterator-owner-widen-recheck.ari -o $(BUILD_DIR)/errors/for-iterator-owner-widen-recheck.elf >$(BUILD_DIR)/errors/for-iterator-owner-widen-recheck.err 2>&1; then exit 1; fi
	grep -q "cannot use dropped binding 'token'" $(BUILD_DIR)/errors/for-iterator-owner-widen-recheck.err
	if $(TARGET) tests/cases/control-flow/errors/while-false-borrow-state.ari -o $(BUILD_DIR)/errors/while-false-borrow-state.elf >$(BUILD_DIR)/errors/while-false-borrow-state.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'first'" $(BUILD_DIR)/errors/while-false-borrow-state.err
	if $(TARGET) tests/cases/control-flow/errors/loop-borrow-state.ari -o $(BUILD_DIR)/errors/loop-borrow-state.elf >$(BUILD_DIR)/errors/loop-borrow-state.err 2>&1; then exit 1; fi
	grep -q "binding 'holder' has incompatible borrow states" $(BUILD_DIR)/errors/loop-borrow-state.err
	if $(TARGET) tests/cases/control-flow/errors/loop-continue-borrow-state.ari -o $(BUILD_DIR)/errors/loop-continue-borrow-state.elf >$(BUILD_DIR)/errors/loop-continue-borrow-state.err 2>&1; then exit 1; fi
	grep -q "binding 'holder' has incompatible borrow states" $(BUILD_DIR)/errors/loop-continue-borrow-state.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-borrow-release-conservative.ari -o $(BUILD_DIR)/errors/loop-break-borrow-release-conservative.elf >$(BUILD_DIR)/errors/loop-break-borrow-release-conservative.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'number'" $(BUILD_DIR)/errors/loop-break-borrow-release-conservative.err
	if $(TARGET) tests/cases/control-flow/errors/loop-break-live-owner.ari -o $(BUILD_DIR)/errors/loop-break-live-owner.elf >$(BUILD_DIR)/errors/loop-break-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before break" $(BUILD_DIR)/errors/loop-break-live-owner.err
	if $(TARGET) tests/cases/control-flow/errors/loop-continue-live-owner.ari -o $(BUILD_DIR)/errors/loop-continue-live-owner.elf >$(BUILD_DIR)/errors/loop-continue-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before continue" $(BUILD_DIR)/errors/loop-continue-live-owner.err
	if $(TARGET) tests/cases/control-flow/errors/while-non-bool.ari -o $(BUILD_DIR)/errors/while-non-bool.elf >$(BUILD_DIR)/errors/while-non-bool.err 2>&1; then exit 1; fi
	grep -q "condition must be bool or integer-convertible, got string" $(BUILD_DIR)/errors/while-non-bool.err
	if $(TARGET) tests/cases/control-flow/errors/break-outside-loop.ari -o $(BUILD_DIR)/errors/break-outside-loop.elf >$(BUILD_DIR)/errors/break-outside-loop.err 2>&1; then exit 1; fi
	grep -q "break used outside a loop" $(BUILD_DIR)/errors/break-outside-loop.err
	if $(TARGET) tests/cases/control-flow/errors/continue-outside-loop.ari -o $(BUILD_DIR)/errors/continue-outside-loop.elf >$(BUILD_DIR)/errors/continue-outside-loop.err 2>&1; then exit 1; fi
	grep -q "continue used outside a loop" $(BUILD_DIR)/errors/continue-outside-loop.err
	if $(TARGET) tests/cases/control-flow/errors/continue-values-while.ari -o $(BUILD_DIR)/errors/continue-values-while.elf >$(BUILD_DIR)/errors/continue-values-while.err 2>&1; then exit 1; fi
	grep -q "continue values are only valid inside init-while loops" $(BUILD_DIR)/errors/continue-values-while.err
	if $(TARGET) tests/cases/control-flow/errors/for-not-range.ari -o $(BUILD_DIR)/errors/for-not-range.elf >$(BUILD_DIR)/errors/for-not-range.err 2>&1; then exit 1; fi
	grep -q "for currently supports range values" $(BUILD_DIR)/errors/for-not-range.err
	if $(TARGET) tests/cases/control-flow/errors/range-arg-count.ari -o $(BUILD_DIR)/errors/range-arg-count.elf >$(BUILD_DIR)/errors/range-arg-count.err 2>&1; then exit 1; fi
	grep -q "range expects start and end values" $(BUILD_DIR)/errors/range-arg-count.err
	if $(TARGET) tests/cases/control-flow/errors/for-vector-empty.ari -o $(BUILD_DIR)/errors/for-vector-empty.elf >$(BUILD_DIR)/errors/for-vector-empty.err 2>&1; then exit 1; fi
	grep -Fq "empty [] literals need an explicit Vec[T] type or non-empty array elements" $(BUILD_DIR)/errors/for-vector-empty.err
	if $(TARGET) tests/cases/control-flow/errors/for-vector-mixed-types.ari -o $(BUILD_DIR)/errors/for-vector-mixed-types.elf >$(BUILD_DIR)/errors/for-vector-mixed-types.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/errors/for-vector-mixed-types.err
	if $(TARGET) tests/cases/control-flow/errors/init-while-next-count.ari -o $(BUILD_DIR)/errors/init-while-next-count.elf >$(BUILD_DIR)/errors/init-while-next-count.err 2>&1; then exit 1; fi
	grep -q "next value count must match init binding count" $(BUILD_DIR)/errors/init-while-next-count.err
	if $(TARGET) tests/cases/control-flow/errors/let-while-removed.ari -o $(BUILD_DIR)/errors/let-while-removed.elf >$(BUILD_DIR)/errors/let-while-removed.err 2>&1; then exit 1; fi
	grep -q "let-while loop state was removed; use init ... while ... next" $(BUILD_DIR)/errors/let-while-removed.err
	if $(TARGET) tests/cases/control-flow/errors/init-while-continue-count.ari -o $(BUILD_DIR)/errors/init-while-continue-count.elf >$(BUILD_DIR)/errors/init-while-continue-count.err 2>&1; then exit 1; fi
	grep -q "continue value count must match init binding count" $(BUILD_DIR)/errors/init-while-continue-count.err
	if $(TARGET) tests/cases/borrowing/errors/mutable-borrow-immutable.ari -o $(BUILD_DIR)/errors/mutable-borrow-immutable.elf >$(BUILD_DIR)/errors/mutable-borrow-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow immutable binding 'number'" $(BUILD_DIR)/errors/mutable-borrow-immutable.err
	if $(TARGET) tests/cases/ownership/errors/move-borrowed-owner.ari -o $(BUILD_DIR)/errors/move-borrowed-owner.elf >$(BUILD_DIR)/errors/move-borrowed-owner.err 2>&1; then exit 1; fi
	grep -q "cannot move borrowed binding 'token'" $(BUILD_DIR)/errors/move-borrowed-owner.err
	if $(TARGET) tests/cases/borrowing/errors/mutable-borrow-conflict.ari -o $(BUILD_DIR)/errors/mutable-borrow-conflict.elf >$(BUILD_DIR)/errors/mutable-borrow-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed binding 'number'" $(BUILD_DIR)/errors/mutable-borrow-conflict.err
	if $(TARGET) tests/cases/borrowing/errors/return-borrow.ari -o $(BUILD_DIR)/errors/return-borrow.elf >$(BUILD_DIR)/errors/return-borrow.err 2>&1; then exit 1; fi
	grep -q "function return cannot return a borrow of local binding 'local'" $(BUILD_DIR)/errors/return-borrow.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-return-multiple-params.ari -o $(BUILD_DIR)/errors/borrow-return-multiple-params.elf >$(BUILD_DIR)/errors/borrow-return-multiple-params.err 2>&1; then exit 1; fi
	grep -q "borrow-valued function returns currently require exactly one borrow parameter" $(BUILD_DIR)/errors/borrow-return-multiple-params.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-return-call-conflict.ari -o $(BUILD_DIR)/errors/borrow-return-call-conflict.elf >$(BUILD_DIR)/errors/borrow-return-call-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed binding 'number'" $(BUILD_DIR)/errors/borrow-return-call-conflict.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-return-method-conflict.ari -o $(BUILD_DIR)/errors/borrow-return-method-conflict.elf >$(BUILD_DIR)/errors/borrow-return-method-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed field 'cell.0'" $(BUILD_DIR)/errors/borrow-return-method-conflict.err
	if $(TARGET) tests/cases/borrowing/errors/discard-borrow.ari -o $(BUILD_DIR)/errors/discard-borrow.elf >$(BUILD_DIR)/errors/discard-borrow.err 2>&1; then exit 1; fi
	grep -q "borrow expression result must be passed directly to a call" $(BUILD_DIR)/errors/discard-borrow.err
	$(TARGET) tests/cases/borrowing/ok/borrow-call.ari -o $(BUILD_DIR)/errors/borrow-call.elf
	$(BUILD_DIR)/errors/borrow-call.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/borrowing/ok/borrow-symbol-syntax.ari -o $(BUILD_DIR)/errors/borrow-symbol-syntax.elf
	$(BUILD_DIR)/errors/borrow-symbol-syntax.elf; code=$$?; test $$code -eq 15
	$(TARGET) tests/cases/borrowing/ok/borrow-named.ari -o $(BUILD_DIR)/errors/borrow-named.elf
	$(BUILD_DIR)/errors/borrow-named.elf; code=$$?; test $$code -eq 12
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-named.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-named.elf; $(BUILD_DIR)/errors/borrow-named.elf; code=$$?; test $$code -eq 12; fi
	$(TARGET) tests/cases/patterns/ok/pattern-ref-binding-mode.ari -o $(BUILD_DIR)/errors/pattern-ref-binding-mode.elf
	$(BUILD_DIR)/errors/pattern-ref-binding-mode.elf; code=$$?; test $$code -eq 146
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/patterns/ok/pattern-ref-binding-mode.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/pattern-ref-binding-mode.elf; $(BUILD_DIR)/errors/pattern-ref-binding-mode.elf; code=$$?; test $$code -eq 146; fi
	$(TARGET) tests/cases/patterns/ok/pattern-ref-ownership-aggregate.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-aggregate.elf
	$(BUILD_DIR)/errors/pattern-ref-ownership-aggregate.elf; code=$$?; test $$code -eq 54
	$(TARGET) tests/cases/patterns/ok/pattern-ref-ownership-vector.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector.elf
	$(BUILD_DIR)/errors/pattern-ref-ownership-vector.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/patterns/ok/pattern-ref-ownership-vector-aggregate.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-aggregate.elf
	$(BUILD_DIR)/errors/pattern-ref-ownership-vector-aggregate.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/patterns/ok/pattern-ref-ownership-vector-rest.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest.elf
	$(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/patterns/ok/pattern-ref-ownership-vector-rest-alias.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest-alias.elf
	$(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest-alias.elf; code=$$?; test $$code -eq 2
	$(TARGET) tests/cases/patterns/ok/pattern-ref-ownership-vector-dynamic-suffix.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-dynamic-suffix.elf
	$(BUILD_DIR)/errors/pattern-ref-ownership-vector-dynamic-suffix.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/patterns/ok/pattern-ref-mut-ownership-vector-dynamic-suffix.ari -o $(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix.elf
	$(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/patterns/ok/pattern-ref-mut-ownership-vector-dynamic-suffix-multi.ari -o $(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix-multi.elf
	$(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix-multi.elf; code=$$?; test $$code -eq 0
	$(TARGET) tests/cases/patterns/ok/pattern-value-ownership-vector.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector.elf
	$(BUILD_DIR)/errors/pattern-value-ownership-vector.elf; code=$$?; test $$code -eq 19
	$(TARGET) tests/cases/patterns/ok/pattern-value-ownership-vector-rest.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector-rest.elf
	$(BUILD_DIR)/errors/pattern-value-ownership-vector-rest.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/patterns/ok/pattern-value-ownership-vector-skip.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector-skip.elf
	$(BUILD_DIR)/errors/pattern-value-ownership-vector-skip.elf; code=$$?; test $$code -eq 11
	$(TARGET) tests/cases/patterns/ok/pattern-value-ownership-vector-rest-skip.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector-rest-skip.elf
	$(BUILD_DIR)/errors/pattern-value-ownership-vector-rest-skip.elf; code=$$?; test $$code -eq 12
	$(TARGET) tests/cases/patterns/ok/pattern-value-ownership-vector-dynamic-suffix.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector-dynamic-suffix.elf
	$(BUILD_DIR)/errors/pattern-value-ownership-vector-dynamic-suffix.elf; code=$$?; test $$code -eq 120
	$(TARGET) tests/cases/patterns/ok/pattern-value-ownership-vector-rest-alias.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector-rest-alias.elf
	$(BUILD_DIR)/errors/pattern-value-ownership-vector-rest-alias.elf; code=$$?; test $$code -eq 2
	$(TARGET) tests/cases/patterns/ok/pattern-slice-owner-element-paths.ari -o $(BUILD_DIR)/errors/pattern-slice-owner-element-paths.elf
	$(BUILD_DIR)/errors/pattern-slice-owner-element-paths.elf; code=$$?; test $$code -eq 40
	$(TARGET) tests/cases/patterns/ok/pattern-amp-binding-mode.ari -o $(BUILD_DIR)/errors/pattern-amp-binding-mode.elf
	$(BUILD_DIR)/errors/pattern-amp-binding-mode.elf; code=$$?; test $$code -eq 238
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/patterns/ok/pattern-amp-binding-mode.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/pattern-amp-binding-mode.elf; $(BUILD_DIR)/errors/pattern-amp-binding-mode.elf; code=$$?; test $$code -eq 238; fi
	$(TARGET) tests/cases/patterns/ok/pattern-ref-runtime-sequence-rest.ari -o $(BUILD_DIR)/errors/pattern-ref-runtime-sequence-rest.elf
	$(BUILD_DIR)/errors/pattern-ref-runtime-sequence-rest.elf; code=$$?; test $$code -eq 140
	$(TARGET) tests/cases/patterns/ok/pattern-ref-enum-payload.ari -o $(BUILD_DIR)/errors/pattern-ref-enum-payload.elf
	$(BUILD_DIR)/errors/pattern-ref-enum-payload.elf; code=$$?; test $$code -eq 181
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-compact-enum-payload.ari -o $(BUILD_DIR)/errors/pattern-ref-compact-enum-payload.elf >$(BUILD_DIR)/errors/pattern-ref-compact-enum-payload.err 2>&1; then exit 1; fi
	grep -Fq "compact enum payload reference binding for i32 is value-only" $(BUILD_DIR)/errors/pattern-ref-compact-enum-payload.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-non-addressable-enum-payload.ari -o $(BUILD_DIR)/errors/pattern-ref-non-addressable-enum-payload.elf >$(BUILD_DIR)/errors/pattern-ref-non-addressable-enum-payload.err 2>&1; then exit 1; fi
	grep -Fq "non-addressable enum payload reference binding for i32 is value-only" $(BUILD_DIR)/errors/pattern-ref-non-addressable-enum-payload.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-mut-immutable.ari -o $(BUILD_DIR)/errors/pattern-ref-mut-immutable.elf >$(BUILD_DIR)/errors/pattern-ref-mut-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow immutable binding 'value'" $(BUILD_DIR)/errors/pattern-ref-mut-immutable.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-ownership-moved-field.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-moved-field.elf >$(BUILD_DIR)/errors/pattern-ref-ownership-moved-field.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'holder.token'" $(BUILD_DIR)/errors/pattern-ref-ownership-moved-field.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-ownership-vector-moved-field.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-moved-field.elf >$(BUILD_DIR)/errors/pattern-ref-ownership-vector-moved-field.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'values.0'" $(BUILD_DIR)/errors/pattern-ref-ownership-vector-moved-field.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-ownership-vector-aggregate-moved-field.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-aggregate-moved-field.elf >$(BUILD_DIR)/errors/pattern-ref-ownership-vector-aggregate-moved-field.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'holders.0.0'" $(BUILD_DIR)/errors/pattern-ref-ownership-vector-aggregate-moved-field.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-ownership-vector-rest.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest.elf >$(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'values.2'" $(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-ownership-vector-dynamic-suffix-moved.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-dynamic-suffix-moved.elf >$(BUILD_DIR)/errors/pattern-ref-ownership-vector-dynamic-suffix-moved.err 2>&1; then exit 1; fi
	grep -q "cannot borrow partially moved owning binding 'values'" $(BUILD_DIR)/errors/pattern-ref-ownership-vector-dynamic-suffix-moved.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-mut-ownership-vector-dynamic-suffix.ari -o $(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix.elf >$(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix.err 2>&1; then exit 1; fi
	grep -Fq "cannot drop borrowed binding 'values'" $(BUILD_DIR)/errors/pattern-ref-mut-ownership-vector-dynamic-suffix.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-ownership-vector-rest-alias.ari -o $(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest-alias.elf >$(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest-alias.err 2>&1; then exit 1; fi
	grep -Fq "cannot drop borrowed binding 'values'" $(BUILD_DIR)/errors/pattern-ref-ownership-vector-rest-alias.err
	if $(TARGET) tests/cases/patterns/errors/pattern-value-ownership-vector-dynamic-rest-alias.ari -o $(BUILD_DIR)/errors/pattern-value-ownership-vector-dynamic-rest-alias.elf >$(BUILD_DIR)/errors/pattern-value-ownership-vector-dynamic-rest-alias.err 2>&1; then exit 1; fi
	grep -Fq "unknown-length ownership-carrying Vec[T] value rest aliases are not supported" $(BUILD_DIR)/errors/pattern-value-ownership-vector-dynamic-rest-alias.err
	if $(TARGET) tests/cases/patterns/errors/slice-owner-index-move.ari -o $(BUILD_DIR)/errors/slice-owner-index-move.elf >$(BUILD_DIR)/errors/slice-owner-index-move.err 2>&1; then exit 1; fi
	grep -Fq "cannot move ownership out of non-owning Slice[own T] view" $(BUILD_DIR)/errors/slice-owner-index-move.err
	if $(TARGET) tests/cases/patterns/errors/slice-owner-value-pattern-move.ari -o $(BUILD_DIR)/errors/slice-owner-value-pattern-move.elf >$(BUILD_DIR)/errors/slice-owner-value-pattern-move.err 2>&1; then exit 1; fi
	grep -Fq "cannot move ownership out of non-owning Slice[own T] view" $(BUILD_DIR)/errors/slice-owner-value-pattern-move.err
	if $(TARGET) tests/cases/patterns/errors/slice-owner-assignment.ari -o $(BUILD_DIR)/errors/slice-owner-assignment.elf >$(BUILD_DIR)/errors/slice-owner-assignment.err 2>&1; then exit 1; fi
	grep -Fq "cannot assign ownership-valued elements through non-owning Slice[own T] view" $(BUILD_DIR)/errors/slice-owner-assignment.err
	if $(TARGET) tests/cases/patterns/errors/pattern-ref-temporary.ari -o $(BUILD_DIR)/errors/pattern-ref-temporary.elf >$(BUILD_DIR)/errors/pattern-ref-temporary.err 2>&1; then exit 1; fi
	grep -q "reference pattern binding initializer must be a local binding, field access, tuple index, or constant array/vector index" $(BUILD_DIR)/errors/pattern-ref-temporary.err
	$(TARGET) tests/cases/borrowing/ok/named-borrow-last-use.ari -o $(BUILD_DIR)/errors/named-borrow-last-use.elf
	$(BUILD_DIR)/errors/named-borrow-last-use.elf; code=$$?; test $$code -eq 18
	$(TARGET) tests/cases/borrowing/ok/borrow-reborrow.ari -o $(BUILD_DIR)/errors/borrow-reborrow.elf
	$(BUILD_DIR)/errors/borrow-reborrow.elf; code=$$?; test $$code -eq 14
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-reborrow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-reborrow.elf; $(BUILD_DIR)/errors/borrow-reborrow.elf; code=$$?; test $$code -eq 14; fi
	$(TARGET) tests/cases/borrowing/ok/borrow-reborrow-paths.ari -o $(BUILD_DIR)/errors/borrow-reborrow-paths.elf
	$(BUILD_DIR)/errors/borrow-reborrow-paths.elf; code=$$?; test $$code -eq 61
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-reborrow-paths.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-reborrow-paths.elf; $(BUILD_DIR)/errors/borrow-reborrow-paths.elf; code=$$?; test $$code -eq 61; fi
	$(TARGET) tests/cases/borrowing/ok/method-ref-mut-to-shared-receiver.ari -o $(BUILD_DIR)/errors/method-ref-mut-to-shared-receiver.elf
	$(BUILD_DIR)/errors/method-ref-mut-to-shared-receiver.elf; code=$$?; test $$code -eq 43
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/method-ref-mut-to-shared-receiver.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/method-ref-mut-to-shared-receiver.elf; $(BUILD_DIR)/errors/method-ref-mut-to-shared-receiver.elf; code=$$?; test $$code -eq 43; fi
	$(TARGET) tests/cases/borrowing/ok/borrow-control-flow-result.ari -o $(BUILD_DIR)/errors/borrow-control-flow-result.elf
	$(BUILD_DIR)/errors/borrow-control-flow-result.elf; code=$$?; test $$code -eq 4
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-control-flow-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-control-flow-result.elf; $(BUILD_DIR)/errors/borrow-control-flow-result.elf; code=$$?; test $$code -eq 4; fi
	$(TARGET) tests/cases/borrowing/ok/borrow-return-param.ari -o $(BUILD_DIR)/errors/borrow-return-param.elf
	$(BUILD_DIR)/errors/borrow-return-param.elf; code=$$?; test $$code -eq 80
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-return-param.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-return-param.elf; $(BUILD_DIR)/errors/borrow-return-param.elf; code=$$?; test $$code -eq 80; fi
	$(TARGET) tests/cases/borrowing/ok/borrow-return-subpath.ari -o $(BUILD_DIR)/errors/borrow-return-subpath.elf
	$(BUILD_DIR)/errors/borrow-return-subpath.elf; code=$$?; test $$code -eq 182
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-return-subpath.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-return-subpath.elf; $(BUILD_DIR)/errors/borrow-return-subpath.elf; code=$$?; test $$code -eq 182; fi
	$(TARGET) tests/cases/borrowing/ok/borrow-return-explicit-source.ari -o $(BUILD_DIR)/errors/borrow-return-explicit-source.elf
	$(BUILD_DIR)/errors/borrow-return-explicit-source.elf; code=$$?; test $$code -eq 39
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/cases/borrowing/ok/borrow-return-explicit-source.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-return-explicit-source.elf; $(BUILD_DIR)/errors/borrow-return-explicit-source.elf; code=$$?; test $$code -eq 39; fi
	if $(TARGET) tests/cases/borrowing/errors/borrow-return-path-mismatch.ari -o $(BUILD_DIR)/errors/borrow-return-path-mismatch.elf >$(BUILD_DIR)/errors/borrow-return-path-mismatch.err 2>&1; then exit 1; fi
	grep -q "function return borrow result must use the same source path on every return" $(BUILD_DIR)/errors/borrow-return-path-mismatch.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-return-explicit-path-mismatch.ari -o $(BUILD_DIR)/errors/borrow-return-explicit-path-mismatch.elf >$(BUILD_DIR)/errors/borrow-return-explicit-path-mismatch.err 2>&1; then exit 1; fi
	grep -q "function return borrow result must use the same source path on every return" $(BUILD_DIR)/errors/borrow-return-explicit-path-mismatch.err
	if $(TARGET) tests/cases/borrowing/errors/named-borrow-assign-source.ari -o $(BUILD_DIR)/errors/named-borrow-assign-source.elf >$(BUILD_DIR)/errors/named-borrow-assign-source.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'number'" $(BUILD_DIR)/errors/named-borrow-assign-source.err
	if $(TARGET) tests/cases/borrowing/errors/named-borrow-conflict.ari -o $(BUILD_DIR)/errors/named-borrow-conflict.elf >$(BUILD_DIR)/errors/named-borrow-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed binding 'number'" $(BUILD_DIR)/errors/named-borrow-conflict.err
	if $(TARGET) tests/cases/borrowing/errors/named-borrow-reassign.ari -o $(BUILD_DIR)/errors/named-borrow-reassign.elf >$(BUILD_DIR)/errors/named-borrow-reassign.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrow binding 'shared'" $(BUILD_DIR)/errors/named-borrow-reassign.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-reborrow-mut-from-shared.ari -o $(BUILD_DIR)/errors/borrow-reborrow-mut-from-shared.elf >$(BUILD_DIR)/errors/borrow-reborrow-mut-from-shared.err 2>&1; then exit 1; fi
	grep -q "cannot mutably reborrow immutable borrow binding 'shared'" $(BUILD_DIR)/errors/borrow-reborrow-mut-from-shared.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-reborrow-active-conflict.ari -o $(BUILD_DIR)/errors/borrow-reborrow-active-conflict.elf >$(BUILD_DIR)/errors/borrow-reborrow-active-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow already borrowed binding 'unique'" $(BUILD_DIR)/errors/borrow-reborrow-active-conflict.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-reborrow-path-mut-from-shared.ari -o $(BUILD_DIR)/errors/borrow-reborrow-path-mut-from-shared.elf >$(BUILD_DIR)/errors/borrow-reborrow-path-mut-from-shared.err 2>&1; then exit 1; fi
	grep -q "cannot mutably reborrow immutable borrow binding 'shared'" $(BUILD_DIR)/errors/borrow-reborrow-path-mut-from-shared.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-reborrow-path-active-conflict.ari -o $(BUILD_DIR)/errors/borrow-reborrow-path-active-conflict.elf >$(BUILD_DIR)/errors/borrow-reborrow-path-active-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed field 'unique.0'" $(BUILD_DIR)/errors/borrow-reborrow-path-active-conflict.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-control-flow-mismatch.ari -o $(BUILD_DIR)/errors/borrow-control-flow-mismatch.elf >$(BUILD_DIR)/errors/borrow-control-flow-mismatch.err 2>&1; then exit 1; fi
	grep -q "if expression must borrow the same source path and mode in every result arm" $(BUILD_DIR)/errors/borrow-control-flow-mismatch.err
	if $(TARGET) tests/cases/borrowing/errors/borrow-control-flow-local-escape.ari -o $(BUILD_DIR)/errors/borrow-control-flow-local-escape.elf >$(BUILD_DIR)/errors/borrow-control-flow-local-escape.err 2>&1; then exit 1; fi
	grep -q "block expression cannot return a borrow of local binding 'inner'" $(BUILD_DIR)/errors/borrow-control-flow-local-escape.err
	if $(TARGET) tests/cases/literals/errors/u8-literal-range.ari -o $(BUILD_DIR)/errors/u8-literal-range.elf >$(BUILD_DIR)/errors/u8-literal-range.err 2>&1; then exit 1; fi
	grep -q "integer literal 256 is out of range for u8" $(BUILD_DIR)/errors/u8-literal-range.err
	if $(TARGET) tests/cases/literals/errors/i8-literal-range.ari -o $(BUILD_DIR)/errors/i8-literal-range.elf >$(BUILD_DIR)/errors/i8-literal-range.err 2>&1; then exit 1; fi
	grep -q "integer literal -129 is out of range for i8" $(BUILD_DIR)/errors/i8-literal-range.err
	if $(TARGET) tests/cases/literals/errors/integer-literal-too-large.ari -o $(BUILD_DIR)/errors/integer-literal-too-large.elf >$(BUILD_DIR)/errors/integer-literal-too-large.err 2>&1; then exit 1; fi
	grep -q "integer literal is too large" $(BUILD_DIR)/errors/integer-literal-too-large.err
	if $(TARGET) tests/cases/literals/errors/byte-char-too-long.ari -o $(BUILD_DIR)/errors/byte-char-too-long.elf >$(BUILD_DIR)/errors/byte-char-too-long.err 2>&1; then exit 1; fi
	grep -q "byte character literal must contain exactly one byte" $(BUILD_DIR)/errors/byte-char-too-long.err
	if $(TARGET) tests/cases/literals/errors/byte-char-unicode.ari -o $(BUILD_DIR)/errors/byte-char-unicode.elf >$(BUILD_DIR)/errors/byte-char-unicode.err 2>&1; then exit 1; fi
	grep -q "Unicode escape in byte character literal must fit in ASCII" $(BUILD_DIR)/errors/byte-char-unicode.err
	if $(TARGET) tests/cases/literals/errors/class-removed.ari -o $(BUILD_DIR)/errors/class-removed.elf >$(BUILD_DIR)/errors/class-removed.err 2>&1; then exit 1; fi
	grep -q "expected top-level declaration" $(BUILD_DIR)/errors/class-removed.err
	if $(TARGET) tests/cases/literals/errors/interface-removed.ari -o $(BUILD_DIR)/errors/interface-removed.elf >$(BUILD_DIR)/errors/interface-removed.err 2>&1; then exit 1; fi
	grep -q "expected top-level declaration" $(BUILD_DIR)/errors/interface-removed.err
	if $(TARGET) tests/cases/traits/errors/unknown-trait-impl.ari -o $(BUILD_DIR)/errors/unknown-trait-impl.elf >$(BUILD_DIR)/errors/unknown-trait-impl.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'MissingTrait'" $(BUILD_DIR)/errors/unknown-trait-impl.err
	if $(TARGET) tests/cases/traits/errors/iterable-trait-arity.ari -o $(BUILD_DIR)/errors/iterable-trait-arity.elf >$(BUILD_DIR)/errors/iterable-trait-arity.err 2>&1; then exit 1; fi
	grep -q "trait 'std::Iterable' expects 1 type argument" $(BUILD_DIR)/errors/iterable-trait-arity.err
	if $(TARGET) tests/cases/traits/errors/duplicate-trait-impl.ari -o $(BUILD_DIR)/errors/duplicate-trait-impl.elf >$(BUILD_DIR)/errors/duplicate-trait-impl.err 2>&1; then exit 1; fi
	grep -q "duplicate impl of trait 'std::Iterable' for Bag" $(BUILD_DIR)/errors/duplicate-trait-impl.err
	if $(TARGET) tests/cases/operators/errors/logical-non-bool.ari -o $(BUILD_DIR)/errors/logical-non-bool.elf >$(BUILD_DIR)/errors/logical-non-bool.err 2>&1; then exit 1; fi
	grep -q "logical operand must be bool, got i64" $(BUILD_DIR)/errors/logical-non-bool.err
	if $(TARGET) tests/cases/operators/errors/not-non-bool.ari -o $(BUILD_DIR)/errors/not-non-bool.elf >$(BUILD_DIR)/errors/not-non-bool.err 2>&1; then exit 1; fi
	grep -q "logical operand must be bool, got i64" $(BUILD_DIR)/errors/not-non-bool.err
	if $(TARGET) tests/cases/operators/errors/invalid-cast.ari -o $(BUILD_DIR)/errors/invalid-cast.elf >$(BUILD_DIR)/errors/invalid-cast.err 2>&1; then exit 1; fi
	grep -q "explicit casts currently require integer types, float types, integer/float casts, or raw pointer casts, got bool as i64" $(BUILD_DIR)/errors/invalid-cast.err
	if $(TARGET) tests/cases/match/errors/enum-payload-unsupported-vector.ari -o $(BUILD_DIR)/errors/enum-payload-unsupported-vector.elf >$(BUILD_DIR)/errors/enum-payload-unsupported-vector.err 2>&1; then exit 1; fi
	grep -q "enum aggregate payloads currently support integer, float, bool, pointer-shaped, one-word enum, owned i64/u64, plain tuple/array/struct aggregate, fixed-capacity vector, or nested aggregate enum values" $(BUILD_DIR)/errors/enum-payload-unsupported-vector.err
	if $(TARGET) tests/cases/match/errors/enum-payload-count.ari -o $(BUILD_DIR)/errors/enum-payload-count.elf >$(BUILD_DIR)/errors/enum-payload-count.err 2>&1; then exit 1; fi
	grep -q "wrong payload count for enum case 'Some'" $(BUILD_DIR)/errors/enum-payload-count.err
	if $(TARGET) tests/cases/match/errors/match-nonexhaustive.ari -o $(BUILD_DIR)/errors/match-nonexhaustive.elf >$(BUILD_DIR)/errors/match-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "match must cover all cases of enum 'OptionI32'" $(BUILD_DIR)/errors/match-nonexhaustive.err
	if $(TARGET) tests/cases/match/errors/match-payload-required.ari -o $(BUILD_DIR)/errors/match-payload-required.elf >$(BUILD_DIR)/errors/match-payload-required.err 2>&1; then exit 1; fi
	grep -q "enum case 'Some' requires a payload pattern" $(BUILD_DIR)/errors/match-payload-required.err
	if $(TARGET) tests/cases/match/errors/match-empty.ari -o $(BUILD_DIR)/errors/match-empty.elf >$(BUILD_DIR)/errors/match-empty.err 2>&1; then exit 1; fi
	grep -q "match must have at least one arm" $(BUILD_DIR)/errors/match-empty.err
	if $(TARGET) tests/cases/match/errors/match-duplicate-arm.ari -o $(BUILD_DIR)/errors/match-duplicate-arm.elf >$(BUILD_DIR)/errors/match-duplicate-arm.err 2>&1; then exit 1; fi
	grep -q "duplicate match arm for enum case 'A'" $(BUILD_DIR)/errors/match-duplicate-arm.err
	if $(TARGET) tests/cases/match/errors/match-unreachable-after-wildcard.ari -o $(BUILD_DIR)/errors/match-unreachable-after-wildcard.elf >$(BUILD_DIR)/errors/match-unreachable-after-wildcard.err 2>&1; then exit 1; fi
	grep -q "unreachable match arm after wildcard" $(BUILD_DIR)/errors/match-unreachable-after-wildcard.err
	if $(TARGET) tests/cases/match/errors/match-no-payload.ari -o $(BUILD_DIR)/errors/match-no-payload.elf >$(BUILD_DIR)/errors/match-no-payload.err 2>&1; then exit 1; fi
	grep -q "enum case 'A' has no payload" $(BUILD_DIR)/errors/match-no-payload.err
	if $(TARGET) tests/cases/match/errors/match-wrong-enum-case.ari -o $(BUILD_DIR)/errors/match-wrong-enum-case.elf >$(BUILD_DIR)/errors/match-wrong-enum-case.err 2>&1; then exit 1; fi
	grep -q "enum case 'B' does not belong to Left" $(BUILD_DIR)/errors/match-wrong-enum-case.err
	if $(TARGET) tests/cases/match/errors/match-non-enum.ari -o $(BUILD_DIR)/errors/match-non-enum.elf >$(BUILD_DIR)/errors/match-non-enum.err 2>&1; then exit 1; fi
	grep -q "match value must be an enum, integer, bool, tuple, array, or struct, got string" $(BUILD_DIR)/errors/match-non-enum.err
	if $(TARGET) tests/cases/match/errors/match-branch-state-mismatch.ari -o $(BUILD_DIR)/errors/match-branch-state-mismatch.elf >$(BUILD_DIR)/errors/match-branch-state-mismatch.err 2>&1; then exit 1; fi
	grep -q "has incompatible ownership states after match arms" $(BUILD_DIR)/errors/match-branch-state-mismatch.err
	if $(TARGET) tests/cases/modules/errors/private-module-function.ari -o $(BUILD_DIR)/errors/private-module-function.elf >$(BUILD_DIR)/errors/private-module-function.err 2>&1; then exit 1; fi
	grep -q "function 'Secret::value' is not public" $(BUILD_DIR)/errors/private-module-function.err
	if $(TARGET) tests/cases/modules/errors/duplicate-use-alias.ari -o $(BUILD_DIR)/errors/duplicate-use-alias.elf >$(BUILD_DIR)/errors/duplicate-use-alias.err 2>&1; then exit 1; fi
	grep -q "duplicate use alias 'value'" $(BUILD_DIR)/errors/duplicate-use-alias.err
	if $(TARGET) tests/cases/modules/errors/private-module-use.ari -o $(BUILD_DIR)/errors/private-module-use.elf >$(BUILD_DIR)/errors/private-module-use.err 2>&1; then exit 1; fi
	grep -q "function 'Secret::value' is not public" $(BUILD_DIR)/errors/private-module-use.err
	if $(TARGET) tests/cases/modules/errors/private-module-enum-type.ari -o $(BUILD_DIR)/errors/private-module-enum-type.elf >$(BUILD_DIR)/errors/private-module-enum-type.err 2>&1; then exit 1; fi
	grep -q "enum 'Hidden::Secret' is not public" $(BUILD_DIR)/errors/private-module-enum-type.err
	if $(TARGET) tests/cases/modules/errors/private-module-enum-case.ari -o $(BUILD_DIR)/errors/private-module-enum-case.elf >$(BUILD_DIR)/errors/private-module-enum-case.err 2>&1; then exit 1; fi
	grep -q "enum case 'Hidden::A' is not public" $(BUILD_DIR)/errors/private-module-enum-case.err
	if $(TARGET) tests/cases/modules/errors/missing-module.ari -o $(BUILD_DIR)/errors/missing-module.elf >$(BUILD_DIR)/errors/missing-module.err 2>&1; then exit 1; fi
	grep -q "cannot find module file for 'definitely_missing_module'" $(BUILD_DIR)/errors/missing-module.err
	if $(TARGET) tests/cases/standard-library/errors/format/format-placeholder-count.ari -o $(BUILD_DIR)/errors/format-placeholder-count.elf >$(BUILD_DIR)/errors/format-placeholder-count.err 2>&1; then exit 1; fi
	grep -q "format string has 2 placeholders but 1 values were provided" $(BUILD_DIR)/errors/format-placeholder-count.err
	if $(TARGET) tests/cases/standard-library/errors/format/format-nonliteral.ari -o $(BUILD_DIR)/errors/format-nonliteral.elf >$(BUILD_DIR)/errors/format-nonliteral.err 2>&1; then exit 1; fi
	grep -q "expects a string literal format argument" $(BUILD_DIR)/errors/format-nonliteral.err
	if $(TARGET) tests/cases/standard-library/errors/format/format-named-capture-call.ari -o $(BUILD_DIR)/errors/format-named-capture-call.elf >$(BUILD_DIR)/errors/format-named-capture-call.err 2>&1; then exit 1; fi
	grep -q "optional :? or :.N specs" $(BUILD_DIR)/errors/format-named-capture-call.err
	if $(TARGET) tests/cases/standard-library/errors/format/format-precision-nonfloat.ari -o $(BUILD_DIR)/errors/format-precision-nonfloat.elf >$(BUILD_DIR)/errors/format-precision-nonfloat.err 2>&1; then exit 1; fi
	grep -q "format precision placeholders require f32 or f64 arguments" $(BUILD_DIR)/errors/format-precision-nonfloat.err
	if $(TARGET) tests/cases/standard-library/errors/format/format-precision-syntax.ari -o $(BUILD_DIR)/errors/format-precision-syntax.elf >$(BUILD_DIR)/errors/format-precision-syntax.err 2>&1; then exit 1; fi
	grep -q "format precision placeholder expects digits after colon-dot" $(BUILD_DIR)/errors/format-precision-syntax.err
	if $(TARGET) tests/cases/standard-library/errors/format/prelude-format-in-no-debug.ari -o $(BUILD_DIR)/errors/prelude-format-in-no-debug.elf >$(BUILD_DIR)/errors/prelude-format-in-no-debug.err 2>&1; then exit 1; fi
	grep -q "format_in! debug placeholders require Debug values" $(BUILD_DIR)/errors/prelude-format-in-no-debug.err
	if $(TARGET) tests/cases/ffi/errors/extern-cpp-removed.ari -o $(BUILD_DIR)/errors/extern-cpp-removed.elf >$(BUILD_DIR)/errors/extern-cpp-removed.err 2>&1; then exit 1; fi
	grep -q 'extern ABI must be "C" or "ari"' $(BUILD_DIR)/errors/extern-cpp-removed.err
	if $(TARGET) tests/cases/ffi/errors/extern-body.ari -o $(BUILD_DIR)/errors/extern-body.elf >$(BUILD_DIR)/errors/extern-body.err 2>&1; then exit 1; fi
	grep -q "extern functions cannot have a body" $(BUILD_DIR)/errors/extern-body.err
	if $(TARGET) tests/cases/ffi/errors/extern-generic.ari -o $(BUILD_DIR)/errors/extern-generic.elf >$(BUILD_DIR)/errors/extern-generic.err 2>&1; then exit 1; fi
	grep -q "extern C declarations cannot be generic; declare concrete C symbols or wrap generic foreign APIs in C" $(BUILD_DIR)/errors/extern-generic.err
	if $(TARGET) tests/cases/ffi/errors/extern-invalid-link-name.ari -o $(BUILD_DIR)/errors/extern-invalid-link-name.elf >$(BUILD_DIR)/errors/extern-invalid-link-name.err 2>&1; then exit 1; fi
	grep -q "invalid external link symbol 'bad-symbol'" $(BUILD_DIR)/errors/extern-invalid-link-name.err
