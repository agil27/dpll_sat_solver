//
// DPLL algorithm.
//

#include "DPLL.h"

#include <utility>

DPLL::DPLL(formula phi) : phi(std::move(phi)), answer(Interpretation(phi.num_variable)), gamma(ImplicationGraph(phi.num_variable)) {}

bool DPLL::check_sat() {
    //return dfs(answer);
    //return dfs_stack();
    return dfs_backjump();
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

bool DPLL::dfs_backjump() {
    std::stack<SearchState> s;
    s.push(SearchState(answer, 0, 0, gamma));
    while (!s.empty()) {
        SearchState state = s.top();
        s.pop();
        Interpretation d = state.interp;
        int decide_level = state.decide_level;
        int last_decide = state.last_decide;
        ImplicationGraph g = state.graph;

        if (last_decide != 0) {
            g.setDecision(last_decide, decide_level);
            if (DEBUG) {
                printf("[trace] decide on %d\n", last_decide);
            }
        }

        if (d.satisfy(phi)) {
            answer = d;
            return true;
        }

        if (d.unsatisfy_backjump(phi, g)) {
            auto reason = g.find_reason();
            if (reason.first == 0 || reason.second == 0) {
                continue;
            }
            if (DEBUG) {
                printf("[trace] backjump on %d, %d\n", reason.first, reason.second);
            }
            while (!s.empty()) {
                SearchState top = s.top();
                if (VAR(top.last_decide) != reason.second) {
                    s.pop();
                } else {
                    break;
                }
            }
            int initial_decide_level = g.decisions[reason.second];
            while (g.decisions[VAR(d.back())] > initial_decide_level) {
                d = d.pop();
            }
            literal new_assignment = -g.parity[reason.first];
            g.tidyup(d.getDecision());
            d = d.assign(new_assignment);
            g.connect(reason.second, reason.first);
            g.setDecision(new_assignment, g.decisions[reason.second]);
            s.push(SearchState(d, 0, g.decisions[reason.second], g));
            continue;
        }

        literal unit = d.check_unit_backjump(phi, g);
        if (unit) {
            if (DEBUG) {
                printf("[trace] found unit %d\n", unit);
            }
            s.push(SearchState(d.assign(unit), 0, decide_level, g));
            continue;
        }

        literal split = d.first_atom();
        s.push(SearchState(d.assign(-split), split, decide_level + 1, g));
        s.push(SearchState(d.assign(split), split, decide_level + 1, g));
    }
    return false;
}