# Repository Guidelines

## Project Structure & Module Organization

Plane Sailing is a small Qt C++ desktop application that models aircraft tracks and exports KML. Source and header files live in the repository root. `main.cpp` starts the UI; `mainwnd.*` contains the main window and settings workflow; `thread.*` runs the simulation; `track3d.*`, `point*`, and `pointset.*` provide geometry; and `kmlfile.*` handles output. Distribution and unit helpers are in `distribution*`, `units.*`, and `util.*`. The qmake project definition is `plane-sailing.pro`. There are currently no separate asset or test directories.

## Build, Test, and Development Commands

Use a Qt 6 development environment with qmake, a C++17-capable compiler, and pthreads.

```sh
mkdir -p build && cd build
qmake6 ../plane-sailing.pro
make -j$(nproc)
./plane-sailing
```

Building outside the source tree keeps generated Makefiles and object files out of the repository. Run `make clean` in `build/` to remove compiled intermediates. On Windows, use the equivalent Qt Creator or MinGW qmake build configuration.

## Coding Style & Naming Conventions

Follow the existing C++ style: four-space indentation, opening braces on their own line, and one class per matching `.h`/`.cpp` pair. Use `PascalCase` for types (`PointSet`), `camelCase` for methods and local variables (`addPoint`), and a trailing underscore for data members (`points_`). Header guards use uppercase names such as `POINTSET_H`. Keep Qt signal/slot declarations in their conventional sections and prefer standard-library containers and RAII. No formatter or linter is configured, so match nearby code closely and keep diffs focused.

## Testing Guidelines

No automated test framework or coverage target is present. Before submitting, perform a clean build, launch the application, exercise the changed UI or simulation path, and verify generated KML when output logic changes. If adding tests, place them under `tests/`, name files after the component (for example, `test_pointset.cpp`), and document the command needed to run them.

## Commit & Pull Request Guidelines

Recent commits use short, sentence-case summaries such as `Windows fixes` and `Removed unused values from random box`. Keep the subject concise, describe one logical change, and omit a trailing period. Pull requests should explain the motivation and behavior change, list manual verification steps, link relevant issues, and include screenshots for visible UI changes or sample KML details for output changes.
