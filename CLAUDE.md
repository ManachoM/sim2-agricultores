# CLAUDE.md - Agricultural Simulation System Guide

## Build Commands
- `make libs` - Compile required libraries (bsponmpi, libpqxx, libpq)
- `make` - Compile the simulation
- `make clean` - Remove object files and binary
- `make init_db` - Initialize PostgreSQL database for simulation

## Run Commands
- `mpirun -np 4 ./bin/agro-sim -c [config_file.json]` - Run simulation with MPI
- `make run_scenarios` - Run multiple simulations for all config files in sim_config_files

## Project Structure
- `src/` - C++ source files
- `includes/` - Header files
- `inputs/` - Input data (JSON files)
- `sim_config_files/` - Simulation configurations
- `metaheuristics/` - Optimization algorithms for simulation partitioning
- `data_analysis/` - Results and analysis scripts
- `libs` and `libs_build` contain the source code and compilation result of the dependencies.

## Class responsibilities
- For the parallel version of the simulation, which is the one present on this branch, the entrypoint for the simulation is the `ParallelSimulation` class.
- Here, the `run()` method starts the simulation, and the `route_event(Event *e)` ensures that the events are correctly routed or processed.
- Another very important class is the `Message` and `MesaggeSerializer` classes. These allow agents to communicate with one another, even among processors.
- Another extremely important class is the `PostgresMonitor` class. Its responsibility is to correctly record, aggregate and collect events and metrics to be registered in the simlaator's DB. 

## Code Style
- C++17 standard with modern features
- Class-based, agent-oriented design pattern
- 2-space indentation
- CamelCase for class names, snake_case for methods/variables
- Doxygen-style comments for class documentation
- Error handling with try/catch blocks and error codes in message objects
- See .clang-format file for more style guides.

## Dependencies
- BSP on MPI (bsponmpi) for parallel processing. It's especially important to understand MPI mechanisms and the <bsp.h> file. The other dependencies are not worth looking into unless required for a prompt.
- PostgreSQL and libpqxx for data persistence
- nlohmann/json for JSON handling
- Discrete event simulation framework with FEL (Future Event List)
- Message passing interface for agent communication

## Configuration
Modify `sim_config.json` to change simulation parameters:
- Input files (products, markets, land plots)
- Agent types (farmers, traders, consumers)
- Database connection string


