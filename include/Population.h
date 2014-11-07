///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////    CONSTRUCTORS    /////////////////////////////////////////////////////////
	
	Population::Population() {
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	Population::Population(long int _size) {
		this->size = _size;
		this->individuals = (Song*)malloc(sizeof(Song)*this->size);
		this->tempSize = _size*_size;
		this->tempIndividuals = (Song*)malloc(sizeof(Song)*this->tempSize);
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	Population::Population(long int _size, double cumulativeTransitionProbabilityMatrix[12][12]) {
	        int i;
	
		this->size = _size;
		this->individuals = (Song*)malloc(sizeof(Song)*this->size);
		this->tempSize = _size*_size;
		this->tempIndividuals = (Song*)malloc(sizeof(Song)*this->tempSize);

	        for (i=0; i<this->size; i++) {
	                individuals[i] = Song(48, cumulativeTransitionProbabilityMatrix);
	        }
	}
	
///////////////////////////////////////////////////////// END OF CONSTRUCTORS  ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////    FITNESS RELATED FUNCTIONS    /////////////////////////////////////////////////
	
	void Population::findFitness() {
	        int i;
	        for (i=0; i<this->size; i++) {
	                individuals[i].findFitness();
	        }

	        for (i=0; i<this->tempSize; i++) {
	                tempIndividuals[i].findFitness();
	        }		
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Song Population::findFittestIndividual() {
	        int i;
	        this->findFitness();
	
	        int fittestIndividualIndex = 0;
	
	        for (i=1; i<this->size; i++) {
	                if (this->individuals[i].fitness > this->individuals[fittestIndividualIndex].fitness) {
	                        fittestIndividualIndex = i;
	                }
	        }
	        cout<<endl<<fittestIndividualIndex<<endl;
	
	        return (this->individuals[fittestIndividualIndex]);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Song Population::findTempFittestIndividual() {
	        int i;
	
	        int fittestIndividualIndex = 0;
	
	        for (i=1; i<this->tempSize; i++) {
	                if ((this->tempIndividuals[i].fitness >= 0) && (this->tempIndividuals[i].fitness > this->tempIndividuals[fittestIndividualIndex].fitness)) {
	                        fittestIndividualIndex = i;
	                }
	        }
	
		this->tempIndividuals[fittestIndividualIndex].fitness = -1;
	        return (this->tempIndividuals[fittestIndividualIndex]);
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Population::findNextGeneration() {
	        int i, j;

		this->findFitness();
		for (i=0; i<this->size; i++) {
			this->individuals[i] = this->findTempFittestIndividual();
		}
	}

///////////////////////////////////////////////// END OF FITNESS RELATED FUNCTIONS  ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////    CROSSOVER RELATED FUNCTIONS    //////////////////////////////////////////////////

	void Population::crossover(double mutationRate, double fusionRate, double elisionRate, double replacementRate) {
		//cout<<"\nStarting crossover";
		long int i, j, tempCounter = 0;
		Song children[2];
	
		for (i=0, tempCounter = 0; i<this->size; i++, tempCounter++) {
			this->tempIndividuals[tempCounter] = this->individuals[i];
		}

		//cout<<"\nCrossing Loop";
		for (i=0; i<this->size-1; i++) {
			for (j=i+1; j<this->size; j++) {
				this->cross(this->individuals[i], this->individuals[j], children);
				this->tempIndividuals[tempCounter] = children[0];
				this->tempIndividuals[tempCounter+1] = children[1];
				tempCounter += 2;
			}
		}

		//cout<<"\nFinding next generation\t";
		this->findNextGeneration();
		//cout<<"Next generation found";

		//cout<<"\nDoing genetic operations\t";
		this->mutate(mutationRate);
		this->fuse(fusionRate);
		this->replace(replacementRate);
		this->elide(elisionRate);
		//cout<<"Genetic operations done";
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Population::cross(Song parent1, Song parent2, Song children[2]) {
		//cout<<"\nStarting cross\t";
		Song child1, child2, piece[5], temp;
		int crossStyle = rand()%3;
		int crossPoint1, crossPoint2, shufflePoint1, shufflePoint2, shufflePoint3, shufflePoint4, i, j;

		//cout<<"\nStarting cross. Style="<<crossStyle<<"\t";
		switch (crossStyle) {
			case 0:
				//cout<<"Starting case 0\t";
				crossPoint1 = rand()%(min(parent1.length, parent2.length));

				child1 = Song(parent2.length);
				for (i=0; i<crossPoint1; i++) {
					child1.addNote(parent1.notes[i]);
				}
				for (i=crossPoint1; i<parent2.length; i++) {
					child1.addNote(parent2.notes[i]);
				}
			
				//cout<<"About to initialize child2\t";
				child2 = Song(parent1.length);
				//cout<<"child2 constructed\t";
				for (i=0; i<crossPoint1; i++) {
					child2.addNote(parent2.notes[i]);
				}
				for (i=crossPoint1; i<parent1.length; i++) {
					child2.addNote(parent1.notes[i]);
				}

				//cout<<"Copying to arguments\t";
				children[0] = child1;
				children[1] = child2;
				//cout<<"Finishing case\t";
				break;

			case 1:
				do {
					crossPoint2 = rand()%(min(parent1.length, parent2.length));
				} while (crossPoint2 < 2);				
			
				do {
					crossPoint1 = rand()%crossPoint2;
				} while (crossPoint1 == 0);
			
				child1 = Song(parent1.length);
				for (i=0; i<crossPoint1; i++) {
					child1.addNote(parent1.notes[i]);
				}
				for (i=crossPoint1; i<crossPoint2; i++) {
					child1.addNote(parent2.notes[i]);
				}
				for (i=crossPoint2; i<parent1.length; i++) {
					child1.addNote(parent1.notes[i]);
				}

				child2 = Song(parent2.length);
				for (i=0; i<crossPoint1; i++) {
					child2.addNote(parent2.notes[i]);
				}
				for (i=crossPoint1; i<crossPoint2; i++) {
					child2.addNote(parent1.notes[i]);
				}
				for (i=crossPoint2; i<parent2.length; i++) {
					child2.addNote(parent2.notes[i]);
				}

				children[0] = child1;
				children[1] = child2;

				break;

			case 2:
				do {
					shufflePoint4 = rand()%parent1.length;
				} while (shufflePoint4 < 4);
				do {
					shufflePoint3 = rand()%shufflePoint4;
				} while (shufflePoint3 < 3);
				do {
					shufflePoint2 = rand()%shufflePoint3;
				} while (shufflePoint2 < 2);
				do {
					shufflePoint1 = rand()%shufflePoint2;
				} while (shufflePoint1 == 0);

				piece[0] = Song(shufflePoint1);
				piece[1] = Song(shufflePoint2 - shufflePoint1);
				piece[2] = Song(shufflePoint3 - shufflePoint2);
				piece[3] = Song(shufflePoint4 - shufflePoint3);
				piece[4] = Song(parent1.length - shufflePoint4);

				for (i=0; i<shufflePoint1; i++) {
					piece[0].addNote(parent1.notes[shufflePoint1-1-i]);
				}
				for (i=shufflePoint1; i<shufflePoint2; i++) {
					piece[1].addNote(parent1.notes[i]);
				}
				for (i=shufflePoint2; i<shufflePoint3; i++) {
					piece[2].addNote(parent1.notes[i]);
				}
				for (i=shufflePoint3; i<shufflePoint4; i++) {
					piece[3].addNote(parent1.notes[i]);
				}
				for (i=shufflePoint4; i<parent1.length; i++) {
					piece[4].addNote(parent1.notes[i]);
				}

				for (i=0; i<3; i++) {
					crossPoint2 = rand()%5;
					crossPoint1 = rand()%5;
					temp = piece[crossPoint1];
					piece[crossPoint1] = piece[crossPoint2];
					piece[crossPoint2] = temp;
				}

				child1 = Song(parent1.length);
				for (i=0; i<5; i++) {
					for (j=0; j<piece[i].length; j++) {
						child1.addNote(piece[i].notes[j]);
					}
				}

				do {
					shufflePoint4 = rand()%parent2.length;
				} while (shufflePoint4 < 4);
				do {
					shufflePoint3 = rand()%shufflePoint4;
				} while (shufflePoint3 < 3);
				do {
					shufflePoint2 = rand()%shufflePoint3;
				} while (shufflePoint2 < 2);
				do {
					shufflePoint1 = rand()%shufflePoint2;
				} while (shufflePoint1 == 0);

				piece[0] = Song(shufflePoint1);
				piece[1] = Song(shufflePoint2 - shufflePoint1);
				piece[2] = Song(shufflePoint3 - shufflePoint2);
				piece[3] = Song(shufflePoint4 - shufflePoint3);
				piece[4] = Song(parent2.length - shufflePoint4);

				for (i=0; i<shufflePoint1; i++) {
					piece[0].addNote(parent2.notes[shufflePoint1-1-i]);
				}
				for (i=shufflePoint1; i<shufflePoint2; i++) {
					piece[1].addNote(parent2.notes[i]);
				}
				for (i=shufflePoint2; i<shufflePoint3; i++) {
					piece[2].addNote(parent2.notes[i]);
				}
				for (i=shufflePoint3; i<shufflePoint4; i++) {
					piece[3].addNote(parent2.notes[i]);
				}
				for (i=shufflePoint4; i<parent2.length; i++) {
					piece[4].addNote(parent2.notes[i]);
				}

				for (i=0; i<3; i++) {
					crossPoint2 = rand()%5;
					crossPoint1 = rand()%5;
					temp = piece[crossPoint1];
					piece[crossPoint1] = piece[crossPoint2];
					piece[crossPoint2] = temp;
				}

				child2 = Song(parent2.length);
				for (i=0; i<5; i++) {
					for (j=0; j<piece[i].length; j++) {
						child2.addNote(piece[i].notes[j]);
					}
				}

				children[0] = child1;
				children[1] = child2;

				break;

			default:
				children[0] = parent1;
				children[1] = parent2;
				break;
		}
		//cout<<"\nFinishing cross";
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Population::mutate(double mutationRate) {
		int *mutationVector;
		int i, mutationPoint;
	
		if (mutationRate == 1) {
			for (i=0; i<this->size; i++) {
				this->individuals[i].mutate();
			}
		}
		else {
			if (this->size <= 0) {
				cout<<"\n\nError in mutationVector initialization. Size = 0";
				cin.get();
			}

			mutationVector = (int*)malloc(sizeof(int)*this->size);	

			for (i=0; i<this->size; i++) {
				mutationVector[i] = 0;
			}
		
			for (i=0; i<(mutationRate*this->size); i++) {
				do {
					mutationPoint = rand()%this->size;
				} while(mutationVector[mutationPoint] == 1);
			
				mutationVector[mutationPoint] = 1;
				this->individuals[mutationPoint].mutate();	
			}

			free(mutationVector);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Population::fuse(double fusionRate) {
		//cout<<"\nFusing\t";
		int *fusionVector;
		int i, fusionPoint, crossPoint1, crossPoint2;
		Song piece, copy;

		if (this->size <= 0) {
			cout<<"\n\nError in fusionVector initialization. Size = 0";
			cin.get();
		}

		fusionVector = (int*)malloc(sizeof(int)*this->size);

		for (i=0; i<this->size; i++) {
			fusionVector[i] = 0;
		}

		for (i=0; i<(fusionRate*this->size); i++) {
			do {
				fusionPoint = rand()%this->size;
			} while(fusionVector[fusionPoint] == 1);
		
			fusionVector[fusionPoint] = 1;

			do {
				crossPoint2 = rand()%this->individuals[fusionPoint].length;
			} while (crossPoint2 < 5);				
	
			do {
				crossPoint1 = rand()%crossPoint2;
			} while (crossPoint2 - crossPoint1 < 4);

			piece = Song(crossPoint2 - crossPoint1 + 1);
			copy = Song(this->individuals[fusionPoint].length);

			for (i=crossPoint1; i<crossPoint2; i++) {
				piece.addNote(this->individuals[fusionPoint].notes[i]);
			}

			for (i=0; i<this->individuals[fusionPoint].length; i++) {
				copy.addNote(this->individuals[fusionPoint].notes[i]);
			}

			this->individuals[fusionPoint].fuse(copy, piece);
		}

		free(fusionVector);
		//cout<<"Fused";
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Population::replace(double replacementRate) {
		int *replacementVector;
		int i, replacementPoint;

		if (this->size <= 0) {
			cout<<"\n\nError in replacementVector initialization. Size = 0";
			cin.get();
		}

		replacementVector = (int*)malloc(sizeof(int)*this->size);

		for (i=0; i<this->size; i++) {
			replacementVector[i] = 0;
		}

		for (i=0; i<(replacementRate*this->size); i++) {
			do {
				replacementPoint = rand()%this->size;
			} while(replacementVector[replacementPoint] == 1);
		
			replacementVector[replacementPoint] = 1;
			this->individuals[replacementPoint].replace();			
		}

		free(replacementVector);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Population::elide(double elisionRate) {
		int *elisionVector;
		int i, elisionPoint;

		if (this->size <= 0) {
			cout<<"\n\nError in elisionVector initialization. Size = 0";
			cin.get();
		}

		elisionVector = (int*)malloc(sizeof(int)*this->size);

		for (i=0; i<this->size; i++) {
			elisionVector[i] = 0;
		}

		for (i=0; i<(elisionRate*this->size); i++) {
			do {
				elisionPoint = rand()%this->size;
			} while(elisionVector[elisionPoint] == 1);
		
			elisionVector[elisionPoint] = 1;
			this->individuals[elisionPoint].elide();			
		}

		free(elisionVector);
	}

////////////////////////////////////////////// END OF CROSSOVER RELATED FUNCTIONS  ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
