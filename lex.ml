open Combo
open Fun

type token
  = IDENT of string
  | IF
  | ELSE
  | BBLOCK
  | EBLOCK

let keywords = [
    ("if", IF);
    ("else", ELSE)
  ]

let (<<) p q s =
  match p s with
    None -> None
  | Some (_, l) as e ->
     match q l with
       None -> None
     | Some _ -> e

let isid = function
    'a' .. 'z' -> true
  | 'A' .. 'Z' -> true
  | '0' .. '9' -> true
  | '_' | '\'' -> true
  | _ -> false

let check_keyword s =
  let e = List.assoc_opt (inplode s) keywords in
  match e with
    None -> IDENT (inplode s)
  | Some e -> e

let no_indent t =
  [t], None

let keyword =
  no_indent <$>
    (check_keyword <$>
       (many1 (satisfy isid) << satisfy (negate isid)))


let get_tab n n' =
  let rec times n t =
    match n with
      0 -> []
    | n -> t :: (times (n - 1) t)
  in
  if n = n'
  then ([], None)
  else
    let t = if n > n' then EBLOCK else BBLOCK in
    times (n - n') t, Some n'

let indent n =
  newline *>
    (get_tab n <$>
       (List.length <$> many tab))
