#include <stdio.h>
#include <string.h>

//params: output file, ... list of input files
int main(int argc, char** args){
	if(argc == 1){
		printf("No filename given\n");
		return 0;
	}
	FILE* fout = fopen(args[1], "w");
	
	for(int i = 2; i < argc; ++i){
		FILE* fin = fopen(args[i], "r");
		if(!fin){
			printf("Error opening file %s\n", args[i]);
		}

		float time = 0;
		int count = 0;
		double addedTime = 0;

		while(fscanf(fin, "%d %f", &count, &time) == 2){
			addedTime += time;
		}
		fclose(fin);
		fprintf(fout, "%f\n", addedTime / count);
	}

	fclose(fout);
	return 0;
}