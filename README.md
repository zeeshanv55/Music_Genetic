Music_Genetic
=============

A project to generate music based on the style of an artist/genre.
Timing, duration and silence between notes not taken into account for now.
Input: midi music sheets in a folder.
Output: skini music sheet (easily converted to .wav format music using stk toolkit).

To run:

1. make
2. cd bin
3. ./main <midi folder>
   For example, ./main ../midis/The_Beatles
4. ./demo <instrument name> -ow <output file name> -if output.ski
   For example, ./demo Plucked -ow output.wav -if output.ski
