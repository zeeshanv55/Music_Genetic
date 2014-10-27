Note::Note() {
}

Note::Note(int _pitch) {
	this->pitch = _pitch;
	this->chromaticPitch = _pitch%12;
}

Note::Note(long int _start, long int _end, int _pitch) {
	this->start = _start;
	this->end = _end;
	this->duration = _end - _start;
	this->pitch = _pitch;
	this->chromaticPitch = _pitch%12;
}

void Note::clear() {
	this->start = 0;
	this->end = 0;
	this->duration = 0;
	this->pitch = -1;
	this->chromaticPitch = -1;
}
