# AL Compiler
Welcome to my project! The goal is to write a compiler without using any libraries (outside of the C++ STL). This will involve hand-rolling a lexer, parser, and assembly code generator.

Arithmetic Language is a functional language. AL is intended to be powerful enough to express complex programs. It is inspired by other functional languages, especially OCaml and Haskell.

The parser uses LL(1) parsing combined with iteration, which helps avoid some convoluted left-factored grammars. It also produces gcc-like error messages.

Game plan:

- `[X]` Implement lexer and parser

- `[X]` Write an inefficient but correct interpreter that straightforwardly applies Lambda calculus rules to evaluate expressions

- `[ ]` Algebraic Data Types (sum, product, unit type, etc.)

- `[ ]` Add type inference and compile-time type checking

- `[ ]` Add 'let rec f x ...' and 'let f arg1 arg2 arg3 ...' syntax sugar for function declarations

- `[ ]` Type functions and generics

- `[ ]` Allow type aliasing (ex. type int_t = int)

- `[ ]` Module system

- `[ ]` Implement lists as syntax sugar for Cons/Nil type

- `[ ]` Support chars; string types as syntax sugar for a list of chars

- `[ ]` Support imperative data types (arrays, refs, etc)

- `[ ]` Add print/read IO as built-in functions

- `[ ]` Generate bytecode targeting a simple ISA (ex. MIPS)

- `[ ]` Implement optimizations on bytecode (ex. tail recursion, common subexpression elimination, etc.)

- `[ ]` Self-Hosting AL Compiler (reach goal!)

## Grammar

### Tokens
* IntLit : `[0-9]+`
* BoolLit : `true` | `false`
* Ident : `(A-Za-z_)[A-Za-z0-9_']*`
* Comments are opened with `(*`, closed with `*)`, and may be nested. Ex. `(* (* I am a comment *) and I am one too *)`

### Productions
```
<Program> ::= (<TypeDecl>)* <Expr>

<Expr> ::= Ident
         | IntLit
         | BoolLit
         | <EUnitLit>
         | <ERecordLit>
         | <ELet>
         | <EIf>
         | <EFun>
         | <EFix>
         | <EFunAp>
         | <EUnaryOp>
         | <EBinOp>
         | <EMatch>
         | <EFieldAccess>
         | '(' <Expr> ')'
         | <Expr> ':' <EType>

<EUnitLit> ::= '(' ')'

<ERecordLit> ::= '{' Ident '=' <Expr> (',' Ident '=' <Expr>)* '}' // TODO

<EType> ::= Ident
          | <EType> '->' <EType>
          | <EType> '*' <EType>
          | '(' <EType> ')'

<ELet> ::= 'let' <Expr> '=' <Expr> 'in' <Expr>

<EIf> ::= 'if' <Expr> 'then' <Expr> 'else' <Expr>

<EFun> ::= 'fun' <Expr> '->' <Expr>

<EFix> ::= 'fix' <Expr> '->' <Expr>

<EFunAp> ::= <Expr> <Expr>

<EUnaryOp> ::= '-' <Expr>
             | '!' <Expr>

<EBinOp> ::= <Expr> '=' <Expr>
           | <Expr> '!=' <Expr>
           | <Expr> '<' <Expr>
           | <Expr> '>' <Expr>
           | <Expr> '<=' <Expr>
           | <Expr> '>=' <Expr>
           | <Expr> '&&' <Expr>
           | <Expr> '||' <Expr>
           | <Expr> '+' <Expr>
           | <Expr> '-' <Expr>
           | <Expr> '*' <Expr>
           | <Expr> '/' <Expr>
           | <Expr> '%' <Expr>

<EMatch> ::= 'match' <Expr> 'with' ('|')? <MatchCase> ('|' <MatchCase>)* // TODO

<MatchCase> ::= Ident (<MatchCaseBinding>)? '->' <Expr>

<MatchCaseBinding> ::= Ident
                     | '(' Ident ')'

<EFieldAccess> ::= <Expr> '.' Ident // TODO

<TypeDecl> ::= 'type' Ident '=' ('|')? <VariantCaseDecl> ('|' <VariantCaseDecl>)* ';'
             | 'type' Ident '=' '{' <RecordFieldDecl> (',' <RecordFieldDecl>)* '}' ';'

<VariantCaseDecl> ::= Ident
                    | Ident <EType>

<RecordFieldDecl> ::= Ident ':' <EType>

TODO: Variant constructors (FunAp on an ident and an arg), Record literals, pattern-matching, field-access, etc..

```
