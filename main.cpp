#include "Classes.h"
#include "Note.h"
#include "Song.h"
#include "Population.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

double transitionProbabilityMatrix_Sum[12][12];
char midiDirectory[512];

void readFromFile() {
	FILE* file;
	int i, j;
	double matrix[12][12];
	
	if ((file = fopen("tpm.mat", "r")) == NULL) {
        	cout<<"\nError opening file for read...\n";
        	exit (0);
    	}
	else {
		for (i=0; i<12; ++i) {
    			fread(matrix[i], sizeof(matrix[i][0]), 12, file);
			for (j=0; j<12; j++) {
				transitionProbabilityMatrix_Sum[i][j] += matrix[i][j];
			}
		}
		fclose (file);
	}
}

void convertMidi2Notes(char fileName[]) {

	char argv1[512], argv0[512];
	strcpy(argv0, "./midi2notes");
	strcpy(argv1, midiDirectory);
	strcat(argv1, fileName);
	

	pid_t parent = getpid();
	pid_t pid = fork();
	char *argv[] = {argv0, argv1, NULL};

	if (pid == -1)
	{
	    // error, failed to fork()
	} 
	else if (pid > 0)
	{
	    int status;
	    waitpid(pid, &status, 0);
	    cout<<endl<<"Finished"<<endl;
	    cout<<"Reading transition probability matrix..."<<endl;
	    readFromFile();
	    cout<<"Transition probability matrix read"<<endl;
	    return;
	}
	else 
	{
	    execve("./midi2notes", argv, NULL);
	    _exit(EXIT_FAILURE);   // exec never returns
	}
}

int main(int argc, char* argv[]) {

	DIR *pDIR;
        struct dirent *entry;
	int fileCount = 0, i, j;
	double transitionProbabilityMatrix[12][12];
	double cumulativeTransitionProbabilityMatrix[12][12];
	double sum, factor;

	strcpy(midiDirectory, argv[1]);

	for (i=0; i<12; i++)
		for (j=0; j<12; j++)
			transitionProbabilityMatrix_Sum[i][j] = 0;

	if( pDIR=opendir(midiDirectory) ){
		while(entry = readdir(pDIR)){
                        if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				cout<<endl<<"Reading "<<entry->d_name;
				convertMidi2Notes(entry->d_name);
				fileCount++;
			}
                }
                closedir(pDIR);
        }
	cout<<endl<<fileCount<<" files read."<<endl;
	cout<<"Transition Probability Matrix Generated. "<<endl;

	for (i=0; i<12; i++) {
		sum = 0;
		for (j=0; j<12; j++) {
			transitionProbabilityMatrix[i][j] = transitionProbabilityMatrix_Sum[i][j]/(double)fileCount;
			sum += transitionProbabilityMatrix[i][j];
			cumulativeTransitionProbabilityMatrix[i][j] = sum;
		}
	}
	
	for (i=0; i<12; i++) {
		if (cumulativeTransitionProbabilityMatrix[i][11] != 0)
			factor = 1.0/cumulativeTransitionProbabilityMatrix[i][11];
		else
			factor = 0.0;
		if (factor != 1) {
			for (j=0; j<12; j++) {
				cumulativeTransitionProbabilityMatrix[i][j] = cumulativeTransitionProbabilityMatrix[i][j]*factor;
			}
		}
	}	

	cout<<"Cumulative Transition Probability Matrix Generated: "<<endl;

	for (i=0; i<12; i++) {
		for (j=0; j<12; j++) {
			cout<<cumulativeTransitionProbabilityMatrix[i][j]<<"\t";
		}
		cout<<endl;
	}

	cout<<endl<<"Press any key to generate something...";
	cin.get();

	srand(time(NULL));
	Population population(144, cumulativeTransitionProbabilityMatrix);

	cout<<"Something Generated. Running genetic iterations..."<<endl;

	for (i=0; i<48; i++) {
		population.crossover(1, 0.05, 0.1, 0.05);
		cout<<endl<<i;
	}

	Song song = population.findFittestIndividual();
	song.replace();
	//song.printChromaticNotes_Timeless();
	song.printToSkini();
	cout<<endl<<"Fitness: "<<song.fitness<<endl;
	//cout<<endl<<"Zipf Pitch Fitness: "<<song.zipfPitchFitness;
   	//cout<<endl<<"Zipf Chromatic Pitch Fitness: "<<song.zipfChromaticPitchFitness;
   	//cout<<endl<<"Zipf Pitch Distance Fitness: "<<song.pitchDistanceFitness;
   	//cout<<endl<<"Zipf Chromatic Pitch Distance Fitness: "<<song.chromaticPitchDistanceFitness;
	//cout<<endl<<"Zipf Length: "<<song.zipfLength;
	//cout<<endl<<"Chromatic Zipf Length: "<<song.zipfChromaticLength;
	//cout<<endl<<"Length: "<<song.length;
	cout<<endl<<endl;

	return 0;
}
