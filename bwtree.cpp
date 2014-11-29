#include <unordered_map>
#include "bwtree.hpp"
#include <cassert>
namespace BwTree {

    template<typename Key, typename Data>
    Data *Tree<Key, Data>::search(Key key) {
        PID pid = findDataPage(key);
        if (pid == std::numeric_limits<PID>::max()) {
            return nullptr;
        }
        Node<Key, Data> *node = mapping[pid];
        while (node != nullptr) {
            switch (node->type) {
                case PageType::leaf: {
                    auto node2 = static_cast<Leaf<Key, Data> *>(node);
                    for (int i = 0; i < node2->recordCount; ++i) {
                        if (std::get<0>(node2->records[i]) == key) { //TODO with binary search
                            return std::get<1>(node2->records[i]);
                        }
                    }
                    return nullptr;
                }
                case PageType::deltaInsert: {
                    auto node1 = static_cast<DeltaInsert<Key, Data> *>(node);
                    if (std::get<0>(node1->record) == key) {
                        return std::get<1>(node1->record);
                    } else {
                        node = node1->origin;
                        continue;
                    }
                }
                case PageType::deltaDelete: {
                    auto node2 = static_cast<DeltaDelete<Key, Data> *>(node);
                    if (node2->key == key) {
                        return nullptr;
                    }
                    node = node2->origin;
                    continue;
                }
                case PageType::deltaSplit: {
                    auto node1 = static_cast<DeltaSplit<Key, Data> *>(node);
                    /*if (key > node1->key) {
                        node = node1->sidelink;
                        continue;
                    }*/
                    node = node1->origin;
                    continue;
                };
                default: {
                    assert(false);
                }
            }
            node = nullptr;
        }

        return nullptr;
    }


    template<typename Key, typename Data>
    PID Tree<Key, Data>::findDataPage(Key key) {// TODO return memory location as well
        auto nextPID = root;
        std::size_t debugTMPCheck = 0;
        while (nextPID != std::numeric_limits<PID>::max()) {
            if (debugTMPCheck++ > 100) {
                assert(true);
                std::cout << "test" << std::endl;
            }
            std::size_t pageDepth = 0;
            Node<Key, Data> *nextNode = mapping[nextPID];
            while (nextNode != nullptr) {
                ++pageDepth;
                if (pageDepth == 1000) {//TODO save for later
                    consolidatePage(pid);
                }
                switch (nextNode->type) {
                    case PageType::deltaIndex:
                        assert(false);//not implemented
                    case PageType::inner:{
                        auto node1 = static_cast<InnerNode<Key,Data>*>(nextNode);
                        // TODO with binary search
                        for (int i = 0; i < node1->nodeCount; ++i) {
                            if (std::get<0>(node1->nodes[i]) < key){
                                nextNode = nullptr;
                                nextPID = std::get<1>(node1->nodes[i]);
                                continue;
                            }
                        }
                        if (nextNode == nullptr) {
                            continue;
                        }
                        return std::numeric_limits<PID>::max();
                    };
                    case PageType::leaf: {
                        auto node1 = static_cast<Leaf<Key, Data> *>(nextNode);
                        // TODO with binary search
                        for (int i = 0; i < node1->recordCount; ++i) {
                            if (std::get<0>(node1->records[i]) < key){
                                return nextPID;
                            }
                        }
                        return std::numeric_limits<PID>::max();
                    };
                    case PageType::deltaInsert: {
                        auto node1 = static_cast<DeltaInsert<Key, Data> *>(nextNode);
                        if (std::get<0>(node1->record) == key) {
                            return nextPID;
                        }
                        nextNode = node1->origin;
                        assert(nextNode != nullptr);
                        continue;
                    };
                    case PageType::deltaDelete: {
                        auto node1 = static_cast<DeltaDelete<Key, Data> *>(nextNode);
                        if (node1->key == key) {
                            return std::numeric_limits<PID>::max();
                        }
                        nextNode = node1->origin;
                        assert(nextNode != nullptr);
                        continue;
                    };
                    case PageType::deltaSplit: {
                        auto node1 = static_cast<DeltaSplit<Key, Data> *>(nextNode);
                        if (key > node1->key) {
                            nextPID = node1->sidelink;
                            continue;
                        }
                        nextNode = node1->origin;
                        assert(nextNode != nullptr);
                        continue;
                        return nextPID;//TODO traverse further? - yes
                    };
                    default: {
                        assert(false); // not implemented
                    }
                }
                nextNode = nullptr;
            }
        }
        return std::numeric_limits<PID>::max();
    }


    template<typename Key, typename Data>
    void Tree<Key, Data>::insert(Key key, Data *record) {
        auto nextNode = root;
        while (nextNode != std::numeric_limits<PID>::max()) {
            Node<Key, Data> *node = mapping[nextNode];
            switch (node->type) {
                case PageType::inner:
                    break;
                case PageType::deltaInsert:
                case PageType::deltaDelete:
                case PageType::leaf: {
                    DeltaInsert<Key, Data> *node1 = CreateDeltaInsert<Key, Data>();
                    std::get<0>(node1->record) = key;
                    std::get<1>(node1->record) = record;
                    node1->origin = node;
                    if (!mapping[nextNode].compare_exchange_weak(node, node1)) {
                        free(node1);
                        nextNode = root;
                        continue;
                    }
                    return;
                }
/*                    case PageType::deltaDelete: {
                        auto node2 = static_cast<DeltaDelete<Key, Data> *>(nextNode);
                        nextNode = node2->origin;
                        continue;
                    }*/

                default: {
                }
                    switch (node->type) {
                        //TODO the ones that are less likely to occur
                    }
            }
        }
    }

    template<typename Key, typename Data>
    void Tree<Key, Data>::deleteKey(Key key) {
        PID nextNode = findDataPage(key);
        Node<Key, Data> *node = mapping[nextNode];
        switch (node->type) {
            case PageType::deltaDelete:
            case PageType::deltaInsert:
            case PageType::leaf: {
                DeltaDelete<Key, Data> *node1 = CreateDeltaDelete<Key, Data>();
                node1->key = key;
                node1->origin = node;
                if (!mapping[nextNode].compare_exchange_weak(node, node1)) {
                    free(node1);
                    nextNode = root;
                    deleteKey(key);//TODO without recursion
                    return;
                }
                return;
            }
            default: {
            }
                switch (node->type) {
                    //TODO the ones that are less likely to occur
                }
        }
    }

    template<typename Key, typename Data>
    void Tree<Key,Data>::consolidatePage(PID pid) {
        Node<Key,Data> *startNode = mapping[pid];

        Node<Key,Data> *node = startNode;
        std::vector<std::tuple<Key,Data*>> records;
        std::unordered_map<Key, bool> consideredKeys;
        Key stopAtKey;
        bool pageSplit = false;
        PID prev, next;
        while (node != nullptr) {
            switch (node->type) {
                case PageType::leaf: {
                    auto node1 = static_cast<Leaf<Key, Data> *>(node);
                    for (int i = 0; i < node1->recordCount; ++i) {
                        if (consideredKeys.find(std::get<0>(node1->records[i])) == consideredKeys.end()) {
                            records.push_back(node1->records[i]);
                            consideredKeys[std::get<0>(node1->records[i])] = true;
                        }
                    }
                    prev = node1->prev;
                    if (!pageSplit) {
                        next = node1->next;
                    }
                    // found last element in the chain
                    break;
                }
                case PageType::deltaInsert: {
                    auto node1 = static_cast<DeltaInsert<Key, Data> *>(node);
                    if (consideredKeys.find(std::get<0>(node1->record)) == consideredKeys.end()) {
                        records.push_back(node1->record);
                        consideredKeys[std::get<0>(node1->record)] = true;
                    }
                    node = node1->origin;
                    continue;
                }
                case PageType::deltaDelete: {
                    auto node2 = static_cast<DeltaDelete<Key, Data> *>(node);
                    if (consideredKeys.find(node2->key) == consideredKeys.end()) {
                        consideredKeys[node2->key] = true;
                    }
                    node = node2->origin;
                    continue;
                }
                case PageType::deltaSplit: {
                    auto node1 = static_cast<DeltaSplit<Key, Data> *>(node);
                    if (!pageSplit) {
                        pageSplit = true;
                        stopAtKey = node1->key;
                        next = node1->sidelink;
                    }
                    node = node1->origin;
                    continue;
                };
                default: {
                }
            }
            node = nullptr;
        }
        // construct a new node
        auto newNode = CreateLeaf<Key,Data>(records.size());
        records;//sort TODO
        records.data(); // TODO memcopy
        int i = 0;
        for (auto &r : records) {
            newNode->records[i++] = r;
        }
        newNode->next = next;
        newNode->prev=prev;

        if (!mapping[pid].compare_exchange_weak(startNode, newNode)) {
            consolidatePage(pid);
        }
    }
}

















