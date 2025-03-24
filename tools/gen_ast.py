from typing import *
import sys, os

HEADERS = """#pragma once
#include <token>
#include <vector>
#include <memory>

"""

# exprs = {
#   "Binary": [("std::unique_ptr<Expr>", "left"), ("std::unique_ptr<Token>", "op"), ("std::unique_ptr<Expr>", "right")],
#   "Grouping": [("std::unique_ptr<Expr>", "expression")],
#   "Literal": [("std::any", "value")],
#   "Unary": [("std::unique_ptr<Token>", "op"), ("std::unique_ptr<Expr>", "right")],
#   "Call": [("std::unique_ptr<Expr>", "callee"), ("std::unique_ptr<Token>", "paren"), ("std::vector<std::unique_ptr<Expr>>", "args")],
#   "Variable": [("std::unique_ptr<Token>", "label")],
#   "Ternary": [("std::unique_ptr<Expr>", "condition"), ("std::unique_ptr<Expr>", "true_case"), ("std::unique_ptr<Expr>", "false_case")]
# }
exprs = {
  "Binary": [("std::unique_ptr<Expr>", "left"), ("Token*", "op"), ("std::unique_ptr<Expr>", "right")],
  "Grouping": [("std::unique_ptr<Expr>", "expression")],
  "Literal": [("std::any", "value")],
  "Unary": [("Token*", "op"), ("std::unique_ptr<Expr>", "right")],
  "Call": [("std::unique_ptr<Expr>", "callee"), ("Token*", "paren"), ("std::vector<std::unique_ptr<Expr>>", "args")],
  "Variable": [("Token*", "label")],
  "Ternary": [("std::unique_ptr<Expr>", "condition"), ("std::unique_ptr<Expr>", "true_case"), ("std::unique_ptr<Expr>", "false_case")]
}

move_f: Callable[[str], str] = lambda s: f"std::move({s})"
special_param = {
  "std::unique_ptr<Expr>": move_f,
  "std::unique_ptr<Token>": move_f,
  "std::vector<std::unique_ptr<Expr>>": move_f,
}

const_f: Callable[[str], str] = lambda s: f"const {s}"
special_type = {
  "Token*": const_f
}

type Member = Tuple[str, str]

def define_types_decl(types: List[str]):
  source = ""
  for tn in types:
    source += f"struct {tn};\n"
  source += "\n"
  return source

def define_visitor(name: str, to_visit: List[str]):
  source = ""
  # defining base class
  source += f"struct {name}VisitorBase {{\n"
  for vn in to_visit:
    source += f"\tvirtual void visit{vn}{name}({vn}& expr) = 0;\n"
  source += "};\n\n"

  # defining main visitor
  source += f"template <typename T>\n"
  source += f"struct {name}Visitor : {name}VisitorBase {{\n"
  source += f"\tT result_{name.lower()};\n"
  source += f"}};\n\n"

  return source

def define_types(name: str, types: Dict[str, List[Member]]):
  def handle_param(m_type: str, m_name: str) -> str:
    return special_param.get(m_type, lambda s: s)(m_name)
  
  def handle_constructor_t(m_type: str) -> str:
    return special_type.get(m_type, lambda s: s)(m_type)

  source = ""
  # defining main class
  source += f"struct {name} {{\n"
  source += f"\ttemplate <typename T>\n"
  source += f"\tT accept({name}Visitor<T>& visitor) {{\n"
  source += f"\t\tdo_accept(visitor);\n"
  source += f"\t\treturn visitor.result_{name.lower()};\n"
  source += f"\t}}\n\n"
  source += f"\tvirtual void do_accept({name}VisitorBase& visitor) = 0;\n"
  source += f"\tvirtual ~{name}() = default;\n"
  source += f"}};\n\n"

  for tn, members in types.items():
    source += f"struct {tn} : {name} {{\n"
    #defining type in class
    source += "\n".join(f"\tconst {m_type} {m_name};" for m_type, m_name in members) + "\n\n"
    # defining constructor parameters
    source += f"\t{tn}({", ".join(f"{handle_constructor_t(m_type)} {m_name}" for m_type, m_name in members)})"
    # defining member init list
    source += f" : {", ".join(f"{m_name}({handle_param(m_type, m_name)})" for m_type, m_name in members)} {{}};\n\n"
    source += f"\tvoid do_accept({name}VisitorBase& visitor) {{ visitor.visit{tn}{name}(*this); }}\n"
    source += f"}};\n\n"

  return source

def main(args: list[str]):
  if len(args) != 2:
    print(f"usage: {args[0]} <include_dir>")
    exit()

  include_dir = args[1]

  source = HEADERS

  source += define_types_decl(list(exprs.keys()))
  source += define_visitor("Expr", list(exprs.keys()))
  source += define_types("Expr", exprs)

  with open(f"{os.getcwd()}/{include_dir}/expr", "w") as f:
    f.write(source)

if __name__ == "__main__":
  main(sys.argv)
