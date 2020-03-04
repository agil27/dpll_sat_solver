//
// DPLL algorithm.
//

#ifndef DPLL_DPLL_H
#define DPLL_DPLL_H

#include "common.h"

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

public:
    explicit Interpretation(int num_variable) {
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

    literal check_unit(const formula &f) {
        for (const clause &c: f.clauses) {
            literal result = check_unit(c);
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

    bool dfs(const formula &f, Interpretation d);
};


#endif //DPLL_DPLL_H
