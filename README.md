# linemt5

```sh
pacman -S zlib-devel
```

# Building
### Configure
```sh
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
```
### Build
```sh
cmake --build .
```