# Q wrapper for FXCM's ForexConnect API

*Note: This repository is still under development!*

Download the ForexConnect API at <http://fxcodebase.com/wiki/index.php/Download>.

Create a symbolic link to your ForexConnect API with
````
ln -s /path/to/your/ForexConnectAPI ForexConnectAPI
````

You can build the project using the cmake build system (available at <http://cmake.org/cmake/resources/software.html>).
To build a sample, just run `build.sh`.

Make sure to symlink to `libForexConnect.dylib` and `libsample_tools.dylib` in `/usr/local/lib` so that this library can find them when loaded into Q:
````
ln -s /path/to/ForexConnectAPI/lib/libForexConnect.dylib /usr/local/lib
ln -s /path/to/ForexConnectAPI/samples/cpp/sample_tools/lib/libsample_tools.dylib /usr/local/lib
````
