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

	/* table.hbs file 열어서 binary형 숫자를 string으로 쭉 한줄로 저장 */
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
		get_binary = get_binary + tmp.to_string();	// get binary에 binary 값을 저장
	}
	fclose(table_FILE);


	/* get_binary(table.hbs)를 이용해 아스키 code 별로 code decode */
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
			table.insert(pair<int, string>(stoi(first, nullptr, 2), third));	//store table (아스키 코드별 code)
			first = "";
			second = "";
			third = "";
		}
		count++;
	}
	
	/* 각 아스키 코드당 변환 code를 이용해서 tree를 생성 */
	map<int, string>::iterator it;
	it = table.begin();
	
	for (int i = 0; i < table.size(); i++) {
		Huf_Tree* current_node = root;

		for (int j = 0; j < it->second.length(); j++) {		//for문이 다돌면 한개의 아스키코드값이 tree의 node로 생성된 것
			
			// 변환코드가 0이고 이동 가능할떄
			if (it->second[j] == '0' && current_node->LeftNode != NULL) {	
				current_node = current_node->LeftNode;
			}
			
			// 변환 코드가 0이고 이동 불가할 때
			else if (it->second[j] == '0' && current_node->LeftNode == NULL) {
				Huf_Tree* new_node= new Huf_Tree();		//이동할 노드를 만들어 이동
				new_node->ParentNode = current_node;	
				current_node->LeftNode = new_node;
				current_node = new_node;
			}
			
			// 변환 코드가 1이고 이동 가능할 때 
			else if (it->second[j] == '1' && current_node->RightNode != NULL) {
				current_node = current_node->RightNode;
			}
			
			//변환 코드가 1이고 이동 불가할때
			else if (it->second[j] == '1' && current_node->RightNode == NULL) {
				Huf_Tree* new_node = new Huf_Tree();	//이동할 노드를 만들어 이동
				new_node->ParentNode = current_node;
				current_node->RightNode = new_node;
				current_node = new_node;
			}
			
			else {
				printf("error");
				return 0;
			}

			//변환 코드 길이의 마지막엔 이동이 완료되어, 해당 노드에 자신의 아스키 코드, 변환 코드를 남김
			if (j == it->second.length() - 1) {
				current_node->ascy = it->first;
				current_node->code = it->second;
			}
		}
		it++;		//다음 아스키 값 트리에 넣으러 감
	}



	/* code.hbs file 열어서 binary형 숫자를 string으로 한줄로 저장 */
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


	/* table을 이용해 만든 tree를 이용해 decoding start */
	/* 원리 : root에서 한글자씩 상황에 맞는 위치로 내려가며 이동한 node의 자식이 없으면 찾은것*/
	Huf_Tree* c_node = root;
	for (int i = 0 ; i < huf_codes.length() ; i++) {


		if (huf_codes[i] == '0') {		// ready move to the left
			if (c_node->LeftNode == NULL) { // 내려갈 곳 없으므로 그 위치의 node가 최종 결과
				if (c_node->ascy == 128) {  // EOD 찾은 경우 끝!
					break;
				}
				fputc(c_node->ascy , result_FILE);	
				c_node = root->LeftNode;	// 저장했고, 안움직인 상태니까 root에서 움직인 상태로 바꿔줌
			}
			else {	//move
				c_node = c_node->LeftNode;
			}
		}

		else if (huf_codes[i] == '1') {		// ready move to the right
			if (c_node->RightNode == NULL) {	// 내려갈 곳 없으므로 그 위치의 node가 최종 결과
				if (c_node->ascy == 128) {		// EOD 찾은 경우 끝!
					break;
				}
				fputc(c_node->ascy, result_FILE);
				c_node = root->RightNode;		//// 저장했고, 안움직인 상태니까 root에서 움직인 상태로 바꿔줌
			}
			else {
				c_node = c_node->RightNode;
			}
		}
	}
	fclose(result_FILE);


	return 0;
}