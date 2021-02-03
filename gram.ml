open Lex

type expr
  = Var of string
  | Int of int

type statement
  = Assign of expr * expr
  | If of expr * statement list * (statement list) option
