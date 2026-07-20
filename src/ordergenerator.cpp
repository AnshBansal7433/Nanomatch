#include <cstdio>
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <cmath>

int main(int argc, char* argv[]) {
    if (argc < 3) return 1;

    FILE* file = fopen(argv[1], "w");
    if (!file) return 1;

    int num_orders = std::stoi(argv[2]);
    fprintf(file, "timestamp,order_id,msg_type,side,price,qty\n");

    std::unordered_map<int, int> active_registry;
    std::vector<int> active_ids;

    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<int> qty_dist(5, 50);

    // Buy Price: Triangle peaking at 100 (Range 95 to 100)
    auto get_buy_price = [&]() {
        double u = dist(rng);
        return 95 + (int)(5 * std::sqrt(u)); 
    };

    // Sell Price: Triangle peaking at 97 (Range 97 to 102)
    auto get_sell_price = [&]() {
        double u = dist(rng);
        return 102 - (int)(5 * std::sqrt(u));
    };

    for (int i = 1; i <= num_orders; ++i) {
        double action = dist(rng);

        // 15% Cancel
        if (action < 0.15 && !active_ids.empty()) {
            int idx = (int)(dist(rng) * active_ids.size());
            int id_to_cancel = active_ids[idx];
            fprintf(file, "1000,%d,C,B,0,0\n", id_to_cancel);
            active_ids.erase(active_ids.begin() + idx);
        } 
        // 15% Market ('M')
        else if (action < 0.30) {
            char side = (dist(rng) > 0.5) ? 'B' : 'S';
            fprintf(file, "1000,%d,M,%c,0,%d\n", i, side, qty_dist(rng));
        }
        // 70% Limit ('A')
        else {
            bool isBuy = (dist(rng) > 0.5);
            int price = isBuy ? get_buy_price() : get_sell_price();
            fprintf(file, "1000,%d,A,%c,%d,%d\n", i, isBuy ? 'B' : 'S', price, qty_dist(rng));
            active_ids.push_back(i);
        }

        // Cleanup: Cap active order list to prevent infinite growth
        if (active_ids.size() > 5000) {
            active_ids.erase(active_ids.begin());
        }

    }

    fclose(file);
    printf("Done! CSV generated.\n");
    return 0;
}