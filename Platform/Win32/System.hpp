#pragma once


#include "Input.hpp"

#include <InlineMath.hpp>
#include <filesystem>
#include <vector>


namespace inl {


using ModuleHandle = HMODULE;
using WindowHandle = HWND;

enum class eCursorVisual {
	ARROW,
	IBEAM,
	WAIT,
	CROSS,
	UPARROW,
	SIZE,
	ICON,
	SIZENWSE,
	SIZENESW,
	SIZEWE,
	SIZENS,
	SIZEALL,
	NO,
	HAND,
	APPSTARTING,
	HELP,
};


class System {
public:
	// Dll
	static ModuleHandle LoadModule(const char* path);
	static void UnloadModule(ModuleHandle handle) noexcept;
	static void* GetModuleSymbolAddress(ModuleHandle handle, const char* symbolName) noexcept;

	// Cursor
	static Vec2i GetCursorPosition();
	static void SetCursorPosition(const Vec2i& pos);
	static void SetCursorVisual(eCursorVisual visual, WindowHandle windowHandle);
	static void SetCursorVisible(bool visible);

	// File paths
	static std::filesystem::path GetExecutableDir();
	static std::filesystem::path GetAppdataDir();
	static std::filesystem::path GetTempDir();
	static std::filesystem::path GetHomeDir();
};



} // namespace inl