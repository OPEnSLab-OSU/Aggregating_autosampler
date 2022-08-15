#pragma once
#include <map>
#include <string>
#include <KPFoundation.hpp>

class Application;
namespace ShellSpace {
	using func = void (*)(Application & app, const std::string * args);
	struct func_args {
		func function;
		unsigned short n_args;
	};

};	// namespace ShellSpace

class Shell : public KPComponent {
public:
	std::map<std::string, ShellSpace::func_args> commands;

	Shell(const char * name, KPController * controller) : KPComponent(name, controller){};
	void setup() override;
	void runFunction(const std::string *, const unsigned short);
	void addFunction(const char *, const unsigned short, ShellSpace::func);
};