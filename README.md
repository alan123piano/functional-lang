# AL Compiler
Arithmetic Language is a minimal, Turing-complete functional language.

## Grammar

### Tokens
* Ident : `(A-Za-z_)[A-Za-z0-9_']*`
* IntLit : `[0-9]+`
* Keyword : `let` | `in` | `if` | `then` | `else` | `fun`
* Symbol : `=` | `!=` | `!` | `<` | `>` | `<=` | `>=` | `+` | `-` | `*` | `/` | `%` | `(` | `)` | `->`
* Comments are opened with `(*`, closed with `*)`, and may be nested. Ex. `(* (* I am a comment *) and I am one too *)`

### Productions
```
<Expr> ::= <ELet>
         | <EIf>
         | <EFun>
         | <EFunAp>
         | <EUnaryOp>
         | <EBinOp>
         | IntLit
         | Ident
         | '(' <Expr> ')'

<ELet> ::= 'let' Ident '=' <Expr> 'in' <Expr>

<EIf> ::= 'if' <Expr> 'then' <Expr> 'else' <Expr>

<EFun> ::= 'fun' Ident '->' <Expr>

<EFunAp> ::= <Expr> <Expr>

<EUnaryOp> ::= '+' <Expr>
             | '-' <Expr>
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

## Features
* Lexer
* Parser

## Planned features
* Interpreter
* Algebraic Data Types (unit type, sums, products)
* Implementing lists as a built-in syntax sugar for Cons/Nil
* Type inference (Meta Language)
