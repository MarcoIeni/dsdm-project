#include <iostream>
#include <string>
#include <sys/stat.h>

#include "RetimingManager.h"

#define DEFAULT_INPUT_FILEPATH "input/example1.txt"
#define DEFAULT_CONTROL_STEPS 1
#define DEFAULT_CLOCK_PERIOD 4
#define DEFAULT_OUTPUT_DIR "output/"

/**
 * it creates a directory with the given name if it does not exist
 * @param dir_name the name of the directory
 */
void create_dir_if_do_not_exists(std::string dir_name) {
	const char* folder = dir_name.c_str();
	struct stat sb;

	//if directory does not exists
	if (!(stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode))) {
		// directory does not exists, so I create it
		const int dir_err = mkdir(folder,
		S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (dir_err == -1) {
			std::cerr << "Error creating directory" << std::endl;
		}
	}
}

/**
 * get filename from a path
 * @param path the path from which you want to extract the filename
 * @return the filename contained in the path
 */
std::string base_name(std::string const & path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

/**
 * get directory from a path
 * @param path the path from which you want to extract the directory
 * @return the name of the directory contained in the path
 */
std::string get_directory(std::string const & path) {
	return path.substr(0, path.find_last_of("/\\"));
}

int main(int argc, char *argv[]) {
	/*
	 * argv[1] = input_filename
	 * argv[2] = control_steps
	 * argv[3] = clock_period
	 * argv[4] = output_path (optional)
	 */
	std::string input_filepath;
	std::string output_filepath;
	int control_steps;
	int clock_period;

	if (argc == 4 || argc == 5) {
		input_filepath = argv[1];
		control_steps = std::stoi(argv[2]);
		clock_period = std::stoi(argv[3]);
	} else {
		input_filepath = DEFAULT_INPUT_FILEPATH;
		control_steps = DEFAULT_CONTROL_STEPS;
		clock_period = DEFAULT_CLOCK_PERIOD;
	}

	std::string input_filename = base_name(input_filepath);

	if (argc == 5) {
		output_filepath = argv[4];
	} else {
		output_filepath = DEFAULT_OUTPUT_DIR + input_filename;
	}

	std::string output_dir = get_directory(output_filepath);

	std::cout << "input filepath: " << input_filepath << std::endl;
	std::cout << "output filepath: " << output_filepath << std::endl;
	std::cout << "control steps: " << control_steps << std::endl;
	std::cout << "clock period: " << clock_period << std::endl;

	RetimingManager rm(input_filepath, control_steps, clock_period);
	try {
		rm.apply_retiming();
	} catch (const char* msg) {
		std::cerr << msg << std::endl;
		return -1;
	}
	rm.print_main_graph();
	std::cout << "w matrix:" << std::endl;
	rm.get_w_matrix().print_matrix();
	std::cout << "d matrix:" << std::endl;
	rm.get_d_matrix().print_matrix();
	std::cout << "pc matrix:" << std::endl;
	rm.get_pc_matrix().print_matrix();

	create_dir_if_do_not_exists(output_dir);

	rm.write_main_graph(output_filepath);
	std::cout << "scheduling: ";
	for (auto i : rm.get_scheduling()) {
		std::cout << i << " ";
	}
	std::cout << std::endl;
	return 0;
}
