#include<cstdio>
#include<map>
#include<algorithm>
#include<vector>
using namespace std;

enum ModifierType {
    STAT_MODIFIER,
    TYPE_MODIFIER,
    COST_MODIFIER,
    IDENTITY_MODIFIER,
    REROLL_MODIFIER,
    UNDEFINED,
};

struct Modifier {
    int identifier; // 0~26

    ModifierType type = UNDEFINED;
    int stat_modifier_amount = 0; // -1~5
    int stat_modifier_index = -1; // 0~3
    int type_modifier_index = -1; // 0,1
    int cost_modifier_amount = 0; // +-1
    int reroll_modifier_amount = 0; // 1~2

    const bool operator<(const Modifier& rhs) const {
        return identifier < rhs.identifier;
    }

    void print() const {
        printf("Identifier: %d\n", identifier);
        printf("Type: %d\n", type);
        printf("Stat amount/index: %d/%d\n", stat_modifier_amount, stat_modifier_index);
        printf("Type modifier index: %d\n", type_modifier_index);
        printf("Cost modifier amount: %d\n", cost_modifier_amount);
        printf("Reroll modifier amount: %d\n", reroll_modifier_amount);
    }
};

typedef long double probtype;
const int GEM_STAT_NUM = 4;
const int NUM_MODIFIERS = 27;
vector<pair<Modifier,probtype>> MODIFIERS;

// Initializes MODIFIERS vector.
void init_modifiers(){
    int identifier = 0;
    for(int stat_modifier_index = 0; stat_modifier_index < GEM_STAT_NUM; stat_modifier_index++) {
        Modifier modifier;
        modifier.type = STAT_MODIFIER;
        modifier.stat_modifier_index = stat_modifier_index;

        static const vector<pair<int, probtype>> stat_modifier_constants = {
            {1, 0.1165},
            {2, 0.0440},
            {3, 0.0175},
            {4, 0.0045},
            {-1, 0.03}};

        for(const auto& stat_modifier: stat_modifier_constants) {
            modifier.identifier = identifier++;
            modifier.stat_modifier_amount = stat_modifier.first;
            MODIFIERS.push_back({modifier, stat_modifier.second});
        }
    }

    for(int type_modifier_index = 0; type_modifier_index < 2; type_modifier_index++) {
        Modifier modifier;
        modifier.identifier = identifier++;
        modifier.type = TYPE_MODIFIER;
        modifier.type_modifier_index = type_modifier_index;
        MODIFIERS.push_back({modifier, 0.0325});
    }

    for(int cost_modifier_amount = -1; cost_modifier_amount <= 1; cost_modifier_amount++) {
        if (cost_modifier_amount == 0) continue;
        Modifier modifier;
        modifier.type = COST_MODIFIER;
        modifier.identifier = identifier++;
        modifier.cost_modifier_amount = cost_modifier_amount;
        MODIFIERS.push_back({modifier, 0.0175});
    }

    {
        Modifier modifier;
        modifier.type = IDENTITY_MODIFIER;
        modifier.identifier = identifier++;
        MODIFIERS.push_back({modifier, 0.0175});
    }

    static const vector<pair<int, probtype>> reroll_modifier_constants = {
        {1, 0.0250},
        {2, 0.0075}};
    for (const auto& reroll_modifier: reroll_modifier_constants) {
        Modifier modifier;
        modifier.type = REROLL_MODIFIER;
        modifier.identifier = identifier++;
        modifier.reroll_modifier_amount = reroll_modifier.first;
        MODIFIERS.push_back({modifier, reroll_modifier.second});
    }

    // Verification.
    printf("Total modifiers: %lu\n", MODIFIERS.size());
    probtype tot_prob = 0;
    for (int i=0; i<NUM_MODIFIERS; i++) {
        MODIFIERS[i].first.print();
        printf("Prob: %.10Lf%%\n\n", MODIFIERS[i].second*100);
        tot_prob += MODIFIERS[i].second;
    }
    printf("Total Prob: %.10Lf\n\n", tot_prob);
}

struct Gem {
    vector<int> stat;
    int charge;
    int cost;
    int reroll;

    Gem(): stat(GEM_STAT_NUM, 1), charge(0), cost(0), reroll(0) {}

    const bool operator< (const Gem& rhs) const {
        if (stat != rhs.stat) return stat<rhs.stat;
        if (charge != rhs.charge) return charge<rhs.charge;
        if (cost != rhs.cost) return cost<rhs.cost;
        return reroll < rhs.reroll;
    }

    void print() const {
        printf("Stat: %d %d %d %d\n", stat[0], stat[1], stat[2], stat[3]);
        printf("Charge/Cost/Reroll: %d/%d/%d\n", charge, cost, reroll);
        printf("\n");
    }
};

bool verify_modifier(const Gem& gem, const int modifier_id) {
    const Modifier& modifier = MODIFIERS[modifier_id].first;
    if (modifier.type == TYPE_MODIFIER || modifier.type == IDENTITY_MODIFIER) return true;
    if (modifier.type == STAT_MODIFIER) {
        int next_stat = gem.stat[modifier.stat_modifier_index] + modifier.stat_modifier_amount;
        if (next_stat <= 0 || next_stat >= 6) return false;
        else return true;
    }
    if (modifier.type == COST_MODIFIER) {
        if (gem.charge == 1) return false;
        if (gem.cost == modifier.cost_modifier_amount) return false;
        return true;
    }
    if (modifier.type == REROLL_MODIFIER) {
        if (gem.charge == 1) return false;
        return true;
    }

    // Unreachable
    return true;
}

Gem modify(const Gem& gem, const int modifier_id) {
    const Modifier& modifier = MODIFIERS[modifier_id].first;
    Gem ret = gem;
    ret.charge--;
    if (modifier.type == STAT_MODIFIER) {
        ret.stat[modifier.stat_modifier_index] += modifier.stat_modifier_amount;
    }
    else if (modifier.type == COST_MODIFIER) {
        ret.cost += modifier.cost_modifier_amount;
    }
    else if (modifier.type == REROLL_MODIFIER) {
        ret.reroll += modifier.reroll_modifier_amount;
    }

    return ret;
}

vector<pair<vector<int>, probtype>> Explore(const Gem& gem) {
    probtype base_residue = 1;
    for(int i=0; i<NUM_MODIFIERS; i++) if (!verify_modifier(gem, i)) base_residue -= MODIFIERS[i].second;

    vector<pair<vector<int>, probtype>> ret;
    // TODO: Generalize?
    for(int a=0; a<NUM_MODIFIERS; a++) if (verify_modifier(gem, a))
        for(int b=a+1; b<NUM_MODIFIERS; b++) if (verify_modifier(gem, b))
            for(int c=b+1; c<NUM_MODIFIERS; c++) if (verify_modifier(gem, c))
                for(int d=c+1; d<NUM_MODIFIERS; d++) if (verify_modifier(gem, d)) {
        probtype prob = 0;
        vector<int> modifiers = {a, b, c, d};
        do {
            probtype residue = base_residue;

            probtype cur = MODIFIERS[modifiers[0]].second / residue;
            residue -= MODIFIERS[modifiers[0]].second;

            cur *= MODIFIERS[modifiers[1]].second / residue;
            residue -= MODIFIERS[modifiers[1]].second;

            cur *= MODIFIERS[modifiers[2]].second / residue;
            residue -= MODIFIERS[modifiers[2]].second;

            cur *= MODIFIERS[modifiers[3]].second / residue;

            prob += cur;
        } while (next_permutation(modifiers.begin(), modifiers.end()));

        ret.push_back(make_pair(modifiers, prob));
    }

    return ret;
}

void test_explore() {
    Gem gem;
    gem.stat = {5, 5, 5, 5};
    gem.charge = 1;
    gem.cost = 1;
    auto ret = Explore(gem);

    probtype tot_prob = 0;
    for(const auto& mod_prob: ret) {
        tot_prob += mod_prob.second;
    }
    for (const auto& mod_prob: ret) {
        for (const auto& mod: mod_prob.first) printf("%d\t", mod);
        printf("%.10Lf", mod_prob.second);
        printf("\n");
    }
    printf("Exp size: %lu\n", ret.size());
    printf("Tot prob: %.10Lf\n", tot_prob);
}

probtype Foo(const Gem& gem, const vector<int>& stat_objective, bool can_reroll, map<Gem, probtype>& data);
probtype Bar(const Gem& gem, const vector<int>& stat_objective, bool can_reroll, const vector<int>& modifier_list, map<Gem, probtype>& data);

probtype Foo(const Gem& gem, const vector<int>& stat_objective, bool can_reroll, map<Gem, probtype>& data) {
    // Check Cache first.
    if (data.find(gem) != data.end()) return data[gem];

    // Check Boundary conditions.
    bool done = true;
    for(int i=0; i<GEM_STAT_NUM; i++) if (gem.stat[i] < stat_objective[i]) done = false;
    if (done) return data[gem] = 1.;
    if (gem.charge == 0) return data[gem] = 0.;

    probtype& ret = data[gem];
    ret = 0;

    vector<pair<vector<int>, probtype>> candidates = Explore(gem);
    for(const auto& cand: candidates) {
        ret += Bar(gem, stat_objective, can_reroll, cand.first, data) * cand.second;
    }

    return ret;
}

probtype Bar(const Gem& gem, const vector<int>& stat_objective, bool can_reroll, const vector<int>& modifier_list, map<Gem, probtype>& data) {
    // Two options: reroll, process
    
    // Process
    probtype process_prob = 0;
    for(int modifier_id: modifier_list) {
        process_prob += 0.25 * Foo(modify(gem, modifier_id), stat_objective, can_reroll, data);
    }

    // Reroll
    probtype reroll_prob = 0;
    if (can_reroll && gem.reroll > 0) {
        Gem reroll_gem = gem;
        reroll_gem.reroll--;
        reroll_prob = Foo(reroll_gem, stat_objective, can_reroll, data);
    }

    return max(process_prob, reroll_prob);
}

void test_modifiers() {
    Gem gem;
    gem.charge = 7;
    gem.reroll = 1;
    gem.print();

    for (int i=0; i<NUM_MODIFIERS; i++) modify(gem, i).print();
}

map<pair<vector<int>, bool>, map<Gem, probtype>> Database;
void Test(int charge, int reroll, const vector<int>& stat_objective, bool can_reroll) {
    Gem gem;
    gem.charge = charge; gem.reroll = reroll;

    printf("=== Preparing data ===\n");
    printf("=== Charge %d, Reroll %d ===\n", charge, reroll);
    printf("=== Target "); for (int t: stat_objective) printf("%d ", t); printf("===\n");
    printf("=== Use reroll? "); printf(can_reroll ? "Y" : "N"); printf(" ===\n");
    fflush(stdout);
    map<Gem, probtype>& data = Database[make_pair(stat_objective, can_reroll)];
    Foo(gem, stat_objective, can_reroll, data);
    printf("Done.\n");
    fflush(stdout);
}

// Solve if we can't reroll at first turn.
probtype RandomSolve(int charge, int reroll, const vector<int>& stat_objective, bool can_reroll) {
    Gem gem;
    gem.charge = charge; gem.reroll = reroll;

    printf("=== Solving for random gem ===\n");
    printf("=== Charge %d, Reroll %d ===\n", charge, reroll);
    printf("=== Target "); for (int t: stat_objective) printf("%d ", t); printf("===\n");
    printf("=== Use reroll? "); printf(can_reroll ? "Y" : "N"); printf(" ===\n");
    fflush(stdout);

    probtype ret = 0;
    map<Gem, probtype>& data = Database[make_pair(stat_objective, can_reroll)];
    vector<pair<vector<int>, probtype>> candidates = Explore(gem);
    for (const auto& cand: candidates) {
        for (const int modifier_id: cand.first) {
            ret += 0.25 * Foo(modify(gem, modifier_id), stat_objective, can_reroll, data) * cand.second;
        }
    }

    printf("Prob: %.10Lf\n", ret);
    fflush(stdout);
    return ret;
}

int main() {
    init_modifiers();
    /*
    test_modifiers();
    test_explore();

    fflush(stdout);
    */

    Test(7, 1, {1,1,4,4}, false);
    Test(9, 2, {1,1,4,4}, false);
    Test(7, 1, {1,1,4,5}, false);
    Test(9, 2, {1,1,4,5}, false);
    Test(7, 1, {1,1,5,5}, false);
    Test(9, 2, {1,1,5,5}, false);

    Test(7, 1, {1,1,4,4}, true);
    Test(9, 2, {1,1,4,4}, true);
    Test(7, 1, {1,1,4,5}, true);
    Test(9, 2, {1,1,4,5}, true);
    Test(7, 1, {1,1,5,5}, true);
    Test(9, 2, {1,1,5,5}, true);


    RandomSolve(7, 1, {1,1,4,4}, false);
    RandomSolve(9, 2, {1,1,4,4}, false);
    RandomSolve(7, 1, {1,1,4,5}, false);
    RandomSolve(9, 2, {1,1,4,5}, false);
    RandomSolve(7, 1, {1,1,5,5}, false);
    RandomSolve(9, 2, {1,1,5,5}, false);

    RandomSolve(7, 1, {1,1,4,4}, true);
    RandomSolve(9, 2, {1,1,4,4}, true);
    RandomSolve(7, 1, {1,1,4,5}, true);
    RandomSolve(9, 2, {1,1,4,5}, true);
    RandomSolve(7, 1, {1,1,5,5}, true);
    RandomSolve(9, 2, {1,1,5,5}, true);
}
