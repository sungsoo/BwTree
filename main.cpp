#include <iostream>
#include <tuple>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include "bwtree.hpp"

using namespace BwTree;

void randomThreadTest() {
    Tree<unsigned long long, unsigned long long> tree;

    std::vector<std::thread> threads;
    constexpr int numberOfThreads = 4;
    constexpr int numberValues = numberOfThreads * 30000;


    std::default_random_engine d;
    std::uniform_int_distribution<unsigned long long> rand(0, std::numeric_limits<unsigned long long>::max());
    std::array<unsigned long long,numberValues> values;
    std::unordered_set<unsigned long long> keys;
    for (int i = 0; i < numberValues; ++i) {
        unsigned long long val;
        do {
            val = rand(d);
        } while (keys.find(val) != keys.end());
        values[i] = val;
    }
    std::size_t start = 0;
    std::size_t delta = numberValues / numberOfThreads;
    const auto &t_values = values;
    for (int i = 0; i < numberOfThreads; ++i) {
        threads.push_back(std::thread([&tree, &t_values, start, delta]() {
            for (int i = start; i < start + delta; ++i) {
                tree.insert(t_values[i], &(t_values[i]));
            }
            for (int i = start; i < start + delta; ++i) {
                auto &v = t_values[i];
                auto r = tree.search(v);
                if (r == nullptr || *r!=v) {
                    std::cout << "wrong value inner!! " << (r== nullptr ? "NULLPTR" : std::to_string(*r)) << " " << v << std::endl;
                }
            }
        }));
        start += delta;
    }

    for (auto &thread : threads) {
        thread.join();
    }
    for (auto &v : values) {
        auto r = tree.search(v);
        if (r == nullptr || *r!=v) {
            std::cout << "wrong value!! " << (r== nullptr ? "NULLPTR" : std::to_string(*r)) << " " << v << std::endl;
        }
    }
    std::cout << "exchange collisions: " << tree.getAtomicCollisions() << std::endl;
    std::cout << "successful consolidation: " << tree.getSuccessfulConsolidate() << std::endl;
    std::cout << "failed consolidation: " << tree.getFailedConsolidate() << std::endl;
    std::cout << "successful split: " << tree.getSuccessfulSplit() << std::endl;
    std::cout << "failed split: " << tree.getFailedSplit() << std::endl;
}

int main() {
    randomThreadTest();
    return 0;
    /**
    * Tasks for next week (3.12.2014)
    * - random test pattern - OK
    * - multi thread access - OK
    * - deconstructor - OK
    * - binary search in nodes - OK
    */
    /**
    * Tasks for next week (11.12.2014)
    * - fix random bugs that occur with threads :( - OK!
    * - manuel split of nodes
    * - consolidate inner nodes as well
    */
    Tree<unsigned long, unsigned> a;
    std::unordered_map<unsigned long, unsigned> map;

    std::vector<unsigned> values;

    std::default_random_engine d;
    std::uniform_int_distribution<int> rand(0, 1000000);
    constexpr int numberValues = 10000;
    for (int i = 0; i < numberValues; ++i) {
        values.push_back(rand(d));
    }

    for (int j = 0; j < numberValues; ++j) {
        a.insert(j, &values[j]);
        map[j] = values[j];
        if (map[j] != *a.search(j)) {
            std::cout << "Wrong result!! result in map:" << map[j] << " result in bw Tree: " << *a.search(j) << std::endl;
        }
    }

    std::vector<unsigned> values2;
    for (int i = 0; i < numberValues; ++i) {
        values2.push_back(rand(d));
    }

    for (int j = 0; j < numberValues; ++j) {
        if (rand(d) < 300000) {
            a.deleteKey(j);
            map.erase(j);
        } else {
            a.insert(j, &values2[j]);
            map[j] = values2[j];
        }
        const auto r1 = map.find(j);
        const auto r2 = a.search(j);
        if ((r1 != map.end() && r2 == nullptr) || (r1 == map.end() && r2 != nullptr) || (r2 != nullptr && map[j] != *r2)) {
            std::cout << "Wrong result!! result in map:" << (r1 == map.end() ? "nullptr" : std::to_string(r1->second)) << " result in bw Tree: " << (r2 == nullptr ? "nullptr" : std::to_string(*r2)) << std::endl;
        }
    }


/*
    unsigned val = 65;
    unsigned val2 = 42;
    unsigned val3 = 67;
    a.insert(6, &val);
    a.insert(2, &val2);
    a.insert(1, &val3);

    auto b = a.search(6);
    std::cout << "result" << *b << std::endl;
    b = a.search(2);
    std::cout << "result" << *b << std::endl;
    b = a.search(1);
    std::cout << "result " << *b << std::endl;

    a.deleteKey(1);
    b = a.search(1);
    std::cout << "result %x" << b << std::endl;
//    std::get<4>();*/
    return 0;
}