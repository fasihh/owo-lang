import sys, os

if len(sys.argv) < 2:
    print("Usage : py generate_ast.py <src_header_dir>")
    print("Both directories should be relative to path:", os.getcwd())
    exit()

def define_ast(types: list[str], base_name: str) -> None:
    source: list[str] = [
        "template <typename T>\n"
        f"struct {base_name}Visitor: {base_name}VisitorBase {{\n",
        f"\tT result_{base_name.lower()};\n"
        "};\n\n",
        f"struct {base_name} {{\n",
        "\ttemplate <typename T>\n",
        f"\tT accept({base_name}Visitor<T>& visitor) {{\n",
        "\t\tdo_accept(visitor);\n",
        f"\t\treturn visitor.result_{base_name.lower()};\n",
        "\t}\n\n",
        f"\tvirtual void do_accept({base_name}VisitorBase& visitor) = 0;\n",
        f"\t~{base_name}() {{ delete this; }}\n"
        "};\n\n",
    ]
    return define_visitor(types, base_name) + source + sum([define_type(*d_type, base_name) for d_type in types], [])

def define_type(class_name: str, fields, base_name: str, ):
    constructor = f"\t{class_name}() {{}};\n"
    destructor = f"\t~{class_name}() {{}}\n"
    types = ['']
    if fields:
        types = [f"\t{field};\n" for field in fields.split(', ')]
        constructor = f"\t{class_name}({fields}): {", ".join([f"{field.split(' ')[1]}({field.split(' ')[1]})" for field in fields.split(', ')])} {{}};\n"
        destructor = f"\t~{class_name}() {{\n{"".join([f"\t\tdelete {field.split(' ')[1]};\n" if 'vector' not in field else f"\t\tfor (auto val : {field.split(' ')[1]}) delete val.first, delete val.second;\n" if 'pair' in field else f"\t\tfor (auto val : {field.split(' ')[1]}) delete val;\n" for field in fields.split(', ') if '*' in field])}\t}}\n"

    type_source = [
        f"struct {class_name}: {base_name} {{\n",
        *types,
        constructor,
        destructor,
        f"\n\tvoid do_accept({base_name}VisitorBase& visitor) override {{ visitor.visit{class_name}{base_name}(*this); }}\n",
        "};\n\n",
    ]
    return type_source

def define_visitor(types, base_name: str):
    visitor_source = [
        *[f"struct {d_type[0]};\n" for d_type in types],
        f"\nstruct {base_name}VisitorBase {{\n",
        *[f"\tvirtual void visit{d_type[0]}{base_name}({d_type[0]}& {base_name.lower()}) = 0;\n" for d_type in types],
        "};\n\n",
    ]
    return visitor_source

def main():
    src_header_dir: str = sys.argv[1]
    expr_types: list[(str, str)] = [
        ("Binary", "std::unique_ptr<Expr> left, const Token& op, std::unique_ptr<Expr> right"),
        # ("Assign", "Token* name, Expr* value"),
        ("Grouping", "std::unique_ptr<Expr> expression"),
        ("Literal", "std::any value"),
        # ("DoubleLiteral", "double value"),
        # ("StringLiteral", "std::string value"),
        # ("NullLiteral", None),
        # ("BooleanLiteral", "bool value"),
        ("Unary", "const Token& op, std::unique_ptr<Expr> right"),
        # ("Call", "Expr* callee, Token* paren, std::vector<Expr*> args"),
        # ("Variable", "Token* name"),
        # ("Ternary", "Expr* condition, Expr* true_condition, Expr* false_condition")
    ]
    expr_source = define_ast(expr_types, "Expr")
    with open(f"{os.getcwd()}/{src_header_dir}/expr", "w") as f:
        f.writelines(["#pragma once\n", "#include <token>\n#include <vector>\n#include <memory>\n\n"] + expr_source)
    # stmt_types: list[(str, str)] = [
    #     ("Expression", "std::vector<Expr*> expressions"),
    #     ("Print", "std::vector<Expr*> expressions"),
    #     ("Var", "std::vector<std::pair<Token*,Expr*>> variables"),
    #     ("Function", "Token* name, std::vector<Token*> params, std::vector<Stmt*> body"),
    #     ("Block", "std::vector<Stmt*> statements"),
    #     ("If", "Expr* condition, Stmt* then_branch, Stmt* else_branch"),
    #     ("While", "Expr* condition, Stmt* body"),
    #     ("For", "Stmt* initializer, Expr* condition, Stmt* increment, Stmt* body"),
    #     ("Break", None),
    #     ("Continue", None),
    #     ("Return", "Token* keyword, Expr* value"),
    # ]
    # stmt_source = define_ast(stmt_types, "Stmt")
    # with open(f"{os.getcwd().replace('/', '\\')}\\{src_header_dir.replace('/', '\\')}\\stmt.h", "w") as f:
    #     f.writelines(["#pragma once\n", "#include \"token.h\"\n#include \"expr.h\"\n#include <vector>\n\n"] + stmt_source)

if __name__ == '__main__':
    main()