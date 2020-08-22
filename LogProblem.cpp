#include "LogProblem.hpp"
#include "OrientedGraph.hpp"
#include <fstream>
#include <string>
#include <map>
#include <set>

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
	//std::map<int, std::set<int>> rides;
	//std::map<int, std::set<int>> flights;

	std::set<int> rideDestinations;
	std::set<int> flightDestinations;

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
						cumulativeCost += 11;
					}
					if (package.position != package.destination)
					{
						if (package.state != Package::State::IN_TRUCK)
						{
							cumulativeCost += 2;
						}
						cumulativeCost += 2;
						//rides[place].insert(package.destination);
						rideDestinations.insert(package.destination);
					}
					else
					{
						if (package.state == Package::State::IN_TRUCK)
						{
							cumulativeCost += 2;
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
							cumulativeCost += 2;
						}
						cumulativeCost += 2;
						//rides[place].insert(currentAirport);
						rideDestinations.insert(currentAirport);
					}
					if (destinationAirport != package.destination)
					{
						cumulativeCost += 2 + 2;
						//rides[destinationAirport].insert(package.destination);
						rideDestinations.insert(package.destination);
					}

					if (currentAirport == package.position)
					{
						if (package.state == Package::State::IN_TRUCK)
						{
							cumulativeCost += 2 + 14;
						}
						if (package.state == Package::State::OUT)
						{
							cumulativeCost += 14;
						}
					}
					else
					{
						cumulativeCost += 14;
					}
					cumulativeCost += 11;
					//flights[currentAirport].insert(destinationAirport);
					flightDestinations.insert(destinationAirport);
				}
			}
		}
	}

	int rideCount = 0;
	int flightCount = 0;
	/*
	OrientedGraph rideGraph(setting.PlaceCount());
	OrientedGraph flightGraph(setting.CityCount());

	for (auto&& package : packages)
	{
		if (package.position != package.destination)
		{
			rideGraph.AddOrientedEdge(package.position, package.destination);
		}
		if (setting.GetPlaceCity(package.position) != setting.GetPlaceCity(package.destination))
		{
			flightGraph.AddOrientedEdge(
				setting.GetPlaceCity(package.position), setting.GetPlaceCity(package.destination));
		}
	}

	auto rideLooseEnds = rideGraph.EnumerateLooseEnds();
	auto flightLooseEnds = flightGraph.EnumerateLooseEnds();

	bool truckPresent = false;
	for (int looseEnd : rideLooseEnds)
	{
		for (auto&& truck : trucks)
		{
			if (truck.position == looseEnd)
			{
				truckPresent = true;
				break;
			}
		}
		if (truckPresent)
		{
			break;
		}
	}
	if (!truckPresent && !rideLooseEnds.empty())
	{
		++rideCount;
	}

	bool planePresent = false;
	for (int looseEnd : flightLooseEnds)
	{
		for (auto&& truck : trucks)
		{
			if (truck.position == looseEnd)
			{
				planePresent = true;
				break;
			}
		}
		if (planePresent)
		{
			break;
		}
	}
	if (!planePresent && !flightLooseEnds.empty())
	{
		++flightCount;
	}*/

	rideCount += rideDestinations.size();
	flightCount += flightDestinations.size();
	
	cumulativeCost += rideCount * 17;
	cumulativeCost += flightCount * 1000;

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
		cost = 17;
		break;
	case Action::Type::LOAD:
	case Action::Type::UNLOAD:
		cost = 2;
		break;
	case Action::Type::FLY:
		cost = 1000;
		break;
	case Action::Type::PICK_UP:
		cost = 14;
		break;
	case Action::Type::DROP_OFF:
		cost = 11;
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
