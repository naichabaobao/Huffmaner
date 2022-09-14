#pragma once

#include <cmath>

#include <map>
#include <queue>
#include <functional>
#include <iostream>
#include <string>
#include <fstream>

#define ShowInfo
//#undef ShowInfo

void showProgress(double progress) {
	std::cout.width(3);
	std::cout << static_cast<int>(progress * 100.0) << "%";
	std::cout << "\b\b\b\b";
}

std::string bitsetToString(std::vector<bool> set) {
	std::string result;
	for (auto it = set.begin(); it != set.end(); it++) {
		result.append((*it) ? "1" : "0");
	}
	return result;
}

bool compareBitset(std::vector<bool> bitset1, std::vector<bool> bitset2) {
	if (bitset1.size() != bitset2.size()) {
		return false;
	}
	for (size_t index = 0; index < bitset1.size(); index++) {
		if (bitset1[index] != bitset2[index]) {
			return false;
		}
	}
	return true;
}

class Huffmaner {
public:
	void decodeFile(std::string filepath, std::string outfilepath) {
		char* buf;
		char* resultData;

		std::ifstream in;
		in.open(filepath, std::ios::in | std::ios::binary);
		in.seekg(0, in.end);
		auto size = in.tellg();
		in.seekg(0, in.beg);

		buf = (char*)malloc(size * sizeof(char));
		in.read(buf, size);

		if (in.is_open()) {
			in.close();
		}

			
		unsigned int resultSize = 0;
		decodeData(buf, size, resultData, resultSize);

		std::ofstream out;
		out.open(outfilepath, std::ios::out | std::ios::binary);
		out.write(resultData, resultSize * sizeof(char));
		if (out.is_open()) {
			out.close();
		}
		free(resultData);
		free(buf);
	}
#define SizeByte 2
	int decodeData(char* source, int sourceSize, char*& resultData, unsigned int& resultDataSize) {
#ifdef ShowInfo
		printf("decode:");
#endif // ShowInfo
		if (sourceSize < SizeByte) {
			throw "not found table";
		}
		auto tableSize = static_cast<unsigned>(source[0]) << 8;
		tableSize += static_cast<unsigned>(source[1]);
		auto calcSize = tableSize * sizeof(char) + tableSize * sizeof(int);
		auto headerSize = SizeByte + calcSize;

		if (sourceSize < headerSize) {
			throw "not found keys";
		}
		if (!map.empty()) {
			map.clear();
		}
		unsigned int  maxChar = 0;
		for (size_t index = 0; index < tableSize; index++) {
			int offset = SizeByte + index * (sizeof(char) + sizeof(int));
			char ch = source[offset];
			unsigned int count = 0;
			count += static_cast<unsigned int>(static_cast<unsigned char>(source[offset + 1])) << 24;
			count += static_cast<unsigned int>(static_cast<unsigned char>(source[offset + 2])) << 16;
			count += static_cast<unsigned int>(static_cast<unsigned char>(source[offset + 3])) << 8;
			count += static_cast<unsigned int>(static_cast<unsigned char>(source[offset + 4]));
			map[ch] = count;
			maxChar += count;
		}

		buildTree();

		HuffmanNode* node = this->root;
		auto buffDataSize = sourceSize - headerSize;
		unsigned int currentCharCount = 0;
		unsigned int calcResultSize = maxChar;

		resultDataSize = calcResultSize;
		resultData = (char*)malloc(resultDataSize * sizeof(char));
		// each huff data
		for (size_t index = 0; index < buffDataSize; index++) {
			char ch = source[headerSize + index];
			for (size_t position = 0; position < 8 && currentCharCount < maxChar; position++) {
				bool high = ((ch & (0b10000000 >> position)) >> (8 - position - 1)) == 0b00000001;
				if (high) {
					node = node->right;
				}
				else {
					node = node->left;
				}
				if (node == nullptr) {
					break;
				}
				if (node->leaf()) {
					//datas.push_back(node->data);
					resultData[currentCharCount] = node->data;
					currentCharCount++;
					node = this->root;
#ifdef ShowInfo
					showProgress(static_cast<double>(currentCharCount) / static_cast<double>(maxChar));
#endif // ShowInfo

				}
			}
		}

		if (currentCharCount != maxChar) {
			throw "parse error!";
		}

		return resultDataSize;
	}
#undef SizeByte
	void encodeFile(std::string filepath, std::string outfilepath) {
		std::ifstream in;
		in.open(filepath, std::ios::in | std::ios::binary);
		in.seekg(0, in.end);
		auto size = in.tellg();
		in.seekg(0, in.beg);
		char* buf = (char*)malloc(size * sizeof(char));
		in.read(buf, size);

		if (in.is_open()) {
			in.close();
		}

		char* resultData;
		int resultSize = 0;
		encodeData(buf, size, resultData, resultSize);

		std::ofstream out;
		out.open(outfilepath, std::ios::out | std::ios::binary);
		out.write(resultData, resultSize * sizeof(char));
		if (out.is_open()) {
			out.close();
		}
		free(buf);
		free(resultData);
	}
	// contain header data
	int encodeData(char* source, int sourceSize, char*& resultData, int& resultDataSize) {
		char* compressData;
		int compressSize = 0;
		encode(source, sourceSize, compressData, compressSize);
		char* headerData;
		int headerSize = 0;
		createHeader(headerData, headerSize);

		resultData = (char*)malloc((compressSize + headerSize) * sizeof(char));
		resultDataSize = compressSize + headerSize;

		auto position = 0;
		for (size_t index = 0; index < headerSize; index++) {
			resultData[position] = headerData[index];
			position++;
		}
		for (size_t index = 0; index < compressSize; index++) {
			resultData[position] = compressData[index];
			position++;
		}

		free(compressData);
		free(headerData);

		return resultDataSize;
	}
	int encode(char* source, int sourceSize, char*& resultData, int& resultDataSize) {
		statistics(source, sourceSize);


		buildTree();

		buildMapping();

		encoding(source, sourceSize, resultData, resultDataSize);

		return resultDataSize;
	};
	Huffmaner() {
	}
	~Huffmaner() {
		if (root != nullptr) {
			delete root;
		}
	}
private:

	class HuffmanNode;
	class HuffmanNode {
	public:
		int weight = 0;
		HuffmanNode* left;
		HuffmanNode* right;
		char data = NULL;
		HuffmanNode() {
		}
		~HuffmanNode() {
			if (this->left!=nullptr) {
				delete this->left;
			}
			if (this->right != nullptr) {
				delete this->right;
			}
		}
		HuffmanNode(char ch, int weight) {
			this->data = ch;
			this->weight = weight;
		}
		HuffmanNode(char ch, int weight, HuffmanNode* left, HuffmanNode* right) {
			this->data = ch;
			this->weight = weight;
			if (left == nullptr || right == nullptr) {
				throw "children node cannot be empty!";
			}
			this->left = left;
			this->right = right;
		}
		bool leaf() {
			return this->left == nullptr && this->right == nullptr;
		}
		int incrementWeight() {
			this->weight++;
			return this->weight;
		}
		void print() {
			printf("Node {data: %c, weight: %d, left: %d, right: %d, leaf: %s}\n", data, weight, &left, &right, leaf() ? "true" : "false");
		}
		int size() {
			if (leaf()) {
				return 0;
			}
			else {
				return 2 + left->size() + right->size();
			}
		}
	};
	HuffmanNode* root;
	class HuffmanMapping {
	public:
		std::vector<bool> bitset;
		char key;

		std::string sting() {
			return bitsetToString(this->bitset);
		}
		bool empty() {
			return bitset.empty();
		}
	};

	std::map<char, unsigned> map;
	void statistics(char* source, int sourceSize) {
		if (map.size() > 0) {
			map.clear();
		}
		for (size_t position = 0; position < sourceSize; position++) {
			char ch = source[position];
			if (map.find(ch) != map.end()) {
				map[ch] = map[ch] + 1;
			}
			else {
				map[ch] = 1;
			}
		}
	}
	std::vector<HuffmanMapping> mappings() {
		std::vector<HuffmanMapping> mappingCollect;

		std::function<void(HuffmanNode*, std::vector<bool>)> func;
		func = [&func, &mappingCollect](HuffmanNode* node, std::vector<bool> set) {
			if (node->leaf()) {
				HuffmanMapping map;
				map.bitset = set;
				map.key = node->data;
				mappingCollect.push_back(map);
			}
			else {
				std::vector<bool> lset;
				lset.assign(set.begin(), set.end());
				lset.push_back(false);
				func(node->left, lset);

				std::vector<bool> rset;
				rset.assign(set.begin(), set.end());
				rset.push_back(true);
				func(node->right, rset);
			}
		};
		func(root, std::vector<bool>());

		return mappingCollect;
	}
	std::vector<HuffmanMapping> maps;
	void buildMapping() {
		if (!maps.empty()) {
			maps.clear();
		}
		maps = mappings();
	}
	HuffmanMapping getCode(char key) {
		for (auto it = maps.begin(); it != maps.end(); it++) {
			auto value = *it;
			if (value.key == key) {
				return value;
			}
		}
	}
	HuffmanMapping geChar(std::vector<bool> bitset) {
		for (auto it = maps.begin(); it != maps.end(); it++) {
			auto value = *it;
			if (compareBitset(value.bitset, bitset)) {
				return value;
			}
		}
	}
	void buildTree() {
		// max first
		auto compare = [](HuffmanNode* node1, HuffmanNode* node2) {
			
			return node1->weight > node2->weight;
		};
		// build leafs
		std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, decltype(compare)> que(compare, std::vector<HuffmanNode*>());
		for (auto it = map.begin(); it != map.end(); it++) {
			auto key = (*it).first;
			auto value = (*it).second;
			auto node = new HuffmanNode(key, value);
			que.push(node);
		}

		auto count = 0;
		while (que.size() > 1) {
			auto leftNode = que.top();
			que.pop();
			auto rightNode = que.top();
			que.pop();
			auto parent = new HuffmanNode(NULL, leftNode->weight + rightNode->weight, leftNode, rightNode);
			que.push(parent);
			count++;
		}

		auto node = que.top();
		que.pop();
		if (this->root != nullptr) {
			delete root;
		}
		this->root = node;
	}
	void encoding(char* source, int sourceSize, char*& resultData, int& resultDataSize) {
#ifdef ShowInfo
		printf("encode:");
#endif // ShowInfo
		// calc bit size
		auto bitSize = 0;
		for (size_t position = 0; position < sourceSize; position++) {
			char ch = source[position];

			auto mapping = getCode(ch);
			bitSize += mapping.bitset.size();
		}
		auto byteSize = (int)ceil(static_cast<double>(bitSize) / 8.0);
		resultData = (char*)malloc(byteSize * sizeof(char));
		resultDataSize = byteSize;

		std::deque<bool> bitset;
		auto handle = [&bitset]()-> char {
			char data = 0b00000000;
			for (size_t num = 0; num < 8 && !bitset.empty(); num++) {
				bool high = bitset.front();
				if (high) {
					data |= 0b10000000 >> num;
				}
				bitset.pop_front();
			}
			return data;
		};
		auto writePosition = 0;
		for (size_t position = 0; position < sourceSize; position++) {
			char ch = source[position];
			
			auto mapping = getCode(ch);
			for (size_t index = 0; index < mapping.bitset.size(); index++) {
				bitset.push_back(mapping.bitset[index]);
			}
			if (bitset.size() >= 8) {
				resultData[writePosition] = handle();
				writePosition++;
			}
#ifdef ShowInfo
			showProgress(static_cast<double>(position) / static_cast<double>(sourceSize));
#endif // ShowInfo
		}
		// flush
		if (!bitset.empty()) {
			resultData[writePosition] = handle();
		}
	}
	void createHeader(char* &resultData, int &resultSize) {
		resultSize = 2 * sizeof(unsigned char) + map.size() * sizeof(unsigned char) + map.size() * sizeof(int);
		resultData = (char*)malloc(resultSize);
		auto position = 0;
		
		resultData[position] = static_cast<unsigned char>((map.size() & 0xff00) >> 8);
		position++;
		resultData[position] = static_cast<unsigned char>((map.size() & 0xff));
		position++;
		for (auto it = map.begin(); it != map.end(); it++) {
			auto key = (*it).first;
			auto value = (*it).second;
			resultData[position] = key;
			position++;
			resultData[position] = static_cast<unsigned char>(value >> 24);
			position++;
			resultData[position] = static_cast<unsigned char>((value & 0x00ff0000) >> 16);
			position++;
			resultData[position] = static_cast<unsigned char>((value & 0x0000ff00) >> 8);
			position++;
			resultData[position] = static_cast<unsigned char>(value & 0x000000ff);
			position++;
		}
	}
	using NodeFunc = std::function<void(HuffmanNode*)>;
	void forEach(HuffmanNode* root, NodeFunc callback) {
		NodeFunc func;
		func = [&func, &callback](HuffmanNode* node) {
			if (node != nullptr) {
				callback(node);
			}
			if (!node->leaf()) {
				func(node->left);
				func(node->right);
			}
		};
		func(root);
	}
};


