#include "HCTree.hpp"



HCTree::~HCTree() {
    function<void(HCNode*)> postDelete = [&](HCNode* node) {
        if(node == nullptr) return;
        postDelete(node->c0);
        postDelete(node->c1);
        delete node;
        node = nullptr;
    };
    postDelete(root);
    for(int i = 0; i < static_cast<int>(leaves.size()); i++) {
        leaves[i] = nullptr;
    }
}

void HCTree::build(const vector<int>& freqs) {
    // If frequency has a tie, break alphabetically according to symbol
    priority_queue<HCNode*, vector<HCNode*>, HCNodePtrComp> pq;
    for(int i = 0; i < static_cast<int>(freqs.size()); i++) {
        if(freqs[i] > 0) {
            HCNode* leave = new HCNode(freqs[i], i);
            leaves[i] = leave;
            pq.push(leave);
        }
    }

    // if the file is empty
    if(pq.empty()) return;

    while(pq.size() > 1) {
        HCNode* smaller = pq.top();
        pq.pop();
        HCNode* bigger = pq.top();
        pq.pop();
        // assign the parent's value to be the smaller child's value
        HCNode* parent = new HCNode(smaller->count + bigger->count, smaller->symbol);
        parent->c0 = smaller;
        smaller->p = parent;
        parent->c1 = bigger;
        bigger->p = parent;
        pq.push(parent);
    }
    root = pq.top();
}

void HCTree::encode(unsigned char symbol, FancyOutputStream & out) const {
    HCNode* cur = leaves[symbol];
    vector<int> bits_seq;
    while(cur != nullptr && cur->p != nullptr) {
        HCNode* parent = cur->p;
        int bit = 0;
        if(cur == parent->c1) {
            bit = 1;
        }
        bits_seq.push_back(bit);
        cur = cur->p;
    }
    // write the bit sequence in the reverse order of the path
    for(int i = bits_seq.size() - 1; i >= 0; i--) {
        out.write_bit(bits_seq[i]);
    }
}

unsigned char HCTree::decode(FancyInputStream & in) const {
    HCNode* cur = root;
    if(cur == nullptr) {
        error("uninitialized root");
    }
    while(cur->c0 != nullptr && cur->c1 != nullptr) {
        if(!in.good()) {
            error("end of the file");
        }
        int bit = in.read_bit();
        if(bit == 1) {
            cur = cur->c1;
        } else {
            cur = cur->c0;
        }
    }
    return cur->symbol;
}

bool HCTree::isEmpty() const {
    return root == nullptr;
}

HCNode* HCTree::getRoot() const {
    return root;
}

void HCTree::setRoot(HCNode* node) {
    root = node;
}

void HCTree::setLeaves(const unsigned char& symbol, HCNode* node) {
    leaves[symbol] = node;
}