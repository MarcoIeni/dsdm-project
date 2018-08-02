#ifndef RETIMINGMANAGER_H_
#define RETIMINGMANAGER_H_

#include "SquareMatrix.h"

#include <utility>                          // for std::pair
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>

#define SOURCE 0 // source node

typedef std::pair<int, int> iPair;
// I have chosen adjacency_list over adjacency_matrix, because the number of edges approaches should be less than |V|^2.
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
		boost::no_property, boost::property<boost::edge_weight_t, int> > Graph;
typedef boost::adjacency_matrix<boost::directedS, boost::no_property,
		boost::property<boost::edge_weight_t, int> > DenseGraph;
typedef std::pair<int, int> Edge;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;
typedef boost::property_map<Graph, boost::edge_weight_t>::type weights_map;
typedef boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
typedef boost::graph_traits<Graph> GraphTraits;

class RetimingManager {
public:
	/**
	 * Construct a RetimingManager object with the input graph contained in the
	 * file named filename, with the given control steps and clock period
	 * @param filename the name of the file that contains the input graph
	 * @param control_steps number of flip flops to be put in front of the source node
	 * @param clock_period desired clock period for the application of the retiming algorithm
	 */
	RetimingManager(std::string filename, int control_steps, int clock_period);

	/**
	 * Copy constructor.
	 * Construct a retiming manager with the same characteristics of another one
	 * @param other the retiming manager that you want to copy
	 */
	RetimingManager(const RetimingManager& other);

	/**
	 * Overload of assignment operator. It copies the fields of the given retiming manager.
	 * @param other the retiming manager that you want to copy the fields
	 * @return the starting retiming manager, but with the fields of the other one.
	 */
	RetimingManager& operator=(RetimingManager other);
	virtual ~RetimingManager();

	/**
	 * Get W matrix
	 * @return W matrix
	 */
	SquareMatrix get_w_matrix() const;

	/**
	 * Get D matrix
	 * @return D matrix
	 */
	SquareMatrix get_d_matrix() const;

	/**
	 * Get PC matrix
	 * @return PC matrix
	 */
	SquareMatrix get_pc_matrix() const;

	/**
	 * print in the standard output the main graph
	 */
	void print_main_graph() const;

	/**
	 * write the main graph in the file with the given filename.
	 * If the file does not exist it will be created.
	 * Call this function after you have called the apply_retiming() function
	 * in order to write the output graph.
	 * @param filename the name of the file where you want to write the graph
	 */
	void write_main_graph(std::string filename) const;

	/**
	 * Calculate the scheduling.
	 * Call this function after you have called the apply_retiming() function.
	 * @return a vector that at index i contains the schedule of the i-th instruction
	 */
	std::vector<int> get_scheduling() const;

	/**
	 * apply the full retiming algorithm: calculate w, d and pc matrix;
	 * calculate and analyze pc graph and finally (if it doesn't contain
	 * negative cycles) move flip flops in the main graph.
	 */
	void apply_retiming();

private:
	/**
	 * the graph that will be taken as input and modified to be written in
	 * output.
	 * For this graph, I have chosen adjacency_list over adjacency_matrix,
	 * because the number of edges approaches should be less than |V|^2 in
	 * general.
	 */
	Graph *main_graph;

	/**
	 * A vector that at position i contains the delay of instruction i.
	 * It represents the labels of the nodes of the main graph.
	 */
	std::vector<int> delays;

	/**
	 * w matrix is the cost of the minimum path from row element to column one
	 */
	SquareMatrix *w;

	/**
	 * The D matrix contains elements Dij, where Dij is the largest delay of a
	 * path from node i to node j which has the minimum number of latches.
	 * If i = j than Dii corresponds to delay of node i.
	 */
	SquareMatrix *d;

	/**
	 * This matrix represents a graph where PC(i,j) is the weight of the edge
	 * which goes from node j to node i.
	 */
	SquareMatrix *pc;

	/**
	 * the graph created starting from pC matrix.
	 * In the graph there is one more node, the origin (O), connected to all
	 * nodes. The weight of all edges starting from O is 0.
	 * For this graph, I have chosen adjacency_matrix over adjacency_list,
	 * because the number of edges approaches should be greater than |V|^2
	 */
	DenseGraph *pc_graph;

	/**
	 * the required clock period
	 */
	int clock_period;

	/**
	 * build W and D matrices
	 */
	void build_w_d_matrices();

	/**
	 * build pC matrix
	 */
	void build_pc_matrix();

	/**
	 * build pC graph starting from pC matrix
	 */
	void build_pc_graph();

	/**
	 * Basic Dijkstra algorithm, but with the if statement modified in order to
	 * maximize the delay among all the possible shortest paths.
	 * In addition to this, this function avoids to choose cycles.
	 *
	 * based on examples found on www.geeksforgeeks.org
	 * @param src the dource node
	 * @param dist distance array
	 * @param tot_delays total delays of nodes array
	 */
	void dijkstra_max_delay(int src, int* dist, int* tot_delay);

	/**
	 * Apply bellman ford shortest paths algorithm on the pC graph.
	 * If the algorithm doesn't find negative cycles, then move the flip flops
	 */
	void apply_final_step();

	/**
	 * move the flip flop according to retiming algorithm. This function
	 * is called if and only if there are no negative cycles in the pc
	 * graph
	 * @param pc_distances the minimum distances found applying bellman ford
	 */
	void move_weights(std::vector<int> &pc_distances);

	/**
	 * swap the field of two Retiming Managers.
	 * It is used for the copy constructor and the overload of the
	 * assignment operator.
	 * @param first the first Retiming Manager that you want to swap the fields
	 * @param second the second Retiming Manager that you want to swap the fields
	 */
	friend void swap(RetimingManager& first, RetimingManager& second);

	/**
	 * read the edges from the given Input file stream and save them and their
	 * weights in the given vectors.
	 * Optionally, you can specify the weight after the tail and the head node.
	 * @param infile the Input file stream where the edges start
	 * @param weights the vector of the weights
	 * @param edges the vector of the edges
	 * @param control_steps the control_steps, that will be the weight assigned
	 *                      to the outgoing arcs of the source
	 */
	void read_edges(std::ifstream &infile, std::vector<int> &weights,
			std::vector<Edge> &edges, int control_steps);

	bool is_main_graph_cyclic_util(int v, bool visited[], bool *recStack);
	bool is_main_graph_cyclic();
};

#endif /* RETIMINGMANAGER_H_ */
