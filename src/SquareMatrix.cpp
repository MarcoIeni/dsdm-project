#include "SquareMatrix.h"

#include <iostream>

SquareMatrix::SquareMatrix(unsigned int side) {
	this->side = side;
	matrix = new int*[side];
	for (unsigned int i = 0; i < side; i++) {
		matrix[i] = new int[side];
	}
}

SquareMatrix::SquareMatrix(const SquareMatrix &other) {
	this->side = other.side;
	matrix = new int*[side];
	for (unsigned int i = 0; i < side; i++) {
		matrix[i] = new int[side];
	}
	*this = other;
}

SquareMatrix::SquareMatrix(unsigned int side, int default_value) :
		SquareMatrix(side) {
	for (unsigned int i = 0; i < side; i++) {
		std::fill_n(matrix[i], side, default_value);
	}
}

SquareMatrix& SquareMatrix::operator=(const SquareMatrix & other) {
	if (this->side != other.side) {
		throw "Different matrix size";
	} else {
		for (unsigned int i = 0; i < side; i++) {
			for (unsigned int j = 0; j < side; j++) {
				set(i, j, other.get(i, j));
			}
		}
	}
	return *this;
}

SquareMatrix::~SquareMatrix() {
	for (unsigned int i = 0; i < side; i++) {
		delete[] matrix[i]; // delete array within matrix
	}
	// delete actual matrix
	delete[] matrix;
}

int** SquareMatrix::get() const {
	return matrix;
}

int SquareMatrix::get(unsigned int i, unsigned int j) const {
	return matrix[i][j];
}

void SquareMatrix::set(unsigned int i, unsigned int j, int value) {
	matrix[i][j] = value;
}

unsigned int SquareMatrix::get_side() const {
	return side;
}

void SquareMatrix::print_matrix() const {
	// print column indices
	unsigned int i;
	for (i = 0; i < side; i++) {
		std::cout << "\t" << i << ":";
	}
	std::cout << "\n";
	for (i = 0; i < side; i++) {
		std::cout << i << ":\t"; // print row index
		for (unsigned int j = 0; j < side; j++) {
			if (matrix[i][j] == INF) {
				std::cout << "INF";
			} else {
				std::cout << matrix[i][j];
			}
			std::cout << "\t";
		}
		std::cout << std::endl;
	}
}
