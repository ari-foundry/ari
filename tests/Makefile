# Test targets are included by the repository-root Makefile.
.PHONY: check check-debug check-sanitize check-cli check-prelude check-control-flow check-match check-modules check-operators check-generics check-meta check-traits check-structs check-variables check-ffi check-functions check-function-errors check-errors

check-debug: TARGET := $(DEBUG_TARGET)
check-debug: $(DEBUG_TARGET) check

check-sanitize: TARGET := env ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 $(SANITIZE_TARGET)
check-sanitize: $(SANITIZE_TARGET) check

check: $(TARGET) check-cli check-prelude check-control-flow check-match check-modules check-operators check-generics check-meta check-traits check-structs check-variables check-ffi check-functions check-errors
	mkdir -p $(BUILD_DIR)/ir
	$(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/ir/count.ll
	grep -q 'define i64 @"_ARNv4main"' $(BUILD_DIR)/ir/count.ll
	$(TARGET) tests/ok/prelude-io.ari --emit-llvm $(BUILD_DIR)/ir/prelude-io.ll
	grep -q "declare i32 @printf" $(BUILD_DIR)/ir/prelude-io.ll
	$(TARGET) tests/ok/format-print.ari --emit-llvm $(BUILD_DIR)/ir/format-print.ll
	grep -q "@ari_builtin_write_i64" $(BUILD_DIR)/ir/format-print.ll
	$(TARGET) tests/ok/context-arg.ari --emit-llvm $(BUILD_DIR)/ir/context-arg.ll
	grep -q "@ari_builtin_context_arg" $(BUILD_DIR)/ir/context-arg.ll
	$(TARGET) tests/ok/ffi-c-puts.ari --emit-llvm $(BUILD_DIR)/ir/ffi-c-puts.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ir/ffi-c-puts.ll
	$(TARGET) tests/ok/ffi-c-module.ari --emit-llvm $(BUILD_DIR)/ir/ffi-c-module.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ir/ffi-c-module.ll
	$(TARGET) tests/ok/ffi-c-alias.ari --emit-llvm $(BUILD_DIR)/ir/ffi-c-alias.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ir/ffi-c-alias.ll
	$(TARGET) tests/ok/library-export.ari --shared --emit-llvm $(BUILD_DIR)/ir/library-export.ll
	grep -q 'define i64 @"_ARNv3add"' $(BUILD_DIR)/ir/library-export.ll
	if grep -q "__P" $(BUILD_DIR)/ir/library-export.ll; then exit 1; fi
	if grep -q "__R" $(BUILD_DIR)/ir/library-export.ll; then exit 1; fi
	$(TARGET) --freestanding examples/count.ari -o $(BUILD_DIR)/count.elf
	$(BUILD_DIR)/count.elf; code=$$?; test $$code -eq 5
	$(TARGET) --freestanding tests/ok/own-return.ari -o $(BUILD_DIR)/own-return.elf
	$(BUILD_DIR)/own-return.elf; code=$$?; test $$code -eq 3
	$(TARGET) --freestanding tests/ok/bitwise.ari -o $(BUILD_DIR)/bitwise.elf
	$(BUILD_DIR)/bitwise.elf; code=$$?; test $$code -eq 2
	$(TARGET) --freestanding tests/ok/control-flow-freestanding.ari -o $(BUILD_DIR)/control-flow-freestanding.elf
	$(BUILD_DIR)/control-flow-freestanding.elf; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/for-vector-freestanding.ari -o $(BUILD_DIR)/for-vector-freestanding.elf
	$(BUILD_DIR)/for-vector-freestanding.elf; code=$$?; test $$code -eq 9
	$(TARGET) --freestanding tests/ok/module-freestanding.ari -o $(BUILD_DIR)/module-freestanding.elf
	$(BUILD_DIR)/module-freestanding.elf; code=$$?; test $$code -eq 3

check-cli: $(TARGET)
	mkdir -p $(BUILD_DIR)/cli
	$(TARGET) examples/count.ari --emit-llvm $(BUILD_DIR)/cli/count.ll
	test -s $(BUILD_DIR)/cli/count.ll
	grep -q 'define i64 @"_ARNv4main"' $(BUILD_DIR)/cli/count.ll
	grep -q 'define i64 @"ari::main"()' $(BUILD_DIR)/cli/count.ll
	grep -q 'define i64 @ari_entry(i32 %argc, ptr %argv)' $(BUILD_DIR)/cli/count.ll
	grep -q 'call i64 @"ari::main"()' $(BUILD_DIR)/cli/count.ll
	grep -q "define i32 @main" $(BUILD_DIR)/cli/count.ll
	$(TARGET) --freestanding examples/count.ari -o $(BUILD_DIR)/cli/count.raw
	test -x $(BUILD_DIR)/cli/count.raw
	$(BUILD_DIR)/cli/count.raw; code=$$?; test $$code -eq 5
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) examples/count.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/cli/output.elf; test -x $(BUILD_DIR)/cli/output.elf; $(BUILD_DIR)/cli/output.elf; code=$$?; test $$code -eq 5; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/library-export.ari --llvm-cc $(LLVM_CC) --shared -o $(BUILD_DIR)/cli/libari_cli.so; test -s $(BUILD_DIR)/cli/libari_cli.so; fi
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
	if $(TARGET) tests/ok/package-module-main.ari --module-path >$(BUILD_DIR)/cli/missing-module-path.err 2>&1; then exit 1; fi
	grep -q -- "--module-path expects a path" $(BUILD_DIR)/cli/missing-module-path.err
	if $(TARGET) tests/ok/package-module-main.ari --emit-module-metadata >$(BUILD_DIR)/cli/missing-emit-module-metadata.err 2>&1; then exit 1; fi
	grep -q -- "--emit-module-metadata expects a path" $(BUILD_DIR)/cli/missing-emit-module-metadata.err
	if $(TARGET) tests/ok/package-module-main.ari --check-module-metadata >$(BUILD_DIR)/cli/missing-check-module-metadata.err 2>&1; then exit 1; fi
	grep -q -- "--check-module-metadata expects a path" $(BUILD_DIR)/cli/missing-check-module-metadata.err
	if $(TARGET) tests/ok/package-module-main.ari --emit-module-cache >$(BUILD_DIR)/cli/missing-emit-module-cache.err 2>&1; then exit 1; fi
	grep -q -- "--emit-module-cache expects a path" $(BUILD_DIR)/cli/missing-emit-module-cache.err
	if $(TARGET) tests/ok/package-module-main.ari --use-module-cache >$(BUILD_DIR)/cli/missing-use-module-cache.err 2>&1; then exit 1; fi
	grep -q -- "--use-module-cache expects a path" $(BUILD_DIR)/cli/missing-use-module-cache.err
	if $(TARGET) tests/ok/package-module-main.ari -I >$(BUILD_DIR)/cli/missing-I.err 2>&1; then exit 1; fi
	grep -q -- "-I expects a path" $(BUILD_DIR)/cli/missing-I.err
	if $(TARGET) examples/count.ari --backend llvm >$(BUILD_DIR)/cli/backend-removed.err 2>&1; then exit 1; fi
	grep -q -- "--backend was removed; LLVM IR is the only host backend" $(BUILD_DIR)/cli/backend-removed.err
	if $(TARGET) examples/count.ari --cc >$(BUILD_DIR)/cli/missing-cc.err 2>&1; then exit 1; fi
	grep -q -- "--cc was removed; use --llvm-cc" $(BUILD_DIR)/cli/missing-cc.err
	if $(TARGET) examples/count.ari --llvm-cc >$(BUILD_DIR)/cli/missing-llvm-cc.err 2>&1; then exit 1; fi
	grep -q -- "--llvm-cc expects a compiler path" $(BUILD_DIR)/cli/missing-llvm-cc.err
	if $(TARGET) examples/count.ari --target >$(BUILD_DIR)/cli/missing-target.err 2>&1; then exit 1; fi
	grep -q -- "--target expects a target triple" $(BUILD_DIR)/cli/missing-target.err
	if $(TARGET) examples/count.ari --target mystery-cpu --emit-llvm $(BUILD_DIR)/cli/bad-target.ll >$(BUILD_DIR)/cli/bad-target.err 2>&1; then exit 1; fi
	grep -q "unsupported target triple 'mystery-cpu'" $(BUILD_DIR)/cli/bad-target.err
	if $(TARGET) --freestanding examples/count.ari --emit-llvm $(BUILD_DIR)/cli/bad.ll >$(BUILD_DIR)/cli/freestanding-emit-llvm.err 2>&1; then exit 1; fi
	grep -q -- "--freestanding cannot be combined with --emit-llvm or --shared" $(BUILD_DIR)/cli/freestanding-emit-llvm.err
	if $(TARGET) examples/count.ari extra.ari >$(BUILD_DIR)/cli/unexpected-arg.err 2>&1; then exit 1; fi
	grep -q "unexpected argument 'extra.ari'" $(BUILD_DIR)/cli/unexpected-arg.err

check-prelude: $(TARGET)
	mkdir -p $(BUILD_DIR)/prelude
	$(TARGET) --freestanding tests/ok/prelude-builtins.ari -o $(BUILD_DIR)/prelude/prelude-builtins.raw
	$(BUILD_DIR)/prelude/prelude-builtins.raw; code=$$?; test $$code -eq 7
	$(TARGET) --freestanding tests/ok/prelude-std-root-builtins.ari -o $(BUILD_DIR)/prelude/prelude-std-root-builtins.raw
	$(BUILD_DIR)/prelude/prelude-std-root-builtins.raw >$(BUILD_DIR)/prelude/prelude-std-root-builtins.out; code=$$?; test $$code -eq 9
	grep -qx "4" $(BUILD_DIR)/prelude/prelude-std-root-builtins.out
	$(TARGET) --freestanding tests/ok/prelude-macros.ari -o $(BUILD_DIR)/prelude/prelude-macros.raw
	$(BUILD_DIR)/prelude/prelude-macros.raw >$(BUILD_DIR)/prelude/prelude-macros.out; code=$$?; test $$code -eq 7
	grep -q "macro=5 ok=1" $(BUILD_DIR)/prelude/prelude-macros.out
	$(TARGET) tests/ok/prelude-macros.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-macros.ll
	grep -q "@ari_builtin_assert_eq_i64" $(BUILD_DIR)/prelude/prelude-macros.ll
	grep -q "@ari_builtin_assert_eq_bool" $(BUILD_DIR)/prelude/prelude-macros.ll
	$(TARGET) tests/ok/prelude-input.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-input.ll
	grep -q "@ari_builtin_read_byte" $(BUILD_DIR)/prelude/prelude-input.ll
	grep -q "@ari_builtin_context_arg" $(BUILD_DIR)/prelude/prelude-input.ll
	grep -q "declare i32 @getchar" $(BUILD_DIR)/prelude/prelude-input.ll
	$(TARGET) tests/ok/prelude-std-qualified-print.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	grep -q "call i64 @ari_builtin_write_i64(i64 1)" $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	grep -q "call i64 @ari_builtin_write_i64(i64 3)" $(BUILD_DIR)/prelude/prelude-std-qualified-print.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/prelude-std-qualified-print.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-std-qualified-print.elf; $(BUILD_DIR)/prelude/prelude-std-qualified-print.elf >$(BUILD_DIR)/prelude/prelude-std-qualified-print.out; code=$$?; test $$code -eq 6; grep -qx "std root 1" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; grep -qx "std io 2" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; grep -qx "std alias 3" $(BUILD_DIR)/prelude/prelude-std-qualified-print.out; fi
	$(TARGET) tests/ok/prelude-read-line.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-read-line.ll
	grep -q "@ari_builtin_read_line" $(BUILD_DIR)/prelude/prelude-read-line.ll
	grep -q "declare ptr @fgets" $(BUILD_DIR)/prelude/prelude-read-line.ll
	grep -q "@ari_line_buffer" $(BUILD_DIR)/prelude/prelude-read-line.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then \
		$(TARGET) tests/ok/prelude-read-line.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-read-line.elf; \
		printf 'alpha\nbeta\ngamma\ndelta\n' | $(BUILD_DIR)/prelude/prelude-read-line.elf >$(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "alpha" $(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "beta" $(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "gamma" $(BUILD_DIR)/prelude/prelude-read-line.out; \
		grep -qx "delta" $(BUILD_DIR)/prelude/prelude-read-line.out; \
	fi
	$(TARGET) --freestanding tests/ok/prelude-trait-surface.ari -o $(BUILD_DIR)/prelude/prelude-trait-surface.raw
	$(BUILD_DIR)/prelude/prelude-trait-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) tests/ok/prelude-header-main.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-header-main.ll
	grep -q 'call i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/prelude-header-main.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/prelude/prelude-header-main.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/prelude/prelude-header-main.ll
	if grep -q 'extern "C".*ari_builtin' lib/std.arih; then exit 1; fi
	$(TARGET) tests/ok/ari-builtin-extern.ari --emit-llvm $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	grep -q 'call i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	grep -q 'call i64 @"ari_builtin_newline"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll
	if grep -q 'declare i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll; then exit 1; fi
	if grep -q 'declare i64 @"ari_builtin_newline"' $(BUILD_DIR)/prelude/ari-builtin-extern.ll; then exit 1; fi
	$(TARGET) tests/ok/std-prelude-unqualified.ari --emit-llvm $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -q 'call i64 @"_ARNv10span_score"' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -q 'call ptr @"ari_builtin_zone_create"' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	grep -F -q 'getelementptr inbounds { i64, [3 x i64] }' $(BUILD_DIR)/prelude/std-prelude-unqualified.ll
	$(TARGET) tests/ok/std-prelude-local-shadow.ari --emit-llvm $(BUILD_DIR)/prelude/std-prelude-local-shadow.ll
	grep -q 'call i64 @"_ARNv5Debug"' $(BUILD_DIR)/prelude/std-prelude-local-shadow.ll
	$(TARGET) tests/ok/prelude-option-result.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-option-result.ll
	grep -q "try.ok" $(BUILD_DIR)/prelude/prelude-option-result.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/prelude/prelude-option-result.ll
	$(TARGET) tests/ok/prelude-slice-view.ari --emit-llvm $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q '_ARNv3std12slice__G_i64' $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "slice.bounds.fail" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "slice.range.bounds.fail" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "store i64 8" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "store i64 7" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	grep -q "store i64 5" $(BUILD_DIR)/prelude/prelude-slice-view.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/prelude-option-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-option-result.elf; $(BUILD_DIR)/prelude/prelude-option-result.elf; code=$$?; test $$code -eq 67; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/prelude-slice-view.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-slice-view.elf; $(BUILD_DIR)/prelude/prelude-slice-view.elf; code=$$?; test $$code -eq 101; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then \
		$(TARGET) tests/ok/prelude-header-main.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/prelude-header-main.elf; \
		$(BUILD_DIR)/prelude/prelude-header-main.elf >$(BUILD_DIR)/prelude/prelude-header-main.out; \
		code=$$?; test $$code -eq 21; \
		grep -qx "1" $(BUILD_DIR)/prelude/prelude-header-main.out; \
	fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/std-prelude-unqualified.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-prelude-unqualified.elf; $(BUILD_DIR)/prelude/std-prelude-unqualified.elf; code=$$?; test $$code -eq 39; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/std-prelude-local-shadow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/prelude/std-prelude-local-shadow.elf; $(BUILD_DIR)/prelude/std-prelude-local-shadow.elf; code=$$?; test $$code -eq 8; fi
	$(TARGET) --no-implicit-std --module-path lib tests/ok/prelude-header-explicit-std.ari --emit-module-metadata $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta --emit-llvm $(BUILD_DIR)/prelude/prelude-header-explicit-std.ll
	grep -q 'option	implicit_std	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'source	std	lib/std.arih	0' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'item	std::io	extern-ari-fn	std::io::write_i64	1' $(BUILD_DIR)/prelude/prelude-header-explicit-std.arimeta
	grep -q 'call i64 @"ari_builtin_write_i64"' $(BUILD_DIR)/prelude/prelude-header-explicit-std.ll
	if $(TARGET) --no-implicit-std tests/errors/std-use-without-implicit.ari -o $(BUILD_DIR)/prelude/std-use-without-implicit.elf >$(BUILD_DIR)/prelude/std-use-without-implicit.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'write_i64'" $(BUILD_DIR)/prelude/std-use-without-implicit.err
	if $(TARGET) --no-implicit-std tests/errors/prelude-bare-without-std.ari -o $(BUILD_DIR)/prelude/prelude-bare-without-std.elf >$(BUILD_DIR)/prelude/prelude-bare-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'write_i64'" $(BUILD_DIR)/prelude/prelude-bare-without-std.err
	if $(TARGET) --no-implicit-std tests/errors/prelude-path-without-std.ari -o $(BUILD_DIR)/prelude/prelude-path-without-std.elf >$(BUILD_DIR)/prelude/prelude-path-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'io::write_i64'" $(BUILD_DIR)/prelude/prelude-path-without-std.err
	if $(TARGET) tests/errors/prelude-maybe-removed.ari -o $(BUILD_DIR)/prelude/prelude-maybe-removed.elf >$(BUILD_DIR)/prelude/prelude-maybe-removed.err 2>&1; then exit 1; fi
	grep -q "unsupported executable type 'Maybe'" $(BUILD_DIR)/prelude/prelude-maybe-removed.err
	if $(TARGET) --no-implicit-std tests/errors/prelude-special-without-std.ari -o $(BUILD_DIR)/prelude/prelude-special-without-std.elf >$(BUILD_DIR)/prelude/prelude-special-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'size_of'" $(BUILD_DIR)/prelude/prelude-special-without-std.err
	if $(TARGET) --no-implicit-std --module-path tests/fixtures/std-missing-helper tests/errors/prelude-special-missing-decl.ari -o $(BUILD_DIR)/prelude/prelude-special-missing-decl.elf >$(BUILD_DIR)/prelude/prelude-special-missing-decl.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'std::mem::size_of'" $(BUILD_DIR)/prelude/prelude-special-missing-decl.err
	if $(TARGET) --no-implicit-std tests/errors/prelude-trait-without-std.ari -o $(BUILD_DIR)/prelude/prelude-trait-without-std.elf >$(BUILD_DIR)/prelude/prelude-trait-without-std.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'Debug'" $(BUILD_DIR)/prelude/prelude-trait-without-std.err
	if $(TARGET) tests/errors/ari-builtin-unknown.ari -o $(BUILD_DIR)/prelude/ari-builtin-unknown.elf >$(BUILD_DIR)/prelude/ari-builtin-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown Ari builtin symbol 'puts'" $(BUILD_DIR)/prelude/ari-builtin-unknown.err
	if $(TARGET) tests/errors/ari-builtin-missing-link.ari -o $(BUILD_DIR)/prelude/ari-builtin-missing-link.elf >$(BUILD_DIR)/prelude/ari-builtin-missing-link.err 2>&1; then exit 1; fi
	grep -q 'extern "ari" builtin declarations require an explicit builtin symbol' $(BUILD_DIR)/prelude/ari-builtin-missing-link.err
	if $(TARGET) tests/errors/prelude-result-arity.ari -o $(BUILD_DIR)/prelude/prelude-result-arity.elf >$(BUILD_DIR)/prelude/prelude-result-arity.err 2>&1; then exit 1; fi
	grep -q "enum 'std::Result' expects 2 type arguments" $(BUILD_DIR)/prelude/prelude-result-arity.err
	if $(TARGET) --freestanding tests/errors/prelude-read-line-freestanding.ari -o $(BUILD_DIR)/prelude/prelude-read-line-freestanding.elf >$(BUILD_DIR)/prelude/prelude-read-line-freestanding.err 2>&1; then exit 1; fi
	grep -q "freestanding backend does not lower line input helpers yet" $(BUILD_DIR)/prelude/prelude-read-line-freestanding.err
	if $(TARGET) tests/errors/prelude-macro-format-planned.ari -o $(BUILD_DIR)/prelude/prelude-macro-format-planned.elf >$(BUILD_DIR)/prelude/prelude-macro-format-planned.err 2>&1; then exit 1; fi
	grep -q "prelude macro 'format!' needs owned runtime strings" $(BUILD_DIR)/prelude/prelude-macro-format-planned.err
	if $(TARGET) tests/errors/prelude-macro-assert-eq-type.ari -o $(BUILD_DIR)/prelude/prelude-macro-assert-eq-type.elf >$(BUILD_DIR)/prelude/prelude-macro-assert-eq-type.err 2>&1; then exit 1; fi
	grep -q "assert_eq! arguments must both be integer values or both be bool values" $(BUILD_DIR)/prelude/prelude-macro-assert-eq-type.err
	if $(TARGET) tests/errors/prelude-assert-type.ari -o $(BUILD_DIR)/prelude/prelude-assert-type.elf >$(BUILD_DIR)/prelude/prelude-assert-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i64" $(BUILD_DIR)/prelude/prelude-assert-type.err

check-control-flow: $(TARGET)
	mkdir -p $(BUILD_DIR)/control-flow
	$(TARGET) --freestanding tests/ok/control-flow-if-else-if.ari -o $(BUILD_DIR)/control-flow/control-flow-if-else-if.raw
	$(BUILD_DIR)/control-flow/control-flow-if-else-if.raw; code=$$?; test $$code -eq 3
	$(TARGET) --freestanding tests/ok/control-flow-boolish.ari -o $(BUILD_DIR)/control-flow/control-flow-boolish.raw
	$(BUILD_DIR)/control-flow/control-flow-boolish.raw; code=$$?; test $$code -eq 10
	$(TARGET) --freestanding tests/ok/if-expression.ari -o $(BUILD_DIR)/control-flow/if-expression.raw
	$(BUILD_DIR)/control-flow/if-expression.raw; code=$$?; test $$code -eq 13
	$(TARGET) --freestanding tests/ok/if-let-expression.ari -o $(BUILD_DIR)/control-flow/if-let-expression.raw
	$(BUILD_DIR)/control-flow/if-let-expression.raw; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/if-let-or-pattern.ari -o $(BUILD_DIR)/control-flow/if-let-or-pattern.raw
	$(BUILD_DIR)/control-flow/if-let-or-pattern.raw; code=$$?; test $$code -eq 44
	$(TARGET) --freestanding tests/ok/if-let-aggregate-pattern.ari -o $(BUILD_DIR)/control-flow/if-let-aggregate-pattern.raw
	$(BUILD_DIR)/control-flow/if-let-aggregate-pattern.raw; code=$$?; test $$code -eq 17
	$(TARGET) --freestanding tests/ok/block-expression.ari -o $(BUILD_DIR)/control-flow/block-expression.raw
	$(BUILD_DIR)/control-flow/block-expression.raw; code=$$?; test $$code -eq 27
	$(TARGET) --freestanding tests/ok/labeled-block-expression.ari -o $(BUILD_DIR)/control-flow/labeled-block-expression.raw
	$(BUILD_DIR)/control-flow/labeled-block-expression.raw; code=$$?; test $$code -eq 17
	$(TARGET) --freestanding tests/ok/labeled-block-aggregate.ari -o $(BUILD_DIR)/control-flow/labeled-block-aggregate.raw
	$(BUILD_DIR)/control-flow/labeled-block-aggregate.raw; code=$$?; test $$code -eq 9
	$(TARGET) --freestanding tests/ok/control-flow-aggregate-expr-freestanding.ari -o $(BUILD_DIR)/control-flow/control-flow-aggregate-expr-freestanding.raw
	$(BUILD_DIR)/control-flow/control-flow-aggregate-expr-freestanding.raw; code=$$?; test $$code -eq 82
	$(TARGET) --freestanding tests/ok/for-range-operator.ari -o $(BUILD_DIR)/control-flow/for-range-operator.raw
	$(BUILD_DIR)/control-flow/for-range-operator.raw; code=$$?; test $$code -eq 10
	$(TARGET) --freestanding tests/ok/for-range-inclusive.ari -o $(BUILD_DIR)/control-flow/for-range-inclusive.raw
	$(BUILD_DIR)/control-flow/for-range-inclusive.raw; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/range-values.ari -o $(BUILD_DIR)/control-flow/range-values.raw
	$(BUILD_DIR)/control-flow/range-values.raw; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/range-generic-bounds.ari -o $(BUILD_DIR)/control-flow/range-generic-bounds.raw
	$(BUILD_DIR)/control-flow/range-generic-bounds.raw; code=$$?; test $$code -eq 32
	$(TARGET) --freestanding tests/ok/for-vector-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-pattern.raw
	$(BUILD_DIR)/control-flow/for-vector-pattern.raw; code=$$?; test $$code -eq 50
	$(TARGET) --freestanding tests/ok/for-vector-alias-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-alias-pattern.raw
	$(BUILD_DIR)/control-flow/for-vector-alias-pattern.raw; code=$$?; test $$code -eq 75
	$(TARGET) --freestanding tests/ok/if-while-let.ari -o $(BUILD_DIR)/control-flow/if-while-let.raw
	$(BUILD_DIR)/control-flow/if-while-let.raw; code=$$?; test $$code -eq 5
	$(TARGET) --freestanding tests/ok/while-let-or-pattern.ari -o $(BUILD_DIR)/control-flow/while-let-or-pattern.raw
	$(BUILD_DIR)/control-flow/while-let-or-pattern.raw; code=$$?; test $$code -eq 55
	$(TARGET) --freestanding tests/ok/let-while.ari -o $(BUILD_DIR)/control-flow/let-while.raw
	$(BUILD_DIR)/control-flow/let-while.raw; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/labeled-break.ari -o $(BUILD_DIR)/control-flow/labeled-break.raw
	$(BUILD_DIR)/control-flow/labeled-break.raw; code=$$?; test $$code -eq 2
	$(TARGET) --freestanding tests/ok/labeled-block.ari -o $(BUILD_DIR)/control-flow/labeled-block.raw
	$(BUILD_DIR)/control-flow/labeled-block.raw; code=$$?; test $$code -eq 2
	$(TARGET) tests/ok/if-while-let.ari --emit-llvm $(BUILD_DIR)/control-flow/if-while-let.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/if-while-let.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-while-let.ll
	$(TARGET) tests/ok/while-let-or-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/while-let-or-pattern.ll
	grep -q "whilelet.arm" $(BUILD_DIR)/control-flow/while-let-or-pattern.ll
	grep -q "whilelet.test" $(BUILD_DIR)/control-flow/while-let-or-pattern.ll
	$(TARGET) tests/ok/for-range-inclusive.ari --emit-llvm $(BUILD_DIR)/control-flow/for-range-inclusive.ll
	grep -q "icmp sle" $(BUILD_DIR)/control-flow/for-range-inclusive.ll
	$(TARGET) tests/ok/range-values.ari --emit-llvm $(BUILD_DIR)/control-flow/range-values.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/range-values.ll
	$(TARGET) tests/ok/range-generic-bounds.ari --emit-llvm $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ult i8" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ule i8" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ult i16" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	grep -q "icmp ult i64" $(BUILD_DIR)/control-flow/range-generic-bounds.ll
	$(TARGET) tests/ok/for-vector-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-pattern.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/for-vector-pattern.ll
	$(TARGET) tests/ok/for-vector-alias-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-alias-pattern.ll
	grep -q "extractvalue" $(BUILD_DIR)/control-flow/for-vector-alias-pattern.ll
	$(TARGET) tests/ok/for-vector-stored.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-stored.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/control-flow/for-vector-stored.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/control-flow/for-vector-stored.ll
	$(TARGET) tests/ok/for-vector-stored-alias-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-vector-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-pattern.elf; $(BUILD_DIR)/control-flow/for-vector-pattern.elf; code=$$?; test $$code -eq 50; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-vector-alias-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-alias-pattern.elf; $(BUILD_DIR)/control-flow/for-vector-alias-pattern.elf; code=$$?; test $$code -eq 75; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-vector-stored.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-stored.elf; $(BUILD_DIR)/control-flow/for-vector-stored.elf; code=$$?; test $$code -eq 20; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-vector-stored-alias-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.elf; $(BUILD_DIR)/control-flow/for-vector-stored-alias-pattern.elf; code=$$?; test $$code -eq 28; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/while-let-or-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/while-let-or-pattern.elf; $(BUILD_DIR)/control-flow/while-let-or-pattern.elf; code=$$?; test $$code -eq 55; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/if-let-or-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/if-let-or-pattern.elf; $(BUILD_DIR)/control-flow/if-let-or-pattern.elf; code=$$?; test $$code -eq 44; fi
	$(TARGET) tests/ok/labeled-block.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block.ll
	grep -q "block.end" $(BUILD_DIR)/control-flow/labeled-block.ll
	$(TARGET) tests/ok/if-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/if-expression.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-expression.ll
	grep -q "if.expr.end" $(BUILD_DIR)/control-flow/if-expression.ll
	$(TARGET) tests/ok/if-let-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/if-let-expression.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-let-expression.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-let-expression.ll
	grep -q "lshr" $(BUILD_DIR)/control-flow/if-let-expression.ll
	$(TARGET) tests/ok/if-let-or-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/if-let-or-pattern.ll
	grep -q "match.arm" $(BUILD_DIR)/control-flow/if-let-or-pattern.ll
	grep -q "if.expr.then" $(BUILD_DIR)/control-flow/if-let-or-pattern.ll
	$(TARGET) tests/ok/block-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/block-expression.ll
	grep -q "_ARNv4main" $(BUILD_DIR)/control-flow/block-expression.ll
	$(TARGET) tests/ok/labeled-block-expression.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block-expression.ll
	grep -q "block.expr.end" $(BUILD_DIR)/control-flow/labeled-block-expression.ll
	$(TARGET) tests/ok/labeled-block-aggregate.ari --emit-llvm $(BUILD_DIR)/control-flow/labeled-block-aggregate.ll
	grep -q "store { i64, i64 }" $(BUILD_DIR)/control-flow/labeled-block-aggregate.ll
	$(TARGET) tests/ok/for-iterator-direct.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-direct.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-direct.ll
	$(TARGET) tests/ok/for-generic-iterator-direct.ari --emit-llvm $(BUILD_DIR)/control-flow/for-generic-iterator-direct.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-generic-iterator-direct.ll
	$(TARGET) tests/ok/for-into-iterator-direct.ari --emit-llvm $(BUILD_DIR)/control-flow/for-into-iterator-direct.ll
	grep -q "into_iter" $(BUILD_DIR)/control-flow/for-into-iterator-direct.ll
	$(TARGET) tests/ok/for-into-iterator-result.ari --emit-llvm $(BUILD_DIR)/control-flow/for-into-iterator-result.ll
	grep -q "BagIter.*next" $(BUILD_DIR)/control-flow/for-into-iterator-result.ll
	$(TARGET) tests/ok/for-generic-into-iterator-result.ari --emit-llvm $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.ll
	grep -q "BagIter_i64.*next" $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.ll
	$(TARGET) tests/ok/for-into-iterator-mut-self.ari --emit-llvm $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.ll
	grep -q "into_iter" $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.ll
	$(TARGET) tests/ok/for-iterator-refutable-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.ll
	$(TARGET) tests/ok/for-iterator-filter-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.ll
	$(TARGET) tests/ok/for-iterator-payload-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.ll
	$(TARGET) tests/ok/for-iterator-aggregate-payload-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.ll
	grep -F -q "extractvalue { i32, { i32, i64 } }" $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.ll
	$(TARGET) tests/ok/for-iterator-nested-multi-payload-pattern.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.ll
	grep -F -q "extractvalue { i32, { i32, i64, i64 } }" $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.ll
	$(TARGET) tests/ok/for-iterator-mut-self.ari --emit-llvm $(BUILD_DIR)/control-flow/for-iterator-mut-self.ll
	grep -q "whilelet.cond" $(BUILD_DIR)/control-flow/for-iterator-mut-self.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-direct.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-direct.elf; $(BUILD_DIR)/control-flow/for-iterator-direct.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-generic-iterator-direct.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-generic-iterator-direct.elf; $(BUILD_DIR)/control-flow/for-generic-iterator-direct.elf; code=$$?; test $$code -eq 11; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-into-iterator-direct.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-into-iterator-direct.elf; $(BUILD_DIR)/control-flow/for-into-iterator-direct.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-into-iterator-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-into-iterator-result.elf; $(BUILD_DIR)/control-flow/for-into-iterator-result.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-generic-into-iterator-result.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.elf; $(BUILD_DIR)/control-flow/for-generic-into-iterator-result.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-into-iterator-mut-self.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.elf; $(BUILD_DIR)/control-flow/for-into-iterator-mut-self.elf; code=$$?; test $$code -eq 9; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-refutable-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-refutable-pattern.elf; code=$$?; test $$code -eq 52; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-filter-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-filter-pattern.elf; code=$$?; test $$code -eq 20; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-payload-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-payload-pattern.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-aggregate-payload-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-aggregate-payload-pattern.elf; code=$$?; test $$code -eq 21; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-nested-multi-payload-pattern.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.elf; $(BUILD_DIR)/control-flow/for-iterator-nested-multi-payload-pattern.elf; code=$$?; test $$code -eq 55; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/for-iterator-mut-self.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/control-flow/for-iterator-mut-self.elf; $(BUILD_DIR)/control-flow/for-iterator-mut-self.elf; code=$$?; test $$code -eq 15; fi
	if $(TARGET) tests/errors/for-iterator-planned.ari -o $(BUILD_DIR)/control-flow/for-iterator-planned.elf >$(BUILD_DIR)/control-flow/for-iterator-planned.err 2>&1; then exit 1; fi
	grep -q "copyable IntoIterator\\[T\\] values" $(BUILD_DIR)/control-flow/for-iterator-planned.err
	if $(TARGET) tests/errors/for-into-iterator-result-not-iterator.ari -o $(BUILD_DIR)/control-flow/for-into-iterator-result-not-iterator.elf >$(BUILD_DIR)/control-flow/for-into-iterator-result-not-iterator.err 2>&1; then exit 1; fi
	grep -q "method 'into_iter' for IntoIterator\\[i64\\] must return a type that implements Iterator\\[i64\\], got i64" $(BUILD_DIR)/control-flow/for-into-iterator-result-not-iterator.err
	if $(TARGET) tests/errors/for-generic-into-iterator-result-not-iterator.ari -o $(BUILD_DIR)/control-flow/for-generic-into-iterator-result-not-iterator.elf >$(BUILD_DIR)/control-flow/for-generic-into-iterator-result-not-iterator.err 2>&1; then exit 1; fi
	grep -q "method 'into_iter' for IntoIterator\\[T\\] must return a type that implements Iterator\\[T\\], got T" $(BUILD_DIR)/control-flow/for-generic-into-iterator-result-not-iterator.err
	if $(TARGET) tests/errors/for-pattern-planned.ari -o $(BUILD_DIR)/control-flow/for-pattern-planned.elf >$(BUILD_DIR)/control-flow/for-pattern-planned.err 2>&1; then exit 1; fi
	grep -q "for enum-case patterns are planned for Iterator\\[T\\] values" $(BUILD_DIR)/control-flow/for-pattern-planned.err
	if $(TARGET) tests/errors/for-vector-refutable-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-refutable-pattern.elf >$(BUILD_DIR)/control-flow/for-vector-refutable-pattern.err 2>&1; then exit 1; fi
	grep -q "refutable for-loop patterns require for-let filters on Iterator\\[T\\]" $(BUILD_DIR)/control-flow/for-vector-refutable-pattern.err
	if $(TARGET) tests/errors/for-let-filter-range.ari -o $(BUILD_DIR)/control-flow/for-let-filter-range.elf >$(BUILD_DIR)/control-flow/for-let-filter-range.err 2>&1; then exit 1; fi
	grep -q "for-let filters currently require an Iterator\\[T\\] or IntoIterator\\[T\\] value" $(BUILD_DIR)/control-flow/for-let-filter-range.err
	if $(TARGET) tests/errors/for-vector-alias-refutable-pattern.ari -o $(BUILD_DIR)/control-flow/for-vector-alias-refutable-pattern.elf >$(BUILD_DIR)/control-flow/for-vector-alias-refutable-pattern.err 2>&1; then exit 1; fi
	grep -q "refutable for-loop patterns require for-let filters on Iterator\\[T\\]" $(BUILD_DIR)/control-flow/for-vector-alias-refutable-pattern.err
	if $(TARGET) tests/errors/range-type-non-integer.ari -o $(BUILD_DIR)/control-flow/range-type-non-integer.elf >$(BUILD_DIR)/control-flow/range-type-non-integer.err 2>&1; then exit 1; fi
	grep -q "prelude range types require integer bounds" $(BUILD_DIR)/control-flow/range-type-non-integer.err
	if $(TARGET) tests/errors/break-unknown-label.ari -o $(BUILD_DIR)/control-flow/break-unknown-label.elf >$(BUILD_DIR)/control-flow/break-unknown-label.err 2>&1; then exit 1; fi
	grep -q "unknown loop label 'missing'" $(BUILD_DIR)/control-flow/break-unknown-label.err
	if $(TARGET) tests/errors/duplicate-loop-label.ari -o $(BUILD_DIR)/control-flow/duplicate-loop-label.elf >$(BUILD_DIR)/control-flow/duplicate-loop-label.err 2>&1; then exit 1; fi
	grep -q "duplicate active loop label 'loop'" $(BUILD_DIR)/control-flow/duplicate-loop-label.err
	if $(TARGET) tests/errors/if-expression-type-mismatch.ari -o $(BUILD_DIR)/control-flow/if-expression-type-mismatch.elf >$(BUILD_DIR)/control-flow/if-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/control-flow/if-expression-type-mismatch.err
	if $(TARGET) tests/errors/if-let-expression-type-mismatch.ari -o $(BUILD_DIR)/control-flow/if-let-expression-type-mismatch.elf >$(BUILD_DIR)/control-flow/if-let-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/control-flow/if-let-expression-type-mismatch.err
	if $(TARGET) tests/errors/if-let-aggregate-irrefutable-else.ari -o $(BUILD_DIR)/control-flow/if-let-aggregate-irrefutable-else.elf >$(BUILD_DIR)/control-flow/if-let-aggregate-irrefutable-else.err 2>&1; then exit 1; fi
	grep -q "irrefutable if-let aggregate pattern cannot have else" $(BUILD_DIR)/control-flow/if-let-aggregate-irrefutable-else.err
	if $(TARGET) tests/errors/labeled-block-break-type.ari -o $(BUILD_DIR)/control-flow/labeled-block-break-type.elf >$(BUILD_DIR)/control-flow/labeled-block-break-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i64" $(BUILD_DIR)/control-flow/labeled-block-break-type.err
	if $(TARGET) tests/errors/labeled-block-ownership-state.ari -o $(BUILD_DIR)/control-flow/labeled-block-ownership-state.elf >$(BUILD_DIR)/control-flow/labeled-block-ownership-state.err 2>&1; then exit 1; fi
	grep -q "binding 'token' has incompatible ownership states after labeled block exits" $(BUILD_DIR)/control-flow/labeled-block-ownership-state.err
	if $(TARGET) tests/errors/block-expression-final-semicolon.ari -o $(BUILD_DIR)/control-flow/block-expression-final-semicolon.elf >$(BUILD_DIR)/control-flow/block-expression-final-semicolon.err 2>&1; then exit 1; fi
	grep -q "block expression must end with a value" $(BUILD_DIR)/control-flow/block-expression-final-semicolon.err
	if $(TARGET) tests/errors/block-expression-nonfinal-missing-semicolon.ari -o $(BUILD_DIR)/control-flow/block-expression-nonfinal-missing-semicolon.elf >$(BUILD_DIR)/control-flow/block-expression-nonfinal-missing-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected ; after non-final expression statement" $(BUILD_DIR)/control-flow/block-expression-nonfinal-missing-semicolon.err
	if $(TARGET) tests/errors/nonblock-statement-missing-semicolon.ari -o $(BUILD_DIR)/control-flow/nonblock-statement-missing-semicolon.elf >$(BUILD_DIR)/control-flow/nonblock-statement-missing-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected ; after binding" $(BUILD_DIR)/control-flow/nonblock-statement-missing-semicolon.err

check-match: $(TARGET)
	mkdir -p $(BUILD_DIR)/match
	$(TARGET) --freestanding tests/ok/enum-match.ari -o $(BUILD_DIR)/match/enum-match.raw
	$(BUILD_DIR)/match/enum-match.raw; code=$$?; test $$code -eq 10
	$(TARGET) --freestanding tests/ok/match-patterns.ari -o $(BUILD_DIR)/match/match-patterns.raw
	$(BUILD_DIR)/match/match-patterns.raw; code=$$?; test $$code -eq 34
	$(TARGET) --freestanding tests/ok/match-wildcard.ari -o $(BUILD_DIR)/match/match-wildcard.raw
	$(BUILD_DIR)/match/match-wildcard.raw; code=$$?; test $$code -eq 18
	$(TARGET) --freestanding tests/ok/match-return-flow.ari -o $(BUILD_DIR)/match/match-return-flow.raw
	$(BUILD_DIR)/match/match-return-flow.raw; code=$$?; test $$code -eq 18
	$(TARGET) --freestanding tests/ok/match-freestanding.ari -o $(BUILD_DIR)/match/match-freestanding.raw
	$(BUILD_DIR)/match/match-freestanding.raw; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/match-expression.ari -o $(BUILD_DIR)/match/match-expression.raw
	$(BUILD_DIR)/match/match-expression.raw; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/match-literals.ari -o $(BUILD_DIR)/match/match-literals.raw
	$(BUILD_DIR)/match/match-literals.raw; code=$$?; test $$code -eq 128
	$(TARGET) --freestanding tests/ok/match-tuple-pattern.ari -o $(BUILD_DIR)/match/match-tuple-pattern.raw
	$(BUILD_DIR)/match/match-tuple-pattern.raw; code=$$?; test $$code -eq 32
	$(TARGET) --freestanding tests/ok/match-struct-pattern.ari -o $(BUILD_DIR)/match/match-struct-pattern.raw
	$(BUILD_DIR)/match/match-struct-pattern.raw; code=$$?; test $$code -eq 21
	$(TARGET) --freestanding tests/ok/match-product-exhaustive.ari -o $(BUILD_DIR)/match/match-product-exhaustive.raw
	$(BUILD_DIR)/match/match-product-exhaustive.raw; code=$$?; test $$code -eq 17
	$(TARGET) --freestanding tests/ok/match-symbolic-product.ari -o $(BUILD_DIR)/match/match-symbolic-product.raw
	$(BUILD_DIR)/match/match-symbolic-product.raw; code=$$?; test $$code -eq 5
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/match-aggregate-enum-payload-literals.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.elf; $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.elf; code=$$?; test $$code -eq 64; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/match-aggregate-enum-nested-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.elf; $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.elf; code=$$?; test $$code -eq 99; fi
	$(TARGET) --freestanding tests/ok/match-or-binding.ari -o $(BUILD_DIR)/match/match-or-binding.raw
	$(BUILD_DIR)/match/match-or-binding.raw; code=$$?; test $$code -eq 41
	$(TARGET) --freestanding tests/ok/match-range-overlap.ari -o $(BUILD_DIR)/match/match-range-overlap.raw
	$(BUILD_DIR)/match/match-range-overlap.raw; code=$$?; test $$code -eq 1
	$(TARGET) --freestanding tests/ok/match-shadow-warning.ari -o $(BUILD_DIR)/match/match-shadow-warning.raw >$(BUILD_DIR)/match/match-shadow-warning.warn 2>&1
	grep -q "warning: scalar match pattern is fully shadowed by earlier match arms" $(BUILD_DIR)/match/match-shadow-warning.warn
	$(BUILD_DIR)/match/match-shadow-warning.raw; code=$$?; test $$code -eq 13
	$(TARGET) --freestanding tests/ok/match-product-shadow-warning.ari -o $(BUILD_DIR)/match/match-product-shadow-warning.raw >$(BUILD_DIR)/match/match-product-shadow-warning.warn 2>&1
	grep -q "warning: aggregate match pattern is fully shadowed by earlier match arms" $(BUILD_DIR)/match/match-product-shadow-warning.warn
	$(BUILD_DIR)/match/match-product-shadow-warning.raw; code=$$?; test $$code -eq 1
	$(TARGET) --freestanding tests/ok/match-alias-pattern.ari -o $(BUILD_DIR)/match/match-alias-pattern.raw
	$(BUILD_DIR)/match/match-alias-pattern.raw; code=$$?; test $$code -eq 28
	$(TARGET) --freestanding tests/ok/match-alias-or-pattern.ari -o $(BUILD_DIR)/match/match-alias-or-pattern.raw
	$(BUILD_DIR)/match/match-alias-or-pattern.raw; code=$$?; test $$code -eq 66
	$(TARGET) --freestanding tests/ok/match-enum-payload-literals.ari -o $(BUILD_DIR)/match/match-enum-payload-literals.raw
	$(BUILD_DIR)/match/match-enum-payload-literals.raw; code=$$?; test $$code -eq 66
	$(TARGET) --freestanding tests/ok/constant-patterns.ari -o $(BUILD_DIR)/match/constant-patterns.raw
	$(BUILD_DIR)/match/constant-patterns.raw; code=$$?; test $$code -eq 61
	$(TARGET) --freestanding tests/ok/constant-folding.ari -o $(BUILD_DIR)/match/constant-folding.raw
	$(BUILD_DIR)/match/constant-folding.raw; code=$$?; test $$code -eq 25
	$(TARGET) --freestanding tests/ok/constant-aggregates.ari -o $(BUILD_DIR)/match/constant-aggregates.raw
	$(BUILD_DIR)/match/constant-aggregates.raw; code=$$?; test $$code -eq 34
	$(TARGET) --freestanding tests/ok/array-patterns.ari -o $(BUILD_DIR)/match/array-patterns.raw
	$(BUILD_DIR)/match/array-patterns.raw; code=$$?; test $$code -eq 25
	$(TARGET) tests/ok/match-patterns.ari --emit-llvm $(BUILD_DIR)/match/match-patterns.ll
	grep -q "match.arm" $(BUILD_DIR)/match/match-patterns.ll
	grep -q "lshr" $(BUILD_DIR)/match/match-patterns.ll
	$(TARGET) tests/ok/match-expression.ari --emit-llvm $(BUILD_DIR)/match/match-expression.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/match-expression.ll
	grep -q "phi i64" $(BUILD_DIR)/match/match-expression.ll
	$(TARGET) tests/ok/match-literals.ari --emit-llvm $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp eq i1" $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp sge i64" $(BUILD_DIR)/match/match-literals.ll
	grep -q "icmp slt i64" $(BUILD_DIR)/match/match-literals.ll
	$(TARGET) tests/ok/match-alias-pattern.ari --emit-llvm $(BUILD_DIR)/match/match-alias-pattern.ll
	grep -q "store i64" $(BUILD_DIR)/match/match-alias-pattern.ll
	grep -q "store i8" $(BUILD_DIR)/match/match-alias-pattern.ll
	$(TARGET) tests/ok/match-alias-or-pattern.ari --emit-llvm $(BUILD_DIR)/match/match-alias-or-pattern.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/match/match-alias-or-pattern.ll
	grep -q "store { i64, i1 }" $(BUILD_DIR)/match/match-alias-or-pattern.ll
	$(TARGET) tests/ok/match-enum-payload-literals.ari --emit-llvm $(BUILD_DIR)/match/match-enum-payload-literals.ll
	grep -q "4294967297" $(BUILD_DIR)/match/match-enum-payload-literals.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/match/match-enum-payload-literals.ll
	grep -q "icmp sle i32" $(BUILD_DIR)/match/match-enum-payload-literals.ll
	$(TARGET) tests/ok/enum-multi-payload.ari --emit-llvm $(BUILD_DIR)/match/enum-multi-payload.ll
	grep -q "insertvalue { i32, i64, i64, i64 }" $(BUILD_DIR)/match/enum-multi-payload.ll
	grep -q "extractvalue { i32, i64, i64, i64 }" $(BUILD_DIR)/match/enum-multi-payload.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/enum-multi-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-multi-payload.elf; $(BUILD_DIR)/match/enum-multi-payload.elf; code=$$?; test $$code -eq 33; fi
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-enum-local.ari -o $(BUILD_DIR)/match/freestanding-aggregate-enum-local.raw
	$(BUILD_DIR)/match/freestanding-aggregate-enum-local.raw; code=$$?; test $$code -eq 30
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-enum-payload-tests.ari -o $(BUILD_DIR)/match/freestanding-aggregate-enum-payload-tests.raw
	$(BUILD_DIR)/match/freestanding-aggregate-enum-payload-tests.raw; code=$$?; test $$code -eq 73
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-enum-nested-payload.ari -o $(BUILD_DIR)/match/freestanding-aggregate-enum-nested-payload.raw
	$(BUILD_DIR)/match/freestanding-aggregate-enum-nested-payload.raw; code=$$?; test $$code -eq 99
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-enum-nested-aggregate-payload.ari -o $(BUILD_DIR)/match/freestanding-aggregate-enum-nested-aggregate-payload.raw
	$(BUILD_DIR)/match/freestanding-aggregate-enum-nested-aggregate-payload.raw; code=$$?; test $$code -eq 49
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-enum-pointer-copy.ari -o $(BUILD_DIR)/match/freestanding-aggregate-enum-pointer-copy.raw
	$(BUILD_DIR)/match/freestanding-aggregate-enum-pointer-copy.raw; code=$$?; test $$code -eq 21
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-enum-direct-store.ari -o $(BUILD_DIR)/match/freestanding-aggregate-enum-direct-store.raw
	$(BUILD_DIR)/match/freestanding-aggregate-enum-direct-store.raw; code=$$?; test $$code -eq 23
	$(TARGET) tests/ok/enum-pointer-shaped-payloads.ari --emit-llvm $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	grep -q "ptrtoint ptr" $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	grep -q "inttoptr i64" $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	grep -q "call i64 %" $(BUILD_DIR)/match/enum-pointer-shaped-payloads.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/enum-pointer-shaped-payloads.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/enum-pointer-shaped-payloads.elf; $(BUILD_DIR)/match/enum-pointer-shaped-payloads.elf; code=$$?; test $$code -eq 25; fi
	$(TARGET) --freestanding tests/ok/enum-multi-payload.ari -o $(BUILD_DIR)/match/enum-multi-payload.raw
	$(BUILD_DIR)/match/enum-multi-payload.raw; code=$$?; test $$code -eq 33
	$(TARGET) tests/ok/constant-patterns.ari --emit-llvm $(BUILD_DIR)/match/constant-patterns.ll
	grep -q "30064771072" $(BUILD_DIR)/match/constant-patterns.ll
	grep -q "icmp eq i1" $(BUILD_DIR)/match/constant-patterns.ll
	$(TARGET) tests/ok/constant-folding.ari --emit-llvm $(BUILD_DIR)/match/constant-folding.ll
	grep -q "br i1 1" $(BUILD_DIR)/match/constant-folding.ll
	$(TARGET) tests/ok/constant-aggregates.ari --emit-llvm $(BUILD_DIR)/match/constant-aggregates.ll
	grep -q "insertvalue { i64, i64 }" $(BUILD_DIR)/match/constant-aggregates.ll
	grep -q "insertvalue \\[3 x i64\\]" $(BUILD_DIR)/match/constant-aggregates.ll
	$(TARGET) tests/ok/array-patterns.ari --emit-llvm $(BUILD_DIR)/match/array-patterns.ll
	grep -q "extractvalue \\[4 x i64\\]" $(BUILD_DIR)/match/array-patterns.ll
	$(TARGET) tests/ok/match-symbolic-product.ari --emit-llvm $(BUILD_DIR)/match/match-symbolic-product.ll
	grep -q "icmp ule i16" $(BUILD_DIR)/match/match-symbolic-product.ll
	$(TARGET) tests/ok/match-aggregate-enum-payload-literals.ari --emit-llvm $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	grep -q "icmp sle i64" $(BUILD_DIR)/match/match-aggregate-enum-payload-literals.ll
	$(TARGET) tests/ok/match-aggregate-enum-nested-payload.ari --emit-llvm $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	grep -q "lshr i64" $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	grep -q "icmp sle i32" $(BUILD_DIR)/match/match-aggregate-enum-nested-payload.ll
	$(TARGET) tests/ok/match-aggregate-enum-nested-aggregate-payload.ari --emit-llvm $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.ll
	grep -F -q "insertvalue { i32, { i32, i64 } }" $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.ll
	grep -F -q "extractvalue { i32, { i32, i64 } }" $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/match-aggregate-enum-nested-aggregate-payload.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.elf; $(BUILD_DIR)/match/match-aggregate-enum-nested-aggregate-payload.elf; code=$$?; test $$code -eq 31; fi
	if $(TARGET) tests/errors/match-expression-type-mismatch.ari -o $(BUILD_DIR)/match/match-expression-type-mismatch.elf >$(BUILD_DIR)/match/match-expression-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/match/match-expression-type-mismatch.err
	if $(TARGET) tests/errors/match-literal-nonexhaustive.ari -o $(BUILD_DIR)/match/match-literal-nonexhaustive.elf >$(BUILD_DIR)/match/match-literal-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "integer match must include a wildcard arm" $(BUILD_DIR)/match/match-literal-nonexhaustive.err
	if $(TARGET) tests/errors/match-literal-type.ari -o $(BUILD_DIR)/match/match-literal-type.elf >$(BUILD_DIR)/match/match-literal-type.err 2>&1; then exit 1; fi
	grep -q "integer match patterns must be integer literals or _" $(BUILD_DIR)/match/match-literal-type.err
	if $(TARGET) tests/errors/match-range-pattern-order.ari -o $(BUILD_DIR)/match/match-range-pattern-order.elf >$(BUILD_DIR)/match/match-range-pattern-order.err 2>&1; then exit 1; fi
	grep -q "range pattern start must be <= end" $(BUILD_DIR)/match/match-range-pattern-order.err
	if $(TARGET) tests/errors/match-range-pattern-nonexhaustive.ari -o $(BUILD_DIR)/match/match-range-pattern-nonexhaustive.elf >$(BUILD_DIR)/match/match-range-pattern-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "integer match must include a wildcard arm" $(BUILD_DIR)/match/match-range-pattern-nonexhaustive.err
	if $(TARGET) tests/errors/match-or-pattern-binding.ari -o $(BUILD_DIR)/match/match-or-pattern-binding.elf >$(BUILD_DIR)/match/match-or-pattern-binding.err 2>&1; then exit 1; fi
	grep -q "or-pattern alternatives must bind the same names" $(BUILD_DIR)/match/match-or-pattern-binding.err
	if $(TARGET) tests/errors/match-or-pattern-binding-type.ari -o $(BUILD_DIR)/match/match-or-pattern-binding-type.elf >$(BUILD_DIR)/match/match-or-pattern-binding-type.err 2>&1; then exit 1; fi
	grep -q "or-pattern binding 'value' has inconsistent types: i32 and u32" $(BUILD_DIR)/match/match-or-pattern-binding-type.err
	if $(TARGET) tests/errors/match-tuple-nonexhaustive.ari -o $(BUILD_DIR)/match/match-tuple-nonexhaustive.elf >$(BUILD_DIR)/match/match-tuple-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "tuple match must cover every supported product case" $(BUILD_DIR)/match/match-tuple-nonexhaustive.err
	if $(TARGET) tests/errors/match-product-nonexhaustive-bool.ari -o $(BUILD_DIR)/match/match-product-nonexhaustive-bool.elf >$(BUILD_DIR)/match/match-product-nonexhaustive-bool.err 2>&1; then exit 1; fi
	grep -q "tuple match must cover every supported product case" $(BUILD_DIR)/match/match-product-nonexhaustive-bool.err
	if $(TARGET) tests/errors/match-symbolic-product-missing.ari -o $(BUILD_DIR)/match/match-symbolic-product-missing.elf >$(BUILD_DIR)/match/match-symbolic-product-missing.err 2>&1; then exit 1; fi
	grep -q "missing case such as (_, true)" $(BUILD_DIR)/match/match-symbolic-product-missing.err
	if $(TARGET) tests/errors/match-struct-pattern-missing-field.ari -o $(BUILD_DIR)/match/match-struct-pattern-missing-field.elf >$(BUILD_DIR)/match/match-struct-pattern-missing-field.err 2>&1; then exit 1; fi
	grep -q "struct match pattern must mention all fields or use '..'" $(BUILD_DIR)/match/match-struct-pattern-missing-field.err
	if $(TARGET) tests/errors/match-enum-payload-literal-nonexhaustive.ari -o $(BUILD_DIR)/match/match-enum-payload-literal-nonexhaustive.elf >$(BUILD_DIR)/match/match-enum-payload-literal-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "match must cover all cases of enum 'OptionI32'" $(BUILD_DIR)/match/match-enum-payload-literal-nonexhaustive.err
	if $(TARGET) tests/errors/match-enum-bool-payload-literal-nonexhaustive.ari -o $(BUILD_DIR)/match/match-enum-bool-payload-literal-nonexhaustive.elf >$(BUILD_DIR)/match/match-enum-bool-payload-literal-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "match must cover all cases of enum 'MaybeBool'" $(BUILD_DIR)/match/match-enum-bool-payload-literal-nonexhaustive.err
	if $(TARGET) tests/errors/match-enum-payload-literal-type.ari -o $(BUILD_DIR)/match/match-enum-payload-literal-type.elf >$(BUILD_DIR)/match/match-enum-payload-literal-type.err 2>&1; then exit 1; fi
	grep -q "bool payload patterns require a bool enum payload" $(BUILD_DIR)/match/match-enum-payload-literal-type.err
	if $(TARGET) tests/errors/match-enum-payload-duplicate-literal.ari -o $(BUILD_DIR)/match/match-enum-payload-duplicate-literal.elf >$(BUILD_DIR)/match/match-enum-payload-duplicate-literal.err 2>&1; then exit 1; fi
	grep -q "duplicate match arm for enum payload pattern 'Some'" $(BUILD_DIR)/match/match-enum-payload-duplicate-literal.err
	if $(TARGET) tests/errors/constant-pattern-type.ari -o $(BUILD_DIR)/match/constant-pattern-type.elf >$(BUILD_DIR)/match/constant-pattern-type.err 2>&1; then exit 1; fi
	grep -q "integer match constant pattern must have an integer type" $(BUILD_DIR)/match/constant-pattern-type.err
	if $(TARGET) tests/errors/constant-folding-div-zero.ari -o $(BUILD_DIR)/match/constant-folding-div-zero.elf >$(BUILD_DIR)/match/constant-folding-div-zero.err 2>&1; then exit 1; fi
	grep -q "constant expression divides by zero" $(BUILD_DIR)/match/constant-folding-div-zero.err
	if $(TARGET) tests/errors/constant-cycle-path.ari -o $(BUILD_DIR)/match/constant-cycle-path.elf >$(BUILD_DIR)/match/constant-cycle-path.err 2>&1; then exit 1; fi
	grep -q "constant cycle: A -> B -> C -> A" $(BUILD_DIR)/match/constant-cycle-path.err
	if $(TARGET) tests/errors/constant-aggregate-array-arity.ari -o $(BUILD_DIR)/match/constant-aggregate-array-arity.elf >$(BUILD_DIR)/match/constant-aggregate-array-arity.err 2>&1; then exit 1; fi
	grep -q "array constant has 1 element but type expects 2" $(BUILD_DIR)/match/constant-aggregate-array-arity.err
	if $(TARGET) tests/errors/array-pattern-arity.ari -o $(BUILD_DIR)/match/array-pattern-arity.elf >$(BUILD_DIR)/match/array-pattern-arity.err 2>&1; then exit 1; fi
	grep -q "array match pattern has 2 elements but value has 3" $(BUILD_DIR)/match/array-pattern-arity.err
	if $(TARGET) tests/errors/enum-nested-aggregate-payload-mixed-slot.ari -o $(BUILD_DIR)/match/enum-nested-aggregate-payload-mixed-slot.elf >$(BUILD_DIR)/match/enum-nested-aggregate-payload-mixed-slot.err 2>&1; then exit 1; fi
	grep -q "mixed nested aggregate enum payload slots need an explicit ABI rule" $(BUILD_DIR)/match/enum-nested-aggregate-payload-mixed-slot.err

check-modules: $(TARGET)
	mkdir -p $(BUILD_DIR)/modules
	$(TARGET) --freestanding tests/ok/modules.ari -o $(BUILD_DIR)/modules/modules.raw
	$(BUILD_DIR)/modules/modules.raw; code=$$?; test $$code -eq 13
	$(TARGET) --freestanding tests/ok/module-nested-alias.ari -o $(BUILD_DIR)/modules/module-nested-alias.raw
	$(BUILD_DIR)/modules/module-nested-alias.raw; code=$$?; test $$code -eq 29
	$(TARGET) --freestanding tests/ok/module-use-list.ari -o $(BUILD_DIR)/modules/module-use-list.raw
	$(BUILD_DIR)/modules/module-use-list.raw; code=$$?; test $$code -eq 15
	$(TARGET) --freestanding tests/ok/module-use-glob.ari -o $(BUILD_DIR)/modules/module-use-glob.raw
	$(BUILD_DIR)/modules/module-use-glob.raw; code=$$?; test $$code -eq 7
	$(TARGET) --freestanding tests/ok/module-use-alias-prefix.ari -o $(BUILD_DIR)/modules/module-use-alias-prefix.raw
	$(BUILD_DIR)/modules/module-use-alias-prefix.raw; code=$$?; test $$code -eq 8
	$(TARGET) --freestanding tests/ok/module-relative-paths.ari -o $(BUILD_DIR)/modules/module-relative-paths.raw
	$(BUILD_DIR)/modules/module-relative-paths.raw; code=$$?; test $$code -eq 16
	$(TARGET) --freestanding tests/ok/module-pub-reexport.ari -o $(BUILD_DIR)/modules/module-pub-reexport.raw
	$(BUILD_DIR)/modules/module-pub-reexport.raw; code=$$?; test $$code -eq 16
	$(TARGET) --freestanding tests/ok/module-pub-glob-reexport.ari -o $(BUILD_DIR)/modules/module-pub-glob-reexport.raw
	$(BUILD_DIR)/modules/module-pub-glob-reexport.raw; code=$$?; test $$code -eq 5
	$(TARGET) --freestanding tests/ok/file-module-alias-main.ari -o $(BUILD_DIR)/modules/file-module-alias-main.raw
	$(BUILD_DIR)/modules/file-module-alias-main.raw; code=$$?; test $$code -eq 6
	$(TARGET) --freestanding --module-path tests/packages tests/ok/package-module-main.ari -o $(BUILD_DIR)/modules/package-module-main.raw
	$(BUILD_DIR)/modules/package-module-main.raw; code=$$?; test $$code -eq 31
	$(TARGET) -Itests/packages tests/ok/package-module-main.ari --emit-llvm $(BUILD_DIR)/modules/package-module-main.ll
	grep -q '_ARNv12package_math5value' $(BUILD_DIR)/modules/package-module-main.ll
	$(TARGET) -Itests/packages tests/ok/package-module-main.ari --emit-module-metadata $(BUILD_DIR)/modules/package-module-main.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-metadata.ll
	grep -q "ari-module-metadata-v2" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -q "source.*package_math.*tests/packages/package_math.ari" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -Eq "source.*package_math.*tests/packages/package_math.ari.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -q "import.*package_math::nested.*tests/packages/nested/mod.ari" $(BUILD_DIR)/modules/package-module-main.arimeta
	grep -q "item.*fn.*package_math::value" $(BUILD_DIR)/modules/package-module-main.arimeta
	$(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-metadata-check.ll
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "import") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-duplicate-import.arimeta
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-duplicate-import.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-duplicate-import.ll >$(BUILD_DIR)/modules/package-module-main-duplicate-import.err 2>&1; then exit 1; fi
	grep -q "duplicate import record" $(BUILD_DIR)/modules/package-module-main-duplicate-import.err
	$(TARGET) -Itests/packages tests/ok/package-module-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-module-main.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache.ll
	grep -q "ari-module-cache-v5" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -q "metadata.*ari-module-metadata-v2" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -q "source.*package_math.*tests/packages/package_math.ari" $(BUILD_DIR)/modules/package-module-main.aricache
	grep -Eq "ast-summary.*package_math.*tests/packages/package_math.ari.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-module-main.aricache
	$(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-module-main-cache.ll $(BUILD_DIR)/modules/package-module-main-cache-use.ll
	$(TARGET) -Itests/packages tests/ok/package-cache-decls-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-decls.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-decls-cache.ll
	grep -q "ari-ast-decls-v2" $(BUILD_DIR)/modules/package-cache-decls.aricache
	grep -Eq "ast-summary.*cache_decls.*tests/packages/cache_decls.ari.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-decls.aricache
	$(TARGET) -Itests/packages tests/ok/package-cache-decls-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-decls.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-decls-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-decls-cache.ll $(BUILD_DIR)/modules/package-cache-decls-cache-use.ll
	$(TARGET) -Itests/packages tests/ok/package-cache-const-header-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-const-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-header-cache.ll
	grep -q "ari-ast-decls-v2" $(BUILD_DIR)/modules/package-cache-const-header.aricache
	grep -Eq "ast-summary.*cache_consts.*tests/packages/cache_consts.arih.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-const-header.aricache
	$(TARGET) -Itests/packages tests/ok/package-cache-const-header-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-const-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-header-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-const-header-cache.ll $(BUILD_DIR)/modules/package-cache-const-header-cache-use.ll
	$(TARGET) -Itests/packages tests/ok/package-cache-const-aggregate-header-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache.ll
	grep -q "ari-ast-decls-v2" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -Eq "ast-summary.*cache_const_aggregates.*tests/packages/cache_const_aggregates.arih.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "5:tuple" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "6:struct" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "6:vector" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	grep -q "4:call" $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache
	$(TARGET) -Itests/packages tests/ok/package-cache-const-aggregate-header-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-const-aggregate-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache.ll $(BUILD_DIR)/modules/package-cache-const-aggregate-header-cache-use.ll
	$(TARGET) -Itests/packages tests/ok/package-cache-extern-header-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-cache-extern-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-extern-header-cache.ll
	grep -Eq "ast-summary.*extern_header.*tests/packages/extern_header.arih.*[0-9a-f]{16}" $(BUILD_DIR)/modules/package-cache-extern-header.aricache
	$(TARGET) -Itests/packages tests/ok/package-cache-extern-header-main.ari --use-module-cache $(BUILD_DIR)/modules/package-cache-extern-header.aricache --emit-llvm $(BUILD_DIR)/modules/package-cache-extern-header-cache-use.ll
	cmp -s $(BUILD_DIR)/modules/package-cache-extern-header-cache.ll $(BUILD_DIR)/modules/package-cache-extern-header-cache-use.ll
	awk 'NR == 1 { print "ari-module-cache-v1"; next } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-v1.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-v1.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-v1.ll >$(BUILD_DIR)/modules/package-module-main-cache-v1.err 2>&1; then exit 1; fi
	grep -q "old module cache format" $(BUILD_DIR)/modules/package-module-main-cache-v1.err
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "source") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.ll >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.err 2>&1; then exit 1; fi
	grep -q "duplicate source record" $(BUILD_DIR)/modules/package-module-main-cache-duplicate-source.err
	awk 'BEGIN { done = 0 } { print; if (!done && $$1 == "ast-summary") { print; done = 1 } }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.ll >$(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.err 2>&1; then exit 1; fi
	grep -q "duplicate ast-summary record" $(BUILD_DIR)/modules/package-module-main-cache-duplicate-summary.err
	sed '/ast-summary.*package_math.*tests\/packages\/package_math.ari/d' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-missing-summary.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-missing-summary.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-missing-summary.ll >$(BUILD_DIR)/modules/package-module-main-cache-missing-summary.err 2>&1; then exit 1; fi
	grep -q "missing AST summary" $(BUILD_DIR)/modules/package-module-main-cache-missing-summary.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ast-summary" && $$2 == "package_math" { $$12 = "99" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.err 2>&1; then exit 1; fi
	grep -q "has function count 99 but declaration summary contains" $(BUILD_DIR)/modules/package-module-main-cache-summary-tampered.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ast-summary" && $$2 == "package_math" { $$6 = "0000000000000000" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.err 2>&1; then exit 1; fi
	grep -q "declaration summary hashes to" $(BUILD_DIR)/modules/package-module-main-cache-summary-hash-tampered.err
	awk 'BEGIN { FS=OFS="\t" } $$1 == "ast-summary" && $$2 == "package_math" { $$7 = "tampered" } { print }' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.err 2>&1; then exit 1; fi
	grep -q "declaration summary hashes to" $(BUILD_DIR)/modules/package-module-main-cache-summary-payload-tampered.err
	sed '0,/package_math::value/s/package_math::value/package_math::wrong/' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-tampered.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-tampered.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-tampered.ll >$(BUILD_DIR)/modules/package-module-main-cache-tampered.err 2>&1; then exit 1; fi
	grep -q "does not match the current source graph" $(BUILD_DIR)/modules/package-module-main-cache-tampered.err
	grep -q "new item fn 'package_math::value'" $(BUILD_DIR)/modules/package-module-main-cache-tampered.err
	sed 's/\\nimport\\t\\tpackage_math\\tpackage_math\\ttests\/packages\/package_math.ari\\t0//' $(BUILD_DIR)/modules/package-module-main.aricache >$(BUILD_DIR)/modules/package-module-main-cache-missing-import.aricache
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-cache-missing-import.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-missing-import.ll >$(BUILD_DIR)/modules/package-module-main-cache-missing-import.err 2>&1; then exit 1; fi
	grep -q "module cache is missing validated import 'package_math'" $(BUILD_DIR)/modules/package-module-main-cache-missing-import.err
	if $(TARGET) -Itests/packages --cfg-feature stale tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-cache-stale.ll >$(BUILD_DIR)/modules/package-module-main-cache-stale.err 2>&1; then exit 1; fi
	grep -q "current source graph has new cfg feature 'stale'" $(BUILD_DIR)/modules/package-module-main-cache-stale.err
	$(TARGET) -Itests/packages --target i686-unknown-linux-gnu tests/ok/package-module-main.ari --emit-module-cache $(BUILD_DIR)/modules/package-module-main-target.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-target.ll
	if $(TARGET) -Itests/packages --target x86_64-pc-linux-gnu tests/ok/package-module-main.ari --use-module-cache $(BUILD_DIR)/modules/package-module-main-target.aricache --emit-llvm $(BUILD_DIR)/modules/package-module-main-target-stale.ll >$(BUILD_DIR)/modules/package-module-main-target-stale.err 2>&1; then exit 1; fi
	grep -q "target option changed from 'i686-unknown-linux-gnu' to 'x86_64-pc-linux-gnu'" $(BUILD_DIR)/modules/package-module-main-target-stale.err
	rm -rf $(BUILD_DIR)/modules/cache-root $(BUILD_DIR)/modules/cache-packages
	mkdir -p $(BUILD_DIR)/modules/cache-root $(BUILD_DIR)/modules/cache-packages/nested
	cp tests/ok/package-module-main.ari $(BUILD_DIR)/modules/cache-root/app.ari
	cp tests/packages/package_math.ari $(BUILD_DIR)/modules/cache-packages/package_math.ari
	cp tests/packages/package_header.arih $(BUILD_DIR)/modules/cache-packages/package_header.arih
	cp tests/packages/nested/mod.ari $(BUILD_DIR)/modules/cache-packages/nested/mod.ari
	$(TARGET) -I$(BUILD_DIR)/modules/cache-packages $(BUILD_DIR)/modules/cache-root/app.ari --emit-module-cache $(BUILD_DIR)/modules/cache-source.aricache --emit-llvm $(BUILD_DIR)/modules/cache-source.ll
	printf '\n// stale\n' >>$(BUILD_DIR)/modules/cache-packages/package_math.ari
	if $(TARGET) -I$(BUILD_DIR)/modules/cache-packages $(BUILD_DIR)/modules/cache-root/app.ari --use-module-cache $(BUILD_DIR)/modules/cache-source.aricache --emit-llvm $(BUILD_DIR)/modules/cache-source-stale.ll >$(BUILD_DIR)/modules/cache-source-stale.err 2>&1; then exit 1; fi
	grep -q "content hash changed" $(BUILD_DIR)/modules/cache-source-stale.err
	rm -rf $(BUILD_DIR)/modules/cache-shadow-root $(BUILD_DIR)/modules/cache-shadow-packages
	mkdir -p $(BUILD_DIR)/modules/cache-shadow-root $(BUILD_DIR)/modules/cache-shadow-packages/nested
	cp tests/ok/package-module-main.ari $(BUILD_DIR)/modules/cache-shadow-root/app.ari
	cp tests/packages/package_math.ari $(BUILD_DIR)/modules/cache-shadow-packages/package_math.ari
	cp tests/packages/package_header.arih $(BUILD_DIR)/modules/cache-shadow-packages/package_header.arih
	cp tests/packages/nested/mod.ari $(BUILD_DIR)/modules/cache-shadow-packages/nested/mod.ari
	$(TARGET) -I$(BUILD_DIR)/modules/cache-shadow-packages $(BUILD_DIR)/modules/cache-shadow-root/app.ari --emit-module-cache $(BUILD_DIR)/modules/cache-shadow.aricache --emit-llvm $(BUILD_DIR)/modules/cache-shadow.ll
	printf 'pub fn value() -> i64 {\n  99\n}\n' >$(BUILD_DIR)/modules/cache-shadow-root/package_math.ari
	if $(TARGET) -I$(BUILD_DIR)/modules/cache-shadow-packages $(BUILD_DIR)/modules/cache-shadow-root/app.ari --use-module-cache $(BUILD_DIR)/modules/cache-shadow.aricache --emit-llvm $(BUILD_DIR)/modules/cache-shadow-stale.ll >$(BUILD_DIR)/modules/cache-shadow-stale.err 2>&1; then exit 1; fi
	grep -q "resolved to" $(BUILD_DIR)/modules/cache-shadow-stale.err
	awk 'BEGIN { FS=OFS="\t" } NR == 1 { print "ari-module-metadata-v1"; next } $$1 == "source" { print $$1, $$2, $$3, $$4; next } { print }' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-v1.arimeta
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-v1.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-v1-check.ll >$(BUILD_DIR)/modules/package-module-main-v1-check.err 2>&1; then exit 1; fi
	grep -q "does not include source content hashes" $(BUILD_DIR)/modules/package-module-main-v1-check.err
	if $(TARGET) -Itests/packages --cfg-feature stale tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-stale.ll >$(BUILD_DIR)/modules/package-module-main-stale.err 2>&1; then exit 1; fi
	grep -q "does not match the current source graph" $(BUILD_DIR)/modules/package-module-main-stale.err
	grep -q "new cfg feature 'stale'" $(BUILD_DIR)/modules/package-module-main-stale.err
	sed '/source.*package_math.*tests\/packages\/package_math.ari/s/[0-9a-f][0-9a-f]*$$/deadbeefdeadbeef/' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-stale-source-hash.arimeta
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-stale-source-hash.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-stale-source-hash.ll >$(BUILD_DIR)/modules/package-module-main-stale-source-hash.err 2>&1; then exit 1; fi
	grep -q "content hash changed" $(BUILD_DIR)/modules/package-module-main-stale-source-hash.err
	sed '/source.*package_math.*tests\/packages\/package_math.ari/d' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-missing-source.arimeta
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-missing-source.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-missing-source.ll >$(BUILD_DIR)/modules/package-module-main-missing-source.err 2>&1; then exit 1; fi
	grep -q "new source 'package_math' at 'tests/packages/package_math.ari'" $(BUILD_DIR)/modules/package-module-main-missing-source.err
	sed '/import.*package_math::nested.*tests\/packages\/nested\/mod.ari/d' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-missing-import.arimeta
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-missing-import.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-missing-import.ll >$(BUILD_DIR)/modules/package-module-main-missing-import.err 2>&1; then exit 1; fi
	grep -q "new import 'package_math::nested as nested'" $(BUILD_DIR)/modules/package-module-main-missing-import.err
	sed '/item.*fn.*package_math::value/d' $(BUILD_DIR)/modules/package-module-main.arimeta >$(BUILD_DIR)/modules/package-module-main-missing-item.arimeta
	if $(TARGET) -Itests/packages tests/ok/package-module-main.ari --check-module-metadata $(BUILD_DIR)/modules/package-module-main-missing-item.arimeta --emit-llvm $(BUILD_DIR)/modules/package-module-main-missing-item.ll >$(BUILD_DIR)/modules/package-module-main-missing-item.err 2>&1; then exit 1; fi
	grep -q "new item fn 'package_math::value'" $(BUILD_DIR)/modules/package-module-main-missing-item.err
	if $(TARGET) tests/errors/module-private-reexport.ari -o $(BUILD_DIR)/modules/module-private-reexport.elf >$(BUILD_DIR)/modules/module-private-reexport.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'Api::answer'" $(BUILD_DIR)/modules/module-private-reexport.err
	if $(TARGET) tests/errors/module-glob-private.ari -o $(BUILD_DIR)/modules/module-glob-private.elf >$(BUILD_DIR)/modules/module-glob-private.err 2>&1; then exit 1; fi
	grep -q "unknown function or enum case 'hidden'" $(BUILD_DIR)/modules/module-glob-private.err
	if $(TARGET) tests/errors/private-module-constant.ari -o $(BUILD_DIR)/modules/private-module-constant.elf >$(BUILD_DIR)/modules/private-module-constant.err 2>&1; then exit 1; fi
	grep -q "constant 'Hidden::SECRET' is not public" $(BUILD_DIR)/modules/private-module-constant.err
	if $(TARGET) tests/errors/module-glob-duplicate.ari -o $(BUILD_DIR)/modules/module-glob-duplicate.elf >$(BUILD_DIR)/modules/module-glob-duplicate.err 2>&1; then exit 1; fi
	grep -q "duplicate use alias 'value'" $(BUILD_DIR)/modules/module-glob-duplicate.err
	if $(TARGET) tests/errors/module-super-root.ari -o $(BUILD_DIR)/modules/module-super-root.elf >$(BUILD_DIR)/modules/module-super-root.err 2>&1; then exit 1; fi
	grep -q "super:: path cannot escape the root module" $(BUILD_DIR)/modules/module-super-root.err
	if $(TARGET) tests/errors/private-nested-module.ari -o $(BUILD_DIR)/modules/private-nested-module.elf >$(BUILD_DIR)/modules/private-nested-module.err 2>&1; then exit 1; fi
	grep -q "module 'Outer::Inner' is not public" $(BUILD_DIR)/modules/private-nested-module.err

check-operators: $(TARGET)
	mkdir -p $(BUILD_DIR)/operators
	$(TARGET) --freestanding tests/ok/operators-numeric-suffixes.ari -o $(BUILD_DIR)/operators/operators-numeric-suffixes.raw
	$(BUILD_DIR)/operators/operators-numeric-suffixes.raw; code=$$?; test $$code -eq 41
	$(TARGET) --freestanding tests/ok/operators-numeric-bases.ari -o $(BUILD_DIR)/operators/operators-numeric-bases.raw
	$(BUILD_DIR)/operators/operators-numeric-bases.raw; code=$$?; test $$code -eq 33
	$(TARGET) --freestanding tests/ok/operators-float-suffixes.ari -o $(BUILD_DIR)/operators/operators-float-suffixes.raw
	$(BUILD_DIR)/operators/operators-float-suffixes.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/operators-bitwise-not.ari -o $(BUILD_DIR)/operators/operators-bitwise-not.raw
	$(BUILD_DIR)/operators/operators-bitwise-not.raw; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/operators-compound-assignment.ari -o $(BUILD_DIR)/operators/operators-compound-assignment.raw
	$(BUILD_DIR)/operators/operators-compound-assignment.raw; code=$$?; test $$code -eq 8
	$(TARGET) --freestanding tests/ok/operators-try.ari -o $(BUILD_DIR)/operators/operators-try.raw
	$(BUILD_DIR)/operators/operators-try.raw; code=$$?; test $$code -eq 52
	$(TARGET) --freestanding tests/ok/operators-try-residual-conversion.ari -o $(BUILD_DIR)/operators/operators-try-residual-conversion.raw
	$(BUILD_DIR)/operators/operators-try-residual-conversion.raw; code=$$?; test $$code -eq 65
	$(TARGET) --freestanding tests/ok/operators-null-coalescing.ari -o $(BUILD_DIR)/operators/operators-null-coalescing.raw
	$(BUILD_DIR)/operators/operators-null-coalescing.raw; code=$$?; test $$code -eq 12
	$(TARGET) tests/ok/operators-numeric-suffixes.ari --emit-llvm $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i8 -8" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i16 2" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i32 3" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	grep -q "store i64 4" $(BUILD_DIR)/operators/operators-numeric-suffixes.ll
	$(TARGET) tests/ok/operators-numeric-bases.ari --emit-llvm $(BUILD_DIR)/operators/operators-numeric-bases.ll
	grep -q "store i8 255" $(BUILD_DIR)/operators/operators-numeric-bases.ll
	grep -q "store i16 8" $(BUILD_DIR)/operators/operators-numeric-bases.ll
	$(TARGET) tests/ok/string-escapes.ari --emit-llvm $(BUILD_DIR)/operators/string-escapes.ll
	grep -Fq '\0A\09\0D\07\08\0C\0B\5C\22\27\3F\21\41\42' $(BUILD_DIR)/operators/string-escapes.ll
	grep -Fq '\F0\9F\98\80\53\00' $(BUILD_DIR)/operators/string-escapes.ll
	$(TARGET) tests/ok/operators-float-suffixes.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "alloca float" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "alloca double" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "store float 1.000000" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	grep -q "store double 10.000000" $(BUILD_DIR)/operators/operators-float-suffixes.ll
	$(TARGET) tests/ok/operators-float-llvm.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fadd float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fsub float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fmul float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fdiv float" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fadd double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fsub double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fmul double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fdiv double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	grep -q "fcmp ogt double" $(BUILD_DIR)/operators/operators-float-llvm.ll
	$(TARGET) --freestanding tests/ok/operators-float-llvm.ari -o $(BUILD_DIR)/operators/operators-float-llvm.raw
	$(BUILD_DIR)/operators/operators-float-llvm.raw; code=$$?; test $$code -eq 10
	$(TARGET) tests/ok/operators-float-casts.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-casts.ll
	grep -q "fpext float" $(BUILD_DIR)/operators/operators-float-casts.ll
	grep -q "fptrunc double" $(BUILD_DIR)/operators/operators-float-casts.ll
	$(TARGET) --freestanding tests/ok/operators-float-casts.ari -o $(BUILD_DIR)/operators/operators-float-casts.raw
	$(BUILD_DIR)/operators/operators-float-casts.raw; code=$$?; test $$code -eq 15
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/operators-float-casts.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-float-casts.elf; $(BUILD_DIR)/operators/operators-float-casts.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/ok/operators-float-int-casts.ari --emit-llvm $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "sitofp i64" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "uitofp i8" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "fptosi double" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	grep -q "fptoui float" $(BUILD_DIR)/operators/operators-float-int-casts.ll
	$(TARGET) --freestanding tests/ok/operators-float-int-casts.ari -o $(BUILD_DIR)/operators/operators-float-int-casts.raw
	$(BUILD_DIR)/operators/operators-float-int-casts.raw; code=$$?; test $$code -eq 15
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/operators-float-int-casts.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-float-int-casts.elf; $(BUILD_DIR)/operators/operators-float-int-casts.elf; code=$$?; test $$code -eq 15; fi
	$(TARGET) tests/ok/operators-f128-llvm.ari --emit-llvm $(BUILD_DIR)/operators/operators-f128-llvm.ll
	grep -q "alloca fp128" $(BUILD_DIR)/operators/operators-f128-llvm.ll
	grep -q "fadd fp128" $(BUILD_DIR)/operators/operators-f128-llvm.ll
	grep -q "fcmp ogt fp128" $(BUILD_DIR)/operators/operators-f128-llvm.ll
	$(TARGET) tests/ok/operators-bitwise-not.ari --emit-llvm $(BUILD_DIR)/operators/operators-bitwise-not.ll
	grep -q "xor i64 .* -1" $(BUILD_DIR)/operators/operators-bitwise-not.ll
	grep -q "xor i8 .* -1" $(BUILD_DIR)/operators/operators-bitwise-not.ll
	$(TARGET) tests/ok/operators-compound-assignment.ari --emit-llvm $(BUILD_DIR)/operators/operators-compound-assignment.ll
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
	$(TARGET) tests/ok/operators-try.ari --emit-llvm $(BUILD_DIR)/operators/operators-try.ll
	grep -q "try.ok" $(BUILD_DIR)/operators/operators-try.ll
	grep -q "try.return" $(BUILD_DIR)/operators/operators-try.ll
	$(TARGET) tests/ok/operators-try-residual-conversion.ari --emit-llvm $(BUILD_DIR)/operators/operators-try-residual-conversion.ll
	grep -q "try.return" $(BUILD_DIR)/operators/operators-try-residual-conversion.ll
	grep -q "shl i64" $(BUILD_DIR)/operators/operators-try-residual-conversion.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/operators-try-residual-conversion.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-try-residual-conversion.elf; $(BUILD_DIR)/operators/operators-try-residual-conversion.elf; code=$$?; test $$code -eq 65; fi
	$(TARGET) tests/ok/operators-try-aggregate.ari --emit-llvm $(BUILD_DIR)/operators/operators-try-aggregate.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/operators/operators-try-aggregate.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/operators-try-aggregate.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/operators/operators-try-aggregate.elf; $(BUILD_DIR)/operators/operators-try-aggregate.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) tests/ok/operators-null-coalescing.ari --emit-llvm $(BUILD_DIR)/operators/operators-null-coalescing.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/operators/operators-null-coalescing.ll
	grep -q "coalesce.fallback" $(BUILD_DIR)/operators/operators-null-coalescing.ll
	if $(TARGET) tests/errors/operators-float-int-suffix.ari -o $(BUILD_DIR)/operators/operators-float-int-suffix.elf >$(BUILD_DIR)/operators/operators-float-int-suffix.err 2>&1; then exit 1; fi
	grep -q "float literal suffix must be f32, f64, or f128" $(BUILD_DIR)/operators/operators-float-int-suffix.err
	if $(TARGET) tests/errors/operators-int-suffix-range.ari -o $(BUILD_DIR)/operators/operators-int-suffix-range.elf >$(BUILD_DIR)/operators/operators-int-suffix-range.err 2>&1; then exit 1; fi
	grep -q "integer literal 256 is out of range for u8" $(BUILD_DIR)/operators/operators-int-suffix-range.err
	if $(TARGET) tests/errors/operators-invalid-suffix.ari -o $(BUILD_DIR)/operators/operators-invalid-suffix.elf >$(BUILD_DIR)/operators/operators-invalid-suffix.err 2>&1; then exit 1; fi
	grep -q "unsupported numeric literal suffix 'usize'" $(BUILD_DIR)/operators/operators-invalid-suffix.err
	if $(TARGET) tests/errors/operators-invalid-binary-literal.ari -o $(BUILD_DIR)/operators/operators-invalid-binary-literal.elf >$(BUILD_DIR)/operators/operators-invalid-binary-literal.err 2>&1; then exit 1; fi
	grep -q "invalid digit '2' in binary literal" $(BUILD_DIR)/operators/operators-invalid-binary-literal.err
	if $(TARGET) tests/errors/string-invalid-hex-escape.ari -o $(BUILD_DIR)/operators/string-invalid-hex-escape.elf >$(BUILD_DIR)/operators/string-invalid-hex-escape.err 2>&1; then exit 1; fi
	grep -q "\\\\x escape requires at least one digit" $(BUILD_DIR)/operators/string-invalid-hex-escape.err
	if $(TARGET) tests/errors/string-invalid-unicode-escape.ari -o $(BUILD_DIR)/operators/string-invalid-unicode-escape.elf >$(BUILD_DIR)/operators/string-invalid-unicode-escape.err 2>&1; then exit 1; fi
	grep -q "invalid Unicode scalar value in string escape" $(BUILD_DIR)/operators/string-invalid-unicode-escape.err
	if $(TARGET) tests/errors/operators-question.ari -o $(BUILD_DIR)/operators/operators-question.elf >$(BUILD_DIR)/operators/operators-question.err 2>&1; then exit 1; fi
	grep -q "postfix ? can only early-return from functions returning MaybeI64, got i32" $(BUILD_DIR)/operators/operators-question.err
	if $(TARGET) tests/errors/operators-try-residual-mismatch.ari -o $(BUILD_DIR)/operators/operators-try-residual-mismatch.elf >$(BUILD_DIR)/operators/operators-try-residual-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected bool, got i32" $(BUILD_DIR)/operators/operators-try-residual-mismatch.err
	if $(TARGET) tests/errors/operators-question-non-enum.ari -o $(BUILD_DIR)/operators/operators-question-non-enum.elf >$(BUILD_DIR)/operators/operators-question-non-enum.err 2>&1; then exit 1; fi
	grep -q "postfix ? requires an enum value, got i64" $(BUILD_DIR)/operators/operators-question-non-enum.err
	if $(TARGET) tests/errors/operator-null-coalescing-non-enum.ari -o $(BUILD_DIR)/operators/operator-null-coalescing-non-enum.elf >$(BUILD_DIR)/operators/operator-null-coalescing-non-enum.err 2>&1; then exit 1; fi
	grep -q "operator ?? requires an Option/Result-style enum value, got i64" $(BUILD_DIR)/operators/operator-null-coalescing-non-enum.err
	if $(TARGET) tests/errors/operators-bitwise-not-bool.ari -o $(BUILD_DIR)/operators/operators-bitwise-not-bool.elf >$(BUILD_DIR)/operators/operators-bitwise-not-bool.err 2>&1; then exit 1; fi
	grep -q "bitwise-not operand must be integer, got bool" $(BUILD_DIR)/operators/operators-bitwise-not-bool.err
	if $(TARGET) tests/errors/operators-compound-immutable.ari -o $(BUILD_DIR)/operators/operators-compound-immutable.elf >$(BUILD_DIR)/operators/operators-compound-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'value'" $(BUILD_DIR)/operators/operators-compound-immutable.err

check-generics: $(TARGET)
	mkdir -p $(BUILD_DIR)/generics
	$(TARGET) --freestanding tests/ok/function-generic-call.ari -o $(BUILD_DIR)/generics/function-generic-call.raw
	$(BUILD_DIR)/generics/function-generic-call.raw; code=$$?; test $$code -eq 22
	$(TARGET) --freestanding tests/ok/generic-function-specializations.ari -o $(BUILD_DIR)/generics/generic-function-specializations.raw
	$(BUILD_DIR)/generics/generic-function-specializations.raw; code=$$?; test $$code -eq 21
	$(TARGET) --freestanding tests/ok/generic-function-pointer.ari -o $(BUILD_DIR)/generics/generic-function-pointer.raw
	$(BUILD_DIR)/generics/generic-function-pointer.raw; code=$$?; test $$code -eq 42
	$(TARGET) tests/ok/generic-function-pointer.ari --emit-llvm $(BUILD_DIR)/generics/generic-function-pointer.ll
	grep -q "identity__G_i64" $(BUILD_DIR)/generics/generic-function-pointer.ll
	grep -q "identity__G_bool" $(BUILD_DIR)/generics/generic-function-pointer.ll
	grep -q 'call i64 %' $(BUILD_DIR)/generics/generic-function-pointer.ll
	$(TARGET) --freestanding tests/ok/generic-any-name.ari -o $(BUILD_DIR)/generics/generic-any-name.raw
	$(BUILD_DIR)/generics/generic-any-name.raw; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/generic-explicit-type-args.ari -o $(BUILD_DIR)/generics/generic-explicit-type-args.raw
	$(BUILD_DIR)/generics/generic-explicit-type-args.raw; code=$$?; test $$code -eq 27
	$(TARGET) tests/ok/generic-explicit-type-args.ari --emit-llvm $(BUILD_DIR)/generics/generic-explicit-type-args.ll
	grep -q "identity__G_i64" $(BUILD_DIR)/generics/generic-explicit-type-args.ll
	grep -q "identity__G_bool" $(BUILD_DIR)/generics/generic-explicit-type-args.ll
	$(TARGET) --freestanding tests/ok/generic-angle-call-index.ari -o $(BUILD_DIR)/generics/generic-angle-call-index.raw
	$(BUILD_DIR)/generics/generic-angle-call-index.raw; code=$$?; test $$code -eq 22
	$(TARGET) --freestanding tests/ok/struct-surface.ari -o $(BUILD_DIR)/generics/struct-surface.raw
	$(BUILD_DIR)/generics/struct-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/trait-generic-impl.ari -o $(BUILD_DIR)/generics/trait-generic-impl.raw
	$(BUILD_DIR)/generics/trait-generic-impl.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/generic-adt-surface.ari -o $(BUILD_DIR)/generics/generic-adt-surface.raw
	$(BUILD_DIR)/generics/generic-adt-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) tests/ok/generic-enum-constructors.ari --emit-llvm $(BUILD_DIR)/generics/generic-enum-constructors.ll
	grep -q "insertvalue { i32, i64 }" $(BUILD_DIR)/generics/generic-enum-constructors.ll
	$(TARGET) tests/ok/generic-enum-try.ari --emit-llvm $(BUILD_DIR)/generics/generic-enum-try.ll
	grep -q "try.ok" $(BUILD_DIR)/generics/generic-enum-try.ll
	grep -q "coalesce.ok" $(BUILD_DIR)/generics/generic-enum-try.ll
	grep -q "extractvalue { i32, i64 }" $(BUILD_DIR)/generics/generic-enum-try.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-enum-try.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/generics/generic-enum-try.elf; $(BUILD_DIR)/generics/generic-enum-try.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) --freestanding tests/ok/generic-enum-patterns.ari -o $(BUILD_DIR)/generics/generic-enum-patterns.raw
	$(BUILD_DIR)/generics/generic-enum-patterns.raw; code=$$?; test $$code -eq 23
	$(TARGET) tests/ok/generic-enum-patterns.ari --emit-llvm $(BUILD_DIR)/generics/generic-enum-patterns.ll
	grep -q "match.expr.arm" $(BUILD_DIR)/generics/generic-enum-patterns.ll
	grep -q "lshr" $(BUILD_DIR)/generics/generic-enum-patterns.ll
	$(TARGET) --freestanding tests/ok/generic-enum-constants.ari -o $(BUILD_DIR)/generics/generic-enum-constants.raw
	$(BUILD_DIR)/generics/generic-enum-constants.raw; code=$$?; test $$code -eq 7
	if $(TARGET) tests/errors/generic-inference-conflict.ari -o $(BUILD_DIR)/generics/generic-inference-conflict.elf >$(BUILD_DIR)/generics/generic-inference-conflict.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' inferred as both i64 and bool" $(BUILD_DIR)/generics/generic-inference-conflict.err
	if $(TARGET) tests/errors/generic-uninferred.ari -o $(BUILD_DIR)/generics/generic-uninferred.elf >$(BUILD_DIR)/generics/generic-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' could not be inferred for 'make'" $(BUILD_DIR)/generics/generic-uninferred.err
	if $(TARGET) tests/errors/generic-enum-constructor-uninferred.ari -o $(BUILD_DIR)/generics/generic-enum-constructor-uninferred.elf >$(BUILD_DIR)/generics/generic-enum-constructor-uninferred.err 2>&1; then exit 1; fi
	grep -q "cannot infer type argument 'T' for enum case 'std::None'" $(BUILD_DIR)/generics/generic-enum-constructor-uninferred.err
	if $(TARGET) tests/errors/generic-enum-constructor-arity.ari -o $(BUILD_DIR)/generics/generic-enum-constructor-arity.elf >$(BUILD_DIR)/generics/generic-enum-constructor-arity.err 2>&1; then exit 1; fi
	grep -q "enum 'std::Option' expects 1 type argument" $(BUILD_DIR)/generics/generic-enum-constructor-arity.err
	if $(TARGET) tests/errors/generic-enum-constructor-mismatch.ari -o $(BUILD_DIR)/generics/generic-enum-constructor-mismatch.elf >$(BUILD_DIR)/generics/generic-enum-constructor-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected std::Option\\[i64\\], got std::Option\\[bool\\]" $(BUILD_DIR)/generics/generic-enum-constructor-mismatch.err
	if $(TARGET) tests/errors/generic-enum-null-coalescing-type.ari -o $(BUILD_DIR)/generics/generic-enum-null-coalescing-type.elf >$(BUILD_DIR)/generics/generic-enum-null-coalescing-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/generics/generic-enum-null-coalescing-type.err
	if $(TARGET) tests/errors/generic-enum-pattern-type.ari -o $(BUILD_DIR)/generics/generic-enum-pattern-type.elf >$(BUILD_DIR)/generics/generic-enum-pattern-type.err 2>&1; then exit 1; fi
	grep -q "bool payload patterns require a bool enum payload" $(BUILD_DIR)/generics/generic-enum-pattern-type.err
	if $(TARGET) tests/errors/generic-enum-constant-mismatch.ari -o $(BUILD_DIR)/generics/generic-enum-constant-mismatch.elf >$(BUILD_DIR)/generics/generic-enum-constant-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected std::Option\\[i32\\], got std::Option\\[bool\\]" $(BUILD_DIR)/generics/generic-enum-constant-mismatch.err
	if $(TARGET) tests/errors/generic-function-pointer-uninferred.ari -o $(BUILD_DIR)/generics/generic-function-pointer-uninferred.elf >$(BUILD_DIR)/generics/generic-function-pointer-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' could not be inferred for function pointer 'keep'" $(BUILD_DIR)/generics/generic-function-pointer-uninferred.err
	if $(TARGET) tests/errors/generic-duplicate-param.ari -o $(BUILD_DIR)/generics/generic-duplicate-param.elf >$(BUILD_DIR)/generics/generic-duplicate-param.err 2>&1; then exit 1; fi
	grep -q "duplicate generic parameter 'T' in function 'same'" $(BUILD_DIR)/generics/generic-duplicate-param.err
	if $(TARGET) tests/errors/generic-explicit-arity.ari -o $(BUILD_DIR)/generics/generic-explicit-arity.elf >$(BUILD_DIR)/generics/generic-explicit-arity.err 2>&1; then exit 1; fi
	grep -q "generic function 'identity' expects 1 type argument" $(BUILD_DIR)/generics/generic-explicit-arity.err
	if $(TARGET) tests/errors/generic-explicit-mismatch.ari -o $(BUILD_DIR)/generics/generic-explicit-mismatch.elf >$(BUILD_DIR)/generics/generic-explicit-mismatch.err 2>&1; then exit 1; fi
	grep -q "generic type 'T' inferred as both i64 and bool" $(BUILD_DIR)/generics/generic-explicit-mismatch.err
	if $(TARGET) tests/errors/generic-nongeneric-type-args.ari -o $(BUILD_DIR)/generics/generic-nongeneric-type-args.elf >$(BUILD_DIR)/generics/generic-nongeneric-type-args.err 2>&1; then exit 1; fi
	grep -q "function 'add' does not take type arguments" $(BUILD_DIR)/generics/generic-nongeneric-type-args.err
	if $(TARGET) tests/errors/array-literal-index-out-of-range.ari -o $(BUILD_DIR)/generics/array-literal-index-out-of-range.elf >$(BUILD_DIR)/generics/array-literal-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "array literal index 2 is out of range for 2 elements" $(BUILD_DIR)/generics/array-literal-index-out-of-range.err

check-meta: $(TARGET)
	mkdir -p $(BUILD_DIR)/meta
	$(TARGET) --freestanding tests/ok/attributes-builtin.ari -o $(BUILD_DIR)/meta/attributes-builtin.raw
	$(BUILD_DIR)/meta/attributes-builtin.raw; code=$$?; test $$code -eq 4
	$(TARGET) tests/ok/attributes-builtin.ari --emit-llvm $(BUILD_DIR)/meta/attributes-builtin.ll >$(BUILD_DIR)/meta/attribute-deprecated.warn 2>&1
	grep -q "warning: use of deprecated function 'old_value': use next_value" $(BUILD_DIR)/meta/attribute-deprecated.warn
	$(TARGET) tests/ok/attribute-repr-borrow-fields.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-borrow-fields.ll
	grep -q "{ ptr, ptr, ptr }" $(BUILD_DIR)/meta/attribute-repr-borrow-fields.ll
	grep -q "extractvalue { ptr, ptr, ptr }" $(BUILD_DIR)/meta/attribute-repr-borrow-fields.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/attribute-repr-borrow-fields.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/attribute-repr-borrow-fields.elf; $(BUILD_DIR)/meta/attribute-repr-borrow-fields.elf; code=$$?; test $$code -eq 38; fi
	$(TARGET) tests/ok/attribute-repr-generic-pointer-fields.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.ll
	grep -q "{ ptr, ptr, ptr, i64 }" $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/attribute-repr-generic-pointer-fields.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.elf; $(BUILD_DIR)/meta/attribute-repr-generic-pointer-fields.elf; code=$$?; test $$code -eq 26; fi
	$(TARGET) tests/ok/attribute-repr-generic-value-fields.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.ll
	grep -q "{ i64, i8 }" $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/attribute-repr-generic-value-fields.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.elf; $(BUILD_DIR)/meta/attribute-repr-generic-value-fields.elf; code=$$?; test $$code -eq 42; fi
	$(TARGET) --freestanding tests/ok/attribute-repr-generic-fieldless-enum.ari -o $(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.raw
	$(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.raw; code=$$?; test $$code -eq 11
	$(TARGET) tests/ok/attribute-repr-generic-fieldless-enum.ari --emit-llvm $(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.ll
	grep -q "icmp eq i64" $(BUILD_DIR)/meta/attribute-repr-generic-fieldless-enum.ll
	$(TARGET) --freestanding tests/ok/attributes-user-meta.ari -o $(BUILD_DIR)/meta/attributes-user-meta.raw
	$(BUILD_DIR)/meta/attributes-user-meta.raw; code=$$?; test $$code -eq 9
	$(TARGET) --freestanding tests/ok/attribute-cfg.ari -o $(BUILD_DIR)/meta/attribute-cfg.raw
	$(BUILD_DIR)/meta/attribute-cfg.raw; code=$$?; test $$code -eq 10
	$(TARGET) --cfg-feature fast --freestanding tests/ok/attribute-cfg-feature.ari -o $(BUILD_DIR)/meta/attribute-cfg-feature.raw
	$(BUILD_DIR)/meta/attribute-cfg-feature.raw; code=$$?; test $$code -eq 13
	$(TARGET) --target x86_64-pc-windows-msvc tests/ok/attribute-cfg-target.ari --emit-llvm $(BUILD_DIR)/meta/attribute-cfg-target.ll
	grep -q 'target triple = "x86_64-pc-windows-msvc"' $(BUILD_DIR)/meta/attribute-cfg-target.ll
	$(TARGET) --test --freestanding tests/ok/attribute-test-runner.ari -o $(BUILD_DIR)/meta/attribute-test-runner.raw
	$(BUILD_DIR)/meta/attribute-test-runner.raw; code=$$?; test $$code -eq 0
	$(TARGET) tests/ok/attribute-export-symbols.ari --shared --emit-llvm $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'define i64 @"ari_public_add"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'define i64 @"increment"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'define i64 @"reset"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	grep -q 'call i64 @"ari_public_add"' $(BUILD_DIR)/meta/attribute-export-symbols.ll
	$(TARGET) tests/ok/c-header-export.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-export.h --emit-llvm $(BUILD_DIR)/meta/c-header-export.ll
	grep -q '#pragma once' $(BUILD_DIR)/meta/c-header-export.h
	grep -q 'int64_t ari_add_i64(int64_t arg0, int64_t arg1);' $(BUILD_DIR)/meta/c-header-export.h
	grep -q 'void ari_touch(void\* arg0, bool arg1);' $(BUILD_DIR)/meta/c-header-export.h
	if grep -q 'hidden' $(BUILD_DIR)/meta/c-header-export.h; then exit 1; fi
	$(TARGET) tests/ok/c-header-repr-struct.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-repr-struct.h --emit-llvm $(BUILD_DIR)/meta/c-header-repr-struct.ll
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
	$(TARGET) --freestanding tests/ok/attribute-export-symbols-freestanding.ari -o $(BUILD_DIR)/meta/attribute-export-symbols-freestanding.raw
	$(BUILD_DIR)/meta/attribute-export-symbols-freestanding.raw; code=$$?; test $$code -eq 10
	readelf -sW $(BUILD_DIR)/meta/attribute-export-symbols-freestanding.raw >$(BUILD_DIR)/meta/attribute-export-symbols-freestanding.sym
	grep -q 'ari_public_add' $(BUILD_DIR)/meta/attribute-export-symbols-freestanding.sym
	grep -q 'increment' $(BUILD_DIR)/meta/attribute-export-symbols-freestanding.sym
	grep -q 'zero' $(BUILD_DIR)/meta/attribute-export-symbols-freestanding.sym
	if grep -Eq '_ARNv3add|_ARNv9increment|_ARNv4zero' $(BUILD_DIR)/meta/attribute-export-symbols-freestanding.sym; then exit 1; fi
	$(TARGET) tests/ok/shared-visibility.ari --shared --emit-llvm $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define hidden i64 @"_ARNv6helper"' $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define i64 @"_ARNv10public_api"' $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define i64 @"ari_named_private"' $(BUILD_DIR)/meta/shared-visibility.ll
	grep -q 'define hidden i64 @ari_builtin_write_i64' $(BUILD_DIR)/meta/shared-visibility.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v nm >/dev/null 2>&1; then $(TARGET) tests/ok/shared-visibility.ari --shared --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/meta/shared-visibility.so; nm -D $(BUILD_DIR)/meta/shared-visibility.so >$(BUILD_DIR)/meta/shared-visibility.sym; grep -q '_ARNv10public_api' $(BUILD_DIR)/meta/shared-visibility.sym; grep -q 'ari_named_private' $(BUILD_DIR)/meta/shared-visibility.sym; if grep -Eq '_ARNv6helper|_ARNv4main|ari_builtin_' $(BUILD_DIR)/meta/shared-visibility.sym; then exit 1; fi; fi
	$(TARGET) tests/ok/shared-visibility.ari --emit-llvm $(BUILD_DIR)/meta/shared-visibility-exe.ll
	grep -q 'define i64 @"_ARNv6helper"' $(BUILD_DIR)/meta/shared-visibility-exe.ll
	$(TARGET) --freestanding tests/ok/meta-function-surface.ari -o $(BUILD_DIR)/meta/meta-function-surface.raw
	$(BUILD_DIR)/meta/meta-function-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/matches-macro.ari -o $(BUILD_DIR)/meta/matches-macro.raw
	$(BUILD_DIR)/meta/matches-macro.raw; code=$$?; test $$code -eq 15
	if $(TARGET) tests/errors/attribute-unknown.ari -o $(BUILD_DIR)/meta/attribute-unknown.elf >$(BUILD_DIR)/meta/attribute-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown attribute '@unknown'" $(BUILD_DIR)/meta/attribute-unknown.err
	if $(TARGET) tests/errors/attribute-placement.ari -o $(BUILD_DIR)/meta/attribute-placement.elf >$(BUILD_DIR)/meta/attribute-placement.err 2>&1; then exit 1; fi
	grep -q "attribute '@derive' is only supported on structs and enums" $(BUILD_DIR)/meta/attribute-placement.err
	if $(TARGET) tests/errors/attribute-repr-unsupported.ari -o $(BUILD_DIR)/meta/attribute-repr-unsupported.elf >$(BUILD_DIR)/meta/attribute-repr-unsupported.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr' currently supports only C layout" $(BUILD_DIR)/meta/attribute-repr-unsupported.err
	if $(TARGET) tests/errors/attribute-repr-enum-payload.ari -o $(BUILD_DIR)/meta/attribute-repr-enum-payload.elf >$(BUILD_DIR)/meta/attribute-repr-enum-payload.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr(C)' currently supports only fieldless enums" $(BUILD_DIR)/meta/attribute-repr-enum-payload.err
	if $(TARGET) tests/errors/attribute-repr-own-field.ari -o $(BUILD_DIR)/meta/attribute-repr-own-field.elf >$(BUILD_DIR)/meta/attribute-repr-own-field.err 2>&1; then exit 1; fi
	grep -q "attribute '@repr(C)' fields cannot use own; expose ownership through an explicit ptr/ref ABI" $(BUILD_DIR)/meta/attribute-repr-own-field.err
	if $(TARGET) tests/errors/c-header-large-aggregate-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-large-aggregate-by-value.h >$(BUILD_DIR)/meta/c-header-large-aggregate-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter Wide is limited to direct aggregate ABI values up to 16 bytes and 8-byte alignment" $(BUILD_DIR)/meta/c-header-large-aggregate-by-value.err
	if $(TARGET) --target x86_64-pc-windows-msvc tests/errors/c-header-target-aggregate-by-value.ari --shared --emit-c-header $(BUILD_DIR)/meta/c-header-target-aggregate-by-value.h >$(BUILD_DIR)/meta/c-header-target-aggregate-by-value.err 2>&1; then exit 1; fi
	grep -q "C header emission for by-value parameter ApiPair is currently supported only on 64-bit Unix targets" $(BUILD_DIR)/meta/c-header-target-aggregate-by-value.err
	if $(TARGET) tests/errors/attribute-cfg-argument.ari -o $(BUILD_DIR)/meta/attribute-cfg-argument.elf >$(BUILD_DIR)/meta/attribute-cfg-argument.err 2>&1; then exit 1; fi
	grep -q "unknown cfg predicate 'enabled'" $(BUILD_DIR)/meta/attribute-cfg-argument.err
	if $(TARGET) tests/errors/attribute-deprecated-argument.ari -o $(BUILD_DIR)/meta/attribute-deprecated-argument.elf >$(BUILD_DIR)/meta/attribute-deprecated-argument.err 2>&1; then exit 1; fi
	grep -q "attribute '@deprecated' expects no arguments or one string message" $(BUILD_DIR)/meta/attribute-deprecated-argument.err
	if $(TARGET) tests/errors/attribute-export-symbol-invalid.ari -o $(BUILD_DIR)/meta/attribute-export-symbol-invalid.elf >$(BUILD_DIR)/meta/attribute-export-symbol-invalid.err 2>&1; then exit 1; fi
	grep -q "invalid exported symbol 'bad-symbol'" $(BUILD_DIR)/meta/attribute-export-symbol-invalid.err
	if $(TARGET) tests/errors/attribute-export-mangle-collision.ari -o $(BUILD_DIR)/meta/attribute-export-mangle-collision.elf >$(BUILD_DIR)/meta/attribute-export-mangle-collision.err 2>&1; then exit 1; fi
	grep -q "emitted symbol '_ARNv6helper' for function 'collides' conflicts with function 'helper'" $(BUILD_DIR)/meta/attribute-export-mangle-collision.err
	if $(TARGET) tests/errors/attribute-no-mangle-argument.ari -o $(BUILD_DIR)/meta/attribute-no-mangle-argument.elf >$(BUILD_DIR)/meta/attribute-no-mangle-argument.err 2>&1; then exit 1; fi
	grep -q "attribute '@no_mangle' does not take arguments" $(BUILD_DIR)/meta/attribute-no-mangle-argument.err
	if $(TARGET) --test tests/errors/attribute-test-param.ari -o $(BUILD_DIR)/meta/attribute-test-param.elf >$(BUILD_DIR)/meta/attribute-test-param.err 2>&1; then exit 1; fi
	grep -q "@test functions cannot take parameters" $(BUILD_DIR)/meta/attribute-test-param.err
	if $(TARGET) tests/errors/meta-function-bad-param.ari -o $(BUILD_DIR)/meta/meta-function-bad-param.elf >$(BUILD_DIR)/meta/meta-function-bad-param.err 2>&1; then exit 1; fi
	grep -q "meta function parameters must use token_stream, ast, or type, got i64" $(BUILD_DIR)/meta/meta-function-bad-param.err
	if $(TARGET) tests/errors/meta-function-generic.ari -o $(BUILD_DIR)/meta/meta-function-generic.elf >$(BUILD_DIR)/meta/meta-function-generic.err 2>&1; then exit 1; fi
	grep -q "meta functions cannot be generic; define one concrete meta entry point per token_stream, ast, or type transform" $(BUILD_DIR)/meta/meta-function-generic.err
	if $(TARGET) tests/errors/matches-macro-arity.ari -o $(BUILD_DIR)/meta/matches-macro-arity.elf >$(BUILD_DIR)/meta/matches-macro-arity.err 2>&1; then exit 1; fi
	grep -q "expected , after matches! value" $(BUILD_DIR)/meta/matches-macro-arity.err
	if $(TARGET) tests/errors/matches-macro-pattern-type.ari -o $(BUILD_DIR)/meta/matches-macro-pattern-type.elf >$(BUILD_DIR)/meta/matches-macro-pattern-type.err 2>&1; then exit 1; fi
	grep -q "integer match patterns must be integer literals or _" $(BUILD_DIR)/meta/matches-macro-pattern-type.err
	if $(TARGET) tests/errors/macro-call-planned.ari -o $(BUILD_DIR)/meta/macro-call-planned.elf >$(BUILD_DIR)/meta/macro-call-planned.err 2>&1; then exit 1; fi
	grep -q "macro invocation 'make_value!' requires compile-time token_stream/ast expansion" $(BUILD_DIR)/meta/macro-call-planned.err
	if $(TARGET) tests/errors/macro-call-unknown.ari -o $(BUILD_DIR)/meta/macro-call-unknown.elf >$(BUILD_DIR)/meta/macro-call-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown macro 'missing!'" $(BUILD_DIR)/meta/macro-call-unknown.err
	if $(TARGET) tests/errors/item-macro-planned.ari -o $(BUILD_DIR)/meta/item-macro-planned.elf >$(BUILD_DIR)/meta/item-macro-planned.err 2>&1; then exit 1; fi
	grep -q "item macro invocation syntax is planned for meta functions but expansion is not supported yet" $(BUILD_DIR)/meta/item-macro-planned.err

check-traits: $(TARGET)
	mkdir -p $(BUILD_DIR)/traits
	$(TARGET) --freestanding tests/ok/trait-surface.ari -o $(BUILD_DIR)/traits/trait-surface.raw
	$(BUILD_DIR)/traits/trait-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/trait-object-type-surface.ari -o $(BUILD_DIR)/traits/trait-object-type-surface.raw
	$(BUILD_DIR)/traits/trait-object-type-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/trait-generic-impl.ari -o $(BUILD_DIR)/traits/trait-generic-impl.raw
	$(BUILD_DIR)/traits/trait-generic-impl.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/trait-multi-generic-impl.ari -o $(BUILD_DIR)/traits/trait-multi-generic-impl.raw
	$(BUILD_DIR)/traits/trait-multi-generic-impl.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/iterable-trait-surface.ari -o $(BUILD_DIR)/traits/iterable-trait-surface.raw
	$(BUILD_DIR)/traits/iterable-trait-surface.raw; code=$$?; test $$code -eq 6
	$(TARGET) --freestanding tests/ok/trait-method-call.ari -o $(BUILD_DIR)/traits/trait-method-call.raw
	$(BUILD_DIR)/traits/trait-method-call.raw; code=$$?; test $$code -eq 0
	$(TARGET) tests/ok/trait-method-call.ari --emit-llvm $(BUILD_DIR)/traits/trait-method-call.ll
	grep -q "_ARNv4impl3i647std__Eq2eq" $(BUILD_DIR)/traits/trait-method-call.ll
	$(TARGET) tests/ok/trait-object-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-dispatch.ll
	grep -q "dyn::vtable::i64::dyn_AddTo_i64_" $(BUILD_DIR)/traits/trait-object-dispatch.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/trait-object-dispatch.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-dispatch.elf; $(BUILD_DIR)/traits/trait-object-dispatch.elf; code=$$?; test $$code -eq 11; fi
	$(TARGET) tests/ok/trait-object-generic-impl-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.ll
	grep -q "dyn::vtable::Box_i64_::dyn_Score_i64_" $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.ll
	grep -q "_ARNv4impl8Box_i64_5Score5score" $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/trait-object-generic-impl-dispatch.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.elf; $(BUILD_DIR)/traits/trait-object-generic-impl-dispatch.elf; code=$$?; test $$code -eq 11; fi
	$(TARGET) --freestanding tests/ok/trait-bound-static-dispatch.ari -o $(BUILD_DIR)/traits/trait-bound-static-dispatch.raw
	$(BUILD_DIR)/traits/trait-bound-static-dispatch.raw; code=$$?; test $$code -eq 6
	$(TARGET) --freestanding tests/ok/trait-bound-generic-args.ari -o $(BUILD_DIR)/traits/trait-bound-generic-args.raw
	$(BUILD_DIR)/traits/trait-bound-generic-args.raw; code=$$?; test $$code -eq 11
	$(TARGET) tests/ok/trait-bound-static-dispatch.ari --emit-llvm $(BUILD_DIR)/traits/trait-bound-static-dispatch.ll
	grep -q "LeftScore" $(BUILD_DIR)/traits/trait-bound-static-dispatch.ll
	$(TARGET) tests/ok/generic-trait-impl.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-impl.ll
	grep -q "Box_i64_" $(BUILD_DIR)/traits/generic-trait-impl.ll
	grep -q "Box_bool_" $(BUILD_DIR)/traits/generic-trait-impl.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-trait-impl.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-impl.elf; $(BUILD_DIR)/traits/generic-trait-impl.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/ok/generic-impl-bounds.ari --emit-llvm $(BUILD_DIR)/traits/generic-impl-bounds.ll
	grep -q "WrappedScore" $(BUILD_DIR)/traits/generic-impl-bounds.ll
	grep -q "Box_i64_" $(BUILD_DIR)/traits/generic-impl-bounds.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-impl-bounds.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-impl-bounds.elf; $(BUILD_DIR)/traits/generic-impl-bounds.elf; code=$$?; test $$code -eq 23; fi
	$(TARGET) tests/ok/trait-associated-call.ari --emit-llvm $(BUILD_DIR)/traits/trait-associated-call.ll
	grep -q "Build" $(BUILD_DIR)/traits/trait-associated-call.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/trait-associated-call.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/trait-associated-call.elf; $(BUILD_DIR)/traits/trait-associated-call.elf; code=$$?; test $$code -eq 9; fi
	$(TARGET) tests/ok/generic-trait-associated.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-associated.ll
	grep -q "Box_i64_" $(BUILD_DIR)/traits/generic-trait-associated.ll
	grep -q "Box_bool_" $(BUILD_DIR)/traits/generic-trait-associated.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-trait-associated.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-associated.elf; $(BUILD_DIR)/traits/generic-trait-associated.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/ok/generic-trait-method.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-method.ll
	grep -q "keep__G_i64" $(BUILD_DIR)/traits/generic-trait-method.ll
	grep -q "keep__G_bool" $(BUILD_DIR)/traits/generic-trait-method.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-trait-method.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-method.elf; $(BUILD_DIR)/traits/generic-trait-method.elf; code=$$?; test $$code -eq 30; fi
	$(TARGET) tests/ok/generic-trait-method-bound.ari --emit-llvm $(BUILD_DIR)/traits/generic-trait-method-bound.ll
	grep -q "Score" $(BUILD_DIR)/traits/generic-trait-method-bound.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-trait-method-bound.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/generic-trait-method-bound.elf; $(BUILD_DIR)/traits/generic-trait-method-bound.elf; code=$$?; test $$code -eq 45; fi
	$(TARGET) tests/ok/iterator-next-surface.ari --emit-llvm $(BUILD_DIR)/traits/iterator-next-surface.ll
	grep -q "Cursor.*next" $(BUILD_DIR)/traits/iterator-next-surface.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/iterator-next-surface.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/traits/iterator-next-surface.elf; $(BUILD_DIR)/traits/iterator-next-surface.elf; code=$$?; test $$code -eq 37; fi
	if $(TARGET) tests/errors/trait-drop-missing-method.ari -o $(BUILD_DIR)/traits/trait-drop-missing-method.elf >$(BUILD_DIR)/traits/trait-drop-missing-method.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'std::Drop' for i64 is missing method 'drop'" $(BUILD_DIR)/traits/trait-drop-missing-method.err
	if $(TARGET) tests/errors/unknown-trait-impl.ari -o $(BUILD_DIR)/traits/unknown-trait-impl.elf >$(BUILD_DIR)/traits/unknown-trait-impl.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'MissingTrait'" $(BUILD_DIR)/traits/unknown-trait-impl.err
	if $(TARGET) tests/errors/trait-object-unknown.ari -o $(BUILD_DIR)/traits/trait-object-unknown.elf >$(BUILD_DIR)/traits/trait-object-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'Missing' in trait object type" $(BUILD_DIR)/traits/trait-object-unknown.err
	if $(TARGET) tests/errors/trait-object-arity.ari -o $(BUILD_DIR)/traits/trait-object-arity.elf >$(BUILD_DIR)/traits/trait-object-arity.err 2>&1; then exit 1; fi
	grep -q "trait 'Compare' expects 1 type argument" $(BUILD_DIR)/traits/trait-object-arity.err
	if $(TARGET) tests/errors/trait-object-qualifier.ari -o $(BUILD_DIR)/traits/trait-object-qualifier.elf >$(BUILD_DIR)/traits/trait-object-qualifier.err 2>&1; then exit 1; fi
	grep -q "trait object ownership qualifiers are planned" $(BUILD_DIR)/traits/trait-object-qualifier.err
	if $(TARGET) tests/errors/trait-object-value-planned.ari -o $(BUILD_DIR)/traits/trait-object-value-planned.elf >$(BUILD_DIR)/traits/trait-object-value-planned.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected dyn Score, got i64" $(BUILD_DIR)/traits/trait-object-value-planned.err
	if $(TARGET) tests/errors/trait-object-coerce-missing-impl.ari -o $(BUILD_DIR)/traits/trait-object-coerce-missing-impl.elf >$(BUILD_DIR)/traits/trait-object-coerce-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type i64 does not implement trait 'Score' for dyn conversion" $(BUILD_DIR)/traits/trait-object-coerce-missing-impl.err
	if $(TARGET) tests/errors/trait-object-upcast.ari -o $(BUILD_DIR)/traits/trait-object-upcast.elf >$(BUILD_DIR)/traits/trait-object-upcast.err 2>&1; then exit 1; fi
	grep -q "trait object upcasts are not supported; convert from a concrete value with as dyn Trait\\[...\\]" $(BUILD_DIR)/traits/trait-object-upcast.err
	if $(TARGET) tests/errors/trait-object-generic-method-conversion.ari -o $(BUILD_DIR)/traits/trait-object-generic-method-conversion.elf >$(BUILD_DIR)/traits/trait-object-generic-method-conversion.err 2>&1; then exit 1; fi
	grep -q "trait object method 'keep' is generic and is not object-safe" $(BUILD_DIR)/traits/trait-object-generic-method-conversion.err
	if $(TARGET) tests/errors/trait-object-generic-method-call.ari -o $(BUILD_DIR)/traits/trait-object-generic-method-call.elf >$(BUILD_DIR)/traits/trait-object-generic-method-call.err 2>&1; then exit 1; fi
	grep -q "trait object method 'keep' is generic and is not object-safe" $(BUILD_DIR)/traits/trait-object-generic-method-call.err
	if $(TARGET) tests/errors/iterable-trait-arity.ari -o $(BUILD_DIR)/traits/iterable-trait-arity.elf >$(BUILD_DIR)/traits/iterable-trait-arity.err 2>&1; then exit 1; fi
	grep -q "trait 'std::Iterable' expects 1 type argument" $(BUILD_DIR)/traits/iterable-trait-arity.err
	if $(TARGET) tests/errors/duplicate-trait-impl.ari -o $(BUILD_DIR)/traits/duplicate-trait-impl.elf >$(BUILD_DIR)/traits/duplicate-trait-impl.err 2>&1; then exit 1; fi
	grep -q "duplicate impl of trait 'std::Iterable' for Vec\\[i64\\]" $(BUILD_DIR)/traits/duplicate-trait-impl.err
	if $(TARGET) tests/errors/trait-duplicate-decl.ari -o $(BUILD_DIR)/traits/trait-duplicate-decl.elf >$(BUILD_DIR)/traits/trait-duplicate-decl.err 2>&1; then exit 1; fi
	grep -q "duplicate trait 'Show'" $(BUILD_DIR)/traits/trait-duplicate-decl.err
	if $(TARGET) tests/errors/trait-duplicate-method.ari -o $(BUILD_DIR)/traits/trait-duplicate-method.elf >$(BUILD_DIR)/traits/trait-duplicate-method.err 2>&1; then exit 1; fi
	grep -q "duplicate method 'show' in trait 'Show'" $(BUILD_DIR)/traits/trait-duplicate-method.err
	if $(TARGET) tests/errors/trait-impl-missing-method.ari -o $(BUILD_DIR)/traits/trait-impl-missing-method.elf >$(BUILD_DIR)/traits/trait-impl-missing-method.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'Show' for i64 is missing method 'show'" $(BUILD_DIR)/traits/trait-impl-missing-method.err
	if $(TARGET) tests/errors/trait-impl-extra-method.ari -o $(BUILD_DIR)/traits/trait-impl-extra-method.elf >$(BUILD_DIR)/traits/trait-impl-extra-method.err 2>&1; then exit 1; fi
	grep -q "method 'debug' is not declared by trait 'Show'" $(BUILD_DIR)/traits/trait-impl-extra-method.err
	if $(TARGET) tests/errors/trait-impl-param-mismatch.ari -o $(BUILD_DIR)/traits/trait-impl-param-mismatch.elf >$(BUILD_DIR)/traits/trait-impl-param-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'eq' parameter 2 type mismatch: expected i64, got bool" $(BUILD_DIR)/traits/trait-impl-param-mismatch.err
	if $(TARGET) tests/errors/trait-impl-return-mismatch.ari -o $(BUILD_DIR)/traits/trait-impl-return-mismatch.elf >$(BUILD_DIR)/traits/trait-impl-return-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'show' return type mismatch: expected i64, got bool" $(BUILD_DIR)/traits/trait-impl-return-mismatch.err
	if $(TARGET) tests/errors/generic-trait-method-count.ari -o $(BUILD_DIR)/traits/generic-trait-method-count.elf >$(BUILD_DIR)/traits/generic-trait-method-count.err 2>&1; then exit 1; fi
	grep -q "method 'keep' generic parameter count mismatch: trait expects 1, impl has 0" $(BUILD_DIR)/traits/generic-trait-method-count.err
	if $(TARGET) tests/errors/generic-trait-method-bound-missing.ari -o $(BUILD_DIR)/traits/generic-trait-method-bound-missing.elf >$(BUILD_DIR)/traits/generic-trait-method-bound-missing.err 2>&1; then exit 1; fi
	grep -q "method 'keep' generic parameter 1 bound mismatch: expected Score, got none" $(BUILD_DIR)/traits/generic-trait-method-bound-missing.err
	if $(TARGET) tests/errors/generic-trait-method-bound-mismatch.ari -o $(BUILD_DIR)/traits/generic-trait-method-bound-mismatch.elf >$(BUILD_DIR)/traits/generic-trait-method-bound-mismatch.err 2>&1; then exit 1; fi
	grep -q "method 'keep' generic parameter 1 bound mismatch: expected Score, got Other" $(BUILD_DIR)/traits/generic-trait-method-bound-mismatch.err
	if $(TARGET) tests/errors/trait-method-unknown.ari -o $(BUILD_DIR)/traits/trait-method-unknown.elf >$(BUILD_DIR)/traits/trait-method-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown method 'missing' for type i64" $(BUILD_DIR)/traits/trait-method-unknown.err
	if $(TARGET) tests/errors/trait-method-ambiguous.ari -o $(BUILD_DIR)/traits/trait-method-ambiguous.elf >$(BUILD_DIR)/traits/trait-method-ambiguous.err 2>&1; then exit 1; fi
	grep -q "method call 'score' for type i64 is ambiguous" $(BUILD_DIR)/traits/trait-method-ambiguous.err
	if $(TARGET) tests/errors/trait-associated-ambiguous.ari -o $(BUILD_DIR)/traits/trait-associated-ambiguous.elf >$(BUILD_DIR)/traits/trait-associated-ambiguous.err 2>&1; then exit 1; fi
	grep -q "associated function 'pick' for type Box is ambiguous" $(BUILD_DIR)/traits/trait-associated-ambiguous.err
	if $(TARGET) tests/errors/trait-bound-missing-impl.ari -o $(BUILD_DIR)/traits/trait-bound-missing-impl.elf >$(BUILD_DIR)/traits/trait-bound-missing-impl.err 2>&1; then exit 1; fi
	grep -q "type i64 does not implement trait 'NeedsScore' required by generic parameter 'T'" $(BUILD_DIR)/traits/trait-bound-missing-impl.err
	if $(TARGET) tests/errors/generic-impl-bound-missing.ari -o $(BUILD_DIR)/traits/generic-impl-bound-missing.elf >$(BUILD_DIR)/traits/generic-impl-bound-missing.err 2>&1; then exit 1; fi
	grep -q "type i64 does not implement trait 'Score' required by impl generic parameter 'T'" $(BUILD_DIR)/traits/generic-impl-bound-missing.err
	if $(TARGET) tests/errors/generic-impl-coherence-concrete.ari -o $(BUILD_DIR)/traits/generic-impl-coherence-concrete.elf >$(BUILD_DIR)/traits/generic-impl-coherence-concrete.err 2>&1; then exit 1; fi
	grep -q "impl of trait 'Score' for Box\\[i64\\] overlaps generic impl of trait 'Score' for Box\\[T\\]" $(BUILD_DIR)/traits/generic-impl-coherence-concrete.err
	if $(TARGET) tests/errors/generic-impl-coherence-generic.ari -o $(BUILD_DIR)/traits/generic-impl-coherence-generic.elf >$(BUILD_DIR)/traits/generic-impl-coherence-generic.err 2>&1; then exit 1; fi
	grep -q "generic impl of trait 'Pick' for Pair\\[First, Second\\] overlaps generic impl of trait 'Pick' for Pair\\[T, T\\]" $(BUILD_DIR)/traits/generic-impl-coherence-generic.err
	if $(TARGET) tests/errors/trait-bound-unknown.ari -o $(BUILD_DIR)/traits/trait-bound-unknown.elf >$(BUILD_DIR)/traits/trait-bound-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown trait bound 'MissingTrait'" $(BUILD_DIR)/traits/trait-bound-unknown.err

check-structs: $(TARGET)
	mkdir -p $(BUILD_DIR)/structs
	$(TARGET) --freestanding tests/ok/struct-surface.ari -o $(BUILD_DIR)/structs/struct-surface.raw
	$(BUILD_DIR)/structs/struct-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/struct-module-surface.ari -o $(BUILD_DIR)/structs/struct-module-surface.raw
	$(BUILD_DIR)/structs/struct-module-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/adt-enum-patterns.ari -o $(BUILD_DIR)/structs/adt-enum-patterns.raw
	$(BUILD_DIR)/structs/adt-enum-patterns.raw; code=$$?; test $$code -eq 7
	$(TARGET) --freestanding tests/ok/generic-adt-surface.ari -o $(BUILD_DIR)/structs/generic-adt-surface.raw
	$(BUILD_DIR)/structs/generic-adt-surface.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/generic-struct-infer.ari -o $(BUILD_DIR)/structs/generic-struct-infer.raw
	$(BUILD_DIR)/structs/generic-struct-infer.raw; code=$$?; test $$code -eq 40
	$(TARGET) tests/ok/generic-struct-infer.ari --emit-llvm $(BUILD_DIR)/structs/generic-struct-infer.ll
	grep -q "insertvalue { i64 }" $(BUILD_DIR)/structs/generic-struct-infer.ll
	grep -q "insertvalue { i64, i1 }" $(BUILD_DIR)/structs/generic-struct-infer.ll
	$(TARGET) tests/ok/generic-impl-method.ari --emit-llvm $(BUILD_DIR)/structs/generic-impl-method.ll
	grep -q "Box_i64_" $(BUILD_DIR)/structs/generic-impl-method.ll
	grep -q "Box_bool_" $(BUILD_DIR)/structs/generic-impl-method.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-impl-method.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-impl-method.elf; $(BUILD_DIR)/structs/generic-impl-method.elf; code=$$?; test $$code -eq 21; fi
	$(TARGET) tests/ok/generic-method-type-args.ari --emit-llvm $(BUILD_DIR)/structs/generic-method-type-args.ll
	grep -q "replace__G_i64" $(BUILD_DIR)/structs/generic-method-type-args.ll
	grep -q "score_with__G_i64" $(BUILD_DIR)/structs/generic-method-type-args.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-method-type-args.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-method-type-args.elf; $(BUILD_DIR)/structs/generic-method-type-args.elf; code=$$?; test $$code -eq 22; fi
	$(TARGET) tests/ok/generic-method-infer.ari --emit-llvm $(BUILD_DIR)/structs/generic-method-infer.ll
	grep -q "replace__G_i64" $(BUILD_DIR)/structs/generic-method-infer.ll
	grep -q "score_with__G_i64" $(BUILD_DIR)/structs/generic-method-infer.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-method-infer.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-method-infer.elf; $(BUILD_DIR)/structs/generic-method-infer.elf; code=$$?; test $$code -eq 22; fi
	$(TARGET) --freestanding tests/ok/tuple-layout.ari -o $(BUILD_DIR)/structs/tuple-layout.raw
	$(BUILD_DIR)/structs/tuple-layout.raw; code=$$?; test $$code -eq 13
	$(TARGET) tests/ok/tuple-layout.ari --emit-llvm $(BUILD_DIR)/structs/tuple-layout.ll
	grep -q "insertvalue { i64, i1 }" $(BUILD_DIR)/structs/tuple-layout.ll
	grep -q "extractvalue { i64, i1 }" $(BUILD_DIR)/structs/tuple-layout.ll
	$(TARGET) --freestanding tests/ok/struct-lowering.ari -o $(BUILD_DIR)/structs/struct-lowering.raw
	$(BUILD_DIR)/structs/struct-lowering.raw; code=$$?; test $$code -eq 18
	$(TARGET) tests/ok/struct-lowering.ari --emit-llvm $(BUILD_DIR)/structs/struct-lowering.ll
	grep -q "insertvalue { i64, i64, i1 }" $(BUILD_DIR)/structs/struct-lowering.ll
	grep -q "extractvalue { i64, i64, i1 }" $(BUILD_DIR)/structs/struct-lowering.ll
	$(TARGET) tests/ok/impl-associated-new.ari --emit-llvm $(BUILD_DIR)/structs/impl-associated-new.ll
	grep -q "_ARNv4impl5Point8inherent3new" $(BUILD_DIR)/structs/impl-associated-new.ll
	grep -q "call { i64, i64 }" $(BUILD_DIR)/structs/impl-associated-new.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/impl-associated-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/impl-associated-new.elf; $(BUILD_DIR)/structs/impl-associated-new.elf; code=$$?; test $$code -eq 9; fi
	$(TARGET) tests/ok/generic-associated-new.ari --emit-llvm $(BUILD_DIR)/structs/generic-associated-new.ll
	grep -q "Box_i64_" $(BUILD_DIR)/structs/generic-associated-new.ll
	grep -q "Box_bool_" $(BUILD_DIR)/structs/generic-associated-new.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-associated-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-associated-new.elf; $(BUILD_DIR)/structs/generic-associated-new.elf; code=$$?; test $$code -eq 22; fi
	$(TARGET) tests/ok/generic-associated-method-type-args.ari --emit-llvm $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	grep -q "make__G_i64" $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	grep -q "make__G_bool" $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	grep -q "scored__G_i64" $(BUILD_DIR)/structs/generic-associated-method-type-args.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/generic-associated-method-type-args.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/structs/generic-associated-method-type-args.elf; $(BUILD_DIR)/structs/generic-associated-method-type-args.elf; code=$$?; test $$code -eq 22; fi
	if $(TARGET) tests/errors/struct-duplicate-decl.ari -o $(BUILD_DIR)/structs/struct-duplicate-decl.elf >$(BUILD_DIR)/structs/struct-duplicate-decl.err 2>&1; then exit 1; fi
	grep -q "duplicate struct 'Point'" $(BUILD_DIR)/structs/struct-duplicate-decl.err
	if $(TARGET) tests/errors/struct-field-semicolon.ari -o $(BUILD_DIR)/structs/struct-field-semicolon.elf >$(BUILD_DIR)/structs/struct-field-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected , after struct field" $(BUILD_DIR)/structs/struct-field-semicolon.err
	if $(TARGET) tests/errors/tuple-struct-semicolon.ari -o $(BUILD_DIR)/structs/tuple-struct-semicolon.elf >$(BUILD_DIR)/structs/tuple-struct-semicolon.err 2>&1; then exit 1; fi
	grep -q "tuple struct declarations do not end with ;" $(BUILD_DIR)/structs/tuple-struct-semicolon.err
	if $(TARGET) tests/errors/enum-case-semicolon.ari -o $(BUILD_DIR)/structs/enum-case-semicolon.elf >$(BUILD_DIR)/structs/enum-case-semicolon.err 2>&1; then exit 1; fi
	grep -q "expected , after enum case" $(BUILD_DIR)/structs/enum-case-semicolon.err
	if $(TARGET) tests/errors/struct-duplicate-field.ari -o $(BUILD_DIR)/structs/struct-duplicate-field.elf >$(BUILD_DIR)/structs/struct-duplicate-field.err 2>&1; then exit 1; fi
	grep -q "duplicate field 'x' in struct 'Point'" $(BUILD_DIR)/structs/struct-duplicate-field.err
	if $(TARGET) tests/errors/struct-unknown-field-type.ari -o $(BUILD_DIR)/structs/struct-unknown-field-type.elf >$(BUILD_DIR)/structs/struct-unknown-field-type.err 2>&1; then exit 1; fi
	grep -q "unsupported executable type 'Missing'" $(BUILD_DIR)/structs/struct-unknown-field-type.err
	if $(TARGET) tests/errors/struct-generic-arity.ari -o $(BUILD_DIR)/structs/struct-generic-arity.elf >$(BUILD_DIR)/structs/struct-generic-arity.err 2>&1; then exit 1; fi
	grep -q "struct 'Pair' expects 1 type argument" $(BUILD_DIR)/structs/struct-generic-arity.err
	if $(TARGET) tests/errors/generic-struct-infer-conflict.ari -o $(BUILD_DIR)/structs/generic-struct-infer-conflict.elf >$(BUILD_DIR)/structs/generic-struct-infer-conflict.err 2>&1; then exit 1; fi
	grep -q "conflicting inferred type for generic parameter 'T': i64 and bool" $(BUILD_DIR)/structs/generic-struct-infer-conflict.err
	if $(TARGET) tests/errors/generic-struct-infer-missing.ari -o $(BUILD_DIR)/structs/generic-struct-infer-missing.elf >$(BUILD_DIR)/structs/generic-struct-infer-missing.err 2>&1; then exit 1; fi
	grep -q "cannot infer type argument 'T' for struct 'Phantom'" $(BUILD_DIR)/structs/generic-struct-infer-missing.err
	if $(TARGET) tests/errors/generic-associated-arity.ari -o $(BUILD_DIR)/structs/generic-associated-arity.elf >$(BUILD_DIR)/structs/generic-associated-arity.err 2>&1; then exit 1; fi
	grep -q "generic associated function expects 1 type argument" $(BUILD_DIR)/structs/generic-associated-arity.err
	if $(TARGET) tests/errors/generic-method-arity.ari -o $(BUILD_DIR)/structs/generic-method-arity.elf >$(BUILD_DIR)/structs/generic-method-arity.err 2>&1; then exit 1; fi
	grep -q "generic method 'replace' expects 1 type argument" $(BUILD_DIR)/structs/generic-method-arity.err
	if $(TARGET) tests/errors/generic-method-uninferred.ari -o $(BUILD_DIR)/structs/generic-method-uninferred.elf >$(BUILD_DIR)/structs/generic-method-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'U' could not be inferred for method 'make'" $(BUILD_DIR)/structs/generic-method-uninferred.err
	if $(TARGET) tests/errors/generic-associated-method-uninferred.ari -o $(BUILD_DIR)/structs/generic-associated-method-uninferred.elf >$(BUILD_DIR)/structs/generic-associated-method-uninferred.err 2>&1; then exit 1; fi
	grep -q "generic type 'U' could not be inferred for associated function 'make'" $(BUILD_DIR)/structs/generic-associated-method-uninferred.err
	if $(TARGET) tests/errors/private-module-struct-type.ari -o $(BUILD_DIR)/structs/private-module-struct-type.elf >$(BUILD_DIR)/structs/private-module-struct-type.err 2>&1; then exit 1; fi
	grep -q "struct 'Hidden::Secret' is not public" $(BUILD_DIR)/structs/private-module-struct-type.err
	if $(TARGET) tests/errors/struct-field-immutable.ari -o $(BUILD_DIR)/structs/struct-field-immutable.elf >$(BUILD_DIR)/structs/struct-field-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable field 'x' of struct 'Point'" $(BUILD_DIR)/structs/struct-field-immutable.err
	if $(TARGET) tests/errors/struct-field-immutable-binding.ari -o $(BUILD_DIR)/structs/struct-field-immutable-binding.elf >$(BUILD_DIR)/structs/struct-field-immutable-binding.err 2>&1; then exit 1; fi
	grep -q "cannot assign to field of immutable binding 'meter'" $(BUILD_DIR)/structs/struct-field-immutable-binding.err
	if $(TARGET) tests/errors/struct-field-unknown.ari -o $(BUILD_DIR)/structs/struct-field-unknown.elf >$(BUILD_DIR)/structs/struct-field-unknown.err 2>&1; then exit 1; fi
	grep -q "struct 'Point' has no field 'y'" $(BUILD_DIR)/structs/struct-field-unknown.err
	if $(TARGET) tests/errors/impl-associated-private.ari -o $(BUILD_DIR)/structs/impl-associated-private.elf >$(BUILD_DIR)/structs/impl-associated-private.err 2>&1; then exit 1; fi
	grep -q "method 'new' for type Factory::Point is not public" $(BUILD_DIR)/structs/impl-associated-private.err
	if $(TARGET) tests/errors/impl-associated-unknown.ari -o $(BUILD_DIR)/structs/impl-associated-unknown.elf >$(BUILD_DIR)/structs/impl-associated-unknown.err 2>&1; then exit 1; fi
	grep -q "unknown associated function 'missing' for type Point" $(BUILD_DIR)/structs/impl-associated-unknown.err
	if $(TARGET) tests/errors/tuple-index-out-of-range.ari -o $(BUILD_DIR)/structs/tuple-index-out-of-range.elf >$(BUILD_DIR)/structs/tuple-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "tuple index 2 is out of range for (i64, bool)" $(BUILD_DIR)/structs/tuple-index-out-of-range.err

check-variables: $(TARGET)
	mkdir -p $(BUILD_DIR)/variables
	$(TARGET) --freestanding tests/ok/variables-basic.ari -o $(BUILD_DIR)/variables/variables-basic.raw
	$(BUILD_DIR)/variables/variables-basic.raw; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/variables-widths.ari -o $(BUILD_DIR)/variables/variables-widths.raw
	$(BUILD_DIR)/variables/variables-widths.raw; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/freestanding-scalar-width-local.ari -o $(BUILD_DIR)/variables/freestanding-scalar-width-local.raw
	$(BUILD_DIR)/variables/freestanding-scalar-width-local.raw; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/freestanding-scalar-stack-pack.ari -o $(BUILD_DIR)/variables/freestanding-scalar-stack-pack.raw
	$(BUILD_DIR)/variables/freestanding-scalar-stack-pack.raw; code=$$?; test $$code -eq 10
	objdump -d $(BUILD_DIR)/variables/freestanding-scalar-stack-pack.raw >$(BUILD_DIR)/variables/freestanding-scalar-stack-pack.asm
	grep -q 'sub.*0x10,%rsp' $(BUILD_DIR)/variables/freestanding-scalar-stack-pack.asm
	$(TARGET) --freestanding tests/ok/variables-match-binding.ari -o $(BUILD_DIR)/variables/variables-match-binding.raw
	$(BUILD_DIR)/variables/variables-match-binding.raw; code=$$?; test $$code -eq 42
	$(TARGET) --freestanding tests/ok/variables-tuple-pattern.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern.raw
	$(BUILD_DIR)/variables/variables-tuple-pattern.raw; code=$$?; test $$code -eq 38
	$(TARGET) --freestanding tests/ok/unit-empty-tuple.ari -o $(BUILD_DIR)/variables/unit-empty-tuple.raw
	$(BUILD_DIR)/variables/unit-empty-tuple.raw; code=$$?; test $$code -eq 10
	$(TARGET) --freestanding tests/ok/variables-struct-pattern.ari -o $(BUILD_DIR)/variables/variables-struct-pattern.raw
	$(BUILD_DIR)/variables/variables-struct-pattern.raw; code=$$?; test $$code -eq 25
	$(TARGET) --freestanding tests/ok/variables-enum-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-enum-pattern-binding.raw
	$(BUILD_DIR)/variables/variables-enum-pattern-binding.raw; code=$$?; test $$code -eq 13
	$(TARGET) --freestanding tests/ok/variables-enum-or-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.raw
	$(BUILD_DIR)/variables/variables-enum-or-pattern-binding.raw; code=$$?; test $$code -eq 33
	$(TARGET) --freestanding tests/ok/ownership-aggregate-field-access.ari -o $(BUILD_DIR)/variables/ownership-aggregate-field-access.raw
	$(BUILD_DIR)/variables/ownership-aggregate-field-access.raw; code=$$?; test $$code -eq 7
	$(TARGET) --freestanding tests/ok/ownership-aggregate-field-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-field-move.raw
	$(BUILD_DIR)/variables/ownership-aggregate-field-move.raw; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/ownership-aggregate-nested-field-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.raw
	$(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.raw; code=$$?; test $$code -eq 13
	$(TARGET) --freestanding tests/ok/ownership-array-element-move.ari -o $(BUILD_DIR)/variables/ownership-array-element-move.raw
	$(BUILD_DIR)/variables/ownership-array-element-move.raw; code=$$?; test $$code -eq 6
	$(TARGET) --freestanding tests/ok/array-fixed.ari -o $(BUILD_DIR)/variables/array-fixed.raw
	$(BUILD_DIR)/variables/array-fixed.raw; code=$$?; test $$code -eq 4
	$(TARGET) --freestanding tests/ok/array-literal-default.ari -o $(BUILD_DIR)/variables/array-literal-default.raw
	$(BUILD_DIR)/variables/array-literal-default.raw; code=$$?; test $$code -eq 14
	$(TARGET) --freestanding tests/ok/array-dynamic-index.ari -o $(BUILD_DIR)/variables/array-dynamic-index.raw
	$(BUILD_DIR)/variables/array-dynamic-index.raw; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/array-len-literal.ari -o $(BUILD_DIR)/variables/array-len-literal.raw
	$(BUILD_DIR)/variables/array-len-literal.raw; code=$$?; test $$code -eq 4
	$(TARGET) tests/ok/variables-basic.ari --emit-llvm $(BUILD_DIR)/variables/variables-basic.ll
	grep -q "alloca i64" $(BUILD_DIR)/variables/variables-basic.ll
	grep -q "alloca i1" $(BUILD_DIR)/variables/variables-basic.ll
	$(TARGET) tests/ok/variables-widths.ari --emit-llvm $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i8" $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i16" $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i32" $(BUILD_DIR)/variables/variables-widths.ll
	grep -q "alloca i64" $(BUILD_DIR)/variables/variables-widths.ll
	$(TARGET) tests/ok/variables-tuple-pattern.ari --emit-llvm $(BUILD_DIR)/variables/variables-tuple-pattern.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/variables-tuple-pattern.ll
	grep -q "extractvalue { { i64, i1 }, i64 }" $(BUILD_DIR)/variables/variables-tuple-pattern.ll
	$(TARGET) tests/ok/ir-builder-smoke.ari --emit-llvm $(BUILD_DIR)/variables/ir-builder-smoke.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/ir-builder-smoke.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/ir-builder-smoke.ll
	$(TARGET) tests/ok/unit-empty-tuple.ari --emit-llvm $(BUILD_DIR)/variables/unit-empty-tuple.ll
	grep -q "define {}" $(BUILD_DIR)/variables/unit-empty-tuple.ll
	grep -q "store {} zeroinitializer" $(BUILD_DIR)/variables/unit-empty-tuple.ll
	$(TARGET) tests/ok/variables-struct-pattern.ari --emit-llvm $(BUILD_DIR)/variables/variables-struct-pattern.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/variables/variables-struct-pattern.ll
	grep -q "extractvalue { { i64, i64 }, { i64, i64 } }" $(BUILD_DIR)/variables/variables-struct-pattern.ll
	$(TARGET) tests/ok/variables-enum-pattern-binding.ari --emit-llvm $(BUILD_DIR)/variables/variables-enum-pattern-binding.ll
	grep -q "match.arm" $(BUILD_DIR)/variables/variables-enum-pattern-binding.ll
	$(TARGET) tests/ok/variables-enum-or-pattern-binding.ari --emit-llvm $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.ll
	grep -q "match.arm" $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.ll
	grep -q "store i64" $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.ll
	$(TARGET) tests/ok/variables-enum-pattern-binding-trap.ari --emit-llvm $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.ll
	grep -q "@ari_builtin_panic" $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.ll
	$(TARGET) tests/ok/variables-host-scalars.ari --emit-llvm $(BUILD_DIR)/variables/variables-host-scalars.ll
	grep -q "alloca ptr" $(BUILD_DIR)/variables/variables-host-scalars.ll
	grep -q "alloca double" $(BUILD_DIR)/variables/variables-host-scalars.ll
	grep -q "store double 10.000000" $(BUILD_DIR)/variables/variables-host-scalars.ll
	$(TARGET) tests/ok/ownership-aggregate-storage.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-storage.ll
	grep -q "alloca { i64, i64 }" $(BUILD_DIR)/variables/ownership-aggregate-storage.ll
	grep -q "alloca { ptr, ptr }" $(BUILD_DIR)/variables/ownership-aggregate-storage.ll
	$(TARGET) tests/ok/drop-aggregate-fields.ari --emit-llvm $(BUILD_DIR)/variables/drop-aggregate-fields.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/drop-aggregate-fields.ll) -eq 2
	$(TARGET) tests/ok/drop-aggregate-moved-field.ari --emit-llvm $(BUILD_DIR)/variables/drop-aggregate-moved-field.ll
	test $$(grep -c 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/variables/drop-aggregate-moved-field.ll) -eq 2
	$(TARGET) tests/ok/ownership-aggregate-field-access.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-field-access.ll
	grep -q "store i64" $(BUILD_DIR)/variables/ownership-aggregate-field-access.ll
	$(TARGET) tests/ok/ownership-aggregate-field-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-field-move.ll
	grep -q "extractvalue" $(BUILD_DIR)/variables/ownership-aggregate-field-move.ll
	$(TARGET) tests/ok/ownership-aggregate-nested-field-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.ll
	grep -q "extractvalue" $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.ll
	$(TARGET) tests/ok/ownership-array-element-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-array-element-move.ll
	grep -q "\\[2 x i64\\]" $(BUILD_DIR)/variables/ownership-array-element-move.ll
	$(TARGET) tests/ok/ownership-vector-element-move.ari --emit-llvm $(BUILD_DIR)/variables/ownership-vector-element-move.ll
	grep -q "getelementptr inbounds { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/ownership-vector-element-move.ll
	$(TARGET) tests/ok/array-fixed.ari --emit-llvm $(BUILD_DIR)/variables/array-fixed.ll
	grep -q "\\[3 x i64\\]" $(BUILD_DIR)/variables/array-fixed.ll
	$(TARGET) tests/ok/array-literal-default.ari --emit-llvm $(BUILD_DIR)/variables/array-literal-default.ll
	grep -q "alloca \\[3 x i64\\]" $(BUILD_DIR)/variables/array-literal-default.ll
	$(TARGET) tests/ok/array-dynamic-index.ari --emit-llvm $(BUILD_DIR)/variables/array-dynamic-index.ll
	grep -q "getelementptr inbounds \\[3 x i64\\]" $(BUILD_DIR)/variables/array-dynamic-index.ll
	grep -q "array.bounds.fail" $(BUILD_DIR)/variables/array-dynamic-index.ll
	$(TARGET) tests/ok/vector-storage.ari --emit-llvm $(BUILD_DIR)/variables/vector-storage.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-storage.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-storage.ll
	$(TARGET) tests/ok/vector-storage-reassign.ari --emit-llvm $(BUILD_DIR)/variables/vector-storage-reassign.ll
	grep -q "getelementptr inbounds { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-storage-reassign.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-storage-reassign.ll
	$(TARGET) tests/ok/vector-empty-typed.ari --emit-llvm $(BUILD_DIR)/variables/vector-empty-typed.ll
	grep -q "store { i64, \\[0 x i64\\] }" $(BUILD_DIR)/variables/vector-empty-typed.ll
	grep -q "store { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-empty-typed.ll
	$(TARGET) tests/ok/vector-len.ari --emit-llvm $(BUILD_DIR)/variables/vector-len.ll
	grep -q "extractvalue { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-len.ll
	$(TARGET) tests/ok/vector-push-reserve.ari --emit-llvm $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "vector.push.full" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i64 0, ptr" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "vector.truncate.negative" $(BUILD_DIR)/variables/vector-push-reserve.ll
	grep -q "store i64 9, ptr" $(BUILD_DIR)/variables/vector-push-reserve.ll
	$(TARGET) tests/ok/vector-push-autogrow.ari --emit-llvm $(BUILD_DIR)/variables/vector-push-autogrow.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-push-autogrow.ll
	grep -q "alloca { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-push-autogrow.ll
	$(TARGET) tests/ok/vector-pop.ari --emit-llvm $(BUILD_DIR)/variables/vector-pop.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-pop.ll
	grep -q "vector.pop.empty" $(BUILD_DIR)/variables/vector-pop.ll
	$(TARGET) tests/ok/vector-first-last.ari --emit-llvm $(BUILD_DIR)/variables/vector-first-last.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-first-last.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-first-last.ll
	$(TARGET) tests/ok/vector-get.ari --emit-llvm $(BUILD_DIR)/variables/vector-get.ll
	grep -q "alloca { i64, \\[2 x i64\\] }" $(BUILD_DIR)/variables/vector-get.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-get.ll
	$(TARGET) tests/ok/vector-swap.ari --emit-llvm $(BUILD_DIR)/variables/vector-swap.ll
	grep -q "alloca { i64, \\[3 x i64\\] }" $(BUILD_DIR)/variables/vector-swap.ll
	grep -q "vector.bounds.fail" $(BUILD_DIR)/variables/vector-swap.ll
	$(TARGET) tests/ok/vector-remove.ari --emit-llvm $(BUILD_DIR)/variables/vector-remove.ll
	grep -q "alloca { i64, \\[4 x i64\\] }" $(BUILD_DIR)/variables/vector-remove.ll
	grep -q "vector.remove.body" $(BUILD_DIR)/variables/vector-remove.ll
	$(TARGET) tests/ok/vector-insert.ari --emit-llvm $(BUILD_DIR)/variables/vector-insert.ll
	grep -q "alloca { i64, \\[5 x i64\\] }" $(BUILD_DIR)/variables/vector-insert.ll
	grep -q "vector.insert.body" $(BUILD_DIR)/variables/vector-insert.ll
	$(TARGET) tests/ok/vector-search.ari --emit-llvm $(BUILD_DIR)/variables/vector-search.ll
	grep -q "vector.contains.body" $(BUILD_DIR)/variables/vector-search.ll
	grep -q "vector.index_of.body" $(BUILD_DIR)/variables/vector-search.ll
	grep -q "vector.count.body" $(BUILD_DIR)/variables/vector-search.ll
	$(TARGET) tests/ok/array-len-literal.ari --emit-llvm $(BUILD_DIR)/variables/array-len-literal.ll
	grep -q "ret i64 4" $(BUILD_DIR)/variables/array-len-literal.ll
	$(TARGET) tests/ok/field-borrow.ari --emit-llvm $(BUILD_DIR)/variables/field-borrow.ll
	grep -q "getelementptr inbounds { i64, i64 }" $(BUILD_DIR)/variables/field-borrow.ll
	$(TARGET) tests/ok/array-dynamic-index-trap.ari --emit-llvm $(BUILD_DIR)/variables/array-dynamic-index-trap.ll
	grep -q "array.bounds.fail" $(BUILD_DIR)/variables/array-dynamic-index-trap.ll
	$(TARGET) --freestanding tests/ok/array-dynamic-index-trap.ari -o $(BUILD_DIR)/variables/array-dynamic-index-trap.raw
	if $(BUILD_DIR)/variables/array-dynamic-index-trap.raw; then exit 1; fi
	$(TARGET) --freestanding tests/ok/variables-enum-pattern-binding-trap.ari -o $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.raw
	if $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.raw; then exit 1; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/variables-basic.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-basic.elf; $(BUILD_DIR)/variables/variables-basic.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ir-builder-smoke.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ir-builder-smoke.elf; $(BUILD_DIR)/variables/ir-builder-smoke.elf; code=$$?; test $$code -eq 16; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/unit-empty-tuple.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/unit-empty-tuple.elf; $(BUILD_DIR)/variables/unit-empty-tuple.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/variables-host-scalars.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-host-scalars.elf; $(BUILD_DIR)/variables/variables-host-scalars.elf; code=$$?; test $$code -eq 0; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ownership-aggregate-storage.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-storage.elf; $(BUILD_DIR)/variables/ownership-aggregate-storage.elf; code=$$?; test $$code -eq 47; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ownership-aggregate-field-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-field-access.elf; $(BUILD_DIR)/variables/ownership-aggregate-field-access.elf; code=$$?; test $$code -eq 7; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ownership-aggregate-field-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-field-move.elf; $(BUILD_DIR)/variables/ownership-aggregate-field-move.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ownership-aggregate-nested-field-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.elf; $(BUILD_DIR)/variables/ownership-aggregate-nested-field-move.elf; code=$$?; test $$code -eq 13; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ownership-array-element-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-array-element-move.elf; $(BUILD_DIR)/variables/ownership-array-element-move.elf; code=$$?; test $$code -eq 6; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ownership-vector-element-move.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/ownership-vector-element-move.elf; $(BUILD_DIR)/variables/ownership-vector-element-move.elf; code=$$?; test $$code -eq 8; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/array-fixed.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-fixed.elf; $(BUILD_DIR)/variables/array-fixed.elf; code=$$?; test $$code -eq 4; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/array-literal-default.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-literal-default.elf; $(BUILD_DIR)/variables/array-literal-default.elf; code=$$?; test $$code -eq 14; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/array-dynamic-index.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-dynamic-index.elf; $(BUILD_DIR)/variables/array-dynamic-index.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-storage.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-storage.elf; $(BUILD_DIR)/variables/vector-storage.elf; code=$$?; test $$code -eq 16; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-storage-reassign.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-storage-reassign.elf; $(BUILD_DIR)/variables/vector-storage-reassign.elf; code=$$?; test $$code -eq 14; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-empty-typed.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-empty-typed.elf; $(BUILD_DIR)/variables/vector-empty-typed.elf; code=$$?; test $$code -eq 3; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-len.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-len.elf; $(BUILD_DIR)/variables/vector-len.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-push-reserve.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-push-reserve.elf; $(BUILD_DIR)/variables/vector-push-reserve.elf; code=$$?; test $$code -eq 24; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-push-autogrow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-push-autogrow.elf; $(BUILD_DIR)/variables/vector-push-autogrow.elf; code=$$?; test $$code -eq 23; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-pop.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-pop.elf; $(BUILD_DIR)/variables/vector-pop.elf; code=$$?; test $$code -eq 26; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-first-last.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-first-last.elf; $(BUILD_DIR)/variables/vector-first-last.elf; code=$$?; test $$code -eq 20; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-get.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-get.elf; $(BUILD_DIR)/variables/vector-get.elf; code=$$?; test $$code -eq 26; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-swap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-swap.elf; $(BUILD_DIR)/variables/vector-swap.elf; code=$$?; test $$code -eq 24; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-remove.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-remove.elf; $(BUILD_DIR)/variables/vector-remove.elf; code=$$?; test $$code -eq 32; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-insert.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-insert.elf; $(BUILD_DIR)/variables/vector-insert.elf; code=$$?; test $$code -eq 29; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-search.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-search.elf; $(BUILD_DIR)/variables/vector-search.elf; code=$$?; test $$code -eq 10; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/field-borrow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/field-borrow.elf; $(BUILD_DIR)/variables/field-borrow.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/array-dynamic-index-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/array-dynamic-index-trap.elf; if $(BUILD_DIR)/variables/array-dynamic-index-trap.elf; then exit 1; fi; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/vector-storage-shrink-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/vector-storage-shrink-trap.elf; if $(BUILD_DIR)/variables/vector-storage-shrink-trap.elf; then exit 1; fi; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/variables-enum-pattern-binding.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-enum-pattern-binding.elf; $(BUILD_DIR)/variables/variables-enum-pattern-binding.elf; code=$$?; test $$code -eq 13; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/variables-enum-or-pattern-binding.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.elf; $(BUILD_DIR)/variables/variables-enum-or-pattern-binding.elf; code=$$?; test $$code -eq 33; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/variables-enum-pattern-binding-trap.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.elf; if $(BUILD_DIR)/variables/variables-enum-pattern-binding-trap.elf; then exit 1; fi; fi
	if $(TARGET) tests/errors/variables-assign-immutable.ari -o $(BUILD_DIR)/variables/variables-assign-immutable.elf >$(BUILD_DIR)/variables/variables-assign-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'value'" $(BUILD_DIR)/variables/variables-assign-immutable.err
	if $(TARGET) tests/errors/variables-local-shadowing.ari -o $(BUILD_DIR)/variables/variables-local-shadowing.elf >$(BUILD_DIR)/variables/variables-local-shadowing.err 2>&1; then exit 1; fi
	grep -q "local name 'value' shadows or redeclares an existing local" $(BUILD_DIR)/variables/variables-local-shadowing.err
	if $(TARGET) tests/errors/variables-local-shadowing-block.ari -o $(BUILD_DIR)/variables/variables-local-shadowing-block.elf >$(BUILD_DIR)/variables/variables-local-shadowing-block.err 2>&1; then exit 1; fi
	grep -q "local name 'value' shadows or redeclares an existing local" $(BUILD_DIR)/variables/variables-local-shadowing-block.err
	if $(TARGET) tests/errors/variables-assign-type-mismatch.ari -o $(BUILD_DIR)/variables/variables-assign-type-mismatch.elf >$(BUILD_DIR)/variables/variables-assign-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i32, got bool" $(BUILD_DIR)/variables/variables-assign-type-mismatch.err
	if $(TARGET) tests/errors/aggregate-borrow-source-assign.ari -o $(BUILD_DIR)/variables/aggregate-borrow-source-assign.elf >$(BUILD_DIR)/variables/aggregate-borrow-source-assign.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'number'" $(BUILD_DIR)/variables/aggregate-borrow-source-assign.err
	if $(TARGET) tests/errors/field-borrow-assign.ari -o $(BUILD_DIR)/variables/field-borrow-assign.elf >$(BUILD_DIR)/variables/field-borrow-assign.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed field 'pair.0'" $(BUILD_DIR)/variables/field-borrow-assign.err
	if $(TARGET) tests/errors/field-borrow-conflict.ari -o $(BUILD_DIR)/variables/field-borrow-conflict.elf >$(BUILD_DIR)/variables/field-borrow-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow already borrowed field 'pair.0'" $(BUILD_DIR)/variables/field-borrow-conflict.err
	if $(TARGET) tests/errors/aggregate-owner-undropped.ari -o $(BUILD_DIR)/variables/aggregate-owner-undropped.elf >$(BUILD_DIR)/variables/aggregate-owner-undropped.err 2>&1; then exit 1; fi
	grep -q "owning binding 'pair' must be moved or dropped before return" $(BUILD_DIR)/variables/aggregate-owner-undropped.err
	if $(TARGET) tests/errors/ownership-aggregate-field-use-after-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-field-use-after-move.elf >$(BUILD_DIR)/variables/ownership-aggregate-field-use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'holder.0'" $(BUILD_DIR)/variables/ownership-aggregate-field-use-after-move.err
	if $(TARGET) tests/errors/ownership-aggregate-partial-move.ari -o $(BUILD_DIR)/variables/ownership-aggregate-partial-move.elf >$(BUILD_DIR)/variables/ownership-aggregate-partial-move.err 2>&1; then exit 1; fi
	grep -q "cannot move partially moved owning binding 'holder'" $(BUILD_DIR)/variables/ownership-aggregate-partial-move.err
	if $(TARGET) tests/errors/ownership-temporary-field-move.ari -o $(BUILD_DIR)/variables/ownership-temporary-field-move.elf >$(BUILD_DIR)/variables/ownership-temporary-field-move.err 2>&1; then exit 1; fi
	grep -q "moving owned fields out of temporary aggregate values is not supported; bind the aggregate first" $(BUILD_DIR)/variables/ownership-temporary-field-move.err
	if $(TARGET) tests/errors/ownership-temporary-tuple-field-move.ari -o $(BUILD_DIR)/variables/ownership-temporary-tuple-field-move.elf >$(BUILD_DIR)/variables/ownership-temporary-tuple-field-move.err 2>&1; then exit 1; fi
	grep -q "moving owned fields out of temporary aggregate values is not supported; bind the aggregate first" $(BUILD_DIR)/variables/ownership-temporary-tuple-field-move.err
	if $(TARGET) tests/errors/ownership-temporary-array-element-move.ari -o $(BUILD_DIR)/variables/ownership-temporary-array-element-move.elf >$(BUILD_DIR)/variables/ownership-temporary-array-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements out of temporary values is not supported; bind the aggregate first" $(BUILD_DIR)/variables/ownership-temporary-array-element-move.err
	if $(TARGET) tests/errors/ownership-array-element-use-after-move.ari -o $(BUILD_DIR)/variables/ownership-array-element-use-after-move.elf >$(BUILD_DIR)/variables/ownership-array-element-use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved owned field 'values.0'" $(BUILD_DIR)/variables/ownership-array-element-use-after-move.err
	if $(TARGET) tests/errors/ownership-array-dynamic-element-move.ari -o $(BUILD_DIR)/variables/ownership-array-dynamic-element-move.elf >$(BUILD_DIR)/variables/ownership-array-dynamic-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements through dynamic indexes is not supported; use a constant index or move the whole aggregate" $(BUILD_DIR)/variables/ownership-array-dynamic-element-move.err
	if $(TARGET) tests/errors/ownership-vector-dynamic-element-move.ari -o $(BUILD_DIR)/variables/ownership-vector-dynamic-element-move.elf >$(BUILD_DIR)/variables/ownership-vector-dynamic-element-move.err 2>&1; then exit 1; fi
	grep -q "moving owning aggregate elements through dynamic indexes is not supported; use a constant index or move the whole aggregate" $(BUILD_DIR)/variables/ownership-vector-dynamic-element-move.err
	if $(TARGET) tests/errors/array-size-mismatch.ari -o $(BUILD_DIR)/variables/array-size-mismatch.elf >$(BUILD_DIR)/variables/array-size-mismatch.err 2>&1; then exit 1; fi
	grep -q "array literal has 3 elements but type expects 2" $(BUILD_DIR)/variables/array-size-mismatch.err
	if $(TARGET) tests/errors/array-index-out-of-range.ari -o $(BUILD_DIR)/variables/array-index-out-of-range.elf >$(BUILD_DIR)/variables/array-index-out-of-range.err 2>&1; then exit 1; fi
	grep -q "array index 2 is out of range for 2 elements" $(BUILD_DIR)/variables/array-index-out-of-range.err
	if $(TARGET) tests/errors/vector-len-non-vec.ari -o $(BUILD_DIR)/variables/vector-len-non-vec.elf >$(BUILD_DIR)/variables/vector-len-non-vec.err 2>&1; then exit 1; fi
	grep -q "len expects an array, Vec, or Slice value, got i64" $(BUILD_DIR)/variables/vector-len-non-vec.err
	if $(TARGET) tests/errors/vector-empty-untyped.ari -o $(BUILD_DIR)/variables/vector-empty-untyped.elf >$(BUILD_DIR)/variables/vector-empty-untyped.err 2>&1; then exit 1; fi
	grep -Fq "empty [] literals need an explicit Vec[T] type or non-empty array elements" $(BUILD_DIR)/variables/vector-empty-untyped.err
	if $(TARGET) tests/errors/vector-api-freeze.ari -o $(BUILD_DIR)/variables/vector-api-freeze.elf >$(BUILD_DIR)/variables/vector-api-freeze.err 2>&1; then exit 1; fi
	grep -Fq "local Vec method 'map' is reserved for allocator-backed std collection APIs" $(BUILD_DIR)/variables/vector-api-freeze.err
	if $(TARGET) tests/errors/variables-match-binding-immutable.ari -o $(BUILD_DIR)/variables/variables-match-binding-immutable.elf >$(BUILD_DIR)/variables/variables-match-binding-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'inner'" $(BUILD_DIR)/variables/variables-match-binding-immutable.err
	if $(TARGET) tests/errors/variables-let-pattern-binding.ari -o $(BUILD_DIR)/variables/variables-let-pattern-binding.elf >$(BUILD_DIR)/variables/variables-let-pattern-binding.err 2>&1; then exit 1; fi
	grep -q "cannot assign to immutable binding 'inner'" $(BUILD_DIR)/variables/variables-let-pattern-binding.err
	if $(TARGET) tests/errors/variables-tuple-pattern-non-tuple.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern-non-tuple.elf >$(BUILD_DIR)/variables/variables-tuple-pattern-non-tuple.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern requires a tuple value, got i64" $(BUILD_DIR)/variables/variables-tuple-pattern-non-tuple.err
	if $(TARGET) tests/errors/variables-tuple-pattern-arity.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern-arity.elf >$(BUILD_DIR)/variables/variables-tuple-pattern-arity.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern has 3 elements but value has 2" $(BUILD_DIR)/variables/variables-tuple-pattern-arity.err
	if $(TARGET) tests/errors/variables-tuple-pattern-rest-arity.ari -o $(BUILD_DIR)/variables/variables-tuple-pattern-rest-arity.elf >$(BUILD_DIR)/variables/variables-tuple-pattern-rest-arity.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern has 3 non-rest elements but value has 2" $(BUILD_DIR)/variables/variables-tuple-pattern-rest-arity.err
	if $(TARGET) tests/errors/variables-struct-pattern-non-struct.ari -o $(BUILD_DIR)/variables/variables-struct-pattern-non-struct.elf >$(BUILD_DIR)/variables/variables-struct-pattern-non-struct.err 2>&1; then exit 1; fi
	grep -q "struct binding pattern requires a struct value, got i64" $(BUILD_DIR)/variables/variables-struct-pattern-non-struct.err
	if $(TARGET) tests/errors/variables-struct-pattern-field.ari -o $(BUILD_DIR)/variables/variables-struct-pattern-field.elf >$(BUILD_DIR)/variables/variables-struct-pattern-field.err 2>&1; then exit 1; fi
	grep -q "struct 'Point' has no field 'y'" $(BUILD_DIR)/variables/variables-struct-pattern-field.err
check-ffi: $(TARGET)
	mkdir -p $(BUILD_DIR)/ffi
	$(TARGET) tests/ok/ffi-c-puts.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-puts.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ffi/ffi-c-puts.ll
	$(TARGET) tests/ok/ffi-c-module.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-module.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ffi/ffi-c-module.ll
	$(TARGET) tests/ok/ffi-c-alias.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-alias.ll
	grep -q 'declare i32 @"puts"(ptr)' $(BUILD_DIR)/ffi/ffi-c-alias.ll
	$(TARGET) tests/ok/ffi-c-libc-types.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-libc-types.ll
	grep -q 'declare i32 @"abs"(i32)' $(BUILD_DIR)/ffi/ffi-c-libc-types.ll
	grep -q 'declare i64 @"strlen"(ptr)' $(BUILD_DIR)/ffi/ffi-c-libc-types.ll
	$(TARGET) tests/ok/ffi-c-types.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-types.ll
	grep -q 'declare i64 @"ari_test_add_i64"(i64, i64)' $(BUILD_DIR)/ffi/ffi-c-types.ll
	grep -q 'declare i64 @"ari_test_strlen"(ptr)' $(BUILD_DIR)/ffi/ffi-c-types.ll
	grep -q 'declare void @"ari_test_inc_i64"(ptr)' $(BUILD_DIR)/ffi/ffi-c-types.ll
	$(TARGET) tests/ok/ffi-c-unsigned-aliases.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i1' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i8' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i16' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i32' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	grep -q 'alloca i64' $(BUILD_DIR)/ffi/ffi-c-unsigned-aliases.ll
	$(TARGET) --target i686-unknown-linux-gnu tests/ok/ffi-c-target-ilp32.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-target-ilp32.ll
	grep -q 'target triple = "i686-unknown-linux-gnu"' $(BUILD_DIR)/ffi/ffi-c-target-ilp32.ll
	grep -q 'alloca i32' $(BUILD_DIR)/ffi/ffi-c-target-ilp32.ll
	$(TARGET) --target x86_64-pc-linux-gnu tests/ok/ffi-c-target-lp64.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-target-lp64.ll
	grep -q 'target triple = "x86_64-pc-linux-gnu"' $(BUILD_DIR)/ffi/ffi-c-target-lp64.ll
	grep -q 'alloca i64' $(BUILD_DIR)/ffi/ffi-c-target-lp64.ll
	$(TARGET) --target x86_64-pc-windows-msvc tests/ok/ffi-c-target-llp64.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	grep -q 'target triple = "x86_64-pc-windows-msvc"' $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	grep -q 'alloca i32' $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	grep -q 'alloca i64' $(BUILD_DIR)/ffi/ffi-c-target-llp64.ll
	$(TARGET) tests/ok/ffi-c-null-pointer.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-null-pointer.ll
	grep -q 'declare i64 @"ari_test_is_null"(ptr)' $(BUILD_DIR)/ffi/ffi-c-null-pointer.ll
	grep -q 'store ptr null' $(BUILD_DIR)/ffi/ffi-c-null-pointer.ll
	$(TARGET) tests/ok/nullable-type-pointer.ari --emit-llvm $(BUILD_DIR)/ffi/nullable-type-pointer.ll
	grep -q 'load i64, ptr' $(BUILD_DIR)/ffi/nullable-type-pointer.ll
	grep -q 'store ptr null' $(BUILD_DIR)/ffi/nullable-type-pointer.ll
	$(TARGET) --freestanding tests/ok/nullable-type-pointer.ari -o $(BUILD_DIR)/ffi/nullable-type-pointer.raw
	$(BUILD_DIR)/ffi/nullable-type-pointer.raw; code=$$?; test $$code -eq 12
	$(TARGET) tests/ok/pointer-casts.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-casts.ll
	grep -q 'ptrtoint ptr' $(BUILD_DIR)/ffi/pointer-casts.ll
	grep -q 'inttoptr i64' $(BUILD_DIR)/ffi/pointer-casts.ll
	$(TARGET) tests/ok/pointer-offset.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-offset.ll
	grep -q 'getelementptr i8, ptr' $(BUILD_DIR)/ffi/pointer-offset.ll
	grep -q 'ptrtoint ptr' $(BUILD_DIR)/ffi/pointer-offset.ll
	$(TARGET) --freestanding tests/ok/pointer-offset.ari -o $(BUILD_DIR)/ffi/pointer-offset.raw
	$(BUILD_DIR)/ffi/pointer-offset.raw; code=$$?; test $$code -eq 8
	$(TARGET) tests/ok/layout-query.ari --emit-llvm $(BUILD_DIR)/ffi/layout-query.ll
	grep -q 'getelementptr i8, ptr' $(BUILD_DIR)/ffi/layout-query.ll
	$(TARGET) --freestanding tests/ok/layout-query.ari -o $(BUILD_DIR)/ffi/layout-query.raw
	$(BUILD_DIR)/ffi/layout-query.raw; code=$$?; test $$code -eq 35
	$(TARGET) tests/ok/zone-allocation.ari --emit-llvm $(BUILD_DIR)/ffi/zone-allocation.ll
	grep -q '@ari_builtin_zone_create' $(BUILD_DIR)/ffi/zone-allocation.ll
	grep -q 'declare ptr @malloc' $(BUILD_DIR)/ffi/zone-allocation.ll
	grep -q 'declare void @free' $(BUILD_DIR)/ffi/zone-allocation.ll
	$(TARGET) tests/ok/zone-typed-allocation.ari --emit-llvm $(BUILD_DIR)/ffi/zone-typed-allocation.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-typed-allocation.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/zone-typed-allocation.ll
	$(TARGET) tests/ok/zone-placement-new.ari --emit-llvm $(BUILD_DIR)/ffi/zone-placement-new.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-placement-new.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/zone-placement-new.ll
	$(TARGET) tests/ok/zone-associated-new.ari --emit-llvm $(BUILD_DIR)/ffi/zone-associated-new.ll
	grep -q "_ARNv4impl5Point8inherent3new" $(BUILD_DIR)/ffi/zone-associated-new.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-associated-new.ll
	$(TARGET) tests/ok/zone-temp-function-local.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-function-local.ll
	grep -q 'call ptr @"ari_builtin_zone_create"' $(BUILD_DIR)/ffi/zone-temp-function-local.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-function-local.ll
	$(TARGET) tests/ok/zone-temp-nested.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-nested.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-nested.ll
	$(TARGET) tests/ok/zone-temp-control-flow.ari --emit-llvm $(BUILD_DIR)/ffi/zone-temp-control-flow.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-temp-control-flow.ll
	$(TARGET) tests/ok/zone-scratch-local.ari --emit-llvm $(BUILD_DIR)/ffi/zone-scratch-local.ll
	grep -q 'call ptr @"ari_builtin_zone_create"' $(BUILD_DIR)/ffi/zone-scratch-local.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-scratch-local.ll
	grep -q 'call void @"ari_builtin_zone_destroy"' $(BUILD_DIR)/ffi/zone-scratch-local.ll
	$(TARGET) tests/ok/zone-promote-return.ari --emit-llvm $(BUILD_DIR)/ffi/zone-promote-return.ll
	grep -q 'call ptr @"ari_builtin_zone_alloc"' $(BUILD_DIR)/ffi/zone-promote-return.ll
	grep -q 'load { i64, i64 }, ptr' $(BUILD_DIR)/ffi/zone-promote-return.ll
	$(TARGET) tests/ok/pointer-add.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-add.ll
	grep -q 'getelementptr i64, ptr' $(BUILD_DIR)/ffi/pointer-add.ll
	grep -q 'getelementptr i8, ptr' $(BUILD_DIR)/ffi/pointer-add.ll
	$(TARGET) --freestanding tests/ok/pointer-add.ari -o $(BUILD_DIR)/ffi/pointer-add.raw
	$(BUILD_DIR)/ffi/pointer-add.raw; code=$$?; test $$code -eq 19
	$(TARGET) tests/ok/pointer-add-aggregate.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-add-aggregate.ll
	grep -q 'getelementptr { i64, i64 }, ptr' $(BUILD_DIR)/ffi/pointer-add-aggregate.ll
	grep -q 'getelementptr \[3 x i64\], ptr' $(BUILD_DIR)/ffi/pointer-add-aggregate.ll
	$(TARGET) --freestanding tests/ok/pointer-add-aggregate.ari -o $(BUILD_DIR)/ffi/pointer-add-aggregate.raw
	$(BUILD_DIR)/ffi/pointer-add-aggregate.raw; code=$$?; test $$code -eq 56
	$(TARGET) tests/ok/pointer-load-store.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-load-store.ll
	grep -q 'load i64, ptr' $(BUILD_DIR)/ffi/pointer-load-store.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/pointer-load-store.ll
	$(TARGET) --freestanding tests/ok/pointer-load-store.ari -o $(BUILD_DIR)/ffi/pointer-load-store.raw
	$(BUILD_DIR)/ffi/pointer-load-store.raw; code=$$?; test $$code -eq 12
	$(TARGET) tests/ok/pointer-float-load-store.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'load float, ptr' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'store float' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'load double, ptr' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	grep -q 'store double' $(BUILD_DIR)/ffi/pointer-float-load-store.ll
	$(TARGET) --freestanding tests/ok/pointer-float-load-store.ari -o $(BUILD_DIR)/ffi/pointer-float-load-store.raw
	$(BUILD_DIR)/ffi/pointer-float-load-store.raw; code=$$?; test $$code -eq 2
	$(TARGET) tests/ok/pointer-deref.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-deref.ll
	grep -q 'load i64, ptr' $(BUILD_DIR)/ffi/pointer-deref.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/pointer-deref.ll
	grep -q 'store i8' $(BUILD_DIR)/ffi/pointer-deref.ll
	$(TARGET) --freestanding tests/ok/pointer-deref.ari -o $(BUILD_DIR)/ffi/pointer-deref.raw
	$(BUILD_DIR)/ffi/pointer-deref.raw; code=$$?; test $$code -eq 15
	$(TARGET) tests/ok/pointer-aggregate-access.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	grep -q 'getelementptr inbounds { i64, i64 }' $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	grep -q 'getelementptr inbounds \[3 x i64\]' $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	grep -q 'store i64' $(BUILD_DIR)/ffi/pointer-aggregate-access.ll
	$(TARGET) --freestanding tests/ok/pointer-aggregate-access.ari -o $(BUILD_DIR)/ffi/pointer-aggregate-access.raw
	$(BUILD_DIR)/ffi/pointer-aggregate-access.raw; code=$$?; test $$code -eq 45
	$(TARGET) --freestanding tests/ok/freestanding-aggregate-byte-layout.ari -o $(BUILD_DIR)/ffi/freestanding-aggregate-byte-layout.raw
	$(BUILD_DIR)/ffi/freestanding-aggregate-byte-layout.raw; code=$$?; test $$code -eq 45
	$(TARGET) tests/ok/pointer-aggregate-load-store.ari --emit-llvm $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	grep -q 'load { i64, i64 }, ptr' $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	grep -q 'store { i64, i64 }' $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	grep -q 'store \[2 x i64\]' $(BUILD_DIR)/ffi/pointer-aggregate-load-store.ll
	$(TARGET) --freestanding tests/ok/pointer-aggregate-load-store.ari -o $(BUILD_DIR)/ffi/pointer-aggregate-load-store.raw
	$(BUILD_DIR)/ffi/pointer-aggregate-load-store.raw; code=$$?; test $$code -eq 48
	$(TARGET) tests/ok/ffi-c-varargs.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-varargs.ll
	grep -q 'declare i32 @printf(ptr, ...)' $(BUILD_DIR)/ffi/ffi-c-varargs.ll
	grep -q 'call i32 @"printf"(ptr' $(BUILD_DIR)/ffi/ffi-c-varargs.ll
	$(TARGET) tests/ok/ffi-c-varargs-promotions.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'sext i8' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'zext i16' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'zext i1' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	grep -q 'fpext float' $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.ll
	$(TARGET) tests/ok/ffi-c-callback.ari --emit-llvm $(BUILD_DIR)/ffi/ffi-c-callback.ll
	grep -q 'declare i64 @"ari_test_apply_i64"(ptr, i64)' $(BUILD_DIR)/ffi/ffi-c-callback.ll
	grep -q 'call i64 @"ari_test_apply_i64"(ptr @"_ARNv7add_ten", i64 5)' $(BUILD_DIR)/ffi/ffi-c-callback.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(LLVM_CC) -c tests/ffi/c_helpers.c -o $(BUILD_DIR)/ffi/c_helpers.o; ar rcs $(BUILD_DIR)/ffi/libari_testffi.a $(BUILD_DIR)/ffi/c_helpers.o; $(TARGET) tests/ok/ffi-c-types.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-types.elf; $(BUILD_DIR)/ffi/ffi-c-types.elf; code=$$?; test $$code -eq 36; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/ok/ffi-c-null-pointer.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-null-pointer.elf; $(BUILD_DIR)/ffi/ffi-c-null-pointer.elf; code=$$?; test $$code -eq 3; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-casts.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/pointer-casts.elf; $(BUILD_DIR)/ffi/pointer-casts.elf; code=$$?; test $$code -eq 1; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-offset.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-offset.elf; $(BUILD_DIR)/ffi/pointer-offset.elf; code=$$?; test $$code -eq 8; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/layout-query.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/layout-query.elf; $(BUILD_DIR)/ffi/layout-query.elf; code=$$?; test $$code -eq 35; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-allocation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-allocation.elf; $(BUILD_DIR)/ffi/zone-allocation.elf; code=$$?; test $$code -eq 54; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-typed-allocation.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-typed-allocation.elf; $(BUILD_DIR)/ffi/zone-typed-allocation.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-placement-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-placement-new.elf; $(BUILD_DIR)/ffi/zone-placement-new.elf; code=$$?; test $$code -eq 46; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-associated-new.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-associated-new.elf; $(BUILD_DIR)/ffi/zone-associated-new.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-temp-function-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-function-local.elf; $(BUILD_DIR)/ffi/zone-temp-function-local.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-temp-nested.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-nested.elf; $(BUILD_DIR)/ffi/zone-temp-nested.elf; code=$$?; test $$code -eq 22; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-temp-control-flow.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-temp-control-flow.elf; $(BUILD_DIR)/ffi/zone-temp-control-flow.elf; code=$$?; test $$code -eq 37; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-scratch-local.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-scratch-local.elf; $(BUILD_DIR)/ffi/zone-scratch-local.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/zone-promote-return.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/zone-promote-return.elf; $(BUILD_DIR)/ffi/zone-promote-return.elf; code=$$?; test $$code -eq 42; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-add.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-add.elf; $(BUILD_DIR)/ffi/pointer-add.elf; code=$$?; test $$code -eq 19; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-add-aggregate.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-add-aggregate.elf; $(BUILD_DIR)/ffi/pointer-add-aggregate.elf; code=$$?; test $$code -eq 56; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-load-store.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-load-store.elf; $(BUILD_DIR)/ffi/pointer-load-store.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-deref.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-deref.elf; $(BUILD_DIR)/ffi/pointer-deref.elf; code=$$?; test $$code -eq 15; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-aggregate-access.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-aggregate-access.elf; $(BUILD_DIR)/ffi/pointer-aggregate-access.elf; code=$$?; test $$code -eq 45; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/pointer-aggregate-load-store.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/pointer-aggregate-load-store.elf; $(BUILD_DIR)/ffi/pointer-aggregate-load-store.elf; code=$$?; test $$code -eq 48; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ffi-c-libc-types.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/ffi-c-libc-types.elf; $(BUILD_DIR)/ffi/ffi-c-libc-types.elf; code=$$?; test $$code -eq 12; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ffi-c-varargs.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/ffi-c-varargs.elf; $(BUILD_DIR)/ffi/ffi-c-varargs.elf >$(BUILD_DIR)/ffi/ffi-c-varargs.out; code=$$?; test $$code -eq 0; grep -q "ari varargs: 7 ok" $(BUILD_DIR)/ffi/ffi-c-varargs.out; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/ffi-c-varargs-promotions.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.elf; $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.elf >$(BUILD_DIR)/ffi/ffi-c-varargs-promotions.out; code=$$?; test $$code -eq 0; grep -q "ari promotions: 7 9 1 1.5" $(BUILD_DIR)/ffi/ffi-c-varargs-promotions.out; fi
	if command -v $(LLVM_CC) >/dev/null 2>&1 && command -v ar >/dev/null 2>&1; then $(TARGET) tests/ok/ffi-c-callback.ari --llvm-cc $(LLVM_CC) -L $(BUILD_DIR)/ffi -l ari_testffi -o $(BUILD_DIR)/ffi/ffi-c-callback.elf; $(BUILD_DIR)/ffi/ffi-c-callback.elf; code=$$?; test $$code -eq 15; fi
	if $(TARGET) tests/errors/extern-void-param.ari -o $(BUILD_DIR)/ffi/extern-void-param.elf >$(BUILD_DIR)/ffi/extern-void-param.err 2>&1; then exit 1; fi
	grep -q "extern parameter cannot have void type; use ptr c_void for void\\*" $(BUILD_DIR)/ffi/extern-void-param.err
	if $(TARGET) tests/errors/extern-varargs-nonextern.ari -o $(BUILD_DIR)/ffi/extern-varargs-nonextern.elf >$(BUILD_DIR)/ffi/extern-varargs-nonextern.err 2>&1; then exit 1; fi
	grep -q "variadic parameters are only supported on extern \"C\" functions" $(BUILD_DIR)/ffi/extern-varargs-nonextern.err
	if $(TARGET) tests/errors/extern-varargs-empty.ari -o $(BUILD_DIR)/ffi/extern-varargs-empty.elf >$(BUILD_DIR)/ffi/extern-varargs-empty.err 2>&1; then exit 1; fi
	grep -q "C variadic extern functions require at least one fixed parameter" $(BUILD_DIR)/ffi/extern-varargs-empty.err
	if $(TARGET) tests/errors/extern-varargs-aggregate.ari -o $(BUILD_DIR)/ffi/extern-varargs-aggregate.elf >$(BUILD_DIR)/ffi/extern-varargs-aggregate.err 2>&1; then exit 1; fi
	grep -q "C variadic argument type is not supported: (i64, i64)" $(BUILD_DIR)/ffi/extern-varargs-aggregate.err
	if $(TARGET) tests/errors/extern-varargs-function-pointer.ari -o $(BUILD_DIR)/ffi/extern-varargs-function-pointer.elf >$(BUILD_DIR)/ffi/extern-varargs-function-pointer.err 2>&1; then exit 1; fi
	grep -q "variadic extern function 'printf' cannot be used as a function pointer value" $(BUILD_DIR)/ffi/extern-varargs-function-pointer.err
	if $(TARGET) tests/errors/extern-generic.ari -o $(BUILD_DIR)/ffi/extern-generic.elf >$(BUILD_DIR)/ffi/extern-generic.err 2>&1; then exit 1; fi
	grep -q "extern C declarations cannot be generic; declare concrete C symbols or wrap generic foreign APIs in C" $(BUILD_DIR)/ffi/extern-generic.err
	if $(TARGET) --freestanding tests/errors/freestanding-extern-c-call.ari -o $(BUILD_DIR)/ffi/freestanding-extern-c-call.raw >$(BUILD_DIR)/ffi/freestanding-extern-c-call.err 2>&1; then exit 1; fi
	grep -q 'freestanding backend cannot call extern "C" function' $(BUILD_DIR)/ffi/freestanding-extern-c-call.err
	if $(TARGET) tests/errors/null-non-pointer.ari -o $(BUILD_DIR)/ffi/null-non-pointer.elf >$(BUILD_DIR)/ffi/null-non-pointer.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got ptr void" $(BUILD_DIR)/ffi/null-non-pointer.err
	if $(TARGET) tests/errors/nullable-qualified.ari -o $(BUILD_DIR)/ffi/nullable-qualified.elf >$(BUILD_DIR)/ffi/nullable-qualified.err 2>&1; then exit 1; fi
	grep -q "nullable type suffix ? cannot be combined with own, ref, or ptr qualifiers" $(BUILD_DIR)/ffi/nullable-qualified.err
	if $(TARGET) tests/errors/nullable-value-initializer.ari -o $(BUILD_DIR)/ffi/nullable-value-initializer.elf >$(BUILD_DIR)/ffi/nullable-value-initializer.err 2>&1; then exit 1; fi
	grep -q "nullable type 'i64?' is a raw pointer spelling; initialize it with null, a raw pointer, or a compatible string pointer value" $(BUILD_DIR)/ffi/nullable-value-initializer.err
	if $(TARGET) tests/errors/pointer-deref-non-pointer.ari -o $(BUILD_DIR)/ffi/pointer-deref-non-pointer.elf >$(BUILD_DIR)/ffi/pointer-deref-non-pointer.err 2>&1; then exit 1; fi
	grep -q "pointer dereference expects a raw pointer, got i64" $(BUILD_DIR)/ffi/pointer-deref-non-pointer.err
	if $(TARGET) tests/errors/layout-query-type-args.ari -o $(BUILD_DIR)/ffi/layout-query-type-args.elf >$(BUILD_DIR)/ffi/layout-query-type-args.err 2>&1; then exit 1; fi
	grep -q "size_of expects exactly one type argument" $(BUILD_DIR)/ffi/layout-query-type-args.err
	if $(TARGET) tests/errors/zone-destroy-required.ari -o $(BUILD_DIR)/ffi/zone-destroy-required.elf >$(BUILD_DIR)/ffi/zone-destroy-required.err 2>&1; then exit 1; fi
	grep -q "owning binding 'zone' must be moved or dropped before return" $(BUILD_DIR)/ffi/zone-destroy-required.err
	if $(TARGET) tests/errors/zone-drop.ari -o $(BUILD_DIR)/ffi/zone-drop.elf >$(BUILD_DIR)/ffi/zone-drop.err 2>&1; then exit 1; fi
	grep -q "use zone::destroy(zone) to release a Zone" $(BUILD_DIR)/ffi/zone-drop.err
	if $(TARGET) tests/errors/zone-temp-move.ari -o $(BUILD_DIR)/ffi/zone-temp-move.elf >$(BUILD_DIR)/ffi/zone-temp-move.err 2>&1; then exit 1; fi
	grep -q "temporary zone 'scratch' cannot be moved" $(BUILD_DIR)/ffi/zone-temp-move.err
	if $(TARGET) tests/errors/zone-temp-return-escape.ari -o $(BUILD_DIR)/ffi/zone-temp-return-escape.elf >$(BUILD_DIR)/ffi/zone-temp-return-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'item' from temporary zone 'scratch' cannot escape through function return" $(BUILD_DIR)/ffi/zone-temp-return-escape.err
	if $(TARGET) tests/errors/zone-temp-outer-binding-escape.ari -o $(BUILD_DIR)/ffi/zone-temp-outer-binding-escape.elf >$(BUILD_DIR)/ffi/zone-temp-outer-binding-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'leaked' from temporary zone 'scratch' cannot outlive the temporary zone scope" $(BUILD_DIR)/ffi/zone-temp-outer-binding-escape.err
	if $(TARGET) tests/errors/zone-temp-struct-escape.ari -o $(BUILD_DIR)/ffi/zone-temp-struct-escape.elf >$(BUILD_DIR)/ffi/zone-temp-struct-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'item' from temporary zone 'scratch' cannot escape through struct literal" $(BUILD_DIR)/ffi/zone-temp-struct-escape.err
	if $(TARGET) tests/errors/zone-scratch-return-escape.ari -o $(BUILD_DIR)/ffi/zone-scratch-return-escape.elf >$(BUILD_DIR)/ffi/zone-scratch-return-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer 'item' from temporary zone" $(BUILD_DIR)/ffi/zone-scratch-return-escape.err
	grep -q "cannot escape through function return" $(BUILD_DIR)/ffi/zone-scratch-return-escape.err
	if $(TARGET) tests/errors/zone-scratch-expression.ari -o $(BUILD_DIR)/ffi/zone-scratch-expression.elf >$(BUILD_DIR)/ffi/zone-scratch-expression.err 2>&1; then exit 1; fi
	grep -q "zone::scratch<T> can only initialize a local pointer binding" $(BUILD_DIR)/ffi/zone-scratch-expression.err
	if $(TARGET) tests/errors/zone-promote-source-type.ari -o $(BUILD_DIR)/ffi/zone-promote-source-type.elf >$(BUILD_DIR)/ffi/zone-promote-source-type.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected ptr i64, got i64" $(BUILD_DIR)/ffi/zone-promote-source-type.err
	if $(TARGET) tests/errors/zone-alloc-type-args.ari -o $(BUILD_DIR)/ffi/zone-alloc-type-args.elf >$(BUILD_DIR)/ffi/zone-alloc-type-args.err 2>&1; then exit 1; fi
	grep -q "zone::alloc<T> expects exactly one type argument" $(BUILD_DIR)/ffi/zone-alloc-type-args.err
	if $(TARGET) tests/errors/zone-alloc-zero-sized.ari -o $(BUILD_DIR)/ffi/zone-alloc-zero-sized.elf >$(BUILD_DIR)/ffi/zone-alloc-zero-sized.err 2>&1; then exit 1; fi
	grep -q "zone::alloc<T> requires a non-zero-sized type" $(BUILD_DIR)/ffi/zone-alloc-zero-sized.err
	if $(TARGET) tests/errors/zone-new-type-args.ari -o $(BUILD_DIR)/ffi/zone-new-type-args.elf >$(BUILD_DIR)/ffi/zone-new-type-args.err 2>&1; then exit 1; fi
	grep -q "zone::new<T> expects exactly one type argument" $(BUILD_DIR)/ffi/zone-new-type-args.err
	if $(TARGET) tests/errors/zone-new-own-aggregate.ari -o $(BUILD_DIR)/ffi/zone-new-own-aggregate.elf >$(BUILD_DIR)/ffi/zone-new-own-aggregate.err 2>&1; then exit 1; fi
	grep -q "zone::new<T> cannot place ownership- or borrow-valued types yet" $(BUILD_DIR)/ffi/zone-new-own-aggregate.err
	if $(TARGET) tests/errors/zone-pointer-after-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-reset.err
	if $(TARGET) tests/errors/zone-pointer-after-destroy.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-destroy.elf >$(BUILD_DIR)/ffi/zone-pointer-after-destroy.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-destroy.err
	if $(TARGET) tests/errors/zone-wrapper-pointer-after-reset.ari -o $(BUILD_DIR)/ffi/zone-wrapper-pointer-after-reset.elf >$(BUILD_DIR)/ffi/zone-wrapper-pointer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-wrapper-pointer-after-reset.err
	if $(TARGET) tests/errors/zone-associated-pointer-after-reset.ari -o $(BUILD_DIR)/ffi/zone-associated-pointer-after-reset.elf >$(BUILD_DIR)/ffi/zone-associated-pointer-after-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'point' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-associated-pointer-after-reset.err
	if $(TARGET) tests/errors/zone-pointer-after-if-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-if-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-if-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-if-reset.err
	if $(TARGET) tests/errors/zone-pointer-after-match-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-match-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-match-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-match-reset.err
	if $(TARGET) tests/errors/zone-pointer-after-loop-reset.ari -o $(BUILD_DIR)/ffi/zone-pointer-after-loop-reset.elf >$(BUILD_DIR)/ffi/zone-pointer-after-loop-reset.err 2>&1; then exit 1; fi
	grep -q "cannot use zone pointer 'item' after zone 'zone' was reset or destroyed" $(BUILD_DIR)/ffi/zone-pointer-after-loop-reset.err
	if $(TARGET) tests/errors/zone-pointer-struct-escape.ari -o $(BUILD_DIR)/ffi/zone-pointer-struct-escape.elf >$(BUILD_DIR)/ffi/zone-pointer-struct-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer cannot escape into struct literal" $(BUILD_DIR)/ffi/zone-pointer-struct-escape.err
	if $(TARGET) tests/errors/zone-pointer-ffi-escape.ari -o $(BUILD_DIR)/ffi/zone-pointer-ffi-escape.elf >$(BUILD_DIR)/ffi/zone-pointer-ffi-escape.err 2>&1; then exit 1; fi
	grep -q "zone pointer cannot escape into extern C call argument" $(BUILD_DIR)/ffi/zone-pointer-ffi-escape.err
	if $(TARGET) tests/errors/zone-pointer-multizone-return.ari -o $(BUILD_DIR)/ffi/zone-pointer-multizone-return.elf >$(BUILD_DIR)/ffi/zone-pointer-multizone-return.err 2>&1; then exit 1; fi
	grep -q "zone pointer returns must come from the function's single zone borrow parameter" $(BUILD_DIR)/ffi/zone-pointer-multizone-return.err
	if $(TARGET) --freestanding tests/errors/zone-freestanding.ari -o $(BUILD_DIR)/ffi/zone-freestanding.raw >$(BUILD_DIR)/ffi/zone-freestanding.err 2>&1; then exit 1; fi
	grep -q "freestanding backend does not lower zone allocation yet" $(BUILD_DIR)/ffi/zone-freestanding.err
	if $(TARGET) tests/errors/pointer-deref-aggregate.ari -o $(BUILD_DIR)/ffi/pointer-deref-aggregate.elf >$(BUILD_DIR)/ffi/pointer-deref-aggregate.err 2>&1; then exit 1; fi
	grep -q "pointer dereference cannot copy ownership- or borrow-valued aggregates through raw pointers yet" $(BUILD_DIR)/ffi/pointer-deref-aggregate.err
check-functions: $(TARGET) check-function-errors
	mkdir -p $(BUILD_DIR)/functions
	$(TARGET) --freestanding tests/ok/function-recursion.ari -o $(BUILD_DIR)/functions/function-recursion.elf
	$(BUILD_DIR)/functions/function-recursion.elf; code=$$?; test $$code -eq 120
	readelf -sW $(BUILD_DIR)/functions/function-recursion.elf > $(BUILD_DIR)/functions/function-recursion.sym
	grep -q "_ARNv4fact" $(BUILD_DIR)/functions/function-recursion.sym
	grep -q "_ARNv4main" $(BUILD_DIR)/functions/function-recursion.sym
	if grep -q "__P" $(BUILD_DIR)/functions/function-recursion.sym; then exit 1; fi
	if grep -q "__R" $(BUILD_DIR)/functions/function-recursion.sym; then exit 1; fi
	$(TARGET) tests/ok/function-recursion.ari --emit-llvm $(BUILD_DIR)/functions/function-recursion.ll
	grep -q "_ARNv4fact" $(BUILD_DIR)/functions/function-recursion.ll
	grep -q "_ARNv4main" $(BUILD_DIR)/functions/function-recursion.ll
	if grep -q "__P" $(BUILD_DIR)/functions/function-recursion.ll; then exit 1; fi
	if grep -q "__R" $(BUILD_DIR)/functions/function-recursion.ll; then exit 1; fi
	$(TARGET) --freestanding tests/ok/function-recursion-freestanding.ari -o $(BUILD_DIR)/functions/function-recursion-freestanding.elf
	$(BUILD_DIR)/functions/function-recursion-freestanding.elf; code=$$?; test $$code -eq 120
	readelf -sW $(BUILD_DIR)/functions/function-recursion-freestanding.elf > $(BUILD_DIR)/functions/function-recursion-freestanding.sym
	grep -q "_ARNv4fact" $(BUILD_DIR)/functions/function-recursion-freestanding.sym
	grep -q "_ARNv4main" $(BUILD_DIR)/functions/function-recursion-freestanding.sym
	$(TARGET) --freestanding tests/ok/function-void-return.ari -o $(BUILD_DIR)/functions/function-void-return.elf
	$(BUILD_DIR)/functions/function-void-return.elf; code=$$?; test $$code -eq 7
	$(TARGET) tests/ok/function-param-patterns.ari --emit-llvm $(BUILD_DIR)/functions/function-param-patterns.ll
	grep -q "extractvalue { i64, i64 }" $(BUILD_DIR)/functions/function-param-patterns.ll
	grep -q "match.arm" $(BUILD_DIR)/functions/function-param-patterns.ll
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/function-param-patterns.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/functions/function-param-patterns.elf; $(BUILD_DIR)/functions/function-param-patterns.elf; code=$$?; test $$code -eq 33; fi
	$(TARGET) --freestanding tests/ok/function-six-args-freestanding.ari -o $(BUILD_DIR)/functions/function-six-args-freestanding.elf
	$(BUILD_DIR)/functions/function-six-args-freestanding.elf; code=$$?; test $$code -eq 21
	$(TARGET) --freestanding tests/ok/function-eight-args-freestanding.ari -o $(BUILD_DIR)/functions/function-eight-args-freestanding.elf
	$(BUILD_DIR)/functions/function-eight-args-freestanding.elf; code=$$?; test $$code -eq 36
	$(TARGET) --freestanding tests/ok/function-seven-args-host.ari -o $(BUILD_DIR)/functions/function-seven-args-host.elf
	$(BUILD_DIR)/functions/function-seven-args-host.elf; code=$$?; test $$code -eq 28
	$(TARGET) --freestanding tests/ok/function-narrow-return-freestanding.ari -o $(BUILD_DIR)/functions/function-narrow-return-freestanding.elf
	$(BUILD_DIR)/functions/function-narrow-return-freestanding.elf; code=$$?; test $$code -eq 23
	$(TARGET) --freestanding tests/ok/function-float-freestanding.ari -o $(BUILD_DIR)/functions/function-float-freestanding.elf
	$(BUILD_DIR)/functions/function-float-freestanding.elf; code=$$?; test $$code -eq 3
	$(TARGET) tests/ok/function-float-freestanding.ari --emit-llvm $(BUILD_DIR)/functions/function-float-freestanding.ll
	grep -q 'define double @"_ARNv5widen"' $(BUILD_DIR)/functions/function-float-freestanding.ll
	grep -q 'define float @"_ARNv6narrow"' $(BUILD_DIR)/functions/function-float-freestanding.ll
	$(TARGET) --freestanding tests/ok/function-aggregate-return-freestanding.ari -o $(BUILD_DIR)/functions/function-aggregate-return-freestanding.elf
	$(BUILD_DIR)/functions/function-aggregate-return-freestanding.elf; code=$$?; test $$code -eq 69
	$(TARGET) --freestanding tests/ok/function-aggregate-params-freestanding.ari -o $(BUILD_DIR)/functions/function-aggregate-params-freestanding.elf
	$(BUILD_DIR)/functions/function-aggregate-params-freestanding.elf; code=$$?; test $$code -eq 149
	$(TARGET) --freestanding tests/ok/function-aggregate-indirect-return-freestanding.ari -o $(BUILD_DIR)/functions/function-aggregate-indirect-return-freestanding.elf
	$(BUILD_DIR)/functions/function-aggregate-indirect-return-freestanding.elf; code=$$?; test $$code -eq 76
	$(TARGET) --freestanding tests/ok/function-generic-surface.ari -o $(BUILD_DIR)/functions/function-generic-surface.elf
	$(BUILD_DIR)/functions/function-generic-surface.elf; code=$$?; test $$code -eq 0
	$(TARGET) --freestanding tests/ok/function-generic-call.ari -o $(BUILD_DIR)/functions/function-generic-call.elf
	$(BUILD_DIR)/functions/function-generic-call.elf; code=$$?; test $$code -eq 22
	$(TARGET) --freestanding tests/ok/function-generic-companion.ari -o $(BUILD_DIR)/functions/function-generic-companion.elf
	$(BUILD_DIR)/functions/function-generic-companion.elf; code=$$?; test $$code -eq 12
	$(TARGET) --freestanding tests/ok/function-pointer-call.ari -o $(BUILD_DIR)/functions/function-pointer-call.raw
	$(BUILD_DIR)/functions/function-pointer-call.raw; code=$$?; test $$code -eq 7
	$(TARGET) tests/ok/function-pointer-call.ari --emit-llvm $(BUILD_DIR)/functions/function-pointer-call.ll
	grep -q 'call i64 %' $(BUILD_DIR)/functions/function-pointer-call.ll
	$(TARGET) tests/ok/function-extern-c.ari --emit-llvm $(BUILD_DIR)/functions/function-extern-c.ll
	grep -q 'declare i64 @"puts"(ptr)' $(BUILD_DIR)/functions/function-extern-c.ll
	grep -q 'call i64 @"puts"' $(BUILD_DIR)/functions/function-extern-c.ll
	$(TARGET) --freestanding tests/ok/function-module-pub.ari -o $(BUILD_DIR)/functions/function-module-pub.elf
	$(BUILD_DIR)/functions/function-module-pub.elf; code=$$?; test $$code -eq 10
	$(TARGET) tests/ok/function-module-pub.ari --emit-llvm $(BUILD_DIR)/functions/function-module-pub.ll
	grep -q 'define i64 @"_ARNv4Math5value"' $(BUILD_DIR)/functions/function-module-pub.ll
	grep -q 'define i64 @"_ARNv4Math11call_hidden"' $(BUILD_DIR)/functions/function-module-pub.ll

check-function-errors: $(TARGET)
	mkdir -p $(BUILD_DIR)/functions/errors
	if $(TARGET) tests/errors/function-missing-main.ari -o $(BUILD_DIR)/functions/errors/function-missing-main.elf >$(BUILD_DIR)/functions/errors/function-missing-main.err 2>&1; then exit 1; fi
	grep -q "missing executable function 'main'" $(BUILD_DIR)/functions/errors/function-missing-main.err
	if $(TARGET) tests/errors/function-main-params.ari -o $(BUILD_DIR)/functions/errors/function-main-params.elf >$(BUILD_DIR)/functions/errors/function-main-params.err 2>&1; then exit 1; fi
	grep -q "main cannot take parameters" $(BUILD_DIR)/functions/errors/function-main-params.err
	if $(TARGET) tests/errors/function-main-return-type.ari -o $(BUILD_DIR)/functions/errors/function-main-return-type.elf >$(BUILD_DIR)/functions/errors/function-main-return-type.err 2>&1; then exit 1; fi
	grep -q "main must return i64 in the executable subset" $(BUILD_DIR)/functions/errors/function-main-return-type.err
	if $(TARGET) tests/errors/function-missing-return.ari -o $(BUILD_DIR)/functions/errors/function-missing-return.elf >$(BUILD_DIR)/functions/errors/function-missing-return.err 2>&1; then exit 1; fi
	grep -q "function 'main' must return a value on all paths" $(BUILD_DIR)/functions/errors/function-missing-return.err
	if $(TARGET) tests/errors/function-branch-missing-return.ari -o $(BUILD_DIR)/functions/errors/function-branch-missing-return.elf >$(BUILD_DIR)/functions/errors/function-branch-missing-return.err 2>&1; then exit 1; fi
	grep -q "function 'choose' must return a value on all paths" $(BUILD_DIR)/functions/errors/function-branch-missing-return.err
	if $(TARGET) tests/errors/function-return-type-mismatch.ari -o $(BUILD_DIR)/functions/errors/function-return-type-mismatch.elf >$(BUILD_DIR)/functions/errors/function-return-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/functions/errors/function-return-type-mismatch.err
	if $(TARGET) tests/errors/function-wrong-arg-count.ari -o $(BUILD_DIR)/functions/errors/function-wrong-arg-count.elf >$(BUILD_DIR)/functions/errors/function-wrong-arg-count.err 2>&1; then exit 1; fi
	grep -q "wrong argument count for 'add'" $(BUILD_DIR)/functions/errors/function-wrong-arg-count.err
	if $(TARGET) tests/errors/function-arg-type-mismatch.ari -o $(BUILD_DIR)/functions/errors/function-arg-type-mismatch.elf >$(BUILD_DIR)/functions/errors/function-arg-type-mismatch.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/functions/errors/function-arg-type-mismatch.err
	if $(TARGET) tests/errors/function-param-pattern.ari -o $(BUILD_DIR)/functions/errors/function-param-pattern.elf >$(BUILD_DIR)/functions/errors/function-param-pattern.err 2>&1; then exit 1; fi
	grep -q "tuple binding pattern requires a tuple value, got i64" $(BUILD_DIR)/functions/errors/function-param-pattern.err
	if $(TARGET) tests/errors/function-overload-params.ari -o $(BUILD_DIR)/functions/errors/function-overload-params.elf >$(BUILD_DIR)/functions/errors/function-overload-params.err 2>&1; then exit 1; fi
	grep -q "duplicate executable function 'pick'" $(BUILD_DIR)/functions/errors/function-overload-params.err
	if $(TARGET) tests/errors/function-void-return-value.ari -o $(BUILD_DIR)/functions/errors/function-void-return-value.elf >$(BUILD_DIR)/functions/errors/function-void-return-value.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected void, got i64" $(BUILD_DIR)/functions/errors/function-void-return-value.err
	if $(TARGET) tests/errors/function-module-private.ari -o $(BUILD_DIR)/functions/errors/function-module-private.elf >$(BUILD_DIR)/functions/errors/function-module-private.err 2>&1; then exit 1; fi
	grep -q "function 'Secret::hidden' is not public" $(BUILD_DIR)/functions/errors/function-module-private.err

check-errors: $(TARGET) check-function-errors
	mkdir -p $(BUILD_DIR)/errors
	if $(TARGET) tests/errors/use-after-move.ari -o $(BUILD_DIR)/errors/use-after-move.elf >$(BUILD_DIR)/errors/use-after-move.err 2>&1; then exit 1; fi
	grep -q "cannot use moved binding 'token'" $(BUILD_DIR)/errors/use-after-move.err
	if $(TARGET) tests/errors/undropped-own.ari -o $(BUILD_DIR)/errors/undropped-own.elf >$(BUILD_DIR)/errors/undropped-own.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before scope exit" $(BUILD_DIR)/errors/undropped-own.err
	if $(TARGET) tests/errors/return-live-owner.ari -o $(BUILD_DIR)/errors/return-live-owner.elf >$(BUILD_DIR)/errors/return-live-owner.err 2>&1; then exit 1; fi
	grep -q "owning binding 'token' must be moved or dropped before return" $(BUILD_DIR)/errors/return-live-owner.err
	if $(TARGET) tests/errors/overwrite-own.ari -o $(BUILD_DIR)/errors/overwrite-own.elf >$(BUILD_DIR)/errors/overwrite-own.err 2>&1; then exit 1; fi
	grep -q "cannot overwrite owning binding 'token'" $(BUILD_DIR)/errors/overwrite-own.err
	if $(TARGET) tests/errors/discard-own-result.ari -o $(BUILD_DIR)/errors/discard-own-result.elf >$(BUILD_DIR)/errors/discard-own-result.err 2>&1; then exit 1; fi
	grep -q "owning expression result must be bound, returned, passed, or dropped" $(BUILD_DIR)/errors/discard-own-result.err
	if $(TARGET) tests/errors/own-bool-condition.ari -o $(BUILD_DIR)/errors/own-bool-condition.elf >$(BUILD_DIR)/errors/own-bool-condition.err 2>&1; then exit 1; fi
	grep -q "condition must be bool or integer-convertible, got own bool" $(BUILD_DIR)/errors/own-bool-condition.err
	$(TARGET) tests/ok/drop-destructor-lowering.ari --emit-llvm $(BUILD_DIR)/errors/drop-destructor-lowering.ll
	grep -q 'call void @"_ARNv4impl3i649std__Drop4drop"' $(BUILD_DIR)/errors/drop-destructor-lowering.ll
	$(TARGET) --freestanding tests/ok/drop-destructor-lowering.ari -o $(BUILD_DIR)/errors/drop-destructor-lowering.raw
	$(BUILD_DIR)/errors/drop-destructor-lowering.raw; code=$$?; test $$code -eq 0
	if $(TARGET) tests/errors/if-drop-one-branch.ari -o $(BUILD_DIR)/errors/if-drop-one-branch.elf >$(BUILD_DIR)/errors/if-drop-one-branch.err 2>&1; then exit 1; fi
	grep -q "changes ownership state in if without else" $(BUILD_DIR)/errors/if-drop-one-branch.err
	if $(TARGET) tests/errors/if-branch-state-mismatch.ari -o $(BUILD_DIR)/errors/if-branch-state-mismatch.elf >$(BUILD_DIR)/errors/if-branch-state-mismatch.err 2>&1; then exit 1; fi
	grep -q "has incompatible ownership states after if branches" $(BUILD_DIR)/errors/if-branch-state-mismatch.err
	if $(TARGET) tests/errors/loop-drop-owner.ari -o $(BUILD_DIR)/errors/loop-drop-owner.elf >$(BUILD_DIR)/errors/loop-drop-owner.err 2>&1; then exit 1; fi
	grep -q "cannot change ownership state inside loop yet" $(BUILD_DIR)/errors/loop-drop-owner.err
	if $(TARGET) tests/errors/while-non-bool.ari -o $(BUILD_DIR)/errors/while-non-bool.elf >$(BUILD_DIR)/errors/while-non-bool.err 2>&1; then exit 1; fi
	grep -q "condition must be bool or integer-convertible, got string" $(BUILD_DIR)/errors/while-non-bool.err
	if $(TARGET) tests/errors/break-outside-loop.ari -o $(BUILD_DIR)/errors/break-outside-loop.elf >$(BUILD_DIR)/errors/break-outside-loop.err 2>&1; then exit 1; fi
	grep -q "break used outside a loop" $(BUILD_DIR)/errors/break-outside-loop.err
	if $(TARGET) tests/errors/continue-outside-loop.ari -o $(BUILD_DIR)/errors/continue-outside-loop.elf >$(BUILD_DIR)/errors/continue-outside-loop.err 2>&1; then exit 1; fi
	grep -q "continue used outside a loop" $(BUILD_DIR)/errors/continue-outside-loop.err
	if $(TARGET) tests/errors/continue-values-while.ari -o $(BUILD_DIR)/errors/continue-values-while.elf >$(BUILD_DIR)/errors/continue-values-while.err 2>&1; then exit 1; fi
	grep -q "continue values are only valid inside init-while loops" $(BUILD_DIR)/errors/continue-values-while.err
	if $(TARGET) tests/errors/for-not-range.ari -o $(BUILD_DIR)/errors/for-not-range.elf >$(BUILD_DIR)/errors/for-not-range.err 2>&1; then exit 1; fi
	grep -q "for currently supports range values" $(BUILD_DIR)/errors/for-not-range.err
	if $(TARGET) tests/errors/range-arg-count.ari -o $(BUILD_DIR)/errors/range-arg-count.elf >$(BUILD_DIR)/errors/range-arg-count.err 2>&1; then exit 1; fi
	grep -q "range expects start and end values" $(BUILD_DIR)/errors/range-arg-count.err
	if $(TARGET) tests/errors/for-vector-empty.ari -o $(BUILD_DIR)/errors/for-vector-empty.elf >$(BUILD_DIR)/errors/for-vector-empty.err 2>&1; then exit 1; fi
	grep -Fq "empty [] literals need an explicit Vec[T] type or non-empty array elements" $(BUILD_DIR)/errors/for-vector-empty.err
	if $(TARGET) tests/errors/for-vector-mixed-types.ari -o $(BUILD_DIR)/errors/for-vector-mixed-types.elf >$(BUILD_DIR)/errors/for-vector-mixed-types.err 2>&1; then exit 1; fi
	grep -q "type mismatch: expected i64, got bool" $(BUILD_DIR)/errors/for-vector-mixed-types.err
	if $(TARGET) tests/errors/init-while-next-count.ari -o $(BUILD_DIR)/errors/init-while-next-count.elf >$(BUILD_DIR)/errors/init-while-next-count.err 2>&1; then exit 1; fi
	grep -q "next value count must match init binding count" $(BUILD_DIR)/errors/init-while-next-count.err
	if $(TARGET) tests/errors/init-while-continue-count.ari -o $(BUILD_DIR)/errors/init-while-continue-count.elf >$(BUILD_DIR)/errors/init-while-continue-count.err 2>&1; then exit 1; fi
	grep -q "continue value count must match init binding count" $(BUILD_DIR)/errors/init-while-continue-count.err
	if $(TARGET) tests/errors/mutable-borrow-immutable.ari -o $(BUILD_DIR)/errors/mutable-borrow-immutable.elf >$(BUILD_DIR)/errors/mutable-borrow-immutable.err 2>&1; then exit 1; fi
	grep -q "cannot mutably borrow immutable binding 'number'" $(BUILD_DIR)/errors/mutable-borrow-immutable.err
	if $(TARGET) tests/errors/move-borrowed-owner.ari -o $(BUILD_DIR)/errors/move-borrowed-owner.elf >$(BUILD_DIR)/errors/move-borrowed-owner.err 2>&1; then exit 1; fi
	grep -q "cannot move borrowed binding 'token'" $(BUILD_DIR)/errors/move-borrowed-owner.err
	if $(TARGET) tests/errors/mutable-borrow-conflict.ari -o $(BUILD_DIR)/errors/mutable-borrow-conflict.elf >$(BUILD_DIR)/errors/mutable-borrow-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed binding 'number'" $(BUILD_DIR)/errors/mutable-borrow-conflict.err
	if $(TARGET) tests/errors/return-borrow.ari -o $(BUILD_DIR)/errors/return-borrow.elf >$(BUILD_DIR)/errors/return-borrow.err 2>&1; then exit 1; fi
	grep -q "borrowed references cannot be returned in the executable subset yet" $(BUILD_DIR)/errors/return-borrow.err
	if $(TARGET) tests/errors/discard-borrow.ari -o $(BUILD_DIR)/errors/discard-borrow.elf >$(BUILD_DIR)/errors/discard-borrow.err 2>&1; then exit 1; fi
	grep -q "borrow expression result must be passed directly to a call" $(BUILD_DIR)/errors/discard-borrow.err
	$(TARGET) --freestanding tests/ok/borrow-named.ari -o $(BUILD_DIR)/errors/borrow-named.raw
	$(BUILD_DIR)/errors/borrow-named.raw; code=$$?; test $$code -eq 12
	if command -v $(LLVM_CC) >/dev/null 2>&1; then $(TARGET) tests/ok/borrow-named.ari --llvm-cc $(LLVM_CC) -o $(BUILD_DIR)/errors/borrow-named.elf; $(BUILD_DIR)/errors/borrow-named.elf; code=$$?; test $$code -eq 12; fi
	if $(TARGET) tests/errors/named-borrow-assign-source.ari -o $(BUILD_DIR)/errors/named-borrow-assign-source.elf >$(BUILD_DIR)/errors/named-borrow-assign-source.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrowed binding 'number'" $(BUILD_DIR)/errors/named-borrow-assign-source.err
	if $(TARGET) tests/errors/named-borrow-conflict.ari -o $(BUILD_DIR)/errors/named-borrow-conflict.elf >$(BUILD_DIR)/errors/named-borrow-conflict.err 2>&1; then exit 1; fi
	grep -q "cannot immutably borrow mutably borrowed binding 'number'" $(BUILD_DIR)/errors/named-borrow-conflict.err
	if $(TARGET) tests/errors/named-borrow-reassign.ari -o $(BUILD_DIR)/errors/named-borrow-reassign.elf >$(BUILD_DIR)/errors/named-borrow-reassign.err 2>&1; then exit 1; fi
	grep -q "cannot assign to borrow binding 'shared'" $(BUILD_DIR)/errors/named-borrow-reassign.err
	if $(TARGET) tests/errors/u8-literal-range.ari -o $(BUILD_DIR)/errors/u8-literal-range.elf >$(BUILD_DIR)/errors/u8-literal-range.err 2>&1; then exit 1; fi
	grep -q "integer literal 256 is out of range for u8" $(BUILD_DIR)/errors/u8-literal-range.err
	if $(TARGET) tests/errors/i8-literal-range.ari -o $(BUILD_DIR)/errors/i8-literal-range.elf >$(BUILD_DIR)/errors/i8-literal-range.err 2>&1; then exit 1; fi
	grep -q "integer literal -129 is out of range for i8" $(BUILD_DIR)/errors/i8-literal-range.err
	if $(TARGET) tests/errors/integer-literal-too-large.ari -o $(BUILD_DIR)/errors/integer-literal-too-large.elf >$(BUILD_DIR)/errors/integer-literal-too-large.err 2>&1; then exit 1; fi
	grep -q "integer literal is too large" $(BUILD_DIR)/errors/integer-literal-too-large.err
	if $(TARGET) --freestanding tests/errors/float-backend.ari -o $(BUILD_DIR)/errors/float-backend.elf >$(BUILD_DIR)/errors/float-backend.err 2>&1; then exit 1; fi
	grep -q "freestanding backend does not lower f128 float operators yet" $(BUILD_DIR)/errors/float-backend.err
	if $(TARGET) --freestanding tests/errors/string-backend.ari -o $(BUILD_DIR)/errors/string-backend.elf >$(BUILD_DIR)/errors/string-backend.err 2>&1; then exit 1; fi
	grep -q "backend does not lower string values yet" $(BUILD_DIR)/errors/string-backend.err
	if $(TARGET) tests/errors/class-removed.ari -o $(BUILD_DIR)/errors/class-removed.elf >$(BUILD_DIR)/errors/class-removed.err 2>&1; then exit 1; fi
	grep -q "expected top-level declaration" $(BUILD_DIR)/errors/class-removed.err
	if $(TARGET) tests/errors/interface-removed.ari -o $(BUILD_DIR)/errors/interface-removed.elf >$(BUILD_DIR)/errors/interface-removed.err 2>&1; then exit 1; fi
	grep -q "expected top-level declaration" $(BUILD_DIR)/errors/interface-removed.err
	if $(TARGET) tests/errors/unknown-trait-impl.ari -o $(BUILD_DIR)/errors/unknown-trait-impl.elf >$(BUILD_DIR)/errors/unknown-trait-impl.err 2>&1; then exit 1; fi
	grep -q "unknown trait 'MissingTrait'" $(BUILD_DIR)/errors/unknown-trait-impl.err
	if $(TARGET) tests/errors/iterable-trait-arity.ari -o $(BUILD_DIR)/errors/iterable-trait-arity.elf >$(BUILD_DIR)/errors/iterable-trait-arity.err 2>&1; then exit 1; fi
	grep -q "trait 'std::Iterable' expects 1 type argument" $(BUILD_DIR)/errors/iterable-trait-arity.err
	if $(TARGET) tests/errors/duplicate-trait-impl.ari -o $(BUILD_DIR)/errors/duplicate-trait-impl.elf >$(BUILD_DIR)/errors/duplicate-trait-impl.err 2>&1; then exit 1; fi
	grep -q "duplicate impl of trait 'std::Iterable' for Vec\\[i64\\]" $(BUILD_DIR)/errors/duplicate-trait-impl.err
	if $(TARGET) tests/errors/logical-non-bool.ari -o $(BUILD_DIR)/errors/logical-non-bool.elf >$(BUILD_DIR)/errors/logical-non-bool.err 2>&1; then exit 1; fi
	grep -q "logical operand must be bool, got i64" $(BUILD_DIR)/errors/logical-non-bool.err
	if $(TARGET) tests/errors/not-non-bool.ari -o $(BUILD_DIR)/errors/not-non-bool.elf >$(BUILD_DIR)/errors/not-non-bool.err 2>&1; then exit 1; fi
	grep -q "logical operand must be bool, got i64" $(BUILD_DIR)/errors/not-non-bool.err
	if $(TARGET) tests/errors/invalid-cast.ari -o $(BUILD_DIR)/errors/invalid-cast.elf >$(BUILD_DIR)/errors/invalid-cast.err 2>&1; then exit 1; fi
	grep -q "explicit casts currently require integer types, float types, integer/float casts, or raw pointer casts, got bool as i64" $(BUILD_DIR)/errors/invalid-cast.err
	if $(TARGET) tests/errors/enum-payload-too-wide.ari -o $(BUILD_DIR)/errors/enum-payload-too-wide.elf >$(BUILD_DIR)/errors/enum-payload-too-wide.err 2>&1; then exit 1; fi
	grep -q "enum aggregate payloads currently support integer, bool, pointer-shaped, one-word enum, or homogeneous nested aggregate enum values" $(BUILD_DIR)/errors/enum-payload-too-wide.err
	if $(TARGET) tests/errors/enum-payload-count.ari -o $(BUILD_DIR)/errors/enum-payload-count.elf >$(BUILD_DIR)/errors/enum-payload-count.err 2>&1; then exit 1; fi
	grep -q "wrong payload count for enum case 'Some'" $(BUILD_DIR)/errors/enum-payload-count.err
	if $(TARGET) tests/errors/match-nonexhaustive.ari -o $(BUILD_DIR)/errors/match-nonexhaustive.elf >$(BUILD_DIR)/errors/match-nonexhaustive.err 2>&1; then exit 1; fi
	grep -q "match must cover all cases of enum 'OptionI32'" $(BUILD_DIR)/errors/match-nonexhaustive.err
	if $(TARGET) tests/errors/match-payload-required.ari -o $(BUILD_DIR)/errors/match-payload-required.elf >$(BUILD_DIR)/errors/match-payload-required.err 2>&1; then exit 1; fi
	grep -q "enum case 'Some' requires a payload pattern" $(BUILD_DIR)/errors/match-payload-required.err
	if $(TARGET) tests/errors/match-empty.ari -o $(BUILD_DIR)/errors/match-empty.elf >$(BUILD_DIR)/errors/match-empty.err 2>&1; then exit 1; fi
	grep -q "match must have at least one arm" $(BUILD_DIR)/errors/match-empty.err
	if $(TARGET) tests/errors/match-duplicate-arm.ari -o $(BUILD_DIR)/errors/match-duplicate-arm.elf >$(BUILD_DIR)/errors/match-duplicate-arm.err 2>&1; then exit 1; fi
	grep -q "duplicate match arm for enum case 'A'" $(BUILD_DIR)/errors/match-duplicate-arm.err
	if $(TARGET) tests/errors/match-unreachable-after-wildcard.ari -o $(BUILD_DIR)/errors/match-unreachable-after-wildcard.elf >$(BUILD_DIR)/errors/match-unreachable-after-wildcard.err 2>&1; then exit 1; fi
	grep -q "unreachable match arm after wildcard" $(BUILD_DIR)/errors/match-unreachable-after-wildcard.err
	if $(TARGET) tests/errors/match-no-payload.ari -o $(BUILD_DIR)/errors/match-no-payload.elf >$(BUILD_DIR)/errors/match-no-payload.err 2>&1; then exit 1; fi
	grep -q "enum case 'A' has no payload" $(BUILD_DIR)/errors/match-no-payload.err
	if $(TARGET) tests/errors/match-wrong-enum-case.ari -o $(BUILD_DIR)/errors/match-wrong-enum-case.elf >$(BUILD_DIR)/errors/match-wrong-enum-case.err 2>&1; then exit 1; fi
	grep -q "enum case 'B' does not belong to Left" $(BUILD_DIR)/errors/match-wrong-enum-case.err
	if $(TARGET) tests/errors/match-non-enum.ari -o $(BUILD_DIR)/errors/match-non-enum.elf >$(BUILD_DIR)/errors/match-non-enum.err 2>&1; then exit 1; fi
	grep -q "match value must be an enum, integer, bool, tuple, array, or struct, got string" $(BUILD_DIR)/errors/match-non-enum.err
	if $(TARGET) tests/errors/match-branch-state-mismatch.ari -o $(BUILD_DIR)/errors/match-branch-state-mismatch.elf >$(BUILD_DIR)/errors/match-branch-state-mismatch.err 2>&1; then exit 1; fi
	grep -q "has incompatible ownership states after match arms" $(BUILD_DIR)/errors/match-branch-state-mismatch.err
	if $(TARGET) tests/errors/private-module-function.ari -o $(BUILD_DIR)/errors/private-module-function.elf >$(BUILD_DIR)/errors/private-module-function.err 2>&1; then exit 1; fi
	grep -q "function 'Secret::value' is not public" $(BUILD_DIR)/errors/private-module-function.err
	if $(TARGET) tests/errors/duplicate-use-alias.ari -o $(BUILD_DIR)/errors/duplicate-use-alias.elf >$(BUILD_DIR)/errors/duplicate-use-alias.err 2>&1; then exit 1; fi
	grep -q "duplicate use alias 'value'" $(BUILD_DIR)/errors/duplicate-use-alias.err
	if $(TARGET) tests/errors/private-module-use.ari -o $(BUILD_DIR)/errors/private-module-use.elf >$(BUILD_DIR)/errors/private-module-use.err 2>&1; then exit 1; fi
	grep -q "function 'Secret::value' is not public" $(BUILD_DIR)/errors/private-module-use.err
	if $(TARGET) tests/errors/private-module-enum-type.ari -o $(BUILD_DIR)/errors/private-module-enum-type.elf >$(BUILD_DIR)/errors/private-module-enum-type.err 2>&1; then exit 1; fi
	grep -q "enum 'Hidden::Secret' is not public" $(BUILD_DIR)/errors/private-module-enum-type.err
	if $(TARGET) tests/errors/private-module-enum-case.ari -o $(BUILD_DIR)/errors/private-module-enum-case.elf >$(BUILD_DIR)/errors/private-module-enum-case.err 2>&1; then exit 1; fi
	grep -q "enum case 'Hidden::A' is not public" $(BUILD_DIR)/errors/private-module-enum-case.err
	if $(TARGET) tests/errors/missing-module.ari -o $(BUILD_DIR)/errors/missing-module.elf >$(BUILD_DIR)/errors/missing-module.err 2>&1; then exit 1; fi
	grep -q "cannot find module file for 'definitely_missing_module'" $(BUILD_DIR)/errors/missing-module.err
	if $(TARGET) --freestanding tests/errors/vector-backend.ari -o $(BUILD_DIR)/errors/vector-backend.elf >$(BUILD_DIR)/errors/vector-backend.err 2>&1; then exit 1; fi
	grep -q "backend does not lower vector values yet" $(BUILD_DIR)/errors/vector-backend.err
	if $(TARGET) tests/errors/format-placeholder-count.ari -o $(BUILD_DIR)/errors/format-placeholder-count.elf >$(BUILD_DIR)/errors/format-placeholder-count.err 2>&1; then exit 1; fi
	grep -q "format string has 2 placeholders but 1 values were provided" $(BUILD_DIR)/errors/format-placeholder-count.err
	if $(TARGET) tests/errors/format-nonliteral.ari -o $(BUILD_DIR)/errors/format-nonliteral.elf >$(BUILD_DIR)/errors/format-nonliteral.err 2>&1; then exit 1; fi
	grep -q "expects a string literal format argument" $(BUILD_DIR)/errors/format-nonliteral.err
	if $(TARGET) tests/errors/extern-cpp-removed.ari -o $(BUILD_DIR)/errors/extern-cpp-removed.elf >$(BUILD_DIR)/errors/extern-cpp-removed.err 2>&1; then exit 1; fi
	grep -q 'extern ABI must be "C" or "ari"' $(BUILD_DIR)/errors/extern-cpp-removed.err
	if $(TARGET) tests/errors/extern-body.ari -o $(BUILD_DIR)/errors/extern-body.elf >$(BUILD_DIR)/errors/extern-body.err 2>&1; then exit 1; fi
	grep -q "extern functions cannot have a body" $(BUILD_DIR)/errors/extern-body.err
	if $(TARGET) tests/errors/extern-generic.ari -o $(BUILD_DIR)/errors/extern-generic.elf >$(BUILD_DIR)/errors/extern-generic.err 2>&1; then exit 1; fi
	grep -q "extern C declarations cannot be generic; declare concrete C symbols or wrap generic foreign APIs in C" $(BUILD_DIR)/errors/extern-generic.err
	if $(TARGET) tests/errors/extern-invalid-link-name.ari -o $(BUILD_DIR)/errors/extern-invalid-link-name.elf >$(BUILD_DIR)/errors/extern-invalid-link-name.err 2>&1; then exit 1; fi
	grep -q "invalid external link symbol 'bad-symbol'" $(BUILD_DIR)/errors/extern-invalid-link-name.err
