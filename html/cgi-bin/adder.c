#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(int argc,char *argv[]){
	char *data;
	//FIXME
	//data = getenv("QUERY_STRING");
	
	/*
	int ret = -1;
	for(int i = 0;i < strlen(data);i++)
		if(data[i] == '&')
			ret = i;
	if(ret < 0)
		return 0;
	char p1[ret+1];
	for(int j = 0;j <ret;j++)
		p1[j] = data[j];
	p1[ret] = '\0';
	char *p2 = strchr(data,'&') + 1;
	*/
	int a,b;
	char *p1 = argv[0];
	char *p2 = argv[1];
	a = atoi(p1);
	b = atoi(p2);
	printf("a=%d,b=%d\n",a,b);
	printf("The answer is : %d + %d = %d\n",a,b,a+b);
	return 0;
}

