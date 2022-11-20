#include <stdio.h>
#include <stdlib.h>
int main()
{
	
	printf("Child has: \n%s\n", getenv("PATH"));
	printf("%s\n", getenv("HOME"));
	printf("%s\n", getenv("gabo"));
}
