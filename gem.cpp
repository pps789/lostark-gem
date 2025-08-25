#include<cstdio>
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
const int NUM_MODIFIERS = 27;
vector<pair<Modifier,probtype>> MODIFIERS;

// Initializes MODIFIERS vector.
void init_modifiers(){
    int identifier = 0;
    for(int stat_modifier_index = 0; stat_modifier_index < 4; stat_modifier_index++) {
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
        modifier.cost_modifier_amount = reroll_modifier.first;
        MODIFIERS.push_back({modifier, reroll_modifier.second});
    }

    // Verification.
    printf("Total modifiers: %d\n", MODIFIERS.size());
    for (int i=0; i<NUM_MODIFIERS; i++) {
        MODIFIERS[i].first.print();
        printf("Prob: %.10Lf%%\n\n", MODIFIERS[i].second*100);
    }
}

int main() {
    init_modifiers();
}
