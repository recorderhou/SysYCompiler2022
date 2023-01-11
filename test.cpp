#include<iostream>
#include<cstdio>
#include<cstdlib>
using namespace std;

int f(int x) {
	const int y = x + 1;
	return y;
}

int main() {
  int a = f(1);
  printf("%d", a);
  return 10;
}






