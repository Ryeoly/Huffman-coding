#include <fstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <bitset>
using namespace std;

#pragma warning(disable:4996)

//Huffman tree struct
typedef struct Huf_Tree {			
	Huf_Tree* LeftNode;
	Huf_Tree* RightNode;
	Huf_Tree* ParentNode;
	vector<int> ascy;				// ASCII codes that a node has as a child
	double probability;				// weight of node
}Huf_Tree;

//Huffman tree constructor
Huf_Tree* make_Node(vector<int> as, double prob) {
	Huf_Tree* node;
	node = new Huf_Tree();

	node->LeftNode = NULL;
	node->RightNode = NULL;
	node->ParentNode = NULL;
	node->ascy = as;					
	node->probability = prob;

	return node;
}

/* Used to combine two nodes with the smallest weight (when make tree) */
Huf_Tree* combine_Node(Huf_Tree* first_Tree, Huf_Tree* second_Tree) {
	Huf_Tree* node;
	node = new Huf_Tree();

	//among the two nodes, the node with the smaller weight is the left child.
	if (first_Tree->probability > second_Tree->probability) {
		node->LeftNode = second_Tree;
		node->RightNode = first_Tree;
	}
	else {
		node->LeftNode = first_Tree;
		node->RightNode = second_Tree;
	}
	node->ParentNode = NULL;

	// Combining vectors of two nodes
	vector<int> tmp = first_Tree->ascy;
	tmp.insert(tmp.end(), second_Tree->ascy.begin(), second_Tree->ascy.end());
	node->ascy = tmp;
	node->probability = first_Tree->probability + second_Tree->probability;

	// In order for the parent node to have the ASCII code of its children
	first_Tree->ParentNode = node;
	second_Tree->ParentNode = node;

	return node;
}

/* The conversion code suitable for the ASCII code is returned. (like huffman_table) */
string make_codeword(Huf_Tree* tree, int asci) {
	string binary_code = "";
	Huf_Tree* node = tree;						//unconditionally root

	// ASCII code not in input.txt
	if (*find(node->ascy.begin(), node->ascy.end()-1, asci) != asci) { 
		return binary_code;
	}

	// Traversing the tree and looking for transformation codes
	while (1) {
		if (node->LeftNode == NULL && node->RightNode == NULL) { // leaf node. (already find code)
			break;
		}

		else{
			// If the left node has the corresponding ASCII code
			node = node->LeftNode;
			if (*find(node->ascy.begin(), node->ascy.end()-1, asci) == asci) {
				binary_code = binary_code + "0";
				continue;
			}

			// If the right node has the corresponding ASCII code
			node = (node->ParentNode)->RightNode;
			binary_code = binary_code + "1";
		}
	}
	return binary_code;
}



int main(void) 
{
	double asci[128] = { 0, };
	double count = 0;
	multimap<double, vector<int>> value;			// probability, Ascii code

	/* input.txt file open , error check */
	fstream fp("input_data.txt");
	if (!fp) {
		printf("file open fail \n");
		return 0;
	}

	/* input_data.txt words count and check */
	char tmp;
	while (fp.get(tmp)) {
		count++;
		int ascy = tmp;
		if (ascy >= 0 && ascy <= 127) {
			asci[ascy]++;							// count Ascii code
		}
	}
	fp.close();

	/* convert frequency to probability */
	for (int i = 0; i < 128; i++) {
		if (asci[i] != 0.0) {
			double percent = asci[i] / count;		// counting number / total number = probability
			value.insert(pair<double, vector<int>>(percent, { i }));
		}
	}
	value.insert(pair<double, vector<int>>(0, { 128 })); // Insert 128 ASCII code for EOD



	map<double, vector<int>>::iterator it;
	double a_prob=0;
	vector<int> a_vector;
	double b_prob=0;
	vector<int> b_vector;
	int value_size = value.size();

	map<vector<int>, Huf_Tree*> vector_table;			// to make ASCII codes into one tree

	for (int i = 1 ; i < value_size ; i++) {
		Huf_Tree* Node_a= NULL;
		Huf_Tree* Node_b= NULL;

		it = value.begin();								// The one with the least probability = a
		a_prob = it->first;
		a_vector = it->second;
		if (a_vector.size() == 0) {
			return 0;
		}
		else if (a_vector.size() == 1) {				// only one ASCII code, there is no node
			Node_a = make_Node(a_vector, a_prob);
		}
		else {
			if (vector_table.size() != 0) {
				vector<int> idx= a_vector;
				Node_a = vector_table[idx];				// Take a tree out of a small tree table
			}
		}
		value.erase(it++);								// 첫번째 삭제하면서 두번째로 확률이 작은 애 선택



		b_prob = it->first;								// The second least probable one = b
		b_vector = it->second;
		if (b_vector.size() == 0) {
			return 0;
		}
		else if (b_vector.size() == 1) {
			Node_b = make_Node(b_vector, b_prob);
		}
		else{
			if (vector_table.size() != 0) {
				vector<int> idx2 = b_vector;
				Node_b = vector_table[idx2];	
			}
		}
		value.erase(it);


		vector_table.erase(a_vector);
		vector_table.erase(b_vector);
		a_vector.insert(a_vector.end(), b_vector.begin(), b_vector.end());
		float tmp1 = a_prob + b_prob;
		// Combines two nodes into a small tree
		vector_table.insert(pair<vector<int>, Huf_Tree*>(a_vector, combine_Node(Node_a, Node_b)));
		//store small tree
		value.insert( pair<double, vector<int>> (tmp1, a_vector) );	
	}
	//여기까지 오면 vector table에는 마지막에 완성된 tree 하나만 남아 있고,
	//value에는 모든 값들이 합쳐져 확률이 1인 것만 남는다. 


	/* 허프만 테이블 코드 string으로 쭉 이어 붙이는 부분 */
	map<vector<int>, Huf_Tree*>::iterator iter;
	iter = vector_table.begin();
	string table_code = "";											
	for (int i = 0; i <= 128; i++) {								// 0 to 128 because of EOD (128은 EOD)
		string code = "";
		code = make_codeword(iter->second, i);						// code is convert code

		if (code.length() != 0) {
			bitset<8> tmp(i);										// tmp is Ascii code (ver binary)
			table_code = table_code + tmp.to_string();
			
			bitset<8> code_word_len(code.length());					// code_word_len is length of convert code
			table_code = table_code + code_word_len.to_string();

			table_code = table_code + code;
		}
	}

	/* stuffing bit 추가해주는 부분*/
	int add_bit =  8 - (table_code.length() % 8);					// 얼마나 stuffing을 추가해야하는지 
	for (int i = 0; i < add_bit; i++) {
		table_code = table_code + "0";
	}

	/* Write Huffman_table.hbs */
	FILE* table_FILE = fopen("Huffman_table.hbs", "wb");

	int byte_w = 0;													// converted to byte and saved
	for (int i = 0; i < table_code.length() ; i++) {
		
		if (table_code[i] == '1') {
			byte_w = byte_w + (int)pow(2, 7 - (i % 8));				// 8bit씩 값을 계산해준다.
		}

		if ((i % 8) == 7) {											// 8bit가 지나면 저장 후 변수 초기화 
			fputc(byte_w, table_FILE);
			byte_w = 0;
		}
		
	}
	fclose(table_FILE);


	/* input 관련해서 처리하는 부분, input을 code로 변환해 주는 부분 */
	string code_code = "";										
	fstream fp2("input_data.txt");
	if (!fp2) {
		printf("file2 open fail \n");
		return 0;
	}

	char tmp2;
	while (fp2.get(tmp2)) {
		int ascy2 = tmp2;
		if (ascy2 >= 0 && ascy2 <= 127) {
			code_code = code_code + make_codeword(iter->second, ascy2);	// 코드들을 입력받는 즉시 변환 후 저장
		}
	}
	fp2.close();
	code_code = code_code + make_codeword(iter->second, 128);			// add EOD code
	
	add_bit = 8 - (code_code.length() % 8);								// add stuffing bit
	for (int i = 0; i < add_bit; i++) {
		code_code = code_code + "0";
	}

	/* write huffman_code.hbs  */
	FILE* code_FILE = fopen("huffman_code.hbs", "wb");

	byte_w = 0;															// table.hbs를 저장한 것과 같은 알고리즘
	for (int i = 0; i < code_code.length(); i++) {

		if (code_code[i] == '1') {
			byte_w = byte_w + (int)pow(2, 7 - (i % 8));
		}

		if ((i % 8) == 7) {
			fputc(byte_w, code_FILE);
			byte_w = 0;
		}

	}
	fclose(code_FILE);

	return 0;
}