DeclarationIndex source=tests/cases/generics/ok/generic-aggregate-monomorphization.ari modules=1 declarations=33 resolver_surface=imports,uses,declarations order=module-kind-name-location-text
  Decl module=<root> kind=enum name=AggregateEnvelope visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:64:6 generics=[T, E] cases=[Empty, Item(Pair[T, Maybe[E]]), State(ParseFrame[T, E])]
  Decl module=<root> kind=enum name=Either visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:18:6 generics=[L, R] cases=[Left(L), Right(R)]
  Decl module=<root> kind=enum name=Maybe visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:13:6 generics=[T] cases=[Nothing, Just(T)]
  Decl module=<root> kind=enum name=PassResult visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:23:6 generics=[T, E] cases=[Failed(E), Passed(T)]
  Decl module=<root> kind=fn name=consume_owned visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:187:4 generics=[] params=[value:Box[own i64]] return=i64 body=true
  Decl module=<root> kind=fn name=either_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:114:4 generics=[] params=[value:Either[LexError, Token]] return=i64 body=true
  Decl module=<root> kind=fn name=envelope_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:162:4 generics=[] params=[value:AggregateEnvelope[Token, Diagnostic]] return=i64 body=true
  Decl module=<root> kind=fn name=frame_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:154:4 generics=[] params=[frame:ParseFrame[Token, Diagnostic]] return=i64 body=true
  Decl module=<root> kind=fn name=main visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:194:4 generics=[] params=[] return=i64 body=true
  Decl module=<root> kind=fn name=make_owned visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:183:4 generics=[] params=[value:i64] return=own i64 body=true
  Decl module=<root> kind=fn name=make_token visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:95:4 generics=[] params=[kind:i64, start:i64, end:i64] return=Token body=true
  Decl module=<root> kind=fn name=make_token_box visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:99:4 generics=[] params=[kind:i64] return=TokenBox body=true
  Decl module=<root> kind=fn name=maybe_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:103:4 generics=[] params=[value:Maybe[Token]] return=i64 body=true
  Decl module=<root> kind=fn name=mutate_slot visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:179:4 generics=[] params=[slot:ref mut AstSlot[Token]] return=void body=true
  Decl module=<root> kind=fn name=nested_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:136:4 generics=[] params=[value:NestedAlias[Token, Diagnostic]] return=i64 body=true
  Decl module=<root> kind=fn name=pass_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:125:4 generics=[] params=[value:PassAlias[AstSlot[Token], Diagnostic]] return=i64 body=true
  Decl module=<root> kind=fn name=span_len visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:87:4 generics=[] params=[span:Span] return=i64 body=true
  Decl module=<root> kind=fn name=token_score visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:91:4 generics=[] params=[token:Token] return=i64 body=true
  Decl module=<root> kind=impl name=Box[T] visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:75:9 generics=[T] associated_witnesses=[] methods=[unwrap([self:Self])->T{body}]
  Decl module=<root> kind=impl name=Pair[A, B] visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:81:12 generics=[A, B] associated_witnesses=[] methods=[right_value([self:Self])->B{body}]
  Decl module=<root> kind=struct name=AstSlot visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:53:8 generics=[T] tuple=false fields=[id:mut i64, payload:Maybe[T]]
  Decl module=<root> kind=struct name=Box visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:4:8 generics=[T] tuple=false fields=[value:T]
  Decl module=<root> kind=struct name=Diagnostic visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:43:8 generics=[] tuple=false fields=[primary:Span, severity:i64]
  Decl module=<root> kind=struct name=LexError visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:38:8 generics=[] tuple=false fields=[span:Span, code:i64]
  Decl module=<root> kind=struct name=Pair visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:8:8 generics=[A, B] tuple=false fields=[left:A, right:B]
  Decl module=<root> kind=struct name=ParseFrame visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:58:8 generics=[T, E] tuple=false fields=[current:ParserBox[T], previous:Maybe[AstSlot[T]], status:PassResult[AstSlot[T], E]]
  Decl module=<root> kind=struct name=ParserBox visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:48:8 generics=[T] tuple=false fields=[cursor:i64, value:T]
  Decl module=<root> kind=struct name=Span visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:28:8 generics=[] tuple=false fields=[start:i64, end:i64]
  Decl module=<root> kind=struct name=Token visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:33:8 generics=[] tuple=false fields=[kind:i64, span:Span]
  Decl module=<root> kind=type name=NestedAlias visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:73:6 generics=[T, E] target=PassResult[Box[T], Maybe[E]]
  Decl module=<root> kind=type name=PassAlias visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:72:6 generics=[T, E] target=PassResult[T, E]
  Decl module=<root> kind=type name=SlotAlias visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:71:6 generics=[T] target=AstSlot[T]
  Decl module=<root> kind=type name=TokenBox visibility=private loc=tests/cases/generics/ok/generic-aggregate-monomorphization.ari:70:6 generics=[] target=Box[Token]
