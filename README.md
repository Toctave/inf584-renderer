# INF584 renderer & StyLit implementation

![StyLit demonstration image](report/beauty.png)

## What is this ?

This repository is a from-scratch implementation of a pathtracer, along with an implementation of the paper "StyLit: Illumination-Guided Example-Based Stylization of 3D Renderings" by Fišer et al.

The above image illustrates the subject of the paper : given a simple render of a glossy ball (top-left), and a stylized version of it that was drawn by hand (top-right), the algorithm is able to synthesize a stylized version of any render (bottom row). For more detail, see [my full report](report/main.pdf). 

## Build instructions

SDL must be installed (`sudo apt install libsdl2-dev` on a Debian-based distro). All other dependencies are included

```
mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
```

## Running the programs 

The above creates two executables : `renderer` and `stylit`.

`renderer` operates on a `.toml` scene file (examples are provided in the `scenes` directory) :

```
# Simplest command :
./build/renderer scenes/scene0.toml

# With options :
./build/renderer -w 1024 -h 1024 -s 1000 -o scene0_1000samples scenes/scene0.toml "L*E" "LDE" "L*DDE"
```

`stylit` works on `.png` image files (examples are provided in the stylit-tests directory), which must all have the same size. Unfiltered images must also have the same channel count.

```
# Single channel image :
./build/stylit source_unfiltered.png , source_filtered.png , target_unfiltered.png , target_output.png

# Multi-channel image :
./build/stylit su1.png su2.png su3.png su4.png , sf.png , tu1.png tu2.png tu3.png tu4.png , tf.png

# Options :
./build/stylit su1.png su2.png su3.png su4.png , sf.png , tu1.png tu2.png tu3.png tu4.png , tf.png --levels 6 --kappa 1 --epsilon 10
```
