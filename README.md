# Plane Sailing

Plane Sailing is a Monte Carlo flight-path reconstruction tool created to support the continuing search for **VH-MDX**, a Cessna 210 that disappeared with five people aboard near the Mount Barrington ranges in New South Wales on 9 August 1981.

For the history of the disappearance, current research, and field-search activity, visit the [VH-MDX search project](https://vhmdx.com.au/).

## What it does

The application estimates possible impact locations from the final reported phase of the flight. Each simulation samples uncertainties in:

- radio-fix range and bearing;
- aircraft heading, bank rate, and bank-rate acceleration;
- the reported time, altitude, and airspeed profile; and
- wind direction and wind speed at different altitudes.

The resulting impact positions are accumulated into a search grid. The application exports nominal flight paths and the relative density of simulated impact locations as KML for inspection in Google Earth or another KML viewer.

The supplied `MLE V3 (2018)` data set is based on the parameters published in [VH-MDX Most Likely Extent V3](https://vhmdx.com.au/wp-content/uploads/2015/08/VH-MDX-Most-Likely-Extent-V3.pdf). Values are editable and can be saved as named data sets. Custom data sets are preserved during upgrades; older copies of the built-in MLE preset are upgraded automatically.

The MLE defines bounded areas rather than probability distributions. For this stochastic model, each published MLE bearing or range bound is represented as a three-standard-deviation limit around its midpoint. The default profile integrates the MLE's 50°M–80°M ground-track range directly at its 196-knot ground speed for 246 seconds. This ground speed combines the analysis's 146-knot true airspeed and simplified 50-knot tailwind, so wind is not applied again during integration. The confirmed Williamtown radar-head coordinates come from [Maximum Possible Extent of Flight V3C](https://vhmdx.com.au/wp-content/uploads/2015/08/Max-Possible-Extent-of-Flight-V3C.pdf). This is a Monte Carlo interpretation of the MLE inputs, not a reproduction of its geometric extent calculation.

## Requirements

- Qt 6 development libraries, including Qt Widgets
- qmake 6
- a C++17 compiler
- POSIX threads (`pthread`)

The current threading implementation is POSIX-specific. Building on Windows therefore requires a toolchain that supplies `pthread`, such as an appropriate MinGW environment.

## Building

Build outside the source directory to keep generated files separate:

```sh
mkdir -p build
cd build
qmake6 ../plane-sailing.pro
make -j$(nproc)
```

Then run:

```sh
./plane-sailing
```

## Using the application

1. Select an existing data set or enter a name for a new one.
2. Review the fixed position, calculation, flight-profile, uncertainty, and wind-profile values.
3. Select **Save** to retain edited values in `settings.ini` beside the executable.
4. Select **Start** to run the simulation; select **Cancel** to stop it early.
5. Open the generated `track.kml`, also written beside the executable, in a KML viewer.

The **Iterations** value is a base-10 exponent: a value of `6` requests approximately `10^6` trials. Grid size is controlled by the number of square cells and metres per cell. Bearings are entered in degrees, ranges in nautical miles, altitudes in feet, speeds in knots, and times as `HH:MM:SS`.

## Coordinate and model notes

The reconstruction uses legacy AMG66/MGRS coordinates in UTM zone 56 and converts output to WGS84 for KML. Some coordinate handling is specific to grid cell `56HLJ`. Results depend entirely on the chosen assumptions, uncertainty distributions, simplified flight dynamics, and source observations. They should be treated as a research aid for prioritising search areas, not as a definitive crash location or operational navigation product.
