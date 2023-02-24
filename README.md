# AL Compiler
Welcome to my project! The goal is to write a compiler without using any libraries (outside of the C++ STL). This will involve hand-rolling a lexer, parser, and assembly code generator.

Arithmetic Language is a functional language. AL is intended to be powerful enough to express complex programs. It is inspired by other functional languages, especially OCaml and Haskell.

The parser uses LL(1) parsing combined with iteration, which helps avoid some convoluted left-factored grammars. It also produces gcc-like error messages.

Game plan:

- `[X]` Implement lexer and parser

- `[X]` Add type inference and compile-time type checking

- `[X]` Write an interpreter that straightforwardly applies Lambda calculus rules to evaluate expressions

- `[ ]` Algebraic Data Types (sum, product, unit type, etc.)

- `[ ]` Add 'let rec f x ...' and 'let f arg1 arg2 arg3 ...' syntax sugar for function declarations

- `[ ]` Type functions and generics

- `[ ]` Allow type aliasing (ex. type int_t = int)

- `[ ]` Module system

- `[ ]` Implement lists as syntax sugar for Cons/Nil type

- `[ ]` Support chars; string types as syntax sugar for a list of chars

- `[ ]` Support imperative data types (arrays, refs, etc)

- `[ ]` Add print/read IO as built-in functions

- `[ ]` Generate bytecode targeting a simple ISA (ex. LLVM)

- `[ ]` Implement optimizations on bytecode (ex. tail recursion, common subexpression elimination, etc.)

- `[ ]` Self-Hosting AL Compiler (reach goal!)

## Grammar

### Tokens
* IntLit : `[0-9]+`
* FloatLit : `[0-9]+(.)[0-9]*(E|e)(+|-)?[0-9]+`
* BoolLit : `true` | `false`
* Ident : `(A-Za-z_)[A-Za-z0-9_']*`
* Comments are opened with `(*`, closed with `*)`, and may be nested. Ex. `(* (* I am a comment *) and I am one too *)`

### Productions
```
<Program> ::= (<TypeDecl>)* <Expr>

<Expr> ::= Ident
         | IntLit
         | FloatLit
         | BoolLit
         | <EUnitLit>
         | '(' <Expr> ')'
         | <ETupleLit>
         | <ERecordLit>
         | <ELet>
         | <EIf>
         | <EFun>
         | <EFunAp>
         | <EUnaryOp>
         | <EBinaryOp>
         | <EMatch>
         | <EFieldAccess>

<EUnitLit> ::= '(' ')'

<ETupleLit> ::= '(' <Expr> (',' <Expr>)+ ')'

<ERecordLit> ::= '{' Ident '=' <Expr> (',' Ident '=' <Expr>)* '}'

// TODO
<EVar> ::= Ident
         | '(' Ident ':' <EType> ')'

<EType> ::= Ident
          | <EType> '->' <EType>
          | <EType> '*' <EType>
          | '(' <EType> ')'

// TODO: EVar, function expressions
<ELet> ::= 'let' ('rec')? <ELetBody> ('and' <ELetBody>)*

<ELetBody> ::= (<EVar>)+ (':' <EType>)? '=' <Expr> 'in' <Expr>

<EIf> ::= 'if' <Expr> 'then' <Expr> 'else' <Expr>

<EFun> ::= 'fun' <EVar> '->' <Expr> // TODO

<EFunAp> ::= <Expr> <Expr>

<EUnaryOp> ::= '!' <Expr>   // bool -> bool
             | '-' <Expr>   // int -> int
             | '-.' <Expr>  // float -> float

<EBinaryOp> ::= <Expr> '=' <Expr>   // 'a -> 'a -> bool
              | <Expr> '!=' <Expr>  // 'a -> 'a -> bool
              | <Expr> '<' <Expr>   // 'a -> 'a -> bool
              | <Expr> '>' <Expr>   // 'a -> 'a -> bool
              | <Expr> '<=' <Expr>  // 'a -> 'a -> bool
              | <Expr> '>=' <Expr>  // 'a -> 'a -> bool
              | <Expr> '&&' <Expr>  // bool -> bool -> bool
              | <Expr> '||' <Expr>  // bool -> bool -> bool
              | <Expr> '+' <Expr>   // int -> int -> int
              | <Expr> '-' <Expr>   // int -> int -> int
              | <Expr> '*' <Expr>   // int -> int -> int
              | <Expr> '/' <Expr>   // int -> int -> int
              | <Expr> '%' <Expr>   // int -> int -> int
// TODO
              | <Expr> '+.' <Expr>   // float -> float -> float
              | <Expr> '-.' <Expr>   // float -> float -> float
              | <Expr> '*.' <Expr>   // float -> float -> float
              | <Expr> '/.' <Expr>   // float -> float -> float

// TODO
<EMatch> ::= 'match' <Expr> 'with' ('|')? <MatchCase> ('|' <MatchCase>)*

<MatchCase> ::= Ident (<MatchCaseBinding>)? '->' <Expr>

<MatchCaseBinding> ::= Ident
                     | '(' Ident ')'

// TODO
<EFieldAccess> ::= <Expr> '.' Ident

<TypeDecl> ::= 'type' Ident '=' ('|')? <VariantCaseDecl> ('|' <VariantCaseDecl>)* ';'
             | 'type' Ident '=' '{' <RecordFieldDecl> (',' <RecordFieldDecl>)* '}' ';'

<VariantCaseDecl> ::= Ident
                    | Ident <EType>

<RecordFieldDecl> ::= Ident ':' <EType>

TODO: Variant constructors (FunAp on an ident and an arg), Record literals, pattern-matching, field-access, etc..

```
