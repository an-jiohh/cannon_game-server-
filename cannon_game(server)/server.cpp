#include<iostream>

using namespace std;

class map {
public:
	void print_map();
protected:
	int num[10][10] = { NULL };
private:
};

void map::print_map() {
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			if (num[i][j] == 0) cout << "бр";
		}
		cout << endl;
	}
}

int main() {
	map m1;
	m1.print_map();
	return 0;
}