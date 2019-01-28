#include <iostream>
#include <map>
#include <random>
#include <time.h>
#include <vector>
#include <z3++.h>

// -1 2 4
// 2 -6
// 4 6

bool EE(z3::model &model, int v, std::map<int, z3::func_decl> &decls) {
  z3::expr b = model.get_const_interp(decls.at(v));
  return b.bool_value() == Z3_L_TRUE;
}

int main() {
  srand(time(NULL));
  z3::context c;

  // create variables
  std::vector<int> vars{1, 2, 4, 6};
  std::map<int, z3::expr> exprs;
  std::map<int, z3::func_decl> decls;

  for (int v : vars) {
    z3::expr l =
        c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
    exprs.insert(std::pair<int, z3::expr>{v, l});
    decls.insert(std::pair<int, z3::func_decl>{v, l.decl()});
  }

  // create the forms
  z3::optimize opt(c);
  {
    z3::expr_vector V(c);
    V.push_back(!exprs.at(1));
    V.push_back(exprs.at(2));
    V.push_back(exprs.at(4));
    opt.add(mk_or(V));
  }
  {
    z3::expr_vector V(c);
    V.push_back(exprs.at(2));
    V.push_back(!exprs.at(6));
    opt.add(mk_or(V));
  }
  {
    z3::expr_vector V(c);
    V.push_back(exprs.at(4));
    V.push_back(exprs.at(6));
    opt.add(mk_or(V));
  }

  for (int tt = 0; tt < 100; tt++) {
    // opt.push();

    z3::check_result exists = opt.check();
    if (exists != z3::sat)
      break;
    z3::model m = opt.get_model();
    std::cout << EE(m, 1, decls) << EE(m, 2, decls) << EE(m, 4, decls)
              << EE(m, 6, decls) << std::endl;

    z3::expr_vector V(c);
    for (int v : vars)
      V.push_back(EE(m, v, decls) ? !exprs.at(v) : exprs.at(v));

    opt.add(mk_or(V));

    // opt.pop();
  }

  return 0;
}
