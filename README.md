# libZpg [![Build status](https://ci.appveyor.com/api/projects/status/tb9yl3u79j0t3v78?svg=true)](https://ci.appveyor.com/project/Tardo/zpg) [![CircleCI](https://circleci.com/gh/Tardo/Zpg.svg?style=svg)](https://circleci.com/gh/Tardo/Zpg)
Library & Tools for Game Assets Packaging

## Features
- Load assets in memory on demand.
- DEFLATE compression
- Easy usage
- Included tool for manage .zpg files

## Dependencies
- zlib

## Library Build & Installation
#### Linux
```sh
~$ mkdir build && cd build
~$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE:STRING=Release .. && make && sudo make install
```
#### Windows
```batch
md build32 & cd build32
cmake -Werror=dev -G"Visual Studio 14 2015" ..
cd ..
cmake --build build32 --config Release --target install
```

## 'zpg_packer' Tool Build
#### Linux
```sh
~$ mkdir build && cd build
~$ cmake -DCMAKE_BUILD_TYPE:STRING=Release .. && make zpg_packer
```
#### Windows
```batch
md build32 & cd build32
cmake -Werror=dev -G"Visual Studio 14 2015" ..
cd ..
cmake --build build32 --config Release --target zpg_packer
```

## Library Basic Usage Example
#### Write
```cpp
#include <Zpg/Zpg.hpp>

int main()
{
  Zpg myZpg;
  
  if (!myZpg.addFromFile("assets/images/image.png", "data/image.png") 
        || !myZpg.addFromFile("assets/docs/text.txt", "data/mytext.txt"))
    return -1;
    
  myZpg.saveToFile("myassets.zpg");

  return 0;
}
```

#### Read
```cpp
#include <Zpg/Zpg.hpp>

int main()
{
  Zpg myZpg;
  if (!myZpg.load("myassets.zpg"))
    return -1;
    
  unsigned long imageSize = 0;
  const unsigned char *pFileData = myZpg.getFileData("data/image.png", &imageSize);
  // Do something with 'pFileData'
  unsigned long textSize = 0;
  const unsigned char *pFileTextData = myZpg.getFileData("data/mytext.txt", &textSize);
  std::string myString = Zpg::toString(pFileTextData, textSize);
  // Do something with 'myString'
  myZpg.unloadData("data/mytext.txt")
  
  // Don't need free any data... LibZpg handles it by itself.
  // But you can 'force' it: myZpg.unloadAll()
  return 0;
}
```

## 'zpg_packer' Tool Basic Usage Example
Syntaxis: ``zpg_packer <file> <options>``
#### Create .zpg file
```sh
~$ zpg_packer assets.zpg -C -A data/

```
More info: https://github.com/Tardo/Zpg/blob/master/tools/zpg_packer.cpp