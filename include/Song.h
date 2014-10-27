#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

void Song::addNote(Note newNote) {
	if (this->length == this->maxLength) {
		cout<<"\nCannot add more. Overflow!";
		return;
	}

	this->notes[this->length] = newNote;
	this->length++;
	this->frequencyTable[newNote.pitch]++;
	this->chromaticFrequencyTable[newNote.chromaticPitch]++;	
}

void Song::preprocess() {
	if (this->length == 0) {
		return;
	}
	
	int i, diff;
	if (this->notes[0].start != 0) {
		diff = this->notes[0].start;
		for (i=0; i<this->length; i++) {
			this->notes[i].start -= diff;
			this->notes[i].end -= diff;	
		}
	}

	for (i=0; i<this->length; i++) {
		this->notes[i].duration = this->notes[i].end - this->notes[i].start;
	}			
}

void Song::findFitness() {
	this->zipfPitchFitness = 1.0/(this->zipfPitchVariance());
	this->zipfChromaticPitchFitness = 1.0/(this->zipfChromaticPitchVariance());
	this->pitchDistanceFitness = ((double)this->zipfLength)/(this->pitchDistanceVariance());
	this->chromaticPitchDistanceFitness = ((double)this->zipfChromaticLength)/(this->chromaticPitchDistanceVariance());
	this->lz77PitchCompressionFactor = (double)this->length/(double)this->lz77Length();
	this->lz77ChromaticPitchCompressionFactor = (double)this->length/(double)this->lz77LengthChromatic();
	this->fitness = (
				(this->zipfPitchFitness) + 
				(this->zipfChromaticPitchFitness) + 
				1.0*(this->pitchDistanceFitness) + 
				1.0*(this->chromaticPitchDistanceFitness)
			//)
			//	/
			//(
			//	this->lz77ChromaticPitchCompressionFactor
			);
}

void Song::clear() {
	this->length = 0;
	this->maxLength = 0;
	this->zipfPitchFitness = -1;
	this->zipfChromaticPitchFitness = -1;
	this->pitchDistanceFitness = -1;
	this->chromaticPitchDistanceFitness = -1;
}

int Song::lz77Length() {
	int lookAheadPtr, searchPtr;
	int outputLength = 0;

	if (this->length == 0)
		return 0;

	for (lookAheadPtr = 0; lookAheadPtr<this->length; lookAheadPtr++) {
		for (searchPtr = 0; searchPtr<lookAheadPtr; searchPtr++ ) {
			if (this->notes[lookAheadPtr].pitch == this->notes[searchPtr].pitch) {
				while (this->notes[lookAheadPtr].pitch == this->notes[searchPtr].pitch) {
					lookAheadPtr++;
					searchPtr++;
				}
			}
		}
		outputLength++;
	}

	return outputLength;
}

int Song::lz77LengthChromatic() {
	int lookAheadPtr, searchPtr;
	int outputLength = 0;

	if (this->length == 0)
		return 0;

	for (lookAheadPtr = 0; lookAheadPtr<this->length; lookAheadPtr++) {
		for (searchPtr = 0; searchPtr<lookAheadPtr; searchPtr++ ) {
			if (this->notes[lookAheadPtr].chromaticPitch == this->notes[searchPtr].chromaticPitch) {
				while (this->notes[lookAheadPtr].chromaticPitch == this->notes[searchPtr].chromaticPitch) {
					lookAheadPtr++;
					searchPtr++;
				}
			}
		}
		outputLength++;
	}

	return outputLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////    GENETIC ALGORITHM RELATED FUNCTIONS    /////////////////////////////////////////////

void Song::mutate() {
	int mutationPoint;
	int newPitch;
		
	mutationPoint = rand()%this->length;
	newPitch = rand()%12;

	this->frequencyTable[this->notes[mutationPoint].pitch] -= 1;
	this->chromaticFrequencyTable[this->notes[mutationPoint].pitch] -= 1;

	this->notes[mutationPoint].pitch = newPitch;
	this->notes[mutationPoint].chromaticPitch = newPitch;	

	this->frequencyTable[newPitch] += 1;
	this->chromaticFrequencyTable[newPitch] += 1;
}

void Song::fuse(Song copy, Song piece) {
	int i;
	
	this->maxLength = (copy.length + piece.length + 1);
	this->length = this->maxLength;
	this->notes = (Note*)malloc(sizeof(Note)*length);

	for (i=0; i<88; i++) {
		this->frequencyTable[i] = 0;
	}

	for (i=0; i<12; i++) {
		this->chromaticFrequencyTable[i] = 0;
	}

	for (i=0; i<copy.length; i++) {
		this->notes[i] = Note(copy.notes[i]);
		this->frequencyTable[copy.notes[i].pitch]++;
		this->chromaticFrequencyTable[copy.notes[i].chromaticPitch]++;
	}

	for (i=0; i<piece.length; i++) {
		this->notes[i+copy.length] = Note(piece.notes[i]);
		this->frequencyTable[piece.notes[i].pitch]++;
		this->chromaticFrequencyTable[piece.notes[i].chromaticPitch]++;
	}
}

void Song::replace() {
	int replacementLength, replacementPoint, pastePoint1, pastePoint2, i;

	replacementPoint = rand()%(this->length-1);
	replacementLength = rand()%(this->length-replacementPoint);
	pastePoint1 = rand()%(this->length-replacementPoint);
	pastePoint2 = rand()%(this->length-replacementPoint);


	for (i=pastePoint1; i<(pastePoint1+replacementLength); i++ ) {
		this->frequencyTable[this->notes[i].pitch]++;
		this->chromaticFrequencyTable[this->notes[i].chromaticPitch]++;

		this->frequencyTable[this->notes[i+replacementLength].pitch]--;
		this->chromaticFrequencyTable[this->notes[i+replacementLength].chromaticPitch]--;

		this->notes[i+replacementLength] = this->notes[i];
	}

	for (i=pastePoint2; i<(pastePoint2+replacementLength); i++ ) {
		this->frequencyTable[this->notes[i].pitch]++;
		this->chromaticFrequencyTable[this->notes[i].chromaticPitch]++;

		this->frequencyTable[this->notes[i+replacementLength].pitch]--;
		this->chromaticFrequencyTable[this->notes[i+replacementLength].chromaticPitch]--;

		this->notes[i+replacementLength] = this->notes[i];
	}
}

void Song::elide() {
	int i, elisionPoint;

	elisionPoint = rand()%this->length;
	this->frequencyTable[this->notes[elisionPoint].pitch] -= 1;
	this->chromaticFrequencyTable[this->notes[elisionPoint].pitch] -= 1;

	for (i=elisionPoint; i<this->length-1; i++) {
		this->notes[i] = this->notes[i+1];
	}

	this->length -= 1;
	this->maxLength -= 1;
}

//////////////////////////////////////////// END OF GENETIC ALGORITHM RELATED FUNCTIONS  //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////    CONSTRUCTORS    /////////////////////////////////////////////////////////
	
	Song::Song() {
		this->length = 0;
		int i;
	
		for (i=0; i<88; i++)
			this->frequencyTable[i] = 0;
	
		for (i=0; i<12; i++)
			this->chromaticFrequencyTable[i] = 0;
	}

/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	Song::Song(int _maxLength) {
		this->maxLength = _maxLength;
		this->notes = (Note*)malloc(sizeof(Note)*this->maxLength);
		this->length = 0;
		int i;
		
		for (i=0; i<88; i++)
			this->frequencyTable[i] = 0;
	
		for (i=0; i<12; i++)
			this->chromaticFrequencyTable[i] = 0;
	}
	
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Song::Song(int _length, double cumulativeTransitionProbabilityMatrix[12][12]) {
		int i, j, currentPitch, randomNumber;
	
		this->length = this->maxLength = _length;
		this->notes = (Note*)malloc(sizeof(Note)*length);
	
		for (i=0; i<88; i++)
			this->frequencyTable[i] = 0;
	
		for (i=0; i<12; i++)
			this->chromaticFrequencyTable[i] = 0;
	
		currentPitch = 3;
		this->notes[0] = Note(currentPitch);
		this->frequencyTable[currentPitch]++;
		this->chromaticFrequencyTable[currentPitch]++;
	
		for (i=1; i<_length; i++) {
			randomNumber = rand()%100000;
			for (j=0; j<12; j++) {
				if (cumulativeTransitionProbabilityMatrix[currentPitch][j] > (double)randomNumber/100000.0)
					break;
			}
			currentPitch = j;
			this->notes[i] = Note(currentPitch);
			this->frequencyTable[currentPitch]++;
			this->chromaticFrequencyTable[currentPitch]++;
		}
	}
	
///////////////////////////////////////////////////////// END OF CONSTRUCTORS  ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////    PRINT FUNCTIONS    ////////////////////////////////////////////////////////

	void Song::printAll() {
		int i;
		for(i=0; i<this->length; i++) {
			cout<<endl<<this->notes[i].start<<"\t"<<this->notes[i].end<<"\t"<<this->notes[i].pitch<<"\t"<<this->notes[i].chromaticPitch;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Song::printAll_Timeless() {
		int i;
		for(i=0; i<this->length; i++) {
			cout<<endl<<this->notes[i].pitch<<"\t"<<this->notes[i].chromaticPitch;
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Song::printToSkini() {
		ofstream skiniFile;
		int i, temp;
		char _2_0[] = "1.000000";
		char _1_0[] = "0.750000";
		char _0_5[] = "0.330000";
		char _0_25[] = "0.250000";
		
		int _2_0p = 0;
		int _1_0p= 0;
		int _0_5p= 0;
		int _0_25p= 0;
	
		this->notes[0].pitch = this->notes[0].chromaticPitch + 57;
		for (i=1; i<this->length; i++) {
			temp = this->notes[i].chromaticPitch + 21;
			while ( this->notes[i-1].pitch-temp > 6 ) {
				temp += 12;
			}

			if (temp > 83)
				this->notes[i].pitch = temp-24;
			else if (temp < 36)
				this->notes[i].pitch = temp+24;
			else
				this->notes[i].pitch = temp;
		}
		
		skiniFile.open("output.ski");

		for (i=0; i<this->length; i++) {
			skiniFile<<"NoteOn\t0.000000\t1\t"<<this->notes[i].pitch<<"\t100\n";

			if (this->notes[i].pitch == this->notes[i-1].pitch) {
				if (i>0 && _2_0p == 1) {
					skiniFile<<"NoteOff\t"<<_0_5<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
					_2_0p = 0;
					_1_0p= 0;
					_0_5p= 1;
					_0_25p= 0;
				}
				else if (i>1 && _1_0p == 1 && this->notes[i-2].pitch==this->notes[i].pitch) {
					skiniFile<<"NoteOff\t"<<_0_25<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
					_2_0p = 0;
					_1_0p= 0;
					_0_5p= 0;
					_0_25p= 1;
				}
				else {
					skiniFile<<"NoteOff\t"<<_2_0<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
					_2_0p = 1;
					_1_0p= 0;
					_0_5p= 0;
					_0_25p= 0;
				}
			}

			else if (
			this->notes[i].chromaticPitch == 1 ||
			this->notes[i].chromaticPitch == 4 ||
			this->notes[i].chromaticPitch == 6 ||
			this->notes[i].chromaticPitch == 9 ||
			this->notes[i].chromaticPitch == 11
			) {
				skiniFile<<"NoteOff\t"<<_0_25<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
				_2_0p = 0;
				_1_0p= 0;
				_0_5p= 0;
				_0_25p= 1;
			}
	
			else if (this->notes[i].chromaticPitch == 3) {
				skiniFile<<"NoteOff\t"<<_1_0<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
				_2_0p = 0;
				_1_0p= 1;
				_0_5p= 0;
				_0_25p= 0;
			}
	
			else if (this->notes[i].chromaticPitch == this->zipfChromaticLength-1) {
				skiniFile<<"NoteOff\t"<<_2_0<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
				_2_0p = 1;
				_1_0p= 0;
				_0_5p= 0;
				_0_25p= 0;
			}
			else {
				skiniFile<<"NoteOff\t"<<_0_5<<"\t1\t"<<this->notes[i].pitch<<"\t100\n";
				_2_0p = 0;
				_1_0p= 0;
				_0_5p= 1;
				_0_25p= 0;
			}
	    	}

		skiniFile.close();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Song::printChromaticNotes_Timeless() {
		int i;
		for(i=0; i<this->length; i++) {
			switch (notes[i].chromaticPitch) {
				case 0:
					cout<<endl<<"A";
					break;
				case 1:
					cout<<endl<<"A#";
					break;
				case 2:
					cout<<endl<<"B";
					break;
				case 3:
					cout<<endl<<"C";
					break;
				case 4:
					cout<<endl<<"C#";
					break;
				case 5:
					cout<<endl<<"D";
					break;
				case 6:
					cout<<endl<<"D#";
					break;
				case 7:
					cout<<endl<<"E";
					break;
				case 8:
					cout<<endl<<"F";
					break;
				case 9:
					cout<<endl<<"F#";
					break;
				case 10:
					cout<<endl<<"G";
					break;
				case 11:
					cout<<endl<<"G#";
					break;
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Song::printFrequencyTable() {
		int i;
		for (i=0; i<this->zipfLength; i++) {
			cout<<endl<<this->pitchRanks[i]<<"\t"<<this->pitchFrequencies[i];
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Song::printPitchDistances() {
		int i, j;
		for (i=0; i<this->zipfLength; i++) {
			for (j=0; this->pitchDistances[i][j-1]!=-1; j++) {
				cout<<this->pitchDistances[i][j]<<"  ";
			}
			cout<<endl;
		}
	}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Song::printChromaticFrequencyTable() {
		int i;
		for (i=0; i<12; i++) {
			cout<<endl<<this->chromaticPitchRanks[i]<<"\t"<<this->chromaticPitchFrequencies[i];
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void Song::printTransitionProbabilityMatrix() {
		int i, j;
		cout<<endl;
		for (i=0; i<12; i++) {
			for (j=0; j<12; j++) {
				cout<<this->transitionProbabilityMatrix[i][j]<<"\t";
			}
			cout<<endl;
		}
	}
	
////////////////////////////////////////////////////// END OF PRINT FUNCTIONS  ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////    TRANSITION PROBABILITY MATRIX RELATED FUNCTIONS    //////////////////////////////////////////
/**/
/**/	int Song::areAdjacentNotes(Note note_1, Note note_2) {
/**/		if (note_2.start - note_1.end >= 0 && note_2.start - note_1.end < 10)
/**/			return 1;
/**/		else
/**/			if (note_2.start - note_1.end > 10)
/**/				return -1;
/**/			else
/**/				return 0;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	int Song::findFollowingChromaticPitchesFor(int currentPitch, double followingPitchOccurances[]) {
/**/		int i, j, sum = 0;
/**/
/**/		for (i=0; i<12; i++)
/**/			followingPitchOccurances[i] = 0;
/**/			
/**/		for (i=0; i<this->length; i++) {
/**/			if (this->notes[i].chromaticPitch == currentPitch) {
/**/				for (j=i+1; j<this->length; j++) {
/**/					if (areAdjacentNotes(this->notes[i], this->notes[j]) == 1) {
/**/						followingPitchOccurances[this->notes[j].chromaticPitch]++;
/**/					}
/**/					if (areAdjacentNotes(this->notes[i], this->notes[j]) == -1) {
/**/						break;
/**/					}
/**/				}
/**/			}
/**/		}
/**/	
/**/		for (i=0; i<12; i++) {
/**/			sum += followingPitchOccurances[i];
/**/		}
/**/
/**/		return sum;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	void Song::createTransitionProbabilityMatrix() {
/**/		int i, j;
/**/		for (i=0; i<12; i++) {
/**/			for (j=0; j<12; j++) {
/**/				this->transitionProbabilityMatrix[i][j] = 0.0;
/**/			}
/**/		}
/**/	
/**/		int currentPitch;
/**/		double followingPitchOccurances[12];
/**/		double totalPitchOccurances;
/**/
/**/		for (currentPitch = 0; currentPitch < 12; currentPitch++) {
/**/			totalPitchOccurances = this->findFollowingChromaticPitchesFor(currentPitch, followingPitchOccurances);
/**/			for (i=0; i<12; i++) {
/**/				if (totalPitchOccurances == 0)
/**/					this->transitionProbabilityMatrix[currentPitch][i] = 0;
/**/				else
/**/					this->transitionProbabilityMatrix[currentPitch][i] = (double)(followingPitchOccurances[i]/totalPitchOccurances);
/**/			}
/**/		}
/**/	}
/**/	
///////////////////////////////////// END OF TRANSITION PROBABILITY MATRIX RELATED FUNCTIONS  /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////    ZIPF LAW RELATED FUNCTIONS    /////////////////////////////////////////////////////
/**/
/**/	int Song::mostFrequent() {
/**/		int i;
/**/		int max = -1;
/**/		for (i=0; i<88; i++) {
/**/			if (max == -1 || frequencyTable[i] > frequencyTable[max])
/**/				max = i;
/**/		}
/**/		return max;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	int Song::chromaticMostFrequent() {
/**/		int i;
/**/		int max = -1;
/**/		for (i=0; i<12; i++) {
/**/			if (max == -1 || chromaticFrequencyTable[i] > chromaticFrequencyTable[max])
/**/				max = i;
/**/		}
/**/		return max;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	void Song::rankify() {
/**/		int i;
/**/		int _frequencyTable[88];
/**/	
/**/		for (i=0; i<88; i++)
/**/			_frequencyTable[i] = frequencyTable[i];
/**/	
/**/		for (i=0; i<88; i++) {
/**/			pitchRanks[i] = mostFrequent();
/**/			pitchFrequencies[i] = frequencyTable[pitchRanks[i]];
/**/			frequencyTable[pitchRanks[i]] = 0;
/**/		}
/**/	
/**/		for (i=0; i<88; i++)
/**/			frequencyTable[i] = _frequencyTable[i];
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	void Song::chromaticRankify() {
/**/		int i;
/**/		int _frequencyTable[12];
/**/		
/**/		for (i=0; i<12; i++)
/**/			_frequencyTable[i] = chromaticFrequencyTable[i];
/**/		
/**/		for (i=0; i<12; i++) {
/**/			chromaticPitchRanks[i] = chromaticMostFrequent();
/**/			chromaticPitchFrequencies[i] = chromaticFrequencyTable[chromaticPitchRanks[i]];
/**/			chromaticFrequencyTable[chromaticPitchRanks[i]] = 0;
/**/		}
/**/	
/**/		for (i=0; i<12; i++)
/**/			chromaticFrequencyTable[i] = _frequencyTable[i];
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	double Song::zipfPitchVariance_independentRank() {
/**/		int i;
/**/	
/**/		this->zipfLogRanks = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		this->zipfLogFrequencies = (double*)malloc(sizeof(double)*this->zipfLength);
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			zipfLogRanks[i] = log(i+1);
/**/			zipfLogFrequencies[i] = log(pitchFrequencies[i]);
/**/		}
/**/	
/**/		double *XY, *X2, sumX=0.0, sumY=0.0, sumXY=0.0, sumX2=0.0, zipfSlope, zipfIntercept;
/**/		XY = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		X2 = (double*)malloc(sizeof(double)*this->zipfLength);
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			XY[i] = zipfLogRanks[i]*zipfLogFrequencies[i];
/**/			X2[i] = zipfLogRanks[i]*zipfLogRanks[i];
/**/		}
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			sumX += zipfLogRanks[i];
/**/			sumY += zipfLogFrequencies[i];
/**/			sumXY += XY[i];
/**/			sumX2 += X2[i];
/**/		}
/**/	
/**/		zipfSlope = ((zipfLength * sumXY)-(sumX*sumY))/((zipfLength*sumX2)-(sumX*sumX));
/**/		zipfIntercept = (sumY-(zipfSlope*sumX))/zipfLength;
/**/	
/**/		double *diff, *squaredDiff, expectedVal, sumSquaredDiff=0.0, result;
/**/	
/**/		diff = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		squaredDiff = (double*)malloc(sizeof(double)*this->zipfLength);
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			expectedVal = zipfIntercept + (zipfSlope*zipfLogRanks[i]);
/**/			diff[i] = expectedVal - zipfLogFrequencies[i];
/**/			squaredDiff[i] = diff[i]*diff[i];
/**/			sumSquaredDiff += squaredDiff[i];
/**/		}
/**/	
/**/		result = sumSquaredDiff/zipfLength;
/**/		
/**/		return result;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	double Song::zipfPitchVariance_independentFrequency() {
/**/		int i;
/**/	
/**/		double *XY, *X2, sumX=0.0, sumY=0.0, sumXY=0.0, sumX2=0.0, zipfSlope, zipfIntercept;
/**/		XY = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		X2 = (double*)malloc(sizeof(double)*this->zipfLength);
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			XY[i] = zipfLogRanks[i]*zipfLogFrequencies[i];
/**/			X2[i] = zipfLogFrequencies[i]*zipfLogFrequencies[i];
/**/		}
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			sumX += zipfLogFrequencies[i];
/**/			sumY += zipfLogRanks[i];
/**/			sumXY += XY[i];
/**/			sumX2 += X2[i];
/**/		}
/**/	
/**/		zipfSlope = ((zipfLength * sumXY)-(sumX*sumY))/((zipfLength*sumX2)-(sumX*sumX));
/**/		zipfIntercept = (sumY-(zipfSlope*sumX))/zipfLength;
/**/	
/**/		double *diff, *squaredDiff, expectedVal, sumSquaredDiff=0.0, result;
/**/	
/**/		diff = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		squaredDiff = (double*)malloc(sizeof(double)*this->zipfLength);
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			expectedVal = zipfIntercept + (zipfSlope*zipfLogFrequencies[i]);
/**/			diff[i] = expectedVal - zipfLogRanks[i];
/**/			squaredDiff[i] = diff[i]*diff[i];
/**/			sumSquaredDiff += squaredDiff[i];
/**/		}
/**/	
/**/		result = sumSquaredDiff/zipfLength;
/**/		
/**/		return result;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	double Song::zipfPitchVariance() {
/**/		int i;
/**/	
/**/		this->rankify();	
/**/	
/**/		for (i=0; i<88; i++) {
/**/			if (pitchFrequencies[i] == 0) {
/**/				break;
/**/			}
/**/		}
/**/		zipfLength = i;
/**/	
/**/		this->zipfLogRanks = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		this->zipfLogFrequencies = (double*)malloc(sizeof(double)*this->zipfLength);
/**/	
/**/		for (i=0; i<zipfLength; i++) {
/**/			zipfLogRanks[i] = log(i+1);
/**/			zipfLogFrequencies[i] = log(pitchFrequencies[i]);
/**/		}
/**/	
/**/		double independentRank = this->zipfPitchVariance_independentRank();
/**/		double independentFrequency = this->zipfPitchVariance_independentFrequency();
/**/	
/**/		return (independentRank+independentFrequency)/2.0;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	double Song::zipfChromaticPitchVariance_independentRank() {
/**/		int i;
/**/	
/**/		this->zipfChromaticLogRanks = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		this->zipfChromaticLogFrequencies = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			zipfChromaticLogRanks[i] = log(i+1);
/**/			zipfChromaticLogFrequencies[i] = log(chromaticPitchFrequencies[i]);
/**/		}
/**/	
/**/		double *XY, *X2, sumX=0.0, sumY=0.0, sumXY=0.0, sumX2=0.0, zipfSlope, zipfIntercept;
/**/		XY = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		X2 = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			XY[i] = zipfChromaticLogRanks[i]*zipfChromaticLogFrequencies[i];
/**/			X2[i] = zipfChromaticLogRanks[i]*zipfChromaticLogRanks[i];
/**/		}
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			sumX += zipfChromaticLogRanks[i];
/**/			sumY += zipfChromaticLogFrequencies[i];
/**/			sumXY += XY[i];
/**/			sumX2 += X2[i];
/**/		}
/**/	
/**/		zipfSlope = ((zipfChromaticLength * sumXY)-(sumX*sumY))/((zipfChromaticLength*sumX2)-(sumX*sumX));
/**/		zipfIntercept = (sumY-(zipfSlope*sumX))/zipfChromaticLength;
/**/	
/**/		double *diff, *squaredDiff, expectedVal, sumSquaredDiff=0.0, result;
/**/	
/**/		diff = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		squaredDiff = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			expectedVal = zipfIntercept + (zipfSlope*zipfChromaticLogRanks[i]);
/**/			diff[i] = expectedVal - zipfChromaticLogFrequencies[i];
/**/			squaredDiff[i] = diff[i]*diff[i];
/**/			sumSquaredDiff += squaredDiff[i];
/**/		}
/**/	
/**/		result = sumSquaredDiff/zipfChromaticLength;
/**/		
/**/		return result;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	double Song::zipfChromaticPitchVariance_independentFrequency() {
/**/		int i;
/**/	
/**/		double *XY, *X2, sumX=0.0, sumY=0.0, sumXY=0.0, sumX2=0.0, zipfSlope, zipfIntercept;
/**/		XY = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		X2 = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			XY[i] = zipfChromaticLogRanks[i]*zipfChromaticLogFrequencies[i];
/**/			X2[i] = zipfChromaticLogFrequencies[i]*zipfChromaticLogFrequencies[i];
/**/		}
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			sumX += zipfChromaticLogFrequencies[i];
/**/			sumY += zipfChromaticLogRanks[i];
/**/			sumXY += XY[i];
/**/			sumX2 += X2[i];
/**/		}
/**/	
/**/		zipfSlope = ((zipfChromaticLength * sumXY)-(sumX*sumY))/((zipfChromaticLength*sumX2)-(sumX*sumX));
/**/		zipfIntercept = (sumY-(zipfSlope*sumX))/zipfChromaticLength;
/**/	
/**/		double *diff, *squaredDiff, expectedVal, sumSquaredDiff=0.0, result;
/**/	
/**/		diff = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		squaredDiff = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			expectedVal = zipfIntercept + (zipfSlope*zipfChromaticLogFrequencies[i]);
/**/			diff[i] = expectedVal - zipfChromaticLogRanks[i];
/**/			squaredDiff[i] = diff[i]*diff[i];
/**/			sumSquaredDiff += squaredDiff[i];
/**/		}
/**/	
/**/		result = sumSquaredDiff/zipfChromaticLength;
/**/		
/**/		return result;
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/	
/**/	double Song::zipfChromaticPitchVariance() {
/**/		int i;
/**/	
/**/		this->chromaticRankify();	
/**/	
/**/		for (i=0; i<12; i++) {
/**/			if (chromaticPitchFrequencies[i] == 0) {
/**/				break;
/**/			}
/**/		}
/**/		zipfChromaticLength = i;
/**/	
/**/		this->zipfChromaticLogRanks = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		this->zipfChromaticLogFrequencies = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/	
/**/		for (i=0; i<zipfChromaticLength; i++) {
/**/			zipfChromaticLogRanks[i] = log(i+1);
/**/			zipfChromaticLogFrequencies[i] = log(chromaticPitchFrequencies[i]);
/**/		}
/**/	
/**/		double independentRank = this->zipfChromaticPitchVariance_independentRank();
/**/		double independentFrequency = this->zipfChromaticPitchVariance_independentFrequency();
/**/	
/**/		return (independentRank+independentFrequency)/2.0;
/**/	}
/**/	
/////////////////////////////////////////////// END OF ZIPF LAW RELATED FUNCTIONS  ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// PITCH DISTANCE RELATED FUNCTIONS //////////////////////////////////////////////////////
/**/
/**/	void Song::findPitchDistances() {
/**/		int i, j, k, numberOfOccurances, currentPitch;
/**/		
/**/		this->pitchDistances = (int**)malloc(sizeof(int*)*this->zipfLength);
/**/		for (i=0; i<this->zipfLength; i++) {
/**/			this->pitchDistances[i] = (int*)malloc(sizeof(int)*this->length);
/**/		}
/**/		
/**/		for (i=0; i<this->zipfLength; i++) {
/**/			currentPitch = pitchRanks[i];
/**/			numberOfOccurances = 0;
/**/			for (j=0, k=0; j<this->length; j++, k++) {
/**/				if (this->notes[j].pitch == currentPitch) {
/**/					this->pitchDistances[i][numberOfOccurances] = k;
/**/					numberOfOccurances += 1;
/**/					k = -1;
/**/				}
/**/			}
/**/			this->pitchDistances[i][numberOfOccurances] = -1;
/**/		} 
/**/	}
/**/
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	double Song::pitchDistanceVariance() {
/**/		int i, j;
/**/		double *variances, sum, mean, diff, squaredDiff, result;
/**/
/**/		variances = (double*)malloc(sizeof(double)*this->zipfLength);
/**/		this->findPitchDistances();
/**/		for (i=0; i<this->zipfLength; i++) {
/**/			sum = 0;
/**/			for (j=0; this->pitchDistances[i][j]!=-1; j++) {
/**/				sum += this->pitchDistances[i][j];
/**/			}
/**/				
/**/			mean = sum/(double)j;
/**/			sum = 0;
/**/	
/**/			for (j=0; this->pitchDistances[i][j]!=-1; j++) {
/**/				diff = this->pitchDistances[i][j] - mean;
/**/				squaredDiff = diff*diff;
/**/				sum += squaredDiff;
/**/			}
/**/	
/**/			variances[i] = sum/(double)j;
/**/		}
/**/
/**/		sum = 0;
/**/		for (i=0; i<this->zipfLength; i++) {
/**/			sum += variances[i];
/**/		}
/**/
/**/		result = sum/(double)this->zipfLength;
/**/
/**/		return result;
/**/	}
/**/	
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	void Song::findChromaticPitchDistances() {
/**/		int i, j, k, numberOfOccurances, currentPitch;
/**/		
/**/		this->chromaticPitchDistances = (int**)malloc(sizeof(int*)*this->zipfChromaticLength);
/**/		for (i=0; i<this->zipfChromaticLength; i++) {
/**/			this->chromaticPitchDistances[i] = (int*)malloc(sizeof(int)*this->length);
/**/		}
/**/		
/**/		for (i=0; i<this->zipfChromaticLength; i++) {
/**/			currentPitch = chromaticPitchRanks[i];
/**/			numberOfOccurances = 0;
/**/			for (j=0, k=0; j<this->length; j++, k++) {
/**/				if (this->notes[j].chromaticPitch == currentPitch) {
/**/					this->chromaticPitchDistances[i][numberOfOccurances] = k;
/**/					numberOfOccurances += 1;
/**/					k = -1;
/**/				}
/**/			}
/**/			this->chromaticPitchDistances[i][numberOfOccurances] = -1;
/**/		} 
/**/	}
/**/	
/**////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
/**/	double Song::chromaticPitchDistanceVariance() {
/**/		int i, j;
/**/		double *variances, sum, mean, diff, squaredDiff, result;
/**/
/**/		variances = (double*)malloc(sizeof(double)*this->zipfChromaticLength);
/**/		this->findChromaticPitchDistances();
/**/		for (i=0; i<this->zipfChromaticLength; i++) {
/**/			sum = 0;
/**/			for (j=0; this->chromaticPitchDistances[i][j]!=-1; j++) {
/**/				sum += this->chromaticPitchDistances[i][j];
/**/			}
/**/				
/**/			mean = sum/(double)j;
/**/			sum = 0;
/**/	
/**/			for (j=0; this->chromaticPitchDistances[i][j]!=-1; j++) {
/**/				diff = this->chromaticPitchDistances[i][j] - mean;
/**/				squaredDiff = diff*diff;
/**/				sum += squaredDiff;
/**/			}
/**/	
/**/			variances[i] = sum/(double)j;
/**/		}
/**/
/**/		sum = 0;
/**/		for (i=0; i<this->zipfChromaticLength; i++) {
/**/			sum += variances[i];
/**/		}
/**/
/**/		result = sum/(double)this->zipfChromaticLength;
/**/
/**/		return result;
/**/	}
/**/
///////////////////////////////////////////   END OF PITCH DISTANCE RELATED FUNCTIONS  ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
