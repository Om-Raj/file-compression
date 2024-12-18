#include <fstream>
#include <iostream>
#include <string.h>

#include <bitset>
#include <map>
#include <queue>
#include <vector>

#define MAX_CHARS 256

bool VERBOSE = false;
std::string encoded;

typedef struct Node {
    char data;
    int freq;
    Node *left, *right;

    Node(char d, int f, Node *l, Node *r) : data(d), freq(f), left(l), right(r) {};
} Node;

typedef struct CompareNode {
    bool operator()(Node *a, Node *b) {
        return a->freq > b->freq;
    } 
} CompareNode;

void get_char_freq(std::string in_path, std::map<char, int> &freq) {
    std::ifstream in_file(in_path);
    if (!in_file.is_open())
        throw "Unable to read input file";
    char c;
    while (in_file.get(c)) {
        freq[c]++;
    }
    in_file.close();
}

Node* create_huff_tree(std::map<char, int> &freq) {
    std::priority_queue<Node*, std::vector<Node*>, CompareNode> pq;
    for (auto [ch, fq] : freq) {
        Node *node = new Node(ch, fq, nullptr, nullptr);
        pq.push(node);
    }
    while (pq.size() > 1) {
        Node *a = pq.top(); pq.pop();
        Node *b = pq.top(); pq.pop();
        Node *c = new Node('\0', a->freq + b->freq, a, b);
        pq.push(c);
    }
    return pq.top();
}

int get_tree_height(Node *root) {
    if (!root) return -1;
    return 1 + std::max(get_tree_height(root->left), get_tree_height(root->right));
}

void deallocate(Node *root) {
    if (root) {
        deallocate(root->left);
        deallocate(root->right);
        free(root);
    }
}

void calc_len_table(Node *root, std::vector<std::vector<char>> &len_table, int code_len) {
    if (root->data != '\0') {
        len_table[code_len-1].push_back(root->data);
    } else {
        calc_len_table(root->left, len_table, code_len + 1);
        calc_len_table(root->right, len_table, code_len + 1);
    }
}

void increment(std::bitset<MAX_CHARS> &code) {
    bool carry = true;
    for (size_t i = 0; i < MAX_CHARS; ++i) {
        if (carry) {
            carry = code[i];  // If the bit is 1, carry remains true
            code[i] = !code[i];  // Flip the bit
        } else {
            break;  // No carry to propagate
        }
    }
}

void calc_keys(std::map<char, std::string> &keys, std::vector<std::vector<char>> &len_table) {
    std::bitset<MAX_CHARS> code; // hard-coded values
    int code_len = 0;
    for (int i = 0; i < len_table.size(); i++) {
        while (code_len < i + 1) {
            code <<= 1;
            code_len++;
        }
        for (char c : len_table[i]) {
            std::string temp;
            for (int j = i; j >= 0; j--) {
                temp += code.test(j)? '1': '0';
            }
            keys[c] = temp;
            increment(code);
        }
    }
}

int bin_conv(std::string binstr) {
    return stoi(binstr, nullptr, 2);
}

void encode(std::string &in_path, std::vector<std::vector<char>> &len_table) {
    std::map<char, std::string> keys;
    calc_keys(keys, len_table);

    if (VERBOSE)
        for (auto [ch, code] : keys)
            std::cout << (ch == ' '? "SPACE" : (ch == '\n'? "NEWLINE" : std::string(1, ch))) << " : " << code << '\n';

    encoded += (char) len_table.size();

    for (int i = 0; i < len_table.size(); i++)
        encoded += (char) len_table[i].size();

    for (int i = 0; i < len_table.size(); i++)
        for (const char &c : len_table[i])
            encoded += c;

    std::ifstream in_file(in_path);
    if (!in_file.is_open())
        throw "Unable to read input file";

    std::string buffer;
    char c;
    while (in_file.get(c)) {
        buffer += keys[c];
        while (buffer.size() >= 8) {
            encoded += (char) bin_conv(buffer.substr(0, 8));
            buffer = buffer.substr(8);
        }
    }
    in_file.close();

    int padding = 8 - buffer.size();
    if (padding < 8) {
        buffer.append(padding, '0');
        encoded += (char) bin_conv(buffer);
    }
    encoded += (char) padding;
}

void compress(std::string &in_path, std::string &out_path) {
    std::map<char, int> freq;
    get_char_freq(in_path, freq); 
    if (freq.empty())
        throw "Empty input file";

    Node *huff_tree = create_huff_tree(freq);
    int huff_tree_height = get_tree_height(huff_tree);

    std::vector<std::vector<char>> len_table(huff_tree_height);
    if (huff_tree_height) {
        calc_len_table(huff_tree, len_table, 0);
    } else {
        len_table = {{freq.begin()->first}}; // file contains only 1 type of character
    }

    if (VERBOSE) {
        for (int i = 0; i < len_table.size(); i++) {
            std::cout << "LEN (" << i+1 << ") : ";
            for (char c : len_table[i]) {
                std::cout << (c == ' '? "SPACE" : (c == '\n'? "NEWLINE" : std::string(1, c))) << " ";
            }
            std::cout << '\n';
        }
    }

    deallocate(huff_tree);

    encode(in_path, len_table);

    std::ofstream out_file(out_path);
    if (!out_file.is_open())
        throw "Unable to create output file";
    
    out_file << encoded;
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2)
            throw "No input file provided";
        else if (argc > 3) 
            throw "Too many arguments";
        else {
            std::string in_path;
            for (int i = 1; i < argc ; i++) {
                if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-V") == 0) {
                    VERBOSE = true;
                } else {
                    in_path = argv[i];
                }
            }
            std::string out_path = in_path + ".omz";
            compress(in_path, out_path);
        }
    } catch (char const* exception) {
        std::cerr << "Exception: " << exception << std::endl;
    } 
    return 0;
}