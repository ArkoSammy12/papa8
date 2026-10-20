# papa8

A CHIP-8 emulator written in C

## Usage

Run the built executable and pass in the CHIP-8 binary ROM file as a command-line argument.

```
./papa8 /roms/1dcell.ch
```

If your file path includes whitespace, surround the path with quotation marks `"`.

```
./papa8 /roms/my game.ch
```

## Building

Make sure to have a C compiler that supports the C23 standard installed. Clang is recommended as it is what this project uses.

Clone the project and run the following in the top level directory:

```
cmake -S . -B build
cmake --build build
```

A binary should have then been generated in `/papa8/build/`.

## License

This project is licensed under the [MIT License](LICENSE).
