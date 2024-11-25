#include "HCTree.hpp"

void level_traversal(HCNode* root) {
    if(root == nullptr) {
        return;
    }

    queue<HCNode*> q;
    q.push(root);
    int levelCnt = q.size();
    int level = 1;
    cout<<"level "<<levelCnt<<" : ";
    while(!q.empty()) {
        HCNode* cur = q.front();
        q.pop();
        cout<<cur->symbol<<" ";
        if(cur->c0) q.push(cur->c0);
        if(cur->c1) q.push(cur->c1);
        levelCnt--;
        if(!q.empty() && levelCnt == 0) {
            levelCnt = q.size();
            level++;
            cout<<endl<<"level "<<level<<" : ";
        }
    }
    cout<<endl;
}

// return the number of bytes in the original file
int read_header(vector<int>& freqs, FancyInputStream& is) {
    // if empty file, no need to build huffmantree
    if(is.filesize() == 0) return 0;
    
    int original_size = 0;
    for(int i = 0; i < static_cast<int>(freqs.size()); i++) {
        int freq = is.read_int();
        original_size += freq;
        freqs[i] = freq;
    }
    return original_size;
}

unsigned char read_byte(FancyInputStream& is) {
    unsigned char byte = 0;
    for(int i = 0; i < 8; i++) {
        int bit = is.read_bit();
        byte = (byte << 1) | bit;
    }
    return byte;
}

void connect_parent(HCNode* parent, HCNode* child) {
    if(child == nullptr) return;
    if(parent != nullptr && child != nullptr) {
        child->p = parent;
    }
    connect_parent(child, child->c0);
    connect_parent(child, child->c1);
}

int read_header2(HCTree& tree, FancyInputStream& is) {
    // if file is empty
    if(is.filesize() == 0) return 0;

    int original_size = is.read_int();
    /**
     * Deserialize the huffman tree
     */
    function<HCNode*()> deserializePreorder = [&]() {
        int bit = is.read_bit();
        if(bit == 1) {
            unsigned char byte = read_byte(is);
            HCNode* newNode = new HCNode(0, byte);
            tree.setLeaves(byte, newNode);
            return newNode;
        }
        HCNode* newNode = new HCNode(0, '#');
        newNode->c0 = deserializePreorder();
        newNode->c1 = deserializePreorder();
        return newNode;
    };

    tree.setRoot(deserializePreorder());
    // connect parent pointers
    // don't actually need it for decoding
    connect_parent(nullptr, tree.getRoot());
    return original_size;
}

void write_decode(HCTree& tree, int original_size,
    FancyInputStream& is, FancyOutputStream& os) {
    for(int i = 0; i < original_size; i++) {
        unsigned char byte = tree.decode(is);
        os.write_byte(byte);
    }

}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        error("correct format to use the program: ./compress <compressed_file> <uncompressed_file>");
    }
    const char* compressed_file = argv[1];
    const char* uncompressed_file = argv[2];

    FancyInputStream is(compressed_file);
    if(!is.good()) {
        error("invalid compressed_file name");
    }
    FancyOutputStream os(uncompressed_file);
    if(!os.good()) {
        error("invalid uncompressed_file name");
    }

    vector<int> freqs(256, 0);
    HCTree tree;
    int original_size = read_header2(tree, is);
    
    write_decode(tree, original_size, is, os);
}