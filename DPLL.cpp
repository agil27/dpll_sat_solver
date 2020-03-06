//
// DPLL algorithm.
//

#include "DPLL.h"

#include <utility>

DPLL::DPLL(formula phi) : phi(std::move(phi)), answer(Interpretation(phi.num_variable)), gamma(ImplicationGraph(phi.num_variable)) {}

bool DPLL::check_sat() {
    //return dfs(answer);
    //return dfs_stack();
    return dfs_backjump(answer, 0, 0);
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

bool DPLL::dfs_backjump(Interpretation d, literal last_decide, int decide_level) {
    if (last_decide != 0) {
        gamma.setDecision(last_decide, decide_level);
    }

    if (d.satisfy(phi)) {
        answer = d;
        return true;
    }
    if (d.unsatisfy_backjump(phi, gamma)) {
        auto reason = gamma.find_reason();
        if (reason.first == 0 || reason.second == 0) {
            return false;
        }
        if (DEBUG) {
            printf("[trace] backjump on %d, %d\n", reason.first, reason.second);
        }
        int initial_decide_level = gamma.decisions[reason.second];
        while (gamma.decisions[VAR(d.back())] > initial_decide_level) {
            d = d.pop();
        }
        literal new_assignment = -gamma.parity[reason.first];
        gamma.tidyup(d.getDecision());
        d = d.assign(new_assignment);
        gamma.connect(reason.second, reason.first);
        gamma.setDecision(new_assignment, gamma.decisions[reason.second]);
        return dfs_backjump(d, 0, gamma.decisions[reason.second]);
    }
    literal unit = d.check_unit(phi);
    if (unit) {
        if (DEBUG) {
            printf("[trace] found unit %d\n", unit);
        }
        return dfs_backjump(d.assign(unit), 0, 0);
    }
    literal split = d.first_atom();
    if (DEBUG) {
        printf("[trace] split on %d\n", split);
    }
    return (dfs_backjump(d.assign(split), split, decide_level + 1) || dfs_backjump(d.assign(-split), -split, decide_level + 1));
}