#include "RetimingManager.h"

#include <iostream>
#include <fstream>
#include <list>

#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <bits/stdc++.h> // for priority queue

RetimingManager::RetimingManager(const std::string filename, int control_steps,
		int clock_period) {
	this->clock_period = clock_period;
	pc_graph = nullptr;
	std::ifstream infile(filename);
	if (!infile) {
		std::cerr << "file not found" << std::endl;
	}
	std::string first_line;
	std::getline(infile, first_line);
	std::stringstream s(first_line);

	/*
	 * the first line of the input file contains the delay of each node,
	 * so I save them into the delays vector
	 */
	int delay;
	while (s >> delay) {
		delays.push_back(delay);
	}
	const unsigned int num_vertices = delays.size();

	std::vector<int> weights;
	std::vector<Edge> edges;
	read_edges(infile, weights, edges, control_steps);

	main_graph = new Graph(&edges[0], &edges[edges.size()], &weights[0],
			num_vertices);
	w = new SquareMatrix(num_vertices, INF);
	d = new SquareMatrix(num_vertices, 0);
	pc = new SquareMatrix(num_vertices);
}

void RetimingManager::read_edges(std::ifstream &infile,
		std::vector<int> &weights, std::vector<Edge> &edges,
		int control_steps) {
	std::string second_line;
	std::getline(infile, second_line);

	//take numbers from string and put it into vector
	std::stringstream stream(second_line);
	std::vector<int> values((std::istream_iterator<int>(stream)), // begin
			(std::istream_iterator<int>()));      // end

	int tail_node, head_node;
	tail_node = values[0];
	head_node = values[1];

	if (values.size() == 3) {
		// the weight is specified in the input file
		int weight = values[2];
		do {
			edges.push_back(Edge(tail_node, head_node));
			if (tail_node == SOURCE) { // edge outgoing from entry
				weights.push_back(control_steps);
			} else {
				weights.push_back(weight);
			}
		} while (infile >> tail_node >> head_node >> weight);
	} else {
		// the weight is not specified in the input file
		do {
			edges.push_back(Edge(tail_node, head_node));
			if (tail_node == SOURCE) { // edge outgoing from entry
				weights.push_back(control_steps);
			} else {
				weights.push_back(0);
			}
		} while (infile >> tail_node >> head_node);
	}
}

void RetimingManager::apply_retiming() {
	if (is_main_graph_cyclic()) {
		throw "input graph contains cycles, you cannot apply retiming.";
	}
	build_w_d_matrices();
	build_pc_matrix();
	build_pc_graph();
	apply_final_step();
}

void swap(RetimingManager& first, RetimingManager& second) {
	// enable ADL
	using std::swap;

	// by swapping the members of two objects,
	// the two objects are effectively swapped
	swap(first.main_graph, second.main_graph);
	swap(first.pc_graph, second.pc_graph);
	swap(first.delays, second.delays);
	swap(first.clock_period, second.clock_period);
	swap(first.w, second.w);
	swap(first.d, second.d);
	swap(first.pc, second.pc);
}

RetimingManager& RetimingManager::operator=(RetimingManager other) {
	swap(*this, other);
	return *this;
}

RetimingManager::RetimingManager(const RetimingManager & other) :
		delays(other.delays) {
	main_graph = new Graph(*(other.main_graph));
	pc_graph = new DenseGraph(*(other.pc_graph));
	clock_period = other.clock_period;
	w = new SquareMatrix(*(other.w));
	d = new SquareMatrix(*(other.d));
	pc = new SquareMatrix(*(other.pc));
}

RetimingManager::~RetimingManager() {
	delete main_graph;
	delete pc_graph;
	delete w;
	delete d;
	delete pc;
}

void RetimingManager::print_main_graph() const {
	const weights_map weights = get(boost::edge_weight, *main_graph);
	boost::property_map<Graph, boost::vertex_index_t>::type vertex_id =
			boost::get(boost::vertex_index, *main_graph);

	// print vertices
	std::cout << "vertices = ";
	std::pair<vertex_iter, vertex_iter> vp;

	for (vp = vertices(*main_graph); vp.first != vp.second; ++vp.first) {
		int vertex_num = boost::get(vertex_id, *vp.first);
		std::cout << "(" << vertex_num << "," << delays[vertex_num] << ") ";
	}
	std::cout << std::endl;

	// print edges
	std::cout << "edges = ";
	boost::graph_traits<Graph>::edge_iterator ei, ei_end;
	for (boost::tie(ei, ei_end) = edges(*main_graph); ei != ei_end; ++ei)
		std::cout << "(" << boost::get(vertex_id, source(*ei, *main_graph))
				<< "," << boost::get(vertex_id, target(*ei, *main_graph)) << ","
				<< boost::get(weights, *ei) << ") ";
	std::cout << std::endl;
}

SquareMatrix RetimingManager::get_w_matrix() const {
	return *w;
}

SquareMatrix RetimingManager::get_d_matrix() const {
	return *d;
}

SquareMatrix RetimingManager::get_pc_matrix() const {
	return *pc;
}

/**
 * Tells if node v is in the path between src and end, where the structure
 * of the tree is described by the parent array
 * @param v the node that you want to check if it is in the path
 * @param src the source node
 * @param end the end node
 * @param parent the parent array, that contains the tree
 * @return
 */
bool isInPath(int v, int src, int end, int parent[]) {
	int i = end;
	if (v == src || v == end) {
		return true;
	}
	while (parent[i] != -1) {
		if (v == parent[i]) {
			return true;
		}
		i = parent[i];
	}
	return false;
}

void RetimingManager::dijkstra_max_delay(int src, int* dist, int* tot_delays) {
	const weights_map weights = get(boost::edge_weight, *main_graph);

	/*
	 * Value of parent[v] for a vertex v stores parent vertex of v in
	 * shortest path tree. Parent of root (or source vertex) is -1.
	 * Whenever we find shorter path through a vertex u, we make u as
	 * parent of current vertex.
	 */
	int parent[num_vertices(*main_graph)];

	// Create a priority queue to store vertices that are being preprocessed
	std::priority_queue<iPair, std::vector<iPair>, std::greater<iPair> > pq;
	const IndexMap index = boost::get(boost::vertex_index, *main_graph);

	// Insert source itself in priority queue and initialize its distance as 0.
	pq.push(std::make_pair(0, src));
	dist[src] = 0;
	parent[src] = -1;
	tot_delays[src] = delays[src];

	// Looping till priority queue becomes empty (or all distances are not finalized)
	while (!pq.empty()) {
		// The first vertex in pair is the minimum distance
		// vertex, extract it from priority queue.
		// vertex label is stored in second of pair.
		int u = pq.top().second;
		pq.pop();
		typename GraphTraits::adjacency_iterator ai;
		typename GraphTraits::adjacency_iterator ai_end;
		typename GraphTraits::out_edge_iterator out_i, out_end;
		typename GraphTraits::edge_descriptor e;
		Vertex targ;
		for (boost::tie(out_i, out_end) = boost::out_edges(u, *main_graph);
				out_i != out_end; ++out_i) {
			e = *out_i;
			targ = boost::target(e, *main_graph);
			int v = index[targ];
			int weight = boost::get(weights, e);
			if (dist[v] > dist[u] + weight
					|| (weight + dist[u] == dist[v]
							&& delays[v] + tot_delays[u] > tot_delays[v])) {
				if (!isInPath(v, src, u, parent)) {
					// Updating total delay and distance of v
					tot_delays[v] = delays[v] + tot_delays[u];
					dist[v] = dist[u] + weight;
					parent[v] = u;
					pq.push(std::make_pair(dist[v], v));
				}
			}
		}
	}
}

void RetimingManager::build_w_d_matrices() {
	for (unsigned int i = 0; i < w->get_side(); i++) {
		dijkstra_max_delay(i, w->get()[i], d->get()[i]);
	}
}

void RetimingManager::build_pc_matrix() {
	for (unsigned int i = 0; i < d->get_side(); i++) {
		for (unsigned int j = 0; j < d->get_side(); j++) {
			if (d->get(i, j) > clock_period) {
				pc->set(i, j, w->get(i, j) - 1);
			} else {
				pc->set(i, j, w->get(i, j));
			}
		}
	}
}

void RetimingManager::build_pc_graph() {
	const unsigned int num_vertices = boost::num_vertices(*main_graph) + 1;
	std::vector<Edge> edges;
	std::vector<int> weights;
	unsigned int i;
	for (i = 1; i < num_vertices; i++) {
		edges.push_back(Edge(SOURCE, i));
		weights.push_back(0);
	}
	for (i = 0; i < boost::num_vertices(*main_graph); i++) {
		for (unsigned int j = 0; j < boost::num_vertices(*main_graph); j++) {
			edges.push_back(Edge(j + 1, i + 1));
			weights.push_back(pc->get(i, j));
		}
	}
	pc_graph = new DenseGraph(&edges[0], &edges[edges.size()], &weights[0],
			num_vertices);
}

void RetimingManager::apply_final_step() {
	std::vector<int> pc_distances(boost::num_vertices(*pc_graph),
			(std::numeric_limits<short>::max)());
	pc_distances[SOURCE] = 0; // the source is at distance 0
	const bool r = boost::bellman_ford_shortest_paths(*pc_graph,
			boost::num_vertices(*pc_graph),
			boost::distance_map(&pc_distances[0]));
	if (!r) {
		std::cerr << "PC_GRAPH CONTAINS NEGATIVE CYCLES, CANNOT APPLY RETIMING"
				<< std::endl;
	} else {
		move_weights(pc_distances);
	}
}

void RetimingManager::move_weights(std::vector<int> &pc_distances) {
	boost::property_map<Graph, boost::vertex_index_t>::type vertex_id =
			boost::get(boost::vertex_index, *main_graph);
	boost::graph_traits<Graph>::edge_iterator ei, ei_end;
	for (boost::tie(ei, ei_end) = edges(*main_graph); ei != ei_end; ++ei) {
		int weight, rv, ru;

		// I do +1 because pc_graph adds an origin node to the main graph
		rv = pc_distances[boost::get(vertex_id, target(*ei, *main_graph)) + 1];
		ru = pc_distances[boost::get(vertex_id, source(*ei, *main_graph)) + 1];

		weight = get(boost::edge_weight_t(), *main_graph, *ei);
		boost::put(boost::edge_weight_t(), *main_graph, *ei, weight + rv - ru);
	}
}

std::vector<int> RetimingManager::get_scheduling() const {
	std::vector<bool> visited(num_vertices(*main_graph), false);
	const weights_map weights = get(boost::edge_weight, *main_graph);
	typename GraphTraits::out_edge_iterator out_i, out_end;
	typename GraphTraits::edge_descriptor e;
	Vertex targ;
	std::vector<int> scheduling(num_vertices(*main_graph));
	const IndexMap index = boost::get(boost::vertex_index, *main_graph);

	// To get the scheduling I do a bfs visit
	std::list<int> q;
	scheduling[SOURCE] = delays[0];
	visited[SOURCE] = true;
	q.push_back(SOURCE);
	while (!q.empty()) {
		int u = q.front();
		q.pop_front();
		for (boost::tie(out_i, out_end) = boost::out_edges(u, *main_graph);
				out_i != out_end; ++out_i) {
			e = *out_i;
			targ = boost::target(e, *main_graph);
			int v = index[targ];
			if (!visited[v]) {
				visited[v] = true;
				scheduling[v] = scheduling[u] + boost::get(weights, e);
				q.push_back(v);
			}
		}
	}
	return scheduling;
}

void RetimingManager::write_main_graph(std::string filename) const {
	const weights_map weights = get(boost::edge_weight, *main_graph);
	boost::property_map<Graph, boost::vertex_index_t>::type vertex_id =
			boost::get(boost::vertex_index, *main_graph);
	std::pair<vertex_iter, vertex_iter> vp;
	boost::graph_traits<Graph>::edge_iterator ei, ei_end;
	std::ofstream myfile;
	myfile.open(filename);

	// write vertices delays in first row
	for (vp = vertices(*main_graph); vp.first != vp.second; ++vp.first) {
		int vertex_num = boost::get(vertex_id, *vp.first);
		myfile << delays[vertex_num] << " ";
	}

	// write edge and weight in each row
	for (boost::tie(ei, ei_end) = edges(*main_graph); ei != ei_end; ++ei)
		myfile << std::endl << boost::get(vertex_id, source(*ei, *main_graph))
				<< " " << boost::get(vertex_id, target(*ei, *main_graph)) << " "
				<< boost::get(weights, *ei);

	myfile.close();
}

bool RetimingManager::is_main_graph_cyclic_util(int v, bool visited[],
		bool *recStack) {
	const IndexMap index = boost::get(boost::vertex_index, *main_graph);
	const weights_map weights = get(boost::edge_weight, *main_graph);

	if (visited[v] == false) {
		// Mark the current node as visited and part of recursion stack
		visited[v] = true;
		recStack[v] = true;

		typename GraphTraits::adjacency_iterator ai;
		typename GraphTraits::adjacency_iterator ai_end;
		typename GraphTraits::out_edge_iterator out_i, out_end;
		typename GraphTraits::edge_descriptor e;
		Vertex targ;
		for (boost::tie(out_i, out_end) = boost::out_edges(v, *main_graph);
				out_i != out_end; ++out_i) {
			e = *out_i;
			targ = boost::target(e, *main_graph);
			int t = index[targ];
			int weight = boost::get(weights, e);

			if (weight == 0) {
				if (!visited[t]
						&& is_main_graph_cyclic_util(t, visited, recStack))
					return true;
				else if (recStack[t])
					return true;
			}
		}
	}
	recStack[v] = false;  // remove the vertex from recursion stack
	return false;
}

/**
 * This function is a variation of DFS() of http://www.geeksforgeeks.org/archives/18212
 * @return true if the graph contains a cycle, else false.
 */
bool RetimingManager::is_main_graph_cyclic() {
	// Mark all the vertices as not visited and not part of recursion stack
	int num_vertices = boost::num_vertices(*main_graph);
	bool *visited = new bool[num_vertices];
	bool *recStack = new bool[num_vertices];
	for (int i = 0; i < num_vertices; i++) {
		visited[i] = false;
		recStack[i] = false;
	}

	// Call the recursive helper function to detect cycle in different DFS trees
	for (int i = 0; i < num_vertices; i++)
		if (is_main_graph_cyclic_util(i, visited, recStack))
			return true;

	return false;
}
