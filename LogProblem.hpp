#pragma once
#include "AStarInterface.hpp"
#include <vector>
#include <unordered_set>

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

	static const int driveCost = 17;
	static const int loadUnloadCost = 2;
	static const int flyCost = 1000;
	static const int pickUpCost = 14;
	static const int dropOffCost = 11;

	Action() = default;
	virtual ~Action() override {};

	Action(Type type, std::pair<int, int> valuePair);

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

	LogConfiguration* GetNewConfiguration(const Action& action,
		const LogSetting& setting) const;

	std::vector<Vehicle>& GetTrucksReference() { return trucks_; }
	std::vector<Vehicle>& GetAirplanesReference() { return airplanes_; }
	std::vector<Package>& GetPackagesReference() { return packages_; }

	const std::vector<Vehicle>& GetTrucksConstReference() const { return trucks_; }
	const std::vector<Vehicle>& GetAirplanesConstReference() const { return airplanes_; }
	const std::vector<Package>& GetPackagesConstReference() const { return packages_; }

	static int ComputeHeuristic(const std::vector<Vehicle>& trucks,
		const std::vector<Vehicle>& airplanes,
		const std::vector<Package>& packages,
		const LogSetting& setting);

	virtual IState* Clone() const override;

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
	virtual IState const* GetInitialState() const override;
	virtual bool IsGoalState(IState const* state) const override;
	virtual void EnumeratePossibleActions(IState const* state,
		std::queue<std::pair<IAction*, IState*>>& possibleActions) const override;
private:
	LogSetting setting_;
	std::unique_ptr<LogConfiguration> initialConfiguration_;
};
