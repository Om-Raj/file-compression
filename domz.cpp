#include <fstream>
#include <iostream>
#include <string.h>

#include <bitset>
#include <map>
#include <vector>

#define MAX_CHARS 256

bool VERBOSE = false;

void calc_len_table(std::ifstream &in_file, std::vector<std::vector<unsigned char>> &len_table) {
    char c;
    if (!in_file.read(&c, sizeof(c)))
        throw "Empty input file";
    unsigned char N = c;
    len_table.resize(N);
    for (int i = 0; i < N; i++) {
        if (!in_file.read(&c, sizeof(c)))
            throw "Unable to initialize length table";
        len_table[i].resize(static_cast<unsigned char>(c));
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < len_table[i].size(); j++) {
            if (!in_file.read(&c, sizeof(c)))
                throw "Unable to contruct length table";
            len_table[i][j] = c;
        }
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

void calc_keys(std::map<std::string, unsigned char> &keys, std::vector<std::vector<unsigned char>> &len_table) {
    std::bitset<MAX_CHARS> code; // hard-coded values
    int code_len = 0;
    for (int i = 0; i < len_table.size(); i++) {
        while (code_len < i + 1) {
            code <<= 1;
            code_len++;
        }
        for (unsigned char c : len_table[i]) {
            std::string temp;
            for (int j = i; j >= 0; j--) {
                temp += code.test(j)? '1': '0';
            }
            keys[temp] = c;
            increment(code);
        }
    }
}

bool decode_char(std::ofstream &out_file, std::string &buffer, int lookahead, std::map<std::string, unsigned char> &keys) {
    std::string code;
    for (int i = 0; i < lookahead; i++) {
        code += buffer[i];
        auto it = keys.find(code);
        if (it != keys.end()) {
            char c = it->second;
            out_file.write(&c, sizeof(c));
            buffer = buffer.substr(i+1);
            return true;
        }
    }
    return false;
}

void decode(std::ifstream &in_file, std::string &out_path, std::vector<std::vector<unsigned char>> &len_table) {
    std::map<std::string, unsigned char> keys;
    calc_keys(keys, len_table);

    if (VERBOSE)
        for (auto [code, ch] : keys)
            std::cout << (ch == ' '? "SPACE" : (ch == '\n'? "NEWLINE" : std::string(1, ch))) << " : " << code << '\n';

    std::ofstream out_file(out_path, std::ios::binary);
    if (!out_file.is_open()) {
        std::cerr << "OUT: " << out_path << std:: endl;
        throw "Unable to create output file";
    }

    std::string buffer;
    char c;
    while (in_file.read(&c, sizeof(c))) {
        std::bitset<8> bits(c);
        buffer += bits.to_string();
        while (buffer.size() >= 16) {   // padded 0's and padding count are present in the last 16 bits
            if (!decode_char(out_file, buffer, len_table.size(), keys))
                throw "Unable to decompress. Invalid data.";
        }
    }

    int rem = buffer.size();
    int padding = stoi(buffer.substr(rem - 8), nullptr, 2);
    buffer = buffer.substr(0, rem - padding - 8);
    while (buffer.size()) {
        if (!decode_char(out_file, buffer, len_table.size(), keys))
            throw "Unable to decompress. Invalid data.";
    }

    out_file.close();
}

void decompress(std::string &in_path, std::string &out_path) {
    std::ifstream in_file(in_path, std::ios::binary);
    if (!in_file.is_open())
        throw "Unable to read input file";

    std::vector<std::vector<unsigned char>> len_table;
    calc_len_table(in_file, len_table);

    if (VERBOSE) {
        for (int i = 0; i < len_table.size(); i++) {
            std::cout << "LEN (" << i+1 << ") : ";
            for (char c : len_table[i]) {
                std::cout << (c == ' '? "SPACE" : (c == '\n'? "NEWLINE" : std::string(1, c))) << " ";
            }
            std::cout << '\n';
        }
    }

    decode(in_file, out_path, len_table);
    in_file.close();
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
            int N = in_path.size();
            if (N < 4 || in_path.substr(N - 4) != ".omz") // Input file should have .omz extension
                throw "Invalid input file";
            std::string out_path = in_path.substr(0, N - 3).append("dec"); // Decompressed file has .dec extension
            decompress(in_path, out_path);
        }
    } catch (char const* exception) {
        std::cerr << "Exception: " << exception << std::endl;
    } 
    return 0;
}