/**
* @file ch2inc.cpp
* @author lakor64
* @date 03/01/2024
* @brief main ch2inc app
*/
#include "ch2inc.hpp"
#include "clangcli.hpp"

#include <iostream>
#include <filesystem>

CH2Inc::CH2Inc()
	: m_opt("ch2inc", "C include to ASM include generator")
	, m_parser()
	, m_file()
	, m_sopts()
	, m_fp(nullptr)
	, m_drvep(nullptr)
	, m_drvfnc(nullptr)
#ifndef DISABLE_DYNLIB
	, m_drv(nullptr)
#endif
{
	// add default options
	m_opt.add_options()
		("I,include", "Add new include", cxxopts::value<std::vector<std::string>>())
		("D,define", "Add new define", cxxopts::value<std::vector<std::string>>())
		("U,undefine", "Removes a define", cxxopts::value<std::vector<std::string>>())
		("h,help", "Show this help screen")
		("p,platform", "Platform to build", cxxopts::value<std::string>())
		("b,platform-bitsize", "Bits size of the platform", cxxopts::value<unsigned int>())
		("d,driver", "Driver to use", cxxopts::value<std::string>())
		("nologo", "Do not print the startup info")
		("msvc", "Run the tool in MSVC compatibility mode")
		("input", "The input file to process", cxxopts::value<std::string>())
		("output", "The output file to result", cxxopts::value<std::string>())
		("verbose", "Enable verbose logging")
		;

	m_opt.parse_positional({ "input", "output" });
}

CH2Inc::~CH2Inc()
{
	if (m_fp)
		fclose(m_fp);

	if (m_drvfnc)
		delete m_drvfnc;

#ifndef DISABLE_DYNLIB
	if (m_drv)
		dynlib_free(m_drv);
#endif
}

void CH2Inc::ShowHelp()
{
	// TODO: use a better cxxopts api for showing abi and abi size
	std::cout << m_opt.help() << std::endl;
	std::cout << "Platform:" << std::endl <<
		"  linux\t\t\tTargets the Linux platform" << std::endl <<
		"  win\t\t\tTargets the Windows platform" << std::endl <<
		"  darwin\t\t\tTargets the MacOS platform" << std::endl <<
		"  dos\t\t\tTargets the DOS platform" << std::endl <<
		"  os2\t\t\tTargets the OS/2 platform" << std::endl <<
		std::endl;
	std::cout << "Platform size:" << std::endl <<
		"  16\t\t\tTargets a 8086 architecture (does not work for MacOS or Linux)" << std::endl <<
		"  32\t\t\tTargets a x86 architecture" << std::endl <<
		"  64\t\t\tTargets a x86_64 architecture (does not work for DOS)" << std::endl;
}

int CH2Inc::ParseCli(int argc, char** argv)
{
	cxxopts::ParseResult res;
	
	try
	{
		res = m_opt.parse(argc, argv);
	}
	catch (std::exception&)
	{
		return false;
	}

	if (	res.count("h") 
		|| !res.count("input")
		|| !res.count("platform") 
		|| !res.count("platform-bitsize")
#ifndef DISABLE_DYNLIB
		|| !res.count("d")
#endif
	)
	{
		return -1;
	}

	if (res.count("verbose"))
		m_sopts.verbose = true;

	if (res.count("nologo"))
		m_sopts.nologo = true;

	if (res.count("msvc"))
		m_sopts.msvc = true;

	auto platformBits = res["platform-bitsize"].as<unsigned int>();
	auto platformName = res["platform"].as<std::string>();

	m_sopts.info.Set(platformName.c_str(), 
		std::to_string(platformBits).c_str(), 
		!m_sopts.msvc, 
		m_sopts.info.GetType() == PlatformType::Win && m_sopts.info.GetBits() == 32 ? CallType::Stdcall : CallType::Cdecl
	);

	if (!m_sopts.info.IsValid())
	{
		return -2;
	}

	m_sopts.input = res["input"].as<std::string>();

	if (res.count("output"))
		m_sopts.output = res["output"].as<std::string>();
	else
	{
		auto path = std::filesystem::path(m_sopts.input);
		path.replace_extension(".inc");
		m_sopts.output = path.string();
	}

	if (res.count("define"))
		m_sopts.defines = res["define"].as<std::vector<std::string>>();

	if (res.count("include"))
		m_sopts.includes = res["include"].as<std::vector<std::string>>();

	if (res.count("undefine"))
		m_sopts.undef = res["undefine"].as<std::vector<std::string>>();


#ifndef DISABLE_DYNLIB
	m_sopts.driver = res["d"].as<std::string>();
#else
	if (res.count("d"))
		std::cout << "Drivers are disabled in this build!" << std::endl;
#endif

	return 0;
}

void CH2Inc::AddDefaultData()
{
	std::string sysname = "none";
	std::string envname = "eabi";

	// general common define
	m_sopts.defines.push_back("__H2INC__");
	m_sopts.defines.push_back("__CH2INC__");

	// platform defines
	if (m_sopts.info.GetType() == PlatformType::Linux)
	{
		m_sopts.defines.push_back("__linux__");
		m_sopts.defines.push_back("linux__");
		m_sopts.defines.push_back("__linux");

		sysname = "linux";
		envname = "gnu";
	}
	else if (m_sopts.info.GetType() == PlatformType::Darwin)
	{
		if (m_sopts.info.GetBits() != 16)
		{
			m_sopts.defines.push_back("__APPLE__");
			m_sopts.defines.push_back("__MACH__");
		}
		else
		{
			m_sopts.defines.push_back("macintosh");
			m_sopts.defines.push_back("Macintosh");
		}

		sysname = "darwin";
		envname = "macho";
	}
	else if (m_sopts.info.GetType() == PlatformType::DOS)
	{
		m_sopts.defines.push_back("MSDOS");
		m_sopts.defines.push_back("_MSDOS");
		m_sopts.defines.push_back("__DOS__");
		m_sopts.defines.push_back("__MSDOS__");

		//sysname = "dos";
		//envname = "pe";
	}
	else if (m_sopts.info.GetType() == PlatformType::Win)
	{
		m_sopts.defines.push_back("__TOS_WIN__");
		m_sopts.defines.push_back("__WINDOWS__");

		if (m_sopts.info.GetBits() == 16)
		{
			m_sopts.defines.push_back("_WIN16");
		}
		else
		{
			m_sopts.defines.push_back("__WIN32__");
			m_sopts.defines.push_back("_WIN32");

			if (m_sopts.info.GetBits() == 64)
			{
				m_sopts.defines.push_back("_WIN64");
			}
		}

		sysname = "win32";

		if (m_sopts.msvc)
			envname = "pe"; // msvc
		else
			envname = "gnu"; // mingw
	}
	else if (m_sopts.info.GetType() == PlatformType::OS2)
	{
		m_sopts.defines.push_back("OS2");
		m_sopts.defines.push_back("_OS2");
		m_sopts.defines.push_back("__OS2__");
		m_sopts.defines.push_back("__TOS_OS2__");

		//sysname = "os2";
		envname = "pe";
	}

	std::string vendorname = "pc";
	std::string archname = "";

	switch (m_sopts.info.GetBits())
	{
	case 64:
		archname = "x86_64";
		m_sopts.extra.push_back("-m64");
		break;
	case 16:
		archname = "thumb";
		envname = "eabi";
		vendorname = "none";
		sysname = "none";
		m_sopts.extra.push_back("-m16");
		break;
	case 32:
		archname = "i386";
		m_sopts.extra.push_back("-m32");
		break;
	}

	// --target=i386-pc-win32-pe
	m_sopts.extra.push_back("--target=" + archname + "-" + vendorname + "-" + sysname + "-" + envname);

	if (m_sopts.msvc)
	{
		m_sopts.extra.push_back("-fms-compatibility");
		m_sopts.extra.push_back("-fms-extensions");
		m_sopts.extra.push_back("-mlong-double-64"); // make "long double" real8 and not real10
		 
		if (m_sopts.info.GetBits() == 32) // 16bit does not default to __stdcall and neither 64-bit does
		{
			m_sopts.extra.push_back("-mrtd"); // make __stdcall the default
			m_sopts.extra.push_back("-fpack-struct=4");
		}
	}
}

bool CH2Inc::SetupDriver()
{
#ifdef DISABLE_DYNLIB
	m_drvep = (DriverEntrypointFunc)DRIVER_ENTRYPOINT;
#else
	m_drv = dynlib_load(m_sopts.driver.c_str());
	if (!m_drv)
		return false;

	m_drvep = (DriverEntrypointFunc)dynlib_getfunc(m_drv, DRIVER_ENTRYPOINT_NAME);
	if (!m_drvep)
	{
		dynlib_free(m_drv);
		return false;
	}
#endif

	m_drvfnc = m_drvep();

	if (!m_drvfnc)
	{
#ifndef DISABLE_DYNLIB
		dynlib_free(m_drv);
#endif
		return false;
	}

	return true;

}

int CH2Inc::Run(int argc, char** argv)
{
	auto err = ParseCli(argc, argv);

	if (err == -1)
	{
		ShowHelp();
		return -1;
	}
	else if (err == -2)
	{
		std::cerr << "Invalid platform combo specified" << std::endl;
		return -2;
	}

	if (!m_sopts.nologo)
		std::cout << "ch2inc build: " << __TIMESTAMP__ << std::endl;

	if (!SetupDriver())
	{
		std::cerr << "Unable to setup driver" << std::endl;
		return -3;
	}

	if (m_sopts.verbose)
		std::cout << "Loaded driver: " << m_drvfnc->GetName() << " v." << m_drvfnc->GetVersion() << " (author: " << m_drvfnc->GetAuthor() << ")" << std::endl;

	AddDefaultData();
	m_drvfnc->AppendExtraDefines(m_sopts.defines);

	ClangCli clcli(m_sopts);

	if (m_sopts.verbose)
	{
		std::cout << "Passing to clang: ";
		for (int i = 0; i < clcli.argc; i++)
		{
			std::cout << clcli.argv[i] << " ";
		}
		std::cout << std::endl;
	}

	m_parser.Visit(m_sopts.input, clcli.argc, (const char**)clcli.argv, m_file, m_sopts.info);

	if (m_parser.GetLastError() != CH2ErrorCodes::None)
	{
		std::cerr << "Error during parsing: " << CH2ErrorCodeStr(m_parser.GetLastError()) << std::endl;
		return -4;
	}

	if (m_sopts.verbose)
		std::cout << "Parsing success! Start writing..." << std::endl;

#ifdef _WIN32
	fopen_s(&m_fp, m_sopts.output.c_str(), "wb");
#else
	m_fp = fopen(m_sopts.output.c_str(), "wb");
#endif

	if (!m_fp)
	{
		std::cerr << "Unable to open output file" << std::endl;
		return -5;
	}

	DriverConfig drvcfg;
	drvcfg.fp = m_fp;
	drvcfg.platform = m_sopts.info;
	drvcfg.verbose = m_sopts.verbose;
	m_drvfnc->SetConfig(drvcfg);

	std::vector<std::string> mc;
	mc.push_back("This file was generated by CH2Inc on " + std::string(__TIMESTAMP__) + "\n");
	mc.push_back("Plaese modify the file\"" + m_sopts.input + "\" insted.\n");
	m_drvfnc->WriteMultiComment(mc);

	m_drvfnc->WriteFileStart();

	for (const auto& type : m_file.GetTypes())
	{
		switch (type->GetTypeID())
		{
		case MemberType::Typedef:
			m_drvfnc->WriteTypeDef(*dynamic_cast<const Typedef*>(type));
			break;
		case MemberType::Union:
			m_drvfnc->WriteUnion(*dynamic_cast<const Union*>(type));
			break;
		case MemberType::Struct:
			m_drvfnc->WriteStruct(*dynamic_cast<const Struct*>(type));
			break;
		case MemberType::Enum:
			m_drvfnc->WriteEnum(*dynamic_cast<const Enum*>(type));
			break;
		case MemberType::Define:
			m_drvfnc->WriteDefine(*dynamic_cast<const Define*>(type));
			break;
		case MemberType::GlobalVar:
			m_drvfnc->WriteGlobalVar(*dynamic_cast<const GlobalVar*>(type));
			break;
		case MemberType::Function:
			m_drvfnc->WriteFunction(*dynamic_cast<const Function*>(type));
			break;
		default:
			break;
		}
	}

	m_drvfnc->WriteFileEnd();

	fclose(m_fp);
	m_fp = nullptr;

	if (m_sopts.verbose)
		std::cout << "Writing success!" << std::endl;

	return 0;
}
