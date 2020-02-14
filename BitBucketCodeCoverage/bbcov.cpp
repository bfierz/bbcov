#include "Plugin/Exporter/IExportPlugin.hpp"
#include "Plugin/Exporter/CoverageData.hpp"
#include "Plugin/Exporter/ModuleCoverage.hpp"
#include "Plugin/Exporter/FileCoverage.hpp"
#include "Plugin/Exporter/LineCoverage.hpp"
#include "Plugin/OptionsParserException.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

class BitBucketCodeCoverage : public Plugin::IExportPlugin
{
public:
	std::optional<std::filesystem::path> Export(
		const Plugin::CoverageData& coverageData,
		const std::optional<std::wstring>& argument) override
	{
		std::filesystem::path output = L"BitBucketCodeCoverage.json";
		std::filesystem::path prefix = L"";
		if (argument)
		{
			const auto separator = argument->find_first_of(L'?');
			if (separator != std::wstring::npos)
				output = argument->substr(separator + 1);
			prefix = argument->substr(0, separator);
		}

		std::wofstream ofs{ output };

		if (!ofs)
			throw std::runtime_error("Cannot create the output file for BitBucketCodeCoverage");

		// Open the json structure
		ofs << LR"({)" << "\n";
		ofs << LR"(  "files": [)" << "\n";

		bool request_module_separator = false;
		for (const auto& mod : coverageData.GetModules())
		{
			const auto& files = mod->GetFiles();

			// Only write out a separator if there is more content.
			// This filters empty modules
			if (!files.empty())
			{
				if (request_module_separator)
					ofs << ",\n";
				request_module_separator = true;
			}
			for (auto file = std::begin(files); file != std::end(files); ++file)
			{
				ofs << LR"(  {)" << "\n";
				auto lines = (*file)->GetLines();

				// Separate covered from uncovered lines
				const auto separator = std::partition(
					std::begin(lines),
					std::end(lines),
					[](const auto& line) { return line.HasBeenExecuted(); }
				);

				// Strip the prefix in order to prepare the files paths for BitBucket.
				// File paths need to be relative to the root.
				const auto path = std::filesystem::relative((*file)->GetPath(), prefix);
				ofs << LR"(    "path": ")" << path.generic_wstring() << "\",\n";
				ofs << LR"(    "coverage": ")";

				// Print fully covered lines
				if (std::begin(lines) != separator)
				{
					ofs << LR"(C:)";
					for (auto line = std::begin(lines); line != separator; ++line)
					{
						ofs << line->GetLineNumber();
						if (line + 1 != separator)
							ofs << ",";
					}
				}

				// Unclear how to determine partially covered lines
				//ofs << LR"(P:)";

				// Print fully uncovered lines
				if (separator != std::end(lines))
				{
					if (std::begin(lines) != separator)
						ofs << ";";

					ofs << LR"(U:)";
					for (auto line = separator; line != std::end(lines); ++line)
					{
						ofs << line->GetLineNumber();
						if (line + 1 != std::end(lines))
							ofs << ",";
					}
				}
				ofs << "\"\n";
				ofs << LR"(  })";

				if (file != std::prev(std::end(files)))
					ofs << ",\n";
			}
		}

		// Close the json structure
		ofs << "\n";
		ofs << LR"(  ])" << "\n";
		ofs << LR"(})" << std::endl;

		return output;
	}

	void CheckArgument(const std::optional<std::wstring>& argument) override
	{
		// The parameter for this plugin contains an optional and non-optional part.
		// The first part is mandatory, in order to create relative paths for BitBucket Server.
		// The optional part contains the output file name.
		if (argument)
		{
			if (const auto separator = argument->find_first_of(L'?'); separator != std::wstring::npos)
			{
				if (!std::filesystem::path{ argument->substr(separator + 1) }.has_filename())
					throw Plugin::OptionsParserException("Invalid argument for BitBucketCodeCoverage export.");
			}
		}
	}

	std::wstring GetArgumentHelpDescription() override
	{
		return L"<prefix>?<output file (optional)>";
	}

	int GetExportPluginVersion() const override
	{
		return Plugin::CurrentExportPluginVersion;
	}
};

extern "C"
{
	__declspec(dllexport) Plugin::IExportPlugin* CreatePlugin()
	{
		return new BitBucketCodeCoverage();
	}
}
