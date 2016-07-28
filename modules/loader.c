#include<stdlib.h>
#include<stdio.h>
#include<dlfcn.h>

int main (int argc, char** argv)
{
	char* error;
	void* handle;
	void (*hello)();
	double (*cosine)(double);

	char path[20] = "libhello.so.1";
	//char path[15] = "/lib/libm.so.6"
	handle = dlopen(path,RTLD_LAZY);

	if (!handle) {
		fputs(dlerror(), stderr);
		exit(EXIT_FAILURE);
	}

	//cosine = dlsym(handle,"cos");
	hello = dlsym(handle,"helloworld");

	if ((error = dlerror()) != NULL) {
		fputs(error, stderr);
		exit(EXIT_FAILURE);
	}

	//printf("%f\n", cosine(2.0));
	//printf("%f\n", cosine(45.0));
	hello();
	dlclose(handle);
}
