#define PROFILER_IMPLEMENTATION
#define PROFILING_ENABLED

#ifndef SIMPLE_PROFILER_HH
#define SIMPLE_PROFILER_HH

#include <string_view>

#ifdef PROFILING_ENABLED

#if __has_cpp_attribute(nodiscard) > 201603
#	define Profiler_Nodiscard(Reason) [[nodiscard(Reason)]]
#else
#	define Profiler_Nodiscard(...) [[nodiscard]]
#endif

namespace profiler
{
	struct Section_Guard;

	void section_start(std::string_view name);
	void section_end();
	void print_summary();

	Profiler_Nodiscard("destruction of return value is treated as an end of a section")
	auto section_guard(std::string_view name) -> Section_Guard;

	struct Section_Guard { ~Section_Guard() { section_end(); } };

	void time_section(std::string_view name);
	void time_subsection(std::string_view name, unsigned level = 1);
}
#else
namespace profiler
{
	struct Section_Guard {};
	void section_start(std::string_view) {}
	void section_end() {}
	void print_summary() {}
	auto section_guard(std::string_view) -> Section_Guard { return {}; }
	void time_section(std::string_view) {}
	void time_subsection(std::string_view, unsigned = 1) {}
}
#endif

#endif // SIMPLE_PROFILER_HH

#if defined(PROFILING_ENABLED) && defined(PROFILER_IMPLEMENTATION)

#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>

namespace profiler
{
	using clock = std::chrono::high_resolution_clock;

	struct Entry
	{
		std::string_view name;
		double seconds_elapsed;
		unsigned short children_count;
		unsigned short indentation;
		double time_percantage;
	};

	struct Time
	{
		clock::time_point start;
		Entry *entry;
	};

#ifndef Profile_Max_Stack_Count
#define Profile_Max_Stack_Count 256
#endif

#ifndef Profile_Max_Summary_Count
#define Profile_Max_Summary_Count 1024
#endif

	static std::array<Time, Profile_Max_Stack_Count>    stack   = {}; static unsigned stack_ptr   = 0;
	static std::array<Entry, Profile_Max_Summary_Count> summary = {}; static unsigned summary_ptr = 0;

	auto section_guard(std::string_view name) -> Section_Guard
	{
		section_start(name);
		return {};
	}

	void section_start(std::string_view name)
	{
		auto &clock = stack[stack_ptr++] = { clock::now(), &summary[summary_ptr++] };
		clock.entry->name = name;
	}

	void section_end()
	{
		auto &time   = stack[--stack_ptr];
		time.entry->seconds_elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(clock::now() - time.start).count();

		if (stack_ptr > 0) {
			stack[stack_ptr - 1].entry->children_count += time.entry->children_count + 1;
		}
	}

	static void fill_metadata(unsigned short root, unsigned short tree_root, unsigned short level = 0)
	{
		summary[root].indentation = level * 2;
		summary[root].time_percantage = summary[root].seconds_elapsed / summary[tree_root].seconds_elapsed * 100;

		// iterate all children nodes
		auto count = summary[root].children_count;
		auto child = root + 1;
		while (count > 0) {
			fill_metadata(child, tree_root, level + 1);
			count -= summary[child].children_count + 1;
			child += summary[child].children_count + 1;
		}
	}

	void print_summary()
	{
		while (stack_ptr > 0)
			section_end();

		auto root = 0u;
		while (root < summary_ptr) {
			fill_metadata(root, root);
			root += summary[root].children_count + 1;
		}

		auto const max = std::max_element(
				std::cbegin(summary), std::cbegin(summary) + summary_ptr,
				[](auto const& lhs, auto const& rhs) { return lhs.name.size() + lhs.indentation < rhs.name.size() + rhs.indentation; });

		auto const max_first_column_width = max->name.size() + max->indentation;

		for (auto i = 0u; i < summary_ptr; ++i) {
			std::cout << std::setw(summary[i].indentation) << std::setfill(' ') << "";
			std::cout << summary[i].name;
			std::cout << std::setw(max_first_column_width - summary[i].name.size() - summary[i].indentation + 4) << std::setfill('.') << "";
			std::cout << std::fixed << std::setprecision(8)	<< summary[i].seconds_elapsed << "s ";
			std::cout << std::fixed << std::setprecision(4) << std::setw(8) << std::setfill(' ') << std::right << summary[i].time_percantage << '%';
			if (summary[i].children_count > 0) std::cout << " (" << summary[i].children_count << ')';
			std::cout << '\n';
		}
	}


	void time_section(std::string_view name)
	{
		time_subsection(name, 0);
	}

	void time_subsection(std::string_view name, unsigned level)
	{
		while (stack_ptr > level)
			section_end();
		section_start(name);
	}
}
#endif // PROFILER_IMPLEMENTATION

// Example of profiler usage
#if 0
#include <thread>
#include <chrono>

using namespace std::literals::chrono_literals;

void sleep1s()
{
	[[maybe_unused]] auto _ = profiler::section_guard("sleep1s function");
	std::this_thread::sleep_for(1s);
}

int main()
{
	profiler::time_section("Main");

	profiler::time_subsection("Sleep 1.5 seconds");
	sleep1s();

	profiler::section_start("Sleep 0.25s");
		std::this_thread::sleep_for(0.25s);
	profiler::section_end();

	profiler::time_subsection("Sleep 0.25s again", 2);
	std::this_thread::sleep_for(0.25s);

	profiler::time_subsection("Sleep 0.5s");
	std::this_thread::sleep_for(0.5s);

	profiler::print_summary();
}
#endif
