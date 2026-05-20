DeclarationIndex source=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari modules=1 declarations=7
  Decl module=<root> kind=enum name=Severity visibility=pub loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:8:10 generics=[] cases=[Error, Warning]
  Decl module=<root> kind=extern-C-fn name=puts visibility=private loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:23:15 generics=[] params=[text:string] return=i32 body=false
  Decl module=<root> kind=fn name=main visibility=private loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:25:4 generics=[] params=[] return=i64 body=true
  Decl module=<root> kind=struct name=Span visibility=pub loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:3:12 generics=[] tuple=false fields=[start:i64, end:i64]
  Decl module=<root> kind=trait name=Named visibility=pub loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:13:11 generics=[] associated_types=[] methods=[name([value:Span])->i64{decl}]
  Decl module=<root> kind=trait-impl name=Named for Span visibility=pub loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:17:10 generics=[] associated_witnesses=[] methods=[name([value:Span])->i64{body}]
  Decl module=<root> kind=type name=SourceId visibility=pub loc=tests/cases/compiler-development/artifact/ok/declaration-index-basic.ari:1:10 generics=[] target=i64
