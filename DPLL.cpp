//
// DPLL algorithm.
//

#include "DPLL.h"

#include <utility>

DPLL::DPLL(formula phi) : phi(std::move(phi)), answer(Interpretation(phi.num_variable)) {}

bool DPLL::check_sat() {
    return dfs(phi, answer);
}

model DPLL::get_model() {
    return answer.export_model();
}

bool DPLL::dfs(const formula &f, Interpretation d) {
    if (d.satisfy(f)) {
        answer = d;
        return true;
    }
    if (d.unsatisfy(f)) {
        return false;
    }
    literal unit = d.check_unit(f);
    if (unit) {
        if (DEBUG) {
            printf("[trace] found unit %d\n", unit);
        }
        return dfs(f, d.assign(unit));
    }
    literal split = d.first_atom();
    if (DEBUG) {
        printf("[trace] split on %d\n", split);
    }
    return (dfs(f, d.assign(-split)) || dfs(f, d.assign(split)));
}

