# AL Compiler
Welcome to my project! The goal is to write a compiler without using any libraries (outside of the C++17 STL). This will involve hand-rolling a lexer, parser, and assembly code generator.

Arithmetic Language is a minimal functional language. AL is intended to be simple, but also powerful enough to elegantly express non-trivial programs. It is inspired by other functional languages including OCaml and Haskell.

The parser uses Pratt parsing (a very cool technique that combines iteration and recursion) to produce an AST without resorting to convoluted LL(1) left-factored grammars. It also produces gcc-like error messages.

Game plan:

- `[X]` Implement lexer and parser

- `[X]` Write an inefficient but correct interpreter that straightforwardly applies Lambda calculus rules to evaluate expressions

- `[ ]` Algebraic Data Types (sum, product, unit type, etc.)

- `[ ]` Add type inference and compile-time type checking

- `[ ]` Add 'let rec f x ...' and 'let f arg1 arg2 arg3 ...' syntax sugar for function declarations

- `[ ]` Type functions and generics

- `[ ]` Implement lists as syntax sugar for Cons/Nil type

- `[ ]` Support chars; string types as syntax sugar for a list of chars

- `[ ]` Support imperative data types (arrays, refs, etc)

- `[ ]` Add print/read IO as built-in functions

- `[ ]` Generate bytecode targeting a simple ISA (ex. MIPS)

- `[ ]` Implement optimizations on bytecode (ex. tail recursion, common subexpression elimination, etc.)

- `[ ]` Self-Hosting AL Compiler (reach goal!)

## Grammar

### Tokens
* Ident : `(A-Za-z_)[A-Za-z0-9_']*`
* IntLit : `[0-9]+`
* BoolLit : `true` | `false`
* Keyword : `let` | `in` | `if` | `then` | `else` | `fun` | `fix` | `rec`
* Symbol : `=` | `!=` | `!` | `<` | `>` | `<=` | `>=` | `+` | `-` | `*` | `/` | `%` | `(` | `)` | `->`
* Comments are opened with `(*`, closed with `*)`, and may be nested. Ex. `(* (* I am a comment *) and I am one too *)`

### Productions
```
<Expr> ::= <ELet>
         | <EIf>
         | <EFun>
         | <EFix>
         | <EFunAp>
         | <EUnaryOp>
         | <EBinOp>
         | IntLit
         | BoolLit
         | Ident
         | '(' <Expr> ')'

<ELet> ::= 'let' Ident '=' <Expr> 'in' <Expr>

<EIf> ::= 'if' <Expr> 'then' <Expr> 'else' <Expr>

<EFun> ::= 'fun' Ident '->' <Expr>

<EFix> ::= 'fix' Ident '->' <Expr>

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

```
