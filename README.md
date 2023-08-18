# gba-daw
A small Digital Audio Workstation for the Gameboy Advance


[![Gameboy Advance Tone Row Generator](http://img.youtube.com/vi/htahcGRbNcc/0.jpg)](http://www.youtube.com/watch?v=htahcGRbNcc "Gameboy Advance Tone Row Generator")


# Features
- Metronome with BPM
- Tone-row generator

# Source code
`metronome.cpp` includes code I made for the Gameboy Advance which was originally a metronome. The code currently uses this metronome functionality to play through musical sequences, such as a generated [tone-row matrix](https://musictheory.pugetsound.edu/mt21c/section-194.html). The tone-row matrix can be generated using any given input string ("Hello, world!" in this case).

# Dependencies
To build this project, you will need GNU make, the Gameboy Advance toolchain and the tonc library installed on your system. Installation instructions for the toolchain and tonc can be found here: [https://www.coranac.com/tonc/text/setup.htm](https://www.coranac.com/tonc/text/setup.htm)
