class Note;
class Song;
class Population;


#ifndef Population_H
#define Population_H
class Population {
	public:
		long int size;
		long int tempSize;
		Song *individuals;
		Song *tempIndividuals;

		Population();
		Population(long int);
		Population(long int, double(*)[12]);
		void findFitness();
		void findNextGeneration();
		Song findFittestIndividual();
		Song findTempFittestIndividual();
		void crossover(double, double, double, double);
		void cross(Song, Song, Song*);
		void mutate(double);
		void fuse(double);
		void replace(double);
		void elide(double);
};
#endif



#ifndef Note_H
#define Note_H
class Note {
	public:
		long int start;
		long int end;
		long int duration;
		int pitch;
		int chromaticPitch;

		Note();
		Note(int);
		Note(long int, long int, int);
		void clear();
};
#endif



#ifndef Song_H
#define Song_H
class Song {
	public:
		int length;
		int maxLength;
		int zipfLength;
		int zipfChromaticLength;
		Note *notes;
		double fitness;
		double transitionProbabilityMatrix[12][12];

		Song();
		Song(int);
		Song(int, double(*)[12]);
		void preprocess();
		void addNote(Note);
		void findFitness();
		void createTransitionProbabilityMatrix();
		void printAll();
		void printChromaticNotes_Timeless();
		void printAll_Timeless();
		void printFrequencyTable(); 
		void printPitchDistances();
		void printChromaticFrequencyTable();
		void printTransitionProbabilityMatrix();
		void printToSkini();
		void mutate();
		void fuse(Song, Song);
		void replace();
		void elide();
		void clear();

	private:
		int frequencyTable[88];
		int pitchRanks[88];
		int pitchFrequencies[88];
		int pitchDistanceVariances[88];
		int chromaticFrequencyTable[12];
		int chromaticPitchRanks[12];
		int chromaticPitchFrequencies[12];
		int chromaticPitchDistanceVariances[12];
		double zipfPitchFitness;
		double zipfChromaticPitchFitness;
		double pitchDistanceFitness;
		double chromaticPitchDistanceFitness;
		double lz77PitchCompressionFactor;
		double lz77ChromaticPitchCompressionFactor;
		double *zipfChromaticLogFrequencies;
		double *zipfChromaticLogRanks;
		double *zipfLogFrequencies;
		double *zipfLogRanks;
		int **pitchDistances;
		int **chromaticPitchDistances;

		void rankify();
		void chromaticRankify();
		int mostFrequent();
		int chromaticMostFrequent();
		double zipfPitchVariance_independentRank();
		double zipfPitchVariance_independentFrequency();
		double zipfChromaticPitchVariance_independentRank();
		double zipfChromaticPitchVariance_independentFrequency();
		double zipfPitchVariance();
		double zipfChromaticPitchVariance();
		void findPitchDistances();
		void findChromaticPitchDistances();
		double pitchDistanceVariance();
		double chromaticPitchDistanceVariance();
		int areAdjacentNotes(Note, Note);
		int findFollowingChromaticPitchesFor(int, double*);
		int lz77Length();
		int lz77LengthChromatic();
};
#endif
