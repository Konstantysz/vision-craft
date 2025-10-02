# Minimal Example

This is the simplest possible kappa-core application. It demonstrates:

- Creating an `Application` with a specification
- Adding a custom `Layer`
- Clearing the screen with a color
- Basic logging

## Building

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Running

```bash
./build/MinimalExample
```

You should see:
- A window titled "kappa-core Minimal Example" (1280x720)
- A dark blue background
- Log messages in the console
- The window remains open until you close it

## Next Steps

From here you can:
1. Add rendering code in `OnRender()`
2. Add update logic in `OnUpdate()`
3. Experiment with the logging system
4. Add event handling with `OnEvent()`
5. Create multiple layers for different purposes
