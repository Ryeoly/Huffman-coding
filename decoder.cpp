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
	int ascy;				// Ascii code
	string code;			// covert code
}Huf_Tree;

//Huffman tree constructor
Huf_Tree* make_Node(int ascy, string code) {
	Huf_Tree* node;
	node = new Huf_Tree();

	node->LeftNode = NULL;
	node->RightNode = NULL;
	node->ParentNode = NULL;
	node->ascy = ascy;
	node->code = code;

	return node;
}


int main(void) {
	int count = 0;
	string get_binary = "";
	Huf_Tree* root = new Huf_Tree();

	/* table.hbs file ��� binary�� ���ڸ� string���� �� ���ٷ� ���� */
	FILE* table_FILE = fopen("Huffman_table.hbs", "rb");
	while (table_FILE == NULL) {
		printf("failed open table file");
		return 0;
	}
	while (1) {
		int c = fgetc(table_FILE);
		if (c == EOF) {
			break;
		}

		bitset<8> tmp(c);
		get_binary = get_binary + tmp.to_string();	// get binary�� binary ���� ����
	}
	fclose(table_FILE);


	/* get_binary(table.hbs)�� �̿��� �ƽ�Ű code ���� code decode */
	map<int, string> table;
	string first = "";								// 8 bit , Ascii code
	string second = "";								// 8 bit , convert code length
	string third = "";								// convert code
	while (1) {
		if (get_binary.length() < 8) {
			break;
		}

		if ((count % 3) == 0) {	
			first = get_binary.substr(0, 8);
			get_binary.erase(0, 8);
		}
		else if ((count % 3) == 1) {
			second = get_binary.substr(0, 8);
			get_binary.erase(0, 8);
		}
		else {
			third = get_binary.substr(0, stoi(second, nullptr, 2));	//use stoi, get second decimal value
			get_binary.erase(0, stoi(second, nullptr, 2));
			table.insert(pair<int, string>(stoi(first, nullptr, 2), third));	//store table (�ƽ�Ű �ڵ庰 code)
			first = "";
			second = "";
			third = "";
		}
		count++;
	}
	
	/* �� �ƽ�Ű �ڵ�� ��ȯ code�� �̿��ؼ� tree�� ���� */
	map<int, string>::iterator it;
	it = table.begin();
	
	for (int i = 0; i < table.size(); i++) {
		Huf_Tree* current_node = root;

		for (int j = 0; j < it->second.length(); j++) {		//for���� �ٵ��� �Ѱ��� �ƽ�Ű�ڵ尪�� tree�� node�� ������ ��
			
			// ��ȯ�ڵ尡 0�̰� �̵� �����ҋ�
			if (it->second[j] == '0' && current_node->LeftNode != NULL) {	
				current_node = current_node->LeftNode;
			}
			
			// ��ȯ �ڵ尡 0�̰� �̵� �Ұ��� ��
			else if (it->second[j] == '0' && current_node->LeftNode == NULL) {
				Huf_Tree* new_node= new Huf_Tree();		//�̵��� ��带 ����� �̵�
				new_node->ParentNode = current_node;	
				current_node->LeftNode = new_node;
				current_node = new_node;
			}
			
			// ��ȯ �ڵ尡 1�̰� �̵� ������ �� 
			else if (it->second[j] == '1' && current_node->RightNode != NULL) {
				current_node = current_node->RightNode;
			}
			
			//��ȯ �ڵ尡 1�̰� �̵� �Ұ��Ҷ�
			else if (it->second[j] == '1' && current_node->RightNode == NULL) {
				Huf_Tree* new_node = new Huf_Tree();	//�̵��� ��带 ����� �̵�
				new_node->ParentNode = current_node;
				current_node->RightNode = new_node;
				current_node = new_node;
			}
			
			else {
				printf("error");
				return 0;
			}

			//��ȯ �ڵ� ������ �������� �̵��� �Ϸ�Ǿ�, �ش� ��忡 �ڽ��� �ƽ�Ű �ڵ�, ��ȯ �ڵ带 ����
			if (j == it->second.length() - 1) {
				current_node->ascy = it->first;
				current_node->code = it->second;
			}
		}
		it++;		//���� �ƽ�Ű �� Ʈ���� ������ ��
	}



	/* code.hbs file ��� binary�� ���ڸ� string���� ���ٷ� ���� */
	string huf_codes = "";
	FILE* code_FILE = fopen("huffman_code.hbs", "rb");
	FILE* result_FILE = fopen("output.txt", "w+");

	while (code_FILE == NULL) {
		printf("failed open code file");
		return 0;
	}

	int d = fgetc(code_FILE);
	while (d != EOF) {
		bitset<8> tmp2(d);			//huf_code is binary number of huffman_code.hbs 
		huf_codes = huf_codes + tmp2.to_string();
		d = fgetc(code_FILE);
	}
	fclose(code_FILE);


	/* table�� �̿��� ���� tree�� �̿��� decoding start */
	/* ���� : root���� �ѱ��ھ� ��Ȳ�� �´� ��ġ�� �������� �̵��� node�� �ڽ��� ������ ã����*/
	Huf_Tree* c_node = root;
	for (int i = 0 ; i < huf_codes.length() ; i++) {


		if (huf_codes[i] == '0') {		// ready move to the left
			if (c_node->LeftNode == NULL) { // ������ �� �����Ƿ� �� ��ġ�� node�� ���� ���
				if (c_node->ascy == 128) {  // EOD ã�� ��� ��!
					break;
				}
				fputc(c_node->ascy , result_FILE);	
				c_node = root->LeftNode;	// �����߰�, �ȿ����� ���´ϱ� root���� ������ ���·� �ٲ���
			}
			else {	//move
				c_node = c_node->LeftNode;
			}
		}

		else if (huf_codes[i] == '1') {		// ready move to the right
			if (c_node->RightNode == NULL) {	// ������ �� �����Ƿ� �� ��ġ�� node�� ���� ���
				if (c_node->ascy == 128) {		// EOD ã�� ��� ��!
					break;
				}
				fputc(c_node->ascy, result_FILE);
				c_node = root->RightNode;		//// �����߰�, �ȿ����� ���´ϱ� root���� ������ ���·� �ٲ���
			}
			else {
				c_node = c_node->RightNode;
			}
		}
	}
	fclose(result_FILE);


	return 0;
}