# Copilot instructions for StudioTVPlayer

Purpose
- Provide targeted guidance for Copilot sessions: build/test/lint commands, high-level architecture, and repo-specific conventions.

Build / Test / Lint
- Primary IDE: Visual Studio 2019+ (v142 toolset). Open StudioTVPlayer.sln.
- Build whole solution (Release, x64):
  msbuild "C:\Projects\StudioTVPlayer\StudioTVPlayer.sln" /p:Configuration=Release /p:Platform=x64
- Build single project (example: Timecode service):
  msbuild "C:\Projects\StudioTVPlayer\TimecodeDecoderService\TimecodeDecoderService.csproj" /p:Configuration=Debug
- Native C++ projects (TVPlayRLib, TVPlayR.Player): build with Platform=x64.
  msbuild "C:\Projects\StudioTVPlayer\TVPlayRLib\TVPlayR.vcxproj" /p:Configuration=Release /p:Platform=x64
- Tests: no managed test runner detected. There is a native C++ test project (TVPlayRLib.Test). Run its test binary from the project's output folder. To run a single test, execute the test binary and pass any supported test runner args (or run via Visual Studio Test Explorer if configured).
- Linting: no repository-wide linter or editorconfig found. Use Visual Studio analyzers; csproj sets AnalysisLevel.

High-level architecture
- Mixed-language solution combining:
  - TVPlayRLib (C++ static library): core media code (FFmpeg, DeckLink, NDI integration), exposes low-level player, encoder, and output implementations.
  - TVPlayR.Player (C++/CLR or managed C++ project): bridges native TVPlayRLib to managed code (CLR), compiles as a DLL referenced by C# projects.
  - StudioTVPlayer (C#, WPF, .NET Framework 4.8): WPF UI using MVVM (View/, ViewModel/, Model/) that controls players, rundowns, and outputs.
  - TimecodeDecoderService (C# Windows Service): runs as a service and references TVPlayR.Player for decoding/timecode tasks.
  - Helpers/ (small test utilities and sample apps) and dependencies/ (FFmpeg, NDI, LibAtem, Decklink SDK headers/binaries).
- Data flow: UI (StudioTVPlayer) calls managed wrapper (TVPlayR.Player) → native core (TVPlayRLib) → FFmpeg/Decklink/NDI for playback/encoding/output.
- Platform assumptions: x64 targets for native libs; .NET assemblies target net48 or AnyCPU with Prefer32Bit=false. Ensure dependencies (FFmpeg DLLs, NDI runtimes, DeckLink drivers) are present on PATH or copied to output (csproj copies FFMpeg bin files).

Key conventions / repository specifics
- Mixed-language build order: build TVPlayRLib (native) → TVPlayR.Player (CLR) → StudioTVPlayer (WPF). Solution references are already configured.
- Prebuilt native dependencies live under dependencies\ (FFMpeg, Ndi, LibAtem). Projects expect these folders; do not move without updating project files.
- Project platform conventions:
  - Use x64 for native C++ projects (VCXPROJ configurations are Debug|x64 and Release|x64)
  - C# WPF app sets <PlatformTarget>x64 in StudioTVPlayer.csproj — match this when building from CLI.
- Precompiled headers: native projects use pch.h/pch.cpp. Maintain pch changes carefully.
- Copying native DLLs: TimecodeDecoderService has a PostBuildEvent that xcopy FFMpeg DLLs into its output; other projects include FFMpeg DLL copy entries in csproj.
- Native includes/libs: AdditionalIncludeDirectories and AdditionalLibraryDirectories reference $(SolutionDir)dependencies\FFMpeg and dependencies\Ndi.

Existing docs checked
- README.md: included prerequisites (Windows + .NET Framework 4.8, NDI runtime, Decklink drivers). Key prereqs reproduced above.
- No CONTRIBUTING.md, CLAUDE.md, AGENTS.md, or other AI-assistant configs were found. Add any project-specific assistant rules if desired.

If you want changes
- Ask to expand sections (e.g., add exact test runner invocation, CI steps, or developer workflow examples).

