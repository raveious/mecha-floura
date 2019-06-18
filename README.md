# mecha-floura
Mechanical floura project

# Environment setup

Environment was based on instructions on the [getting started guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started-cmake/get-started-devkitc.html)

To build the development environment:
```
$ docker build -t mecha-floura .
```

To use the development environment after building it:
```
$ docker run -it --rm --device /dev/ttyUSB0 -v $(pwd):/mecha-floura mecha-floura
```
