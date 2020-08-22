#include "LogProblem.hpp"
#include "AStarInterface.hpp"
#include "AStarSolver.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

bool FileExists(const std::string& filename)
{
	// Try to open the file, if we could extablish a connection, it exists.
	FILE* f;
	fopen_s(&f, filename.c_str(), "rb");
	if (f)
	{
		fclose(f);
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		std::cout << std::endl << "No file input." << std::endl;
		return 0;
	}

	for (int i = 1; i < argc; ++i)
	{
		LogProblem problem(argv[i]);
		AStarSolver solver;
		std::vector<std::unique_ptr<IAction>> solution;

		const auto start = std::chrono::high_resolution_clock::now();
		std::cout << std::endl << '*' << argv[i] << std::endl;
		int cost = solver.Solve(problem, solution);
		const auto end = std::chrono::high_resolution_clock::now();
		const auto timeElapsedNano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

		std::cout << std::endl << "===========SOLUTION===========" << std::endl << std::endl;
		LogProblem::OutputSolution(std::cout, solution);
		std::cout << std::endl << "-- cost: " << cost << std::endl;

		std::ofstream ofs("results.txt", std::ofstream::out | std::ofstream::app);
		std::cout << "in " << timeElapsedNano / 1000000.f << " ms" << std::endl;
	}

	return 0;
}