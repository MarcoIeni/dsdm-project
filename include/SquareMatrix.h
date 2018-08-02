#ifndef SQUAREMATRIX_H_
#define SQUAREMATRIX_H_

#define INF 0x3f3f3f3f

/**
 * Simple implementation of square matrix.
 */
class SquareMatrix {
public:
	/**
	 * construct a square matrix with a given side
	 * @param side the side of the matrix
	 */
	explicit SquareMatrix(unsigned int side);

	/**
	 * construct a square matrix with a given side, where all the elements are
	 * initialized to a given default value
	 * @param side the side of the matrix
	 * @param default_value the value that you want to assign to the elements
	 */
	SquareMatrix(unsigned int side, int default_value);

	/**
	 * Copy constructor.
	 * Construct a square matrix with the same characteristics of another one
	 * @param other the matrix that you want to copy
	 */
	SquareMatrix(const SquareMatrix &other);

	/**
	 * Overload of assignment operator. It copies the elements of the given matrix.
	 * @param other the matrix that you want to copy the elements
	 * @return the starting matrix, but with the elements of the other one.
	 */
	SquareMatrix & operator=(const SquareMatrix & other);
	virtual ~SquareMatrix();

	/**
	 * get raw pointer to matrix
	 * @return pointer to matrix
	 */
	int** get() const;

	/**
	 * get the element at row i and at column j
	 * @param i row index
	 * @param j column index
	 * @return element matrix[i][j]
	 */
	int get(unsigned int i, unsigned int j) const;

	/**
	 * set the element at row i and at column j at the given value
	 * @param i row index
	 * @param j column index
	 * @param value the value to set
	 */
	void set(unsigned int i, unsigned int j, int value);

	/**
	 * get the side of the matrix
	 * @return the side of the matrix
	 */
	unsigned int get_side() const;

	/**
	 * print the matrix in the standard output with row and column indices
	 */
	void print_matrix() const;

private:
	int** matrix;
	unsigned int side;
};

#endif /* SQUAREMATRIX_H_ */
