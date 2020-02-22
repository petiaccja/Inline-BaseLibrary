#include "System.hpp"

#include "../../AtScopeExit.hpp"
#include "../../Exception/Exception.hpp"

#include <Knownfolders.h>
#include <Shlobj.h>


namespace inl {


// Dll
ModuleHandle System::LoadModule(const char* path) {
	ModuleHandle h = LoadLibraryA(path);
	if (h == NULL) {
		throw FileNotFoundException("Library was not found.");
	}
	return h;
}
void System::UnloadModule(ModuleHandle handle) noexcept {
	FreeLibrary(handle);
}
void* System::GetModuleSymbolAddress(ModuleHandle handle, const char* symbolName) noexcept {
	return GetProcAddress(handle, symbolName);
}

// Cursor
Vec2i System::GetCursorPosition() {
	POINT p;
	GetCursorPos(&p);
	return { p.x, p.y };
}
void System::SetCursorPosition(const Vec2i& pos) {
	SetCursorPos(pos.x, pos.y);
}

void System::SetCursorVisual(eCursorVisual visual, WindowHandle windowHandle) {
	HCURSOR cursorHandle = nullptr;
	switch (visual) {
		case eCursorVisual::ARROW: cursorHandle = LoadCursor(nullptr, IDC_ARROW); break;
		case eCursorVisual::IBEAM: cursorHandle = LoadCursor(nullptr, IDC_IBEAM); break;
		case eCursorVisual::WAIT: cursorHandle = LoadCursor(nullptr, IDC_WAIT); break;
		case eCursorVisual::CROSS: cursorHandle = LoadCursor(nullptr, IDC_CROSS); break;
		case eCursorVisual::UPARROW: cursorHandle = LoadCursor(nullptr, IDC_UPARROW); break;
		case eCursorVisual::SIZE: cursorHandle = LoadCursor(nullptr, IDC_SIZE); break;
		case eCursorVisual::ICON: cursorHandle = LoadCursor(nullptr, IDC_ICON); break;
		case eCursorVisual::SIZENWSE: cursorHandle = LoadCursor(nullptr, IDC_SIZENWSE); break;
		case eCursorVisual::SIZENESW: cursorHandle = LoadCursor(nullptr, IDC_SIZENESW); break;
		case eCursorVisual::SIZEWE: cursorHandle = LoadCursor(nullptr, IDC_SIZEWE); break;
		case eCursorVisual::SIZENS: cursorHandle = LoadCursor(nullptr, IDC_SIZENS); break;
		case eCursorVisual::SIZEALL: cursorHandle = LoadCursor(nullptr, IDC_SIZEALL); break;
		case eCursorVisual::NO: cursorHandle = LoadCursor(nullptr, IDC_NO); break;
		case eCursorVisual::HAND: cursorHandle = LoadCursor(nullptr, IDC_HAND); break;
		case eCursorVisual::APPSTARTING: cursorHandle = LoadCursor(nullptr, IDC_APPSTARTING); break;
		case eCursorVisual::HELP: cursorHandle = LoadCursor(nullptr, IDC_HELP); break;
	}
	assert(cursorHandle);

	//if (windowHandle)
	//	SetClassLong(windowHandle, -12, reinterpret_cast<LONG>(cursorHandle)); // #define GCL_HCURSOR (-12)

	SetCursor(cursorHandle);
}

void System::SetCursorVisible(bool visible) {
	ShowCursor(visible);
}

// File paths
std::filesystem::path System::GetExecutableDir() {
	char name[1024];
	GetModuleFileNameA(GetModuleHandleA(nullptr), name, sizeof(name));
	std::string s(name);
	auto idx = s.find_last_of('\\');
	if (idx != s.npos) {
		return s.substr(0, idx);
	}
	else {
		return s;
	}
}

std::filesystem::path System::GetAppdataDir() {
	wchar_t** ppszPath = nullptr;
	AtScopeExit deallocPath{ [&] { CoTaskMemFree(ppszPath); } };
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, ppszPath);
	if (SUCCEEDED(hr)) {
		static_assert(sizeof(wchar_t) == sizeof(uint16_t));
		std::filesystem::path p{ *ppszPath };
	}
	throw RuntimeException("Windows won't return the path.");
}

std::filesystem::path System::GetTempDir() {
	char buf[MAX_PATH + 2];
	GetTempPathA(MAX_PATH + 1, buf);
	return std::filesystem::path{ buf };
}

std::filesystem::path System::GetHomeDir() {
	wchar_t** ppszPath = nullptr;
	AtScopeExit deallocPath{ [&] { CoTaskMemFree(ppszPath); } };
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, ppszPath);
	if (SUCCEEDED(hr)) {
		static_assert(sizeof(wchar_t) == sizeof(uint16_t));
		std::filesystem::path p{ *ppszPath };
	}
	throw RuntimeException("Windows won't return the path.");
}


} // namespace inl