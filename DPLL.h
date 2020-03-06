//
// DPLL algorithm.
//

#ifndef DPLL_DPLL_H
#define DPLL_DPLL_H

#include "common.h"

struct ImplicationGraph {
    std::vector<int> decisions;
    std::vector<int> parity;
    std::vector<std::vector<bool>> adjacent;
    int last_atom;
    int num_variables;

    explicit ImplicationGraph(int num_variables) : last_atom(0), num_variables(num_variables) {
        decisions.resize(num_variables + 1);
        adjacent.resize(num_variables + 1);
        parity.resize(num_variables + 1);
        for (int i = 0; i <= num_variables; i++) {
            decisions[i] = 0;
            parity[i] = 0;
            adjacent[i].resize(num_variables + 1);
            for (int j = 0; j <= num_variables; j++) {
                adjacent[i][j] = false;
            }
        }
    }

    void connect(literal from, literal to) {
        adjacent[VAR(from)][VAR(to)] = true;
        last_atom = VAR(to);
    }

    void setDecision(literal l, int d) {
        decisions[VAR(l)] = d;
        parity[VAR(l)] = l;
    }

    void span(const clause &c, literal unit) {
        int latest_decision = 0;
        for (literal l: c) {
            if (VAR(l) != VAR(unit)) {
                connect(l, unit);
                if (decisions[VAR(l)] > latest_decision) {
                    latest_decision = decisions[VAR(l)];
                }
            }
        }
        setDecision(unit, latest_decision);
    }

    void clear(int i) {
        for (int j = 0; j <= num_variables; j++) {
            adjacent[i][j] = false;
            adjacent[j][i] = false;
        }
        decisions[i] = 0;
        parity[i] = 0;
    }

    void tidyup(const std::vector<literal> &m) {
        std::vector<bool> appear;
        appear.resize(num_variables + 1);
        for (int i = 0; i <= num_variables; i++) {
            appear[i] = false;
        }
        for (literal l: m) {
            appear[VAR(l)] = true;
        }
        for (int i = 1; i <= num_variables; i++) {
            if (!appear[i]) {
                clear(i);
            }
        }
    }

    std::pair<int, int> find_reason() {
        std::stack<int> s;
        s.push(last_atom);
        std::vector<bool> visited;
        visited.resize(num_variables + 1);
        for (int i = 0; i <= num_variables; i++) {
            visited[i] = false;
        }
        std::vector<int> ans;
        while (!s.empty()) {
            int x = s.top();
            s.pop();
            bool hasPrev = false;
            for (int i = 1; i <= num_variables; i++) {
                if (i != x && adjacent[i][x]) {
                    hasPrev = true;
                    if (!visited[i]) {
                        s.push(i);
                        visited[i] = true;
                    }
                }
            }
            if (!hasPrev) {
                ans.push_back(x);
            }
        }
        if (ans.size() >= 2) {
            if (decisions[ans[0]] >= decisions[ans[1]]) {
                return std::make_pair(ans[0], ans[1]);
            } else {
                return std::make_pair(ans[1], ans[0]);
            }
        } else {
            return std::make_pair(0, 0);
        }
    }
};


class Interpretation {
private:
    interp decision;
    atoms remain;

    void remove_remain(int x) {
        auto iter = std::remove(remain.begin(), remain.end(), x);
        remain.erase(iter, remain.end());
    }

    void assign_(literal l) {
        decision.push_back(l);
        remove_remain(VAR(l));
    }

    int state(literal l) {
        for (literal d: decision) {
            if (l == d) {
                return 1;
            }
            if (-l == d) {
                return -1;
            }
        }
        return 0;
    }

    bool belongs(literal l) {
        return (state(l) == 1);
    }

    bool satisfy(const clause &c) {
        for (literal l: c) {
            if (belongs(l)) {
                return true;
            }
        }
        return false;
    }

    bool unsatisfy(const clause &c) {
        for (literal l: c) {
            if (!belongs(-l)) {
                return false;
            }
        }
        return true;
    }

    bool unsatisfy_backjump(const clause &c, ImplicationGraph &g) {
        for (literal l: c) {
            if (!belongs(-l)) {
                return false;
            }
        }
        g.span(c, g.last_atom);
        return true;
    }

    bool unassigned(literal l) {
        return (!state(l));
    }

    literal check_unit(const clause &c) {
        int num_unassigned = 0;
        literal unit = 0;
        for (literal l: c) {
            if (unassigned(l)) {
                num_unassigned++;
                unit = l;
            }
            if (num_unassigned > 1) {
                return 0;
            }
            if (state(l) == 1) {
                return 0;
            }
        }
        if (num_unassigned == 1) {
            return unit;
        }
        return 0;
    }

    literal check_unit_backjump(const clause &c, ImplicationGraph &g) {
        int num_unassigned = 0;
        literal unit = 0;
        for (literal l: c) {
            if (unassigned(l)) {
                num_unassigned++;
                unit = l;
            }
            if (num_unassigned > 1) {
                return 0;
            }
            if (state(l) == 1) {
                return 0;
            }
        }
        if (num_unassigned == 1) {
            g.span(c, unit);
            return unit;
        }
        return 0;
    }

    void pop_() {
        literal removed = decision.back();
        decision.pop_back();
        remain.push_back(VAR(removed));
    }

public:
    explicit Interpretation(int num_variable = 0) {
        init(num_variable);
    }

    void init(int num_variable) {
        remain.resize(num_variable);
        for (int i = 0; i < num_variable; i++) {
            remain[i] = i + 1;
        }
    }

    Interpretation assign(literal l) {
        Interpretation new_interp = *this;
        new_interp.assign_(l);
        return new_interp;
    }

    Interpretation pop() {
        Interpretation new_interp = *this;
        new_interp.pop_();
        return new_interp;
    }

    int first_atom() {
        if (remain.empty()) {
            throw std::logic_error("no remaining atom");
        }
        return remain[0];
    }

    bool satisfy(const formula &f) {
        for (const clause &c: f.clauses) {
            if (!satisfy(c)) {
                return false;
            }
        }
        return true;
    }

    bool unsatisfy(const formula &f) {
        for (const clause &c: f.clauses) {
            if (unsatisfy(c)) {
                return true;
            }
        }
        return false;
    }

    bool unsatisfy_backjump(const formula &f, ImplicationGraph &g) {
        for (const clause &c: f.clauses) {
            if (unsatisfy_backjump(c, g)) {
                return true;
            }
        }
        return false;
    }

    literal check_unit(const formula &f) {
        for (const clause &c: f.clauses) {
            literal result = check_unit(c);
            if (result != 0) {
                return result;
            }
        }
        return 0;
    }

    literal check_unit_backjump(const formula &f, ImplicationGraph &g) {
        for (const clause &c: f.clauses) {
            literal result = check_unit_backjump(c, g);
            if (result != 0) {
                return result;
            }
        }
        return 0;
    }

    model export_model() {
        model answer;
        for (literal l: decision) {
            answer.insert(std::make_pair(VAR(l), POSITIVE(l)));
        }
        return answer;
    }

    std::vector<literal> &getDecision() {
        return decision;
    }

    literal back() {
        return decision.back();
    }

    bool exhausted() {
        return remain.empty();
    }
};

class DPLL {
public:
    /**
     * Constructor.
     *
     * @param phi the formula to be checked
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    explicit DPLL(formula phi);

    /**
     * Check if the formula is satisfiable.
     *
     * @return true if satisfiable, and false if unsatisfiable
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    bool check_sat();

    /**
     * Get a satisfying model (interpretation) of the formula, the model must be *complete*, that is,
     * it must assign every variable a truth value.
     * This function will be called if and only if `check_sat()` returns true.
     *
     * @return an arbitrary (if there exist many) satisfying model
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    model get_model();

private:
    formula phi;
    Interpretation answer;
    ImplicationGraph gamma;

    bool dfs(Interpretation d);

    bool dfs_stack();

    bool dfs_backjump(Interpretation d, literal last_decide, int decide_level);
};


#endif //DPLL_DPLL_H
