open Lex
open Combo

type expr
  = Var of string
 | Int of int
 [@@deriving show]

type statement
  = Assign of expr * expr
  | If of expr * statement list * (statement list) option
[@@deriving show]


let expr t =
  match t with
    IDENT v :: tl ->
     Some (Var v, tl)
  | _ -> None

let rec statement t =
  (ifs <|> assign) t
and ifs t =
  let else_clause =
    opt None ((fun x -> Some x) <$>
                sym ELSE *> sym BBLOCK *> many1 statement <* sym EBLOCK)
  in
  let ifs =
    (fun x y z -> If (x, y, z))
    <$> sym IF
     *> expr
    <* sym BBLOCK
    <*> many1 statement
    <*  sym EBLOCK
    <*> else_clause
  in
  ifs t
and assign t =
  let assign =
    (fun x y -> Assign (x, y)) <$> expr <* sym EQUAL <*> expr
  in
  assign t
