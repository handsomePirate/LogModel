#pragma once
#include "AStarInterface.hpp"
#include <vector>

// This is the description of non-changeable facts about the problem, e.g. the cities and places.
class LogSetting
{
public:
	LogSetting(const std::string& file);

	int CityCount() const { return (int)cityCount_; }
	int PlaceCount() const { return (int)places_.size(); }

	std::vector<int> GetCityPlaces(int city) const;

	int GetPlaceCity(int place) const;

	const std::vector<int>& GetAirports() const { return airports_; };
private:
	int cityCount_;
	std::vector<int> places_;
	std::vector<int> airports_;
};

class LogConfiguration;

class Action : public IAction
{
public:
	enum class Type
	{
		DRIVE,
		LOAD,
		UNLOAD,
		FLY,
		PICK_UP,
		DROP_OFF,
		ACTION_TYPE_COUNT
	} type;
	std::pair<int, int> valuePair;

	Action() = default;
	virtual ~Action() override {};

	Action(Type type, std::pair<int, int> valuePair, 
		std::shared_ptr<LogConfiguration> originalConfiguration,
		const LogSetting& setting);

	virtual IAction* Clone() const override;
};

struct Vehicle
{
	int position;
	std::unordered_set<int> load;
};

struct Package
{
	int position;
	int destination;
	enum class State
	{
		IN_TRUCK,
		IN_PLANE,
		OUT,
		STATE_COUNT
	} state;
	int vehicle;
};

// This is the description of the current configuration, i.e. state of the changeable parts of the problem.
class LogConfiguration : public IState
{
public:
	LogConfiguration(const std::string& file, const LogSetting& setting);

	LogConfiguration(std::vector<Vehicle>& trucks, std::vector<Vehicle>& airplanes,
		std::vector<Package>& packages, int heuristic);

	std::shared_ptr<LogConfiguration> GetNewConfiguration(const Action& action,
		const LogSetting& setting) const;

	std::vector<Vehicle>& GetTrucksReference() { return trucks_; }
	std::vector<Vehicle>& GetAirplanesReference() { return airplanes_; }
	std::vector<Package>& GetPackagesReference() { return packages_; }

	const std::vector<Vehicle>& GetTrucksConstReference() { return trucks_; }
	const std::vector<Vehicle>& GetAirplanesConstReference() { return airplanes_; }
	const std::vector<Package>& GetPackagesConstReference() { return packages_; }

	static int ComputeHeuristic(const std::vector<Vehicle>& trucks,
		const std::vector<Vehicle>& airplanes,
		const std::vector<Package>& packages,
		const LogSetting& setting);

private:
	std::vector<Vehicle> trucks_;
	std::vector<Vehicle> airplanes_;
	std::vector<Package> packages_;

	int LoadConfiguration(const std::string& file, const LogSetting& setting);
	static int TruckRideCheck(int location, int destination, Package::State packageState);
};

// This is the problem assignment.
class LogProblem : public IProblem
{
public:
	static const int truckCapacity = 4;
	static const int planeCapacity = 30;

	LogProblem(const std::string& file);
	static void OutputSolution(std::ostream& out, const std::vector<std::unique_ptr<IAction>>& solution);
	virtual std::shared_ptr<IState> GetInitialState() const override;
	virtual bool IsGoalState(const std::shared_ptr<IState>& state) const override;
	virtual void EnumeratePossibleActions(const std::shared_ptr<IState>& state,
		std::unordered_set<std::unique_ptr<IAction>, IActionHash>& possibleActions) const override;
private:
	LogSetting setting_;
	std::shared_ptr<LogConfiguration> initialConfiguration_;
};
