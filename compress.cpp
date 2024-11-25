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
void buildTree(HCTree& tree) {
    vector<int> freqs(256, 0);
    freqs[68] = 67; // D
    freqs[71] = 20; // G
    freqs[73] = 57; // I
    freqs[76] = 41; // L
    freqs[78] = 89; // N
    freqs[84] = 60; // T
    freqs[85] = 66; // U
    freqs[86] = 21; // V
    freqs[87] = 13; // W
    tree.build(freqs);
}
void testEnDeCode(HCTree& tree) {
    FancyOutputStream os("en.txt");
    for(int i = 0; i < 3; i++) {
        tree.encode('L', os);
    }
    os.flush();
    FancyInputStream is("en.txt");
    for(int i = 0; i < 4; i++) {
        cout<<tree.decode(is)<<endl;
    }
}

void record_freq(vector<int>& freqs, FancyInputStream& is) {
    int original_size = is.filesize();
    for(int i = 0; i < original_size; i++) {
        int byte = is.read_byte();
        freqs[byte]++;
    }
}

/**
 * A fix size array of 256 integers
 * record the frequency of every byte
 */
void write_header(HCTree& tree, vector<int>& freqs, FancyOutputStream& os) {
    // don't write header if nothing to encode
    if(tree.isEmpty()) return;

    for(int i = 0; i < static_cast<int>(freqs.size()); i++) {
        os.write_int(freqs[i]);
    }
}

void write_byte(FancyOutputStream& os, unsigned char const & byte) {
    int bit = 0;
    for(int i = 7; i >= 0; i--) {
        bit = (byte >> i) & 1;
        os.write_bit(bit);
    }
}

/*
 * serialize the huffman tree
 */
void write_header2(HCTree& tree, FancyOutputStream& os, int original_size) {
    HCNode* root = tree.getRoot();
    if(root == nullptr) return;

    // a preorder traversal to serialize the huffman tree
    function<void(HCNode*)> serializePreOrder = [&](HCNode* node) {
        // if the node is a leave, write a bit 1 followed by its symbol
        if(node->c0 == nullptr && node->c1 == nullptr) {
            os.write_bit(1);
            write_byte(os, node->symbol);
            return;
        }
        // if it's an internal node, only write a bit 0
        os.write_bit(0);
        serializePreOrder(node->c0);
        serializePreOrder(node->c1);
    };
    // encode the size of the file at the beginning
    os.write_int(original_size);
    serializePreOrder(root);
}
void write_content(HCTree& tree, FancyInputStream& is, FancyOutputStream& os, int original_size) {
    for(int i = 0; i < original_size; i++) {
        int byte = is.read_byte();
        tree.encode(byte, os);
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3) {
        error("correct format to use the program: ./compress <original_file> <compressed_file>");
    }
    const char* original_file = argv[1];
    const char* compressed_file = argv[2];

    FancyInputStream is(original_file);
    if(!is.good()) {
        error("invalid original_file name");
    }
    FancyOutputStream os(compressed_file);
    if(!os.good()) {
        error("invalid compressed_file name");
    }

    HCTree tree;
    vector<int> freqs(256, 0);
    record_freq(freqs, is);
    tree.build(freqs);

    int original_size = is.filesize();
    write_header2(tree, os, original_size);

    is.reset(); // read from the beginning
    write_content(tree, is, os, original_size);
}