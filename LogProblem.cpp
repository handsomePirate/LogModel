#include "LogProblem.hpp"
#include "OrientedGraph.hpp"
#include <fstream>
#include <string>
#include <map>
#include <set>

//#define OVERCAPACITY_LOG

#ifdef OVERCAPACITY_LOG
#include <iostream>
#endif

LogProblem::LogProblem(const std::string& file)
	: setting_(file), initialConfiguration_(
		std::make_unique<LogConfiguration>(file, setting_)) {}

void LogProblem::OutputSolution(std::ostream& out, const std::vector<std::unique_ptr<IAction>>& solution)
{
	for (const std::unique_ptr<IAction>& iAction : solution)
	{
		const std::unique_ptr<Action>& action = (const std::unique_ptr<Action>&)iAction;

		switch (action->type)
		{
		case Action::Type::DRIVE:
			out << "drive ";
			break;
		case Action::Type::LOAD:
			out << "load ";
			break;
		case Action::Type::UNLOAD:
			out << "unload ";
			break;
		case Action::Type::FLY:
			out << "fly ";
			break;
		case Action::Type::PICK_UP:
			out << "pickUp ";
			break;
		case Action::Type::DROP_OFF:
			out << "dropOff ";
			break;
		default:
			throw std::runtime_error("Undefined action value!");
			break;
		}

		out << action->valuePair.first << ' ' << action->valuePair.second << std::endl;
	}
}

IState const* LogProblem::GetInitialState() const
{
	return initialConfiguration_.get();
}

bool LogProblem::IsGoalState(IState const* state) const
{
	LogConfiguration const* configuration = (LogConfiguration const*) state;
	for (const auto& package : configuration->GetPackagesConstReference())
	{
		if (package.position != package.destination || package.state != Package::State::OUT)
			return false;
	}

	return true;
}

void LogProblem::EnumeratePossibleActions(IState const* state,
	std::queue<std::pair<IAction*, IState*>>& possibleActions) const
{
	LogConfiguration const* configuration = (LogConfiguration const*)state;
	const std::vector<Vehicle>& trucks = configuration->GetTrucksConstReference();
	const std::vector<Vehicle>& airplanes = configuration->GetAirplanesConstReference();
	const std::vector<Package>& packages = configuration->GetPackagesConstReference();

	// For each truck get all places in the same city and return a configuration of driving.
	for (int truck = 0; truck < trucks.size(); ++truck)
	{
		const Vehicle& truckObject = trucks[truck];
		auto placeVector = setting_.GetCityPlaces(setting_.GetPlaceCity(truckObject.position));
		for (int place : placeVector)
		{
			if (place != truckObject.position)
			{
				Action* action = new Action(Action::Type::DRIVE, { truck, place });
				LogConfiguration* state = configuration->GetNewConfiguration(*action, setting_);
				possibleActions.push({ action, state });
			}
		}
	}

	// For all packages that are in trucks or planes, generate an out state.
	for (int package = 0; package < packages.size(); ++package)
	{
		const Package& packageObject = packages[package];
		if (packageObject.state == Package::State::IN_PLANE)
		{
			Action* action = new Action(Action::Type::DROP_OFF, { packageObject.vehicle, package });
			LogConfiguration* state = configuration->GetNewConfiguration(*action, setting_);
			possibleActions.push({ action, state });
		}
		else if (packageObject.state == Package::State::IN_TRUCK)
		{
			Action* action = new Action(Action::Type::UNLOAD, { packageObject.vehicle, package });
			LogConfiguration* state = configuration->GetNewConfiguration(*action, setting_);
			possibleActions.push({ action, state });
		}
	}

	// For all packages that are in a place, where there is a plane or a truck, create a loaded state.
	for (int package = 0; package < packages.size(); ++package)
	{
		const Package& packageObject = packages[package];

		if (packageObject.state == Package::State::OUT)
		{
			for (int truck = 0; truck < trucks.size(); ++truck)
			{
				if (trucks[truck].position == packageObject.position && trucks[truck].load.size() < truckCapacity)
				{
					Action* action = new Action(Action::Type::LOAD, { truck, package });
					LogConfiguration* state = configuration->GetNewConfiguration(*action, setting_);
					possibleActions.push({ action, state });
				}
#ifdef OVERCAPACITY_LOG
				else if (trucks[truck].position == packageObject.position)
				{
					std::cout << "Over capacity!" << std::endl;
				}
#endif
			}

			for (int airplane = 0; airplane < airplanes.size(); ++airplane)
			{
				if (airplanes[airplane].position == packageObject.position && airplanes[airplane].load.size() < planeCapacity)
				{
					Action* action = new Action(Action::Type::PICK_UP, { airplane, package });
					LogConfiguration* state = configuration->GetNewConfiguration(*action, setting_);
					possibleActions.push({ action, state });
				}
			}
		}
	}

	const std::vector<int>& airports = setting_.GetAirports();

	// For all planes create a flight to every other city.
	for (int airplane = 0; airplane < airplanes.size(); ++airplane)
	{
		const Vehicle& airplaneObject = airplanes[airplane];

		for (int airport : airports)
		{
			if (airport != airplaneObject.position)
			{
				Action* action = new Action(Action::Type::FLY, { airplane, airport });
				LogConfiguration* state = configuration->GetNewConfiguration(*action, setting_);
				possibleActions.push({ action, state });
			}
		}
	}
}

int LogConfiguration::TruckRideCheck(int location, int destination, Package::State packageState)
{
	int cumulativeCost = 0;
	if (location != destination)
	{
		// If the package is in the correct city, we will need to drive it to the correct place.
		cumulativeCost += 17;
		// Additionally, if the package is not loaded yet, it will need to be loaded.
		if (packageState != Package::State::IN_TRUCK)
		{
			cumulativeCost += 2;
		}
		// After driving it, the package will need to be unloaded.
		cumulativeCost += 2;
	}
	return cumulativeCost;
}

LogConfiguration::LogConfiguration(const std::string& file, const LogSetting& setting)
{
	heuristic = LoadConfiguration(file, setting);
}

LogConfiguration::LogConfiguration(std::vector<Vehicle>& trucks, std::vector<Vehicle>& airplanes,
	std::vector<Package>& packages, int heuristic)
{
	std::swap(trucks_, trucks);
	std::swap(airplanes_, airplanes);
	std::swap(packages_, packages);
	IState::heuristic = heuristic;
}

LogConfiguration* LogConfiguration::GetNewConfiguration(const Action& action,
	const LogSetting& setting) const
{
	std::vector<Vehicle> trucks;
	std::vector<Vehicle> airplanes;
	std::vector<Package> packages;

	trucks = trucks_;
	airplanes = airplanes_;
	packages = packages_;

	switch (action.type)
	{
	case Action::Type::DRIVE:
		trucks[action.valuePair.first].position = action.valuePair.second;

		for (auto it = trucks[action.valuePair.first].load.begin();
			it != trucks[action.valuePair.first].load.end();
			++it)
		{
			packages[*it].position = action.valuePair.second;
		}

		break;
	case Action::Type::LOAD:
		trucks[action.valuePair.first].load.insert(action.valuePair.second);
		packages[action.valuePair.second].state = Package::State::IN_TRUCK;
		packages[action.valuePair.second].vehicle = action.valuePair.first;
		break;
	case Action::Type::UNLOAD:
		trucks[action.valuePair.first].load.erase(action.valuePair.second);
		packages[action.valuePair.second].state = Package::State::OUT;
		packages[action.valuePair.second].vehicle = -1;
		break;
	case Action::Type::FLY:
		airplanes[action.valuePair.first].position = action.valuePair.second;

		for (auto it = airplanes[action.valuePair.first].load.begin();
			it != airplanes[action.valuePair.first].load.end();
			++it)
		{
			packages[*it].position = action.valuePair.second;
		}

		break;
	case Action::Type::PICK_UP:
		airplanes[action.valuePair.first].load.insert(action.valuePair.second);
		packages[action.valuePair.second].state = Package::State::IN_PLANE;
		packages[action.valuePair.second].vehicle = action.valuePair.first;
		break;
	case Action::Type::DROP_OFF:
		airplanes[action.valuePair.first].load.erase(action.valuePair.second);
		packages[action.valuePair.second].state = Package::State::OUT;
		packages[action.valuePair.second].vehicle = -1;
		break;
	default:
		throw std::runtime_error("Undefined action value!");
		break;
	}

	int heuristic = ComputeHeuristic(trucks, airplanes, packages, setting);
	return new LogConfiguration(trucks, airplanes, packages, heuristic);
}

int LogConfiguration::ComputeHeuristic(const std::vector<Vehicle>& trucks,
	const std::vector<Vehicle>& airplanes,
	const std::vector<Package>& packages,
	const LogSetting& setting)
{
	int cumulativeCost = 0;

	//std::set<int> rideDestinations;
	//std::set<int> flightDestinations;

#pragma region countingPackageTransfers
	// Handle the loading and unloading of packages and cumulate destinations.
	for (int place = 0; place < setting.PlaceCount(); ++place)
	{
		for (const Package& package : packages)
		{
			if (package.position == place)
			{
				bool differentCity = setting.GetPlaceCity(package.position) != setting.GetPlaceCity(package.destination);
				if (!differentCity)
				{
					if (package.state == Package::State::IN_PLANE)
					{
						cumulativeCost += Action::dropOffCost;
					}
					if (package.position != package.destination)
					{
						if (package.state != Package::State::IN_TRUCK)
						{
							cumulativeCost += Action::loadUnloadCost;
						}
						cumulativeCost += Action::loadUnloadCost;
						//rideDestinations.insert(package.destination);
					}
					else
					{
						if (package.state == Package::State::IN_TRUCK)
						{
							cumulativeCost += Action::loadUnloadCost;
						}
					}
				}
				else
				{
					int currentAirport = setting.GetAirports()[setting.GetPlaceCity(package.position)];
					int destinationAirport = setting.GetAirports()[setting.GetPlaceCity(package.destination)];

					if (currentAirport != package.position)
					{
						if (package.state == Package::State::OUT)
						{
							cumulativeCost += Action::loadUnloadCost;
						}
						cumulativeCost += Action::loadUnloadCost;
						//rideDestinations.insert(currentAirport);
					}
					if (destinationAirport != package.destination)
					{
						cumulativeCost += 2 * Action::loadUnloadCost;
						//rideDestinations.insert(package.destination);
					}

					if (currentAirport == package.position)
					{
						if (package.state == Package::State::IN_TRUCK)
						{
							cumulativeCost += Action::loadUnloadCost + Action::pickUpCost;
						}
						if (package.state == Package::State::OUT)
						{
							cumulativeCost += Action::pickUpCost;
						}
					}
					else
					{
						cumulativeCost += Action::pickUpCost;
					}
					cumulativeCost += Action::dropOffCost;
					//flightDestinations.insert(destinationAirport);
				}
			}
		}
	}
#pragma endregion

#pragma region countingRides
	int rideLoops = 0;
	std::set<int> placesToVisitTrucks;
	for (int city = 0; city < setting.CityCount(); ++city)
	{
		// Create an oriented graph for necessary package rides.
		OrientedGraph rideGraph(setting.GetCityPlaces(city).size());

		for (auto&& package : packages)
		{
			if (setting.GetPlaceCity(package.position) == city)
			{
				if (setting.GetPlaceCity(package.position) == setting.GetPlaceCity(package.destination))
				{
					if (package.position != package.destination)
					{
						rideGraph.AddOrientedEdge(package.position, package.destination);
					}
				}
				else
				{
					int airport = setting.GetAirports()[setting.GetPlaceCity(package.position)];
					if (package.position != airport)
					{
						rideGraph.AddOrientedEdge(package.position, airport);
					}
				}
			}
			else if (setting.GetPlaceCity(package.destination) == city)
			{
				int airport = setting.GetAirports()[setting.GetPlaceCity(package.destination)];
				if (airport != package.destination)
				{
					rideGraph.AddOrientedEdge(airport, package.destination);
				}
			}
		}

		// Some trucks may already be in a place where there is at least one package.
		std::set<int> occupiedPlaces;
		for (auto&& truck : trucks)
		{
			for (auto&& package : packages)
			{
				if (package.position == truck.position && occupiedPlaces.find(truck.position) == occupiedPlaces.end())
				{
					occupiedPlaces.insert(truck.position);
				}
			}
		}

		// Count the loops that will cause a truck to return to alread visited places.
		rideLoops = rideGraph.GetLoopCount(occupiedPlaces);

		for (auto&& package : packages)
		{
			if (package.position != package.destination && 
				setting.GetPlaceCity(package.position) == setting.GetPlaceCity(package.destination))
			{
				if (occupiedPlaces.find(package.position) == occupiedPlaces.end())
				{
					placesToVisitTrucks.insert(package.position);
				}
				placesToVisitTrucks.insert(package.destination);
			}
			else if (setting.GetPlaceCity(package.position) != setting.GetPlaceCity(package.destination))
			{
				int posAirport = setting.GetAirports()[setting.GetPlaceCity(package.position)];
				int destAirport = setting.GetAirports()[setting.GetPlaceCity(package.destination)];
				if (setting.GetPlaceCity(package.position) == city &&
					package.position != posAirport)
				{
					if (occupiedPlaces.find(package.position) == occupiedPlaces.end())
					{
						placesToVisitTrucks.insert(package.position);
					}
					placesToVisitTrucks.insert(posAirport);
				}
				else if (setting.GetPlaceCity(package.position) == city &&
					destAirport != package.destination)
				{
					if (occupiedPlaces.find(destAirport) == occupiedPlaces.end())
					{
						placesToVisitTrucks.insert(destAirport);
					}
					placesToVisitTrucks.insert(package.destination);
				}
			}
		}
		/*
		int cityPackageScore = 0;
		for (auto&& package : packages)
		{
			int posCity = setting.GetPlaceCity(package.position);
			if (posCity == city)
			{
				--cityPackageScore;
			}
		}
		for (auto&& truck : trucks)
		{
			int truckCity = setting.GetPlaceCity(truck.position);
			if (truckCity == city)
			{
				cityPackageScore += 4;
			}
		}
		if (cityPackageScore >= 0)
		{
			--cumulativeCost;
		}*/
	}
#pragma endregion

#pragma region countingFlights
	// Similarly for flights.

	OrientedGraph flightGraph(setting.CityCount());

	for (auto&& package : packages)
	{
		int positionCity = setting.GetPlaceCity(package.position);
		int destinationCity = setting.GetPlaceCity(package.destination);
		if (positionCity != destinationCity)
		{
			flightGraph.AddOrientedEdge(positionCity, destinationCity);
		}
	}

	// Some planes may already be in a city where there is at least one package that needs to fly to a different city.
	std::set<int> occupiedCities;
	for (auto&& plane : airplanes)
	{
		for (auto&& package : packages)
		{
			int packagePositionCity = setting.GetPlaceCity(package.position);
			int planePositionCity = setting.GetPlaceCity(plane.position);
			if (packagePositionCity == planePositionCity && occupiedCities.find(planePositionCity) == occupiedCities.end())
			{
				occupiedCities.insert(planePositionCity);
			}
		}
	}

	// Count the loops that will cause a plane to return to alread visited places.
	int flightLoops = flightGraph.GetLoopCount(occupiedCities);

	std::set<int> placesToVisitPlanes;
	for (auto&& package : packages)
	{
		int positionCity = setting.GetPlaceCity(package.position);
		int destinationCity = setting.GetPlaceCity(package.destination);
		if (positionCity != destinationCity)
		{
			if (occupiedCities.find(positionCity) == occupiedCities.end())
			{
				placesToVisitPlanes.insert(positionCity);
			}
			placesToVisitPlanes.insert(destinationCity);
		}
	}
#pragma endregion

	int rideCount = placesToVisitTrucks.size() + rideLoops;
	int flightCount = placesToVisitPlanes.size() + flightLoops;
	
	cumulativeCost += rideCount * Action::driveCost;
	cumulativeCost += flightCount * Action::flyCost;

	return cumulativeCost;
}

IState* LogConfiguration::Clone() const
{
	auto trucks = trucks_;
	auto airplanes = airplanes_;
	auto packages = packages_;
	LogConfiguration* result = new LogConfiguration(trucks, airplanes, packages, heuristic);
	return result;
}

int LogConfiguration::LoadConfiguration(const std::string& file, const LogSetting& setting)
{
	std::ifstream ifs(file);
	std::string line;

	int cityCount = setting.CityCount();
	int placeCount = setting.PlaceCount();

	for (int i = 0; i < cityCount + placeCount + 2; ++i)
	{
		std::getline(ifs, line);
		while (line[0] == '%')
			std::getline(ifs, line);
	}

	std::getline(ifs, line);
	while (line[0] == '%')
		std::getline(ifs, line);
	int truckCount = stoi(line);
	trucks_.resize(truckCount);

	for (int truck = 0; truck < truckCount; ++truck)
	{
		std::getline(ifs, line);
		while (line[0] == '%')
			std::getline(ifs, line);
		trucks_[truck].position = stoi(line);
	}

	std::getline(ifs, line);
	while (line[0] == '%')
		std::getline(ifs, line);
	int airplaneCount = stoi(line);
	airplanes_.resize(airplaneCount);

	for (int airplane = 0; airplane < airplaneCount; ++airplane)
	{
		std::getline(ifs, line);
		while (line[0] == '%')
			std::getline(ifs, line);
		airplanes_[airplane].position = stoi(line);
	}

	std::getline(ifs, line);
	while (line[0] == '%')
		std::getline(ifs, line);
	int packageCount = stoi(line);
	packages_.resize(packageCount);

	for (int package = 0; package < packageCount; ++package)
	{
		std::getline(ifs, line);
		while (line[0] == '%')
			std::getline(ifs, line);

		size_t spacePosition = line.find(' ');

		std::string locationString = line.substr(0, spacePosition);
		std::string destinationString = line.substr(spacePosition + 1, line.length() - spacePosition);

		packages_[package].position = stoi(locationString);
		packages_[package].destination = stoi(destinationString);

		packages_[package].state = Package::State::OUT;
		packages_[package].vehicle = -1;
	}

	ifs.close();

	return ComputeHeuristic(trucks_, airplanes_, packages_, setting);
}

LogSetting::LogSetting(const std::string& file)
{
	std::ifstream ifs(file);
	std::string line;

	if (ifs.bad())
		throw "Unable to open the input file.";

	std::getline(ifs, line);
	while (line[0] == '%')
		std::getline(ifs, line);
	cityCount_ = stoi(line);

	std::getline(ifs, line);
	while (line[0] == '%')
		std::getline(ifs, line);
	int placeCount = stoi(line);

	places_.resize(placeCount);

	for (int place = 0; place < placeCount; ++place)
	{
		std::getline(ifs, line);
		while (line[0] == '%')
			std::getline(ifs, line);
		places_[place] = stoi(line);
	}

	airports_.resize(cityCount_);

	for (int airport = 0; airport < cityCount_; ++airport)
	{
		std::getline(ifs, line);
		while (line[0] == '%')
			std::getline(ifs, line);
		airports_[airport] = stoi(line);
	}
	ifs.close();
}

std::vector<int> LogSetting::GetCityPlaces(int city) const
{
	std::vector<int> places;
	for (int place = 0; place < places_.size(); ++place)
	{
		if (places_[place] == city)
			places.push_back(place);
	}
	return places;
}

int LogSetting::GetPlaceCity(int place) const
{
	return places_[place];
}

Action::Action(Type type, std::pair<int, int> valuePair)
	: type(type), valuePair(valuePair)
{
	switch (type)
	{
	case Action::Type::DRIVE:
		cost = driveCost;
		break;
	case Action::Type::LOAD:
	case Action::Type::UNLOAD:
		cost = loadUnloadCost;
		break;
	case Action::Type::FLY:
		cost = flyCost;
		break;
	case Action::Type::PICK_UP:
		cost = pickUpCost;
		break;
	case Action::Type::DROP_OFF:
		cost = dropOffCost;
		break;
	default:
		throw std::runtime_error("Undefined action value!");
		break;
	}
}

IAction* Action::Clone() const
{
	Action* result = new Action;

	result->cost = cost;
	((Action*)result)->type = type;
	((Action*)result)->valuePair = valuePair;

	return result;
}
