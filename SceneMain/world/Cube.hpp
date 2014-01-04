#ifndef CUBE_HPP
#define CUBE_HPP

struct Cube {
		Cube(unsigned char ID, unsigned char light) : ID(ID), light(light) {}
		Cube() : ID(0), light(MINLIGHT) {}
		unsigned char ID, light;
};

#endif // CUBE_HPP
