//
// DPLL algorithm.
//

#include "DPLL.h"

#include <utility>

DPLL::DPLL(formula phi) : phi(std::move(phi)), answer(Interpretation(phi.num_variable)) {}

bool DPLL::check_sat() {
    //return dfs(answer);
    return dfs_stack();
}

model DPLL::get_model() {
    return answer.export_model();
}

bool DPLL::dfs_stack() {
    std::stack<Interpretation> s;
    s.push(answer);
    while (!s.empty()) {
        Interpretation d = s.top();
        s.pop();
        if (d.satisfy(phi)) {
            answer = d;
            return true;
        }
        if (d.unsatisfy(phi)) {
            continue;
        }
        literal unit = d.check_unit(phi);
        if (unit) {
            if (DEBUG) {
                printf("[trace] found unit %d\n", unit);
            }
            s.push(d.assign(unit));
            continue;
        }
        literal split = d.first_atom();
        if (DEBUG) {
            printf("[trace] split on %d\n", split);
        }
        s.push(d.assign(-split));
        s.push(d.assign(split));
    }
    return false;
}

bool DPLL::dfs(Interpretation d) {
    if (d.satisfy(phi)) {
        answer = d;
        return true;
    }
    if (d.unsatisfy(phi)) {
        return false;
    }
    literal unit = d.check_unit(phi);
    if (unit) {
        if (DEBUG) {
            printf("[trace] found unit %d\n", unit);
        }
        return dfs(d.assign(unit));
    }
    literal split = d.first_atom();
    if (DEBUG) {
        printf("[trace] split on %d\n", split);
    }
    return (dfs(d.assign(split)) || dfs(d.assign(-split)));
}

