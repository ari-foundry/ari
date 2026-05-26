# Generated Standard Library API Index

This file is generated from `tests/std_api_manifest.txt` by
`python3 tools/generate_std_api_docs.py`. Do not hand-edit this file;
edit the manifest coverage note or the source API, then regenerate it.

Use this index when you need the exact public `std` spellings. Use the
hand-written module guides for purpose, examples, ownership rules, and
platform notes.

## How To Read This File

- `API` is the public declaration shape extracted from `lib/std.arih` and
  `lib/std/*.arih`.
- `Coverage note` names the focused test or docs coverage that intentionally
  tracks the API.
- `Tier` comes from the standard-library stability policy: core APIs should
  be portable, alloc APIs require explicit zones or owned buffers, hosted
  APIs require OS/runtime support, and platform APIs expose target or ABI
  facts.
- Generated entries describe current APIs, not future roadmap items. Future
  or experimental APIs should stay in roadmap docs until they have source,
  tests, and manifest coverage.

## Summary

- API entries: `3409`
- Modules: `40`

| Tier | Entries | Stability reading |
| --- | ---: | --- |
| `alloc` | 893 | usable |
| `alloc/hosted` | 36 | usable with hosted entropy APIs |
| `core` | 902 | stable candidate |
| `hosted` | 1415 | platform-backed |
| `platform` | 163 | platform-specific |

| Kind | Entries |
| --- | ---: |
| `enum` | 32 |
| `fn` | 1194 |
| `method` | 1765 |
| `module` | 39 |
| `struct` | 178 |
| `trait` | 39 |
| `trait-method` | 41 |
| `type` | 22 |
| `use` | 99 |

## Modules

| Module | Tier | Entries |
| --- | --- | ---: |
| `std` | `core` | 296 |
| `std::algo` | `alloc` | 41 |
| `std::ascii` | `core` | 33 |
| `std::bits` | `core` | 26 |
| `std::boxed` | `alloc` | 19 |
| `std::c` | `platform` | 49 |
| `std::cell` | `alloc` | 44 |
| `std::cmp` | `core` | 38 |
| `std::collections` | `alloc` | 385 |
| `std::context` | `hosted` | 20 |
| `std::convert` | `core` | 14 |
| `std::encoding` | `core` | 98 |
| `std::env` | `hosted` | 68 |
| `std::error` | `core` | 37 |
| `std::fmt` | `core` | 63 |
| `std::fs` | `hosted` | 325 |
| `std::hash` | `alloc` | 24 |
| `std::input` | `hosted` | 6 |
| `std::io` | `hosted` | 111 |
| `std::iter` | `alloc` | 41 |
| `std::log` | `hosted` | 12 |
| `std::math` | `core` | 39 |
| `std::mem` | `core` | 13 |
| `std::net` | `hosted` | 315 |
| `std::option` | `core` | 12 |
| `std::os` | `platform` | 62 |
| `std::parse` | `core` | 101 |
| `std::path` | `core` | 120 |
| `std::process` | `hosted` | 153 |
| `std::random` | `alloc/hosted` | 36 |
| `std::rc` | `alloc` | 32 |
| `std::result` | `core` | 12 |
| `std::string` | `alloc` | 171 |
| `std::sync` | `hosted` | 209 |
| `std::target` | `platform` | 52 |
| `std::test` | `hosted` | 32 |
| `std::thread` | `hosted` | 90 |
| `std::time` | `hosted` | 74 |
| `std::vec` | `alloc` | 115 |
| `std::zone` | `alloc` | 21 |

## `std`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::Option[T]` | prelude Option/Result tests; docs/dev/test-matrix.md Prelude and Generics rows |
| `enum std::Result[T, E]` | prelude Option/Result tests; docs/dev/test-matrix.md Prelude and Generics rows |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::assert` | prelude assertion tests and macros; docs/dev/test-matrix.md Prelude row |
| `fn std::assert_eq_bool` | prelude assertion macro tests; docs/dev/test-matrix.md Prelude row |
| `fn std::assert_eq_i64` | prelude assertion macro tests; docs/dev/test-matrix.md Prelude row |
| `fn std::assert_equal[T]` | prelude natural generic assertion tests; docs/language/prelude.md |
| `fn std::assert_ne_bool` | prelude assertion macro tests; docs/dev/test-matrix.md Prelude row |
| `fn std::assert_ne_i64` | prelude assertion macro tests; docs/dev/test-matrix.md Prelude row |
| `fn std::assert_not_equal[T]` | prelude natural generic assertion tests; docs/language/prelude.md |
| `fn std::debug_assert` | prelude assertion tests and macros; docs/dev/test-matrix.md Prelude row |
| `fn std::move[T]` | explicit move tests; docs/dev/test-matrix.md Ownership row |
| `fn std::panic` | prelude panic helpers; docs/dev/test-matrix.md Prelude row |
| `fn std::slice[T]` | Slice construction tests; docs/dev/test-matrix.md Prelude row |
| `fn std::take[T]` | explicit take tests; docs/dev/test-matrix.md Ownership row |
| `fn std::todo` | planned panic helper tests; docs/dev/test-matrix.md Prelude row |
| `fn std::unreachable` | planned panic helper tests; docs/dev/test-matrix.md Prelude row |

### method

| API | Coverage note |
| --- | --- |
| `method SlicePair[T]::left` | check-prelude prelude-slice-sequence split_at left half accessor; docs/stdlib/modules/slice.md |
| `method SlicePair[T]::right` | check-prelude prelude-slice-sequence split_at right half accessor; docs/stdlib/modules/slice.md |
| `method SliceValueMut[T]::value` | check-prelude std-iter-slice-vec mutable slice cursor value copy accessor; docs/stdlib/modules/slice.md |
| `method SliceValueMut[T]::value_mut` | check-prelude std-iter-slice-vec mutable slice cursor value borrow accessor; docs/stdlib/modules/slice.md |
| `method Slice[T]::all` | check-prelude prelude-slice-convenience borrowed-predicate universal check; docs/stdlib/modules/slice.md |
| `method Slice[T]::any` | check-prelude prelude-slice-convenience borrowed-predicate existential check; docs/stdlib/modules/slice.md |
| `method Slice[T]::as_ptr` | check-prelude prelude-slice-as-ptr borrowed receiver and provenance tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `method Slice[T]::chunks` | check-prelude prelude-slice-sequence lazy borrowed chunk iterator; docs/stdlib/modules/slice.md |
| `method Slice[T]::compare` | check-prelude prelude-slice-sequence lexicographic slice compare; docs/stdlib/modules/slice.md |
| `method Slice[T]::contains` | check-prelude prelude-slice-methods borrowed receiver tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::contains_slice` | check-prelude prelude-slice-sequence borrowed subsequence predicate; docs/stdlib/modules/slice.md |
| `method Slice[T]::count` | check-prelude prelude-slice-methods borrowed receiver tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::count_if` | check-prelude prelude-slice-convenience borrowed-predicate count; docs/stdlib/modules/slice.md |
| `method Slice[T]::ends_with` | check-prelude prelude-slice-methods borrowed receiver Slice[T] tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::equals` | check-prelude prelude-slice-methods borrowed receiver Slice[T] tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::find` | check-prelude prelude-slice-sequence borrowed subsequence search; docs/stdlib/modules/slice.md |
| `method Slice[T]::find_if` | check-prelude prelude-slice-convenience borrowed-predicate value search; docs/stdlib/modules/slice.md |
| `method Slice[T]::first` | check-prelude prelude-slice-methods borrowed receiver tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::first_mut` | check-prelude prelude-slice-convenience mutable first-element borrow; docs/stdlib/modules/slice.md |
| `method Slice[T]::get` | check-prelude prelude-slice-methods borrowed receiver tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::get_mut` | check-prelude prelude-slice-convenience mutable indexed element borrow; docs/stdlib/modules/slice.md |
| `method Slice[T]::index_of` | check-prelude prelude-slice-methods borrowed receiver tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::is_empty` | check-prelude prelude-slice-metadata borrowed receiver metadata helper; docs/stdlib/api-reference.md Slice section |
| `method Slice[T]::iter` | check-prelude std-iter-slice-vec borrowed slice iterator; docs/stdlib/modules/slice.md |
| `method Slice[T]::iter_mut` | check-prelude std-iter-slice-vec borrowed slice mutable value cursor; docs/stdlib/modules/slice.md |
| `method Slice[T]::last` | check-prelude prelude-slice-methods borrowed receiver tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::last_mut` | check-prelude prelude-slice-convenience mutable last-element borrow; docs/stdlib/modules/slice.md |
| `method Slice[T]::len` | check-prelude prelude-slice-metadata natural borrowed length helper; docs/stdlib/api-reference.md Slice section |
| `method Slice[T]::ordering` | check-prelude prelude-slice-sequence lexicographic Ordering comparison; docs/stdlib/modules/slice.md |
| `method Slice[T]::position` | check-prelude prelude-slice-convenience borrowed-predicate first index search; docs/stdlib/modules/slice.md |
| `method Slice[T]::rposition` | check-prelude prelude-slice-convenience borrowed-predicate last index search; docs/stdlib/modules/slice.md |
| `method Slice[T]::slice` | check-prelude prelude-slice-sequence borrowed range view helper; docs/stdlib/modules/slice.md |
| `method Slice[T]::split` | check-prelude prelude-slice-sequence lazy borrowed delimiter split iterator; docs/stdlib/modules/slice.md |
| `method Slice[T]::split_at` | check-prelude prelude-slice-sequence borrowed left/right split helper; docs/stdlib/modules/slice.md |
| `method Slice[T]::split_first` | check-prelude prelude-slice-convenience optional first value and tail view; docs/stdlib/modules/slice.md |
| `method Slice[T]::split_last` | check-prelude prelude-slice-convenience optional init view and last value; docs/stdlib/modules/slice.md |
| `method Slice[T]::starts_with` | check-prelude prelude-slice-methods borrowed receiver Slice[T] tests; docs/dev/test-matrix.md Prelude row |
| `method Slice[T]::strip_prefix` | check-prelude prelude-slice-convenience optional prefix-stripped view; docs/stdlib/modules/slice.md |
| `method Slice[T]::strip_suffix` | check-prelude prelude-slice-convenience optional suffix-stripped view; docs/stdlib/modules/slice.md |
| `method Slice[T]::try_first` | check-prelude prelude-slice-option-access Option-returning empty-safe access; docs/stdlib/api-reference.md Slice section |
| `method Slice[T]::try_get` | check-prelude prelude-slice-option-access Option-returning out-of-range access; docs/stdlib/api-reference.md Slice section |
| `method Slice[T]::try_last` | check-prelude prelude-slice-option-access Option-returning empty-safe access; docs/stdlib/api-reference.md Slice section |
| `method Slice[T]::windows` | check-prelude prelude-slice-sequence lazy borrowed window iterator; docs/stdlib/modules/slice.md |
| `method std::Option[T]::and[U]` | check-prelude prelude-option-result-combinators eager present-branch combinator; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::and_then[U]` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::as_mut` | check-prelude prelude-option-result-ref-access mutable borrowed option payload view handle; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::as_ref` | check-prelude prelude-option-result-ref-access shared borrowed option payload view handle; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::contains` | check-prelude prelude-option-result-predicates exact value-membership predicate; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::expect` | check-prelude prelude-option-result-unwrap; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::filter` | check-prelude prelude-option-filter borrowed predicate helper; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[T]::get_or_insert` | check-prelude prelude-option-result-ref-access mutable payload initialization helper; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::get_or_insert_with` | check-prelude prelude-option-result-ref-access lazy mutable payload initialization helper; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::inspect` | check-prelude prelude-option-result-combinators borrowed present-branch inspection helper; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::is_none` | check-prelude prelude-option-result-methods; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::is_none_or` | check-prelude prelude-option-result-predicates consuming predicate helper; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[T]::is_some` | check-prelude prelude-option-result-methods; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::is_some_and` | check-prelude prelude-option-result-predicates consuming predicate helper; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[T]::map[U]` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::map_or[U]` | check-prelude prelude-option-result-combinators eager fallback mapping helper; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::map_or_else[U]` | check-prelude prelude-option-result-combinators lazy fallback mapping helper; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::ok_or[E]` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[T]::ok_or_else[E]` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[T]::or` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::or_else` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::replace` | check-prelude prelude-option-result-ref-access replace payload and return previous option; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::take` | check-prelude prelude-option-result-ref-access move payload out and leave None; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::unwrap` | check-prelude prelude-option-result-unwrap; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::unwrap_or` | check-prelude prelude-option-result-methods; docs/dev/test-matrix.md Prelude row |
| `method std::Option[T]::unwrap_or_default` | check-prelude prelude-option-result-ref-access default-backed absence fallback helper; docs/stdlib/modules/option-result.md |
| `method std::Option[T]::unwrap_or_else` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[T]::xor` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[std::Option[T]]::flatten` | check-prelude prelude-option-flatten; docs/stdlib/api-reference.md Option and Result section |
| `method std::Option[std::Result[T,E]]::transpose` | check-prelude prelude-option-transpose; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[T,E]::and[U]` | check-prelude prelude-option-result-combinators eager success-branch combinator; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::and_then[U]` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::as_mut` | check-prelude prelude-option-result-ref-access mutable borrowed result payload view handle; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::as_ref` | check-prelude prelude-option-result-ref-access shared borrowed result payload view handle; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::contains` | check-prelude prelude-option-result-predicates exact success value-membership predicate; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::contains_err` | check-prelude prelude-option-result-predicates exact error value-membership predicate; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::err` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[T,E]::expect` | check-prelude prelude-option-result-unwrap; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::expect_err` | check-prelude prelude-option-result-unwrap; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::inspect` | check-prelude prelude-option-result-combinators borrowed success inspection helper; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::inspect_err` | check-prelude prelude-option-result-combinators borrowed error inspection helper; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::is_err` | check-prelude prelude-option-result-methods; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::is_err_and` | check-prelude prelude-option-result-predicates consuming error predicate helper; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[T,E]::is_ok` | check-prelude prelude-option-result-methods; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::is_ok_and` | check-prelude prelude-option-result-predicates consuming success predicate helper; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[T,E]::map[U]` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::map_err[F]` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::map_or[U]` | check-prelude prelude-option-result-combinators eager success fallback mapping helper; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::map_or_else[U]` | check-prelude prelude-option-result-combinators lazy error fallback mapping helper; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::ok` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[T,E]::or[F]` | check-prelude prelude-option-result-combinators; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[T,E]::or_else[F]` | check-prelude prelude-option-result-combinators; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::unwrap` | check-prelude prelude-option-result-unwrap; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::unwrap_err` | check-prelude prelude-option-result-unwrap; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::unwrap_or` | check-prelude prelude-option-result-methods; docs/dev/test-matrix.md Prelude row |
| `method std::Result[T,E]::unwrap_or_default` | check-prelude prelude-option-result-ref-access default-backed error fallback helper; docs/stdlib/modules/option-result.md |
| `method std::Result[T,E]::unwrap_or_else` | check-prelude prelude-option-result-conversions; docs/stdlib/api-reference.md Option and Result section |
| `method std::Result[std::Option[T],E]::transpose` | check-prelude prelude-result-transpose; docs/stdlib/api-reference.md Option and Result section |
| `method std::Slice[T]::binary_search` | check-prelude prelude-slice-sequence ordered borrowed slice binary search wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::binary_search_by` | check-prelude std-algo-by-helpers comparator borrowed slice binary search wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::copy_from` | check-prelude prelude-slice-sequence borrowed target copy wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::copy_to` | check-prelude prelude-slice-copy-to borrowed receiver and target reset tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `method std::Slice[T]::copy_within` | check-prelude std-vec-range-mutation borrowed slice overlap-safe in-place range copy; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::dedup` | check-prelude prelude-slice-sequence consecutive duplicate compaction wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::dedup_by` | check-prelude std-algo-dedup-partition comparator dedup wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::dedup_by_key[K]` | check-prelude std-algo-dedup-partition key-based dedup wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::equal_range` | check-prelude prelude-slice-sequence ordered equal-range wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::equal_range_by` | check-prelude std-algo-by-helpers comparator equal-range wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::fill` | check-prelude prelude-slice-sequence borrowed target fill wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::fill_range` | check-prelude std-vec-range-mutation borrowed slice half-open range fill; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::is_sorted` | check-prelude prelude-slice-sequence ordered borrowed slice sortedness wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::is_sorted_by` | check-prelude std-algo-by-helpers comparator borrowed slice sortedness wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::lower_bound` | check-prelude prelude-slice-sequence ordered borrowed slice lower-bound wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::lower_bound_by` | check-prelude std-algo-by-helpers comparator borrowed slice lower-bound wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::max` | check-prelude prelude-slice-sequence ordered borrowed slice maximum wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::max_by` | check-prelude std-algo-by-helpers comparator borrowed slice maximum wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::min` | check-prelude prelude-slice-sequence ordered borrowed slice minimum wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::min_by` | check-prelude std-algo-by-helpers comparator borrowed slice minimum wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::partition` | check-prelude prelude-slice-sequence borrowed predicate partition wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::partition_point` | check-prelude prelude-slice-sequence partition-point wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::reverse` | check-prelude prelude-slice-sequence in-place borrowed slice reverse wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::reverse_range` | check-prelude std-vec-range-mutation borrowed slice half-open range reverse; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::rotate_left` | check-prelude prelude-slice-sequence borrowed slice left rotation wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::rotate_range` | check-prelude std-vec-range-mutation borrowed slice half-open range left rotation; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::rotate_right` | check-prelude prelude-slice-sequence borrowed slice right rotation wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::sort` | check-prelude prelude-slice-sequence ordered borrowed slice sort wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::sort_by` | check-prelude std-algo-by-helpers comparator borrowed slice sort wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::stable_partition` | check-prelude std-algo-dedup-partition stable borrowed-predicate partition wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::stable_sort` | check-prelude prelude-slice-sequence ordered borrowed slice stable sort wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::stable_sort_by` | check-prelude std-algo-by-helpers comparator borrowed slice stable sort wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::stable_sort_by_in` | check-prelude std-algo-final-sort comparator borrowed slice stable sort with explicit temporary zone; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::stable_sort_in` | check-prelude std-algo-final-sort ordered borrowed slice stable sort with explicit temporary zone; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::try_stable_sort` | check-prelude std-algo-final-sort direct Result borrowed slice stable sort wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::try_stable_sort_by` | check-prelude std-algo-final-sort comparator direct Result borrowed slice stable sort wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::upper_bound` | check-prelude prelude-slice-sequence ordered borrowed slice upper-bound wrapper; docs/stdlib/modules/slice.md |
| `method std::Slice[T]::upper_bound_by` | check-prelude std-algo-by-helpers comparator borrowed slice upper-bound wrapper; docs/stdlib/modules/slice.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::RangeInclusive[T]` | inclusive range tests; docs/dev/test-matrix.md Control flow row |
| `struct std::Range[T]` | range tests; docs/dev/test-matrix.md Control flow row |
| `struct std::SliceChunks[T]` | check-prelude prelude-slice-sequence borrowed chunk iterator; docs/stdlib/modules/slice.md |
| `struct std::SliceIterMut[T]` | check-prelude std-iter-slice-vec borrowed slice mutable value cursor; docs/stdlib/modules/slice.md |
| `struct std::SliceIter[T]` | check-prelude std-iter-slice-vec borrowed slice value iterator; docs/stdlib/modules/slice.md |
| `struct std::SlicePair[T]` | check-prelude prelude-slice-sequence split_at result pair; docs/stdlib/modules/slice.md |
| `struct std::SliceSplit[T]` | check-prelude prelude-slice-sequence borrowed delimiter split iterator; docs/stdlib/modules/slice.md |
| `struct std::SliceValueMut[T]` | check-prelude std-iter-slice-vec mutable slice value handle; docs/stdlib/modules/slice.md |
| `struct std::SliceWindows[T]` | check-prelude prelude-slice-sequence borrowed window iterator; docs/stdlib/modules/slice.md |
| `struct std::Slice[T]` | Slice view tests; docs/dev/test-matrix.md Prelude row |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::Clone` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::Copy` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::Default` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::DoubleEndedIterator[T]` | check-prelude std-iter-double-ended supertrait child of Iterator; docs/stdlib/modules/iter.md |
| `trait std::Drop` | ownership destructor tests; docs/dev/test-matrix.md Ownership row |
| `trait std::Eq[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::ExactSizeIterator[T]` | check-prelude std-iter-exact-size supertrait child of Iterator; docs/stdlib/modules/iter.md |
| `trait std::From[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::IntoIterator[T]` | iterator tests; docs/dev/test-matrix.md Control flow and Front-end surfaces rows |
| `trait std::Into[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::Iterable[T]` | iterator tests; docs/dev/test-matrix.md Control flow and Front-end surfaces rows |
| `trait std::Iterator[T]` | iterator tests; docs/dev/test-matrix.md Control flow and Front-end surfaces rows |
| `trait std::Ord[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::PartialEq[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::PartialOrd[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::ToOwned` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::ToString` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::TryFrom[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::TryInto[T]` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::Clone::clone` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::Default::default` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::DoubleEndedIterator[T]::next_back` | check-prelude std-iter-double-ended back-to-front cursor method; docs/stdlib/modules/iter.md |
| `trait-method std::Drop::drop` | ownership destructor tests; docs/dev/test-matrix.md Ownership row |
| `trait-method std::Eq[T]::eq` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::ExactSizeIterator[T]::len` | check-prelude std-iter-exact-size exact remaining-length method; docs/stdlib/modules/iter.md |
| `trait-method std::From[T]::from` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::IntoIterator[T]::into_iter` | iterator tests; docs/dev/test-matrix.md Control flow row |
| `trait-method std::Into[T]::into` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::Iterator[T]::next` | iterator tests; docs/dev/test-matrix.md Control flow row |
| `trait-method std::Ord[T]::lt` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::PartialEq[T]::eq` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::PartialOrd[T]::lt` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::ToString::to_string` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::TryFrom[T]::try_from` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::TryInto[T]::try_into` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |

### type

| API | Coverage note |
| --- | --- |
| `type std::char` | check-generics type-alias-char/no-implicit-std tests; docs/language/types.md Type Aliases section |

### use

| API | Coverage note |
| --- | --- |
| `use std::Arc` | check-prelude std-rc-arc-weak root alias for std::rc::Arc[T]; docs/stdlib/modules/rc.md |
| `use std::AtomicBool` | check-prelude std-sync-concurrency-api root alias for std::sync::AtomicBool; docs/stdlib/modules/sync.md |
| `use std::AtomicI64` | check-prelude std-sync-atomic-i64 root alias for std::sync::AtomicI64; docs/stdlib/modules/sync.md |
| `use std::AtomicPtr` | check-prelude std-sync-concurrency-api root alias for std::sync::AtomicPtr[T]; docs/stdlib/modules/sync.md |
| `use std::AtomicUsize` | check-prelude std-sync-concurrency-api root alias for std::sync::AtomicUsize; docs/stdlib/modules/sync.md |
| `use std::Barrier` | check-prelude std-sync-concurrency-api root alias for std::sync::Barrier; docs/stdlib/modules/sync.md |
| `use std::BinaryHeap` | check-prelude std-collections-heap root alias for std::collections::BinaryHeap[T]; docs/stdlib/modules/collections.md |
| `use std::Box` | prelude root Box tests; docs/dev/test-matrix.md Explicit memory zones row |
| `use std::Builder` | check-prelude std-thread-builder root alias for std::thread::Builder; docs/stdlib/modules/thread.md |
| `use std::CStr` | check-prelude std-c-interop root alias for std::c::CStr; docs/stdlib/modules/c.md |
| `use std::CString` | check-prelude std-c-interop root alias for std::c::CString; docs/stdlib/modules/c.md |
| `use std::Cell` | check-prelude std-cell-basic root alias for std::cell::Cell[T]; docs/stdlib/modules/cell.md |
| `use std::Channel` | check-prelude std-sync-concurrency-api root alias for std::sync::Channel[T]; docs/stdlib/modules/sync.md |
| `use std::Condvar` | check-prelude std-sync-concurrency-api root alias for std::sync::Condvar; docs/stdlib/modules/sync.md |
| `use std::Debug` | check-prelude std-string-append-value root alias for std::fmt::Debug; docs/stdlib/modules/fmt.md |
| `use std::Deque` | check-prelude std-collections-deque root alias for std::collections::Deque[T]; docs/stdlib/modules/collections.md |
| `use std::Display` | check-prelude std-string-append-value root alias for std::fmt::Display; docs/stdlib/modules/fmt.md |
| `use std::Error` | check-prelude std-error-basic root alias for std::error::Error; docs/stdlib/modules/error.md |
| `use std::ErrorKind` | check-prelude std-error-basic root alias for std::error::Kind; docs/stdlib/modules/error.md |
| `use std::HashMap` | check-prelude std-collections-hash root alias for std::collections::HashMap[K, V]; docs/stdlib/modules/collections.md |
| `use std::HashSet` | check-prelude std-collections-hash root alias for std::collections::HashSet[T]; docs/stdlib/modules/collections.md |
| `use std::JoinError` | check-prelude std-thread-basic root alias for std::thread::JoinError; docs/stdlib/modules/thread.md |
| `use std::JoinHandle` | check-prelude std-thread-basic root alias for std::thread::JoinHandle; docs/stdlib/modules/thread.md |
| `use std::Lazy` | check-prelude std-cell-basic root alias for std::cell::Lazy[T]; docs/stdlib/modules/cell.md |
| `use std::Library` | check-prelude std-c-interop root alias for std::c::Library; docs/stdlib/modules/c.md |
| `use std::LinkedList` | check-prelude std-collections-linked-list root alias for std::collections::LinkedList[T]; docs/stdlib/modules/collections.md |
| `use std::Mutex` | check-prelude std-sync-value-locks root alias for std::sync::Mutex[T]; docs/stdlib/modules/sync.md |
| `use std::Once` | check-prelude std-sync-mutex-once root alias for std::sync::Once; docs/stdlib/modules/sync.md |
| `use std::OnceCell` | check-prelude std-cell-basic root alias for std::cell::OnceCell[T]; docs/stdlib/modules/cell.md |
| `use std::OnceLock` | check-prelude std-sync-concurrency-api root alias for std::sync::OnceLock[T]; docs/stdlib/modules/sync.md |
| `use std::PriorityQueue` | check-prelude std-collections-heap root alias for std::collections::PriorityQueue[T]; docs/stdlib/modules/collections.md |
| `use std::Prng` | check-prelude std-random-basic root alias for std::random::Prng; docs/stdlib/modules/random.md |
| `use std::RawMutex` | check-prelude std-sync-mutex-once root alias for std::sync::RawMutex; docs/stdlib/modules/sync.md |
| `use std::RawRwLock` | check-prelude std-sync-rwlock root alias for std::sync::RawRwLock; docs/stdlib/modules/sync.md |
| `use std::Rc` | check-prelude std-rc-arc-weak root alias for std::rc::Rc[T]; docs/stdlib/modules/rc.md |
| `use std::Receiver` | check-prelude std-sync-concurrency-api root alias for std::sync::Receiver[T]; docs/stdlib/modules/sync.md |
| `use std::RefCell` | check-prelude std-cell-basic root alias for std::cell::RefCell[T]; docs/stdlib/modules/cell.md |
| `use std::RingBuffer` | check-prelude std-collections-ring-buffer root alias for std::collections::RingBuffer[T]; docs/stdlib/modules/collections.md |
| `use std::RwLock` | check-prelude std-sync-value-locks root alias for std::sync::RwLock[T]; docs/stdlib/modules/sync.md |
| `use std::Sender` | check-prelude std-sync-concurrency-api root alias for std::sync::Sender[T]; docs/stdlib/modules/sync.md |
| `use std::Set` | check-prelude std-collections-set root alias for std::collections::Set[T]; docs/stdlib/modules/collections.md |
| `use std::String` | prelude root String tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::Symbol` | check-prelude std-c-interop root alias for std::c::Symbol; docs/stdlib/modules/c.md |
| `use std::Thread` | check-prelude std-thread-basic root alias for std::thread::Thread; docs/stdlib/modules/thread.md |
| `use std::ThreadId` | check-prelude std-thread-basic root alias for std::thread::ThreadId; docs/stdlib/modules/thread.md |
| `use std::ThreadLocal` | check-prelude std-thread-local root alias for std::thread::ThreadLocal[T]; docs/stdlib/modules/thread.md |
| `use std::ThreadLocalSetError` | check-prelude std-thread-local root alias for std::thread::ThreadLocalSetError[T]; docs/stdlib/modules/thread.md |
| `use std::ThreadResult` | check-prelude std-thread-basic root alias for std::thread::ThreadResult; docs/stdlib/modules/thread.md |
| `use std::TreeMap` | check-prelude std-collections-tree root alias for std::collections::TreeMap[K, V]; docs/stdlib/modules/collections.md |
| `use std::TreeSet` | check-prelude std-collections-tree root alias for std::collections::TreeSet[T]; docs/stdlib/modules/collections.md |
| `use std::Vec` | check-prelude std-vec-root-alias explicit-zone alias for std::vec::Vec[T]; docs/dev/test-matrix.md Explicit memory zones row |
| `use std::Weak` | check-prelude std-rc-arc-weak root alias for std::rc::Weak[T]; docs/stdlib/modules/rc.md |
| `use std::ZoneBacked` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `use std::ZoneMetadata` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `use std::align_of` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::alloc` | std root re-export tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::alloc_array` | std zone raw array allocation root re-export tests; docs/stdlib/modules/zone.md |
| `use std::arg` | std root re-export tests; docs/dev/test-matrix.md Prelude and Context runtime rows |
| `use std::arg_count` | std root re-export tests; docs/dev/test-matrix.md Prelude and Context runtime rows |
| `use std::clamp` | std cmp value helper root re-export tests; docs/dev/test-matrix.md Front-end surfaces row |
| `use std::clamp_by` | check-prelude std-cmp-by-helpers comparator-based clamp root re-export; docs/stdlib/modules/cmp.md |
| `use std::create` | std root re-export tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::destroy` | std root re-export tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::from_zone` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `use std::has_arg` | std context args root re-export tests; docs/stdlib/modules/context.md |
| `use std::input_owned` | std root re-export owned read_line tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::is_between` | std cmp value helper root re-export tests; docs/stdlib/api-reference.md Comparison section |
| `use std::is_between_by` | check-prelude std-cmp-by-helpers comparator-based range root re-export; docs/stdlib/modules/cmp.md |
| `use std::max` | std cmp value helper root re-export tests; docs/dev/test-matrix.md Front-end surfaces row |
| `use std::max_by` | check-prelude std-cmp-by-helpers comparator-based max root re-export; docs/stdlib/modules/cmp.md |
| `use std::metadata` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `use std::min` | std cmp value helper root re-export tests; docs/dev/test-matrix.md Front-end surfaces row |
| `use std::min_by` | check-prelude std-cmp-by-helpers comparator-based min root re-export; docs/stdlib/modules/cmp.md |
| `use std::new` | std root re-export tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::newline` | std root re-export tests; docs/dev/test-matrix.md Prelude row |
| `use std::of` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `use std::promote` | std root re-export tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::ptr_add` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::ptr_load` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::ptr_offset` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::ptr_store` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::range` | std root re-export tests; docs/dev/test-matrix.md Prelude and Control flow rows |
| `use std::range_inclusive` | std root re-export tests; docs/dev/test-matrix.md Prelude and Control flow rows |
| `use std::read_byte` | std root re-export tests; docs/dev/test-matrix.md Prelude row |
| `use std::read_line` | std root re-export tests; docs/dev/test-matrix.md Prelude row |
| `use std::read_line_owned` | std root re-export owned read_line tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::replace` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::reset` | std root re-export tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `use std::size_of` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::swap` | std root re-export tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `use std::write_bool` | std root re-export tests; docs/dev/test-matrix.md Prelude row |
| `use std::write_byte` | std root re-export tests; docs/dev/test-matrix.md Prelude row |
| `use std::write_bytes` | std io byte slice root re-export tests; docs/stdlib/modules/io.md |
| `use std::write_i64` | std root re-export tests; docs/dev/test-matrix.md Prelude row |
| `use std::write_u64` | std root re-export tests; docs/dev/test-matrix.md Prelude row |

## `std::algo`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::algo::binary_search[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers binary search over ordered slices; docs/stdlib/modules/algo.md |
| `fn std::algo::binary_search_by[T]` | check-prelude std-algo-by-helpers comparator binary search over sorted slices; docs/stdlib/modules/algo.md |
| `fn std::algo::clamp[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers clamp wrapper over cmp; docs/stdlib/modules/algo.md |
| `fn std::algo::clamp_by[T]` | check-prelude std-algo-by-helpers comparator clamp wrapper over cmp; docs/stdlib/modules/algo.md |
| `fn std::algo::copy[T]` | check-prelude std-algo-slice-helpers prefix copy between slices; docs/stdlib/modules/algo.md |
| `fn std::algo::copy_within[T]` | check-prelude std-vec-range-mutation overlap-safe in-place range copy; docs/stdlib/modules/algo.md |
| `fn std::algo::dedup[T]` | check-prelude std-algo-slice-helpers consecutive duplicate compaction; docs/stdlib/modules/algo.md |
| `fn std::algo::dedup_by[T]` | check-prelude std-algo-dedup-partition comparator consecutive duplicate compaction; docs/stdlib/modules/algo.md |
| `fn std::algo::dedup_by_key[T, K]` | check-prelude std-algo-dedup-partition key-based consecutive duplicate compaction; docs/stdlib/modules/algo.md |
| `fn std::algo::equal_range[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers ordered equal-range bounds; docs/stdlib/modules/algo.md |
| `fn std::algo::equal_range_by[T]` | check-prelude std-algo-by-helpers comparator equal-range bounds; docs/stdlib/modules/algo.md |
| `fn std::algo::fill[T]` | check-prelude std-algo-slice-helpers fill slice values; docs/stdlib/modules/algo.md |
| `fn std::algo::fill_range[T]` | check-prelude std-vec-range-mutation half-open range fill; docs/stdlib/modules/algo.md |
| `fn std::algo::is_sorted[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers sortedness predicate; docs/stdlib/modules/algo.md |
| `fn std::algo::is_sorted_by[T]` | check-prelude std-algo-by-helpers comparator sortedness predicate; docs/stdlib/modules/algo.md |
| `fn std::algo::lower_bound[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers sorted insertion lower bound; docs/stdlib/modules/algo.md |
| `fn std::algo::lower_bound_by[T]` | check-prelude std-algo-by-helpers comparator sorted insertion lower bound; docs/stdlib/modules/algo.md |
| `fn std::algo::max[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers slice maximum helper; docs/stdlib/modules/algo.md |
| `fn std::algo::max_by[T]` | check-prelude std-algo-by-helpers comparator slice maximum helper; docs/stdlib/modules/algo.md |
| `fn std::algo::min[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers slice minimum helper; docs/stdlib/modules/algo.md |
| `fn std::algo::min_by[T]` | check-prelude std-algo-by-helpers comparator slice minimum helper; docs/stdlib/modules/algo.md |
| `fn std::algo::partition[T]` | check-prelude std-algo-slice-helpers borrowed-predicate partition; docs/stdlib/modules/algo.md |
| `fn std::algo::partition_point[T]` | check-prelude std-algo-slice-helpers partition-point binary search over predicate-partitioned slices; docs/stdlib/modules/algo.md |
| `fn std::algo::reverse[T]` | check-prelude std-algo-slice-helpers in-place slice reverse; docs/stdlib/modules/algo.md |
| `fn std::algo::reverse_range[T]` | check-prelude std-vec-range-mutation half-open range reverse; docs/stdlib/modules/algo.md |
| `fn std::algo::rotate_left[T]` | check-prelude std-algo-slice-helpers left rotation; docs/stdlib/modules/algo.md |
| `fn std::algo::rotate_range[T]` | check-prelude std-vec-range-mutation half-open range left rotation; docs/stdlib/modules/algo.md |
| `fn std::algo::rotate_right[T]` | check-prelude std-algo-slice-helpers right rotation; docs/stdlib/modules/algo.md |
| `fn std::algo::sort[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers ordered sort; docs/stdlib/modules/algo.md |
| `fn std::algo::sort_by[T]` | check-prelude std-algo-slice-helpers comparator sort; docs/stdlib/modules/algo.md |
| `fn std::algo::stable_partition[T]` | check-prelude std-algo-dedup-partition stable borrowed-predicate partition; docs/stdlib/modules/algo.md |
| `fn std::algo::stable_sort[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers ordered stable sort; docs/stdlib/modules/algo.md |
| `fn std::algo::stable_sort_by[T]` | check-prelude std-algo-slice-helpers comparator stable sort; docs/stdlib/modules/algo.md |
| `fn std::algo::stable_sort_by_in[T]` | check-prelude std-algo-final-sort comparator stable sort with explicit temporary zone; docs/stdlib/modules/algo.md |
| `fn std::algo::stable_sort_in[T: std::cmp::Ord[T]` | check-prelude std-algo-final-sort ordered stable sort with explicit temporary zone; docs/stdlib/modules/algo.md |
| `fn std::algo::swap[T]` | check-prelude std-algo-slice-helpers element swap; docs/stdlib/modules/algo.md |
| `fn std::algo::try_stable_sort[T: std::cmp::Ord[T]` | check-prelude std-algo-final-sort direct Result stable sort entry; docs/stdlib/modules/algo.md |
| `fn std::algo::try_stable_sort_by[T]` | check-prelude std-algo-final-sort comparator direct Result stable sort entry; docs/stdlib/modules/algo.md |
| `fn std::algo::upper_bound[T: std::cmp::Ord[T]` | check-prelude std-algo-slice-helpers sorted insertion upper bound; docs/stdlib/modules/algo.md |
| `fn std::algo::upper_bound_by[T]` | check-prelude std-algo-by-helpers comparator sorted insertion upper bound; docs/stdlib/modules/algo.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::algo` | check-prelude std-algo-slice-helpers source algorithm module; docs/stdlib/modules/algo.md |

## `std::ascii`

Tier: `core`. Stability reading: stable candidate.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::ascii::contains_ignore_case` | std ascii case search tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::digit_value` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::ends_with_ignore_case` | std ascii case compare tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::equals_ignore_case` | std ascii case compare tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::hex_value` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::index_of_ignore_case` | std ascii case search tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_alpha` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_alphanumeric` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_blank` | std ascii class helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_control` | std ascii class helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_digit` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_graphic` | std ascii class helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_hex_digit` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_lower` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_printable` | std ascii class helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_punctuation` | std ascii class helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_upper` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::is_whitespace` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::parse_decimal` | std ascii slice helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::parse_decimal_prefix` | std ascii prefix parser tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::parse_hex` | std ascii slice helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::parse_hex_prefix` | std ascii prefix parser tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::parse_signed_decimal` | std ascii signed parser tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::parse_signed_decimal_prefix` | std ascii signed parser tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::skip_whitespace` | std ascii slice helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::starts_with_ignore_case` | std ascii case compare tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::to_lower` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::to_upper` | std ascii byte helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::trim` | std ascii slice helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::trim_end` | std ascii slice helper tests; docs/stdlib/api-reference.md ASCII section |
| `fn std::ascii::trim_start` | std ascii slice helper tests; docs/stdlib/api-reference.md ASCII section |

### module

| API | Coverage note |
| --- | --- |
| `module std::ascii` | module load and std ascii byte helper tests; docs/stdlib/overview.md module map |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::ascii::ParsedInt` | std ascii prefix parser tests; docs/stdlib/api-reference.md ASCII section |

## `std::bits`

Tier: `core`. Stability reading: stable candidate.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::bits::align_down` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::align_up` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::any_set` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::bit_width` | std bits width helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::byte_swap` | std bits byte/population tests; docs/stdlib/modules/bits.md |
| `fn std::bits::ceil_power_of_two` | std bits width helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::checked_align_down` | std bits alignment policy tests; docs/stdlib/modules/bits.md |
| `fn std::bits::checked_align_up` | std bits alignment policy tests; docs/stdlib/modules/bits.md |
| `fn std::bits::clear` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::count_ones` | std bits scan helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::count_zeros` | std bits scan helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::floor_power_of_two` | std bits width helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::is_power_of_two` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::is_set` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::leading_ones` | std bits one-run helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::leading_zeros` | std bits scan helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::low_mask` | std bits width helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::population_count` | std bits byte/population tests; docs/stdlib/modules/bits.md |
| `fn std::bits::rotate_left` | std bits rotate helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::rotate_right` | std bits rotate helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::set` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::toggle` | std bits mask helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::trailing_ones` | std bits one-run helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::trailing_zeros` | std bits scan helper tests; docs/stdlib/api-reference.md Bits section |
| `fn std::bits::wrapping_align_up` | std bits alignment policy tests; docs/stdlib/modules/bits.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::bits` | module load and std bits mask helper tests; docs/stdlib/overview.md module map |

## `std::boxed`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::boxed::new[T]` | check-prelude std-boxed-* provenance tests; docs/dev/test-matrix.md Explicit memory zones row |

### method

| API | Coverage note |
| --- | --- |
| `method std::boxed::Box[T]::as_mut` | check-prelude std-boxed-as-ref-mut mutable value borrow with conflict diagnostic; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::as_mut_ptr` | check-prelude std-boxed-as-mut-ptr mutable borrowed receiver with reset invalidation; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::as_ptr` | check-prelude std-boxed-as-ptr borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::as_ref` | check-prelude std-boxed-as-ref-mut shared value borrow with conflict diagnostic; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::clear` | check-prelude std-boxed-clear value-drop empty-handle clear; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::copy_to` | check-prelude std-boxed-copy-to borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::get` | check-prelude std-boxed-box borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::is_empty` | check-prelude std-boxed-take empty handle contract; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::new` | check-prelude prelude-box-root; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::put_in` | check-prelude std-boxed-put-in same-zone empty-handle refill; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::replace` | check-prelude std-boxed-replace; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::set` | check-prelude std-boxed-box and std-boxed-set-drop-value overwrite drop; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::swap` | check-prelude std-boxed-swap; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::take` | check-prelude std-boxed-take value move-out leaves empty handle; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::boxed::Box[T]::try_get` | check-prelude std-boxed-try-get Option-returning empty-handle read; docs/stdlib/modules/boxed.md |
| `method std::boxed::Box[T]::try_take` | check-prelude std-boxed-try-take Option-returning empty-handle move-out; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |

### module

| API | Coverage note |
| --- | --- |
| `module std::boxed` | module load and boxed handle tests; docs/dev/test-matrix.md Modules and Explicit memory zones rows |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::boxed::Box[T]` | std boxed handle tests; docs/dev/test-matrix.md Explicit memory zones row |

## `std::c`

Tier: `platform`. Stability reading: platform-specific.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::c::close` | check-prelude std-c-interop Result-returning dynamic library close wrapper; docs/stdlib/modules/c.md |
| `fn std::c::close_bool` | check-prelude std-c-interop boolean compatibility dynamic library close wrapper; docs/stdlib/modules/c.md |
| `fn std::c::errno` | check-prelude std-c-interop POSIX errno accessor; docs/stdlib/modules/c.md |
| `fn std::c::error` | check-prelude std-c-interop errno-to-Error bridge; docs/stdlib/modules/c.md |
| `fn std::c::from_cstr_in` | check-prelude std-c-interop Result-returning CString copy from borrowed CStr; docs/stdlib/modules/c.md |
| `fn std::c::from_cstr_unchecked_in` | check-prelude std-c-interop assert-on-invalid CString copy from borrowed CStr; docs/stdlib/modules/c.md |
| `fn std::c::from_ptr` | check-prelude std-c-interop null-checked Result C string wrapper; docs/stdlib/modules/c.md |
| `fn std::c::from_ptr_unchecked` | check-prelude std-c-interop assert-on-null borrowed C string wrapper; docs/stdlib/modules/c.md |
| `fn std::c::from_slice_in` | check-prelude std-c-interop Result-returning CString constructor rejecting interior NUL bytes; docs/stdlib/modules/c.md |
| `fn std::c::from_slice_unchecked_in` | check-prelude std-c-interop assert-on-invalid zone-backed NUL-terminated CString constructor; docs/stdlib/modules/c.md |
| `fn std::c::from_string` | check-prelude std-c-interop borrowed Ari string to CStr wrapper; docs/stdlib/modules/c.md |
| `fn std::c::function[T]` | check-prelude std-c-dynamic-function typed dynamic function pointer extraction; docs/stdlib/modules/c.md |
| `fn std::c::global` | check-prelude std-c-interop dynamic loader flag helper; docs/stdlib/modules/c.md |
| `fn std::c::is_null` | check-prelude std-c-interop raw pointer null predicate; docs/stdlib/modules/c.md |
| `fn std::c::last_error` | check-prelude std-c-interop dynamic loader error view; docs/stdlib/modules/c.md |
| `fn std::c::lazy` | check-prelude std-c-interop dynamic loader flag helper; docs/stdlib/modules/c.md |
| `fn std::c::local` | check-prelude std-c-interop dynamic loader flag helper; docs/stdlib/modules/c.md |
| `fn std::c::main_program` | check-prelude std-c-interop Result-returning current process dynamic symbol handle; docs/stdlib/modules/c.md |
| `fn std::c::main_program_raw` | check-prelude std-c-interop raw current process dynamic symbol handle compatibility helper; docs/stdlib/modules/c.md |
| `fn std::c::now` | check-prelude std-c-interop dynamic loader flag helper; docs/stdlib/modules/c.md |
| `fn std::c::open` | check-prelude std-c-interop Result-returning dynamic library open wrapper; docs/stdlib/modules/c.md |
| `fn std::c::open_raw` | check-prelude std-c-interop raw dynamic library open compatibility helper; docs/stdlib/modules/c.md |
| `fn std::c::symbol` | check-prelude std-c-interop Result-returning dynamic symbol lookup wrapper; docs/stdlib/modules/c.md |
| `fn std::c::symbol_raw` | check-prelude std-c-interop raw dynamic symbol lookup compatibility helper; docs/stdlib/modules/c.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::c::CStr::as_ptr` | check-prelude std-c-interop borrowed raw C pointer accessor; docs/stdlib/modules/c.md |
| `method std::c::CStr::as_slice` | check-prelude std-c-interop borrowed bytes without trailing NUL; docs/stdlib/modules/c.md |
| `method std::c::CStr::is_empty` | check-prelude std-c-interop borrowed C string empty predicate; docs/stdlib/modules/c.md |
| `method std::c::CStr::len` | check-prelude std-c-interop borrowed C string byte length; docs/stdlib/modules/c.md |
| `method std::c::CString::as_bytes_with_nul` | check-prelude std-c-interop owned bytes including trailing NUL; docs/stdlib/modules/c.md |
| `method std::c::CString::as_c_str` | check-prelude std-c-interop owned-to-borrowed C string view; docs/stdlib/modules/c.md |
| `method std::c::CString::as_ptr` | check-prelude std-c-interop owned raw C pointer accessor; docs/stdlib/modules/c.md |
| `method std::c::CString::as_slice` | check-prelude std-c-interop owned bytes without trailing NUL; docs/stdlib/modules/c.md |
| `method std::c::CString::is_empty` | check-prelude std-c-interop owned C string empty predicate; docs/stdlib/modules/c.md |
| `method std::c::CString::len` | check-prelude std-c-interop owned C string byte length; docs/stdlib/modules/c.md |
| `method std::c::Library::close` | check-prelude std-c-interop Result-returning dynamic library close method; docs/stdlib/modules/c.md |
| `method std::c::Library::close_bool` | check-prelude std-c-interop boolean compatibility dynamic library close method; docs/stdlib/modules/c.md |
| `method std::c::Library::invalid` | check-prelude std-c-interop invalid dynamic library sentinel; docs/stdlib/modules/c.md |
| `method std::c::Library::is_open` | check-prelude std-c-interop dynamic library validity predicate; docs/stdlib/modules/c.md |
| `method std::c::Library::symbol` | check-prelude std-c-interop Result-returning dynamic symbol lookup method; docs/stdlib/modules/c.md |
| `method std::c::Library::symbol_raw` | check-prelude std-c-interop raw dynamic symbol lookup compatibility method; docs/stdlib/modules/c.md |
| `method std::c::Symbol::as_ptr` | check-prelude std-c-interop dynamic symbol raw pointer accessor; docs/stdlib/modules/c.md |
| `method std::c::Symbol::function[T]` | check-prelude std-c-dynamic-function typed dynamic function pointer extraction; docs/stdlib/modules/c.md |
| `method std::c::Symbol::invalid` | check-prelude std-c-interop invalid dynamic symbol sentinel; docs/stdlib/modules/c.md |
| `method std::c::Symbol::is_valid` | check-prelude std-c-interop dynamic symbol validity predicate; docs/stdlib/modules/c.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::c` | check-prelude std-c-interop C ABI string, errno, and dynamic loading helpers; docs/stdlib/modules/c.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::c::CStr` | check-prelude std-c-interop borrowed C string view; docs/stdlib/modules/c.md |
| `struct std::c::CString` | check-prelude std-c-interop zone-backed NUL-terminated owned C string; docs/stdlib/modules/c.md |
| `struct std::c::Library` | check-prelude std-c-interop dynamic library handle wrapper; docs/stdlib/modules/c.md |
| `struct std::c::Symbol` | check-prelude std-c-interop dynamic symbol handle wrapper; docs/stdlib/modules/c.md |

## `std::cell`

Tier: `alloc`. Stability reading: usable.

### method

| API | Coverage note |
| --- | --- |
| `method std::cell::Cell[T]::get` | check-prelude std-cell-basic Cell copied value read; docs/stdlib/modules/cell.md |
| `method std::cell::Cell[T]::into_inner` | check-prelude std-cell-basic Cell value extraction; docs/stdlib/modules/cell.md |
| `method std::cell::Cell[T]::new` | check-prelude std-cell-basic Cell constructor; docs/stdlib/modules/cell.md |
| `method std::cell::Cell[T]::replace` | check-prelude std-cell-basic Cell replace helper; docs/stdlib/modules/cell.md |
| `method std::cell::Cell[T]::set` | check-prelude std-cell-basic Cell setter; docs/stdlib/modules/cell.md |
| `method std::cell::Cell[T]::take` | check-prelude std-cell-basic Default-backed Cell take helper; docs/stdlib/modules/cell.md |
| `method std::cell::Lazy[T]::force` | check-prelude std-cell-basic Lazy forced initialization; docs/stdlib/modules/cell.md |
| `method std::cell::Lazy[T]::get` | check-prelude std-cell-basic Lazy optional value read; docs/stdlib/modules/cell.md |
| `method std::cell::Lazy[T]::is_initialized` | check-prelude std-cell-basic Lazy initialization predicate; docs/stdlib/modules/cell.md |
| `method std::cell::Lazy[T]::new` | check-prelude std-cell-basic Lazy constructor with explicit zone; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::get` | check-prelude std-cell-basic OnceCell optional shared value pointer; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::get_mut` | check-prelude std-cell-basic OnceCell optional mutable value pointer; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::get_or_init` | check-prelude std-cell-basic OnceCell lazy initialization; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::is_empty` | check-prelude std-cell-basic OnceCell empty predicate; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::is_initialized` | check-prelude std-cell-basic OnceCell initialized predicate; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::new` | check-prelude std-cell-basic OnceCell constructor with explicit zone; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::replace` | check-prelude std-cell-basic OnceCell replace helper; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::set` | check-prelude std-cell-basic OnceCell one-time setter; docs/stdlib/modules/cell.md |
| `method std::cell::OnceCell[T]::take` | check-prelude std-cell-basic OnceCell value extraction; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::borrow` | check-prelude std-cell-basic RefCell checked shared borrow guard; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::borrow_count` | check-prelude std-cell-basic RefCell runtime borrow count; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::borrow_mut` | check-prelude std-cell-basic RefCell checked mutable borrow guard; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::get_mut` | check-prelude std-cell-basic RefCell exclusive direct mutable access; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::into_inner` | check-prelude std-cell-basic RefCell value extraction; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::is_borrowed` | check-prelude std-cell-basic RefCell runtime borrow predicate; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::new` | check-prelude std-cell-basic RefCell constructor; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::replace` | check-prelude std-cell-basic RefCell replace helper; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::take` | check-prelude std-cell-basic Default-backed RefCell take helper; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::try_borrow` | check-prelude std-cell-basic RefCell optional shared borrow guard; docs/stdlib/modules/cell.md |
| `method std::cell::RefCell[T]::try_borrow_mut` | check-prelude std-cell-basic RefCell optional mutable borrow guard; docs/stdlib/modules/cell.md |
| `method std::cell::RefMut[T]::as_mut` | check-prelude std-cell-basic RefMut mutable pointer accessor; docs/stdlib/modules/cell.md |
| `method std::cell::RefMut[T]::as_ref` | check-prelude std-cell-basic RefMut shared pointer accessor; docs/stdlib/modules/cell.md |
| `method std::cell::RefMut[T]::get` | check-prelude std-cell-basic RefMut copied value read; docs/stdlib/modules/cell.md |
| `method std::cell::RefMut[T]::replace` | check-prelude std-cell-basic RefMut replacement helper; docs/stdlib/modules/cell.md |
| `method std::cell::RefMut[T]::set` | check-prelude std-cell-basic RefMut setter; docs/stdlib/modules/cell.md |
| `method std::cell::Ref[T]::as_ref` | check-prelude std-cell-basic Ref shared pointer accessor; docs/stdlib/modules/cell.md |
| `method std::cell::Ref[T]::get` | check-prelude std-cell-basic Ref copied value read; docs/stdlib/modules/cell.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::cell` | check-prelude std-cell-basic interior mutability and one-time initialization helpers; docs/stdlib/modules/cell.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::cell::Cell[T]` | check-prelude std-cell-basic simple interior-mutable value slot; docs/stdlib/modules/cell.md |
| `struct std::cell::Lazy[T]` | check-prelude std-cell-basic zone-backed lazy initializer handle; docs/stdlib/modules/cell.md |
| `struct std::cell::OnceCell[T]` | check-prelude std-cell-basic zone-backed one-time initialized slot; docs/stdlib/modules/cell.md |
| `struct std::cell::RefCell[T]` | check-prelude std-cell-basic runtime borrow-checked value slot; docs/stdlib/modules/cell.md |
| `struct std::cell::RefMut[T]` | check-prelude std-cell-basic mutable RefCell guard; docs/stdlib/modules/cell.md |
| `struct std::cell::Ref[T]` | check-prelude std-cell-basic shared RefCell guard; docs/stdlib/modules/cell.md |

## `std::cmp`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::cmp::Ordering` | check-prelude std-cmp-ordering three-way comparison result; docs/stdlib/modules/cmp.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::cmp::clamp[T: Ord[T]` | std cmp value helper tests; docs/dev/test-matrix.md Front-end surfaces row |
| `fn std::cmp::clamp_by[T]` | check-prelude std-cmp-by-helpers comparator-based clamping; docs/stdlib/modules/cmp.md |
| `fn std::cmp::compare[T: Ord[T]` | check-prelude std-cmp-ordering generic three-way comparison; docs/stdlib/modules/cmp.md |
| `fn std::cmp::compare_by[T]` | check-prelude std-cmp-by-helpers comparator-based three-way comparison; docs/stdlib/modules/cmp.md |
| `fn std::cmp::is_between[T: Ord[T]` | std cmp value helper tests; docs/stdlib/api-reference.md Comparison section |
| `fn std::cmp::is_between_by[T]` | check-prelude std-cmp-by-helpers comparator-based inclusive range predicate; docs/stdlib/modules/cmp.md |
| `fn std::cmp::is_equal` | check-prelude std-cmp-ordering Ordering equality predicate; docs/stdlib/modules/cmp.md |
| `fn std::cmp::is_greater` | check-prelude std-cmp-ordering Ordering greater-than predicate; docs/stdlib/modules/cmp.md |
| `fn std::cmp::is_greater_or_equal` | check-prelude std-cmp-ordering Ordering greater-or-equal predicate; docs/stdlib/modules/cmp.md |
| `fn std::cmp::is_less` | check-prelude std-cmp-ordering Ordering less-than predicate; docs/stdlib/modules/cmp.md |
| `fn std::cmp::is_less_or_equal` | check-prelude std-cmp-ordering Ordering less-or-equal predicate; docs/stdlib/modules/cmp.md |
| `fn std::cmp::max[T: Ord[T]` | std cmp value helper tests; docs/dev/test-matrix.md Front-end surfaces row |
| `fn std::cmp::max_by[T]` | check-prelude std-cmp-by-helpers comparator-based maximum selection; docs/stdlib/modules/cmp.md |
| `fn std::cmp::min[T: Ord[T]` | std cmp value helper tests; docs/dev/test-matrix.md Front-end surfaces row |
| `fn std::cmp::min_by[T]` | check-prelude std-cmp-by-helpers comparator-based minimum selection; docs/stdlib/modules/cmp.md |
| `fn std::cmp::reverse` | check-prelude std-cmp-ordering reverse comparison result helper; docs/stdlib/modules/cmp.md |
| `fn std::cmp::then` | check-prelude std-cmp-ordering lexicographic comparison chaining helper; docs/stdlib/modules/cmp.md |
| `fn std::cmp::then_compare[T: Ord[T]` | check-prelude std-cmp-ordering lazy-ish generic compare chaining helper; docs/stdlib/modules/cmp.md |
| `fn std::cmp::then_compare_by[T]` | check-prelude std-cmp-by-helpers comparator-based Ordering chain helper; docs/stdlib/modules/cmp.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::cmp::Ordering::is_equal` | check-prelude std-cmp-ordering natural Ordering equality method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::is_greater` | check-prelude std-cmp-ordering natural Ordering greater-than method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::is_greater_or_equal` | check-prelude std-cmp-ordering natural Ordering greater-or-equal method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::is_less` | check-prelude std-cmp-ordering natural Ordering less-than method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::is_less_or_equal` | check-prelude std-cmp-ordering natural Ordering less-or-equal method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::reverse` | check-prelude std-cmp-ordering natural Ordering reverse method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::then` | check-prelude std-cmp-ordering natural Ordering chain method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::then_compare[T: Ord[T]` | check-prelude std-cmp-ordering natural generic Ordering chain method; docs/stdlib/modules/cmp.md |
| `method std::cmp::Ordering::then_compare_by[T]` | check-prelude std-cmp-by-helpers natural comparator-based Ordering chain method; docs/stdlib/modules/cmp.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::cmp` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::cmp::Eq[T]` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::cmp::Ord[T]` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::cmp::PartialEq[T]` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::cmp::PartialOrd[T]` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::cmp::Eq[T]::eq` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::cmp::Ord[T]::lt` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::cmp::PartialEq[T]::eq` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::cmp::PartialOrd[T]::lt` | cmp module trait surface; docs/dev/test-matrix.md Front-end surfaces row |

## `std::collections`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::collections::binary_heap[T]` | check-prelude std-collections-heap explicit-comparator binary heap constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::deque[T]` | check-prelude std-collections-deque growable double-ended queue constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::from_slice_in[T]` | check-prelude std-collections-set source linear set from slice; docs/stdlib/modules/collections.md |
| `fn std::collections::hash_i64` | check-prelude std-collections-hash public i64 hash helper for explicit hasher containers; docs/stdlib/modules/collections.md |
| `fn std::collections::hash_map[K, V]` | check-prelude std-collections-hash explicit-hasher hash map constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::hash_set[T]` | check-prelude std-collections-hash explicit-hasher hash set constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::hash_string` | check-prelude std-collections-string-map public String hash helper for explicit hasher containers; docs/stdlib/modules/collections.md |
| `fn std::collections::less_i64` | check-prelude std-collections-tree public i64 ordering helper for explicit comparator containers; docs/stdlib/modules/collections.md |
| `fn std::collections::linked_list[T]` | check-prelude std-collections-linked-list zone-backed linked list constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::new[T]` | check-prelude std-collections-set source linear set constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::priority_queue[T]` | check-prelude std-collections-heap priority queue constructor over binary-heap ordering; docs/stdlib/modules/collections.md |
| `fn std::collections::ring_buffer[T]` | check-prelude std-collections-ring-buffer fixed-capacity ring buffer constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::string_hash_map[V]` | check-prelude std-collections-string-map String-key HashMap constructor using content hashing; docs/stdlib/modules/collections.md |
| `fn std::collections::string_hash_set` | check-prelude std-collections-string-map String HashSet constructor using content hashing; docs/stdlib/modules/collections.md |
| `fn std::collections::tree_map[K, V]` | check-prelude std-collections-tree explicit-comparator tree map constructor; docs/stdlib/modules/collections.md |
| `fn std::collections::tree_set[T]` | check-prelude std-collections-tree explicit-comparator tree set constructor; docs/stdlib/modules/collections.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::collections::BinaryHeapPeekMut[T]::value` | check-prelude std-collections-heap mutable top guard shared read; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeapPeekMut[T]::value_mut` | check-prelude std-collections-heap mutable top guard payload access; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::capacity` | check-prelude std-collections-heap binary heap capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::clear` | check-prelude std-collections-heap drops live heap values; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::copy_to` | check-prelude std-collections-structure-copy-to target-zone binary heap copy preserving priority order; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for binary heap extension; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven binary heap insertion and sift-up; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven binary heap constructor; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::into_sorted_vec` | check-prelude std-collections-heap draining sorted Vec conversion; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::is_empty` | check-prelude std-collections-heap binary heap emptiness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::len` | check-prelude std-collections-heap binary heap length metadata; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::new` | check-prelude std-collections-heap associated explicit-comparator constructor; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::peek` | check-prelude std-collections-heap asserting highest-priority access; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::peek_mut` | check-prelude std-collections-heap mutable top guard with drop-time sift-down; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::pop` | check-prelude std-collections-heap highest-priority removal; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::push` | check-prelude std-collections-heap same-zone heap insertion and sift-up; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::reserve` | check-prelude std-collections-heap same-zone heap storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::reserve_extra` | check-prelude std-collections-heap same-zone heap spare-capacity growth; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::try_peek` | check-prelude std-collections-heap optional highest-priority access; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::try_pop` | check-prelude std-collections-heap optional highest-priority removal; docs/stdlib/modules/collections.md |
| `method std::collections::BinaryHeap[T]::with_capacity` | check-prelude std-collections-heap trait-driven default-order binary heap constructor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::back` | check-prelude std-collections-deque asserting back accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::capacity` | check-prelude std-collections-deque deque capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::clear` | check-prelude std-collections-deque drops live deque values; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::copy_to` | check-prelude std-collections-structure-copy-to target-zone deque copy preserving logical order; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for deque back extension; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven deque back extension; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven deque constructor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::front` | check-prelude std-collections-deque asserting front accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::get` | check-prelude std-collections-deque logical indexed read through circular storage; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::is_empty` | check-prelude std-collections-deque deque emptiness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::iter` | check-prelude std-collections-deque logical-order iterator and IntoIterator; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::len` | check-prelude std-collections-deque deque length metadata; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::new` | check-prelude std-collections-deque associated growable deque constructor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::pop_back` | check-prelude std-collections-deque back removal; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::pop_front` | check-prelude std-collections-deque front removal; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::push_back` | check-prelude std-collections-deque same-zone push at back; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::push_front` | check-prelude std-collections-deque same-zone push at front; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::reserve` | check-prelude std-collections-deque same-zone circular storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::reserve_extra` | check-prelude std-collections-deque same-zone spare-capacity growth; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::try_back` | check-prelude std-collections-deque optional back accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::try_front` | check-prelude std-collections-deque optional front accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::try_get` | check-prelude std-collections-deque optional indexed read; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::try_pop_back` | check-prelude std-collections-deque optional back removal; docs/stdlib/modules/collections.md |
| `method std::collections::Deque[T]::try_pop_front` | check-prelude std-collections-deque optional front removal; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::and_modify` | check-prelude std-collections-map-entry-api hash map occupied-entry mutation hook; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::insert` | check-prelude std-collections-map-entry-accessors hash map entry replacement helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::insert_entry` | check-prelude std-collections-map-entry-defaults hash map insert and return entry helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::key` | check-prelude std-collections-map-entry-accessors hash map entry key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::or_default` | check-prelude std-collections-map-entry-defaults hash map Default-backed entry insertion; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::or_insert` | check-prelude std-collections-map-entry-api hash map eager entry insertion returning ref mut value; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::or_insert_with` | check-prelude std-collections-map-entry-api hash map lazy entry insertion returning ref mut value; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::remove` | check-prelude std-collections-map-entry-accessors hash map entry value removal helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::value` | check-prelude std-collections-map-entry-accessors hash map entry value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapEntry[K,V]::value_mut` | check-prelude std-collections-map-entry-accessors hash map entry mutable value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapValuesMut[V]::has_next` | check-prelude std-collections-view-api mutable hash map value cursor availability predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashMapValuesMut[V]::next` | check-prelude std-collections-view-api mutable hash map value cursor accessor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::capacity` | check-prelude std-collections-hash hash table capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::clear` | check-prelude std-collections-hash drops live hash map entries; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::contains` | check-prelude std-collections-hash hash lookup predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::contains_key` | check-prelude std-collections-map-natural-api natural hash map key-membership predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::contains_value` | check-prelude std-collections-map-value-predicates live-bucket value-membership predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::copy_to` | check-prelude std-collections-copy-to target-zone hash map copy without tombstones; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::drain` | check-prelude std-collections-view-api hash map draining entry cursor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::entries` | check-prelude std-collections-map-entries live-bucket key-value entry iterator; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::entry` | check-prelude std-collections-map-entry-api hash map update entry handle with same-zone growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::extend[I: std::Iterator[std::collections::MapEntry[K, V]` | check-prelude std-collections-polish-api iterator alias for hash map entry extension; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::extend_iter[I: std::Iterator[std::collections::MapEntry[K, V]` | check-prelude std-collections-polish-api iterator-driven hash map entry extension; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::from_iter[I: std::Iterator[std::collections::MapEntry[K, V]` | check-prelude std-collections-polish-api iterator-driven hash map constructor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::get` | check-prelude std-collections-hash asserting hash lookup; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::get_mut` | check-prelude std-collections-map-mut-access asserting mutable hash lookup; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::get_or` | check-prelude std-collections-map-natural-api fallback hash lookup helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::insert` | check-prelude std-collections-hash insert-or-replace with same-zone growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::is_empty` | check-prelude std-collections-hash metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::iter` | check-prelude std-collections-view-api live-bucket entry iterator alias; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::iter_mut` | check-prelude std-collections-view-api mutable live-bucket entry cursor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::keys` | check-prelude std-collections-hash-iter bucket-order key iterator; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::len` | check-prelude std-collections-hash metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::new` | check-prelude std-collections-hash associated explicit-hasher constructor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::remove` | check-prelude std-collections-hash tombstone removal; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::remove_entry` | check-prelude std-collections-map-entry-api hash map key-value tombstone removal; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::replace` | check-prelude std-collections-map-mut-access named hash map insert-or-replace helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::reserve` | check-prelude std-collections-hash same-zone table growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::reserve_extra` | check-prelude std-collections-implicit-zone spare table growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::retain` | check-prelude std-collections-retain hash map in-place predicate filtering; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::try_get` | check-prelude std-collections-hash Option-returning hash lookup; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::try_get_mut` | check-prelude std-collections-map-mut-access optional hash map mutable value handle lookup; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::values` | check-prelude std-collections-hash-iter bucket-order value iterator; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::values_mut` | check-prelude std-collections-view-api mutable live-bucket value cursor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::with_capacity` | check-prelude std-collections-map-natural-api trait-driven default-hash hash map constructor; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[K,V]::with_hash` | check-prelude std-collections-hash-with-hash explicit custom-hasher constructor and HashMap::new migration path; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::contains_key_bytes` | check-prelude std-collections-string-map borrowed byte-slice lookup for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::get_bytes` | check-prelude std-collections-string-map asserting borrowed byte-slice lookup for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::get_mut_bytes` | check-prelude std-collections-string-map asserting borrowed mutable byte-slice lookup for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::get_or_bytes` | check-prelude std-collections-string-map fallback borrowed byte-slice lookup for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::remove_bytes` | check-prelude std-collections-string-map borrowed byte-slice removal for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::remove_entry_bytes` | check-prelude std-collections-string-map borrowed byte-slice key-value removal for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::try_get_bytes` | check-prelude std-collections-string-map optional borrowed byte-slice lookup for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashMap[std::string::String,V]::try_get_mut_bytes` | check-prelude std-collections-string-map optional borrowed mutable byte-slice lookup for String-key hash maps; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::capacity` | check-prelude std-collections-hash hash set capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::clear` | check-prelude std-collections-hash drops live hash set entries; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::contains` | check-prelude std-collections-hash hash set lookup predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::copy_to` | check-prelude std-collections-copy-to target-zone hash set copy without tombstones; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::drain` | check-prelude std-collections-view-api hash set draining value cursor; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::equals` | check-prelude std-collections-hash-set-relations membership equality predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for hash set insertion; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven hash set insertion; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven hash set constructor; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::get` | check-prelude std-collections-set-representatives asserting stored representative lookup; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::insert` | check-prelude std-collections-hash hash set insertion with same-zone growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::is_disjoint` | check-prelude std-collections-hash-set-relations live-bucket disjointness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::is_empty` | check-prelude std-collections-hash metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::is_subset` | check-prelude std-collections-hash-set-relations live-bucket subset predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::is_superset` | check-prelude std-collections-hash-set-relations live-bucket superset predicate; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::iter` | check-prelude std-collections-hash-iter live-bucket iterator and IntoIterator; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::len` | check-prelude std-collections-hash metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::new` | check-prelude std-collections-hash associated explicit-hasher constructor; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::remove` | check-prelude std-collections-hash drop-on-remove membership helper; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::replace` | check-prelude std-collections-hash replace-or-insert behavior; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::reserve` | check-prelude std-collections-hash same-zone table growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::reserve_extra` | check-prelude std-collections-implicit-zone spare table growth; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::retain` | check-prelude std-collections-retain hash set in-place predicate filtering; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::take` | check-prelude std-collections-hash Option-returning removal; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::try_get` | check-prelude std-collections-set-representatives optional stored representative lookup; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::with_capacity` | check-prelude std-collections-map-natural-api trait-driven default-hash hash set constructor; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[T]::with_hash` | check-prelude std-collections-hash-with-hash explicit custom-hasher constructor and HashSet::new migration path; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[std::string::String]::contains_bytes` | check-prelude std-collections-string-map borrowed byte-slice lookup for String hash sets; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[std::string::String]::get_bytes` | check-prelude std-collections-string-map asserting borrowed byte-slice representative lookup for String hash sets; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[std::string::String]::remove_bytes` | check-prelude std-collections-string-map borrowed byte-slice removal for String hash sets; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[std::string::String]::take_bytes` | check-prelude std-collections-string-map optional borrowed byte-slice representative removal for String hash sets; docs/stdlib/modules/collections.md |
| `method std::collections::HashSet[std::string::String]::try_get_bytes` | check-prelude std-collections-string-map optional borrowed byte-slice representative lookup for String hash sets; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::back` | check-prelude std-collections-linked-list asserting back accessor; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::capacity` | check-prelude std-collections-linked-list linked list capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::clear` | check-prelude std-collections-linked-list drops live linked list values; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::copy_to` | check-prelude std-collections-structure-copy-to target-zone linked-list copy preserving logical order; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for linked-list back extension; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven linked-list back extension; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven linked-list constructor; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::front` | check-prelude std-collections-linked-list asserting front accessor; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::get` | check-prelude std-collections-linked-list indexed traversal read; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::is_empty` | check-prelude std-collections-linked-list linked list emptiness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::iter` | check-prelude std-collections-linked-list forward iterator and IntoIterator; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::len` | check-prelude std-collections-linked-list linked list length metadata; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::new` | check-prelude std-collections-linked-list associated linked list constructor; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::pop_back` | check-prelude std-collections-linked-list back removal and node reuse path; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::pop_front` | check-prelude std-collections-linked-list front removal and node reuse path; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::push_back` | check-prelude std-collections-linked-list same-zone push at back; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::push_front` | check-prelude std-collections-linked-list same-zone push at front; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::remove_at` | check-prelude std-collections-linked-list indexed removal; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::reserve` | check-prelude std-collections-linked-list same-zone node storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::reserve_extra` | check-prelude std-collections-linked-list same-zone spare-node growth; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::try_back` | check-prelude std-collections-linked-list optional back accessor; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::try_front` | check-prelude std-collections-linked-list optional front accessor; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::try_get` | check-prelude std-collections-linked-list optional indexed traversal read; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::try_pop_back` | check-prelude std-collections-linked-list optional back removal; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::try_pop_front` | check-prelude std-collections-linked-list optional front removal; docs/stdlib/modules/collections.md |
| `method std::collections::LinkedList[T]::try_remove_at` | check-prelude std-collections-linked-list optional indexed removal; docs/stdlib/modules/collections.md |
| `method std::collections::MapEntryMut[K,V]::key` | check-prelude std-collections-view-api mutable map entry key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::MapEntryMut[K,V]::value` | check-prelude std-collections-view-api mutable map entry value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::MapEntryMut[K,V]::value_mut` | check-prelude std-collections-view-api mutable map entry value borrow; docs/stdlib/modules/collections.md |
| `method std::collections::MapEntry[K,V]::key` | check-prelude std-collections-map-entry-accessors copied map entry key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::MapEntry[K,V]::value` | check-prelude std-collections-map-entry-accessors copied map entry value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::MapValueMut[V]::value` | check-prelude std-collections-map-mut-access optional mutable map value read helper; docs/stdlib/modules/collections.md |
| `method std::collections::MapValueMut[V]::value_mut` | check-prelude std-collections-map-mut-access optional mutable map value borrow helper; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::capacity` | check-prelude std-collections-heap priority queue capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::clear` | check-prelude std-collections-heap drops live priority queue values; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::copy_to` | check-prelude std-collections-structure-copy-to target-zone priority queue copy preserving priority order; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for priority queue insertion; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven priority queue insertion; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven priority queue constructor; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::is_empty` | check-prelude std-collections-heap priority queue emptiness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::len` | check-prelude std-collections-heap priority queue length metadata; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::new` | check-prelude std-collections-heap associated priority queue constructor; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::peek` | check-prelude std-collections-heap highest-priority access; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::pop` | check-prelude std-collections-heap highest-priority removal; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::push` | check-prelude std-collections-heap same-zone priority queue insertion; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::reserve` | check-prelude std-collections-heap same-zone priority queue storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::reserve_extra` | check-prelude std-collections-heap same-zone priority queue spare-capacity growth; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::try_peek` | check-prelude std-collections-heap optional highest-priority access; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::try_pop` | check-prelude std-collections-heap optional highest-priority removal; docs/stdlib/modules/collections.md |
| `method std::collections::PriorityQueue[T]::with_capacity` | check-prelude std-collections-heap trait-driven default-order priority queue constructor; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::capacity` | check-prelude std-collections-ring-buffer ring buffer fixed capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::clear` | check-prelude std-collections-ring-buffer drops live ring buffer values; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::copy_to` | check-prelude std-collections-structure-copy-to target-zone ring buffer copy preserving fixed capacity and FIFO order; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::get` | check-prelude std-collections-ring-buffer logical indexed read after wraparound; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::is_empty` | check-prelude std-collections-ring-buffer emptiness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::is_full` | check-prelude std-collections-ring-buffer full-capacity predicate; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::iter` | check-prelude std-collections-ring-buffer FIFO iterator and IntoIterator; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::len` | check-prelude std-collections-ring-buffer ring buffer length metadata; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::new` | check-prelude std-collections-ring-buffer associated fixed-capacity constructor; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::peek` | check-prelude std-collections-ring-buffer oldest-value access; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::pop` | check-prelude std-collections-ring-buffer FIFO removal; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::push` | check-prelude std-collections-ring-buffer bounded push failure path; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::push_overwrite` | check-prelude std-collections-ring-buffer overwrite-oldest behavior; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::try_get` | check-prelude std-collections-ring-buffer optional indexed read; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::try_peek` | check-prelude std-collections-ring-buffer optional oldest-value access; docs/stdlib/modules/collections.md |
| `method std::collections::RingBuffer[T]::try_pop` | check-prelude std-collections-ring-buffer optional FIFO removal; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::as_slice` | check-prelude std-collections-set borrowed set storage view; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::capacity` | check-prelude std-collections-set capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::clear` | check-prelude std-collections-set value drop clear; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::contains` | check-prelude std-collections-set linear membership lookup; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::copy_to` | check-prelude std-collections-set target-zone copy; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::drain` | check-prelude std-collections-view-api insertion-order draining value cursor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::equals` | check-prelude std-collections-set-relations membership equality predicate; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for linear set insertion; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven linear set insertion; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::first` | check-prelude std-collections-set-access insertion-order first accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven linear set constructor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::get` | check-prelude std-collections-set-access insertion-order indexed accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::index_of` | check-prelude std-collections-set stable insertion-order index lookup; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::insert` | check-prelude std-collections-set same-zone insertion and duplicate rejection; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::is_disjoint` | check-prelude std-collections-set-relations membership disjointness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::is_empty` | check-prelude std-collections-set metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::is_subset` | check-prelude std-collections-set-relations membership subset predicate; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::is_superset` | check-prelude std-collections-set-relations membership superset predicate; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::iter` | check-prelude std-collections-set-iter standard iterator cursor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::last` | check-prelude std-collections-set-access insertion-order last accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::len` | check-prelude std-collections-set metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::new` | check-prelude std-collections-set associated constructor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::pop` | check-prelude std-collections-set-access last-value removal; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::remove` | check-prelude std-collections-set membership removal; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::replace` | check-prelude std-collections-set-replace replace-or-insert behavior and same-zone diagnostic; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::reserve` | check-prelude std-collections-set-access same-zone capacity growth; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::reserve_extra` | check-prelude std-collections-set-access same-zone spare-capacity growth; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::retain` | check-prelude std-collections-retain linear set in-place predicate filtering; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::take` | check-prelude std-collections-set Option-returning removal; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::try_first` | check-prelude std-collections-set-access optional first accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::try_get` | check-prelude std-collections-set-access optional indexed accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::try_last` | check-prelude std-collections-set-access optional last accessor; docs/stdlib/modules/collections.md |
| `method std::collections::Set[T]::try_pop` | check-prelude std-collections-set-access optional last-value removal; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::and_modify` | check-prelude std-collections-map-entry-api tree map occupied-entry mutation hook; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::insert` | check-prelude std-collections-map-entry-accessors tree map entry replacement helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::insert_entry` | check-prelude std-collections-map-entry-defaults tree map insert and return entry helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::key` | check-prelude std-collections-map-entry-accessors tree map entry key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::or_default` | check-prelude std-collections-map-entry-defaults tree map Default-backed entry insertion; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::or_insert` | check-prelude std-collections-map-entry-api tree map eager entry insertion returning ref mut value; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::or_insert_with` | check-prelude std-collections-map-entry-api tree map lazy entry insertion returning ref mut value; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::remove` | check-prelude std-collections-map-entry-accessors tree map entry value removal helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::value` | check-prelude std-collections-map-entry-accessors tree map entry value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapEntry[K,V]::value_mut` | check-prelude std-collections-map-entry-accessors tree map entry mutable value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapValuesMut[V]::has_next` | check-prelude std-collections-view-api mutable tree map value cursor availability predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMapValuesMut[V]::next` | check-prelude std-collections-view-api mutable tree map value cursor accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::append` | check-prelude std-collections-polish-api same-zone ordered map drain append; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::capacity` | check-prelude std-collections-tree tree map capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::clear` | check-prelude std-collections-tree drops live tree map nodes; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::contains` | check-prelude std-collections-tree ordered lookup predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::contains_key` | check-prelude std-collections-map-natural-api natural ordered map key-membership predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::contains_value` | check-prelude std-collections-map-value-predicates tree map value-membership predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::copy_to` | check-prelude std-collections-copy-to target-zone ordered map copy; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::drain` | check-prelude std-collections-view-api sorted draining entry cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::entries` | check-prelude std-collections-map-entries sorted key-value entry iterator; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::entry` | check-prelude std-collections-map-entry-api tree map update entry handle with same-zone growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::extend[I: std::Iterator[std::collections::MapEntry[K, V]` | check-prelude std-collections-polish-api iterator alias for ordered map entry extension; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::extend_iter[I: std::Iterator[std::collections::MapEntry[K, V]` | check-prelude std-collections-polish-api iterator-driven ordered map entry extension; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::first_entry` | check-prelude std-collections-tree-entry-boundaries asserting smallest key-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::first_key` | check-prelude std-collections-tree-boundaries asserting smallest-key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::first_value` | check-prelude std-collections-tree-boundaries asserting value for smallest key; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::from_iter[I: std::Iterator[std::collections::MapEntry[K, V]` | check-prelude std-collections-polish-api iterator-driven ordered map constructor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::get` | check-prelude std-collections-tree asserting ordered lookup; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::get_mut` | check-prelude std-collections-map-mut-access asserting mutable ordered lookup; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::get_or` | check-prelude std-collections-map-natural-api fallback ordered lookup helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::insert` | check-prelude std-collections-tree red-black insert-or-replace with same-zone growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::is_empty` | check-prelude std-collections-tree metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::iter` | check-prelude std-collections-view-api sorted entry iterator alias; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::iter_mut` | check-prelude std-collections-view-api mutable sorted entry cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::keys` | check-prelude std-collections-tree-iter sorted key iterator; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::last_entry` | check-prelude std-collections-tree-entry-boundaries asserting largest key-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::last_key` | check-prelude std-collections-tree-boundaries asserting largest-key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::last_value` | check-prelude std-collections-tree-boundaries asserting value for largest key; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::len` | check-prelude std-collections-tree metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::lower_bound` | check-prelude std-collections-tree-bounds optional first entry not less than key; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::new` | check-prelude std-collections-tree associated explicit-comparator constructor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::pop_first` | check-prelude std-collections-polish-api optional smallest entry removal; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::pop_last` | check-prelude std-collections-polish-api optional largest entry removal; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::range` | check-prelude std-collections-tree-iter half-open sorted map range cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::range_mut` | check-prelude std-collections-tree-iter half-open mutable sorted map range cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::remove` | check-prelude std-collections-tree-remove direct red-black deletion returning removed value; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::remove_entry` | check-prelude std-collections-map-entry-api tree map key-value removal after direct red-black deletion; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::replace` | check-prelude std-collections-map-mut-access named ordered map insert-or-replace helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::reserve` | check-prelude std-collections-tree same-zone tree storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::reserve_extra` | check-prelude std-collections-implicit-zone spare tree storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::split_off` | check-prelude std-collections-polish-api same-zone ordered map split at lower bound; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_first_entry` | check-prelude std-collections-tree-entry-boundaries optional smallest key-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_first_key` | check-prelude std-collections-tree-boundaries optional smallest-key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_first_value` | check-prelude std-collections-tree-boundaries optional value for smallest key; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_get` | check-prelude std-collections-tree Option-returning ordered lookup; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_get_mut` | check-prelude std-collections-map-mut-access optional ordered map mutable value handle lookup; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_last_entry` | check-prelude std-collections-tree-entry-boundaries optional largest key-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_last_key` | check-prelude std-collections-tree-boundaries optional largest-key accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::try_last_value` | check-prelude std-collections-tree-boundaries optional value for largest key; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::upper_bound` | check-prelude std-collections-tree-bounds optional first entry greater than key; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::values` | check-prelude std-collections-tree-iter sorted-by-key value iterator; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::values_mut` | check-prelude std-collections-view-api mutable sorted-by-key value cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeMap[K,V]::with_capacity` | check-prelude std-collections-map-natural-api trait-driven default-order tree map constructor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::append` | check-prelude std-collections-polish-api same-zone ordered set drain append; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::capacity` | check-prelude std-collections-tree tree set capacity metadata; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::clear` | check-prelude std-collections-tree drops live tree set nodes; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::contains` | check-prelude std-collections-tree ordered set lookup predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::copy_to` | check-prelude std-collections-copy-to target-zone ordered set copy; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::drain` | check-prelude std-collections-view-api sorted draining value cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::equals` | check-prelude std-collections-tree-set-relations membership equality predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::extend[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator alias for ordered set insertion; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven ordered set insertion; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::first` | check-prelude std-collections-tree-boundaries asserting smallest-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::from_iter[I: std::Iterator[T]` | check-prelude std-collections-polish-api iterator-driven ordered set constructor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::get` | check-prelude std-collections-set-representatives asserting ordered representative lookup; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::insert` | check-prelude std-collections-tree red-black set insertion with same-zone growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::is_disjoint` | check-prelude std-collections-tree-set-relations ordered-set disjointness predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::is_empty` | check-prelude std-collections-tree metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::is_subset` | check-prelude std-collections-tree-set-relations ordered-set subset predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::is_superset` | check-prelude std-collections-tree-set-relations ordered-set superset predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::iter` | check-prelude std-collections-tree-iter sorted iterator and IntoIterator; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::last` | check-prelude std-collections-tree-boundaries asserting largest-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::len` | check-prelude std-collections-tree metadata helper; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::lower_bound` | check-prelude std-collections-tree-bounds optional first value not less than target; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::new` | check-prelude std-collections-tree associated explicit-comparator constructor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::range` | check-prelude std-collections-tree-iter half-open sorted set range cursor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::remove` | check-prelude std-collections-tree-remove direct red-black deletion predicate; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::replace` | check-prelude std-collections-tree replace-or-insert behavior; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::reserve` | check-prelude std-collections-tree same-zone tree storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::reserve_extra` | check-prelude std-collections-implicit-zone spare tree storage growth; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::split_off` | check-prelude std-collections-polish-api same-zone ordered set split at lower bound; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::take` | check-prelude std-collections-tree-remove Option-returning direct red-black deletion; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::try_first` | check-prelude std-collections-tree-boundaries optional smallest-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::try_get` | check-prelude std-collections-set-representatives optional ordered representative lookup; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::try_last` | check-prelude std-collections-tree-boundaries optional largest-value accessor; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::upper_bound` | check-prelude std-collections-tree-bounds optional first value greater than target; docs/stdlib/modules/collections.md |
| `method std::collections::TreeSet[T]::with_capacity` | check-prelude std-collections-map-natural-api trait-driven default-order tree set constructor; docs/stdlib/modules/collections.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::collections` | std collections linear set tests; docs/stdlib/modules/collections.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::collections::BinaryHeapPeekMut[T]` | std collections binary heap mutable top guard tests; docs/stdlib/modules/collections.md |
| `struct std::collections::BinaryHeap[T]` | std collections binary heap tests; docs/stdlib/modules/collections.md |
| `struct std::collections::DequeIter[T]` | std collections deque iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::Deque[T]` | std collections deque tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapDrain[K, V]` | std collections map drain tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapEntriesMut[K, V]` | std collections mutable hash map entry cursor tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapEntries[K, V]` | std collections map entry iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapEntry[K, V]` | std collections map entry update tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapKeys[K]` | std collections hash iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapValuesMut[V]` | std collections mutable hash value cursor tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMapValues[V]` | std collections hash iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashMap[K, V]` | std collections hash map tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashSetDrain[T]` | std collections hash set drain tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashSetIter[T]` | std collections hash iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::HashSet[T]` | std collections hash set tests; docs/stdlib/modules/collections.md |
| `struct std::collections::Iter[T]` | std collections iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::LinkedListIter[T]` | std collections linked list iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::LinkedList[T]` | std collections linked list tests; docs/stdlib/modules/collections.md |
| `struct std::collections::MapEntryMut[K, V]` | std collections mutable map entry tests; docs/stdlib/modules/collections.md |
| `struct std::collections::MapEntry[K, V]` | std collections map entry iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::MapValueMut[V]` | std collections mutable map value handle tests; docs/stdlib/modules/collections.md |
| `struct std::collections::PriorityQueue[T]` | std collections priority queue tests; docs/stdlib/modules/collections.md |
| `struct std::collections::RingBufferIter[T]` | std collections ring buffer iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::RingBuffer[T]` | std collections ring buffer tests; docs/stdlib/modules/collections.md |
| `struct std::collections::SetDrain[T]` | std collections linear set drain tests; docs/stdlib/modules/collections.md |
| `struct std::collections::Set[T]` | std collections linear set tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapDrain[K, V]` | std collections tree map drain tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapEntriesMut[K, V]` | std collections mutable tree map entry cursor tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapEntries[K, V]` | std collections map entry iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapEntry[K, V]` | std collections map entry update tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapKeys[K]` | std collections tree iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapRangeMut[K, V]` | std collections mutable tree map range tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapRange[K, V]` | std collections tree map range tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapValuesMut[V]` | std collections mutable tree value cursor tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMapValues[V]` | std collections tree iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeMap[K, V]` | std collections tree map tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeSetDrain[T]` | std collections tree set drain tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeSetIter[T]` | std collections tree iterator tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeSetRange[T]` | std collections tree set range tests; docs/stdlib/modules/collections.md |
| `struct std::collections::TreeSet[T]` | std collections tree set tests; docs/stdlib/modules/collections.md |

## `std::context`

Tier: `hosted`. Stability reading: platform-backed.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::context::arg` | check-prelude context tests; docs/dev/test-matrix.md Context runtime row |
| `fn std::context::argc` | check-prelude context tests; docs/dev/test-matrix.md Context runtime row |
| `fn std::context::cwd` | check-prelude std-context-args startup current-directory snapshot hook; docs/stdlib/modules/context.md |
| `fn std::context::cwd_os` | check-prelude std-context-args startup current-directory OS-string view; docs/stdlib/modules/context.md |
| `fn std::context::cwd_path` | check-prelude std-context-args startup current-directory path-byte view; docs/stdlib/modules/context.md |
| `fn std::context::executable_path` | check-prelude std-context-args startup executable-path snapshot hook; docs/stdlib/modules/context.md |
| `fn std::context::executable_path_os` | check-prelude std-context-args startup executable-path OS-string view; docs/stdlib/modules/context.md |
| `fn std::context::has_arg` | std context args tests; docs/stdlib/modules/context.md |
| `fn std::context::has_args` | check-prelude std-context-args source context predicate; docs/stdlib/modules/context.md |
| `fn std::context::has_cwd` | check-prelude std-context-args source startup cwd predicate; docs/stdlib/modules/context.md |
| `fn std::context::has_executable_path` | check-prelude std-context-args source startup executable-path predicate; docs/stdlib/modules/context.md |
| `fn std::context::has_user_args` | check-prelude std-context-args source user argument predicate; docs/stdlib/modules/context.md |
| `fn std::context::is_main_thread` | check-prelude std-context-args source thread predicate; docs/stdlib/modules/context.md |
| `fn std::context::thread_id` | check-prelude std-context-args context thread id hook; docs/stdlib/modules/context.md |
| `fn std::context::try_cwd` | check-prelude std-context-args optional startup cwd helper; docs/stdlib/modules/context.md |
| `fn std::context::try_cwd_os` | check-prelude std-context-args optional startup cwd OS-string helper; docs/stdlib/modules/context.md |
| `fn std::context::try_executable_path` | check-prelude std-context-args optional startup executable-path helper; docs/stdlib/modules/context.md |
| `fn std::context::try_executable_path_os` | check-prelude std-context-args optional startup executable-path OS-string helper; docs/stdlib/modules/context.md |
| `fn std::context::user_arg_count` | check-prelude std-context-args source user argument count; docs/stdlib/modules/context.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::context` | context runtime tests; docs/dev/test-matrix.md Context runtime row |

## `std::convert`

Tier: `core`. Stability reading: stable candidate.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::convert::from[T, U: From[T]` | std convert value helper tests; docs/stdlib/api-reference.md Conversion section |
| `fn std::convert::identity[T]` | std convert value helper tests; docs/stdlib/api-reference.md Conversion section |
| `fn std::convert::into[T, U: Into[T]` | std convert value helper tests; docs/stdlib/api-reference.md Conversion section |
| `fn std::convert::try_from[T, U: TryFrom[T]` | std convert try helper tests; docs/stdlib/api-reference.md Conversion section |
| `fn std::convert::try_into[T, U: TryInto[T]` | std convert try helper tests; docs/stdlib/api-reference.md Conversion section |

### module

| API | Coverage note |
| --- | --- |
| `module std::convert` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::convert::From[T]` | convert module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::convert::Into[T]` | convert module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::convert::TryFrom[T]` | convert module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::convert::TryInto[T]` | convert module trait surface; docs/dev/test-matrix.md Front-end surfaces row |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::convert::From[T]::from` | convert module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::convert::Into[T]::into` | convert module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::convert::TryFrom[T]::try_from` | convert try helper tests; docs/dev/test-matrix.md Front-end surfaces row |
| `trait-method std::convert::TryInto[T]::try_into` | convert try helper tests; docs/dev/test-matrix.md Front-end surfaces row |

## `std::encoding`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::encoding::CodecErrorKind` | check-prelude std-encoding-codec structured hex/base64 diagnostic categories; docs/stdlib/modules/encoding.md |
| `enum std::encoding::Utf8ErrorKind` | check-prelude std-encoding-text detailed UTF-8 validation failure categories; docs/stdlib/modules/encoding.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::encoding::base64_decoded_len` | check-prelude std-encoding-codec Result-returning standard base64 decoded length validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_decoded_len_optional` | check-prelude std-encoding-codec Option-returning standard base64 decoded length compatibility validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_encoded_len` | check-prelude std-encoding-codec standard base64 encoded length helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_error` | check-prelude std-encoding-codec structured standard base64 diagnostic helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_mime_decoded_len` | check-prelude std-encoding-codec Result-returning MIME base64 decoded length validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_mime_decoded_len_optional` | check-prelude std-encoding-codec Option-returning MIME base64 decoded length compatibility validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_mime_encoded_len` | check-prelude std-encoding-codec line-wrapped MIME base64 encoded length helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_mime_error` | check-prelude std-encoding-codec structured MIME base64 diagnostic helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_url_decoded_len` | check-prelude std-encoding-codec Result-returning URL-safe base64 decoded length validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_url_decoded_len_optional` | check-prelude std-encoding-codec Option-returning URL-safe base64 decoded length compatibility validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_url_encoded_len` | check-prelude std-encoding-codec padded URL-safe base64 encoded length helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_url_error` | check-prelude std-encoding-codec structured URL-safe base64 diagnostic helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::base64_url_unpadded_encoded_len` | check-prelude std-encoding-codec unpadded URL-safe base64 encoded length helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::can_decode_base64` | check-prelude std-encoding-codec standard base64 validation guard; docs/stdlib/modules/encoding.md |
| `fn std::encoding::can_decode_base64_mime` | check-prelude std-encoding-codec MIME base64 validation guard; docs/stdlib/modules/encoding.md |
| `fn std::encoding::can_decode_base64_url` | check-prelude std-encoding-codec URL-safe base64 validation guard; docs/stdlib/modules/encoding.md |
| `fn std::encoding::can_decode_hex` | check-prelude std-encoding-codec hexadecimal validation guard; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64` | check-prelude std-encoding-codec Result-returning standard base64 zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_in` | check-prelude std-encoding-codec Result-returning standard base64 zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_mime` | check-prelude std-encoding-codec Result-returning MIME base64 zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_mime_in` | check-prelude std-encoding-codec Result-returning MIME base64 zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_mime_optional_in` | check-prelude std-encoding-codec Option-returning MIME base64 zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_mime_unchecked_in` | check-prelude std-encoding-codec asserting MIME base64 zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_optional_in` | check-prelude std-encoding-codec Option-returning standard base64 zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_unchecked_in` | check-prelude std-encoding-codec asserting standard base64 zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_url` | check-prelude std-encoding-codec Result-returning URL-safe base64 zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_url_in` | check-prelude std-encoding-codec Result-returning URL-safe base64 zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_url_optional_in` | check-prelude std-encoding-codec Option-returning URL-safe base64 zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_base64_url_unchecked_in` | check-prelude std-encoding-codec asserting URL-safe base64 zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_hex` | check-prelude std-encoding-codec Result-returning hexadecimal zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_hex_in` | check-prelude std-encoding-codec Result-returning hexadecimal zone-backed decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_hex_optional_in` | check-prelude std-encoding-codec Option-returning hexadecimal zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_hex_unchecked_in` | check-prelude std-encoding-codec asserting hexadecimal zone-backed compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_utf8` | check-prelude std-encoding-text Result-returning UTF-8 byte-string decoder with detailed Utf8Error; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_utf8_in` | check-prelude std-encoding-text zone-backed compatibility alias for decode_utf8; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_utf8_optional_in` | check-prelude std-encoding-text Option-returning UTF-8 byte-string compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::decode_utf8_unchecked_in` | check-prelude std-encoding-text asserting UTF-8 byte-string compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_base64_in` | check-prelude std-encoding-codec standard base64 zone-backed encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_base64_mime_in` | check-prelude std-encoding-codec line-wrapped MIME base64 zone-backed encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_base64_url_in` | check-prelude std-encoding-codec padded URL-safe base64 zone-backed encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_base64_url_unpadded_in` | check-prelude std-encoding-codec unpadded URL-safe base64 zone-backed encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_hex_in` | check-prelude std-encoding-codec lowercase hexadecimal zone-backed encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_utf8` | check-prelude std-encoding-utf8-codepoints Result-returning UTF-8 scalar encoder with detailed Utf8Error; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_utf8_in` | check-prelude std-encoding-utf8-codepoints zone-backed compatibility alias for encode_utf8; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_utf8_optional_in` | check-prelude std-encoding-utf8-codepoints Option-returning UTF-8 scalar compatibility encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::encode_utf8_unchecked_in` | check-prelude std-encoding-utf8-codepoints asserting UTF-8 scalar compatibility encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::hex_decoded_len` | check-prelude std-encoding-codec Result-returning hexadecimal decoded length validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::hex_decoded_len_optional` | check-prelude std-encoding-codec Option-returning hexadecimal decoded length compatibility validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::hex_encoded_len` | check-prelude std-encoding-codec hexadecimal encoded length helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::hex_error` | check-prelude std-encoding-codec structured hexadecimal diagnostic helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::is_ascii` | check-prelude std-encoding-text ASCII byte-slice validation; docs/stdlib/modules/encoding.md |
| `fn std::encoding::is_unicode_scalar` | check-prelude std-encoding-utf8-codepoints Unicode scalar value predicate; docs/stdlib/modules/encoding.md |
| `fn std::encoding::is_utf16` | check-prelude std-encoding-text UTF-16 boolean validation; docs/stdlib/modules/encoding.md |
| `fn std::encoding::is_utf8` | check-prelude std-encoding-text UTF-8 boolean validation; docs/stdlib/modules/encoding.md |
| `fn std::encoding::try_decode_base64_in` | check-prelude std-encoding-codec Option-returning standard base64 compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::try_decode_base64_mime_in` | check-prelude std-encoding-codec Option-returning MIME base64 compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::try_decode_base64_url_in` | check-prelude std-encoding-codec Option-returning URL-safe base64 compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::try_decode_hex_in` | check-prelude std-encoding-codec Option-returning hexadecimal compatibility decoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::try_encode_utf8_in` | check-prelude std-encoding-utf8-codepoints Option-returning UTF-8 scalar compatibility encoder; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf16_count` | check-prelude std-encoding-text Result-returning UTF-16 code-point count validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf16_count_optional` | check-prelude std-encoding-text Option-returning UTF-16 code-point count compatibility validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_at` | check-prelude std-encoding-utf8-codepoints UTF-8 scalar decode at byte offset; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_count` | check-prelude std-encoding-text Result-returning UTF-8 code-point count validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_count_optional` | check-prelude std-encoding-text Option-returning UTF-8 code-point count compatibility validator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_encoded_len` | check-prelude std-encoding-utf8-codepoints UTF-8 scalar encoded length helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_error` | check-prelude std-encoding-text detailed UTF-8 validation failure locator; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_next_index` | check-prelude std-encoding-utf8-codepoints UTF-8 next byte offset helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::utf8_width` | check-prelude std-encoding-utf8-codepoints UTF-8 lead-byte width helper; docs/stdlib/modules/encoding.md |
| `fn std::encoding::validate_utf8` | check-prelude std-encoding-text Result-returning UTF-8 validation helper with detailed Utf8Error; docs/stdlib/modules/encoding.md |
| `fn std::encoding::validate_utf8_optional` | check-prelude std-encoding-text Option-returning detailed UTF-8 validation compatibility helper; docs/stdlib/modules/encoding.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::encoding::CodecError::byte` | check-prelude std-encoding-codec failing codec byte accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::index` | check-prelude std-encoding-codec failing codec byte index accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::is_invalid_byte` | check-prelude std-encoding-codec invalid codec alphabet byte predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::is_invalid_length` | check-prelude std-encoding-codec invalid codec length predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::is_invalid_padding` | check-prelude std-encoding-codec invalid codec padding predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::kind` | check-prelude std-encoding-codec codec failure kind accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::message` | check-prelude std-encoding-codec stable codec diagnostic message; docs/stdlib/modules/encoding.md |
| `method std::encoding::CodecError::name` | check-prelude std-encoding-codec stable codec diagnostic name; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Char::len` | check-prelude std-encoding-utf8-codepoints decoded UTF-8 byte length accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Char::next_index` | check-prelude std-encoding-utf8-codepoints decoded UTF-8 next byte offset accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Char::scalar` | check-prelude std-encoding-utf8-codepoints decoded Unicode scalar accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::byte` | check-prelude std-encoding-text failing UTF-8 byte accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::index` | check-prelude std-encoding-text failing UTF-8 byte index accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::is_invalid_continuation` | check-prelude std-encoding-text UTF-8 invalid-continuation predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::is_invalid_lead` | check-prelude std-encoding-text UTF-8 invalid-lead predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::is_out_of_range` | check-prelude std-encoding-text UTF-8 out-of-range scalar predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::is_overlong` | check-prelude std-encoding-text UTF-8 overlong-encoding predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::is_surrogate` | check-prelude std-encoding-text UTF-8 surrogate scalar predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::is_unexpected_end` | check-prelude std-encoding-text UTF-8 truncated-sequence predicate; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::kind` | check-prelude std-encoding-text UTF-8 failure kind accessor; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::message` | check-prelude std-encoding-text stable UTF-8 diagnostic message; docs/stdlib/modules/encoding.md |
| `method std::encoding::Utf8Error::name` | check-prelude std-encoding-text stable UTF-8 diagnostic name; docs/stdlib/modules/encoding.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::encoding` | check-prelude std-encoding-text/std-encoding-codec validation and codec helpers; docs/stdlib/modules/encoding.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::encoding::CodecError` | check-prelude std-encoding-codec detailed hex/base64 validation failure value; docs/stdlib/modules/encoding.md |
| `struct std::encoding::Utf8Char` | check-prelude std-encoding-utf8-codepoints decoded UTF-8 scalar value; docs/stdlib/modules/encoding.md |
| `struct std::encoding::Utf8Error` | check-prelude std-encoding-text detailed UTF-8 validation failure value; docs/stdlib/modules/encoding.md |

## `std::env`

Tier: `hosted`. Stability reading: platform-backed.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::env::arg` | check-prelude std-env-args Result-returning argument helper; docs/stdlib/modules/env.md |
| `fn std::env::arg_count` | check-prelude std-env-args argument count wrapper; docs/stdlib/modules/env.md |
| `fn std::env::arg_optional` | check-prelude std-env-args Option-returning argument compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::arg_os` | check-prelude std-env-os-path-views Result-returning OS-string argument view; docs/stdlib/modules/env.md |
| `fn std::env::arg_os_optional` | check-prelude std-env-os-path-views Option-returning OS-string argument compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::arg_os_unchecked` | check-prelude std-env-os-path-views unchecked OS-string argument compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::arg_unchecked` | check-prelude std-env-args unchecked argument compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::args` | check-prelude std-env-args full owned argument vector helper; docs/stdlib/modules/env.md |
| `fn std::env::args_os` | check-prelude std-env-args full OS-string argument vector helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir` | check-prelude std-env-paths Result-returning current directory helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_optional` | check-prelude std-env-paths Option-returning current directory compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_or_default` | check-prelude std-env-paths empty-string current directory compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_os` | check-prelude std-env-os-path-views Result-returning OS-string current directory view; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_os_optional` | check-prelude std-env-os-path-views Option-returning OS-string current directory compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_os_or_default` | check-prelude std-env-os-path-views OS-string current directory compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_path` | check-prelude std-env-os-path-views Result-returning path-byte current directory view; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_path_optional` | check-prelude std-env-os-path-views Option-returning path-byte current directory compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_path_or_default` | check-prelude std-env-os-path-views path-byte current directory compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::current_dir_raw` | check-prelude std-env-paths raw runtime current directory hook; docs/stdlib/modules/env.md |
| `fn std::env::executable_path` | check-prelude std-env-paths Result-returning executable path helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_optional` | check-prelude std-env-paths Option-returning executable path compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_or_default` | check-prelude std-env-paths empty-string executable path compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_os` | check-prelude std-env-os-path-views Result-returning OS-string executable path view; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_os_optional` | check-prelude std-env-os-path-views Option-returning OS-string executable path compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_os_or_default` | check-prelude std-env-os-path-views OS-string executable path compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_path` | check-prelude std-env-paths Result-returning path-byte executable path view; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_path_optional` | check-prelude std-env-paths Option-returning path-byte executable path compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_path_or_default` | check-prelude std-env-paths path-byte executable path compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::executable_path_raw` | check-prelude std-env-paths raw runtime executable path hook; docs/stdlib/modules/env.md |
| `fn std::env::get` | check-prelude std-env-vars Result-returning environment variable lookup; docs/stdlib/modules/env.md |
| `fn std::env::get_or_default` | check-prelude std-env-vars empty-string compatibility environment variable lookup; docs/stdlib/modules/env.md |
| `fn std::env::get_os` | check-prelude std-env-vars/std-env-os-path-views Result-returning OS-string environment variable lookup; docs/stdlib/modules/env.md |
| `fn std::env::get_os_or_default` | check-prelude std-env-os-path-views OS-string compatibility environment variable lookup; docs/stdlib/modules/env.md |
| `fn std::env::has` | check-prelude std-env-vars environment variable presence; docs/stdlib/modules/env.md |
| `fn std::env::has_arg` | check-prelude std-env-args argument range helper; docs/stdlib/modules/env.md |
| `fn std::env::home_dir` | check-prelude std-env-paths HOME-backed optional path-byte helper; docs/stdlib/modules/env.md |
| `fn std::env::program_name` | check-prelude std-env-args Result-returning program name helper; docs/stdlib/modules/env.md |
| `fn std::env::program_name_optional` | check-prelude std-env-args Option-returning program name compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::program_name_os` | check-prelude std-env-os-path-views Result-returning OS-string program name helper; docs/stdlib/modules/env.md |
| `fn std::env::program_name_os_optional` | check-prelude std-env-os-path-views Option-returning OS-string program name compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::remove` | check-prelude std-env-vars Result-returning environment variable removal; docs/stdlib/modules/env.md |
| `fn std::env::remove_unchecked` | check-prelude std-env-vars bool compatibility environment variable removal; docs/stdlib/modules/env.md |
| `fn std::env::remove_var` | check-prelude std-env-vars Result-returning environment variable removal alias; docs/stdlib/modules/env.md |
| `fn std::env::set` | check-prelude std-env-vars Result-returning environment variable mutation; docs/stdlib/modules/env.md |
| `fn std::env::set_current_dir` | check-prelude std-env-paths Result-returning current directory mutation; docs/stdlib/modules/env.md |
| `fn std::env::set_current_dir_raw` | check-prelude std-env-paths raw runtime current directory mutation hook; docs/stdlib/modules/env.md |
| `fn std::env::set_current_dir_unchecked` | check-prelude std-env-paths bool current directory mutation compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::set_unchecked` | check-prelude std-env-vars bool compatibility environment variable mutation; docs/stdlib/modules/env.md |
| `fn std::env::set_var` | check-prelude std-env-vars Result-returning environment variable mutation alias; docs/stdlib/modules/env.md |
| `fn std::env::try_arg` | check-prelude std-env-args optional argument helper; docs/stdlib/modules/env.md |
| `fn std::env::try_arg_os` | check-prelude std-env-os-path-views optional OS-string argument helper; docs/stdlib/modules/env.md |
| `fn std::env::try_current_dir` | check-prelude std-env-paths optional current directory helper; docs/stdlib/modules/env.md |
| `fn std::env::try_current_dir_os` | check-prelude std-env-os-path-views optional OS-string current directory helper; docs/stdlib/modules/env.md |
| `fn std::env::try_current_dir_path` | check-prelude std-env-os-path-views optional path-byte current directory helper; docs/stdlib/modules/env.md |
| `fn std::env::try_executable_path` | check-prelude std-env-paths optional executable path helper; docs/stdlib/modules/env.md |
| `fn std::env::try_executable_path_os` | check-prelude std-env-os-path-views optional OS-string executable path helper; docs/stdlib/modules/env.md |
| `fn std::env::try_executable_path_path` | check-prelude std-env-paths optional path-byte executable path helper; docs/stdlib/modules/env.md |
| `fn std::env::try_get` | check-prelude std-env-vars optional environment variable helper; docs/stdlib/modules/env.md |
| `fn std::env::try_get_os` | check-prelude std-env-os-path-views optional OS-string environment variable helper; docs/stdlib/modules/env.md |
| `fn std::env::var` | check-prelude std-env-vars Option-returning environment variable lookup; docs/stdlib/modules/env.md |
| `fn std::env::var_optional` | check-prelude std-env-vars Option-returning environment variable compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::var_or_default` | check-prelude std-env-vars empty-string environment variable compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::var_os` | check-prelude std-env-vars Option-returning OS-string environment variable lookup; docs/stdlib/modules/env.md |
| `fn std::env::var_os_optional` | check-prelude std-env-vars Option-returning OS-string environment variable compatibility helper; docs/stdlib/modules/env.md |
| `fn std::env::var_os_or_default` | check-prelude std-env-vars OS-string environment variable compatibility helper; docs/stdlib/modules/env.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::env` | std env argument helper tests; docs/stdlib/modules/env.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::env::Error` | check-prelude std-env-vars shared environment error alias; docs/stdlib/modules/env.md |
| `type std::env::ErrorKind` | check-prelude std-env-vars shared environment error-kind alias; docs/stdlib/modules/env.md |

## `std::error`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::error::Kind` | check-prelude std-error-basic shared recoverable error categories; docs/stdlib/modules/error.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::error::code` | check-prelude std-error-basic error platform code extractor; docs/stdlib/modules/error.md |
| `fn std::error::from_errno` | check-prelude std-error-basic POSIX errno mapping helper; docs/stdlib/modules/error.md |
| `fn std::error::from_os_code` | check-prelude std-error-integration target OS error-code mapping alias; docs/stdlib/modules/error.md |
| `fn std::error::from_raw` | check-prelude std-error-basic compact Result-compatible error reconstruction; docs/stdlib/modules/error.md |
| `fn std::error::is_connection_refused` | check-prelude std-error-integration connection-refused predicate helper; docs/stdlib/modules/error.md |
| `fn std::error::is_interrupted` | check-prelude std-error-basic interrupted predicate helper; docs/stdlib/modules/error.md |
| `fn std::error::is_kind` | check-prelude std-error-basic error-kind predicate helper; docs/stdlib/modules/error.md |
| `fn std::error::is_not_found` | check-prelude std-error-basic not-found predicate helper; docs/stdlib/modules/error.md |
| `fn std::error::is_retryable` | check-prelude std-error-basic retryable predicate helper; docs/stdlib/modules/error.md |
| `fn std::error::kind` | check-prelude std-error-basic error kind extractor; docs/stdlib/modules/error.md |
| `fn std::error::map_errno[T]` | check-prelude std-error-integration Result[T, errno] to Result[T, Error] bridge; docs/stdlib/modules/error.md |
| `fn std::error::map_os_code[T]` | check-prelude std-error-integration Result[T, OS code] to Result[T, Error] bridge; docs/stdlib/modules/error.md |
| `fn std::error::map_raw[T]` | check-prelude std-error-basic Result[T, i64] to Result[T, Error] compatibility bridge; docs/stdlib/modules/error.md |
| `fn std::error::message` | check-prelude std-error-basic stable generic message helper; docs/stdlib/modules/error.md |
| `fn std::error::name` | check-prelude std-error-basic error-kind name helper; docs/stdlib/modules/error.md |
| `fn std::error::new` | check-prelude std-error-basic error constructor without platform code; docs/stdlib/modules/error.md |
| `fn std::error::raw` | check-prelude std-error-basic compact raw error representation helper; docs/stdlib/modules/error.md |
| `fn std::error::to_raw[T]` | check-prelude std-error-basic Result[T, Error] to Result[T, i64] compatibility bridge; docs/stdlib/modules/error.md |
| `fn std::error::try_from_errno` | check-prelude std-error-validation Option-returning POSIX errno mapping helper; docs/stdlib/modules/error.md |
| `fn std::error::try_from_os_code` | check-prelude std-error-integration Option-returning target OS error-code mapping alias; docs/stdlib/modules/error.md |
| `fn std::error::try_from_raw` | check-prelude std-error-validation Option-returning raw error reconstruction; docs/stdlib/modules/error.md |
| `fn std::error::try_with_code` | check-prelude std-error-validation Option-returning error constructor with platform code; docs/stdlib/modules/error.md |
| `fn std::error::with_code` | check-prelude std-error-basic error constructor with platform code; docs/stdlib/modules/error.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::error::Error::code` | check-prelude std-error-basic error platform code method; docs/stdlib/modules/error.md |
| `method std::error::Error::is_connection_refused` | check-prelude std-error-integration connection-refused predicate method; docs/stdlib/modules/error.md |
| `method std::error::Error::is_interrupted` | check-prelude std-error-basic interrupted predicate method; docs/stdlib/modules/error.md |
| `method std::error::Error::is_kind` | check-prelude std-error-basic error-kind predicate method; docs/stdlib/modules/error.md |
| `method std::error::Error::is_not_found` | check-prelude std-error-basic not-found predicate method; docs/stdlib/modules/error.md |
| `method std::error::Error::is_retryable` | check-prelude std-error-basic retryable predicate method; docs/stdlib/modules/error.md |
| `method std::error::Error::kind` | check-prelude std-error-basic error kind method; docs/stdlib/modules/error.md |
| `method std::error::Error::message` | check-prelude std-error-basic stable generic message method; docs/stdlib/modules/error.md |
| `method std::error::Error::name` | check-prelude std-error-basic error-kind name method; docs/stdlib/modules/error.md |
| `method std::error::Error::raw` | check-prelude std-error-basic compact raw error method; docs/stdlib/modules/error.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::error` | check-prelude std-error-basic shared recoverable error module; docs/stdlib/modules/error.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::error::Error` | check-prelude std-error-basic compact shared error value; docs/stdlib/modules/error.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::error::ErrorKind` | check-prelude std-error-integration module-local alias for std::error::Kind; docs/stdlib/modules/error.md |

## `std::fmt`

Tier: `core`. Stability reading: stable candidate.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::fmt::alternate` | check-prelude std-fmt-format-spec alternate integer prefix spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::binary` | check-prelude std-fmt-format-spec binary integer spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::boolean_in` | check-prelude std-fmt-format-spec allocator-backed bool formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::center` | check-prelude std-fmt-format-spec center alignment spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::char_in` | check-prelude std-fmt-char-values allocator-backed byte character display helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::concat2[A: std::fmt::Display, B: std::fmt::Display]` | check-prelude std-fmt-concat-format-value two-value Display concatenation helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::concat3[A: std::fmt::Display, B: std::fmt::Display, C: std::fmt::Display]` | check-prelude std-fmt-concat-format-value three-value Display concatenation helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::debug_char_in` | check-prelude std-fmt-char-values allocator-backed byte character debug helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::debug_text_in` | check-prelude std-fmt-format-spec source debug text quoting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::debug_value[T: std::fmt::Debug]` | check-prelude std-fmt-debug-values generic allocator-backed Debug formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::decimal` | check-prelude std-fmt-format-spec decimal integer spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::float_in` | check-prelude std-fmt-display-builtins allocator-backed float formatting helper and Display impls; docs/stdlib/modules/fmt.md |
| `fn std::fmt::format2[A: std::fmt::Display, B: std::fmt::Display]` | check-prelude std-fmt-concat-format-value Result-returning two-value runtime template formatter; docs/stdlib/modules/fmt.md |
| `fn std::fmt::format3[A: std::fmt::Display, B: std::fmt::Display, C: std::fmt::Display]` | check-prelude std-fmt-concat-format-value Result-returning three-value runtime template formatter; docs/stdlib/modules/fmt.md |
| `fn std::fmt::format[T: std::fmt::Display]` | check-prelude std-fmt-concat-format-value Result-returning one-value runtime template formatter; docs/stdlib/modules/fmt.md |
| `fn std::fmt::format_value[T: std::fmt::Display]` | check-prelude std-fmt-concat-format-value generic allocator-backed Display formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::hex` | check-prelude std-fmt-format-spec hexadecimal integer spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::integer_in` | check-prelude std-fmt-format-spec allocator-backed signed integer formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::left` | check-prelude std-fmt-format-spec left alignment spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::octal` | check-prelude std-fmt-format-spec octal integer spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::print_debug[T: std::fmt::Debug]` | check-prelude std-fmt-debug-values generic stdout Debug formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::print_value[T: std::fmt::Display]` | check-prelude std-fmt-print-value generic stdout Display formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::println_debug[T: std::fmt::Debug]` | check-prelude std-fmt-debug-values generic stdout Debug line formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::println_value[T: std::fmt::Display]` | check-prelude std-fmt-print-value generic stdout Display line formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::right` | check-prelude std-fmt-format-spec right alignment spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::text_in` | check-prelude std-fmt-format-spec allocator-backed text copy helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::try_with_precision` | check-prelude std-fmt-format-validation fallible integer precision spec helper for runtime input; docs/stdlib/modules/fmt.md |
| `fn std::fmt::try_with_width` | check-prelude std-fmt-format-validation fallible width spec helper for runtime input; docs/stdlib/modules/fmt.md |
| `fn std::fmt::unsigned_in` | check-prelude std-fmt-format-spec allocator-backed base/width/precision formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::uppercase` | check-prelude std-fmt-format-spec uppercase digit spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::with_precision` | check-prelude std-fmt-format-spec integer precision spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::with_width` | check-prelude std-fmt-format-spec width spec helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_boolean[W: std::io::Writer]` | check-prelude std-fmt-format-spec Result-returning Writer-backed bool formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_boolean_bool[W: std::io::Writer]` | check-prelude std-fmt-format-spec bool compatibility Writer-backed bool formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_concat2` | check-prelude std-fmt-format-spec Result-returning two-value Writer-backed Display concatenation helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_concat2_bool` | check-prelude std-fmt-format-spec bool compatibility two-value Writer-backed Display concatenation helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_concat3` | check-prelude std-fmt-format-spec Result-returning three-value Writer-backed Display concatenation helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_concat3_bool` | check-prelude std-fmt-format-spec bool compatibility three-value Writer-backed Display concatenation helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_debug[W: std::io::Writer, T: std::fmt::Debug]` | check-prelude std-fmt-debug-values Result-returning generic Writer-backed Debug formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_debug_bool[W: std::io::Writer, T: std::fmt::Debug]` | check-prelude std-fmt-debug-values bool compatibility generic Writer-backed Debug formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_format` | check-prelude std-fmt-concat-format-value Result-returning one-value Writer-backed runtime template formatter; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_format2` | check-prelude std-fmt-concat-format-value Result-returning two-value Writer-backed runtime template formatter; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_format3` | check-prelude std-fmt-concat-format-value Result-returning three-value Writer-backed runtime template formatter; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_integer[W: std::io::Writer]` | check-prelude std-fmt-format-spec Result-returning Writer-backed signed integer formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_integer_bool[W: std::io::Writer]` | check-prelude std-fmt-format-spec bool compatibility Writer-backed signed integer formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_line_debug[W: std::io::Writer, T: std::fmt::Debug]` | check-prelude std-fmt-format-spec Result-returning generic Writer-backed Debug line helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_line_debug_bool[W: std::io::Writer, T: std::fmt::Debug]` | check-prelude std-fmt-format-spec bool compatibility generic Writer-backed Debug line helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_line_text[W: std::io::Writer]` | check-prelude std-fmt-format-spec Result-returning Writer-backed text line helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_line_text_bool[W: std::io::Writer]` | check-prelude std-fmt-format-spec bool compatibility Writer-backed text line helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_line_value[W: std::io::Writer, T: std::fmt::Display]` | check-prelude std-fmt-format-spec Result-returning generic Writer-backed Display line helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_line_value_bool[W: std::io::Writer, T: std::fmt::Display]` | check-prelude std-fmt-format-spec bool compatibility generic Writer-backed Display line helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_text[W: std::io::Writer]` | check-prelude std-fmt-format-spec Result-returning Writer-backed text formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_text_bool[W: std::io::Writer]` | check-prelude std-fmt-format-spec bool compatibility Writer-backed text formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_unsigned[W: std::io::Writer]` | check-prelude std-fmt-format-spec Result-returning Writer-backed base/width/precision formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_unsigned_bool[W: std::io::Writer]` | check-prelude std-fmt-format-spec bool compatibility Writer-backed base/width/precision formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_value[W: std::io::Writer, T: std::fmt::Display]` | check-prelude std-fmt-format-spec Result-returning generic Writer-backed Display formatting helper; docs/stdlib/modules/fmt.md |
| `fn std::fmt::write_value_bool[W: std::io::Writer, T: std::fmt::Display]` | check-prelude std-fmt-format-spec bool compatibility generic Writer-backed Display formatting helper; docs/stdlib/modules/fmt.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::fmt` | prelude trait surface tests; docs/dev/test-matrix.md Front-end surfaces row |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::fmt::FormatSpec` | check-prelude std-fmt-format-spec source formatting options; docs/stdlib/modules/fmt.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::fmt::Debug` | fmt module trait surface; docs/dev/test-matrix.md Front-end surfaces row |
| `trait std::fmt::Display` | fmt module trait surface; docs/dev/test-matrix.md Front-end surfaces row |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::fmt::Debug::debug_in` | check-prelude std-fmt-debug-values Debug borrowed receiver tests; docs/stdlib/modules/fmt.md |
| `trait-method std::fmt::Display::format_in` | format_in Display borrowed receiver tests; docs/dev/test-matrix.md Prelude row |

## `std::fs`

Tier: `hosted`. Stability reading: platform-backed.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::fs::FileKind` | check-prelude std-fs-metadata file/directory/symlink/other metadata classifier; docs/stdlib/modules/fs.md |
| `enum std::fs::Operation` | check-prelude std-fs-detailed-errors structured filesystem operation classifier; docs/stdlib/modules/fs.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::fs::append` | check-prelude std-fs-read-write Result-returning whole-file append helper; docs/stdlib/modules/fs.md |
| `fn std::fs::append_bool` | check-prelude std-fs-read-write bool whole-file append compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::append_detailed` | check-prelude std-fs-detailed-errors path-tagged append error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::append_raw` | check-prelude std-fs-byte-result raw Result-returning whole-file append byte-count compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::can_execute` | check-prelude std-fs-permissions execute/search access hook; docs/stdlib/modules/fs.md |
| `fn std::fs::can_read` | check-prelude std-fs-permissions read access hook; docs/stdlib/modules/fs.md |
| `fn std::fs::can_write` | check-prelude std-fs-permissions write access hook; docs/stdlib/modules/fs.md |
| `fn std::fs::canonicalize` | check-prelude std-fs-query-result Result-returning realpath-backed canonicalization helper; docs/stdlib/modules/fs.md |
| `fn std::fs::canonicalize_detailed` | check-prelude std-fs-detailed-errors path-tagged canonicalization error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::canonicalize_optional` | check-prelude std-fs-query-result Option-returning realpath-backed canonicalization compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::canonicalize_unchecked` | check-prelude std-fs-query-result unchecked realpath-backed canonicalization compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::close` | check-prelude std-fs-basic Result-returning file close helper; docs/stdlib/modules/fs.md |
| `fn std::fs::close_dir` | check-prelude std-fs-read-dir Result-returning directory close helper; docs/stdlib/modules/fs.md |
| `fn std::fs::close_dir_raw` | check-prelude std-fs-read-dir raw Result-returning directory close compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::close_dir_unchecked` | check-prelude std-fs-read-dir unchecked directory close compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::close_raw` | check-prelude std-fs-basic raw Result-returning file close compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::close_unchecked` | check-prelude std-fs-basic unchecked file close compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::copy` | check-prelude std-fs-byte-result Result-returning streaming copy byte-count helper; docs/stdlib/modules/fs.md |
| `fn std::fs::copy_bool` | check-prelude std-fs-create-truncate-copy bool streaming copy compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::copy_detailed` | check-prelude std-fs-detailed-errors source/target-tagged copy error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::copy_raw` | check-prelude std-fs-byte-result raw Result-returning streaming copy byte-count compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create` | check-prelude std-fs-open-result Result-returning create/truncate open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_detailed` | check-prelude std-fs-detailed-errors path-tagged create error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir` | check-prelude std-fs-mutation-result Result-returning single-directory creation helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_all` | check-prelude std-fs-create-dir-all Result-returning recursive directory creation helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_all_bool` | check-prelude std-fs-create-dir-all bool recursive directory creation compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_all_detailed` | check-prelude std-fs-detailed-errors path-tagged recursive directory creation error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_all_raw` | check-prelude std-fs-create-dir-all raw Result-returning recursive directory creation compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_all_unchecked` | check-prelude std-fs-create-dir-all unchecked recursive directory creation compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_bool` | check-prelude std-fs-rename-dir bool single-directory creation compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_detailed` | check-prelude std-fs-detailed-errors path-tagged directory creation error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_raw` | check-prelude std-fs-mutation-result raw Result-returning directory creation compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_dir_unchecked` | check-prelude std-fs-rename-dir unchecked single-directory creation compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::create_optional` | check-prelude std-fs-open-result Option-returning create compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_raw` | check-prelude std-fs-open-result raw Result-returning create compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::create_unchecked` | check-prelude std-fs-open-result unchecked create compatibility helper returning invalid handles; docs/stdlib/modules/fs.md |
| `fn std::fs::ensure_dir` | check-prelude std-fs-ensure-dir idempotent single-directory creation helper; docs/stdlib/modules/fs.md |
| `fn std::fs::ensure_dir_all` | check-prelude std-fs-create-dir-all idempotent recursive directory creation alias; docs/stdlib/modules/fs.md |
| `fn std::fs::ensure_file` | check-prelude std-fs-ensure-file idempotent single-file creation helper; docs/stdlib/modules/fs.md |
| `fn std::fs::exists` | check-prelude std-fs-basic file existence hook; docs/stdlib/modules/fs.md |
| `fn std::fs::file_type` | check-prelude std-fs-query-result Result-returning path kind helper; docs/stdlib/modules/fs.md |
| `fn std::fs::file_type_detailed` | check-prelude std-fs-detailed-errors path-tagged file-kind error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::file_type_optional` | check-prelude std-fs-query-result Option-returning path kind compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::file_type_raw` | check-prelude std-fs-query-result raw Result-returning path kind compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::file_type_unchecked` | check-prelude std-fs-query-result unchecked path kind compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::hard_link` | check-prelude std-fs-links Result-returning hard-link helper; docs/stdlib/modules/fs.md |
| `fn std::fs::hard_link_bool` | check-prelude std-fs-links bool hard-link compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::hard_link_raw` | check-prelude std-fs-links raw Result-returning hard-link compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::hard_link_unchecked` | check-prelude std-fs-links unchecked hard-link compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::is_dir` | check-prelude std-fs-metadata direct directory-kind predicate; docs/stdlib/modules/fs.md |
| `fn std::fs::is_file` | check-prelude std-fs-metadata direct regular-file predicate; docs/stdlib/modules/fs.md |
| `fn std::fs::is_other` | check-prelude std-fs-metadata direct other-kind predicate; docs/stdlib/modules/fs.md |
| `fn std::fs::is_symlink` | check-prelude std-fs-symlink-metadata no-follow direct symlink-kind predicate; docs/stdlib/modules/fs.md |
| `fn std::fs::lock_exclusive` | check-prelude std-fs-lock Result-returning hosted advisory exclusive file lock helper; docs/stdlib/modules/fs.md |
| `fn std::fs::lock_exclusive_raw` | check-prelude std-fs-lock raw Result-returning hosted advisory exclusive file lock helper; docs/stdlib/modules/fs.md |
| `fn std::fs::lock_shared` | check-prelude std-fs-lock Result-returning hosted advisory shared file lock helper; docs/stdlib/modules/fs.md |
| `fn std::fs::lock_shared_raw` | check-prelude std-fs-lock raw Result-returning hosted advisory shared file lock helper; docs/stdlib/modules/fs.md |
| `fn std::fs::metadata` | check-prelude std-fs-query-result Result-returning target-following metadata helper; docs/stdlib/modules/fs.md |
| `fn std::fs::metadata_detailed` | check-prelude std-fs-detailed-errors path-tagged metadata error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::metadata_optional` | check-prelude std-fs-query-result Option-returning target-following metadata compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::metadata_raw` | check-prelude std-fs-query-result raw Result-returning target-following metadata compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::metadata_unchecked` | check-prelude std-fs-query-result unchecked target-following metadata compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::mode` | check-prelude std-fs-query-result Result-returning POSIX permission-mode helper; docs/stdlib/modules/fs.md |
| `fn std::fs::mode_detailed` | check-prelude std-fs-detailed-errors path-tagged mode query error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::mode_optional` | check-prelude std-fs-query-result Option-returning POSIX permission-mode compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::mode_raw` | check-prelude std-fs-query-result raw Result-returning permission-mode compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::mode_unchecked` | check-prelude std-fs-query-result unchecked POSIX permission-mode compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open` | check-prelude std-fs-open-result Result-returning mode-string open helper with Error payload; docs/stdlib/modules/fs.md |
| `fn std::fs::open_append` | check-prelude std-fs-open-result Result-returning append-mode open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_append_optional` | check-prelude std-fs-open-result Option-returning append-mode open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_append_unchecked` | check-prelude std-fs-open-result unchecked append-mode open compatibility helper returning invalid handles; docs/stdlib/modules/fs.md |
| `fn std::fs::open_detailed` | check-prelude std-fs-detailed-errors path-tagged mode-string open error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_dir` | check-prelude std-fs-read-dir Result-returning directory open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_dir_raw` | check-prelude std-fs-query-result raw Result-returning directory open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_dir_unchecked` | check-prelude std-fs-read-dir unchecked directory open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_optional` | check-prelude std-fs-open-result Option-returning mode-string open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_options` | check-prelude std-fs-open-options OpenOptions default constructor helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_raw` | check-prelude std-fs-open-result raw Result-returning mode-string open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_read` | check-prelude std-fs-open-result Result-returning read-mode open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_read_optional` | check-prelude std-fs-open-result Option-returning read-mode open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_read_unchecked` | check-prelude std-fs-open-result unchecked read-mode open compatibility helper returning invalid handles; docs/stdlib/modules/fs.md |
| `fn std::fs::open_unchecked` | check-prelude std-fs-open-result unchecked mode-string open compatibility hook returning invalid handles; docs/stdlib/modules/fs.md |
| `fn std::fs::open_write` | check-prelude std-fs-open-result Result-returning write-mode open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_write_optional` | check-prelude std-fs-open-result Option-returning write-mode open compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::open_write_unchecked` | check-prelude std-fs-open-result unchecked write-mode open compatibility helper returning invalid handles; docs/stdlib/modules/fs.md |
| `fn std::fs::permissions` | check-prelude std-fs-permissions access permission wrapper; docs/stdlib/modules/fs.md |
| `fn std::fs::position` | check-prelude std-fs-seek Result-returning file cursor position helper; docs/stdlib/modules/fs.md |
| `fn std::fs::position_or` | check-prelude std-fs-seek fallback file position compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::position_raw` | check-prelude std-fs-seek raw Result-returning file position compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read` | check-prelude std-fs-read-result Result-returning natural read alias for read_to_string; docs/stdlib/modules/fs.md |
| `fn std::fs::read_byte` | check-prelude std-fs-basic file byte read hook; docs/stdlib/modules/fs.md |
| `fn std::fs::read_bytes` | check-prelude std-fs-byte-result Result-returning whole-file byte-vector helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_bytes_detailed` | check-prelude std-fs-detailed-errors path-tagged whole-file byte-vector read error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_detailed` | check-prelude std-fs-detailed-errors path-tagged whole-file string read error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir` | check-prelude std-fs-query-result Result-returning directory entry-list helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_detailed` | check-prelude std-fs-detailed-errors path-tagged directory read error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_entries` | check-prelude std-fs-query-result Result-returning directory entry-list helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_entries_optional` | check-prelude std-fs-query-result Option-returning directory entry-list compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_entries_unchecked` | check-prelude std-fs-query-result unchecked directory entry-list compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_info` | check-prelude std-fs-dir-entry-info directory entries with per-entry metadata Result snapshots; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_info_detailed` | check-prelude std-fs-dir-entry-info path-tagged directory info read error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_info_optional` | check-prelude std-fs-dir-entry-info Option-returning directory info compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_info_unchecked` | check-prelude std-fs-dir-entry-info unchecked directory info compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_names` | check-prelude std-fs-query-result Result-returning directory name-list helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_names_optional` | check-prelude std-fs-query-result Option-returning directory name-list compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_names_unchecked` | check-prelude std-fs-read-dir unchecked directory name-list compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_next` | check-prelude std-fs-read-dir Option-returning directory entry name helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_optional` | check-prelude std-fs-query-result Option-returning directory entry-list compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_dir_unchecked` | check-prelude std-fs-read-dir unchecked directory entry-list compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_link` | check-prelude std-fs-query-result Result-returning symbolic-link target reader; docs/stdlib/modules/fs.md |
| `fn std::fs::read_link_detailed` | check-prelude std-fs-detailed-errors path-tagged symbolic-link read error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_link_optional` | check-prelude std-fs-query-result Option-returning symbolic-link target compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_link_unchecked` | check-prelude std-fs-query-result unchecked symbolic-link target compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_optional` | check-prelude std-fs-read-result Option-returning whole-file read compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_or_default` | check-prelude std-fs-read-result empty-string whole-file read compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_to_string` | check-prelude std-fs-read-result Result-returning whole-file byte-string read helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_to_string_optional` | check-prelude std-fs-read-result Option-returning whole-file byte-string compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_to_string_or_default` | check-prelude std-fs-read-write empty-string whole-file byte-string compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_to_string_unchecked` | check-prelude std-fs-read-result unchecked whole-file byte-string compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::read_unchecked` | check-prelude std-fs-read-result unchecked whole-file read compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove` | check-prelude std-fs-mutation-result Result-returning file removal helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_bool` | check-prelude std-fs-basic bool file removal compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir` | check-prelude std-fs-mutation-result Result-returning single-directory removal helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_all` | check-prelude std-fs-remove-dir-all Result-returning recursive directory tree removal helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_all_bool` | check-prelude std-fs-remove-dir-all bool recursive directory tree removal compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_all_detailed` | check-prelude std-fs-detailed-errors path-tagged recursive directory removal error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_bool` | check-prelude std-fs-rename-dir bool single-directory removal compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_detailed` | check-prelude std-fs-detailed-errors path-tagged directory removal error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_raw` | check-prelude std-fs-mutation-result raw Result-returning directory removal compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_dir_unchecked` | check-prelude std-fs-rename-dir unchecked single-directory removal compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_file` | check-prelude std-fs-mutation-result Result-returning file removal alias; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_file_bool` | check-prelude std-fs-basic bool file removal alias; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_file_detailed` | check-prelude std-fs-detailed-errors path-tagged file removal error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_raw` | check-prelude std-fs-mutation-result raw Result-returning unlink compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::remove_unchecked` | check-prelude std-fs-basic unchecked file removal compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::rename` | check-prelude std-fs-mutation-result Result-returning file rename helper; docs/stdlib/modules/fs.md |
| `fn std::fs::rename_bool` | check-prelude std-fs-rename-dir bool file rename compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::rename_detailed` | check-prelude std-fs-detailed-errors source/target-tagged rename error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::rename_raw` | check-prelude std-fs-mutation-result raw Result-returning rename compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::rename_unchecked` | check-prelude std-fs-rename-dir unchecked file rename compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::seek` | check-prelude std-fs-seek Result-returning absolute file cursor seek helper; docs/stdlib/modules/fs.md |
| `fn std::fs::seek_raw` | check-prelude std-fs-seek raw Result-returning file seek compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::seek_unchecked` | check-prelude std-fs-seek unchecked file seek compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::set_mode` | check-prelude std-fs-mode chmod-backed permission-mode mutation helper; docs/stdlib/modules/fs.md |
| `fn std::fs::set_permissions` | check-prelude std-fs-mode Permissions-to-mode mutation helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symbolic_link` | check-prelude std-fs-links Result-returning symbolic-link helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symbolic_link_bool` | check-prelude std-fs-links bool symbolic-link compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symbolic_link_raw` | check-prelude std-fs-links raw Result-returning symbolic-link compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symbolic_link_unchecked` | check-prelude std-fs-links unchecked symbolic-link compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::symlink_metadata` | check-prelude std-fs-query-result Result-returning no-follow metadata helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symlink_metadata_detailed` | check-prelude std-fs-detailed-errors path-tagged no-follow metadata error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symlink_metadata_optional` | check-prelude std-fs-query-result Option-returning no-follow metadata compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symlink_metadata_raw` | check-prelude std-fs-query-result raw Result-returning no-follow metadata compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::symlink_metadata_unchecked` | check-prelude std-fs-query-result unchecked no-follow metadata compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::temp_dir` | check-prelude std-fs-temp hosted temporary directory constructor; docs/stdlib/modules/fs.md |
| `fn std::fs::temp_dir_in` | check-prelude std-fs-temp prefixed hosted temporary directory constructor; docs/stdlib/modules/fs.md |
| `fn std::fs::temp_file` | check-prelude std-fs-temp hosted temporary file constructor; docs/stdlib/modules/fs.md |
| `fn std::fs::temp_file_in` | check-prelude std-fs-temp prefixed hosted temporary file constructor; docs/stdlib/modules/fs.md |
| `fn std::fs::truncate` | check-prelude std-fs-create-truncate-copy source create/truncate helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_append` | check-prelude std-fs-read-write Option-returning append byte-count helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_canonicalize` | check-prelude std-fs-canonicalize Option-returning realpath-backed canonicalization helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_copy` | check-prelude std-fs-create-truncate-copy Option-returning copy byte-count helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_create` | check-prelude std-fs-create-truncate-copy Option-returning create helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_file_type` | check-prelude std-fs-metadata Option-returning path kind helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_lock_exclusive` | check-prelude std-fs-lock nonblocking hosted advisory exclusive file lock helper returning acquired flag or Error; docs/stdlib/modules/fs.md |
| `fn std::fs::try_lock_shared` | check-prelude std-fs-lock nonblocking hosted advisory shared file lock helper returning acquired flag or Error; docs/stdlib/modules/fs.md |
| `fn std::fs::try_metadata` | check-prelude std-fs-metadata Option-returning target-following stat-backed metadata helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_mode` | check-prelude std-fs-mode Option-returning permission-mode helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_open` | check-prelude std-fs-open-modes Option-returning mode-string open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_open_append` | check-prelude std-fs-append compatibility Option-returning append helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_open_dir` | check-prelude std-fs-read-dir Option-returning directory open helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_open_read` | check-prelude std-fs-basic compatibility Option-returning read helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_open_write` | check-prelude std-fs-basic compatibility Option-returning write helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read` | check-prelude std-fs-try-read Option-returning natural whole-file read alias; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_byte` | check-prelude std-fs-try-byte Option-returning file byte read helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_dir` | check-prelude std-fs-read-dir Option-returning directory name-list helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_dir_entries` | check-prelude std-fs-read-dir Option-returning directory entry-list helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_dir_info` | check-prelude std-fs-dir-entry-info Option-returning directory info helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_dir_names` | check-prelude std-fs-read-dir Option-returning directory name-list alias; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_link` | check-prelude std-fs-read-link Option-returning symbolic-link target reader; docs/stdlib/modules/fs.md |
| `fn std::fs::try_read_to_string` | check-prelude std-fs-try-read Option-returning whole-file byte-string read helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_symlink_metadata` | check-prelude std-fs-symlink-metadata Option-returning no-follow metadata helper; docs/stdlib/modules/fs.md |
| `fn std::fs::try_write` | check-prelude std-fs-read-write Option-returning write byte-count helper; docs/stdlib/modules/fs.md |
| `fn std::fs::unlock` | check-prelude std-fs-lock Result-returning hosted advisory file unlock helper; docs/stdlib/modules/fs.md |
| `fn std::fs::unlock_raw` | check-prelude std-fs-lock raw Result-returning hosted advisory file unlock helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write` | check-prelude std-fs-read-write Result-returning whole-file truncating write helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_bool` | check-prelude std-fs-read-result bool whole-file truncating write compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_byte` | check-prelude std-fs-basic Result-returning file byte write helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_byte_raw` | check-prelude std-fs-basic raw Result-returning file byte write compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_byte_unchecked` | check-prelude std-fs-basic unchecked file byte write compatibility hook; docs/stdlib/modules/fs.md |
| `fn std::fs::write_bytes` | check-prelude std-fs-basic Result-returning source byte-slice write helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_bytes_raw` | check-prelude std-fs-basic raw Result-returning file slice write compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_bytes_unchecked` | check-prelude std-fs-basic unchecked file slice write compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_detailed` | check-prelude std-fs-detailed-errors path-tagged whole-file write error helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_raw` | check-prelude std-fs-byte-result raw Result-returning whole-file truncating write byte-count compatibility helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_string` | check-prelude std-fs-byte-result Result-returning whole-file string write helper; docs/stdlib/modules/fs.md |
| `fn std::fs::write_string_detailed` | check-prelude std-fs-detailed-errors path-tagged whole-file string write error helper; docs/stdlib/modules/fs.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::fs::Dir::close` | check-prelude std-fs-read-dir Result-returning directory close method; docs/stdlib/modules/fs.md |
| `method std::fs::Dir::close_unchecked` | check-prelude std-fs-read-dir unchecked directory close compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::Dir::invalid` | check-prelude std-fs-read-dir invalid directory fallback constructor; docs/stdlib/modules/fs.md |
| `method std::fs::Dir::is_open` | check-prelude std-fs-read-dir open directory handle predicate; docs/stdlib/modules/fs.md |
| `method std::fs::Dir::next` | check-prelude std-fs-read-dir value directory handle next-name method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::file_type` | check-prelude std-fs-dir-entry-metadata Result-returning directory entry file-kind method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::file_type_optional` | check-prelude std-fs-dir-entry-metadata Option-returning directory entry file-kind compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::file_type_unchecked` | check-prelude std-fs-dir-entry-metadata unchecked directory entry file-kind compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::is_dir` | check-prelude std-fs-dir-entry-metadata directory entry target-following directory predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::is_file` | check-prelude std-fs-dir-entry-metadata directory entry target-following file predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::is_other` | check-prelude std-fs-dir-entry-metadata directory entry target-following other-kind predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::is_symlink` | check-prelude std-fs-dir-entry-metadata directory entry no-follow symlink predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::metadata` | check-prelude std-fs-dir-entry-metadata Result-returning target-following directory entry metadata method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::metadata_optional` | check-prelude std-fs-dir-entry-metadata Option-returning target-following directory entry metadata compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::metadata_unchecked` | check-prelude std-fs-dir-entry-metadata unchecked target-following directory entry metadata compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::name` | check-prelude std-fs-read-dir borrowed directory entry name method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::name_equals` | check-prelude std-fs-read-dir directory entry name comparison helper; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::path` | check-prelude std-fs-read-dir borrowed joined directory entry path method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::path_equals` | check-prelude std-fs-read-dir directory entry path comparison helper; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::symlink_metadata` | check-prelude std-fs-dir-entry-metadata Result-returning no-follow directory entry metadata method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::symlink_metadata_optional` | check-prelude std-fs-dir-entry-metadata Option-returning no-follow directory entry metadata compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::symlink_metadata_unchecked` | check-prelude std-fs-dir-entry-metadata unchecked no-follow directory entry metadata compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::try_file_type` | check-prelude std-fs-dir-entry-metadata Option-returning directory entry file-kind method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::try_metadata` | check-prelude std-fs-dir-entry-metadata Option-returning target-following directory entry metadata method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntry::try_symlink_metadata` | check-prelude std-fs-dir-entry-metadata Option-returning no-follow directory entry metadata method; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::entry` | check-prelude std-fs-dir-entry-info borrowed underlying directory entry accessor; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::file_type` | check-prelude std-fs-dir-entry-info borrowed per-entry file-kind Result snapshot accessor; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::file_type_err` | check-prelude std-fs-dir-entry-info per-entry file-kind error predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::file_type_ok` | check-prelude std-fs-dir-entry-info per-entry file-kind success predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::metadata` | check-prelude std-fs-dir-entry-info borrowed per-entry target-following metadata Result snapshot accessor; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::metadata_err` | check-prelude std-fs-dir-entry-info per-entry target-following metadata error predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::metadata_ok` | check-prelude std-fs-dir-entry-info per-entry target-following metadata success predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::name` | check-prelude std-fs-dir-entry-info borrowed underlying directory entry name accessor; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::name_equals` | check-prelude std-fs-dir-entry-info directory info name comparison helper; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::path` | check-prelude std-fs-dir-entry-info borrowed underlying directory entry path accessor; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::path_equals` | check-prelude std-fs-dir-entry-info directory info path comparison helper; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::symlink_metadata` | check-prelude std-fs-dir-entry-info borrowed per-entry no-follow metadata Result snapshot accessor; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::symlink_metadata_err` | check-prelude std-fs-dir-entry-info per-entry no-follow metadata error predicate; docs/stdlib/modules/fs.md |
| `method std::fs::DirEntryInfo::symlink_metadata_ok` | check-prelude std-fs-dir-entry-info per-entry no-follow metadata success predicate; docs/stdlib/modules/fs.md |
| `method std::fs::File::close` | check-prelude std-fs-basic Result-returning file close method; docs/stdlib/modules/fs.md |
| `method std::fs::File::close_unchecked` | check-prelude std-fs-basic unchecked file close compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::File::descriptor` | check-prelude std-os-fd non-owning std::os::Fd view over a file handle; docs/stdlib/modules/fs.md |
| `method std::fs::File::invalid` | check-prelude std-fs-basic invalid file fallback constructor; docs/stdlib/modules/fs.md |
| `method std::fs::File::is_open` | check-prelude std-fs-basic open-handle predicate; docs/stdlib/modules/fs.md |
| `method std::fs::File::lock_exclusive` | check-prelude std-fs-lock Result-returning hosted advisory exclusive file lock method; docs/stdlib/modules/fs.md |
| `method std::fs::File::lock_shared` | check-prelude std-fs-lock Result-returning hosted advisory shared file lock method; docs/stdlib/modules/fs.md |
| `method std::fs::File::position` | check-prelude std-fs-seek Result-returning file position method; docs/stdlib/modules/fs.md |
| `method std::fs::File::position_or` | check-prelude std-fs-seek fallback file position compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::File::read_byte` | check-prelude std-fs-basic value file handle byte read method; docs/stdlib/modules/fs.md |
| `method std::fs::File::seek` | check-prelude std-fs-seek Result-returning file seek method; docs/stdlib/modules/fs.md |
| `method std::fs::File::seek_unchecked` | check-prelude std-fs-seek unchecked file seek compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::File::try_lock_exclusive` | check-prelude std-fs-lock nonblocking hosted advisory exclusive file lock method returning acquired flag or Error; docs/stdlib/modules/fs.md |
| `method std::fs::File::try_lock_shared` | check-prelude std-fs-lock nonblocking hosted advisory shared file lock method returning acquired flag or Error; docs/stdlib/modules/fs.md |
| `method std::fs::File::try_read_byte` | check-prelude std-fs-try-byte value file handle Option-returning byte read method; docs/stdlib/modules/fs.md |
| `method std::fs::File::unlock` | check-prelude std-fs-lock Result-returning hosted advisory file unlock method; docs/stdlib/modules/fs.md |
| `method std::fs::File::write_byte` | check-prelude std-fs-basic Result-returning file byte write method; docs/stdlib/modules/fs.md |
| `method std::fs::File::write_byte_unchecked` | check-prelude std-fs-basic unchecked file byte write compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::File::write_bytes` | check-prelude std-fs-basic Result-returning file slice write method; docs/stdlib/modules/fs.md |
| `method std::fs::File::write_bytes_unchecked` | check-prelude std-fs-basic unchecked file slice write compatibility method; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::accessed` | check-prelude std-fs-metadata-times access timestamp accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::birth_time` | check-prelude std-fs-metadata-owner optional portable birth/creation timestamp alias; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::changed` | check-prelude std-fs-metadata-times status-change timestamp accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::created` | check-prelude std-fs-metadata-owner optional portable creation timestamp accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::created_or_changed` | check-prelude std-fs-metadata-owner creation-time fallback to POSIX status-change timestamp; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::file_type` | check-prelude std-fs-metadata file kind accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::gid` | check-prelude std-fs-metadata-owner POSIX group id alias; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::group` | check-prelude std-fs-metadata-owner POSIX group id accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::group_id` | check-prelude std-fs-metadata-owner POSIX group id alias; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::is_dir` | check-prelude std-fs-metadata directory kind predicate; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::is_file` | check-prelude std-fs-metadata regular-file kind predicate; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::is_other` | check-prelude std-fs-metadata non-file/non-dir/non-symlink predicate; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::is_symlink` | check-prelude std-fs-metadata symlink kind predicate; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::len` | check-prelude std-fs-metadata byte length accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::modified` | check-prelude std-fs-metadata-times modification timestamp accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::owner` | check-prelude std-fs-metadata-owner POSIX owner id accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::owner_id` | check-prelude std-fs-metadata-owner POSIX owner id alias; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::permissions` | check-prelude std-fs-metadata permission snapshot accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Metadata::uid` | check-prelude std-fs-metadata-owner POSIX owner id alias; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::append` | check-prelude std-fs-open-options append-mode option builder; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::create` | check-prelude std-fs-open-options create-if-missing option builder; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::create_new` | check-prelude std-fs-open-options exclusive create option builder; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::new` | check-prelude std-fs-open-options empty OpenOptions constructor; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::open` | check-prelude std-fs-open-result Result-returning OpenOptions open helper with Error payload; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::open_optional` | check-prelude std-fs-open-result Option-returning OpenOptions open compatibility helper; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::open_raw` | check-prelude std-fs-open-result raw Result-returning OpenOptions open compatibility helper; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::open_unchecked` | check-prelude std-fs-open-result unchecked OpenOptions open compatibility helper returning invalid handles; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::read` | check-prelude std-fs-open-options read option builder; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::truncate` | check-prelude std-fs-open-options truncate option builder; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::try_open` | check-prelude std-fs-open-options Option-returning OpenOptions open helper; docs/stdlib/modules/fs.md |
| `method std::fs::OpenOptions::write` | check-prelude std-fs-open-options write option builder; docs/stdlib/modules/fs.md |
| `method std::fs::PathError::code` | check-prelude std-fs-detailed-errors wrapped compact host error code accessor; docs/stdlib/modules/fs.md |
| `method std::fs::PathError::kind` | check-prelude std-fs-detailed-errors wrapped shared error kind accessor; docs/stdlib/modules/fs.md |
| `method std::fs::PathError::operation` | check-prelude std-fs-detailed-errors structured filesystem operation accessor; docs/stdlib/modules/fs.md |
| `method std::fs::PathError::path` | check-prelude std-fs-detailed-errors borrowed failing path accessor; docs/stdlib/modules/fs.md |
| `method std::fs::PathError::path_equals` | check-prelude std-fs-detailed-errors failing path comparison helper; docs/stdlib/modules/fs.md |
| `method std::fs::PathError::reason` | check-prelude std-fs-detailed-errors wrapped shared Error accessor; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::all` | check-prelude std-fs-mode all-permission constructor for mode mutation; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::any` | check-prelude std-fs-permissions any-access predicate; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::can_execute` | check-prelude std-fs-permissions executable/searchable access method; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::can_read` | check-prelude std-fs-permissions readable access method; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::can_write` | check-prelude std-fs-permissions writable access method; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::none` | check-prelude std-fs-permissions all-false permissions constructor; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::read_only` | check-prelude std-fs-mode readable-only constructor for mode mutation; docs/stdlib/modules/fs.md |
| `method std::fs::Permissions::to_mode` | check-prelude std-fs-mode POSIX permission-mode conversion helper; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::code` | check-prelude std-fs-detailed-errors wrapped compact host error code accessor; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::kind` | check-prelude std-fs-detailed-errors wrapped shared error kind accessor; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::operation` | check-prelude std-fs-detailed-errors structured filesystem operation accessor; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::reason` | check-prelude std-fs-detailed-errors wrapped shared Error accessor; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::source` | check-prelude std-fs-detailed-errors failing source path accessor; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::source_equals` | check-prelude std-fs-detailed-errors failing source path comparison helper; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::target` | check-prelude std-fs-detailed-errors failing target path accessor; docs/stdlib/modules/fs.md |
| `method std::fs::TwoPathError::target_equals` | check-prelude std-fs-detailed-errors failing target path comparison helper; docs/stdlib/modules/fs.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::fs` | std fs file handle tests; docs/stdlib/modules/fs.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::fs::Dir` | std fs directory handle tests; docs/stdlib/modules/fs.md |
| `struct std::fs::DirEntry` | std fs directory entry tests; docs/stdlib/modules/fs.md |
| `struct std::fs::DirEntryInfo` | check-prelude std-fs-dir-entry-info directory entry plus per-entry metadata Result snapshots; docs/stdlib/modules/fs.md |
| `struct std::fs::File` | std fs file handle tests; docs/stdlib/modules/fs.md |
| `struct std::fs::Metadata` | std fs metadata tests; docs/stdlib/modules/fs.md |
| `struct std::fs::OpenOptions` | check-prelude std-fs-open-options file open option builder; docs/stdlib/modules/fs.md |
| `struct std::fs::PathError` | check-prelude std-fs-detailed-errors structured single-path filesystem error; docs/stdlib/modules/fs.md |
| `struct std::fs::Permissions` | std fs permission query tests; docs/stdlib/modules/fs.md |
| `struct std::fs::TwoPathError` | check-prelude std-fs-detailed-errors structured source/target filesystem error; docs/stdlib/modules/fs.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::fs::Error` | check-prelude std-error-integration shared filesystem error alias; docs/stdlib/modules/fs.md |
| `type std::fs::ErrorKind` | check-prelude std-error-integration shared filesystem error-kind alias; docs/stdlib/modules/fs.md |
| `type std::fs::TempDir` | check-prelude std-fs-temp filesystem temporary directory handle alias; docs/stdlib/modules/fs.md |
| `type std::fs::TempFile` | check-prelude std-fs-temp filesystem temporary file handle alias; docs/stdlib/modules/fs.md |

## `std::hash`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::hash::bytes` | check-prelude std-hash-basic byte-slice hasher convenience; docs/stdlib/modules/hash.md |
| `fn std::hash::combine` | check-prelude std-hash-combine-helpers precomputed u64 hash composition helper; docs/stdlib/modules/hash.md |
| `fn std::hash::finish` | check-prelude std-hash-basic hasher finalization; docs/stdlib/modules/hash.md |
| `fn std::hash::new` | check-prelude std-hash-basic hasher constructor; docs/stdlib/modules/hash.md |
| `fn std::hash::pair[T: Hash[T]` | check-prelude std-hash-combine-helpers ordered two-value hash helper; docs/stdlib/modules/hash.md |
| `fn std::hash::reset` | check-prelude std-hash-basic hasher reset; docs/stdlib/modules/hash.md |
| `fn std::hash::string` | check-prelude std-hash-basic owned String hasher convenience; docs/stdlib/modules/hash.md |
| `fn std::hash::value[T: Hash[T]` | check-prelude std-hash-basic generic hash value helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write[T: Hash[T]` | check-prelude std-hash-basic generic hash feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_bool` | check-prelude std-hash-basic primitive bool feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_byte` | check-prelude std-hash-basic primitive byte feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_bytes` | check-prelude std-hash-basic byte-slice feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_i16` | check-prelude std-hash-integer-widths primitive signed 16-bit feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_i32` | check-prelude std-hash-integer-widths primitive signed 32-bit feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_i64` | check-prelude std-hash-basic primitive signed integer feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_i8` | check-prelude std-hash-integer-widths primitive signed 8-bit feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_u16` | check-prelude std-hash-integer-widths primitive unsigned 16-bit feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_u32` | check-prelude std-hash-integer-widths primitive unsigned 32-bit feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_u64` | check-prelude std-hash-basic primitive unsigned integer feed helper; docs/stdlib/modules/hash.md |
| `fn std::hash::write_u8` | check-prelude std-hash-integer-widths primitive unsigned 8-bit feed helper; docs/stdlib/modules/hash.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::hash` | check-prelude std-hash-basic source deterministic hashing helpers; docs/stdlib/modules/hash.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::hash::Hasher` | check-prelude std-hash-basic source hasher state; docs/stdlib/modules/hash.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::hash::Hash[T]` | check-prelude std-hash-basic source hash trait; docs/stdlib/modules/hash.md |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::hash::Hash[T]::hash` | check-prelude std-hash-basic primitive trait impl dispatch; docs/stdlib/modules/hash.md |

## `std::input`

Tier: `hosted`. Stability reading: platform-backed.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::input::line` | prelude input/read_line tests; docs/dev/test-matrix.md Prelude row |
| `fn std::input::owned_line` | prelude owned read_line tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `fn std::input::read_byte` | prelude byte input tests; docs/dev/test-matrix.md Prelude row |
| `fn std::input::try_read_byte` | std input byte option tests; docs/stdlib/modules/input.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::input` | prelude input tests; docs/dev/test-matrix.md Prelude row |

### use

| API | Coverage note |
| --- | --- |
| `use std::input` | std root re-export tests; docs/dev/test-matrix.md Prelude row |

## `std::io`

Tier: `hosted`. Stability reading: platform-backed.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::io::ReadByte` | check-prelude std-io-result byte-read status distinguishing byte, EOF, and adapter error; docs/stdlib/modules/io.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::io::buf_reader[R: Reader]` | std io buffered tests; docs/stdlib/modules/io.md |
| `fn std::io::buf_writer[W: Writer]` | std io buffered tests; docs/stdlib/modules/io.md |
| `fn std::io::copy[R: Reader, W: Writer]` | check-prelude std-io-copy/std-io-result Result-returning Reader-to-Writer byte-count helper; docs/stdlib/modules/io.md |
| `fn std::io::copy_unchecked[R: Reader, W: Writer]` | check-prelude std-io-copy/std-io-result bool compatibility Reader-to-Writer copy wrapper; docs/stdlib/modules/io.md |
| `fn std::io::cursor` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `fn std::io::eprint` | check-prelude std-io-natural-api Result-returning stderr text helper; docs/stdlib/modules/io.md |
| `fn std::io::eprint_text` | check-prelude std-io-natural-api Result-returning stderr text helper; docs/stdlib/modules/io.md |
| `fn std::io::eprintln` | check-prelude std-io-natural-api Result-returning stderr line helper; docs/stdlib/modules/io.md |
| `fn std::io::eprintln_text` | check-prelude std-io-natural-api Result-returning stderr line helper; docs/stdlib/modules/io.md |
| `fn std::io::flush[W: Writer]` | check-prelude std-io-result/std-io-traits-cursor Result-returning writer flush helper; docs/stdlib/modules/io.md |
| `fn std::io::flush_unchecked[W: Writer]` | check-prelude std-io-result bool compatibility writer flush helper; docs/stdlib/modules/io.md |
| `fn std::io::newline` | prelude IO and print tests; docs/dev/test-matrix.md Prelude row |
| `fn std::io::pipe` | check-prelude std-io-pipe Result-returning pipe Reader/Writer adapter constructor; docs/stdlib/modules/io.md |
| `fn std::io::pipe_optional` | check-prelude std-io-pipe Option-returning pipe constructor compatibility helper; docs/stdlib/modules/io.md |
| `fn std::io::print` | check-prelude std-io-natural-api Result-returning stdout text helper; docs/stdlib/modules/io.md |
| `fn std::io::print_text` | check-prelude std-io-natural-api Result-returning stdout text helper; docs/stdlib/modules/io.md |
| `fn std::io::println` | check-prelude std-io-natural-api Result-returning stdout line helper; docs/stdlib/modules/io.md |
| `fn std::io::println_text` | check-prelude std-io-natural-api Result-returning stdout line helper; docs/stdlib/modules/io.md |
| `fn std::io::read[R: Reader]` | check-prelude std-io-result Result-returning partial read helper with EOF count semantics; docs/stdlib/modules/io.md |
| `fn std::io::read_all[R: Reader]` | check-prelude std-io-read-all generic Reader whole-stream collector; docs/stdlib/modules/io.md |
| `fn std::io::read_byte` | prelude byte input tests; docs/dev/test-matrix.md Prelude row |
| `fn std::io::read_exact[R: Reader]` | check-prelude std-io-result/std-io-traits-cursor Result-returning exact-read helper; docs/stdlib/modules/io.md |
| `fn std::io::read_exact_unchecked[R: Reader]` | check-prelude std-io-result bool compatibility exact-read helper; docs/stdlib/modules/io.md |
| `fn std::io::read_line` | prelude read_line/input tests; docs/dev/test-matrix.md Prelude row |
| `fn std::io::read_line_from[R: Reader]` | check-prelude std-io-read-to-string Result-returning generic line reader helper; docs/stdlib/modules/io.md |
| `fn std::io::read_line_owned` | prelude owned read_line tests; docs/dev/test-matrix.md Prelude and Explicit memory zones rows |
| `fn std::io::read_one[R: Reader]` | check-prelude std-io-result generic one-byte read status helper; docs/stdlib/modules/io.md |
| `fn std::io::read_to_string[R: Reader]` | check-prelude std-io-read-to-string Result-returning generic Reader whole-stream owned String collector; docs/stdlib/modules/io.md |
| `fn std::io::read_to_string_unchecked[R: Reader]` | check-prelude std-io-read-to-string unchecked generic Reader whole-stream owned String collector; docs/stdlib/modules/io.md |
| `fn std::io::stderr` | std io stderr tests; docs/stdlib/modules/io.md |
| `fn std::io::stdin` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `fn std::io::stdout` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `fn std::io::try_copy[R: Reader, W: Writer]` | check-prelude std-io-copy generic Reader-to-Writer copy byte-count helper; docs/stdlib/modules/io.md |
| `fn std::io::write[W: Writer]` | check-prelude std-io-natural-api Result-returning generic whole-slice byte-count writer; docs/stdlib/modules/io.md |
| `fn std::io::write_all[W: Writer]` | check-prelude std-io-result/std-io-traits-cursor Result-returning whole-slice write helper; docs/stdlib/modules/io.md |
| `fn std::io::write_all_unchecked[W: Writer]` | check-prelude std-io-result bool compatibility whole-slice write helper; docs/stdlib/modules/io.md |
| `fn std::io::write_bool` | prelude IO and format tests; docs/dev/test-matrix.md Prelude row |
| `fn std::io::write_byte` | prelude byte IO tests; docs/dev/test-matrix.md Prelude row |
| `fn std::io::write_bytes` | std io byte slice tests; docs/stdlib/modules/io.md |
| `fn std::io::write_i64` | prelude IO and format tests; docs/dev/test-matrix.md Prelude row |
| `fn std::io::write_u64` | prelude IO and u64 format tests; docs/dev/test-matrix.md Prelude row |

### method

| API | Coverage note |
| --- | --- |
| `method std::io::BufReader[R]::buffered_len` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::capacity` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::is_empty` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::new` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::read` | check-prelude std-io-result Result-returning buffered partial read method; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::read_line` | check-prelude std-io-read-to-string Result-returning buffered reader line helper; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::read_one` | check-prelude std-io-result buffered one-byte status method; docs/stdlib/modules/io.md |
| `method std::io::BufReader[R]::read_to_string` | check-prelude std-io-read-to-string Result-returning buffered reader whole-stream helper; docs/stdlib/modules/io.md |
| `method std::io::BufWriter[W]::buffered_len` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufWriter[W]::capacity` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufWriter[W]::is_empty` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::BufWriter[W]::new` | std io buffered tests; docs/stdlib/modules/io.md |
| `method std::io::Cursor::read` | check-prelude std-io-result Result-returning cursor partial read method; docs/stdlib/modules/io.md |
| `method std::io::Cursor::read_line` | check-prelude std-io-read-to-string Result-returning cursor line helper; docs/stdlib/modules/io.md |
| `method std::io::Cursor::read_one` | check-prelude std-io-result cursor one-byte status method; docs/stdlib/modules/io.md |
| `method std::io::Cursor::read_to_string` | check-prelude std-io-read-to-string Result-returning cursor whole-stream helper; docs/stdlib/modules/io.md |
| `method std::io::Pipe::close` | check-prelude std-io-pipe Result-returning close for remaining pipe adapter ends; docs/stdlib/modules/io.md |
| `method std::io::Pipe::close_bool` | check-prelude std-io-pipe bool compatibility close for remaining pipe adapter ends; docs/stdlib/modules/io.md |
| `method std::io::Pipe::read_end` | check-prelude std-io-pipe borrowed pipe adapter read descriptor view; docs/stdlib/modules/io.md |
| `method std::io::Pipe::take_reader` | check-prelude std-io-pipe take pipe reader adapter ownership; docs/stdlib/modules/io.md |
| `method std::io::Pipe::take_writer` | check-prelude std-io-pipe take pipe writer adapter ownership; docs/stdlib/modules/io.md |
| `method std::io::Pipe::write_end` | check-prelude std-io-pipe borrowed pipe adapter write descriptor view; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::as_fd` | check-prelude std-io-pipe borrowed descriptor view from pipe reader; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::close` | check-prelude std-io-pipe Result-returning pipe reader close helper; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::close_bool` | check-prelude std-io-pipe bool compatibility pipe reader close helper; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::is_open` | check-prelude std-io-pipe pipe reader open predicate; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::read` | check-prelude std-io-pipe Result-returning pipe reader partial read method; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::read_line` | check-prelude std-io-pipe Result-returning pipe reader line helper; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::read_one` | check-prelude std-io-pipe pipe reader one-byte status method with closed-handle error; docs/stdlib/modules/io.md |
| `method std::io::PipeReader::read_to_string` | check-prelude std-io-pipe Result-returning pipe reader whole-stream helper; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::as_fd` | check-prelude std-io-pipe borrowed descriptor view from pipe writer; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::close` | check-prelude std-io-pipe Result-returning pipe writer close helper; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::close_bool` | check-prelude std-io-pipe bool compatibility pipe writer close helper; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::close_on_exec` | check-prelude std-io-pipe Result-returning pipe writer close-on-exec descriptor flag query; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::close_on_exec_optional` | check-prelude std-io-pipe Option-returning pipe writer close-on-exec compatibility query; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::is_open` | check-prelude std-io-pipe pipe writer open predicate; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::set_close_on_exec` | check-prelude std-io-pipe pipe writer close-on-exec descriptor flag setter; docs/stdlib/modules/io.md |
| `method std::io::PipeWriter::set_close_on_exec_bool` | check-prelude std-io-pipe bool compatibility pipe writer close-on-exec setter; docs/stdlib/modules/io.md |
| `method std::io::ReadByte::byte` | check-prelude std-io-result byte-read payload accessor; docs/stdlib/modules/io.md |
| `method std::io::ReadByte::error` | check-prelude std-io-result byte-read error payload accessor; docs/stdlib/modules/io.md |
| `method std::io::ReadByte::is_byte` | check-prelude std-io-result byte-read byte predicate; docs/stdlib/modules/io.md |
| `method std::io::ReadByte::is_eof` | check-prelude std-io-result byte-read EOF predicate; docs/stdlib/modules/io.md |
| `method std::io::ReadByte::is_error` | check-prelude std-io-result byte-read adapter-error predicate; docs/stdlib/modules/io.md |
| `method std::io::Stdin::read` | check-prelude std-io-result Result-returning stdin partial read method; docs/stdlib/modules/io.md |
| `method std::io::Stdin::read_line` | check-prelude std-io-read-to-string Result-returning stdin line method; docs/stdlib/modules/io.md |
| `method std::io::Stdin::read_one` | check-prelude std-io-result stdin one-byte status method; docs/stdlib/modules/io.md |
| `method std::io::Stdin::read_to_string` | check-prelude std-io-read-to-string Result-returning stdin whole-stream method; docs/stdlib/modules/io.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::io` | prelude IO tests and check-prelude std-io-pipe pipe adapters; docs/stdlib/modules/io.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::io::BufReader[R]` | std io buffered tests; docs/stdlib/modules/io.md |
| `struct std::io::BufWriter[W]` | std io buffered tests; docs/stdlib/modules/io.md |
| `struct std::io::Cursor` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `struct std::io::Pipe` | check-prelude std-io-pipe pipe Reader/Writer adapter owner; docs/stdlib/modules/io.md |
| `struct std::io::PipeReader` | check-prelude std-io-pipe pipe Reader adapter; docs/stdlib/modules/io.md |
| `struct std::io::PipeWriter` | check-prelude std-io-pipe pipe Writer adapter; docs/stdlib/modules/io.md |
| `struct std::io::Stderr` | std io stderr tests; docs/stdlib/modules/io.md |
| `struct std::io::Stdin` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `struct std::io::Stdout` | std io trait/cursor tests; docs/stdlib/modules/io.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::io::Reader` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `trait std::io::Seek` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `trait std::io::Writer` | std io trait/cursor tests; docs/stdlib/modules/io.md |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::io::Reader::read_byte` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `trait-method std::io::Seek::position` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `trait-method std::io::Seek::seek` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `trait-method std::io::Writer::flush` | std io trait/cursor tests; docs/stdlib/modules/io.md |
| `trait-method std::io::Writer::write` | check-prelude std-io-natural-api natural Result-returning trait byte-count write method; docs/stdlib/modules/io.md |
| `trait-method std::io::Writer::write_all` | check-prelude std-io-natural-api natural Result-returning trait whole-slice write method; docs/stdlib/modules/io.md |
| `trait-method std::io::Writer::write_byte` | std io trait/cursor tests; docs/stdlib/modules/io.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::io::Error` | check-prelude std-error-integration shared IO error alias; docs/stdlib/modules/io.md |
| `type std::io::ErrorKind` | check-prelude std-error-integration shared IO error-kind alias; docs/stdlib/modules/io.md |

## `std::iter`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::iter::all[T, I: std::Iterator[T]` | check-prelude std-iter-consumers predicate all consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::any[T, I: std::Iterator[T]` | check-prelude std-iter-consumers predicate any consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::count[T, I: std::Iterator[T]` | check-prelude std-iter-consumers total item count consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::count_if[T, I: std::Iterator[T]` | check-prelude std-iter-consumers predicate count consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::empty[T]` | check-prelude std-iter-once-empty finite empty source iterator; docs/stdlib/modules/iter.md |
| `fn std::iter::enumerate[T, I: std::Iterator[T]` | check-prelude std-iter-adapters lazy tuple index/value adapter; docs/stdlib/modules/iter.md |
| `fn std::iter::filter[T, I: std::Iterator[T]` | check-prelude std-iter-adapters lazy predicate adapter; docs/stdlib/modules/iter.md |
| `fn std::iter::find_if[T, I: std::Iterator[T]` | check-prelude std-iter-consumers predicate find consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::fold[T, U, I: std::Iterator[T]` | check-prelude std-iter-adapters eager fold consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::last[T, I: std::Iterator[T]` | check-prelude std-iter-consumers last item consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::map[T, U, I: std::Iterator[T]` | check-prelude std-iter-adapters lazy mapping adapter; docs/stdlib/modules/iter.md |
| `fn std::iter::nth[T, I: std::Iterator[T]` | check-prelude std-iter-consumers indexed item consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::once[T]` | check-prelude std-iter-once-empty finite single-value source iterator; docs/stdlib/modules/iter.md |
| `fn std::iter::position[T, I: std::Iterator[T]` | check-prelude std-iter-consumers predicate position consumer; docs/stdlib/modules/iter.md |
| `fn std::iter::range[T]` | range for-loop tests; docs/dev/test-matrix.md Control flow row |
| `fn std::iter::range_inclusive[T]` | inclusive range tests; docs/dev/test-matrix.md Control flow row |
| `fn std::iter::reduce[T, I: std::Iterator[T]` | check-prelude std-iter-adapters eager optional reduction; docs/stdlib/modules/iter.md |
| `fn std::iter::repeat_with[T]` | check-prelude std-iter-repeat-with generator-backed source iterator; docs/stdlib/modules/iter.md |
| `fn std::iter::skip[T, I: std::Iterator[T]` | check-prelude std-iter-adapters lazy drop-count adapter; docs/stdlib/modules/iter.md |
| `fn std::iter::take[T, I: std::Iterator[T]` | check-prelude std-iter-adapters lazy bounded adapter; docs/stdlib/modules/iter.md |
| `fn std::iter::zip[T, U, I: std::Iterator[T]` | check-prelude std-iter-adapters lazy tuple pair adapter; docs/stdlib/modules/iter.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::iter` | range and iterator tests; docs/dev/test-matrix.md Control flow row |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::iter::EmptyIter[T]` | check-prelude std-iter-once-empty finite empty iterator state; docs/stdlib/modules/iter.md |
| `struct std::iter::Enumerate[I, T]` | check-prelude std-iter-adapters lazy enumerate adapter state; docs/stdlib/modules/iter.md |
| `struct std::iter::Filter[I, T]` | check-prelude std-iter-adapters lazy filter adapter state; docs/stdlib/modules/iter.md |
| `struct std::iter::Map[I, T, U]` | check-prelude std-iter-adapters lazy map adapter state; docs/stdlib/modules/iter.md |
| `struct std::iter::OnceIter[T]` | check-prelude std-iter-once-empty finite single-value iterator state; docs/stdlib/modules/iter.md |
| `struct std::iter::RangeInclusive[T]` | inclusive range tests; docs/dev/test-matrix.md Control flow row |
| `struct std::iter::Range[T]` | range tests; docs/dev/test-matrix.md Control flow row |
| `struct std::iter::RepeatWith[T]` | check-prelude std-iter-repeat-with generator-backed infinite iterator state; docs/stdlib/modules/iter.md |
| `struct std::iter::Skip[I, T]` | check-prelude std-iter-adapters lazy skip adapter state; docs/stdlib/modules/iter.md |
| `struct std::iter::Take[I, T]` | check-prelude std-iter-adapters lazy take adapter state; docs/stdlib/modules/iter.md |
| `struct std::iter::Zip[I, J, T, U]` | check-prelude std-iter-adapters lazy zip adapter state; docs/stdlib/modules/iter.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::iter::IntoIterator[T]` | iterator module tests; docs/dev/test-matrix.md Control flow row |
| `trait std::iter::Iterable[T]` | iterator module tests; docs/dev/test-matrix.md Control flow row |
| `trait std::iter::Iterator[T]` | iterator module tests; docs/dev/test-matrix.md Control flow row |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::iter::IntoIterator[T]::into_iter` | iterator module tests; docs/dev/test-matrix.md Control flow row |
| `trait-method std::iter::Iterator[T]::next` | iterator module tests; docs/dev/test-matrix.md Control flow row |

### use

| API | Coverage note |
| --- | --- |
| `use std::iter::DoubleEndedIterator` | check-prelude std-iter-double-ended module alias for root supertrait; docs/stdlib/modules/iter.md |
| `use std::iter::ExactSizeIterator` | check-prelude std-iter-exact-size module alias for root supertrait; docs/stdlib/modules/iter.md |
| `use std::iter::collect` | check-prelude std-iter-adapters natural iter::collect alias backed by std::vec; docs/stdlib/modules/iter.md |

## `std::log`

Tier: `hosted`. Stability reading: platform-backed.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::log::Level` | check-prelude std-log-basic source logging level enum; docs/stdlib/modules/log.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::log::debug` | check-prelude std-log-basic debug-level string message helper; docs/stdlib/modules/log.md |
| `fn std::log::enabled` | check-prelude std-log-basic level threshold predicate; docs/stdlib/modules/log.md |
| `fn std::log::error` | check-prelude std-log-basic error-level string message helper; docs/stdlib/modules/log.md |
| `fn std::log::info` | check-prelude std-log-basic info-level string message helper; docs/stdlib/modules/log.md |
| `fn std::log::message` | check-prelude std-log-basic level-prefixed string stderr logging; docs/stdlib/modules/log.md |
| `fn std::log::name` | check-prelude std-log-basic level label helper; docs/stdlib/modules/log.md |
| `fn std::log::rank` | check-prelude std-log-basic level ordering helper; docs/stdlib/modules/log.md |
| `fn std::log::trace` | check-prelude std-log-basic trace-level string message helper; docs/stdlib/modules/log.md |
| `fn std::log::warn` | check-prelude std-log-basic warn-level string message helper; docs/stdlib/modules/log.md |
| `fn std::log::write` | check-prelude std-log-basic level-prefixed byte-slice stderr logging; docs/stdlib/modules/log.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::log` | check-prelude std-log-basic source stderr logging module; docs/stdlib/modules/log.md |

## `std::math`

Tier: `core`. Stability reading: stable candidate.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::math::abs` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::checked_abs` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_add` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_div` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_mul` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_neg` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_pow` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_rem` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::checked_sub` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::div_ceil` | std math division rounding tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::div_floor` | std math division rounding tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::gcd` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::is_even` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::is_negative` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::is_odd` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::is_positive` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::is_zero` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::lcm` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::max_value` | std math integer helper tests; docs/stdlib/modules/math.md |
| `fn std::math::min_value` | std math integer helper tests; docs/stdlib/modules/math.md |
| `fn std::math::mod_floor` | std math division rounding tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::overflowing_add` | std math wrapping/overflowing tests; tuple return keeps wrapped value and overflow flag; docs/stdlib/modules/math.md |
| `fn std::math::overflowing_mul` | std math wrapping/overflowing tests; tuple return keeps wrapped value and overflow flag; docs/stdlib/modules/math.md |
| `fn std::math::overflowing_pow` | std math wrapping/overflowing tests; tuple return keeps wrapped value and overflow flag; docs/stdlib/modules/math.md |
| `fn std::math::overflowing_sub` | std math wrapping/overflowing tests; tuple return keeps wrapped value and overflow flag; docs/stdlib/modules/math.md |
| `fn std::math::pow` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::saturating_abs` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::saturating_add` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::saturating_div` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::saturating_mul` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::saturating_neg` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::saturating_pow` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::saturating_sub` | std math checked/saturating tests; docs/stdlib/modules/math.md |
| `fn std::math::sign` | std math integer helper tests; docs/stdlib/api-reference.md Math section |
| `fn std::math::wrapping_add` | std math wrapping/overflowing tests; docs/stdlib/modules/math.md |
| `fn std::math::wrapping_mul` | std math wrapping/overflowing tests; docs/stdlib/modules/math.md |
| `fn std::math::wrapping_pow` | std math wrapping/overflowing tests; docs/stdlib/modules/math.md |
| `fn std::math::wrapping_sub` | std math wrapping/overflowing tests; docs/stdlib/modules/math.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::math` | std math integer helper tests; docs/stdlib/overview.md module map |

## `std::mem`

Tier: `core`. Stability reading: stable candidate.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::mem::align_of[T]` | layout query tests; docs/dev/test-matrix.md C FFI row |
| `fn std::mem::copy_bytes` | std mem byte operation tests; docs/stdlib/modules/mem.md |
| `fn std::mem::move_bytes` | std mem byte operation tests; docs/stdlib/modules/mem.md |
| `fn std::mem::page_size` | std mem runtime page-size tests; docs/stdlib/modules/mem.md |
| `fn std::mem::ptr_add[T]` | typed pointer arithmetic tests; docs/dev/test-matrix.md C FFI row |
| `fn std::mem::ptr_load[T]` | raw pointer load tests; docs/dev/test-matrix.md C FFI row |
| `fn std::mem::ptr_offset[T]` | byte pointer offset tests; docs/dev/test-matrix.md C FFI row |
| `fn std::mem::ptr_store[T]` | raw pointer store tests; docs/dev/test-matrix.md C FFI row |
| `fn std::mem::replace[T]` | std mem value helper tests; docs/dev/test-matrix.md Prelude and C FFI rows |
| `fn std::mem::set_bytes` | std mem byte operation tests; docs/stdlib/modules/mem.md |
| `fn std::mem::size_of[T]` | layout query tests; docs/dev/test-matrix.md C FFI row |
| `fn std::mem::swap[T]` | std mem value helper tests; docs/dev/test-matrix.md Prelude and C FFI rows |

### module

| API | Coverage note |
| --- | --- |
| `module std::mem` | layout and pointer tests; docs/dev/test-matrix.md C FFI row |

## `std::net`

Tier: `hosted`. Stability reading: platform-backed.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::net::Shutdown` | check-prelude std-net-tcp-loopback and std-net-unix-socket stream shutdown mode enum; docs/stdlib/modules/net.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::net::connect` | check-prelude std-net-tcp-loopback module-level TCP connect helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::connect_host` | check-prelude std-net-dns-lookup host-port TCP connect helper input validation; docs/stdlib/modules/net.md |
| `fn std::net::ipv4` | check-prelude std-net-addresses source IPv4 constructor; docs/stdlib/modules/net.md |
| `fn std::net::ipv6` | check-prelude std-net-addresses source IPv6 constructor; docs/stdlib/modules/net.md |
| `fn std::net::listen` | check-prelude std-net-tcp-loopback module-level TCP listener helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::localhost` | check-prelude std-net-addresses source loopback socket address constructor; docs/stdlib/modules/net.md |
| `fn std::net::lookup_v4` | check-prelude std-net-dns-lookup Result-returning IPv4 name lookup with Error payload; docs/stdlib/modules/net.md |
| `fn std::net::lookup_v4_optional` | check-prelude std-net-dns-lookup Option-returning IPv4 name lookup compatibility helper; docs/stdlib/modules/net.md |
| `fn std::net::lookup_v4_raw` | check-prelude std-net-dns-lookup raw Result-returning IPv4 name lookup compatibility helper; docs/stdlib/modules/net.md |
| `fn std::net::lookup_v6` | check-prelude std-net-dns-lookup Result-returning IPv6 name lookup with Error payload; docs/stdlib/modules/net.md |
| `fn std::net::lookup_v6_optional` | check-prelude std-net-dns-lookup Option-returning IPv6 name lookup compatibility helper; docs/stdlib/modules/net.md |
| `fn std::net::lookup_v6_raw` | check-prelude std-net-dns-lookup raw Result-returning IPv6 name lookup compatibility helper; docs/stdlib/modules/net.md |
| `fn std::net::resolve` | check-prelude std-net-dns-lookup Result-returning host-port endpoint resolver; docs/stdlib/modules/net.md |
| `fn std::net::resolve_all` | check-prelude std-net-dns-lookup zone-backed multi-address host resolver; docs/stdlib/modules/net.md |
| `fn std::net::resolve_optional` | check-prelude std-net-dns-lookup Option-returning host-port endpoint resolver compatibility helper; docs/stdlib/modules/net.md |
| `fn std::net::resolve_raw` | check-prelude std-net-dns-lookup raw Result-returning host-port endpoint resolver; docs/stdlib/modules/net.md |
| `fn std::net::resolve_service` | check-prelude std-net-dns-lookup service-name host resolver using the stdlib service table; docs/stdlib/modules/net.md |
| `fn std::net::service_port` | check-prelude std-net-dns-lookup Result-returning well-known service-name port lookup; docs/stdlib/modules/net.md |
| `fn std::net::service_port_bytes` | check-prelude std-net-dns-lookup byte-slice service-name port lookup; docs/stdlib/modules/net.md |
| `fn std::net::service_port_bytes_optional` | check-prelude std-net-dns-lookup Option-returning byte-slice service-name port lookup; docs/stdlib/modules/net.md |
| `fn std::net::service_port_optional` | check-prelude std-net-dns-lookup Option-returning well-known service-name port lookup; docs/stdlib/modules/net.md |
| `fn std::net::socket_addr` | check-prelude std-net-addresses source socket address constructor; docs/stdlib/modules/net.md |
| `fn std::net::tcp_connect` | check-prelude std-net-tcp-loopback explicit TCP connect helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::tcp_connect_host` | check-prelude std-net-dns-lookup explicit host-port TCP connect helper; docs/stdlib/modules/net.md |
| `fn std::net::tcp_connect_v6` | check-prelude std-net-ipv6-socket explicit IPv6 TCP connect helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::tcp_listen` | check-prelude std-net-tcp-loopback explicit TCP listener helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::tcp_listen_v6` | check-prelude std-net-ipv6-socket explicit IPv6 TCP listener helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::to_socket_addrs` | check-prelude std-net-dns-lookup module-level ToSocketAddrs-shaped endpoint resolver returning Vec; docs/stdlib/modules/net.md |
| `fn std::net::to_socket_addrs_service` | check-prelude std-net-dns-lookup module-level service-name resolver returning Vec; docs/stdlib/modules/net.md |
| `fn std::net::try_lookup_v4` | check-prelude std-net-dns-lookup Option-returning IPv4 name lookup compatibility alias; docs/stdlib/modules/net.md |
| `fn std::net::try_lookup_v6` | check-prelude std-net-dns-lookup Option-returning IPv6 name lookup compatibility alias; docs/stdlib/modules/net.md |
| `fn std::net::try_resolve` | check-prelude std-net-dns-lookup Option-returning host-port endpoint resolver compatibility alias; docs/stdlib/modules/net.md |
| `fn std::net::udp_bind` | check-prelude std-net-udp-socket module-level UDP bind helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::udp_bind_v6` | check-prelude std-net-ipv6-socket module-level IPv6 UDP bind helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::unix_connect` | check-prelude std-net-unix-socket module-level Unix stream connect helper returning Error; docs/stdlib/modules/net.md |
| `fn std::net::unix_listen` | check-prelude std-net-unix-socket module-level Unix listener helper returning Error; docs/stdlib/modules/net.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::net::IpAddr::is_loopback` | check-prelude std-net-addresses generic IP loopback predicate; docs/stdlib/modules/net.md |
| `method std::net::IpAddr::is_unspecified` | check-prelude std-net-addresses generic IP unspecified predicate; docs/stdlib/modules/net.md |
| `method std::net::IpAddr::is_v4` | check-prelude std-net-addresses generic IPv4 family predicate; docs/stdlib/modules/net.md |
| `method std::net::IpAddr::is_v6` | check-prelude std-net-addresses generic IPv6 family predicate; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::any` | check-prelude std-net-addresses IPv4 unspecified constructor; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::as_ip` | check-prelude std-net-addresses IPv4 to generic IP conversion; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::is_loopback` | check-prelude std-net-addresses IPv4 loopback predicate; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::is_unspecified` | check-prelude std-net-addresses IPv4 unspecified predicate; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::localhost` | check-prelude std-net-addresses IPv4 loopback constructor; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::new` | check-prelude std-net-addresses IPv4 associated constructor; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::octet` | check-prelude std-net-addresses IPv4 octet accessor; docs/stdlib/modules/net.md |
| `method std::net::Ipv4Addr::try_octet` | check-prelude std-net-address-validation Option-returning IPv4 octet accessor; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::any` | check-prelude std-net-addresses IPv6 unspecified constructor; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::as_ip` | check-prelude std-net-addresses IPv6 to generic IP conversion; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::is_loopback` | check-prelude std-net-addresses IPv6 loopback predicate; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::is_unspecified` | check-prelude std-net-addresses IPv6 unspecified predicate; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::localhost` | check-prelude std-net-addresses IPv6 loopback constructor; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::new` | check-prelude std-net-addresses IPv6 associated constructor; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::segment` | check-prelude std-net-addresses IPv6 segment accessor; docs/stdlib/modules/net.md |
| `method std::net::Ipv6Addr::try_segment` | check-prelude std-net-address-validation Option-returning IPv6 segment accessor; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::ip` | check-prelude std-net-addresses socket address IP accessor; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::is_loopback` | check-prelude std-net-addresses socket address loopback predicate; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::is_unspecified` | check-prelude std-net-addresses socket address unspecified predicate; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::localhost` | check-prelude std-net-addresses socket address loopback constructor; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::new` | check-prelude std-net-addresses socket address associated constructor; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::port` | check-prelude std-net-addresses socket address port accessor; docs/stdlib/modules/net.md |
| `method std::net::SocketAddr::with_port` | check-prelude std-net-addresses socket address port replacement; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::accept` | check-prelude std-net-tcp-loopback Result-returning TCP accept helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::accept_optional` | check-prelude std-net-tcp-loopback Option-returning TCP accept compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::accept_raw` | check-prelude std-net-tcp-loopback raw Result-returning TCP accept compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::accept_ready` | check-prelude std-os-poll Duration-based TCP listener accept-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::accept_ready_millis` | check-prelude std-os-poll millisecond TCP listener accept-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::bind` | check-prelude std-net-tcp-loopback Result-returning IPv4/IPv6 TCP bind helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::bind_optional` | check-prelude std-net-tcp-loopback Option-returning TCP bind compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::bind_raw` | check-prelude std-net-tcp-loopback raw Result-returning TCP bind compatibility helper and restricted-host error bridge; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::close` | check-prelude std-net-tcp-loopback explicit listener descriptor close helper; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::close_on_exec` | check-prelude std-net-tcp-loopback Result-returning listener close-on-exec descriptor flag query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::close_on_exec_optional` | check-prelude std-net-tcp-loopback Option-returning listener close-on-exec compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::close_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP listener close compatibility method; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::descriptor` | check-prelude std-net-tcp-loopback listener borrowed descriptor view; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::is_nonblocking` | check-prelude std-net-tcp-loopback listener nonblocking descriptor query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::is_nonblocking_optional` | check-prelude std-net-tcp-loopback Option-returning listener nonblocking compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::is_open` | check-prelude std-net-tcp-loopback listener open predicate; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::local_addr` | check-prelude std-net-tcp-loopback listener bound socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::local_addr_optional` | check-prelude std-net-tcp-loopback Option-returning listener local address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::local_addr_v6` | check-prelude std-net-ipv6-socket listener bound IPv6 socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::local_addr_v6_optional` | check-prelude std-net-ipv6-socket Option-returning listener IPv6 local address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::local_port` | check-prelude std-net-tcp-loopback listener bound-port lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::local_port_optional` | check-prelude std-net-tcp-loopback Option-returning listener local port compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::reuse_addr` | check-prelude std-net-tcp-loopback listener reuse-address socket option query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::reuse_addr_optional` | check-prelude std-net-tcp-loopback Option-returning listener reuse-address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::reuse_port` | check-prelude std-net-tcp-loopback listener reuse-port socket option query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::reuse_port_optional` | check-prelude std-net-tcp-loopback Option-returning listener reuse-port compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_accept_timeout` | check-prelude std-net-tcp-loopback Duration-based listener accept timeout setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_accept_timeout_millis` | check-prelude std-net-tcp-loopback listener accept timeout setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_accept_timeout_millis_unchecked` | check-prelude std-net-tcp-loopback unchecked listener accept timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_close_on_exec` | check-prelude std-net-tcp-loopback listener close-on-exec descriptor flag setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_close_on_exec_unchecked` | check-prelude std-net-tcp-loopback unchecked listener close-on-exec compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_nonblocking` | check-prelude std-net-tcp-loopback listener nonblocking descriptor setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_nonblocking_unchecked` | check-prelude std-net-tcp-loopback unchecked listener nonblocking compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_reuse_addr` | check-prelude std-net-tcp-loopback listener reuse-address socket option setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_reuse_addr_unchecked` | check-prelude std-net-tcp-loopback unchecked listener reuse-address compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_reuse_port` | check-prelude std-net-tcp-loopback listener reuse-port socket option setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::set_reuse_port_unchecked` | check-prelude std-net-tcp-loopback unchecked listener reuse-port compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::try_accept` | check-prelude std-net-tcp-loopback Option-returning TCP accept compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::TcpListener::try_bind` | check-prelude std-net-tcp-loopback Option-returning TCP bind compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::close` | check-prelude std-net-tcp-loopback explicit stream descriptor close helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::close_on_exec` | check-prelude std-net-tcp-loopback Result-returning stream close-on-exec descriptor flag query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::close_on_exec_optional` | check-prelude std-net-tcp-loopback Option-returning stream close-on-exec compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::close_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP stream close compatibility method; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::connect` | check-prelude std-net-tcp-loopback Result-returning IPv4/IPv6 TCP connect helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::connect_optional` | check-prelude std-net-tcp-loopback Option-returning TCP connect compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::connect_raw` | check-prelude std-net-tcp-loopback raw Result-returning TCP connect compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::descriptor` | check-prelude std-net-tcp-loopback stream borrowed descriptor view; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::disable_linger` | check-prelude std-net-tcp-loopback TCP SO_LINGER disable helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::disable_linger_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP SO_LINGER disable compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::flush` | check-prelude std-net-tcp-loopback stream flush helper preserving closed-handle errors; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::hop_limit` | check-prelude std-net-ipv6-socket TCP IPv6 hop-limit socket option query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::hop_limit_optional` | check-prelude std-net-ipv6-socket Option-returning TCP IPv6 hop-limit compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::is_nonblocking` | check-prelude std-net-tcp-loopback stream nonblocking descriptor query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::is_nonblocking_optional` | check-prelude std-net-tcp-loopback Option-returning stream nonblocking compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::is_open` | check-prelude std-net-tcp-loopback stream open predicate; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::keepalive` | check-prelude std-net-tcp-loopback TCP keepalive socket option query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::keepalive_optional` | check-prelude std-net-tcp-loopback Option-returning TCP keepalive compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::linger_seconds` | check-prelude std-net-tcp-loopback TCP SO_LINGER seconds query with None for disabled linger; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::local_addr` | check-prelude std-net-tcp-loopback stream local socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::local_addr_optional` | check-prelude std-net-tcp-loopback Option-returning stream local address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::local_addr_v6` | check-prelude std-net-ipv6-socket stream local IPv6 socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::local_addr_v6_optional` | check-prelude std-net-ipv6-socket Option-returning stream IPv6 local address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::nodelay` | check-prelude std-net-tcp-loopback TCP nodelay socket option query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::nodelay_optional` | check-prelude std-net-tcp-loopback Option-returning TCP nodelay compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::peer_addr` | check-prelude std-net-tcp-loopback stream peer socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::peer_addr_optional` | check-prelude std-net-tcp-loopback Option-returning stream peer address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::peer_addr_v6` | check-prelude std-net-ipv6-socket stream peer IPv6 socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::peer_addr_v6_optional` | check-prelude std-net-ipv6-socket Option-returning stream IPv6 peer address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read` | check-prelude std-net-tcp-loopback stream partial buffer read helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_exact` | check-prelude std-net-tcp-loopback stream buffer read helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_exact_slice` | check-prelude std-net-tcp-loopback stream exact slice read helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_exact_unchecked` | check-prelude std-net-tcp-loopback unchecked stream exact-read compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_ready` | check-prelude std-os-poll Duration-based TCP stream read-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_ready_millis` | check-prelude std-os-poll millisecond TCP stream read-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_to_end` | check-prelude std-net-tcp-loopback stream collect-to-Vec helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::read_to_string` | check-prelude std-net-tcp-loopback stream collect-to-String helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::recv_buffer_size` | check-prelude std-net-tcp-loopback TCP receive-buffer-size query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::send_buffer_size` | check-prelude std-net-tcp-loopback TCP send-buffer-size query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_close_on_exec` | check-prelude std-net-tcp-loopback stream close-on-exec descriptor flag setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_close_on_exec_unchecked` | check-prelude std-net-tcp-loopback unchecked stream close-on-exec compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_hop_limit` | check-prelude std-net-ipv6-socket TCP IPv6 hop-limit socket option setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_hop_limit_unchecked` | check-prelude std-net-ipv6-socket unchecked TCP IPv6 hop-limit compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_keepalive` | check-prelude std-net-tcp-loopback TCP keepalive socket option setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_keepalive_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP keepalive compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_linger_seconds` | check-prelude std-net-tcp-loopback TCP SO_LINGER seconds setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_linger_seconds_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP SO_LINGER seconds compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_nodelay` | check-prelude std-net-tcp-loopback TCP nodelay socket option setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_nodelay_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP nodelay compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_nonblocking` | check-prelude std-net-tcp-loopback stream nonblocking descriptor setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_nonblocking_unchecked` | check-prelude std-net-tcp-loopback unchecked stream nonblocking compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_read_timeout` | check-prelude std-net-tcp-loopback Duration-based stream read timeout setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_read_timeout_millis` | check-prelude std-net-tcp-loopback stream read timeout setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_read_timeout_millis_unchecked` | check-prelude std-net-tcp-loopback unchecked stream read timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_recv_buffer_size` | check-prelude std-net-tcp-loopback TCP receive-buffer-size setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_send_buffer_size` | check-prelude std-net-tcp-loopback TCP send-buffer-size setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_ttl` | check-prelude std-net-tcp-loopback TCP IPv4 TTL socket option setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_ttl_unchecked` | check-prelude std-net-tcp-loopback unchecked TCP IPv4 TTL compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_write_timeout` | check-prelude std-net-tcp-loopback Duration-based stream write timeout setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_write_timeout_millis` | check-prelude std-net-tcp-loopback stream write timeout setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::set_write_timeout_millis_unchecked` | check-prelude std-net-tcp-loopback unchecked stream write timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::shutdown` | check-prelude std-net-tcp-loopback stream half/full shutdown helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::try_connect` | check-prelude std-net-tcp-loopback Option-returning TCP connect compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::try_read_byte` | check-prelude std-net-tcp-loopback Option-returning single-byte stream read helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::ttl` | check-prelude std-net-tcp-loopback TCP IPv4 TTL socket option query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::ttl_optional` | check-prelude std-net-tcp-loopback Option-returning TCP IPv4 TTL compatibility query; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::write_all_unchecked` | check-prelude std-net-tcp-loopback unchecked stream write-all compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::write_ready` | check-prelude std-os-poll Duration-based TCP stream write-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::TcpStream::write_ready_millis` | check-prelude std-os-poll millisecond TCP stream write-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UdpRecvFrom::addr` | check-prelude std-net-udp-socket UDP receive source address accessor; docs/stdlib/modules/net.md |
| `method std::net::UdpRecvFrom::len` | check-prelude std-net-udp-socket UDP receive byte-count accessor; docs/stdlib/modules/net.md |
| `method std::net::UdpRecvFrom::source` | check-prelude std-net-udp-socket UDP receive source-address alias; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::bind` | check-prelude std-net-udp-socket Result-returning IPv4/IPv6 UDP bind helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::bind_optional` | check-prelude std-net-udp-socket Option-returning UDP bind compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::bind_raw` | check-prelude std-net-udp-socket raw Result-returning UDP bind compatibility helper and restricted-host error bridge; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::broadcast` | check-prelude std-net-udp-socket UDP broadcast socket option query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::broadcast_optional` | check-prelude std-net-udp-socket Option-returning UDP broadcast compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::close` | check-prelude std-net-udp-socket explicit UDP descriptor close helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::close_on_exec` | check-prelude std-net-udp-socket Result-returning UDP close-on-exec descriptor flag query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::close_on_exec_optional` | check-prelude std-net-udp-socket Option-returning UDP close-on-exec compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::close_unchecked` | check-prelude std-net-udp-socket unchecked UDP socket close compatibility method; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::connect` | check-prelude std-net-udp-socket connected UDP default-peer helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::descriptor` | check-prelude std-net-udp-socket UDP borrowed descriptor view; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::hop_limit` | check-prelude std-net-ipv6-socket UDP IPv6 hop-limit socket option query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::hop_limit_optional` | check-prelude std-net-ipv6-socket Option-returning UDP IPv6 hop-limit compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::is_nonblocking` | check-prelude std-net-udp-socket UDP nonblocking descriptor query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::is_nonblocking_optional` | check-prelude std-net-udp-socket Option-returning UDP nonblocking compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::is_open` | check-prelude std-net-udp-socket UDP open predicate; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::local_addr` | check-prelude std-net-udp-socket UDP local socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::local_addr_optional` | check-prelude std-net-udp-socket Option-returning UDP local address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::local_addr_v6` | check-prelude std-net-ipv6-socket UDP local IPv6 socket-address lookup; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::local_addr_v6_optional` | check-prelude std-net-ipv6-socket Option-returning UDP IPv6 local address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::local_port` | check-prelude std-net-udp-socket UDP bound-port lookup; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::local_port_optional` | check-prelude std-net-udp-socket Option-returning UDP local port compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::peek_from` | check-prelude std-net-udp-socket UDP source-address peek helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv` | check-prelude std-net-udp-socket connected UDP buffer receive helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv_buffer_size` | check-prelude std-net-udp-socket UDP receive-buffer-size query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv_byte` | check-prelude std-net-udp-socket single-byte UDP datagram receive helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv_byte_unchecked` | check-prelude std-net-udp-socket unchecked single-byte UDP receive compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv_from` | check-prelude std-net-udp-socket UDP source-address receive helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv_ready` | check-prelude std-os-poll Duration-based UDP receive-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::recv_ready_millis` | check-prelude std-os-poll millisecond UDP receive-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::reuse_addr` | check-prelude std-net-udp-socket UDP reuse-address socket option query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::reuse_addr_optional` | check-prelude std-net-udp-socket Option-returning UDP reuse-address compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::reuse_port` | check-prelude std-net-udp-socket UDP reuse-port socket option query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::reuse_port_optional` | check-prelude std-net-udp-socket Option-returning UDP reuse-port compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send` | check-prelude std-net-udp-socket connected UDP buffer send helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send_buffer_size` | check-prelude std-net-udp-socket UDP send-buffer-size query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send_byte_to` | check-prelude std-net-udp-socket single-byte UDP datagram send helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send_byte_to_unchecked` | check-prelude std-net-udp-socket unchecked single-byte UDP send compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send_ready` | check-prelude std-os-poll Duration-based UDP send-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send_ready_millis` | check-prelude std-os-poll millisecond UDP send-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::send_to` | check-prelude std-net-udp-socket UDP datagram buffer send helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_broadcast` | check-prelude std-net-udp-socket UDP broadcast socket option setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_broadcast_unchecked` | check-prelude std-net-udp-socket unchecked UDP broadcast compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_close_on_exec` | check-prelude std-net-udp-socket UDP close-on-exec descriptor flag setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_close_on_exec_unchecked` | check-prelude std-net-udp-socket unchecked UDP close-on-exec compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_hop_limit` | check-prelude std-net-ipv6-socket UDP IPv6 hop-limit socket option setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_hop_limit_unchecked` | check-prelude std-net-ipv6-socket unchecked UDP IPv6 hop-limit compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_nonblocking` | check-prelude std-net-udp-socket UDP nonblocking descriptor setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_nonblocking_unchecked` | check-prelude std-net-udp-socket unchecked UDP nonblocking compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_read_timeout` | check-prelude std-net-udp-socket Duration-based UDP read timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_read_timeout_millis` | check-prelude std-net-udp-socket UDP read timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_read_timeout_millis_unchecked` | check-prelude std-net-udp-socket unchecked UDP read timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_recv_buffer_size` | check-prelude std-net-udp-socket UDP receive-buffer-size setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_reuse_addr` | check-prelude std-net-udp-socket UDP reuse-address socket option setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_reuse_addr_unchecked` | check-prelude std-net-udp-socket unchecked UDP reuse-address compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_reuse_port` | check-prelude std-net-udp-socket UDP reuse-port socket option setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_reuse_port_unchecked` | check-prelude std-net-udp-socket unchecked UDP reuse-port compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_send_buffer_size` | check-prelude std-net-udp-socket UDP send-buffer-size setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_ttl` | check-prelude std-net-udp-socket UDP IPv4 TTL socket option setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_ttl_unchecked` | check-prelude std-net-udp-socket unchecked UDP IPv4 TTL compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_write_timeout` | check-prelude std-net-udp-socket Duration-based UDP write timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_write_timeout_millis` | check-prelude std-net-udp-socket UDP write timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::set_write_timeout_millis_unchecked` | check-prelude std-net-udp-socket unchecked UDP write timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::try_bind` | check-prelude std-net-udp-socket Option-returning UDP bind compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::try_recv_byte` | check-prelude std-net-udp-socket Option-returning single-byte UDP datagram receive helper; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::ttl` | check-prelude std-net-udp-socket UDP IPv4 TTL socket option query; docs/stdlib/modules/net.md |
| `method std::net::UdpSocket::ttl_optional` | check-prelude std-net-udp-socket Option-returning UDP IPv4 TTL compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::accept` | check-prelude std-net-unix-socket Result-returning Unix stream accept helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::accept_optional` | check-prelude std-net-unix-socket Option-returning Unix stream accept compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::accept_raw` | check-prelude std-net-unix-socket raw Result-returning Unix stream accept compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::accept_ready` | check-prelude std-os-poll Duration-based Unix listener accept-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::accept_ready_millis` | check-prelude std-os-poll millisecond Unix listener accept-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::bind` | check-prelude std-net-unix-socket Result-returning Unix listener bind helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::bind_optional` | check-prelude std-net-unix-socket Option-returning Unix listener bind compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::bind_raw` | check-prelude std-net-unix-socket raw Result-returning Unix listener bind compatibility helper and restricted-host error bridge; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::close` | check-prelude std-net-unix-socket explicit Unix listener close helper; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::close_on_exec` | check-prelude std-net-unix-socket Result-returning Unix listener close-on-exec descriptor flag query; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::close_on_exec_optional` | check-prelude std-net-unix-socket Option-returning Unix listener close-on-exec compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::close_unchecked` | check-prelude std-net-unix-socket unchecked Unix listener close compatibility method; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::descriptor` | check-prelude std-net-unix-socket Unix listener borrowed descriptor view; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::is_nonblocking` | check-prelude std-net-unix-socket Unix listener nonblocking descriptor query; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::is_nonblocking_optional` | check-prelude std-net-unix-socket Option-returning Unix listener nonblocking compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::is_open` | check-prelude std-net-unix-socket Unix listener open predicate; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::set_close_on_exec` | check-prelude std-net-unix-socket Unix listener close-on-exec descriptor flag setter; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::set_close_on_exec_unchecked` | check-prelude std-net-unix-socket unchecked Unix listener close-on-exec compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::set_nonblocking` | check-prelude std-net-unix-socket Unix listener nonblocking descriptor setter; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::set_nonblocking_unchecked` | check-prelude std-net-unix-socket unchecked Unix listener nonblocking compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::try_accept` | check-prelude std-net-unix-socket Option-returning Unix accept compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::UnixListener::try_bind` | check-prelude std-net-unix-socket Option-returning Unix bind compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::close` | check-prelude std-net-unix-socket explicit Unix stream close helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::close_on_exec` | check-prelude std-net-unix-socket Result-returning Unix stream close-on-exec descriptor flag query; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::close_on_exec_optional` | check-prelude std-net-unix-socket Option-returning Unix stream close-on-exec compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::close_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream close compatibility method; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::connect` | check-prelude std-net-unix-socket Result-returning Unix stream connect helper with Error payload; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::connect_optional` | check-prelude std-net-unix-socket Option-returning Unix stream connect compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::connect_raw` | check-prelude std-net-unix-socket raw Result-returning Unix stream connect compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::descriptor` | check-prelude std-net-unix-socket Unix stream borrowed descriptor view; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::flush` | check-prelude std-net-unix-socket Unix stream flush helper preserving closed-handle errors; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::is_nonblocking` | check-prelude std-net-unix-socket Unix stream nonblocking descriptor query; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::is_nonblocking_optional` | check-prelude std-net-unix-socket Option-returning Unix stream nonblocking compatibility query; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::is_open` | check-prelude std-net-unix-socket Unix stream open predicate; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read` | check-prelude std-net-unix-socket Unix stream partial buffer read helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_exact` | check-prelude std-net-unix-socket Unix stream buffer read helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_exact_slice` | check-prelude std-net-unix-socket Unix stream exact slice read helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_exact_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream exact-read compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_ready` | check-prelude std-os-poll Duration-based Unix stream read-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_ready_millis` | check-prelude std-os-poll millisecond Unix stream read-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_to_end` | check-prelude std-net-unix-socket Unix stream collect-to-Vec helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::read_to_string` | check-prelude std-net-unix-socket Unix stream collect-to-String helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_close_on_exec` | check-prelude std-net-unix-socket Unix stream close-on-exec descriptor flag setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_close_on_exec_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream close-on-exec compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_nonblocking` | check-prelude std-net-unix-socket Unix stream nonblocking descriptor setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_nonblocking_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream nonblocking compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_read_timeout` | check-prelude std-net-unix-socket Duration-based Unix stream read timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_read_timeout_millis` | check-prelude std-net-unix-socket Unix stream read timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_read_timeout_millis_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream read timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_write_timeout` | check-prelude std-net-unix-socket Duration-based Unix stream write timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_write_timeout_millis` | check-prelude std-net-unix-socket Unix stream write timeout setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::set_write_timeout_millis_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream write timeout compatibility setter; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::shutdown` | check-prelude std-net-unix-socket Unix stream half/full shutdown helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::try_connect` | check-prelude std-net-unix-socket Option-returning Unix connect compatibility alias; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::try_read_byte` | check-prelude std-net-unix-socket Option-returning single-byte Unix stream read helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::write_all_unchecked` | check-prelude std-net-unix-socket unchecked Unix stream write-all compatibility helper; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::write_ready` | check-prelude std-os-poll Duration-based Unix stream write-readiness probe; docs/stdlib/modules/net.md |
| `method std::net::UnixStream::write_ready_millis` | check-prelude std-os-poll millisecond Unix stream write-readiness probe; docs/stdlib/modules/net.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::net` | std net address value tests; docs/stdlib/modules/net.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::net::IpAddr` | std net generic IP address tests; docs/stdlib/modules/net.md |
| `struct std::net::Ipv4Addr` | std net IPv4 address tests; docs/stdlib/modules/net.md |
| `struct std::net::Ipv6Addr` | std net IPv6 address tests; docs/stdlib/modules/net.md |
| `struct std::net::SocketAddr` | std net socket address tests; docs/stdlib/modules/net.md |
| `struct std::net::TcpListener` | check-prelude std-net-tcp-loopback owned TCP listener handle; docs/stdlib/modules/net.md |
| `struct std::net::TcpStream` | check-prelude std-net-tcp-loopback owned TCP stream handle and IO traits; docs/stdlib/modules/net.md |
| `struct std::net::UdpRecvFrom` | check-prelude std-net-udp-socket UDP receive metadata value; docs/stdlib/modules/net.md |
| `struct std::net::UdpSocket` | check-prelude std-net-udp-socket owned UDP datagram socket handle; docs/stdlib/modules/net.md |
| `struct std::net::UnixListener` | check-prelude std-net-unix-socket owned Unix domain listener handle; docs/stdlib/modules/net.md |
| `struct std::net::UnixStream` | check-prelude std-net-unix-socket owned Unix domain stream handle and IO traits; docs/stdlib/modules/net.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::net::ToSocketAddrs` | check-prelude std-net-dns-lookup host-port endpoint resolver trait returning Vec; docs/stdlib/modules/net.md |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::net::ToSocketAddrs::to_socket_addrs` | check-prelude std-net-dns-lookup trait-qualified host-port endpoint resolution returning Vec; docs/stdlib/modules/net.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::net::Error` | check-prelude std-error-integration shared networking error alias; docs/stdlib/modules/net.md |
| `type std::net::ErrorKind` | check-prelude std-error-integration shared networking error-kind alias; docs/stdlib/modules/net.md |

## `std::option`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::option::OptionMut[T]` | check-prelude prelude-option-result-ref-access mutable borrowed option payload view union; docs/stdlib/modules/option-result.md |
| `enum std::option::OptionRef[T]` | check-prelude prelude-option-result-ref-access shared borrowed option payload view union; docs/stdlib/modules/option-result.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::option::OptionMut[T]::as_ref` | check-prelude prelude-option-result-ref-access downgrade mutable option payload view to shared view; docs/stdlib/modules/option-result.md |
| `method std::option::OptionMut[T]::expect` | check-prelude prelude-option-result-ref-access mutable option payload expect handle; docs/stdlib/modules/option-result.md |
| `method std::option::OptionMut[T]::is_none` | check-prelude prelude-option-result-ref-access mutable option payload absence predicate; docs/stdlib/modules/option-result.md |
| `method std::option::OptionMut[T]::is_some` | check-prelude prelude-option-result-ref-access mutable option payload presence predicate; docs/stdlib/modules/option-result.md |
| `method std::option::OptionMut[T]::unwrap` | check-prelude prelude-option-result-ref-access mutable option payload unwrap handle; docs/stdlib/modules/option-result.md |
| `method std::option::OptionRef[T]::expect` | check-prelude prelude-option-result-ref-access shared option payload expect handle; docs/stdlib/modules/option-result.md |
| `method std::option::OptionRef[T]::is_none` | check-prelude prelude-option-result-ref-access shared option payload absence predicate; docs/stdlib/modules/option-result.md |
| `method std::option::OptionRef[T]::is_some` | check-prelude prelude-option-result-ref-access shared option payload presence predicate; docs/stdlib/modules/option-result.md |
| `method std::option::OptionRef[T]::unwrap` | check-prelude prelude-option-result-ref-access shared option payload unwrap handle; docs/stdlib/modules/option-result.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::option` | prelude Option method tests; docs/dev/test-matrix.md Prelude row |

## `std::os`

Tier: `platform`. Stability reading: platform-specific.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::os::fd` | check-prelude std-os-fd non-owning descriptor view constructor; docs/stdlib/modules/os.md |
| `fn std::os::invalid` | check-prelude std-os-fd invalid descriptor view constructor; docs/stdlib/modules/os.md |
| `fn std::os::pipe` | check-prelude std-os-pipe Result-returning owned OS pipe descriptor pair constructor; docs/stdlib/modules/os.md |
| `fn std::os::pipe_optional` | check-prelude std-os-pipe Option-returning pipe constructor compatibility helper; docs/stdlib/modules/os.md |
| `fn std::os::poll_read` | check-prelude std-os-poll Duration-based descriptor read readiness helper; docs/stdlib/modules/os.md |
| `fn std::os::poll_read_millis` | check-prelude std-os-poll millisecond descriptor read readiness helper; docs/stdlib/modules/os.md |
| `fn std::os::poll_write` | check-prelude std-os-poll Duration-based descriptor write readiness helper; docs/stdlib/modules/os.md |
| `fn std::os::poll_write_millis` | check-prelude std-os-poll millisecond descriptor write readiness helper; docs/stdlib/modules/os.md |
| `fn std::os::stderr` | check-prelude std-os-fd stderr descriptor view constructor; docs/stdlib/modules/os.md |
| `fn std::os::stdin` | check-prelude std-os-fd stdin descriptor view constructor; docs/stdlib/modules/os.md |
| `fn std::os::stdout` | check-prelude std-os-fd stdout descriptor view constructor; docs/stdlib/modules/os.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::os::Fd::equals` | check-prelude std-os-fd descriptor identity comparison; docs/stdlib/modules/os.md |
| `method std::os::Fd::is_invalid` | check-prelude std-os-fd invalid descriptor predicate; docs/stdlib/modules/os.md |
| `method std::os::Fd::is_standard` | check-prelude std-os-fd standard descriptor predicate; docs/stdlib/modules/os.md |
| `method std::os::Fd::is_stderr` | check-prelude std-os-fd stderr descriptor predicate; docs/stdlib/modules/os.md |
| `method std::os::Fd::is_stdin` | check-prelude std-os-fd stdin descriptor predicate; docs/stdlib/modules/os.md |
| `method std::os::Fd::is_stdout` | check-prelude std-os-fd stdout descriptor predicate; docs/stdlib/modules/os.md |
| `method std::os::Fd::is_valid` | check-prelude std-os-fd valid descriptor predicate; docs/stdlib/modules/os.md |
| `method std::os::Fd::poll_read` | check-prelude std-os-poll Duration-based borrowed descriptor read-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::Fd::poll_read_millis` | check-prelude std-os-poll millisecond borrowed descriptor read-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::Fd::poll_write` | check-prelude std-os-poll Duration-based borrowed descriptor write-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::Fd::poll_write_millis` | check-prelude std-os-poll millisecond borrowed descriptor write-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::Fd::raw` | check-prelude std-os-fd raw descriptor accessor; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::as_fd` | check-prelude std-os-owned-fd borrowed descriptor view from owned handle; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::clone` | check-prelude std-os-owned-fd direct Error owned descriptor duplication helper; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::close` | check-prelude std-os-owned-fd direct Error owned descriptor close and disarm helper; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::close_bool` | check-prelude std-os-owned-fd bool compatibility owned descriptor close helper; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::close_on_exec` | check-prelude std-os-owned-fd direct Error close-on-exec descriptor flag query; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::close_on_exec_optional` | check-prelude std-os-owned-fd-flags Option-returning close-on-exec descriptor flag query; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::from_raw` | check-prelude std-os-owned-fd raw descriptor ownership handoff constructor; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::invalid` | check-prelude std-os-owned-fd invalid owned descriptor constructor; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::is_closed` | check-prelude std-os-owned-fd owned descriptor closed predicate; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::is_nonblocking` | check-prelude std-os-owned-fd direct Error nonblocking descriptor flag query; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::is_nonblocking_optional` | check-prelude std-os-owned-fd-nonblocking Option-returning nonblocking descriptor flag query; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::is_open` | check-prelude std-os-owned-fd owned descriptor open predicate; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::poll_read` | check-prelude std-os-poll Duration-based owned descriptor read-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::poll_read_millis` | check-prelude std-os-poll millisecond owned descriptor read-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::poll_write` | check-prelude std-os-poll Duration-based owned descriptor write-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::poll_write_millis` | check-prelude std-os-poll millisecond owned descriptor write-readiness probe; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::raw` | check-prelude std-os-owned-fd owned descriptor raw accessor; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::set_close_on_exec` | check-prelude std-os-owned-fd direct Error close-on-exec descriptor flag setter; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::set_close_on_exec_bool` | check-prelude std-os-owned-fd-flags bool compatibility close-on-exec descriptor flag setter; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::set_nonblocking` | check-prelude std-os-owned-fd direct Error nonblocking descriptor flag setter; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::set_nonblocking_bool` | check-prelude std-os-owned-fd-nonblocking bool compatibility nonblocking descriptor flag setter; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::take` | check-prelude std-os-owned-fd disarming raw descriptor take helper; docs/stdlib/modules/os.md |
| `method std::os::OwnedFd::try_clone` | check-prelude std-os-owned-fd-duplicate fallible owned descriptor duplication helper; docs/stdlib/modules/os.md |
| `method std::os::Pipe::close` | check-prelude std-os-pipe Result-returning close of both owned pipe ends; docs/stdlib/modules/os.md |
| `method std::os::Pipe::close_bool` | check-prelude std-os-pipe bool compatibility close of both owned pipe ends; docs/stdlib/modules/os.md |
| `method std::os::Pipe::close_read_end` | check-prelude std-os-pipe Result-returning close of owned pipe read end; docs/stdlib/modules/os.md |
| `method std::os::Pipe::close_read_end_bool` | check-prelude std-os-pipe bool compatibility close of owned pipe read end; docs/stdlib/modules/os.md |
| `method std::os::Pipe::close_write_end` | check-prelude std-os-pipe Result-returning close of owned pipe write end; docs/stdlib/modules/os.md |
| `method std::os::Pipe::close_write_end_bool` | check-prelude std-os-pipe bool compatibility close of owned pipe write end; docs/stdlib/modules/os.md |
| `method std::os::Pipe::read_end` | check-prelude std-os-pipe borrowed pipe read descriptor view; docs/stdlib/modules/os.md |
| `method std::os::Pipe::take_read_end` | check-prelude std-os-pipe take pipe read end ownership; docs/stdlib/modules/os.md |
| `method std::os::Pipe::take_write_end` | check-prelude std-os-pipe take pipe write end ownership; docs/stdlib/modules/os.md |
| `method std::os::Pipe::write_end` | check-prelude std-os-pipe borrowed pipe write descriptor view; docs/stdlib/modules/os.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::os` | check-prelude std-os-fd/std-os-owned-fd/std-os-owned-fd-duplicate/std-os-owned-fd-flags/std-os-owned-fd-nonblocking/std-os-pipe OS descriptor view and owner module; docs/stdlib/modules/os.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::os::Fd` | check-prelude std-os-fd non-owning file descriptor view; docs/stdlib/modules/os.md |
| `struct std::os::OwnedFd` | check-prelude std-os-owned-fd owning file descriptor close wrapper; docs/stdlib/modules/os.md |
| `struct std::os::Pipe` | check-prelude std-os-pipe owning pipe descriptor pair; docs/stdlib/modules/os.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::os::Error` | check-prelude std-error-integration shared OS error alias; docs/stdlib/modules/os.md |
| `type std::os::ErrorKind` | check-prelude std-error-integration shared OS error-kind alias; docs/stdlib/modules/os.md |

## `std::parse`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::parse::ParseErrorKind` | check-prelude std-parse-basic integer and float parse diagnostic categories; docs/stdlib/modules/parse.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::parse::binary_integer` | check-prelude std-parse-basic Result-returning signed binary integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::binary_integer_optional` | check-prelude std-parse-basic Option-returning signed binary integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::binary_integer_or` | check-prelude std-parse-basic signed binary integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::binary_integer_with_underscores` | check-prelude std-parse-basic Result-returning signed binary integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::binary_integer_with_underscores_optional` | check-prelude std-parse-basic Option-returning signed binary integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::binary_integer_with_underscores_or` | check-prelude std-parse-basic signed binary integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::boolean` | check-prelude std-parse-basic Result-returning ASCII-trimmed lowercase bool parser; docs/stdlib/modules/parse.md |
| `fn std::parse::boolean_optional` | check-prelude std-parse-basic Option-returning lowercase bool compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::boolean_or` | check-prelude std-parse-basic lowercase bool parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::float` | check-prelude std-parse-basic Result-returning decimal float parser; docs/stdlib/modules/parse.md |
| `fn std::parse::float_error` | check-prelude std-parse-basic decimal float syntax and range diagnostic helper; docs/stdlib/modules/parse.md |
| `fn std::parse::float_optional` | check-prelude std-parse-basic Option-returning decimal float compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::float_or` | check-prelude std-parse-basic fallback decimal float parser; docs/stdlib/modules/parse.md |
| `fn std::parse::float_unchecked` | check-prelude std-parse-basic asserting decimal float compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::float_with_underscores` | check-prelude std-parse-basic Result-returning decimal float parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::float_with_underscores_error` | check-prelude std-parse-basic decimal float syntax and range diagnostic helper with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::float_with_underscores_optional` | check-prelude std-parse-basic Option-returning decimal float parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::float_with_underscores_or` | check-prelude std-parse-basic fallback decimal float parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::hex_integer` | check-prelude std-parse-basic Result-returning signed hexadecimal integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::hex_integer_optional` | check-prelude std-parse-basic Option-returning signed hexadecimal integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::hex_integer_or` | check-prelude std-parse-basic signed hexadecimal integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::hex_integer_with_underscores` | check-prelude std-parse-basic Result-returning signed hexadecimal integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::hex_integer_with_underscores_optional` | check-prelude std-parse-basic Option-returning signed hexadecimal integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::hex_integer_with_underscores_or` | check-prelude std-parse-basic signed hexadecimal integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::integer` | check-prelude std-parse-basic Result-returning ASCII-trimmed signed integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_error` | check-prelude std-parse-basic signed integer parse diagnostic helper; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_optional` | check-prelude std-parse-basic Option-returning signed integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_or` | check-prelude std-parse-basic signed integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix` | check-prelude std-parse-basic Result-returning signed radix integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_error` | check-prelude std-parse-basic signed radix integer parse diagnostic helper; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_optional` | check-prelude std-parse-basic Option-returning signed radix integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_or` | check-prelude std-parse-basic signed radix integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_with_underscores` | check-prelude std-parse-basic Result-returning signed radix integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_with_underscores_error` | check-prelude std-parse-basic signed radix integer parse diagnostic helper with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_with_underscores_optional` | check-prelude std-parse-basic Option-returning signed radix integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_radix_with_underscores_or` | check-prelude std-parse-basic signed radix integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_with_underscores` | check-prelude std-parse-basic Result-returning signed integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_with_underscores_error` | check-prelude std-parse-basic signed integer parse diagnostic helper with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_with_underscores_optional` | check-prelude std-parse-basic Option-returning signed integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::integer_with_underscores_or` | check-prelude std-parse-basic signed integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::is_binary_integer` | check-prelude std-parse-basic signed binary integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_binary_integer_with_underscores` | check-prelude std-parse-basic signed binary integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_boolean` | check-prelude std-parse-basic lowercase bool parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_float` | check-prelude std-parse-basic decimal float validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_float_with_underscores` | check-prelude std-parse-basic decimal float validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_hex_integer` | check-prelude std-parse-basic signed hexadecimal integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_hex_integer_with_underscores` | check-prelude std-parse-basic signed hexadecimal integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_integer` | check-prelude std-parse-basic signed integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_integer_radix` | check-prelude std-parse-basic signed radix integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_integer_radix_with_underscores` | check-prelude std-parse-basic signed radix integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_integer_with_underscores` | check-prelude std-parse-basic signed integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_octal_integer` | check-prelude std-parse-basic signed octal integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_octal_integer_with_underscores` | check-prelude std-parse-basic signed octal integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_parse[T: std::parse::Parse]` | check-prelude std-parse-basic trait-backed typed parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_unsigned` | check-prelude std-parse-basic unsigned integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_unsigned_radix` | check-prelude std-parse-basic unsigned radix integer parser validator; docs/stdlib/modules/parse.md |
| `fn std::parse::is_unsigned_radix_with_underscores` | check-prelude std-parse-basic unsigned radix integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::is_unsigned_with_underscores` | check-prelude std-parse-basic unsigned integer parser validator with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::octal_integer` | check-prelude std-parse-basic Result-returning signed octal integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::octal_integer_optional` | check-prelude std-parse-basic Option-returning signed octal integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::octal_integer_or` | check-prelude std-parse-basic signed octal integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::octal_integer_with_underscores` | check-prelude std-parse-basic Result-returning signed octal integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::octal_integer_with_underscores_optional` | check-prelude std-parse-basic Option-returning signed octal integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::octal_integer_with_underscores_or` | check-prelude std-parse-basic signed octal integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::parse[T: std::parse::Parse]` | check-prelude std-parse-basic trait-backed typed parser dispatch; docs/stdlib/modules/parse.md |
| `fn std::parse::parse_or[T: std::parse::Parse]` | check-prelude std-parse-basic trait-backed typed parser fallback dispatch; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned` | check-prelude std-parse-basic Result-returning unsigned decimal integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_error` | check-prelude std-parse-basic unsigned integer parse diagnostic helper; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_optional` | check-prelude std-parse-basic Option-returning unsigned integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_or` | check-prelude std-parse-basic unsigned decimal integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix` | check-prelude std-parse-basic Result-returning unsigned radix integer parser; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_error` | check-prelude std-parse-basic unsigned radix integer parse diagnostic helper; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_optional` | check-prelude std-parse-basic Option-returning unsigned radix integer compatibility parser; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_or` | check-prelude std-parse-basic unsigned radix integer parser with fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_with_underscores` | check-prelude std-parse-basic Result-returning unsigned radix integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_with_underscores_error` | check-prelude std-parse-basic unsigned radix integer parse diagnostic helper with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_with_underscores_optional` | check-prelude std-parse-basic Option-returning unsigned radix integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_radix_with_underscores_or` | check-prelude std-parse-basic unsigned radix integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_with_underscores` | check-prelude std-parse-basic Result-returning unsigned decimal integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_with_underscores_error` | check-prelude std-parse-basic unsigned integer parse diagnostic helper with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_with_underscores_optional` | check-prelude std-parse-basic Option-returning unsigned integer parser with digit separators; docs/stdlib/modules/parse.md |
| `fn std::parse::unsigned_with_underscores_or` | check-prelude std-parse-basic unsigned decimal integer parser with digit separators and fallback; docs/stdlib/modules/parse.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::parse::ParseError::is_empty_input` | check-prelude std-parse-basic parse diagnostic empty-input predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_expected_digit` | check-prelude std-parse-basic parse diagnostic expected-digit predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_invalid_digit` | check-prelude std-parse-basic parse diagnostic invalid-digit predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_invalid_radix` | check-prelude std-parse-basic parse diagnostic invalid-radix predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_invalid_separator` | check-prelude std-parse-basic parse diagnostic invalid-separator predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_invalid_sign` | check-prelude std-parse-basic parse diagnostic invalid-sign predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_overflow` | check-prelude std-parse-basic parse diagnostic overflow predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::is_underflow` | check-prelude std-parse-basic parse diagnostic underflow predicate; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::kind` | check-prelude std-parse-basic parse diagnostic kind accessor; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::message` | check-prelude std-parse-basic parse diagnostic explanation text; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::name` | check-prelude std-parse-basic parse diagnostic stable label; docs/stdlib/modules/parse.md |
| `method std::parse::ParseError::offset` | check-prelude std-parse-basic parse diagnostic offset accessor; docs/stdlib/modules/parse.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::parse` | check-prelude std-parse-basic whole-input parser helpers; docs/stdlib/modules/parse.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::parse::ParseError` | check-prelude std-parse-basic integer parse diagnostic value; docs/stdlib/modules/parse.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::parse::Parse` | check-prelude std-parse-basic typed parser dispatch trait; docs/stdlib/modules/parse.md |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::parse::Parse::is_parse` | check-prelude std-parse-basic typed parser validator method; docs/stdlib/modules/parse.md |
| `trait-method std::parse::Parse::parse` | check-prelude std-parse-basic typed parser strict method; docs/stdlib/modules/parse.md |
| `trait-method std::parse::Parse::parse_or` | check-prelude std-parse-basic typed parser fallback method; docs/stdlib/modules/parse.md |

## `std::path`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::path::ComponentKind` | check-prelude std-path-components kinded path component categories; docs/stdlib/modules/path.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::path::as_bytes` | check-prelude std-path-buf borrowed path-byte accessor helper; docs/stdlib/modules/path.md |
| `fn std::path::bytes` | check-prelude std-path-bytes typed borrowed path-byte view constructor; docs/stdlib/modules/path.md |
| `fn std::path::components` | check-prelude std-path-components borrowed lexical component iterator; docs/stdlib/modules/path.md |
| `fn std::path::components_with_kinds` | check-prelude std-path-components kinded lexical path component iterator; docs/stdlib/modules/path.md |
| `fn std::path::contains_nul` | check-prelude std-path-buf POSIX path NUL-byte predicate; docs/stdlib/modules/path.md |
| `fn std::path::current_dir_join` | check-prelude std-path-buf Result-returning current-directory path join helper; docs/stdlib/modules/path.md |
| `fn std::path::ends_with` | check-prelude std-path-affixes component-aware suffix predicate; docs/stdlib/modules/path.md |
| `fn std::path::extension` | check-prelude std-path-basic borrowed final-extension view; docs/stdlib/modules/path.md |
| `fn std::path::file_name` | check-prelude std-path-basic borrowed final-component view; docs/stdlib/modules/path.md |
| `fn std::path::file_stem` | check-prelude std-path-basic natural alias for borrowed file-stem view; docs/stdlib/modules/path.md |
| `fn std::path::from_bytes` | check-prelude std-path-buf owned path buffer constructor from bytes; docs/stdlib/modules/path.md |
| `fn std::path::from_os` | check-prelude std-path-bytes path-byte view from OS-string bytes; docs/stdlib/modules/path.md |
| `fn std::path::from_string` | check-prelude std-path-buf owned path buffer constructor from host string; docs/stdlib/modules/path.md |
| `fn std::path::has_extension` | check-prelude std-path-predicates final-extension predicate; docs/stdlib/modules/path.md |
| `fn std::path::has_file_name` | check-prelude std-path-predicates final-component predicate; docs/stdlib/modules/path.md |
| `fn std::path::has_file_stem` | check-prelude std-path-predicates natural alias for file-stem predicate; docs/stdlib/modules/path.md |
| `fn std::path::has_stem` | check-prelude std-path-predicates file-stem predicate; docs/stdlib/modules/path.md |
| `fn std::path::has_windows_drive_prefix` | check-prelude std-path-windows-lexical Windows drive-prefix byte classifier; docs/stdlib/modules/path.md |
| `fn std::path::is_absolute` | check-prelude std-path-basic POSIX absolute path predicate; docs/stdlib/modules/path.md |
| `fn std::path::is_empty` | check-prelude std-path-buf borrowed path empty predicate; docs/stdlib/modules/path.md |
| `fn std::path::is_relative` | check-prelude std-path-basic POSIX relative path predicate; docs/stdlib/modules/path.md |
| `fn std::path::is_separator` | check-prelude std-path-basic POSIX slash separator predicate; docs/stdlib/modules/path.md |
| `fn std::path::is_windows_absolute` | check-prelude std-path-windows-lexical Windows rooted/drive/UNC absolute byte classifier; docs/stdlib/modules/path.md |
| `fn std::path::is_windows_drive_absolute` | check-prelude std-path-windows-lexical Windows drive-rooted byte classifier; docs/stdlib/modules/path.md |
| `fn std::path::is_windows_drive_relative` | check-prelude std-path-windows-lexical Windows drive-relative byte classifier; docs/stdlib/modules/path.md |
| `fn std::path::is_windows_separator` | check-prelude std-path-windows-lexical Windows slash/backslash byte separator predicate; docs/stdlib/modules/path.md |
| `fn std::path::is_windows_unc` | check-prelude std-path-windows-lexical Windows UNC byte classifier; docs/stdlib/modules/path.md |
| `fn std::path::join` | check-prelude std-path-buf owned lexical path join helper; docs/stdlib/modules/path.md |
| `fn std::path::join_in` | check-prelude std-path-basic zone-backed lexical join helper; docs/stdlib/modules/path.md |
| `fn std::path::join_many` | check-prelude std-path-buf owned lexical multi-part path join helper; docs/stdlib/modules/path.md |
| `fn std::path::normalize_in` | check-prelude std-path-basic lightweight lexical normalization helper; docs/stdlib/modules/path.md |
| `fn std::path::parent` | check-prelude std-path-basic borrowed parent path view; docs/stdlib/modules/path.md |
| `fn std::path::starts_with` | check-prelude std-path-affixes component-aware prefix predicate; docs/stdlib/modules/path.md |
| `fn std::path::stem` | check-prelude std-path-basic borrowed file-stem view; docs/stdlib/modules/path.md |
| `fn std::path::strip_prefix` | check-prelude std-path-affixes borrowed component-aware prefix removal; docs/stdlib/modules/path.md |
| `fn std::path::strip_suffix` | check-prelude std-path-affixes borrowed component-aware suffix removal; docs/stdlib/modules/path.md |
| `fn std::path::to_string` | check-prelude std-path-buf owned byte-string copy from path view; docs/stdlib/modules/path.md |
| `fn std::path::trim_trailing_separators` | check-prelude std-path-basic borrowed trailing slash trim helper; docs/stdlib/modules/path.md |
| `fn std::path::windows_drive` | check-prelude std-path-windows-lexical borrowed Windows drive-prefix byte view helper; docs/stdlib/modules/path.md |
| `fn std::path::windows_unc_prefix` | check-prelude std-path-windows-lexical borrowed Windows UNC prefix byte view helper; docs/stdlib/modules/path.md |
| `fn std::path::with_extension_in` | check-prelude std-path-edit zone-backed lexical extension replacement helper; docs/stdlib/modules/path.md |
| `fn std::path::with_file_name_in` | check-prelude std-path-edit zone-backed lexical final-component replacement helper; docs/stdlib/modules/path.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::path::Component::as_slice` | check-prelude std-path-components borrowed bytes for kinded path component; docs/stdlib/modules/path.md |
| `method std::path::Component::is_current` | check-prelude std-path-components current-directory component predicate; docs/stdlib/modules/path.md |
| `method std::path::Component::is_normal` | check-prelude std-path-components normal component predicate; docs/stdlib/modules/path.md |
| `method std::path::Component::is_parent` | check-prelude std-path-components parent-directory component predicate; docs/stdlib/modules/path.md |
| `method std::path::Component::is_root` | check-prelude std-path-components root component predicate; docs/stdlib/modules/path.md |
| `method std::path::Component::kind` | check-prelude std-path-components kinded path component classifier accessor; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::as_bytes` | check-prelude std-path-buf owned path byte accessor; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::as_path` | check-prelude std-path-buf borrowed view from owned path buffer; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::as_string` | check-prelude std-path-buf borrowed internal byte-string view from owned path buffer; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::components` | check-prelude std-path-buf owned path component iterator wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::components_with_kinds` | check-prelude std-path-components owned path kinded component iterator wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::contains_nul` | check-prelude std-path-buf owned path NUL-byte predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::extension` | check-prelude std-path-buf owned path extension wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::file_name` | check-prelude std-path-buf owned path file-name wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::file_stem` | check-prelude std-path-buf owned path file-stem wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::has_windows_drive_prefix` | check-prelude std-path-windows-lexical owned path Windows drive-prefix wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_absolute` | check-prelude std-path-buf owned path absolute predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_empty` | check-prelude std-path-buf owned path empty predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_relative` | check-prelude std-path-buf owned path relative predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_windows_absolute` | check-prelude std-path-windows-lexical owned path Windows absolute wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_windows_drive_absolute` | check-prelude std-path-windows-lexical owned path Windows drive-rooted wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_windows_drive_relative` | check-prelude std-path-windows-lexical owned path Windows drive-relative wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::is_windows_unc` | check-prelude std-path-windows-lexical owned path Windows UNC wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::join` | check-prelude std-path-buf owned path lexical join method; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::len` | check-prelude std-path-buf owned path byte length accessor; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::normalize` | check-prelude std-path-buf owned path normalization method; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::parent` | check-prelude std-path-buf owned path parent wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::stem` | check-prelude std-path-buf owned path stem wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::to_string` | check-prelude std-path-buf owned path byte-string copy helper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::windows_drive` | check-prelude std-path-windows-lexical owned path borrowed Windows drive-prefix wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::windows_unc_prefix` | check-prelude std-path-windows-lexical owned path borrowed Windows UNC-prefix wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::with_extension` | check-prelude std-path-buf owned path extension replacement helper; docs/stdlib/modules/path.md |
| `method std::path::PathBuf::with_file_name` | check-prelude std-path-buf owned path final-component replacement helper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::as_bytes` | check-prelude std-path-buf typed path byte accessor alias; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::as_slice` | check-prelude std-path-bytes borrowed path-byte view accessor; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::components` | check-prelude std-path-bytes typed path component iterator wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::components_with_kinds` | check-prelude std-path-components typed path kinded component iterator wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::contains_nul` | check-prelude std-path-buf typed path NUL-byte predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::ends_with` | check-prelude std-path-affixes typed path suffix predicate wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::extension` | check-prelude std-path-bytes typed path extension wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::file_name` | check-prelude std-path-bytes typed path file-name wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::file_stem` | check-prelude std-path-bytes natural typed path file-stem wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::has_extension` | check-prelude std-path-predicates typed path extension predicate wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::has_file_name` | check-prelude std-path-predicates typed path file-name predicate wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::has_file_stem` | check-prelude std-path-predicates natural typed path file-stem predicate wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::has_stem` | check-prelude std-path-predicates typed path stem predicate wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::has_windows_drive_prefix` | check-prelude std-path-windows-lexical typed path Windows drive-prefix wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_absolute` | check-prelude std-path-bytes typed path absolute predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_empty` | check-prelude std-path-bytes typed path empty predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_relative` | check-prelude std-path-bytes typed path relative predicate; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_windows_absolute` | check-prelude std-path-windows-lexical typed path Windows absolute wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_windows_drive_absolute` | check-prelude std-path-windows-lexical typed path Windows drive-rooted wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_windows_drive_relative` | check-prelude std-path-windows-lexical typed path Windows drive-relative wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::is_windows_unc` | check-prelude std-path-windows-lexical typed path Windows UNC wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::join` | check-prelude std-path-buf typed owned lexical join helper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::join_in` | check-prelude std-path-bytes typed path lexical join wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::len` | check-prelude std-path-bytes typed path length accessor; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::normalize` | check-prelude std-path-buf typed owned normalization helper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::normalize_in` | check-prelude std-path-bytes typed path normalization wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::parent` | check-prelude std-path-bytes typed path parent wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::starts_with` | check-prelude std-path-affixes typed path prefix predicate wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::stem` | check-prelude std-path-bytes typed path stem wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::strip_prefix` | check-prelude std-path-affixes typed path prefix stripping wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::strip_suffix` | check-prelude std-path-affixes typed path suffix stripping wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::to_string` | check-prelude std-path-buf typed owned byte-string copy helper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::trim_trailing_separators` | check-prelude std-path-bytes typed path trailing separator trim wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::windows_drive` | check-prelude std-path-windows-lexical typed path borrowed Windows drive-prefix wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::windows_unc_prefix` | check-prelude std-path-windows-lexical typed path borrowed Windows UNC-prefix wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::with_extension_in` | check-prelude std-path-edit typed path extension replacement wrapper; docs/stdlib/modules/path.md |
| `method std::path::PathBytes::with_file_name_in` | check-prelude std-path-edit typed path final-component replacement wrapper; docs/stdlib/modules/path.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::path` | check-prelude std-path-basic source lexical path helpers; docs/stdlib/modules/path.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::path::Component` | check-prelude std-path-components borrowed kinded path component handle; docs/stdlib/modules/path.md |
| `struct std::path::Components` | check-prelude std-path-components borrowed lexical path component iterator; docs/stdlib/modules/path.md |
| `struct std::path::ComponentsWithKinds` | check-prelude std-path-components borrowed kinded path component iterator; docs/stdlib/modules/path.md |
| `struct std::path::PathBuf` | check-prelude std-path-buf distinct owned POSIX path byte buffer; docs/stdlib/modules/path.md |
| `struct std::path::PathBytes` | check-prelude std-path-bytes typed borrowed path-byte view; docs/stdlib/modules/path.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::path::Path` | check-prelude std-path-buf readability alias for borrowed path bytes; docs/stdlib/modules/path.md |

## `std::process`

Tier: `hosted`. Stability reading: platform-backed.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::process::abort` | check-prelude std-process-abort explicit process abort hook; docs/stdlib/modules/process.md |
| `fn std::process::arg` | check-prelude std-process-command C-argv argument wrapper for Command; docs/stdlib/modules/process.md |
| `fn std::process::arg_bytes` | check-prelude std-process-high-level Result-returning C-argv argument wrapper for owned byte text; docs/stdlib/modules/process.md |
| `fn std::process::command` | check-prelude std-process-command natural Command constructor; docs/stdlib/modules/process.md |
| `fn std::process::command_with_args` | check-prelude std-process-command Command constructor with argv slice; docs/stdlib/modules/process.md |
| `fn std::process::current_dir` | check-prelude std-process-basic Result-returning process-facing current directory wrapper; docs/stdlib/modules/process.md |
| `fn std::process::current_dir_optional` | check-prelude std-process-basic Option-returning process-facing current directory wrapper; docs/stdlib/modules/process.md |
| `fn std::process::current_dir_or_default` | check-prelude std-process-basic empty-string process-facing current directory compatibility wrapper; docs/stdlib/modules/process.md |
| `fn std::process::env_var` | check-prelude std-process-command child environment assignment wrapper; docs/stdlib/modules/process.md |
| `fn std::process::env_var_bytes` | check-prelude std-process-high-level Result-returning child environment assignment wrapper for owned byte text; docs/stdlib/modules/process.md |
| `fn std::process::exec` | check-prelude std-process-command module-level Command exec wrapper returning Error; docs/stdlib/modules/process.md |
| `fn std::process::executable_path` | check-prelude std-process-basic Result-returning process-facing executable path wrapper; docs/stdlib/modules/process.md |
| `fn std::process::executable_path_optional` | check-prelude std-process-basic Option-returning process-facing executable path wrapper; docs/stdlib/modules/process.md |
| `fn std::process::executable_path_or_default` | check-prelude std-process-basic empty-string process-facing executable path compatibility wrapper; docs/stdlib/modules/process.md |
| `fn std::process::exit` | check-prelude std-process-exit explicit process exit hook; docs/stdlib/modules/process.md |
| `fn std::process::exit_code` | check-prelude std-process-high-level typed exit code constructor; docs/stdlib/modules/process.md |
| `fn std::process::exit_status` | check-prelude std-process-command module-level Command typed status wrapper returning Error; docs/stdlib/modules/process.md |
| `fn std::process::failure` | check-prelude std-process-basic source failure status helper; docs/stdlib/modules/process.md |
| `fn std::process::failure_code` | check-prelude std-process-high-level typed failure exit code helper; docs/stdlib/modules/process.md |
| `fn std::process::fork` | check-prelude std-process-result POSIX fork direct Error result helper; docs/stdlib/modules/process.md |
| `fn std::process::fork_raw` | check-prelude std-process-fork-wait POSIX fork raw compatibility hook; docs/stdlib/modules/process.md |
| `fn std::process::gid` | check-prelude std-process-identity current process group id hook; docs/stdlib/modules/process.md |
| `fn std::process::id` | check-prelude std-process-basic current process id hook; docs/stdlib/modules/process.md |
| `fn std::process::is_child` | check-prelude std-process-fork-wait source child-branch predicate; docs/stdlib/modules/process.md |
| `fn std::process::is_failure` | check-prelude std-process-basic source status predicate; docs/stdlib/modules/process.md |
| `fn std::process::is_fork_error` | check-prelude std-process-fork-wait source fork failure predicate; docs/stdlib/modules/process.md |
| `fn std::process::is_parent` | check-prelude std-process-fork-wait source parent-branch predicate; docs/stdlib/modules/process.md |
| `fn std::process::is_root` | check-prelude std-process-identity source root-user predicate; docs/stdlib/modules/process.md |
| `fn std::process::is_success` | check-prelude std-process-basic source status predicate; docs/stdlib/modules/process.md |
| `fn std::process::is_wait_error` | check-prelude std-process-fork-wait source wait failure predicate; docs/stdlib/modules/process.md |
| `fn std::process::kill` | check-prelude std-process-command POSIX signal helper returning Error on failure; docs/stdlib/modules/process.md |
| `fn std::process::kill_signal` | check-prelude std-process-high-level typed signal send helper; docs/stdlib/modules/process.md |
| `fn std::process::output` | check-prelude std-process-high-level natural Command output capture wrapper; docs/stdlib/modules/process.md |
| `fn std::process::output_in` | check-prelude std-process-output module-level Command output capture wrapper returning Error; docs/stdlib/modules/process.md |
| `fn std::process::sig_check` | check-prelude std-process-high-level POSIX signal-zero check helper; docs/stdlib/modules/process.md |
| `fn std::process::sighup` | check-prelude std-process-high-level POSIX SIGHUP typed signal helper; docs/stdlib/modules/process.md |
| `fn std::process::sigint` | check-prelude std-process-high-level POSIX SIGINT typed signal helper; docs/stdlib/modules/process.md |
| `fn std::process::sigkill` | check-prelude std-process-high-level POSIX SIGKILL typed signal helper; docs/stdlib/modules/process.md |
| `fn std::process::signal` | check-prelude std-process-high-level typed signal constructor; docs/stdlib/modules/process.md |
| `fn std::process::sigquit` | check-prelude std-process-high-level POSIX SIGQUIT typed signal helper; docs/stdlib/modules/process.md |
| `fn std::process::sigterm` | check-prelude std-process-high-level POSIX SIGTERM typed signal helper; docs/stdlib/modules/process.md |
| `fn std::process::spawn` | check-prelude std-process-command module-level Command spawn wrapper returning Error; docs/stdlib/modules/process.md |
| `fn std::process::status` | check-prelude std-process-command module-level Command typed status wrapper returning Error; docs/stdlib/modules/process.md |
| `fn std::process::status_code` | check-prelude std-process-command explicit normal-exit-code compatibility wrapper returning Error; docs/stdlib/modules/process.md |
| `fn std::process::status_with_stdin` | check-prelude std-process-stdin module-level bounded pipe-backed stdin status helper; docs/stdlib/modules/process.md |
| `fn std::process::status_with_stdin_string` | check-prelude std-process-stdin module-level string stdin status helper; docs/stdlib/modules/process.md |
| `fn std::process::success` | check-prelude std-process-basic source success status helper; docs/stdlib/modules/process.md |
| `fn std::process::success_code` | check-prelude std-process-high-level typed success exit code helper; docs/stdlib/modules/process.md |
| `fn std::process::temp_dir` | check-prelude std-process-high-level default temp directory constructor; docs/stdlib/modules/process.md |
| `fn std::process::temp_dir_in` | check-prelude std-process-high-level prefixed temp directory constructor; docs/stdlib/modules/process.md |
| `fn std::process::temp_file` | check-prelude std-process-high-level default temp file constructor; docs/stdlib/modules/process.md |
| `fn std::process::temp_file_in` | check-prelude std-process-high-level prefixed temp file constructor; docs/stdlib/modules/process.md |
| `fn std::process::terminate` | check-prelude std-process-command SIGTERM convenience helper; docs/stdlib/modules/process.md |
| `fn std::process::try_current_dir` | check-prelude std-process-high-level optional current directory wrapper; docs/stdlib/modules/process.md |
| `fn std::process::try_executable_path` | check-prelude std-process-high-level optional executable path wrapper; docs/stdlib/modules/process.md |
| `fn std::process::uid` | check-prelude std-process-identity current process user id hook; docs/stdlib/modules/process.md |
| `fn std::process::wait` | check-prelude std-process-result POSIX wait direct Error result helper; docs/stdlib/modules/process.md |
| `fn std::process::wait_raw` | check-prelude std-process-fork-wait POSIX wait raw compatibility hook; docs/stdlib/modules/process.md |
| `fn std::process::wait_status` | check-prelude std-process-exit-status POSIX wait typed ExitStatus helper; docs/stdlib/modules/process.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::process::Child::kill` | check-prelude std-process-command child signal helper; docs/stdlib/modules/process.md |
| `method std::process::Child::pid` | check-prelude std-process-command child pid accessor; docs/stdlib/modules/process.md |
| `method std::process::Child::signal` | check-prelude std-process-high-level typed child signal helper; docs/stdlib/modules/process.md |
| `method std::process::Child::terminate` | check-prelude std-process-command child SIGTERM convenience helper; docs/stdlib/modules/process.md |
| `method std::process::Child::wait` | check-prelude std-process-command child wait Result helper; docs/stdlib/modules/process.md |
| `method std::process::Child::wait_status` | check-prelude std-process-exit-status child typed ExitStatus wait helper; docs/stdlib/modules/process.md |
| `method std::process::Command::arg` | check-prelude std-process-high-level explicit-zone single argv append helper; docs/stdlib/modules/process.md |
| `method std::process::Command::arg_bytes` | check-prelude std-process-high-level Result-returning explicit-zone argv append helper for owned byte text; docs/stdlib/modules/process.md |
| `method std::process::Command::arg_value` | check-prelude std-process-high-level explicit-zone Arg append helper; docs/stdlib/modules/process.md |
| `method std::process::Command::args` | check-prelude std-process-command replace command argument slice; docs/stdlib/modules/process.md |
| `method std::process::Command::clear_env` | check-prelude std-process-high-level child environment clearing policy helper; docs/stdlib/modules/process.md |
| `method std::process::Command::current_dir` | check-prelude std-process-command child working-directory setup; docs/stdlib/modules/process.md |
| `method std::process::Command::current_dir_bytes` | check-prelude std-process-high-level Result-returning child working-directory setup from owned byte text; docs/stdlib/modules/process.md |
| `method std::process::Command::current_dir_path` | check-prelude std-process-high-level Result-returning child working-directory setup from PathBytes; docs/stdlib/modules/process.md |
| `method std::process::Command::env` | check-prelude std-process-command explicit-zone single environment append helper; docs/stdlib/modules/process.md |
| `method std::process::Command::env_bytes` | check-prelude std-process-high-level Result-returning environment append helper for owned byte text; docs/stdlib/modules/process.md |
| `method std::process::Command::env_value` | check-prelude std-process-high-level explicit-zone EnvVar append helper; docs/stdlib/modules/process.md |
| `method std::process::Command::env_values` | check-prelude std-process-command replace child environment assignment slice; docs/stdlib/modules/process.md |
| `method std::process::Command::env_var` | check-prelude std-process-high-level explicit-zone environment append compatibility helper; docs/stdlib/modules/process.md |
| `method std::process::Command::exec` | check-prelude std-process-command replace current process with command; docs/stdlib/modules/process.md |
| `method std::process::Command::exit_status` | check-prelude std-process-exit-status spawn and wait for typed ExitStatus; docs/stdlib/modules/process.md |
| `method std::process::Command::inherit_env` | check-prelude std-process-high-level child environment inheritance policy helper; docs/stdlib/modules/process.md |
| `method std::process::Command::new` | check-prelude std-process-command associated Command constructor; docs/stdlib/modules/process.md |
| `method std::process::Command::output` | check-prelude std-process-high-level natural output capture alias; docs/stdlib/modules/process.md |
| `method std::process::Command::output_in` | check-prelude std-process-output zone-backed stdout/stderr capture helper; docs/stdlib/modules/process.md |
| `method std::process::Command::spawn` | check-prelude std-process-command spawn child process handle; docs/stdlib/modules/process.md |
| `method std::process::Command::spawn_with_stdin_file` | check-prelude std-process-stdin spawn child process with file-backed stdin; docs/stdlib/modules/process.md |
| `method std::process::Command::spawn_with_stdin_null` | check-prelude std-process-stdin spawn child process with /dev/null stdin; docs/stdlib/modules/process.md |
| `method std::process::Command::status` | check-prelude std-process-command spawn and wait for typed exit status; docs/stdlib/modules/process.md |
| `method std::process::Command::status_code` | check-prelude std-process-command explicit normal-exit-code compatibility helper; docs/stdlib/modules/process.md |
| `method std::process::Command::status_with_stdin` | check-prelude std-process-stdin bounded pipe-backed stdin status helper; docs/stdlib/modules/process.md |
| `method std::process::Command::status_with_stdin_file` | check-prelude std-process-stdin child status with file-backed stdin; docs/stdlib/modules/process.md |
| `method std::process::Command::status_with_stdin_file_bytes` | check-prelude std-process-stdin Result-returning child status with owned byte path stdin; docs/stdlib/modules/process.md |
| `method std::process::Command::status_with_stdin_file_path` | check-prelude std-process-stdin Result-returning child status with PathBytes stdin; docs/stdlib/modules/process.md |
| `method std::process::Command::status_with_stdin_null` | check-prelude std-process-stdin child status with /dev/null stdin; docs/stdlib/modules/process.md |
| `method std::process::Command::status_with_stdin_string` | check-prelude std-process-stdin string stdin status helper; docs/stdlib/modules/process.md |
| `method std::process::Command::with_arg` | check-prelude std-process-high-level by-value chainable single argv append helper; docs/stdlib/modules/process.md |
| `method std::process::Command::with_args` | check-prelude std-process-command associated Command constructor with argv slice; docs/stdlib/modules/process.md |
| `method std::process::Command::with_clear_env` | check-prelude std-process-high-level by-value child environment clearing policy helper; docs/stdlib/modules/process.md |
| `method std::process::Command::with_current_dir` | check-prelude std-process-high-level by-value chainable child working-directory setup helper; docs/stdlib/modules/process.md |
| `method std::process::Command::with_env` | check-prelude std-process-high-level by-value chainable environment append helper; docs/stdlib/modules/process.md |
| `method std::process::Command::with_inherit_env` | check-prelude std-process-high-level by-value child environment inheritance policy helper; docs/stdlib/modules/process.md |
| `method std::process::ExitCode::code` | check-prelude std-process-high-level typed exit code accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitCode::exit` | check-prelude std-process-high-level typed process exit helper; docs/stdlib/modules/process.md |
| `method std::process::ExitCode::is_failure` | check-prelude std-process-high-level typed failure predicate; docs/stdlib/modules/process.md |
| `method std::process::ExitCode::is_success` | check-prelude std-process-high-level typed success predicate; docs/stdlib/modules/process.md |
| `method std::process::ExitCode::raw` | check-prelude std-process-high-level raw exit code accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::code` | check-prelude std-process-exit-status optional normal exit code accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::code_or` | check-prelude std-process-exit-status normal exit code fallback accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::exit_code` | check-prelude std-process-high-level typed ExitCode extraction from status; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::exited` | check-prelude std-process-exit-status normal exit predicate; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::is_failure` | check-prelude std-process-exit-status typed exit failure predicate; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::is_success` | check-prelude std-process-exit-status typed exit success predicate; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::raw` | check-prelude std-process-exit-status raw hosted wait-status accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::signal` | check-prelude std-process-exit-status optional signal number accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::signal_or` | check-prelude std-process-exit-status signal fallback accessor; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::signaled` | check-prelude std-process-exit-status signal termination predicate; docs/stdlib/modules/process.md |
| `method std::process::ExitStatus::success` | check-prelude std-process-exit-status natural success predicate alias; docs/stdlib/modules/process.md |
| `method std::process::Output::exit_status` | check-prelude std-process-exit-status captured typed ExitStatus accessor; docs/stdlib/modules/process.md |
| `method std::process::Output::is_success` | check-prelude std-process-output captured exit-success predicate; docs/stdlib/modules/process.md |
| `method std::process::Output::status` | check-prelude std-process-output captured exit-status accessor; docs/stdlib/modules/process.md |
| `method std::process::Output::stderr` | check-prelude std-process-output captured stderr byte view; docs/stdlib/modules/process.md |
| `method std::process::Output::stderr_string` | check-prelude std-process-output UTF-8 stderr copy returning Error on invalid data; docs/stdlib/modules/process.md |
| `method std::process::Output::stdout` | check-prelude std-process-output captured stdout byte view; docs/stdlib/modules/process.md |
| `method std::process::Output::stdout_string` | check-prelude std-process-output UTF-8 stdout copy returning Error on invalid data; docs/stdlib/modules/process.md |
| `method std::process::Signal::is_check` | check-prelude std-process-high-level signal-zero predicate; docs/stdlib/modules/process.md |
| `method std::process::Signal::raw` | check-prelude std-process-high-level raw signal accessor; docs/stdlib/modules/process.md |
| `method std::process::TempDir::as_c_str` | check-prelude std-process-high-level temp directory C path view; docs/stdlib/modules/process.md |
| `method std::process::TempDir::path` | check-prelude std-process-high-level temp directory byte path view; docs/stdlib/modules/process.md |
| `method std::process::TempDir::remove` | check-prelude std-process-high-level temp directory removal helper; docs/stdlib/modules/process.md |
| `method std::process::TempFile::as_c_str` | check-prelude std-process-high-level temp file C path view; docs/stdlib/modules/process.md |
| `method std::process::TempFile::as_fd` | check-prelude std-process-high-level temp file descriptor view; docs/stdlib/modules/process.md |
| `method std::process::TempFile::close` | check-prelude std-process-high-level Result-returning temp file close helper; docs/stdlib/modules/process.md |
| `method std::process::TempFile::close_and_remove` | check-prelude std-process-high-level temp file cleanup helper; docs/stdlib/modules/process.md |
| `method std::process::TempFile::close_bool` | check-prelude std-process-high-level bool compatibility temp file close helper; docs/stdlib/modules/process.md |
| `method std::process::TempFile::is_open` | check-prelude std-process-high-level temp file open-state predicate; docs/stdlib/modules/process.md |
| `method std::process::TempFile::path` | check-prelude std-process-high-level temp file byte path view; docs/stdlib/modules/process.md |
| `method std::process::TempFile::remove` | check-prelude std-process-high-level temp file unlink helper; docs/stdlib/modules/process.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::process` | std process current-process and POSIX fork/wait tests; docs/stdlib/modules/process.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::process::Arg` | check-prelude std-process-command borrowed C argv argument wrapper; docs/stdlib/modules/process.md |
| `struct std::process::Child` | check-prelude std-process-command child process handle; docs/stdlib/modules/process.md |
| `struct std::process::Command` | check-prelude std-process-command process command builder; docs/stdlib/modules/process.md |
| `struct std::process::EnvVar` | check-prelude std-process-command child environment assignment wrapper; docs/stdlib/modules/process.md |
| `struct std::process::ExitCode` | check-prelude std-process-high-level typed process exit code value; docs/stdlib/modules/process.md |
| `struct std::process::ExitStatus` | check-prelude std-process-exit-status typed child exit status value; docs/stdlib/modules/process.md |
| `struct std::process::Output` | check-prelude std-process-output captured child output handle; docs/stdlib/modules/process.md |
| `struct std::process::Signal` | check-prelude std-process-high-level typed POSIX signal value; docs/stdlib/modules/process.md |
| `struct std::process::TempDir` | check-prelude std-process-high-level temp directory handle; docs/stdlib/modules/process.md |
| `struct std::process::TempFile` | check-prelude std-process-high-level temp file handle; docs/stdlib/modules/process.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::process::ChildStderr` | check-prelude std-process-high-level child stderr pipe alias; docs/stdlib/modules/process.md |
| `type std::process::ChildStdin` | check-prelude std-process-high-level child stdin pipe alias; docs/stdlib/modules/process.md |
| `type std::process::ChildStdout` | check-prelude std-process-high-level child stdout pipe alias; docs/stdlib/modules/process.md |
| `type std::process::Error` | check-prelude std-error-integration shared process error alias; docs/stdlib/modules/process.md |
| `type std::process::ErrorKind` | check-prelude std-error-integration shared process error-kind alias; docs/stdlib/modules/process.md |

## `std::random`

Tier: `alloc/hosted`. Stability reading: usable with hosted entropy APIs.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::random::below` | check-prelude std-random-basic unbiased bounded non-crypto PRNG integer helper; docs/stdlib/modules/random.md |
| `fn std::random::boolean` | check-prelude std-random-basic deterministic PRNG boolean helper; docs/stdlib/modules/random.md |
| `fn std::random::entropy` | check-prelude std-random-result Error-returning OS entropy hook; docs/stdlib/modules/random.md |
| `fn std::random::entropy_raw` | check-prelude std-random-result raw Result OS entropy hook; docs/stdlib/modules/random.md |
| `fn std::random::entropy_unchecked` | check-prelude std-random-basic strict OS entropy hook compatibility helper; docs/stdlib/modules/random.md |
| `fn std::random::fill` | check-prelude std-random-result Error-returning OS entropy byte fill helper; docs/stdlib/modules/random.md |
| `fn std::random::fill_from` | check-prelude std-random-basic deterministic PRNG byte fill helper; docs/stdlib/modules/random.md |
| `fn std::random::fill_raw` | check-prelude std-random-result raw Result OS entropy byte fill helper; docs/stdlib/modules/random.md |
| `fn std::random::fill_unchecked` | check-prelude std-random-basic strict OS entropy byte fill compatibility helper; docs/stdlib/modules/random.md |
| `fn std::random::float` | check-prelude std-random-basic unit f64 PRNG helper; docs/stdlib/modules/random.md |
| `fn std::random::from_entropy` | check-prelude std-random-result Error-returning OS-seeded PRNG constructor; docs/stdlib/modules/random.md |
| `fn std::random::from_entropy_unchecked` | check-prelude std-random-basic strict OS-seeded PRNG constructor compatibility helper; docs/stdlib/modules/random.md |
| `fn std::random::next` | check-prelude std-random-basic raw PRNG word helper; docs/stdlib/modules/random.md |
| `fn std::random::range` | check-prelude std-random-basic unbiased bounded PRNG range helper; docs/stdlib/modules/random.md |
| `fn std::random::seed` | check-prelude std-random-basic deterministic PRNG seed constructor; docs/stdlib/modules/random.md |
| `fn std::random::seed_from_os` | check-prelude std-random-result Error-returning OS-seeded PRNG constructor alias; docs/stdlib/modules/random.md |
| `fn std::random::seed_from_os_unchecked` | check-prelude std-random-basic strict OS-seeded PRNG constructor alias; docs/stdlib/modules/random.md |
| `fn std::random::shuffle[T]` | check-prelude std-random-basic Fisher-Yates slice shuffle; docs/stdlib/modules/random.md |
| `fn std::random::try_below` | check-prelude std-random-try-bounds Option-returning unbiased bounded PRNG integer helper; docs/stdlib/modules/random.md |
| `fn std::random::try_range` | check-prelude std-random-try-bounds Option-returning unbiased PRNG range helper; docs/stdlib/modules/random.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::random::Prng::below` | check-prelude std-random-basic unbiased bounded non-crypto PRNG integer method; docs/stdlib/modules/random.md |
| `method std::random::Prng::boolean` | check-prelude std-random-basic deterministic PRNG boolean method; docs/stdlib/modules/random.md |
| `method std::random::Prng::fill` | check-prelude std-random-basic deterministic PRNG byte fill method; docs/stdlib/modules/random.md |
| `method std::random::Prng::float` | check-prelude std-random-basic unit f64 PRNG method; docs/stdlib/modules/random.md |
| `method std::random::Prng::from_entropy` | check-prelude std-random-result Error-returning OS-seeded PRNG associated constructor; docs/stdlib/modules/random.md |
| `method std::random::Prng::from_entropy_unchecked` | check-prelude std-random-basic strict OS-seeded PRNG associated constructor; docs/stdlib/modules/random.md |
| `method std::random::Prng::next` | check-prelude std-random-basic raw PRNG word method; docs/stdlib/modules/random.md |
| `method std::random::Prng::range` | check-prelude std-random-basic unbiased bounded PRNG range method; docs/stdlib/modules/random.md |
| `method std::random::Prng::seed` | check-prelude std-random-basic deterministic PRNG associated seed constructor; docs/stdlib/modules/random.md |
| `method std::random::Prng::seed_from_os` | check-prelude std-random-result Error-returning OS-seeded PRNG associated constructor alias; docs/stdlib/modules/random.md |
| `method std::random::Prng::seed_from_os_unchecked` | check-prelude std-random-basic strict OS-seeded PRNG associated constructor alias; docs/stdlib/modules/random.md |
| `method std::random::Prng::shuffle[T]` | check-prelude std-random-basic Fisher-Yates slice shuffle method; docs/stdlib/modules/random.md |
| `method std::random::Prng::try_below` | check-prelude std-random-try-bounds Option-returning unbiased bounded PRNG integer method; docs/stdlib/modules/random.md |
| `method std::random::Prng::try_range` | check-prelude std-random-try-bounds Option-returning unbiased PRNG range method; docs/stdlib/modules/random.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::random` | check-prelude std-random-basic OS entropy and deterministic non-crypto PRNG helpers; docs/stdlib/modules/random.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::random::Prng` | check-prelude std-random-basic deterministic non-cryptographic PRNG state; docs/stdlib/modules/random.md |

## `std::rc`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::rc::arc[T]` | check-prelude std-rc-arc-weak Arc constructor over zone-backed control block; docs/stdlib/modules/rc.md |
| `fn std::rc::rc[T]` | check-prelude std-rc-arc-weak Rc constructor over zone-backed control block; docs/stdlib/modules/rc.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::rc::Arc[T]::as_ref` | check-prelude std-rc-arc-weak Arc shared pointer accessor; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::clone` | check-prelude std-rc-arc-weak Arc strong-count clone; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::downgrade` | check-prelude std-rc-arc-weak Arc weak downgrade; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::get` | check-prelude std-rc-arc-weak Arc copied value read; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::is_unique` | check-prelude std-rc-arc-weak Arc uniqueness predicate; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::new` | check-prelude std-rc-arc-weak Arc associated constructor; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::ptr_eq` | check-prelude std-rc-arc-weak Arc control-pointer equality; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::strong_count` | check-prelude std-rc-arc-weak Arc strong count; docs/stdlib/modules/rc.md |
| `method std::rc::Arc[T]::weak_count` | check-prelude std-rc-arc-weak Arc weak count; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::as_ref` | check-prelude std-rc-arc-weak Rc shared pointer accessor; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::clone` | check-prelude std-rc-arc-weak Rc strong-count clone; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::downgrade` | check-prelude std-rc-arc-weak Rc weak downgrade; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::get` | check-prelude std-rc-arc-weak Rc copied value read; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::is_unique` | check-prelude std-rc-arc-weak Rc uniqueness predicate; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::new` | check-prelude std-rc-arc-weak Rc associated constructor; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::ptr_eq` | check-prelude std-rc-arc-weak Rc control-pointer equality; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::strong_count` | check-prelude std-rc-arc-weak Rc strong count; docs/stdlib/modules/rc.md |
| `method std::rc::Rc[T]::weak_count` | check-prelude std-rc-arc-weak Rc weak count; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::clone` | check-prelude std-rc-arc-weak Weak clone; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::is_alive` | check-prelude std-rc-arc-weak Weak liveness predicate; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::is_empty` | check-prelude std-rc-arc-weak empty Weak sentinel predicate; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::new` | check-prelude std-rc-arc-weak empty Weak sentinel constructor; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::strong_count` | check-prelude std-rc-arc-weak Weak strong-count view; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::upgrade` | check-prelude std-rc-arc-weak Weak upgrade to Rc; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::upgrade_arc` | check-prelude std-rc-arc-weak Weak upgrade to Arc; docs/stdlib/modules/rc.md |
| `method std::rc::Weak[T]::weak_count` | check-prelude std-rc-arc-weak Weak weak-count view; docs/stdlib/modules/rc.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::rc` | check-prelude std-rc-arc-weak reference-counted shared ownership handles; docs/stdlib/modules/rc.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::rc::Arc[T]` | check-prelude std-rc-arc-weak atomic-count shared owner handle; docs/stdlib/modules/rc.md |
| `struct std::rc::Rc[T]` | check-prelude std-rc-arc-weak shared owner handle; docs/stdlib/modules/rc.md |
| `struct std::rc::Weak[T]` | check-prelude std-rc-arc-weak non-owning shared handle; docs/stdlib/modules/rc.md |

## `std::result`

Tier: `core`. Stability reading: stable candidate.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::result::ResultMut[T, E]` | check-prelude prelude-option-result-ref-access mutable borrowed result payload view union; docs/stdlib/modules/option-result.md |
| `enum std::result::ResultRef[T, E]` | check-prelude prelude-option-result-ref-access shared borrowed result payload view union; docs/stdlib/modules/option-result.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::result::ResultMut[T,E]::as_ref` | check-prelude prelude-option-result-ref-access downgrade mutable result payload view to shared view; docs/stdlib/modules/option-result.md |
| `method std::result::ResultMut[T,E]::is_err` | check-prelude prelude-option-result-ref-access mutable result error predicate; docs/stdlib/modules/option-result.md |
| `method std::result::ResultMut[T,E]::is_ok` | check-prelude prelude-option-result-ref-access mutable result success predicate; docs/stdlib/modules/option-result.md |
| `method std::result::ResultMut[T,E]::unwrap` | check-prelude prelude-option-result-ref-access mutable result success unwrap handle; docs/stdlib/modules/option-result.md |
| `method std::result::ResultMut[T,E]::unwrap_err` | check-prelude prelude-option-result-ref-access mutable result error unwrap handle; docs/stdlib/modules/option-result.md |
| `method std::result::ResultRef[T,E]::is_err` | check-prelude prelude-option-result-ref-access shared result error predicate; docs/stdlib/modules/option-result.md |
| `method std::result::ResultRef[T,E]::is_ok` | check-prelude prelude-option-result-ref-access shared result success predicate; docs/stdlib/modules/option-result.md |
| `method std::result::ResultRef[T,E]::unwrap` | check-prelude prelude-option-result-ref-access shared result success unwrap handle; docs/stdlib/modules/option-result.md |
| `method std::result::ResultRef[T,E]::unwrap_err` | check-prelude prelude-option-result-ref-access shared result error unwrap handle; docs/stdlib/modules/option-result.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::result` | prelude Result method tests; docs/dev/test-matrix.md Prelude row |

## `std::string`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::string::alloc_buffer` | std string allocation seed tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::string::bytes` | check-prelude std-string-byte-literals natural borrowed byte view over string literals; docs/stdlib/modules/string.md |
| `fn std::string::c_bytes` | check-prelude std-string-text-kinds borrowed C string byte view without terminator; docs/stdlib/modules/string.md |
| `fn std::string::c_len` | check-prelude std-string-text-kinds C string byte length without terminator; docs/stdlib/modules/string.md |
| `fn std::string::c_str` | check-prelude std-string-text-kinds convenience constructor returning std::c::CStr; docs/stdlib/modules/string.md |
| `fn std::string::codepoints` | check-prelude std-string-unicode-helpers validated borrowed UTF-8 scalar iterator constructor; docs/stdlib/modules/string.md |
| `fn std::string::contains` | check-prelude std-string-module-views borrowed byte-slice contains helper; docs/stdlib/modules/string.md |
| `fn std::string::copy` | check-prelude std-string-natural-api natural borrowed byte copy constructor; docs/stdlib/modules/string.md |
| `fn std::string::copy_to` | check-prelude std-string-from-copy borrowed source and target reset tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::string::empty` | check-prelude std-string-natural-api natural empty owned string constructor; docs/stdlib/modules/string.md |
| `fn std::string::ends_with` | check-prelude std-string-module-views borrowed byte-slice suffix predicate; docs/stdlib/modules/string.md |
| `fn std::string::find` | check-prelude std-string-module-views borrowed byte-slice search helper; docs/stdlib/modules/string.md |
| `fn std::string::from` | check-prelude std-string-natural-api natural Ari string copy constructor; docs/stdlib/modules/string.md |
| `fn std::string::from_slice_in` | std string from-slice target-zone copy tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::string::from_string` | std string handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::string::join_in` | check-prelude std-string-split-join allocator-backed byte-slice join helper; docs/stdlib/modules/string.md |
| `fn std::string::lines` | check-prelude std-string-module-views borrowed newline split helper for parser-style code; docs/stdlib/modules/string.md |
| `fn std::string::new` | std string handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::string::os_str` | check-prelude std-string-text-kinds typed borrowed OS-string byte view constructor; docs/stdlib/modules/string.md |
| `fn std::string::os_string` | check-prelude std-string-text-kinds owned OS-string byte constructor; docs/stdlib/modules/string.md |
| `fn std::string::os_string_from_text` | check-prelude std-string-text-kinds owned OS-string text constructor; docs/stdlib/modules/string.md |
| `fn std::string::replace` | check-prelude std-string-module-views allocator-backed byte-slice replacement helper; docs/stdlib/modules/string.md |
| `fn std::string::split` | check-prelude std-string-module-views borrowed delimiter split helper; docs/stdlib/modules/string.md |
| `fn std::string::split_once` | check-prelude std-string-module-views borrowed first-delimiter split helper; docs/stdlib/modules/string.md |
| `fn std::string::starts_with` | check-prelude std-string-module-views borrowed byte-slice prefix predicate; docs/stdlib/modules/string.md |
| `fn std::string::strip_prefix` | check-prelude std-string-module-views borrowed prefix-stripping helper; docs/stdlib/modules/string.md |
| `fn std::string::strip_suffix` | check-prelude std-string-module-views borrowed suffix-stripping helper; docs/stdlib/modules/string.md |
| `fn std::string::substring` | check-prelude std-string-module-views borrowed byte-range view helper; docs/stdlib/modules/string.md |
| `fn std::string::trim` | check-prelude std-string-module-views borrowed ASCII trim helper; docs/stdlib/modules/string.md |
| `fn std::string::trim_end` | check-prelude std-string-module-views borrowed ASCII trim-end helper; docs/stdlib/modules/string.md |
| `fn std::string::trim_start` | check-prelude std-string-module-views borrowed ASCII trim-start helper; docs/stdlib/modules/string.md |
| `fn std::string::utf8` | check-prelude std-string-text-kinds validated borrowed UTF-8 byte view constructor; docs/stdlib/modules/string.md |
| `fn std::string::utf8_string` | check-prelude std-string-text-kinds Result-returning owned UTF-8 string constructor; docs/stdlib/modules/string.md |
| `fn std::string::utf8_string_optional` | check-prelude std-string-text-kinds Option-returning owned UTF-8 string compatibility constructor; docs/stdlib/modules/string.md |
| `fn std::string::utf8_string_unchecked` | check-prelude std-string-text-kinds asserting owned UTF-8 string constructor; docs/stdlib/modules/string.md |
| `fn std::string::with_capacity` | std string handle tests; docs/dev/test-matrix.md Explicit memory zones row |

### method

| API | Coverage note |
| --- | --- |
| `method std::string::OsStr::as_slice` | check-prelude std-string-text-kinds borrowed OS string byte accessor; docs/stdlib/modules/string.md |
| `method std::string::OsStr::is_empty` | check-prelude std-string-text-kinds OS string empty predicate; docs/stdlib/modules/string.md |
| `method std::string::OsStr::is_utf8` | check-prelude std-string-text-kinds OS string UTF-8 validation predicate; docs/stdlib/modules/string.md |
| `method std::string::OsStr::len` | check-prelude std-string-text-kinds OS string byte length; docs/stdlib/modules/string.md |
| `method std::string::OsStr::try_utf8` | check-prelude std-string-text-kinds OS string validated UTF-8 conversion; docs/stdlib/modules/string.md |
| `method std::string::OsString::as_os_str` | check-prelude std-string-text-kinds owned OS string borrowed view accessor; docs/stdlib/modules/string.md |
| `method std::string::OsString::as_slice` | check-prelude std-string-text-kinds owned OS string byte accessor; docs/stdlib/modules/string.md |
| `method std::string::OsString::as_string` | check-prelude std-string-text-kinds owned OS string byte-storage accessor; docs/stdlib/modules/string.md |
| `method std::string::OsString::is_empty` | check-prelude std-string-text-kinds owned OS string empty predicate; docs/stdlib/modules/string.md |
| `method std::string::OsString::is_utf8` | check-prelude std-string-text-kinds owned OS string UTF-8 validation predicate; docs/stdlib/modules/string.md |
| `method std::string::OsString::len` | check-prelude std-string-text-kinds owned OS string byte length; docs/stdlib/modules/string.md |
| `method std::string::OsString::to_string` | check-prelude std-string-text-kinds owned OS string byte copy helper; docs/stdlib/modules/string.md |
| `method std::string::OsString::try_utf8` | check-prelude std-string-text-kinds owned OS string validated borrowed UTF-8 conversion; docs/stdlib/modules/string.md |
| `method std::string::OsString::try_utf8_string` | check-prelude std-string-text-kinds owned OS string validated owned UTF-8 conversion; docs/stdlib/modules/string.md |
| `method std::string::SplitOnce::left` | check-prelude std-string-module-views borrowed left side of split_once result; docs/stdlib/modules/string.md |
| `method std::string::SplitOnce::right` | check-prelude std-string-module-views borrowed right side of split_once result; docs/stdlib/modules/string.md |
| `method std::string::String::append` | check-prelude std-string-natural-api natural Ari string append helper; docs/stdlib/modules/string.md |
| `method std::string::String::append_bool_in` | check-prelude std-string-append and same-zone diagnostic; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::string::String::append_byte` | check-prelude std-string-natural-api natural single-byte append helper; docs/stdlib/modules/string.md |
| `method std::string::String::append_bytes` | check-prelude std-string-natural-api natural borrowed-byte append helper; docs/stdlib/modules/string.md |
| `method std::string::String::append_debug_in[T: std::fmt::Debug]` | check-prelude std-string-append-debug generic Debug append, implicit same-zone lowering, and std-string-append-debug-different-zone diagnostic; docs/stdlib/modules/string.md |
| `method std::string::String::append_f32_in` | check-prelude std-string-append-f32 and std-string-append-f32-different-zone; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::string::String::append_f64_in` | check-prelude prelude-format-in and std-string-append-f64-different-zone; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::string::String::append_i64_in` | check-prelude std-string-append and same-zone diagnostic; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::string::String::append_string_in` | check-prelude std-string-append and same-zone diagnostic; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::string::String::append_u64_in` | check-prelude std-string-append-u64, prelude-format-in-u64, and std-string-append-u64-different-zone; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::string::String::append_value_in[T: std::fmt::Display]` | check-prelude std-string-append-value generic Display append, implicit same-zone lowering, and std-string-append-value-different-zone diagnostic; docs/stdlib/modules/string.md |
| `method std::string::String::as_ptr` | check-prelude std-string-handle borrowed receiver and as-ptr provenance tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::as_slice` | check-prelude std-string-handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::bytes` | check-prelude std-string-natural-api natural borrowed byte view method; docs/stdlib/modules/string.md |
| `method std::string::String::capacity` | check-prelude std-string-handle borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::chunks` | check-prelude std-string-split-join byte-view chunk iterator; docs/stdlib/modules/string.md |
| `method std::string::String::clear` | check-prelude std-string-handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::codepoint_at` | check-prelude std-string-unicode-helpers UTF-8 scalar decode at byte offset; docs/stdlib/modules/string.md |
| `method std::string::String::codepoint_count` | check-prelude std-string-unicode-helpers UTF-8 scalar count validator; docs/stdlib/modules/string.md |
| `method std::string::String::codepoint_next_index` | check-prelude std-string-unicode-helpers UTF-8 scalar next byte offset helper; docs/stdlib/modules/string.md |
| `method std::string::String::codepoints` | check-prelude std-string-unicode-helpers Option-returning UTF-8 scalar iterator helper; docs/stdlib/modules/string.md |
| `method std::string::String::contains` | check-prelude std-string-search borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::contains_ignore_case` | check-prelude std-string-ascii-case-helpers borrowed receiver ASCII-only search; docs/stdlib/api-reference.md String section |
| `method std::string::String::contains_slice` | check-prelude std-string-split-join byte-slice search predicate; docs/stdlib/modules/string.md |
| `method std::string::String::contains_text` | check-prelude std-string-natural-api natural exact text search predicate; docs/stdlib/modules/string.md |
| `method std::string::String::contains_text_ignore_case` | check-prelude std-string-natural-api natural ASCII-insensitive text search predicate; docs/stdlib/modules/string.md |
| `method std::string::String::copy_to` | check-prelude std-string-copy-to-method borrowed receiver and target reset tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::count` | check-prelude std-string-search borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::ends_with` | check-prelude std-string-prefix-suffix borrowed receiver Slice[u8] tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::ends_with_ignore_case` | check-prelude std-string-ascii-case-helpers borrowed receiver ASCII-only comparison; docs/stdlib/api-reference.md String section |
| `method std::string::String::ends_with_text` | check-prelude std-string-natural-api natural exact suffix text helper; docs/stdlib/modules/string.md |
| `method std::string::String::ends_with_text_ignore_case` | check-prelude std-string-natural-api natural ASCII-insensitive suffix text helper; docs/stdlib/modules/string.md |
| `method std::string::String::eq` | check-prelude std-string-equals content equality operator backing for owned String values; docs/stdlib/modules/string.md |
| `method std::string::String::equals` | check-prelude std-string-equals borrowed receiver Slice[u8] tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::equals_ignore_case` | check-prelude std-string-ascii-case-helpers borrowed receiver ASCII-only comparison; docs/stdlib/api-reference.md String section |
| `method std::string::String::equals_text` | check-prelude std-string-natural-api natural exact text equality helper; docs/stdlib/modules/string.md |
| `method std::string::String::equals_text_ignore_case` | check-prelude std-string-natural-api natural ASCII-insensitive text equality helper; docs/stdlib/modules/string.md |
| `method std::string::String::extend_from_slice_in` | check-prelude std-string-grow; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::find` | check-prelude std-string-split-join byte-slice search helper; docs/stdlib/modules/string.md |
| `method std::string::String::find_text` | check-prelude std-string-natural-api natural exact text search helper; docs/stdlib/modules/string.md |
| `method std::string::String::first` | check-prelude std-string-first-last borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::get` | check-prelude std-string-handle borrowed receiver and std-string-from-copy tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::index_of` | check-prelude std-string-search borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::index_of_ignore_case` | check-prelude std-string-ascii-case-helpers borrowed receiver ASCII-only search; docs/stdlib/api-reference.md String section |
| `method std::string::String::index_of_text_ignore_case` | check-prelude std-string-natural-api natural ASCII-insensitive text search helper; docs/stdlib/modules/string.md |
| `method std::string::String::insert` | check-prelude std-string-grow; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::insert_in` | check-prelude std-string-grow; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::is_empty` | check-prelude std-string-handle borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::is_utf8` | check-prelude std-string-unicode-helpers UTF-8 validation convenience method; docs/stdlib/modules/string.md |
| `method std::string::String::last` | check-prelude std-string-first-last borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::len` | check-prelude std-string-handle borrowed receiver tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::parse_decimal` | check-prelude std-string-ascii-helpers whole-string ASCII decimal parser; docs/stdlib/api-reference.md String section |
| `method std::string::String::parse_decimal_prefix` | check-prelude std-string-prefix-parsers leading ASCII decimal parser; docs/stdlib/api-reference.md String section |
| `method std::string::String::parse_hex` | check-prelude std-string-ascii-helpers whole-string ASCII hexadecimal parser; docs/stdlib/api-reference.md String section |
| `method std::string::String::parse_hex_prefix` | check-prelude std-string-prefix-parsers leading ASCII hexadecimal parser; docs/stdlib/api-reference.md String section |
| `method std::string::String::parse_signed_decimal` | check-prelude std-string-signed-parsers optional-sign whole-string ASCII decimal parser; docs/stdlib/api-reference.md String section |
| `method std::string::String::parse_signed_decimal_prefix` | check-prelude std-string-signed-parsers optional-sign leading ASCII decimal parser; docs/stdlib/api-reference.md String section |
| `method std::string::String::pop` | check-prelude std-string-handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::push` | check-prelude std-string-handle and std-string-from-copy tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::push_codepoint_in` | check-prelude std-string-unicode-helpers UTF-8 scalar append convenience method; docs/stdlib/modules/string.md |
| `method std::string::String::push_in` | check-prelude std-string-grow and same-zone diagnostic; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::push_str` | check-prelude std-string-module-views borrowed byte-slice builder append helper; docs/stdlib/modules/string.md |
| `method std::string::String::remove` | check-prelude std-string-byte-remove asserting indexed byte removal; docs/stdlib/modules/string.md |
| `method std::string::String::replace` | check-prelude std-string-handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::reserve` | check-prelude std-string-grow; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::reserve_extra` | check-prelude std-string-grow; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::resize_in` | check-prelude std-string-grow; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::retain` | check-prelude std-string-byte-retain stable in-place byte filtering; docs/stdlib/modules/string.md |
| `method std::string::String::set` | check-prelude std-string-handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::slice` | check-prelude std-string-split-join borrowed byte range view; docs/stdlib/modules/string.md |
| `method std::string::String::split` | check-prelude std-string-split-join borrowed delimiter split iterator; docs/stdlib/modules/string.md |
| `method std::string::String::split_at` | check-prelude std-string-split-join borrowed byte split helper; docs/stdlib/modules/string.md |
| `method std::string::String::starts_with` | check-prelude std-string-prefix-suffix borrowed receiver Slice[u8] tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::starts_with_ignore_case` | check-prelude std-string-ascii-case-helpers borrowed receiver ASCII-only comparison; docs/stdlib/api-reference.md String section |
| `method std::string::String::starts_with_text` | check-prelude std-string-natural-api natural exact prefix text helper; docs/stdlib/modules/string.md |
| `method std::string::String::starts_with_text_ignore_case` | check-prelude std-string-natural-api natural ASCII-insensitive prefix text helper; docs/stdlib/modules/string.md |
| `method std::string::String::trim` | check-prelude std-string-ascii-helpers borrowed ASCII trim view; docs/stdlib/api-reference.md String section |
| `method std::string::String::trim_end` | check-prelude std-string-ascii-helpers borrowed ASCII trim view; docs/stdlib/api-reference.md String section |
| `method std::string::String::trim_end_to` | check-prelude std-string-trim-copy owned ASCII trim-end copy in target zone; docs/stdlib/modules/string.md |
| `method std::string::String::trim_start` | check-prelude std-string-ascii-helpers borrowed ASCII trim view; docs/stdlib/api-reference.md String section |
| `method std::string::String::trim_start_to` | check-prelude std-string-trim-copy owned ASCII trim-start copy in target zone; docs/stdlib/modules/string.md |
| `method std::string::String::trim_to` | check-prelude std-string-trim-copy owned ASCII trim copy in target zone; docs/stdlib/modules/string.md |
| `method std::string::String::trimmed` | check-prelude std-string-natural-api natural owned ASCII trim copy alias; docs/stdlib/modules/string.md |
| `method std::string::String::trimmed_end` | check-prelude std-string-natural-api natural owned ASCII trim-end copy alias; docs/stdlib/modules/string.md |
| `method std::string::String::trimmed_start` | check-prelude std-string-natural-api natural owned ASCII trim-start copy alias; docs/stdlib/modules/string.md |
| `method std::string::String::truncate` | check-prelude std-string-handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::string::String::try_first` | check-prelude std-string-try-byte-access Option-returning empty-safe byte access; docs/stdlib/modules/string.md |
| `method std::string::String::try_get` | check-prelude std-string-try-byte-access Option-returning out-of-range byte access; docs/stdlib/modules/string.md |
| `method std::string::String::try_last` | check-prelude std-string-try-byte-access Option-returning empty-safe byte access; docs/stdlib/modules/string.md |
| `method std::string::String::try_pop` | check-prelude std-string-try-byte-access Option-returning empty pop; docs/stdlib/modules/string.md |
| `method std::string::String::try_remove` | check-prelude std-string-byte-remove Option-returning indexed byte removal; docs/stdlib/modules/string.md |
| `method std::string::String::try_utf8` | check-prelude std-string-natural-api natural validated UTF-8 view helper; docs/stdlib/modules/string.md |
| `method std::string::String::windows` | check-prelude std-string-split-join borrowed byte window iterator; docs/stdlib/modules/string.md |
| `method std::string::Utf8::as_slice` | check-prelude std-string-text-kinds validated UTF-8 bytes accessor; docs/stdlib/modules/string.md |
| `method std::string::Utf8::codepoint_at` | check-prelude std-string-text-kinds validated UTF-8 scalar lookup; docs/stdlib/modules/string.md |
| `method std::string::Utf8::codepoint_count` | check-prelude std-string-text-kinds validated UTF-8 scalar count; docs/stdlib/modules/string.md |
| `method std::string::Utf8::codepoints` | check-prelude std-string-unicode-helpers validated UTF-8 scalar iterator helper; docs/stdlib/modules/string.md |
| `method std::string::Utf8::is_empty` | check-prelude std-string-text-kinds UTF-8 byte empty predicate; docs/stdlib/modules/string.md |
| `method std::string::Utf8::len` | check-prelude std-string-text-kinds UTF-8 byte length accessor; docs/stdlib/modules/string.md |
| `method std::string::Utf8::next_index` | check-prelude std-string-unicode-helpers validated UTF-8 next byte offset helper; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::as_slice` | check-prelude std-string-text-kinds owned UTF-8 string byte accessor; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::as_string` | check-prelude std-string-text-kinds owned UTF-8 string byte-storage accessor; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::as_utf8` | check-prelude std-string-text-kinds owned UTF-8 string borrowed UTF-8 view accessor; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::codepoint_at` | check-prelude std-string-text-kinds owned UTF-8 scalar lookup; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::codepoint_count` | check-prelude std-string-text-kinds owned UTF-8 scalar count; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::codepoints` | check-prelude std-string-text-kinds owned UTF-8 scalar iterator helper; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::is_empty` | check-prelude std-string-text-kinds owned UTF-8 byte empty predicate; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::len` | check-prelude std-string-text-kinds owned UTF-8 byte length accessor; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::next_index` | check-prelude std-string-text-kinds owned UTF-8 next byte offset helper; docs/stdlib/modules/string.md |
| `method std::string::Utf8String::to_string` | check-prelude std-string-text-kinds owned UTF-8 byte copy helper; docs/stdlib/modules/string.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::string` | std string allocation seed tests; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::string::Codepoints` | check-prelude std-string-unicode-helpers borrowed UTF-8 scalar iterator state; docs/stdlib/modules/string.md |
| `struct std::string::OsStr` | check-prelude std-string-text-kinds typed borrowed OS string byte view; docs/stdlib/modules/string.md |
| `struct std::string::OsString` | check-prelude std-string-text-kinds owned OS string byte wrapper; docs/stdlib/modules/string.md |
| `struct std::string::RawString` | std string handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `struct std::string::SplitOnce` | check-prelude std-string-module-views borrowed result handle for split_once; docs/stdlib/modules/string.md |
| `struct std::string::String` | std string handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `struct std::string::Utf8` | check-prelude std-string-text-kinds typed validated UTF-8 byte view; docs/stdlib/modules/string.md |
| `struct std::string::Utf8String` | check-prelude std-string-text-kinds owned validated UTF-8 string wrapper; docs/stdlib/modules/string.md |

## `std::sync`

Tier: `hosted`. Stability reading: platform-backed.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::sync::Ordering` | check-prelude std-sync-concurrency-api explicit atomic memory-order vocabulary; docs/stdlib/modules/sync.md |
| `enum std::sync::RecvError` | check-prelude std-sync-concurrency-api channel blocking receive failure reason; docs/stdlib/modules/sync.md |
| `enum std::sync::RecvTimeoutError` | check-prelude std-sync-concurrency-api channel timeout receive failure reason; docs/stdlib/modules/sync.md |
| `enum std::sync::SendError[T]` | check-prelude std-sync-concurrency-api channel blocking send failure with unsent value; docs/stdlib/modules/sync.md |
| `enum std::sync::TryRecvError` | check-prelude std-sync-concurrency-api channel nonblocking receive failure reason; docs/stdlib/modules/sync.md |
| `enum std::sync::TrySendError[T]` | check-prelude std-sync-concurrency-api channel nonblocking send failure with unsent value; docs/stdlib/modules/sync.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::sync::call_once` | check-prelude std-sync-mutex-once source once execution helper; docs/stdlib/modules/sync.md |
| `fn std::sync::channel[T]` | check-prelude std-sync-concurrency-api single-slot MPSC channel constructor; docs/stdlib/modules/sync.md |
| `fn std::sync::compare_exchange` | check-prelude std-sync-atomic-i64 atomic compare-and-exchange hook; docs/stdlib/modules/sync.md |
| `fn std::sync::fetch_add` | check-prelude std-sync-atomic-i64 atomic fetch-add hook; docs/stdlib/modules/sync.md |
| `fn std::sync::is_compare_exchange_order` | check-prelude std-sync-concurrency-api memory-order validation helper for compare-exchange success/failure pairs; docs/stdlib/modules/sync.md |
| `fn std::sync::is_completed` | check-prelude std-sync-mutex-once source once completion predicate; docs/stdlib/modules/sync.md |
| `fn std::sync::is_load_order` | check-prelude std-sync-concurrency-api memory-order validation helper for loads; docs/stdlib/modules/sync.md |
| `fn std::sync::is_locked` | check-prelude std-sync-mutex-once source mutex state predicate; docs/stdlib/modules/sync.md |
| `fn std::sync::is_read_locked` | check-prelude std-sync-rwlock source read-lock diagnostic predicate; docs/stdlib/modules/sync.md |
| `fn std::sync::is_rmw_order` | check-prelude std-sync-concurrency-api memory-order validation helper for RMW operations; docs/stdlib/modules/sync.md |
| `fn std::sync::is_store_order` | check-prelude std-sync-concurrency-api memory-order validation helper for stores; docs/stdlib/modules/sync.md |
| `fn std::sync::is_write_locked` | check-prelude std-sync-rwlock source write-lock diagnostic predicate; docs/stdlib/modules/sync.md |
| `fn std::sync::load` | check-prelude std-sync-atomic-i64 atomic load hook; docs/stdlib/modules/sync.md |
| `fn std::sync::lock` | check-prelude std-sync-mutex-once source mutex spin lock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::mpsc_channel[T]` | check-prelude std-sync-concurrency-api explicit MPSC channel constructor alias; docs/stdlib/modules/sync.md |
| `fn std::sync::read_lock` | check-prelude std-sync-rwlock source read lock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::read_unlock` | check-prelude std-sync-rwlock source read unlock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::reader_count` | check-prelude std-sync-rwlock source reader count helper; docs/stdlib/modules/sync.md |
| `fn std::sync::seq_cst` | check-prelude std-sync-concurrency-api SeqCst memory-order convenience helper; docs/stdlib/modules/sync.md |
| `fn std::sync::store` | check-prelude std-sync-atomic-i64 atomic store hook; docs/stdlib/modules/sync.md |
| `fn std::sync::swap` | check-prelude std-sync-atomic-i64 atomic exchange hook; docs/stdlib/modules/sync.md |
| `fn std::sync::try_lock` | check-prelude std-sync-mutex-once source mutex nonblocking lock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::try_read_lock` | check-prelude std-sync-rwlock source nonblocking read lock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::try_write_lock` | check-prelude std-sync-rwlock source nonblocking write lock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::unlock` | check-prelude std-sync-mutex-once source mutex unlock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::write_lock` | check-prelude std-sync-rwlock source write lock helper; docs/stdlib/modules/sync.md |
| `fn std::sync::write_unlock` | check-prelude std-sync-rwlock source write unlock helper; docs/stdlib/modules/sync.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::sync::AtomicBool::compare_exchange` | check-prelude std-sync-concurrency-api bool compare-and-exchange Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::compare_exchange_bool` | check-prelude std-sync-concurrency-api bool compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::compare_exchange_order` | check-prelude std-sync-concurrency-api bool atomic explicit-order compare-and-exchange Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::compare_exchange_order_bool` | check-prelude std-sync-concurrency-api bool atomic explicit-order compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::load` | check-prelude std-sync-concurrency-api bool atomic load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::load_order` | check-prelude std-sync-concurrency-api bool atomic explicit-order load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::new` | check-prelude std-sync-concurrency-api bool atomic constructor; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::store` | check-prelude std-sync-concurrency-api bool atomic store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::store_order` | check-prelude std-sync-concurrency-api bool atomic explicit-order store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::swap` | check-prelude std-sync-concurrency-api bool atomic exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicBool::swap_order` | check-prelude std-sync-concurrency-api bool atomic explicit-order exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::compare_exchange` | check-prelude std-sync-atomic-i64 compare-and-exchange Result method wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::compare_exchange_bool` | check-prelude std-sync-atomic-i64 compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::compare_exchange_order` | check-prelude std-sync-concurrency-api explicit-order compare-and-exchange Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::compare_exchange_order_bool` | check-prelude std-sync-concurrency-api explicit-order compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::fetch_add` | check-prelude std-sync-atomic-i64 fetch-add method wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::fetch_add_order` | check-prelude std-sync-concurrency-api explicit-order fetch-add wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::load` | check-prelude std-sync-atomic-i64 load method wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::load_order` | check-prelude std-sync-concurrency-api explicit-order load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::new` | check-prelude std-sync-atomic-i64 associated constructor; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::store` | check-prelude std-sync-atomic-i64 store method wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::store_order` | check-prelude std-sync-concurrency-api explicit-order store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::swap` | check-prelude std-sync-atomic-i64 exchange method wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicI64::swap_order` | check-prelude std-sync-concurrency-api explicit-order exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::compare_exchange` | check-prelude std-sync-concurrency-api pointer atomic compare-and-exchange raw-value Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::compare_exchange_bool` | check-prelude std-sync-concurrency-api pointer atomic compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::compare_exchange_order` | check-prelude std-sync-concurrency-api pointer atomic explicit-order compare-and-exchange raw-value Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::compare_exchange_order_bool` | check-prelude std-sync-concurrency-api pointer atomic explicit-order compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::load` | check-prelude std-sync-concurrency-api pointer atomic load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::load_order` | check-prelude std-sync-concurrency-api pointer atomic explicit-order load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::new` | check-prelude std-sync-concurrency-api pointer atomic constructor; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::null_ptr` | check-prelude std-sync-concurrency-api null pointer atomic constructor; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::store` | check-prelude std-sync-concurrency-api pointer atomic store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::store_order` | check-prelude std-sync-concurrency-api pointer atomic explicit-order store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::swap` | check-prelude std-sync-concurrency-api pointer atomic exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicPtr[T]::swap_order` | check-prelude std-sync-concurrency-api pointer atomic explicit-order exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::compare_exchange` | check-prelude std-sync-concurrency-api usize atomic compare-and-exchange Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::compare_exchange_bool` | check-prelude std-sync-concurrency-api usize atomic compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::compare_exchange_order` | check-prelude std-sync-concurrency-api usize atomic explicit-order compare-and-exchange Result wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::compare_exchange_order_bool` | check-prelude std-sync-concurrency-api usize atomic explicit-order compare-and-exchange compatibility predicate; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::fetch_add` | check-prelude std-sync-concurrency-api usize atomic fetch-add wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::fetch_add_order` | check-prelude std-sync-concurrency-api usize atomic explicit-order fetch-add wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::load` | check-prelude std-sync-concurrency-api usize atomic load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::load_order` | check-prelude std-sync-concurrency-api usize atomic explicit-order load wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::new` | check-prelude std-sync-concurrency-api usize atomic constructor; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::store` | check-prelude std-sync-concurrency-api usize atomic store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::store_order` | check-prelude std-sync-concurrency-api usize atomic explicit-order store wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::swap` | check-prelude std-sync-concurrency-api usize atomic exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::AtomicUsize::swap_order` | check-prelude std-sync-concurrency-api usize atomic explicit-order exchange wrapper; docs/stdlib/modules/sync.md |
| `method std::sync::Barrier::new` | check-prelude std-sync-concurrency-api barrier constructor; docs/stdlib/modules/sync.md |
| `method std::sync::Barrier::parties` | check-prelude std-sync-concurrency-api barrier party-count accessor; docs/stdlib/modules/sync.md |
| `method std::sync::Barrier::wait` | check-prelude std-sync-concurrency-api barrier wait coordinator; docs/stdlib/modules/sync.md |
| `method std::sync::Channel[T]::receiver` | check-prelude std-sync-concurrency-api channel receiver accessor; docs/stdlib/modules/sync.md |
| `method std::sync::Channel[T]::sender` | check-prelude std-sync-concurrency-api channel sender accessor; docs/stdlib/modules/sync.md |
| `method std::sync::Channel[T]::split` | check-prelude std-sync-concurrency-api channel split helper; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::generation` | check-prelude std-sync-concurrency-api condition-variable notification generation; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::new` | check-prelude std-sync-concurrency-api condition-variable constructor; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::notify_all` | check-prelude std-sync-concurrency-api condition-variable broadcast notification; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::notify_one` | check-prelude std-sync-concurrency-api condition-variable single notification; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::wait` | check-prelude std-sync-concurrency-api condition-variable wait helper; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::wait_timeout` | check-prelude std-sync-concurrency-api condition-variable spin-yield timeout wait helper; docs/stdlib/modules/sync.md |
| `method std::sync::Condvar::wait_while` | check-prelude std-sync-concurrency-api predicate condition-variable wait helper; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::is_active` | check-prelude std-sync-value-locks value mutex guard active-state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::replace` | check-prelude std-sync-value-locks value mutex guard replace helper; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::set` | check-prelude std-sync-value-locks value mutex guard set helper; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::unlock` | check-prelude std-sync-value-locks value mutex guard idempotent unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::value` | check-prelude std-sync-value-locks value mutex guard copy accessor; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::value_mut` | check-prelude std-sync-value-locks value mutex guard mutable reference accessor; docs/stdlib/modules/sync.md |
| `method std::sync::MutexGuard[T]::value_ref` | check-prelude std-sync-value-locks value mutex guard shared reference accessor; docs/stdlib/modules/sync.md |
| `method std::sync::Mutex[T]::get_mut` | check-prelude std-sync-value-locks value mutex exclusive unlocked payload accessor; docs/stdlib/modules/sync.md |
| `method std::sync::Mutex[T]::into_inner` | check-prelude std-sync-value-locks value mutex consuming payload extraction; docs/stdlib/modules/sync.md |
| `method std::sync::Mutex[T]::is_locked` | check-prelude std-sync-value-locks value mutex state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::Mutex[T]::lock` | check-prelude std-sync-value-locks value mutex guard-returning lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::Mutex[T]::new` | check-prelude std-sync-value-locks value mutex constructor; docs/stdlib/modules/sync.md |
| `method std::sync::Mutex[T]::try_lock` | check-prelude std-sync-value-locks value mutex optional guard nonblocking lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::Once::call_once` | check-prelude std-sync-mutex-once once method execution helper; docs/stdlib/modules/sync.md |
| `method std::sync::Once::is_completed` | check-prelude std-sync-mutex-once once method completion predicate; docs/stdlib/modules/sync.md |
| `method std::sync::Once::new` | check-prelude std-sync-mutex-once once associated constructor; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::get` | check-prelude std-sync-concurrency-api thread-facing once-lock shared value view; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::get_mut` | check-prelude std-sync-concurrency-api thread-facing once-lock mutable value view; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::get_or_init` | check-prelude std-sync-concurrency-api thread-facing lazy initialization helper; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::get_or_try_init` | check-prelude std-sync-concurrency-api thread-facing fallible lazy initialization status helper; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::is_empty` | check-prelude std-sync-concurrency-api once-lock empty predicate; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::is_initialized` | check-prelude std-sync-concurrency-api once-lock initialized predicate; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::new` | check-prelude std-sync-concurrency-api once-lock constructor; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::set` | check-prelude std-sync-concurrency-api once-lock one-time Result setter preserving rejected value; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::set_bool` | check-prelude std-sync-concurrency-api once-lock lossy bool setter compatibility helper; docs/stdlib/modules/sync.md |
| `method std::sync::OnceLock[T]::take` | check-prelude std-sync-concurrency-api once-lock value extraction helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::is_locked` | check-prelude std-sync-mutex-once raw mutex method state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::lock` | check-prelude std-sync-mutex-once raw mutex method guard-returning spin lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::lock_raw` | check-prelude std-sync-mutex-once raw mutex method raw spin lock compatibility helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::new` | check-prelude std-sync-mutex-once raw mutex associated constructor; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::try_lock` | check-prelude std-sync-mutex-once raw mutex method optional guard nonblocking lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::try_lock_bool` | check-prelude std-sync-mutex-once raw mutex method bool nonblocking lock compatibility helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutex::unlock` | check-prelude std-sync-mutex-once raw mutex method unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutexGuard::is_active` | check-prelude std-sync-mutex-once raw mutex guard active-state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawMutexGuard::unlock` | check-prelude std-sync-mutex-once raw mutex guard idempotent unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::is_locked` | check-prelude std-sync-rwlock raw rwlock method any-lock predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::is_read_locked` | check-prelude std-sync-rwlock raw rwlock method read-lock predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::is_write_locked` | check-prelude std-sync-rwlock raw rwlock method write-lock predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::new` | check-prelude std-sync-rwlock raw rwlock associated constructor; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::read` | check-prelude std-sync-rwlock raw rwlock method guard-returning read lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::read_lock` | check-prelude std-sync-rwlock raw rwlock method read lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::read_unlock` | check-prelude std-sync-rwlock raw rwlock method read unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::reader_count` | check-prelude std-sync-rwlock raw rwlock method reader count helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::try_read` | check-prelude std-sync-rwlock raw rwlock method optional read guard helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::try_read_lock` | check-prelude std-sync-rwlock raw rwlock method nonblocking read lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::try_write` | check-prelude std-sync-rwlock raw rwlock method optional write guard helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::try_write_lock` | check-prelude std-sync-rwlock raw rwlock method nonblocking write lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::write` | check-prelude std-sync-rwlock raw rwlock method guard-returning write lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::write_lock` | check-prelude std-sync-rwlock raw rwlock method write lock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLock::write_unlock` | check-prelude std-sync-rwlock raw rwlock method write unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLockReadGuard::is_active` | check-prelude std-sync-rwlock raw rwlock read guard active-state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLockReadGuard::unlock` | check-prelude std-sync-rwlock raw rwlock read guard idempotent unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLockWriteGuard::is_active` | check-prelude std-sync-rwlock raw rwlock write guard active-state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RawRwLockWriteGuard::unlock` | check-prelude std-sync-rwlock raw rwlock write guard idempotent unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::close` | check-prelude std-sync-concurrency-api receiver-side channel close helper; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::is_closed` | check-prelude std-sync-concurrency-api receiver closed predicate; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::is_empty` | check-prelude std-sync-concurrency-api receiver empty predicate; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::recv` | check-prelude std-sync-concurrency-api yielding channel Result receive helper; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::recv_optional` | check-prelude std-sync-concurrency-api yielding channel optional receive compatibility helper; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::recv_timeout` | check-prelude std-sync-concurrency-api timeout channel Result receive helper; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::try_recv` | check-prelude std-sync-concurrency-api nonblocking channel Result receive helper; docs/stdlib/modules/sync.md |
| `method std::sync::Receiver[T]::try_recv_optional` | check-prelude std-sync-concurrency-api nonblocking channel optional receive compatibility helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockReadGuard[T]::is_active` | check-prelude std-sync-value-locks value rwlock read guard active-state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockReadGuard[T]::unlock` | check-prelude std-sync-value-locks value rwlock read guard idempotent unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockReadGuard[T]::value` | check-prelude std-sync-value-locks value rwlock read guard copy accessor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockReadGuard[T]::value_ref` | check-prelude std-sync-value-locks value rwlock read guard shared reference accessor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::is_active` | check-prelude std-sync-value-locks value rwlock write guard active-state predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::replace` | check-prelude std-sync-value-locks value rwlock write guard replace helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::set` | check-prelude std-sync-value-locks value rwlock write guard set helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::unlock` | check-prelude std-sync-value-locks value rwlock write guard idempotent unlock helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::value` | check-prelude std-sync-value-locks value rwlock write guard copy accessor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::value_mut` | check-prelude std-sync-value-locks value rwlock write guard mutable reference accessor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLockWriteGuard[T]::value_ref` | check-prelude std-sync-value-locks value rwlock write guard shared reference accessor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::get_mut` | check-prelude std-sync-value-locks value rwlock exclusive unlocked payload accessor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::into_inner` | check-prelude std-sync-value-locks value rwlock consuming payload extraction; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::is_locked` | check-prelude std-sync-value-locks value rwlock any-lock predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::is_read_locked` | check-prelude std-sync-value-locks value rwlock read-lock predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::is_write_locked` | check-prelude std-sync-value-locks value rwlock write-lock predicate; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::new` | check-prelude std-sync-value-locks value rwlock constructor; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::read` | check-prelude std-sync-value-locks value rwlock guard-returning read helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::reader_count` | check-prelude std-sync-value-locks value rwlock reader count helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::try_read` | check-prelude std-sync-value-locks value rwlock optional read guard helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::try_write` | check-prelude std-sync-value-locks value rwlock optional write guard helper; docs/stdlib/modules/sync.md |
| `method std::sync::RwLock[T]::write` | check-prelude std-sync-value-locks value rwlock guard-returning write helper; docs/stdlib/modules/sync.md |
| `method std::sync::Sender[T]::clone` | check-prelude std-sync-concurrency-api sender handle clone sharing the same single-slot channel state; docs/stdlib/modules/sync.md |
| `method std::sync::Sender[T]::close` | check-prelude std-sync-concurrency-api sender-side channel close helper; docs/stdlib/modules/sync.md |
| `method std::sync::Sender[T]::is_closed` | check-prelude std-sync-concurrency-api sender closed predicate; docs/stdlib/modules/sync.md |
| `method std::sync::Sender[T]::send` | check-prelude std-sync-concurrency-api yielding single-slot Result send helper; docs/stdlib/modules/sync.md |
| `method std::sync::Sender[T]::send_bool` | check-prelude std-sync-concurrency-api lossy nonblocking send compatibility helper; docs/stdlib/modules/sync.md |
| `method std::sync::Sender[T]::try_send` | check-prelude std-sync-concurrency-api nonblocking single-slot Result send helper; docs/stdlib/modules/sync.md |
| `method std::sync::WaitTimeoutResult::timed_out` | check-prelude std-sync-concurrency-api condition-variable timeout predicate; docs/stdlib/modules/sync.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::sync` | check-prelude std-sync-atomic-i64 atomic primitive module; docs/stdlib/modules/sync.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::sync::AtomicBool` | check-prelude std-sync-concurrency-api bool atomic wrapper; docs/stdlib/modules/sync.md |
| `struct std::sync::AtomicI64` | check-prelude std-sync-atomic-i64 atomic primitive value; docs/stdlib/modules/sync.md |
| `struct std::sync::AtomicPtr[T]` | check-prelude std-sync-concurrency-api pointer atomic wrapper; docs/stdlib/modules/sync.md |
| `struct std::sync::AtomicUsize` | check-prelude std-sync-concurrency-api pointer-sized atomic wrapper; docs/stdlib/modules/sync.md |
| `struct std::sync::Barrier` | check-prelude std-sync-concurrency-api source barrier primitive; docs/stdlib/modules/sync.md |
| `struct std::sync::Channel[T]` | check-prelude std-sync-concurrency-api single-slot channel pair; docs/stdlib/modules/sync.md |
| `struct std::sync::Condvar` | check-prelude std-sync-concurrency-api source condition variable primitive; docs/stdlib/modules/sync.md |
| `struct std::sync::MutexGuard[T]` | check-prelude std-sync-value-locks value mutex payload guard; docs/stdlib/modules/sync.md |
| `struct std::sync::Mutex[T]` | check-prelude std-sync-value-locks value-protecting mutex payload handle; docs/stdlib/modules/sync.md |
| `struct std::sync::Once` | check-prelude std-sync-mutex-once source one-time initialization primitive; docs/stdlib/modules/sync.md |
| `struct std::sync::OnceLock[T]` | check-prelude std-sync-concurrency-api source thread-facing one-time slot; docs/stdlib/modules/sync.md |
| `struct std::sync::RawMutex` | check-prelude std-sync-mutex-once source raw mutex primitive; docs/stdlib/modules/sync.md |
| `struct std::sync::RawMutexGuard` | check-prelude std-sync-mutex-once explicit raw mutex unlock guard; docs/stdlib/modules/sync.md |
| `struct std::sync::RawRwLock` | check-prelude std-sync-rwlock source raw reader-writer lock primitive; docs/stdlib/modules/sync.md |
| `struct std::sync::RawRwLockReadGuard` | check-prelude std-sync-rwlock explicit raw rwlock read unlock guard; docs/stdlib/modules/sync.md |
| `struct std::sync::RawRwLockWriteGuard` | check-prelude std-sync-rwlock explicit raw rwlock write unlock guard; docs/stdlib/modules/sync.md |
| `struct std::sync::Receiver[T]` | check-prelude std-sync-concurrency-api channel receiver handle; docs/stdlib/modules/sync.md |
| `struct std::sync::RwLockReadGuard[T]` | check-prelude std-sync-value-locks value rwlock read payload guard; docs/stdlib/modules/sync.md |
| `struct std::sync::RwLockWriteGuard[T]` | check-prelude std-sync-value-locks value rwlock write payload guard; docs/stdlib/modules/sync.md |
| `struct std::sync::RwLock[T]` | check-prelude std-sync-value-locks value-protecting rwlock payload handle; docs/stdlib/modules/sync.md |
| `struct std::sync::Sender[T]` | check-prelude std-sync-concurrency-api channel sender handle; docs/stdlib/modules/sync.md |
| `struct std::sync::WaitTimeoutResult` | check-prelude std-sync-concurrency-api condition-variable timeout result handle; docs/stdlib/modules/sync.md |

## `std::target`

Tier: `platform`. Stability reading: platform-specific.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::target::Arch` | check-prelude std-target-basic compiler-known architecture enum; docs/stdlib/modules/target.md |
| `enum std::target::DebugFormat` | check-prelude std-target-basic compiler-known debug format enum; docs/stdlib/modules/target.md |
| `enum std::target::Env` | check-prelude std-target-basic compiler-known target environment enum; docs/stdlib/modules/target.md |
| `enum std::target::ErrnoAbi` | check-prelude std-target-basic compiler-known errno ABI enum; docs/stdlib/modules/target.md |
| `enum std::target::ObjectFormat` | check-prelude std-target-basic compiler-known object format enum; docs/stdlib/modules/target.md |
| `enum std::target::Os` | check-prelude std-target-basic compiler-known operating-system enum; docs/stdlib/modules/target.md |
| `enum std::target::SyscallAbi` | check-prelude std-target-basic source syscall ABI classifier; docs/stdlib/modules/target.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::target::arch` | check-prelude std-target-basic compiler-known target architecture enum hook; docs/stdlib/modules/target.md |
| `fn std::target::arch_name` | check-prelude std-target-basic compiler-known target architecture string hook; docs/stdlib/modules/target.md |
| `fn std::target::debug_format` | check-prelude std-target-basic compiler-known debug format hook; docs/stdlib/modules/target.md |
| `fn std::target::env` | check-prelude std-target-basic compiler-known target environment enum hook; docs/stdlib/modules/target.md |
| `fn std::target::env_name` | check-prelude std-target-basic compiler-known target environment string hook; docs/stdlib/modules/target.md |
| `fn std::target::errno_abi` | check-prelude std-target-basic compiler-known errno ABI hook; docs/stdlib/modules/target.md |
| `fn std::target::has_capabilities_api` | check-prelude std-target-basic Linux target capability family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_cgroups_api` | check-prelude std-target-basic Linux target cgroups family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_epoll` | check-prelude std-target-basic Linux target epoll family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_eventfd` | check-prelude std-target-basic Linux target eventfd family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_fanotify_api` | check-prelude std-target-basic Linux target fanotify family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_inotify` | check-prelude std-target-basic Linux target inotify family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_io_uring_api` | check-prelude std-target-basic Linux target io_uring family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_memfd` | check-prelude std-target-basic Linux target memfd family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_namespaces_api` | check-prelude std-target-basic Linux target namespaces family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_pidfd_api` | check-prelude std-target-basic Linux target pidfd family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_procfs` | check-prelude std-target-basic Linux procfs family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_seccomp_api` | check-prelude std-target-basic Linux target seccomp family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_signalfd` | check-prelude std-target-basic Linux target signalfd family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_sysfs` | check-prelude std-target-basic Linux sysfs family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_timerfd` | check-prelude std-target-basic Linux target timerfd family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::has_tls` | check-prelude std-target-basic target TLS availability predicate; docs/stdlib/modules/target.md |
| `fn std::target::has_vdso` | check-prelude std-target-basic Linux vDSO family predicate; docs/stdlib/platform/linux.md |
| `fn std::target::is_aarch64` | check-prelude std-target-basic source architecture predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_gnu` | check-prelude std-target-basic source target environment predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_linux` | check-prelude std-target-basic source OS predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_macos` | check-prelude std-target-basic source OS predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_musl` | check-prelude std-target-basic source target environment predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_riscv64` | check-prelude std-target-basic source architecture predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_unix` | check-prelude std-target-basic source Unix-family predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_windows` | check-prelude std-target-basic source OS predicate; docs/stdlib/modules/target.md |
| `fn std::target::is_x86_64` | check-prelude std-target-basic source architecture predicate; docs/stdlib/modules/target.md |
| `fn std::target::long_bits` | check-prelude std-target-basic compiler-known C long width hook; docs/stdlib/modules/target.md |
| `fn std::target::object_format` | check-prelude std-target-basic compiler-known object format hook; docs/stdlib/modules/target.md |
| `fn std::target::os` | check-prelude std-target-basic compiler-known operating-system enum hook; docs/stdlib/modules/target.md |
| `fn std::target::os_name` | check-prelude std-target-basic compiler-known operating-system string hook; docs/stdlib/modules/target.md |
| `fn std::target::pointer_bits` | check-prelude std-target-basic compiler-known pointer width hook; docs/stdlib/modules/target.md |
| `fn std::target::syscall_abi` | check-prelude std-target-basic source Linux syscall ABI classifier; docs/stdlib/modules/target.md |
| `fn std::target::triple` | check-prelude std-target-basic compiler-known target triple string hook; docs/stdlib/modules/target.md |
| `fn std::target::uses_dwarf` | check-prelude std-target-basic source debug format predicate; docs/stdlib/modules/target.md |
| `fn std::target::uses_elf` | check-prelude std-target-basic source object format predicate; docs/stdlib/modules/target.md |
| `fn std::target::uses_glibc` | check-prelude std-target-basic source glibc target predicate; docs/stdlib/modules/target.md |
| `fn std::target::uses_musl` | check-prelude std-target-basic source musl target predicate; docs/stdlib/modules/target.md |
| `fn std::target::uses_posix_errno` | check-prelude std-target-basic source errno ABI predicate; docs/stdlib/modules/target.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::target` | check-prelude std-target-basic compiler-known target fact module; docs/stdlib/modules/target.md |

## `std::test`

Tier: `hosted`. Stability reading: platform-backed.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::test::bench` | check-prelude std-test-report minimal benchmark timer; docs/stdlib/modules/test.md |
| `fn std::test::benchmark` | check-prelude std-test-report benchmark alias; docs/stdlib/modules/test.md |
| `fn std::test::check` | check-prelude std-test-report source unit-test condition aggregation; docs/stdlib/modules/test.md |
| `fn std::test::check_snapshot` | check-prelude std-test-report snapshot/golden comparison aggregation; docs/stdlib/modules/test.md |
| `fn std::test::equal[T]` | check-prelude std-test-report generic equality check aggregation; docs/stdlib/modules/test.md |
| `fn std::test::failed` | check-prelude std-test-report failed-check count accessor; docs/stdlib/modules/test.md |
| `fn std::test::finish` | check-prelude std-test-report executable status helper; docs/stdlib/modules/test.md |
| `fn std::test::golden_matches` | check-prelude std-test-report golden byte comparison helper; docs/stdlib/modules/test.md |
| `fn std::test::matches_snapshot` | check-prelude std-test-report snapshot byte comparison helper; docs/stdlib/modules/test.md |
| `fn std::test::not_equal[T]` | check-prelude std-test-report generic inequality check aggregation; docs/stdlib/modules/test.md |
| `fn std::test::ok` | check-prelude std-test-report report success predicate; docs/stdlib/modules/test.md |
| `fn std::test::passed` | check-prelude std-test-report passed-check count accessor; docs/stdlib/modules/test.md |
| `fn std::test::report` | check-prelude std-test-report unit-test report constructor; docs/stdlib/modules/test.md |
| `fn std::test::require` | check-prelude std-test-report panic-on-failure helper; docs/stdlib/modules/test.md |
| `fn std::test::scratch` | check-prelude std-test-report explicit scratch zone constructor; docs/stdlib/modules/test.md |
| `fn std::test::temp_dir` | check-prelude std-test-report temporary directory result helper; docs/stdlib/modules/test.md |
| `fn std::test::temp_file` | check-prelude std-test-report temporary file result helper; docs/stdlib/modules/test.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::test::Bench::elapsed` | check-prelude std-test-report benchmark elapsed duration; docs/stdlib/modules/test.md |
| `method std::test::Bench::elapsed_nanos` | check-prelude std-test-report benchmark elapsed nanoseconds; docs/stdlib/modules/test.md |
| `method std::test::Bench::iterations` | check-prelude std-test-report benchmark iteration count; docs/stdlib/modules/test.md |
| `method std::test::Bench::nanos_per_iter` | check-prelude std-test-report benchmark per-iteration nanoseconds; docs/stdlib/modules/test.md |
| `method std::test::Report::check` | check-prelude std-test-report method condition aggregation; docs/stdlib/modules/test.md |
| `method std::test::Report::equal[T]` | check-prelude std-test-report method generic equality aggregation; docs/stdlib/modules/test.md |
| `method std::test::Report::failed` | check-prelude std-test-report method failed-count accessor; docs/stdlib/modules/test.md |
| `method std::test::Report::finish` | check-prelude std-test-report method executable status helper; docs/stdlib/modules/test.md |
| `method std::test::Report::not_equal[T]` | check-prelude std-test-report method generic inequality aggregation; docs/stdlib/modules/test.md |
| `method std::test::Report::ok` | check-prelude std-test-report method success predicate; docs/stdlib/modules/test.md |
| `method std::test::Report::passed` | check-prelude std-test-report method passed-count accessor; docs/stdlib/modules/test.md |
| `method std::test::Report::require` | check-prelude std-test-report method panic-on-failure helper; docs/stdlib/modules/test.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::test` | check-prelude std-test-report source unit-test helper module; docs/stdlib/modules/test.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::test::Bench` | check-prelude std-test-report benchmark timing handle; docs/stdlib/modules/test.md |
| `struct std::test::Report` | check-prelude std-test-report unit-test report counts; docs/stdlib/modules/test.md |

## `std::thread`

Tier: `hosted`. Stability reading: platform-backed.

### enum

| API | Coverage note |
| --- | --- |
| `enum std::thread::JoinError` | check-prelude std-thread-basic join handle lifecycle failure reason; docs/stdlib/modules/thread.md |
| `enum std::thread::ThreadLocalSetError[T]` | check-prelude std-thread-local recoverable ThreadLocal set capacity failure preserving value; docs/stdlib/modules/thread.md |

### fn

| API | Coverage note |
| --- | --- |
| `fn std::thread::available_parallelism` | check-prelude std-thread-runtime-helpers Result-returning runtime online processor count helper; docs/stdlib/modules/thread.md |
| `fn std::thread::available_parallelism_or` | check-prelude std-thread-runtime-helpers fallback runtime online processor count helper; docs/stdlib/modules/thread.md |
| `fn std::thread::available_parallelism_raw` | check-prelude std-thread-runtime-helpers raw runtime online processor count hook; docs/stdlib/modules/thread.md |
| `fn std::thread::builder` | check-prelude std-thread-builder thread builder constructor helper; docs/stdlib/modules/thread.md |
| `fn std::thread::current` | check-prelude std-thread-basic current thread info helper; docs/stdlib/modules/thread.md |
| `fn std::thread::id` | check-prelude std-thread-basic typed source wrapper around context thread id; docs/stdlib/modules/thread.md |
| `fn std::thread::id_raw` | check-prelude std-thread-basic raw source wrapper around context thread id; docs/stdlib/modules/thread.md |
| `fn std::thread::is_finished` | check-prelude std-thread-builder advisory thread completion predicate hook; docs/stdlib/modules/thread.md |
| `fn std::thread::is_join_error` | check-prelude std-thread-basic join sentinel helper; docs/stdlib/modules/thread.md |
| `fn std::thread::is_main` | check-prelude std-thread-basic source main-thread predicate; docs/stdlib/modules/thread.md |
| `fn std::thread::join` | check-prelude std-thread-basic Result-returning join handle helper; docs/stdlib/modules/thread.md |
| `fn std::thread::join_compat` | check-prelude std-thread-basic Error-returning raw thread join compatibility helper; docs/stdlib/modules/thread.md |
| `fn std::thread::join_thread` | check-prelude std-thread-basic JoinError-returning raw thread join bridge; docs/stdlib/modules/thread.md |
| `fn std::thread::join_thread_value` | check-prelude std-thread-basic typed ThreadResult raw thread join bridge; docs/stdlib/modules/thread.md |
| `fn std::thread::join_unchecked` | check-prelude std-thread-basic raw-status thread join compatibility hook; docs/stdlib/modules/thread.md |
| `fn std::thread::join_value` | check-prelude std-thread-basic typed ThreadResult join handle helper; docs/stdlib/modules/thread.md |
| `fn std::thread::sleep` | check-prelude std-thread-runtime-helpers duration sleep convenience wrapper; docs/stdlib/modules/thread.md |
| `fn std::thread::spawn` | check-prelude std-thread-basic Result-returning function-pointer JoinHandle spawn helper; docs/stdlib/modules/thread.md |
| `fn std::thread::spawn_configured` | check-prelude std-thread-builder Result-returning configured JoinHandle spawn helper; docs/stdlib/modules/thread.md |
| `fn std::thread::spawn_configured_unchecked` | check-prelude std-thread-builder unchecked configured thread spawn hook; docs/stdlib/modules/thread.md |
| `fn std::thread::spawn_unchecked` | check-prelude std-thread-basic unchecked function-pointer spawn hook; docs/stdlib/modules/thread.md |
| `fn std::thread::thread_local[T]` | check-prelude std-thread-local default-capacity thread-local handle constructor; docs/stdlib/modules/thread.md |
| `fn std::thread::thread_local_with_capacity[T]` | check-prelude std-thread-local explicit-capacity thread-local handle constructor; docs/stdlib/modules/thread.md |
| `fn std::thread::yield_now` | check-prelude std-thread-basic scheduler yield hook; docs/stdlib/modules/thread.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::thread::Builder::configured_name` | check-prelude std-thread-builder builder configured name accessor; docs/stdlib/modules/thread.md |
| `method std::thread::Builder::configured_stack_size` | check-prelude std-thread-builder builder stack-size accessor; docs/stdlib/modules/thread.md |
| `method std::thread::Builder::name` | check-prelude std-thread-builder builder name setter; docs/stdlib/modules/thread.md |
| `method std::thread::Builder::new` | check-prelude std-thread-builder builder constructor; docs/stdlib/modules/thread.md |
| `method std::thread::Builder::spawn` | check-prelude std-thread-builder Result-returning JoinHandle builder spawn helper; docs/stdlib/modules/thread.md |
| `method std::thread::Builder::spawn_unchecked` | check-prelude std-thread-builder unchecked builder spawn compatibility method; docs/stdlib/modules/thread.md |
| `method std::thread::Builder::stack_size` | check-prelude std-thread-builder builder stack-size setter; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::detach` | check-prelude std-thread-builder explicit thread detach lifecycle helper; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::from_thread` | check-prelude std-thread-basic raw Thread to JoinHandle bridge; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::id` | check-prelude std-thread-basic join handle id alias; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::invalid` | check-prelude std-thread-basic invalid join handle sentinel; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::is_finished` | check-prelude std-thread-builder advisory join handle completion predicate; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::is_valid` | check-prelude std-thread-basic join handle validity predicate; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::join` | check-prelude std-thread-basic Result-returning join handle lifecycle helper; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::join_value` | check-prelude std-thread-basic typed ThreadResult join handle lifecycle helper; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::thread` | check-prelude std-thread-basic raw thread info accessor; docs/stdlib/modules/thread.md |
| `method std::thread::JoinHandle::thread_id` | check-prelude std-thread-basic join handle thread id accessor; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::current` | check-prelude std-thread-basic associated current thread info helper; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::id` | check-prelude std-thread-basic typed raw thread id accessor; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::id_raw` | check-prelude std-thread-basic raw thread id accessor; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::invalid` | check-prelude std-thread-basic invalid raw thread sentinel; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::is_finished` | check-prelude std-thread-builder advisory raw thread completion predicate; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::is_joinable` | check-prelude std-thread-basic raw thread joinability predicate; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::is_valid` | check-prelude std-thread-basic raw thread validity predicate; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::join` | check-prelude std-thread-basic Result-returning raw thread join bridge; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::join_unchecked` | check-prelude std-thread-basic raw-status raw thread join compatibility method; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::spawn` | check-prelude std-thread-basic associated Result-returning JoinHandle spawn helper; docs/stdlib/modules/thread.md |
| `method std::thread::Thread::spawn_unchecked` | check-prelude std-thread-basic associated unchecked spawn compatibility method; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadId::as_i64` | check-prelude std-thread-basic typed thread id raw accessor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadId::current` | check-prelude std-thread-basic associated current thread id helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadId::equals` | check-prelude std-thread-basic typed thread id equality helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadId::from_raw` | check-prelude std-thread-basic typed thread id raw constructor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadId::is_main` | check-prelude std-thread-basic typed main-thread id predicate; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadId::is_valid` | check-prelude std-thread-basic typed thread id validity predicate; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::capacity` | check-prelude std-thread-local thread-local slot capacity accessor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::get` | check-prelude std-thread-local current-thread shared value view; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::get_mut` | check-prelude std-thread-local current-thread mutable value view; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::get_or_init` | check-prelude std-thread-local current-thread lazy initialization helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::get_or_try_init` | check-prelude std-thread-local Result-returning current-thread lazy initialization helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::is_full` | check-prelude std-thread-local thread-local capacity exhaustion predicate; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::is_initialized` | check-prelude std-thread-local current-thread initialized predicate; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::len` | check-prelude std-thread-local initialized slot count accessor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::new` | check-prelude std-thread-local default-capacity thread-local constructor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::remaining_capacity` | check-prelude std-thread-local remaining slot capacity accessor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::remove` | check-prelude std-thread-local current-thread value removal alias; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::set` | check-prelude std-thread-local current-thread value replacement helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::take` | check-prelude std-thread-local current-thread value extraction helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::try_set` | check-prelude std-thread-local Result-returning current-thread value replacement helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadLocal[T]::with_capacity` | check-prelude std-thread-local explicit-capacity thread-local constructor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadResult::as_i64` | check-prelude std-thread-basic typed thread result raw accessor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadResult::equals` | check-prelude std-thread-basic typed thread result equality helper; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadResult::from_i64` | check-prelude std-thread-basic typed thread result constructor; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadResult::is_failure` | check-prelude std-thread-basic typed thread result failure predicate; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadResult::is_success` | check-prelude std-thread-basic typed thread result success predicate; docs/stdlib/modules/thread.md |
| `method std::thread::ThreadResult::value` | check-prelude std-thread-basic typed thread result value accessor; docs/stdlib/modules/thread.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::thread` | check-prelude std-thread-basic spawn/join module; docs/stdlib/modules/thread.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::thread::Builder` | check-prelude std-thread-builder thread builder options handle; docs/stdlib/modules/thread.md |
| `struct std::thread::JoinHandle` | check-prelude std-thread-basic owning thread join/detach handle; docs/stdlib/modules/thread.md |
| `struct std::thread::Thread` | check-prelude std-thread-basic raw thread info handle; docs/stdlib/modules/thread.md |
| `struct std::thread::ThreadId` | check-prelude std-thread-basic typed runtime thread id handle; docs/stdlib/modules/thread.md |
| `struct std::thread::ThreadLocal[T]` | check-prelude std-thread-local explicit thread-local storage handle; docs/stdlib/modules/thread.md |
| `struct std::thread::ThreadResult` | check-prelude std-thread-basic typed thread return value wrapper; docs/stdlib/modules/thread.md |

### type

| API | Coverage note |
| --- | --- |
| `type std::thread::Error` | check-prelude std-thread-basic shared thread error alias; docs/stdlib/modules/thread.md |
| `type std::thread::ErrorKind` | check-prelude std-thread-basic shared thread error-kind alias; docs/stdlib/modules/thread.md |

## `std::time`

Tier: `hosted`. Stability reading: platform-backed.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::time::days_in_month` | check-prelude std-time-utc-calendar UTC calendar month length helper; docs/stdlib/modules/time.md |
| `fn std::time::deadline_at` | check-prelude std-time-timeout absolute deadline constructor; docs/stdlib/modules/time.md |
| `fn std::time::elapsed` | check-prelude std-time-basic source elapsed-time helper; docs/stdlib/modules/time.md |
| `fn std::time::is_leap_year` | check-prelude std-time-utc-calendar proleptic Gregorian leap-year helper; docs/stdlib/modules/time.md |
| `fn std::time::microseconds` | check-prelude std-time-basic source duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::milliseconds` | check-prelude std-time-basic source duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::monotonic_nanos` | check-prelude std-time-basic monotonic clock hook; docs/stdlib/modules/time.md |
| `fn std::time::nanoseconds` | check-prelude std-time-basic source duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::now` | check-prelude std-time-basic source monotonic instant helper; docs/stdlib/modules/time.md |
| `fn std::time::seconds` | check-prelude std-time-basic source duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::sleep` | check-prelude std-time-basic source duration sleep helper; docs/stdlib/modules/time.md |
| `fn std::time::sleep_nanos` | check-prelude std-time-basic runtime sleep hook; docs/stdlib/modules/time.md |
| `fn std::time::system_from_unix` | check-prelude std-time-utc-calendar deterministic SystemTime constructor; docs/stdlib/modules/time.md |
| `fn std::time::system_now` | check-prelude std-time-basic source wall-clock helper; docs/stdlib/modules/time.md |
| `fn std::time::timeout` | check-prelude std-time-timeout timeout deadline constructor; docs/stdlib/modules/time.md |
| `fn std::time::timeout_after` | check-prelude std-time-timeout named timeout deadline constructor; docs/stdlib/modules/time.md |
| `fn std::time::try_days_in_month` | check-prelude std-time-calendar-validation Option-returning month-length helper; docs/stdlib/modules/time.md |
| `fn std::time::try_microseconds` | check-prelude std-time-try-duration Option-returning duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::try_milliseconds` | check-prelude std-time-try-duration Option-returning duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::try_nanoseconds` | check-prelude std-time-try-duration Option-returning duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::try_seconds` | check-prelude std-time-try-duration Option-returning duration constructor; docs/stdlib/modules/time.md |
| `fn std::time::try_system_from_unix` | check-prelude std-time-try-unix Option-returning SystemTime Unix constructor; docs/stdlib/modules/time.md |
| `fn std::time::try_utc_from_unix` | check-prelude std-time-try-unix Option-returning UTC Unix conversion; docs/stdlib/modules/time.md |
| `fn std::time::unix_nanos` | check-prelude std-time-basic wall-clock hook; docs/stdlib/modules/time.md |
| `fn std::time::utc_from_unix` | check-prelude std-time-utc-calendar UTC calendar conversion helper; docs/stdlib/modules/time.md |

### method

| API | Coverage note |
| --- | --- |
| `method std::time::Deadline::after` | check-prelude std-time-timeout associated timeout constructor; docs/stdlib/modules/time.md |
| `method std::time::Deadline::at` | check-prelude std-time-timeout associated absolute deadline constructor; docs/stdlib/modules/time.md |
| `method std::time::Deadline::has_expired` | check-prelude std-time-timeout deadline expiry predicate; docs/stdlib/modules/time.md |
| `method std::time::Deadline::instant` | check-prelude std-time-timeout deadline instant accessor; docs/stdlib/modules/time.md |
| `method std::time::Deadline::remaining` | check-prelude std-time-timeout remaining timeout helper; docs/stdlib/modules/time.md |
| `method std::time::Deadline::sleep` | check-prelude std-time-timeout sleep until deadline helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::add` | check-prelude std-time-basic source duration addition helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::as_micros` | check-prelude std-time-basic duration metadata helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::as_millis` | check-prelude std-time-basic duration metadata helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::as_nanos` | check-prelude std-time-basic duration metadata helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::as_seconds` | check-prelude std-time-basic duration metadata helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::is_zero` | check-prelude std-time-basic duration predicate helper; docs/stdlib/modules/time.md |
| `method std::time::Duration::saturating_sub` | check-prelude std-time-basic non-negative duration subtraction; docs/stdlib/modules/time.md |
| `method std::time::Duration::try_microseconds` | check-prelude std-time-try-duration associated Option-returning duration constructor; docs/stdlib/modules/time.md |
| `method std::time::Duration::try_milliseconds` | check-prelude std-time-try-duration associated Option-returning duration constructor; docs/stdlib/modules/time.md |
| `method std::time::Duration::try_nanoseconds` | check-prelude std-time-try-duration associated Option-returning duration constructor; docs/stdlib/modules/time.md |
| `method std::time::Duration::try_seconds` | check-prelude std-time-try-duration associated Option-returning duration constructor; docs/stdlib/modules/time.md |
| `method std::time::Duration::zero` | check-prelude std-time-basic associated zero duration constructor; docs/stdlib/modules/time.md |
| `method std::time::Instant::add` | check-prelude std-time-timeout instant plus duration helper; docs/stdlib/modules/time.md |
| `method std::time::Instant::as_nanos` | check-prelude std-time-basic instant metadata helper; docs/stdlib/modules/time.md |
| `method std::time::Instant::duration_since` | check-prelude std-time-basic elapsed duration helper; docs/stdlib/modules/time.md |
| `method std::time::Instant::elapsed` | check-prelude std-time-basic current elapsed duration helper; docs/stdlib/modules/time.md |
| `method std::time::Instant::now` | check-prelude std-time-basic associated monotonic instant constructor; docs/stdlib/modules/time.md |
| `method std::time::Instant::saturating_duration_since` | check-prelude std-time-timeout nonnegative instant difference helper; docs/stdlib/modules/time.md |
| `method std::time::Instant::try_duration_since` | check-prelude std-time-basic Option-returning elapsed duration helper; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::as_unix_nanos` | check-prelude std-time-basic wall-clock metadata helper; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::as_unix_seconds` | check-prelude std-time-utc-calendar wall-clock seconds helper; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::duration_since_unix_epoch` | check-prelude std-time-basic wall-clock duration wrapper; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::from_unix` | check-prelude std-time-utc-calendar associated deterministic SystemTime constructor; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::now` | check-prelude std-time-basic associated wall-clock constructor; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::subsec_nanos` | check-prelude std-time-utc-calendar subsecond nanosecond helper; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::to_utc` | check-prelude std-time-utc-calendar UTC calendar conversion method; docs/stdlib/modules/time.md |
| `method std::time::SystemTime::try_from_unix` | check-prelude std-time-try-unix associated Option-returning SystemTime Unix constructor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::day` | check-prelude std-time-utc-calendar UTC day accessor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::from_unix` | check-prelude std-time-try-unix associated UTC Unix conversion; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::hour` | check-prelude std-time-utc-calendar UTC hour accessor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::is_leap_year` | check-prelude std-time-utc-calendar UTC leap-year predicate method; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::minute` | check-prelude std-time-utc-calendar UTC minute accessor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::month` | check-prelude std-time-utc-calendar UTC month accessor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::nanosecond` | check-prelude std-time-utc-calendar UTC nanosecond accessor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::second` | check-prelude std-time-utc-calendar UTC second accessor; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::try_from_unix` | check-prelude std-time-try-unix associated Option-returning UTC Unix conversion; docs/stdlib/modules/time.md |
| `method std::time::UtcDateTime::year` | check-prelude std-time-utc-calendar UTC year accessor; docs/stdlib/modules/time.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::time` | std time monotonic/wall-clock helper tests; docs/stdlib/modules/time.md |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::time::Deadline` | check-prelude std-time-timeout monotonic deadline helper; docs/stdlib/modules/time.md |
| `struct std::time::Duration` | std time duration helper tests; docs/stdlib/modules/time.md |
| `struct std::time::Instant` | std time monotonic instant tests; docs/stdlib/modules/time.md |
| `struct std::time::SystemTime` | std time wall-clock tests; docs/stdlib/modules/time.md |
| `struct std::time::UtcDateTime` | check-prelude std-time-utc-calendar UTC calendar value; docs/stdlib/modules/time.md |

## `std::vec`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::vec::alloc_buffer[T]` | std vec allocation tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::vec::collect[T, I: std::Iterator[T]` | check-prelude std-iter-adapters zone-backed iterator collection; docs/stdlib/modules/iter.md |
| `fn std::vec::from_slice_in[T]` | std vec from-slice target-zone copy tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::vec::new[T]` | std vec handle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::vec::with_capacity[T]` | std RawVec tests; docs/dev/test-matrix.md Explicit memory zones row |

### method

| API | Coverage note |
| --- | --- |
| `method std::vec::Vec[T]::append` | check-prelude std-vec-convenience-api vector append move helper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::as_mut_ptr` | check-prelude std-vec-as-mut-ptr mutable borrowed receiver with reset invalidation; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::as_ptr` | check-prelude std-vec-as-ptr borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::as_slice` | check-prelude std-vec-as-slice; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::binary_search` | check-prelude std-vec-sequence ordered sequence binary search wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::binary_search_by` | check-prelude std-algo-by-helpers comparator sequence binary search wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::capacity` | check-prelude std-vec-metadata-methods borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::chunks` | check-prelude std-vec-sequence borrowed chunk iterator wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::clear` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::compare` | check-prelude std-vec-sequence lexicographic borrowed view compare wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::contains` | check-prelude std-vec-fixed-ops borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::contains_slice` | check-prelude std-vec-sequence borrowed subsequence predicate wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::copy_from` | check-prelude std-vec-sequence owned target prefix copy wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::copy_to` | check-prelude std-vec-copy-to borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::copy_within` | check-prelude std-vec-range-mutation owned vector overlap-safe in-place range copy; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::count` | check-prelude std-vec-fixed-ops borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::dedup` | check-prelude std-vec-sequence owned consecutive duplicate compaction and truncation; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::dedup_by` | check-prelude std-algo-dedup-partition owned comparator dedup and truncation; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::dedup_by_key[K]` | check-prelude std-algo-dedup-partition owned key-based dedup and truncation; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::drain` | check-prelude std-vec-convenience-api whole-vector draining cursor; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::drain_range` | check-prelude std-vec-range-mutation half-open vector draining cursor; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::ends_with` | check-prelude std-vec-slice-compare borrowed receiver Slice[T] tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::equal_range` | check-prelude std-vec-sequence ordered equal-range wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::equal_range_by` | check-prelude std-algo-by-helpers comparator equal-range wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::equals` | check-prelude std-vec-slice-compare borrowed receiver Slice[T] tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::extend` | check-prelude std-vec-convenience-api natural slice extension alias; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::extend_from_slice` | check-prelude std-vec-growth-paths owning-zone slice extension; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::extend_from_slice_in` | check-prelude std-vec-extend-from-slice-in; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::extend_iter[I: std::Iterator[T]` | check-prelude std-iter-slice-vec iterator-driven vector extension; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::fill` | check-prelude std-vec-sequence owned live-prefix fill wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::fill_range` | check-prelude std-vec-range-mutation owned vector half-open range fill; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::find` | check-prelude std-vec-sequence borrowed subsequence search wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::first` | check-prelude std-vec-fixed-ops borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::get` | check-prelude std-vec-fixed-ops borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::get_mut` | check-prelude std-vec-get-ref-mut mutable element borrow view; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::get_ref` | check-prelude std-vec-get-ref-mut shared element borrow view; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::index_of` | check-prelude std-vec-fixed-ops borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::insert` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::insert_in` | check-prelude std-vec-insert-in; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::insert_many` | check-prelude std-vec-convenience-api range insertion from slice; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::is_empty` | check-prelude std-vec-metadata-methods borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::is_sorted` | check-prelude std-vec-sequence ordered sequence sortedness wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::is_sorted_by` | check-prelude std-algo-by-helpers comparator sequence sortedness wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::iter` | check-prelude std-vec-iter borrowed receiver and Iterator lowering; docs/dev/test-matrix.md Explicit memory zones and Control flow rows |
| `method std::vec::Vec[T]::iter_mut` | check-prelude std-iter-slice-vec mutable value cursor over vector storage; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::last` | check-prelude std-vec-fixed-ops borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::len` | check-prelude std-vec-metadata-methods borrowed receiver; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::lower_bound` | check-prelude std-vec-sequence ordered sequence lower-bound wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::lower_bound_by` | check-prelude std-algo-by-helpers comparator sequence lower-bound wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::max` | check-prelude std-vec-sequence ordered sequence maximum wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::max_by` | check-prelude std-algo-by-helpers comparator sequence maximum wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::min` | check-prelude std-vec-sequence ordered sequence minimum wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::min_by` | check-prelude std-algo-by-helpers comparator sequence minimum wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::ordering` | check-prelude std-vec-sequence lexicographic Ordering comparison wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::partition` | check-prelude std-vec-sequence owned borrowed-predicate partition wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::partition_point` | check-prelude std-vec-sequence partition-point wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::pop` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::push` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::push_in` | check-prelude std-vec-push-in; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::remove` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::remove_range` | check-prelude std-vec-convenience-api in-place vector range removal; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::replace` | check-prelude std-vec-replace; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::reserve` | check-prelude std-vec-reserve owning-zone capacity growth; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::reserve_extra` | check-prelude std-vec-reserve-extra owning-zone spare capacity growth; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::reserve_extra_in` | check-prelude std-vec-reserve-extra explicit-zone compatibility growth; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::reserve_in` | check-prelude std-vec-reserve explicit-zone compatibility growth; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::resize` | check-prelude std-vec-growth-paths owning-zone resize growth and shrink; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::resize_in` | check-prelude std-vec-resize-in; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::resize_with` | check-prelude std-vec-resize-with generator-based vector growth and shrink; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::retain` | check-prelude std-vec-retain stable in-place predicate filtering; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::reverse` | check-prelude std-vec-sequence in-place sequence reversal wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::reverse_range` | check-prelude std-vec-range-mutation owned vector half-open range reverse; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::rotate_left` | check-prelude std-vec-sequence in-place sequence rotation wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::rotate_range` | check-prelude std-vec-range-mutation owned vector half-open range left rotation; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::rotate_right` | check-prelude std-vec-sequence in-place sequence rotation wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::set` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::shrink_to_fit` | check-prelude std-vec-convenience-api logical capacity shrink to length; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::slice` | check-prelude std-vec-sequence borrowed range view wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::sort` | check-prelude std-vec-sequence ordered in-place sort wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::sort_by` | check-prelude std-algo-by-helpers comparator in-place sort wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::splice` | check-prelude std-vec-convenience-api in-place vector range replacement; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::split` | check-prelude std-vec-sequence borrowed delimiter split iterator wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::split_at` | check-prelude std-vec-sequence borrowed split view wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::split_off` | check-prelude std-vec-complete-convenience-api owned tail split into a same-zone vector; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::stable_partition` | check-prelude std-algo-dedup-partition stable borrowed-predicate partition wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::stable_sort` | check-prelude std-vec-sequence ordered in-place stable sort wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::stable_sort_by` | check-prelude std-algo-by-helpers comparator in-place stable sort wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::stable_sort_by_in` | check-prelude std-algo-final-sort comparator vector stable sort with explicit temporary zone; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::stable_sort_in` | check-prelude std-algo-final-sort ordered vector stable sort with explicit temporary zone; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::starts_with` | check-prelude std-vec-slice-compare borrowed receiver Slice[T] tests; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::swap` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::swap_remove` | check-prelude std-vec-complete-convenience-api unordered O(1) indexed removal; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::truncate` | check-prelude std-vec-fixed-ops; docs/dev/test-matrix.md Explicit memory zones row |
| `method std::vec::Vec[T]::try_first` | check-prelude std-vec-try-access Option-returning empty-safe access; docs/stdlib/api-reference.md Vec section |
| `method std::vec::Vec[T]::try_get` | check-prelude std-vec-try-access Option-returning out-of-range access; docs/stdlib/api-reference.md Vec section |
| `method std::vec::Vec[T]::try_last` | check-prelude std-vec-try-access Option-returning empty-safe access; docs/stdlib/api-reference.md Vec section |
| `method std::vec::Vec[T]::try_pop` | check-prelude std-vec-try-pop Option-returning empty pop; docs/dev/test-matrix.md Explicit memory zones and Prelude rows |
| `method std::vec::Vec[T]::try_remove` | check-prelude std-vec-try-remove Option-returning indexed removal; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::try_reserve` | check-prelude std-vec-convenience-api fallible-looking vector reserve validation; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::try_stable_sort` | check-prelude std-algo-final-sort direct Result vector stable sort wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::try_stable_sort_by` | check-prelude std-algo-final-sort comparator direct Result vector stable sort wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::upper_bound` | check-prelude std-vec-sequence ordered sequence upper-bound wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::upper_bound_by` | check-prelude std-algo-by-helpers comparator sequence upper-bound wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::windows` | check-prelude std-vec-sequence borrowed window iterator wrapper; docs/stdlib/modules/vec.md |
| `method std::vec::Vec[T]::with_capacity` | check-prelude std-vec-convenience-api associated explicit-capacity constructor; docs/stdlib/modules/vec.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::vec` | std vec handle tests; docs/dev/test-matrix.md Explicit memory zones row |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::vec::Drain[T]` | check-prelude std-vec-convenience-api draining cursor tests; docs/stdlib/modules/vec.md |
| `struct std::vec::Iter[T]` | std vec iterator tests; docs/dev/test-matrix.md Explicit memory zones and Control flow rows |
| `struct std::vec::RawVec[T]` | std RawVec allocation tests; docs/dev/test-matrix.md Explicit memory zones row |
| `struct std::vec::Vec[T]` | std vec handle tests; docs/dev/test-matrix.md Explicit memory zones row |

## `std::zone`

Tier: `alloc`. Stability reading: usable.

### fn

| API | Coverage note |
| --- | --- |
| `fn std::zone::alloc` | zone allocation tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::alloc[T]` | typed zone allocation tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::alloc_array[T]` | std zone raw array allocation tests; docs/stdlib/modules/zone.md |
| `fn std::zone::allocation_zone` | zone allocation header metadata tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::create` | zone lifecycle tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::destroy` | zone lifecycle and invalidation tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::from_zone` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `fn std::zone::metadata` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `fn std::zone::new[T]` | placement construction tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::of[T: std::zone::ZoneBacked]` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `fn std::zone::promote[T]` | scratch promotion tests; docs/dev/test-matrix.md Explicit memory zones row |
| `fn std::zone::reset` | zone reset invalidation tests; docs/dev/test-matrix.md Explicit memory zones row |

### method

| API | Coverage note |
| --- | --- |
| `method std::zone::ZoneMetadata::alloc` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `method std::zone::ZoneMetadata::alloc_array[T]` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `method std::zone::ZoneMetadata::as_ptr` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `method std::zone::ZoneMetadata::as_zone_ptr` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
| `method std::zone::ZoneMetadata::equals` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |

### module

| API | Coverage note |
| --- | --- |
| `module std::zone` | zone lifecycle tests; docs/dev/test-matrix.md Explicit memory zones row |

### struct

| API | Coverage note |
| --- | --- |
| `struct std::zone::ZoneMetadata` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |

### trait

| API | Coverage note |
| --- | --- |
| `trait std::zone::ZoneBacked` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |

### trait-method

| API | Coverage note |
| --- | --- |
| `trait-method std::zone::ZoneBacked::zone` | std zone backed handle metadata tests; docs/stdlib/modules/zone.md |
